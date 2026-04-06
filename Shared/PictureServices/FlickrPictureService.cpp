/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2007-2009 by Enrico Ros <enrico.ros@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "FlickrPictureService.h"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QXmlStreamAttributes>
#include <QXmlStreamReader>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#  include <QUrlQuery>
#endif

namespace FlickrInternal
{
struct F_PhotoDescription
{
  QString id; // 3743354066
  QString owner; // 22046259@N04
  QString secret; // b0ba1ca158
  QString server; // 2646
  QString farm; // 3
  QString title; // Rescale Test 1
  bool ispublic; // 1
  bool isfriend; // 0
  bool isfamily; // 0
  QUrl url_t; // http://farm3.static.flickr.com/2548/3744077174_3f6efa0789_t.jpg
  int height_t; // 100
  int width_t; // 75
  QUrl url_o; // http://farm3.static.flickr.com/2548/3744077174_296535ff22_o.jpg
  int height_o; // 3000
  int width_o; // 4000
};

struct Photo
{
  int idx;
  F_PhotoDescription description;
  bool thumbRequested;
  QPixmap thumbnail;
  QList<QNetworkReply *> jobs;

  Photo() : idx(0), thumbRequested(false) {}
  ~Photo()
  {
    foreach(QNetworkReply * job, jobs)
    {
      job->disconnect(0, 0, 0);
      job->abort();
      job->deleteLater();
    }
  }
};
} // namespace FlickrInternal

FlickrPictureService::FlickrPictureService(const QString & apiKey, QNetworkAccessManager * manager, QObject * parent)
: AbstractPictureService(manager, parent), m_apiKey(apiKey), m_searchJob(0)
{
}

FlickrPictureService::~FlickrPictureService()
{ dropSearch(); }

void FlickrPictureService::searchPics(const QString & text)
{
  // clear previous search results
  dropSearch();

  // notify about the start
  emit searchStarted();

  // make request and connect to reply
  KeyValueList options;
  options << KeyValue("text", text);
  options << KeyValue("extras", "url_t,url_o");
  m_searchJob = flickrApiCall("flickr.photos.search", options);
  connect(m_searchJob, SIGNAL(finished()), this, SLOT(slotSearchJobFinished()));
  // connect(m_searchJob, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(...);
  // connect(m_searchJob, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(slotdownloadProgress(qint64, qint64)));
}

void FlickrPictureService::dropSearch()
{
  if(m_searchJob)
  {
    m_searchJob->disconnect(0, 0, 0);
    m_searchJob->abort();
    m_searchJob->deleteLater();
    m_searchJob = 0;
  }
  qDeleteAll(m_searchResults);
  m_searchResults.clear();
  foreach(QNetworkReply * reply, m_prefetches)
  {
    reply->disconnect(0, 0, 0);
    reply->abort();
    reply->deleteLater();
  }
  m_prefetches.clear();
  emit searchEnded(false);
}

bool FlickrPictureService::imageInfo(int idx, QString * url, QString * title, int * width, int * height)
{
  if(idx < 0 || idx >= m_searchResults.size()) return false;
  FlickrInternal::Photo * photo = m_searchResults[idx];
  *url = photo->description.url_o.toString();
  *title = photo->description.title;
  *width = photo->description.width_o;
  *height = photo->description.height_o;
  return true;
}

QNetworkReply * FlickrPictureService::download(int idx)
{
  if(idx < 0 || idx >= m_searchResults.size()) return 0;

  // return an existing prefetch, if any
  if(m_prefetches.contains(idx)) return m_prefetches.take(idx);

  // or start a new transfer
  return get(m_searchResults[idx]->description.url_o);
}

void FlickrPictureService::startPrefetch(int idx)
{
#if QT_VERSION >= 0x040600
  if(idx < 0 || idx >= m_searchResults.size() || m_prefetches.contains(idx)) return;
  m_prefetches[idx] = get(m_searchResults[idx]->description.url_o);
#else
  // no precaching with <= 4.5 since QNetworkReply::isFinished doesn't exist
  Q_UNUSED(idx);
#endif
}

void FlickrPictureService::stopPrefetch(int idx)
{
  if(!m_prefetches.contains(idx)) return;
  QNetworkReply * reply = m_prefetches.take(idx);
  reply->disconnect(0, 0, 0);
  reply->abort();
  reply->deleteLater();
}

