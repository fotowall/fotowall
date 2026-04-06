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

#include "AbstractAppliance.h"

#include <QVariant>

using namespace PlugGui;

AbstractAppliance::~AbstractAppliance()
{
  // development checks: ensure stuff has been deleted
  QList<WidgetPointer> list = m_pTopbar;
  list.append(m_pSidebar);
  list.append(m_pCentral);
  int notKilled = 0;
  foreach(const WidgetPointer & pointer, list)
    if(pointer) notKilled++;
  if(notKilled) qWarning("AbstractAppliance::~AbstractAppliance: some pointers (%d) not destroyed", notKilled);
  if(m_pScene) qWarning("AbstractAppliance::~AbstractAppliance: scene not taken before destroying");
}

bool AbstractAppliance::addToApplianceContainer(Container * container)
{
  // remove from previous, if exists
  if(m_containerPtr)
  {
    qWarning("AbstractAppliance::addToContainer: removing from previous container");
    removeFromApplianceContainer();
  }

  // add the appliance to the container
  m_containerPtr = container;
  if(m_containerPtr)
  {
    m_containerPtr->applianceSetTitle(m_windowTitle);
    m_containerPtr->applianceSetScene(m_pScene.data());
    updateContainerTopbar();
    m_containerPtr->applianceSetSidebar(m_pSidebar.data());
    m_containerPtr->applianceSetCentralwidget(m_pCentral.data());
    for(ValueMap::iterator it = m_values.begin(); it != m_values.end(); ++it)
      m_containerPtr->applianceSetValue(it.key(), it.value());
  }
  return true;
}

void AbstractAppliance::removeFromApplianceContainer()
{
  // sanity check
  if(!m_containerPtr)
  {
    qWarning("AbstractAppliance::removeFromContainer: not on container");
    return;
  }

  // do the clearance
  detachFromContainer();
  m_containerPtr = 0;
}

bool AbstractAppliance::applianceCommand(int command)
{
  qWarning("AbstractAppliance::applianceCommand: current appliance '%s' doesn't handle command %d",
           qPrintable(applianceName()), command);
  return false;
}

bool AbstractAppliance::appliancePendingChanges() const
{ return false; }

bool AbstractAppliance::applianceSave(const QString & filePath)
{
  qWarning("AbstractAppliance::applianceSaveAs: saving not implemented for %s to '%s'", qPrintable(applianceName()),
           qPrintable(filePath));
  return true;
}

void AbstractAppliance::windowTitleSet(const QString & title)
{
  m_windowTitle = title;
  if(m_containerPtr) m_containerPtr->applianceSetTitle(m_windowTitle);
}

void AbstractAppliance::windowTitleClear()
{ windowTitleSet(QString()); }

void AbstractAppliance::sceneSet(AbstractScene * scene)
{
  m_pScene = scene;
  if(m_containerPtr) m_containerPtr->applianceSetScene(m_pScene.data());
}

void AbstractAppliance::sceneClear()
{ sceneSet(0); }

void AbstractAppliance::topbarAddWidget(QWidget * widget, bool rightBar, int index)
{
  if(!widget) return;
  WidgetPointer wPtr(widget);
  widget->setProperty("@rightBar", rightBar);
  if(index < 0 || index >= m_pTopbar.size())
    m_pTopbar.append(wPtr);
  else
    m_pTopbar.insert(index, wPtr);
  updateContainerTopbar();
}

void AbstractAppliance::topbarRemoveWidget(QWidget * widget)
{
  // remove given + any dead widgets
  bool removed = false;
  QList<WidgetPointer>::iterator it = m_pTopbar.begin();
  while(it != m_pTopbar.end())
  {
    WidgetPointer wPtr = *it;
    if(!wPtr || wPtr.data() == widget)
    {
      it = m_pTopbar.erase(it);
      removed = true;
    }
    else
      ++it;
  }

  // set bar only if something changed
  if(removed) updateContainerTopbar();
}

void AbstractAppliance::sidebarSetWidget(QWidget * widget)
{
  m_pSidebar = widget;
  if(m_containerPtr) m_containerPtr->applianceSetSidebar(m_pSidebar.data());
}

void AbstractAppliance::sidebarClearWidget()
{ sidebarSetWidget(0); }

void AbstractAppliance::centralwidgetSet(QWidget * widget)
{
  m_pCentral = widget;
  if(m_containerPtr) m_containerPtr->applianceSetCentralwidget(m_pCentral.data());
}

void AbstractAppliance::centralwidgetClear()
{ centralwidgetSet(0); }

void AbstractAppliance::containerValueSet(quint32 key, const QVariant & value)
{
  if(value.isValid())
    m_values[key] = value;
  else
    m_values.remove(key);
  if(m_containerPtr) m_containerPtr->applianceSetValue(key, value);
}

void AbstractAppliance::setFocusToScene()
{
  if(m_containerPtr) m_containerPtr->applianceSetFocusToScene();
}

void AbstractAppliance::updateContainerTopbar()
{
  if(!m_containerPtr) return;
  QList<QWidget *> widgets;
  foreach(WidgetPointer wpointer, m_pTopbar)
    if(wpointer) widgets.append(wpointer.data());
  m_containerPtr->applianceSetTopbar(widgets);
}

void AbstractAppliance::detachFromContainer()
{
  if(m_containerPtr)
  {
    m_containerPtr->applianceSetTitle(QString());
    m_containerPtr->applianceSetScene(0);
    m_containerPtr->applianceSetTopbar(QList<QWidget *>());
    m_containerPtr->applianceSetSidebar(0);
    m_containerPtr->applianceSetCentralwidget(0);
    for(ValueMap::iterator it = m_values.begin(); it != m_values.end(); ++it)
      m_containerPtr->applianceSetValue(it.key(), QVariant());
  }
}
