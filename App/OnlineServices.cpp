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

#include "OnlineServices.h"

#include "App.h"
#include "Settings.h"
#include "Shared/MetaXmlReader.h"
#include "VersionCheckDialog.h"

#include <QDate>
#include <QDesktopServices>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

// const strings
#define YOUTUBE_URL QUrl("https://www.youtube.com/results?search_query=Fotowall+tutorial")
#define TUTORIAL_URL QUrl("https://fosswire.com/post/2008/09/fotowall-make-wallpaper-collages-from-your-photos/")
#define TUTORIAL_STRING "Peter walks you through how to use Foto"
#define FOTOWALL_WEBSITE_STRING "https://www.enricoros.com/opensource/fotowall/"
#define ENRICO_BLOG_STRING "https://www.enricoros.com/opensource/fotowall/news/"

OnlineServices::OnlineServices(QNetworkAccessManager * nam, QObject * parent)
: QObject(parent), m_nam(nam), m_haveTutorial(false)
{
  // set the global reference
  App::onlineServices = this;

#if !defined(NO_UPDATE_CHECK)
  // start-up operations
  autoUpdate();
#endif
}

OnlineServices::~OnlineServices()
{
  // unset the global reference
  App::onlineServices = 0;
}

bool OnlineServices::checkForTutorial()
{
  // if altready found, don't check again
  if(m_haveTutorial) return true;

  // try to get the tutorial page (note, multiple QNAMs will be deleted on app closure)
  QNetworkReply * reply = m_nam->get(QNetworkRequest(TUTORIAL_URL));
  connect(reply, SIGNAL(finished()), this, SLOT(slotCheckTutorialReply()));
  return false;
}

void OnlineServices::openWebpage()
{
  // if not fetched, queue a post-operation
  MetaXml::Connector * conn = MetaXml::Connector::instance();
  if(!conn->hasDone())
  {
    if(m_postOps.isEmpty())
    {
      connect(conn, SIGNAL(fetched()), this, SLOT(slotConnectorFinished()));
      connect(conn, SIGNAL(fetchError(const QString &)), this, SLOT(slotConnectorFinished()));
    }
    m_postOps.append(OpenWebsite);
    return;
  }

  // if already fetched, grab the url and go
  if(conn->reader())
  {
    foreach(const MetaXml::Website & site, conn->reader()->websites)
    {
      if(site.type == MetaXml::Website::HomePage)
      {
        QDesktopServices::openUrl(QUrl(site.url));
        return;
      }
    }
  }
  QDesktopServices::openUrl(QUrl(FOTOWALL_WEBSITE_STRING));
}

void OnlineServices::openBlog()
{
  // if not fetched, queue a post-operation
  MetaXml::Connector * conn = MetaXml::Connector::instance();
  if(!conn->hasDone())
  {
    if(m_postOps.isEmpty())
    {
      connect(conn, SIGNAL(fetched()), this, SLOT(slotConnectorFinished()));
      connect(conn, SIGNAL(fetchError(const QString &)), this, SLOT(slotConnectorFinished()));
    }
    m_postOps.append(OpenBlog);
    return;
  }

  // ask for confirmation
  if(QMessageBox::question(0, tr("Opening Fotowall's author Blog"),
                           tr("This is the blog of the main author of Fotowall.\nDo you want to open the web page?"),
                           QMessageBox::Yes, QMessageBox::No)
     == QMessageBox::No)
    return;

  // if already fetched, grab the url and go
  if(conn->reader())
  {
    foreach(const MetaXml::Website & site, conn->reader()->websites)
    {
      if(site.type == MetaXml::Website::Blog)
      {
        QDesktopServices::openUrl(QUrl(site.url));
        return;
      }
    }
  }
  QDesktopServices::openUrl(QUrl(ENRICO_BLOG_STRING));
}

void OnlineServices::openYouTube()
{ QDesktopServices::openUrl(YOUTUBE_URL); }

void OnlineServices::openTutorial()
{
  int answer = QMessageBox::question(0, tr("Opening the Web Tutorial"),
                                     tr("The Tutorial is provided on Fosswire by Peter Upfold.\nIt's about Fotowall "
                                        "0.2 a rather old version.\nDo you want to open the web page?"),
                                     QMessageBox::Yes, QMessageBox::No);
  if(answer == QMessageBox::Yes) QDesktopServices::openUrl(TUTORIAL_URL);
}

#if !defined(NO_UPDATE_CHECK)
void OnlineServices::checkForUpdates()
{
  VersionCheckDialog vcd;
  vcd.exec();
  App::settings->setValue("Fotowall/LastUpdateCheck", QDate::currentDate());
}

void OnlineServices::autoUpdate()
{
  // find out the time of the last update check
  QDate lastCheck = App::settings->value("Fotowall/LastUpdateCheck").toDate();
  if(lastCheck.isNull())
  {
    App::settings->setValue("Fotowall/LastUpdateCheck", QDate::currentDate());
    return;
  }

  // check for updates 30 days after the last one
  if(lastCheck.daysTo(QDate::currentDate()) > 30) QTimer::singleShot(2000, this, SLOT(checkForUpdates()));
}
#endif

void OnlineServices::slotCheckTutorialReply()
{
  // set the m_haveTutorial variable if the tutorial was found & is valid
  QNetworkReply * reply = static_cast<QNetworkReply *>(sender());
  if(reply->error() != QNetworkReply::NoError) return;
  QString htmlCode = reply->readAll();
  bool haveTutorial = htmlCode.contains(TUTORIAL_STRING, Qt::CaseInsensitive);

  // notify the listeners about any change
  if(haveTutorial != m_haveTutorial)
  {
    m_haveTutorial = haveTutorial;
    emit tutorialFound(m_haveTutorial);
  }
}

void OnlineServices::slotConnectorFinished()
{
  // perform post-ops now that the connector has finished
  while(!m_postOps.isEmpty())
  {
    switch(m_postOps.takeFirst())
    {
      case OpenWebsite:
        openWebpage();
        break;
      case OpenBlog:
        openBlog();
        break;
    }
  }
}
