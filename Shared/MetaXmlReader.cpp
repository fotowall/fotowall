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

#include "MetaXmlReader.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>

#define METAXML_BASE_URL "http://www.enricoros.com/opensource/fotowall/.meta"
#define NETWORK_TIMEOUT 10000

/* pre-0.8.0 used this settings:
    #define OLD_CHECK_URL "http://code.google.com/p/fotowall/"
    #define OLD_MAGIC_TOKEN "Last version is "
    #define OLD_DOWNLOADS_URL "http://code.google.com/p/fotowall/downloads/list"
*/

using namespace MetaXml;

Reader_1::Reader_1(const QByteArray & data) : QXmlStreamReader(data)
{ read(); }

void Reader_1::read()
{
  while(!atEnd())
  {
    readNext();
    if(isEndElement()) break;
    if(isStartElement())
    {
      if(name() == QLatin1String("fotowall-meta"))
        continue;
      else if(name() == QLatin1String("releases"))
        readReleases();
      else if(name() == QLatin1String("websites"))
        readWebsites();
      else
        readElementText();
    }
  }
}

void Reader_1::readReleases()
{
  releases.clear();
  while(!atEnd())
  {
    readNext();
    if(isEndElement()) break;
    if(isStartElement())
    {
      if(name() == QLatin1String("release"))
        releases.append(readRelease());
      else
        readElementText();
    }
  }
}

Release Reader_1::readRelease()
{
  Release r;
  r.name = attributes().value("name").toString();
  while(!atEnd())
  {
    readNext();
    if(isEndElement()) break;
    if(isStartElement())
    {
      if(name() == QLatin1String("version"))
        r.version = readElementText();
      else if(name() == QLatin1String("download-url"))
        r.url = readElementText();
      else
        readElementText();
    }
  }
  return r;
}

void Reader_1::readWebsites()
{
  websites.clear();
  while(!atEnd())
  {
    readNext();
    if(isEndElement()) break;
    if(isStartElement())
    {
      if(name() == QLatin1String("homepage") || name() == QLatin1String("blog") || name() == QLatin1String("site"))
      {
        Website w;
        w.name = attributes().value("name").toString();
        w.url = readElementText();
        if(name() == QLatin1String("homepage"))
          w.type = Website::HomePage;
        else if(name() == QLatin1String("blog") || w.name.contains("blog", Qt::CaseInsensitive))
          w.type = Website::Blog;
        else
          w.type = Website::Other;
        if(name() == QLatin1String("homepage"))
          websites.prepend(w);
        else
          websites.append(w);
      }
      else
        readElementText();
    }
  }
}

/* Connector */

Q_GLOBAL_STATIC(Connector, connectorInstance);
Connector * Connector::instance()
{ return connectorInstance(); }

Connector::Connector() : m_nam(new QNetworkAccessManager), m_reader(0)
{
  QNetworkRequest request(QUrl(METAXML_BASE_URL));
  QNetworkReply * reply = m_nam->get(request);
  connect(reply, SIGNAL(finished()), this, SLOT(slotGotReply()));
  QTimer::singleShot(NETWORK_TIMEOUT, this, SLOT(slotTimeOut()));
}

bool Connector::hasDone() const
{ return !m_nam; }

bool Connector::isValid() const
{ return m_reader; }

const Reader_1 * Connector::reader() const
{ return m_reader; }

void Connector::slotGotReply()
{
  // dispose the QNAM
  if(!m_nam) return;
  m_nam->deleteLater();
  m_nam = 0;

  // get the data from the network reply
  QNetworkReply * reply = static_cast<QNetworkReply *>(sender());
  QByteArray replyData = reply->readAll();
  QNetworkReply::NetworkError error = reply->error();
  reply->deleteLater();
  if(error != QNetworkReply::NoError)
  {
    emit fetchError(tr("Network Error"));
    return;
  }

  // parse the data and notify the completion
  delete m_reader;
  m_reader = new Reader_1(replyData);
  emit fetched();
}

void Connector::slotTimeOut()
{
  // dispose the QNAM
  if(!m_nam) return;
  m_nam->deleteLater();
  m_nam = 0;

  // emit the error signal
  emit fetchError(tr("Network Timeout"));
}
