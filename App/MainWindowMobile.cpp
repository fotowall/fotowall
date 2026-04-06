/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2010 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "App/MainWindowMobile.h"

#include "App.h"
#include "OnlineServices.h"
#include "PictureSearchWidget.h"
#include "SceneView.h"
#include "Settings.h"
#include "Shared/BreadCrumbBar.h"
#include "Shared/GroupBoxWidget.h"
#include "Shared/RenderOpts.h"
#include "Workflow.h"

#include <QApplication>
#include <QBoxLayout>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QFont>
#include <QFrame>
#include <QNetworkAccessManager>
#include <QPropertyAnimation>
#include <QVariant>

class NavibarContainer : public QWidget
{
public:
  NavibarContainer(QWidget * parent = 0) : QWidget(parent)
  {
    setMinimumHeight(16);
    setAutoFillBackground(false);
  }
};

class TopbarContainer : public QWidget
{
public:
  TopbarContainer(QWidget * parent = 0) : QWidget(parent)
  {
    // setFrameStyle(QFrame::NoFrame);
    setAutoFillBackground(false);
  }

  void mousePressEvent(QMouseEvent * event) { event->ignore(); }
};

MainWindowMobile::MainWindowMobile(QWidget * parent)
: PlugGui::Container(parent), m_networkAccessManager(new QNetworkAccessManager), m_pictureSearch(0), m_sceneView(0),
  m_topbarContainer(0)
{
  // setup widget
  applianceSetTitle(QString());

  // vertical layout
  QVBoxLayout * mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  // container for the navigation bar
  QWidget * nbContainer = new NavibarContainer(this);
  mainLayout->addWidget(nbContainer);
  QLayout * nbLayout = new QHBoxLayout(nbContainer);
  nbLayout->setContentsMargins(0, 0, 0, 0);

  // create the Workflow navigation bar
  BreadCrumbBar * workflowBar = new BreadCrumbBar(nbContainer);
  workflowBar->setObjectName(QString::fromUtf8("applianceNavBar"));
  workflowBar->setClickableLeaves(false);
  workflowBar->setBackgroundOffset(-1);
  nbLayout->addWidget(workflowBar);
  workflowBar->show();

  nbLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));

  // create the exit button on the right side
  BreadCrumbBar * exitBar = new BreadCrumbBar(nbContainer);
  connect(exitBar, SIGNAL(nodeClicked(quint32)), qApp, SLOT(quit()));
  exitBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
  exitBar->setBackgroundOffset(1);
  exitBar->addNode(1, tr("Exit"), 0);
  nbLayout->addWidget(exitBar);
  exitBar->show();

  // create the Help navigation bar
  /*BreadCrumbBar * helpBar = new BreadCrumbBar(m_sceneView);
  connect(helpBar, SIGNAL(nodeClicked(quint32)), this, SLOT(slotHelpBarClicked(quint32)));
  helpBar->setBackgroundOffset(1);
  helpBar->addNode(1, tr(" ? "), 0);
  helpBar->show();
  addNavigationWidget(helpBar, 0, Qt::AlignRight);*/

  // the scene view, where applicances will plug into!
  m_sceneView = new SceneView(this);
  mainLayout->addWidget(m_sceneView);
  m_sceneView->setFrameShape(QFrame::NoFrame);

  // the topbar container, populated by the framework
  m_topbarContainer = new TopbarContainer(m_sceneView);
  m_topbarContainer->setFixedHeight(App::TopBarHeight);
  m_topbarContainer->show();
  QHBoxLayout * topbarLayout = new QHBoxLayout(m_topbarContainer);
  topbarLayout->setContentsMargins(0, 0, 0, 0);
  topbarLayout->setSpacing(0);

  // show (with last geometry)
  showFullScreen();

  // start the workflow
  new Workflow((PlugGui::Container *)this, workflowBar);

  // create the online services
  new OnlineServices(m_networkAccessManager);

  // focus the scene
  applianceSetFocusToScene();

#if 0
    // start with the Help appliance the first time
    if (App::settings->firstTime())
        App::workflow->stackHelpAppliance();
#endif
}

MainWindowMobile::~MainWindowMobile()
{
  // delete everything
  delete App::workflow;
  delete App::onlineServices;
  delete m_networkAccessManager;
}

QSize MainWindowMobile::sceneViewSize() const
{ return m_sceneView->viewport()->size(); }

void MainWindowMobile::applianceSetTitle(const QString & title)
{
  QString tString = title.isEmpty() ? QString() : title + " - ";
  tString += QCoreApplication::applicationName() + " ";
  if(title.isEmpty()) tString += tr("'RETRO' ");
  tString += QCoreApplication::applicationVersion();
  setWindowTitle(tString);
}

void MainWindowMobile::applianceSetScene(AbstractScene * scene)
{ m_sceneView->setScene(scene); }