void FlickrPictureService::slotSearchJobFinished()
{
  QByteArray replyContent = m_searchJob->readAll();
  m_searchJob->disconnect(0, 0, 0);
  m_searchJob->deleteLater();
  m_searchJob = 0;

  // read response
  int idx = 0;
  QXmlStreamReader sr(replyContent);
  while(!sr.atEnd())
  {
    sr.readNext();
    if(sr.isStartElement() && sr.name().toString() == "photo")
    {

      // create the F_PhotoDescription
      FlickrInternal::F_PhotoDescription pd;
      QXmlStreamAttributes attribs = sr.attributes();
      pd.id = attribs.value("id").toString();
      pd.owner = attribs.value("owner").toString();
      pd.secret = attribs.value("secret").toString();
      pd.server = attribs.value("server").toString();
      pd.farm = attribs.value("farm").toString();
      pd.title = attribs.value("title").toString();
      pd.ispublic = attribs.value("ispublic").toString() != "0";
      pd.isfriend = attribs.value("isfriend").toString() != "0";
      pd.isfamily = attribs.value("isfamily").toString() != "0";
      pd.url_t = attribs.value("url_t").toString();
      pd.height_t = attribs.value("height_t").toString().toUInt();
      pd.width_t = attribs.value("width_t").toString().toUInt();
      pd.url_o = attribs.value("url_o").toString();
      pd.height_o = attribs.value("height_o").toString().toUInt();
      pd.width_o = attribs.value("width_o").toString().toUInt();

      // if no original, use standard url (info on http://www.flickr.com/services/api/misc.urls.html)
      if(pd.url_o.isEmpty())
        pd.url_o = QString("http://farm%1.static.flickr.com/%2/%3_%4.jpg")
                       .arg(pd.farm)
                       .arg(pd.server)
                       .arg(pd.id)
                       .arg(pd.secret);

      // create a new Photo
      FlickrInternal::Photo * photo = new FlickrInternal::Photo();
      photo->idx = idx++;
      photo->description = pd;
      m_searchResults.append(photo);

      // notify about the new item
      emit searchResult(photo->idx, pd.title, pd.width_t, pd.height_t);
    }
  }

  // start 2 thumbnail jobs
  if(startNextThumbnailJobs(2)) emit searchEnded(false);
}

void FlickrPictureService::slotThumbJobFinished()
{
  QNetworkReply * job = static_cast<QNetworkReply *>(sender());
  QByteArray replyContent = job->readAll();
  job->deleteLater();

  // find the Photo that owns the job
  foreach(FlickrInternal::Photo * photo, m_searchResults)
  {

    // remove the job from the photo
    if(!photo->jobs.removeAll(job)) continue;

    // stop if network Error
    if(job->error() != QNetworkReply::NoError) break;

    // make pixmap from the network data
    QImage image = QImage::fromData(replyContent);
    if(image.isNull()) break;
    photo->thumbnail = QPixmap::fromImage(image);
    emit searchThumbnail(photo->idx, photo->thumbnail);
    break;
  }

  // start a new job
  if(startNextThumbnailJobs(1)) emit searchEnded(false);
}

bool FlickrPictureService::startNextThumbnailJobs(int count)
{
  bool finished = true;
  foreach(FlickrInternal::Photo * photo, m_searchResults)
  {
    if(photo->thumbRequested) continue;

    if(count-- <= 0) break;

    // start the thumbnail job
    QNetworkReply * job = get(photo->description.url_t);
    connect(job, SIGNAL(finished()), this, SLOT(slotThumbJobFinished()));
    photo->jobs << job;
    photo->thumbRequested = true;
    finished = false;
  }
  return finished;
}

QNetworkReply * FlickrPictureService::flickrApiCall(const QString & method, const KeyValueList & params)
{
  KeyValueList fullList;
  fullList.append(KeyValue("method", method));
  fullList.append(KeyValue("api_key", m_apiKey));
  fullList.append(params);

  // build URL with method+apikey+params
  QUrl url("https://api.flickr.com/services/rest/");
  QUrlQuery query;
  query.setQueryItems(fullList);
  url.setQuery(query);
  return get(url);
}
