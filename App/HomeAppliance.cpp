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

#include "HomeAppliance.h"

#include "App.h"
#include "HomeScene.h"
#include "Settings.h"
#include "Shared/PixmapButton.h"
#include "UrlHistoryBox.h"
#include "Workflow.h"

#include <QHBoxLayout>

/** File (Open) groupbox **/

class FileBoxWidget : public GroupBoxWidget
{
public:
  PixmapButton * openButton;

  FileBoxWidget() : openButton(0)
  {
    // our layout
    QHBoxLayout * lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    // add open button
    openButton = new PixmapButton(this);
    openButton->setFixedSize(QSize(64, 60));
    openButton->setToolTip(QObject::tr("Open"));
    openButton->setPixmap(QPixmap(":/data/action-open.png"));
    lay->addWidget(openButton);
  }
};

/** Home Appliance **/

HomeAppliance::HomeAppliance(QObject * parent) : QObject(parent), m_scene(0), m_fileBox(0), m_historyBox(0)
{
  // create and set the scene
  m_scene = new HomeScene;
  connect(m_scene, SIGNAL(keyPressed(int)), this, SLOT(slotSceneKeyPressed(int)));
  connect(m_scene, SIGNAL(startCanvas()), this, SLOT(slotStartCanvas()));
#if defined(HAS_WORDCLOUD_APPLIANCE)
  connect(m_scene, SIGNAL(startWordcloud()), this, SLOT(slotStartWordcloud()));
#endif
  connect(m_scene, SIGNAL(startWizard()), this, SLOT(slotStartWizard()));
  sceneSet(m_scene);

  // create the History Box, if enough history
  QList<QUrl> recentUrls = App::settings->recentFotowallUrls();
  QPalette brightPal;
  brightPal.setBrush(QPalette::Window, QColor(255, 255, 255, 128));
  if(!recentUrls.isEmpty())
  {
    m_historyBox = new UrlHistoryBox(recentUrls);
    m_historyBox->setTitle(tr("RECENT FILES"));
#if !defined(MOBILE_UI)
    m_historyBox->setBorderFlags(0x0000);
    m_historyBox->setCheckable(false);
#endif
    m_historyBox->setPalette(brightPal);
    m_historyBox->setAutoFillBackground(true);
    connect(m_historyBox, SIGNAL(urlClicked(const QUrl &)), this, SLOT(slotLoadCanvas(const QUrl &)));
    connect(m_historyBox, SIGNAL(urlRemoved(const QUrl &)), this, SLOT(slotRemoveFromHistory(const QUrl &)));
    topbarAddWidget(m_historyBox);
  }

  // create the File Box
  m_fileBox = new FileBoxWidget;
  m_fileBox->setTitle(tr("OPEN"));
#if !defined(MOBILE_UI)
  m_fileBox->setBorderFlags(0x0000);
  m_fileBox->setCheckable(false);
#endif
  m_fileBox->setPalette(brightPal);
  m_fileBox->setAutoFillBackground(true);
  connect(m_fileBox->openButton, SIGNAL(clicked()), this, SLOT(slotOpenFile()));
  topbarAddWidget(m_fileBox);
}

HomeAppliance::~HomeAppliance()
{
  delete m_fileBox;
  delete m_historyBox;
  delete m_scene;
}

void HomeAppliance::slotSceneKeyPressed(int qtKey)
{
  // pressed a number, activate relative Url
  if(qtKey >= Qt::Key_1 && qtKey <= Qt::Key_9)
  {
    QUrl url = m_historyBox->urlForEntry(qtKey - Qt::Key_0 - 1);
    if(!url.isEmpty()) slotLoadCanvas(url);
  }
}

void HomeAppliance::slotLoadCanvas(const QUrl & url)
{
  // handle fotowall files
  if(App::isFotowallFile(url.toString()))
  {
    App::workflow->loadCanvas_A(url.toString());
    return;
  }

  // handle other files
  qWarning("HomeAppliance::slotLoadCanvas: don't know how to load URL '%s'", qPrintable(url.toString()));
}

void HomeAppliance::slotRemoveFromHistory(const QUrl & url)
{
  // remove url
  App::settings->removeRecentFotowallUrl(url);

  // change urls
  QList<QUrl> recentUrls = App::settings->recentFotowallUrls();
  m_historyBox->changeUrls(recentUrls, false);
}

void HomeAppliance::slotOpenFile()
{ App::workflow->loadCanvas_A(); }

void HomeAppliance::slotStartCanvas()
{ App::workflow->startCanvas_A(); }

#if defined(HAS_WORDCLOUD_APPLIANCE)
void HomeAppliance::slotStartWordcloud()
{ App::workflow->startWordcloud_A(); }
#endif

void HomeAppliance::slotStartWizard()
{ HERE }
