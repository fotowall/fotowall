/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "Workflow.h"

#include "App.h"
#include "Canvas/Canvas.h"
#include "CanvasAppliance.h"
#include "FotowallFile.h"
#include "HelpAppliance.h"
#include "HomeAppliance.h"
#include "Settings.h"
#include "Shared/BreadCrumbBar.h"
#include "Shared/ButtonsDialog.h"
#include "Shared/PlugGui/Container.h"
#if defined(HAS_WORDCLOUD_APPLIANCE)
#  include "WordcloudAppliance.h"
#endif

#include <QTimer>
#include <QVariant>

// on mobile the "cancel" button doesn't behave in a good way.. use the "no" one
#if defined(MOBILE_UI)
#  define STANDARDBUTTON_NEGATIVE QDialogButtonBox::No
#else
#  define STANDARDBUTTON_NEGATIVE QDialogButtonBox::Cancel
#endif

Workflow::Workflow(PlugGui::Container * container, BreadCrumbBar * bar, QObject * parent)
: QObject(parent), m_container(container), m_bar(bar), m_commandTimer(0), m_processingQueue(false)
{
  // set the global reference
  App::workflow = this;

  // wire up the stacker and the bar
  connect(m_bar, SIGNAL(nodeClicked(quint32)), this, SLOT(slotNodeClicked(quint32)));

  // load a fotowall file if asked from the command line
  QStringList contentUrls = App::settings->commandlineUrls();
  if(contentUrls.size() == 1 && App::isFotowallFile(contentUrls.first()))
  {
    if(loadCanvas_A(contentUrls.first())) return;
  }

  // load content in a canvas
  if(!contentUrls.isEmpty())
  {
    Canvas * canvas = new Canvas(m_container->physicalDpiX(), m_container->physicalDpiY(), this);
    canvas->resize(m_container->sceneViewSize());
    canvas->addAutoContent(contentUrls);
    pushNode(new CanvasAppliance(canvas));
    return;
  }

  // show the home screen (if no valid commandline args)
  pushNode(new HomeAppliance);
}

Workflow::~Workflow()
{
  // warn if we have any appliance left, shouldn't be
  while(!m_stack.isEmpty())
  {
    qWarning("Workflow::~Workflow: not empty stack. dropping all");
    popNode(false);
  }

  // unset the global reference
  App::workflow = 0;
  m_container = 0;

  // this is an example of 'autosave-like function'
  // QString tempPath = QDir::tempPath() + QDir::separator() + "autosave.fotowall";
  // m_canvas->saveToFile(tempPath);

  // bar and container are external: don't delete
}

bool Workflow::loadCanvas_A(const QString & __fwFilePath)
{
  // ask for file name, if not provided - can CANCEL
  QString fwFilePath = __fwFilePath.isEmpty() ? FotowallFile::getLoadFotowallFile() : __fwFilePath;
  if(fwFilePath.isEmpty()) return false;

  // schedule canvas loading
  scheduleCommand(Command::ResetToLevel);
  scheduleCommand(Command(Command::MasterCanvas, fwFilePath));
  return true;
}

void Workflow::startCanvas_A()
{
  // schedule canvas creation
  scheduleCommand(Command::ResetToLevel);
  scheduleCommand(Command::MasterCanvas);
}

void Workflow::stackSlaveCanvas_A(SingleResourceLoaner * resource)
{
  // schedule slave canvas
  scheduleCommand(Command(Command::SlaveCanvas, QVariant(), resource));
}

#if defined(HAS_WORDCLOUD_APPLIANCE)
void Workflow::startWordcloud_A()
{
  // schedule wordcloud creation
  scheduleCommand(Command::ResetToLevel);
  scheduleCommand(Command::MasterWordcloud);
}

void Workflow::stackSlaveWordcloud_A(SingleResourceLoaner * resource)
{
  // schedule slave wordcloud
  scheduleCommand(Command(Command::SlaveWordcloud, QVariant(), resource));
}
#endif

void Workflow::stackHelpAppliance()
{
  // if no help app, immediately stack it
  if(m_stack.isEmpty() || !dynamic_cast<HelpAppliance *>(m_stack.last().appliance)) pushNode(new HelpAppliance);
}

void Workflow::popCurrentAppliance()
{
  if(!m_stack.isEmpty()) scheduleCommand(Command(Command::ResetToLevel, m_stack.size() - 1));
}

QString Workflow::applianceName() const
{
  if(!m_stack.isEmpty()) return m_stack.last().appliance->applianceName();
  return "Workflow";
}