void MainWindowMobile::applianceSetTopbar(const QList<QWidget *> & widgets)
{
  // hide all the current panels
  foreach(GroupBoxWidget * panel, m_panels)
  {
    disconnect(panel, 0, this, 0);
    panel->hide();
    panel->setParent(0);
  }
  m_panels.clear();

  // clear the topbar layout hiding all widgets
  QLayout * topbarLayout = m_topbarContainer->layout();
  while(QLayoutItem * item = topbarLayout->takeAt(0))
  {
    if(QWidget * embeddedWidget = item->widget()) embeddedWidget->setVisible(false);
    delete item;
  }

  // add the widgets to..
  QList<QWidget *> reverseWidgets = widgets;
  while(!reverseWidgets.isEmpty())
  {
    QWidget * widget = reverseWidgets.takeLast();
    // ..the manually handled topbar, or
    if(widget->property("@onTopbar").toBool() || !widget->inherits("GroupBoxWidget"))
    {
      topbarLayout->addWidget(widget);
      widget->setFixedHeight(App::TopBarHeight);
      widget->setVisible(true);
      continue;
    }
    // ..enable as containers
    GroupBoxWidget * g = static_cast<GroupBoxWidget *>(widget);
    connect(g, SIGNAL(panelRaising()), this, SLOT(slotHideOtherPanels()));
    connect(g, SIGNAL(panelLowering()), this, SLOT(slotShowOtherPanels()));
    connect(g, SIGNAL(labelSizeChanged()), this, SLOT(slotPanelLabelSizeChanged()));
    g->setSmartPanel(true);
    g->setParent(m_sceneView);
    g->setFixedHeight(App::TopBarHeight);
    g->setVisible(true);
    g->show();
    m_panels.append(g);
  }

  // reposition items
  resizeEvent(0);
}

void MainWindowMobile::applianceSetSidebar(QWidget * sidebar)
{
  if(sidebar) qWarning("MainWindowMobile::applianceSetSidebar: not used on mobile");
}

void MainWindowMobile::applianceSetCentralwidget(QWidget * widget)
{
  if(widget) qWarning("MainWindowMobile::applianceSetCentralwidget: not used on mobile");
}

void MainWindowMobile::applianceSetValue(quint32 key, const QVariant & value)
{
  if(key == App::CC_ShowPictureSearch)
  {

    // destroy if needed
    bool visible = value.toBool();
    if(!visible && m_pictureSearch)
    {
      m_pictureSearch->deleteLater();
      m_pictureSearch = 0;
      return;
    }

    // create if needed
    if(visible && !m_pictureSearch)
    {
      m_pictureSearch = new PictureSearchWidget(m_networkAccessManager);
      connect(m_pictureSearch, SIGNAL(requestClosure()), this, SLOT(slotClosePictureSearch()), Qt::QueuedConnection);
      // m_pictureSearch->setWindowIcon(QIcon(":/data/insert-download.png"));
      m_pictureSearch->setWindowTitle(tr("Search Web Pictures"));
      m_pictureSearch->setWindowFlags(Qt::Tool);
      QPoint newPos = mapToGlobal(m_sceneView->pos()) + QPoint(-20, 10);
      if(newPos.x() < 0) newPos.setX(0);
      m_pictureSearch->move(newPos);
      m_pictureSearch->layout()->activate();
      m_pictureSearch->resize(50, 50);
      m_pictureSearch->show();
      m_pictureSearch->setFocus();
      return;
    }
  }
  else if(key == App::CC_HideTopBar)
  {

    bool hidden = value.toBool();

    int x = m_topbarContainer->x();
    QPropertyAnimation * ani = new QPropertyAnimation(m_topbarContainer, "pos", m_topbarContainer);
    ani->setEndValue(hidden ? QPoint(x, -App::TopBarHeight) : QPoint(x, 0));
    ani->setEasingCurve(QEasingCurve::OutQuad);
    ani->setDuration(200);
    ani->start(QAbstractAnimation::DeleteWhenStopped);
    if(hidden)
      connect(ani, SIGNAL(finished()), m_topbarContainer, SLOT(hide()));
    else
      m_topbarContainer->show();
  }
  else
    qWarning("MainWindowMobile::applianceSetValue: unknown key 0x%x", key);
}

void MainWindowMobile::applianceSetFocusToScene()
{ m_sceneView->setFocus(Qt::OtherFocusReason); }

void MainWindowMobile::closeEvent(QCloseEvent * event)
{ event->setAccepted(App::workflow->requestExit()); }

void MainWindowMobile::resizeEvent(QResizeEvent *)
{
  // resize the topbar container
  const int supposedTopWidth = 300;
  m_topbarContainer->setGeometry((width() - supposedTopWidth) / 2, 0, supposedTopWidth, App::TopBarHeight);

  // reposition the Smart Panels
  int baseY = m_sceneView->height();
  int baseX = width() > 400 ? 60 : 0;
  foreach(GroupBoxWidget * panel, m_panels)
  {
    panel->setBasePos(QPoint(baseX, baseY));
    baseX += qMin(200, panel->labelWidth()) + 8;
  }
}

void MainWindowMobile::slotClosePictureSearch()
{ App::workflow->applianceCommand(App::AC_ClosePicureSearch); }

void MainWindowMobile::slotHideOtherPanels()
{
  GroupBoxWidget * lowering = static_cast<GroupBoxWidget *>(sender());
  foreach(GroupBoxWidget * panel, m_panels)
    if(panel != lowering) panel->smartFall();
}

void MainWindowMobile::slotShowOtherPanels()
{
  GroupBoxWidget * lowering = static_cast<GroupBoxWidget *>(sender());
  foreach(GroupBoxWidget * panel, m_panels)
    if(panel != lowering) panel->smartRaise();
}

void MainWindowMobile::slotPanelLabelSizeChanged()
{ resizeEvent(0); }
