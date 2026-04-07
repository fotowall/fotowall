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

#include "HelpAppliance.h"

#include "App.h"
#include "HelpItem.h"
#include "OnlineServices.h"
#include "Shared/AbstractScene.h"
#include "Shared/RenderOpts.h"
#include "Workflow.h"

#include <QDesktopServices>
#include <QLinearGradient>
#include <QPainter>

/** Help Scene **/

class HelpScene : public AbstractScene
{
public:
  HelpItem * helpItem;

  HelpScene() : helpItem(0)
  {
    // create and place the help item
    helpItem = new HelpItem;
    helpItem->setZValue(10001);
    addItem(helpItem);
  }

  bool sceneSelectable() const { return false; }

  void resizeEvent() { helpItem->setPos(QPointF(sceneCenter().toPoint())); }

  void drawBackground(QPainter * painter, const QRectF & rect)
  {
    // draw a gray shade as background
    painter->setCompositionMode(QPainter::CompositionMode_Source);
    QLinearGradient lg(0, 0, 0, sceneHeight());
    lg.setColorAt(0.0, QColor(192, 192, 192, RenderOpts::ARGBWindow ? 200 : 255));
    lg.setColorAt(1.0, QColor(128, 128, 128, RenderOpts::ARGBWindow ? 200 : 255));
    painter->fillRect(rect, lg);
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
  }
};

/** Help Appliance **/

HelpAppliance::HelpAppliance(QObject * parent) : QObject(parent), m_helpScene(new HelpScene), m_dummyWidget(new QWidget)
{
  // create the gui components over a dummy widget
  ui.setupUi(m_dummyWidget);
  ui.tutorialButton->setVisible(App::onlineServices->checkForTutorial());
  connect(App::onlineServices, SIGNAL(tutorialFound(bool)), ui.tutorialButton, SLOT(setVisible(bool)));
  connect(ui.webButton, SIGNAL(clicked()), App::onlineServices, SLOT(openWebpage()));
  connect(ui.blogButton, SIGNAL(clicked()), App::onlineServices, SLOT(openBlog()));
  connect(ui.youtubeButton, SIGNAL(clicked()), App::onlineServices, SLOT(openYouTube()));
  connect(ui.tutorialButton, SIGNAL(clicked()), App::onlineServices, SLOT(openTutorial()));
#if !defined(NO_UPDATE_CHECK)
  connect(ui.updateButton, SIGNAL(clicked()), App::onlineServices, SLOT(checkForUpdates()));
#else
  ui.updateButton->setVisible(false);
#endif

  // listen to scene events
  connect(m_helpScene->helpItem, SIGNAL(closeMe()), this, SLOT(slotClose()));

  // customize appliance
  sceneSet(m_helpScene);
  topbarAddWidget(ui.helpBox);
  setFocusToScene();
}

HelpAppliance::~HelpAppliance()
{
  delete ui.helpBox;
  delete m_helpScene;
  delete m_dummyWidget;
}

void HelpAppliance::slotClose()
{ App::workflow->popCurrentAppliance(); }