bool Workflow::applianceCommand(int command)
{
  if(!m_stack.isEmpty()) m_stack.last().appliance->applianceCommand(command);
  return false;
}

bool Workflow::requestExit()
{
  // save only if the user wants so
  bool allowSaving = false;

  // don't show the dialog from the home screen
  if(m_stack.size() > 1)
  {

    // count master appliances that can be saved
    int requiringSave = 0;
    foreach(const Node & node, m_stack)
      if(!node.loaner && node.appliance->appliancePendingChanges()) ++requiringSave;

    // build the closure dialog
    ButtonsDialog quitAsk("Workflow-Exit", tr("Closing Fotowall..."));
    quitAsk.setMinimumWidth(350);
    quitAsk.setButtonText(STANDARDBUTTON_NEGATIVE, tr("Cancel"));
    if(requiringSave)
    {
      quitAsk.setMessage(tr("Are you sure you want to quit and lose your changes?"));
      quitAsk.setButtonText(QDialogButtonBox::Save, tr("Save"));
      quitAsk.setButtonText(QDialogButtonBox::Close, tr("Don't Save"));
      quitAsk.setButtons(QDialogButtonBox::Save | QDialogButtonBox::Close | STANDARDBUTTON_NEGATIVE);
    }
    else
    {
      quitAsk.setMessage(tr("Are you sure you want to quit?"));
      quitAsk.setButtonText(QDialogButtonBox::Close, tr("Quit"));
      quitAsk.setButtons(QDialogButtonBox::Close | STANDARDBUTTON_NEGATIVE);
    }

    // react to the dialog's answer
    QDialogButtonBox::StandardButton button = quitAsk.execute();
    if(button == STANDARDBUTTON_NEGATIVE) return false;
    if(button == QDialogButtonBox::Save) allowSaving = true;
  }

  // close (and maybe save) appliances
  while(!m_stack.isEmpty())
  {
    // can cancel even here
    if(!popNode(allowSaving)) return false;
  }
  return true;
}

void Workflow::scheduleCommand(const Command & command)
{
  if(!m_commandTimer)
  {
    m_commandTimer = new QTimer(this);
    m_commandTimer->setSingleShot(true);
    connect(m_commandTimer, SIGNAL(timeout()), this, SLOT(slotProcessQueue()));
  }
  m_commands.append(command);
  m_commandTimer->start(0);
}

bool Workflow::processCommand(const Workflow::Command & command)
{
  switch(command.type)
  {
    case Command::ResetToLevel:
    {
      int level = qMax(1, command.param.toInt());
      while(m_stack.size() > level && !dynamic_cast<HomeAppliance *>(m_stack.last().appliance))
      {
        // ResetToLevel: save changes on each popped level
        bool allowSaving = false;
        const Node & node = m_stack.last();
        if(!node.loaner && node.appliance->appliancePendingChanges())
        {
          // build the closure dialog
          ButtonsDialog saveDlg("Workflow-ResetToLevel", tr("Closing File"));
          saveDlg.setMinimumWidth(350);
          saveDlg.setButtonText(STANDARDBUTTON_NEGATIVE, tr("Cancel"));
          saveDlg.setMessage(tr("Do you want to save your changes?"));
          saveDlg.setButtonText(QDialogButtonBox::Save, tr("Save"));
          saveDlg.setButtonText(QDialogButtonBox::Close, tr("Don't Save"));
          saveDlg.setButtons(QDialogButtonBox::Save | QDialogButtonBox::Close | STANDARDBUTTON_NEGATIVE);

          // react to the dialog's answer
          QDialogButtonBox::StandardButton button = saveDlg.execute();
          if(button == STANDARDBUTTON_NEGATIVE) return false;
          if(button == QDialogButtonBox::Save) allowSaving = true;
        }

        // pop current node and break if asked for saving and canceled saving
        if(!popNode(allowSaving)) return false;
      }
    }
      return true;

    case Command::MasterCanvas:
    {
      Canvas * canvas = new Canvas(m_container->physicalDpiX(), m_container->physicalDpiY(), this);

      // load a file, if in params
      if(command.param.type() == QVariant::String)
      {
        QString fwFilePath = command.param.toString();
        if(!FotowallFile::read(fwFilePath, canvas, true))
          qWarning("Workflow::processCommand: MasterCanvas: can't load canvas. Display error?");
      }

      // create the canvas appliance
      CanvasAppliance * canvasApp = new CanvasAppliance(canvas);
      pushNode(canvasApp);
    }
      return true;

    case Command::SlaveCanvas:
    {
      // get the Canvas out of the resource
      Canvas * canvas = static_cast<Canvas *>(command.loaner->takeResource().value<void *>());

      // create the canvas appliance
      if(canvas)
      {
        CanvasAppliance * canvasApp = new CanvasAppliance(canvas);
        pushNode(Node(canvasApp, command.loaner));
      }
    }
      return true;

#if defined(HAS_WORDCLOUD_APPLIANCE)
    case Command::MasterWordcloud:
    {
      Wordcloud::Cloud * cloud = new Wordcloud::Cloud(this);

      // TODO: load from file
      if(command.param.type() == QVariant::String) HERE;

      // create the wordcloud appliance
      WordcloudAppliance * wcApp = new WordcloudAppliance(cloud, this);
      pushNode(wcApp);
    }
      return true;

    case Command::SlaveWordcloud:
    {
      // get the Cloud out of the resource
      Wordcloud::Cloud * cloud = static_cast<Wordcloud::Cloud *>(command.loaner->takeResource().value<void *>());

      // create the wordcloud appliance
      WordcloudAppliance * wcApp = new WordcloudAppliance(cloud, this);
      pushNode(Node(wcApp, command.loaner));
    }
      return true;
#endif
  }

  // catch errors
  return false;
}

void Workflow::pushNode(const Node & node)
{
  // remove previous Appliance from container
  if(!m_stack.isEmpty() && m_container) m_stack.last().appliance->removeFromApplianceContainer();
  m_stack.append(node);
  // show this Appliance on container
  if(m_container) node.appliance->addToApplianceContainer(m_container);
  updateBreadcrumb();
}

bool Workflow::popNode(bool allowSave)
{
  // remove last Node and close its Appliance [can be canceled]
  if(!m_stack.isEmpty())
  {

    // handle saving (Master & Changes & !disabled)
    const Node & node = m_stack.last();
    if(allowSave && !node.loaner && node.appliance->appliancePendingChanges())
      if(!node.appliance->applianceSave()) return false;

    // specific resource return/deletion
    if(CanvasAppliance * cApp = dynamic_cast<CanvasAppliance *>(node.appliance))
    {
      Canvas * canvas = cApp->takeCanvas();
      if(node.loaner)
        node.loaner->returnResource(QVariant::fromValue((void *)canvas));
      else
        delete canvas;
#if defined(HAS_WORDCLOUD_APPLIANCE)
    }
    else if(WordcloudAppliance * wApp = dynamic_cast<WordcloudAppliance *>(node.appliance))
    {
      Wordcloud::Cloud * cloud = wApp->takeCloud();
      if(node.loaner)
        node.loaner->returnResource(QVariant::fromValue((void *)cloud));
      else
        delete cloud;
#endif
    }
    else if(!dynamic_cast<HomeAppliance *>(node.appliance) && !dynamic_cast<HelpAppliance *>(node.appliance))
      qWarning("Workflow::popNode: pop of appliance '%s' not handled", qPrintable(node.appliance->applianceName()));

    // now that we popped, delete the Appliance
    if(m_container) node.appliance->removeFromApplianceContainer();
    delete node.appliance;
    m_stack.removeLast();
  }

  // show the last-1
  if(m_container && !m_stack.isEmpty()) m_stack.last().appliance->addToApplianceContainer(m_container);

  updateBreadcrumb();

  // tell that we popped
  return true;
}

void Workflow::updateBreadcrumb()
{
  // build the new breadcrumbbar's contents
  m_bar->clearNodes();
  if(!m_stack.isEmpty())
  {
    quint32 index = 0;
    foreach(const Node & node, m_stack)
    {
      m_bar->addNode(index + 1, node.appliance->applianceName(), index);
      index++;
    }
  }

  // issue a complete repaint
  m_container->update();
}

void Workflow::slotNodeClicked(quint32 level)
{ scheduleCommand(Command(Command::ResetToLevel, level)); }

void Workflow::slotProcessQueue()
{
  if(m_processingQueue)
  {
    qWarning("Workflow::slotProcessQueue: nesting detected -FIXME-");
    return;
  }
  m_processingQueue = true;
  while(!m_commands.isEmpty())
  {
    // process the topmost command...
    if(!processCommand(m_commands.takeFirst()))
    {
      // ...and drop queue if it fails
      m_commands.clear();
    }
  }
  m_processingQueue = false;
}
