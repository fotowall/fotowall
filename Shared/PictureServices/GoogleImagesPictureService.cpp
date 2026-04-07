/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Original work                                                         *
 *      file             : mainwindow.cpp                                  *
 *      license          : GPL v3                                          *
 *      copyright notice : follows below                                   *
 *                                                                         *
 ***************************************************************************/

//        Google Image Downloader. Search and download as many images fit your search criteria
//        Copyright (C) 2009  Marco Bavagnoli - lil.deimos@gmail.com
//
//        This program is free software: you can redistribute it and/or modify
//        it under the terms of the GNU General Public License as published by
//        the Free Software Foundation, either version 3 of the License, or
//        (at your option) any later version.
//
//        This program is distributed in the hope that it will be useful,
//        but WITHOUT ANY WARRANTY; without even the implied warranty of
//        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//        GNU General Public License for more details.
//
//        You should have received a copy of the GNU General Public License
//        along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "GoogleImagesPictureService.h"

#include <QDebug>
#include <QNetworkReply>

namespace GoogleImagesInternal
{

struct G_PhotoDescription
{
  QString href_thumbnail_at_google; // http://www.tktpoint.it/public/zucchero01din_5.jpg
  QString href_google_thumb_download; // http://t0.gstatic.com/images
  QString ID_google_thumb; // TkcA1QGjiRrHKM:
  QString href_original_page; // http://www.tktpoint.it/news_dett.asp%3FIDNews%3D81
  QString desc; // 500 x 448 - 41k
  QString kind; // jpg
  QString url_t; // http://t0.gstatic.com/images?q=tbn:TkcA1QGjiRrHKM:http://www.tktpoint.it/public/zucchero01din_5.jpg
  int width_t; // 130
  int height_t; // 116
  QString url_o; // http://www.tktpoint.it/public/zucchero01din_5.jpg
  int height_o; // TODO parse desc
  int width_o; // TODO parse desc
};

struct Photo
{
  int idx;
  G_PhotoDescription description;
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
} // namespace GoogleImagesInternal

GoogleImagesPictureService::GoogleImagesPictureService(QNetworkAccessManager * manager, QObject * parent)
: AbstractPictureService(manager, parent), m_contentType(0), m_sizeType(0), m_searchJob(0)
{
  // init regular expressions
  m_rxStart.setPattern("setResults[(][[]");
  // m_rxData retrieves a string with all data of the thumbnail
  m_rxData.setPattern("[[]\".*[]],");
  m_rxData.setMinimal(true);
  m_rxHref.setPattern("(http://.*)[\\\\|\"]");
  m_rxHref.setMinimal(true);
  m_rxOther.setPattern(",[\"](.*)[\"]");
  m_rxOther.setMinimal(true);
}

GoogleImagesPictureService::~GoogleImagesPictureService()
{ dropSearch(); }

void GoogleImagesPictureService::configure(int contentType, int sizeType)
{
  m_contentType = contentType;
  m_sizeType = sizeType;
}

void GoogleImagesPictureService::searchPics(const QString & text)
{
  // clear previous search results
  dropSearch();

  // notify about the start
  emit searchStarted();

  // build the google images search string
  QString requestString = "&q=" + QUrl::toPercentEncoding(text);
#if 1
  int startImageNumber = 0;
  requestString += "&start=" + QString::number(startImageNumber);
#endif
#if 1
  // 0=any content  1=news content  2=faces  3=photo content  4=clip art 5=line drawings
  QString content_type;
  switch(m_contentType)
  {
    case 0:
      content_type = "";
      break;
    case 1:
      content_type = "news";
      break;
    case 2:
      content_type = "face";
      break;
    case 3:
      content_type = "photo";
      break;
    case 4:
      content_type = "clipart";
      break;
    case 5:
      content_type = "lineart";
      break;
  }
  if(!content_type.isEmpty()) requestString += "&imgtype=" + content_type;
#endif
#if 1
  QString image_size;
  switch(m_sizeType)
  {
    case 0:
      image_size = "";
      break;
    case 1:
      image_size = "m";
      break;
    case 2:
      image_size = "l";
      break;
    case 3:
      image_size = "i";
      break;
    case 4:
      image_size = "qsvga";
      break;
    case 5:
      image_size = "vga";
      break;
    case 6:
      image_size = "svga";
      break;
    case 7:
      image_size = "xga";
      break;
    case 8:
      image_size = "2mp";
      break;
    case 9:
      image_size = "4mp";
      break;
    case 10:
      image_size = "6mp";
      break;
    case 11:
      image_size = "8mp";
      break;
    case 12:
      image_size = "10mp";
      break;
    case 13:
      image_size = "12mp";
      break;
    case 14:
      image_size = "15mp";
      break;
    case 15:
      image_size = "20mp";
      break;
    case 16:
      image_size = "40mp";
      break;
    case 17:
      image_size = "70mp";
      break;
  }
  if(!image_size.isEmpty()) requestString += "&imgsz=" + image_size;
#endif
#if 0
    switch (ui->CB_coloration->currentIndex()) {
        case 0: coloration=""; break;
        case 1: coloration="gray"; break;
        case 2: coloration="color"; break;
    }
    requestString += "&imgc=" + coloration;
#endif
#if 0
    switch (ui->CB_filter->currentIndex()) {
        case 0: safeFilter="off"; break;
        case 1: safeFilter="images"; break;
        case 2: safeFilter="active"; break;
    }
    requestString += "&safe=" + safeFilter;
#endif
#if 0
    site_search = ui->CB_domain->currentText();
    if (!site_search.isEmpty())
        requestString += "&as_sitesearch=" + site_search;
#endif

  // make request and connect to reply
  QUrl url("http://images.google.com/images");
  url.setQuery(requestString.toLatin1());
  m_searchJob = get(url);
  connect(m_searchJob, SIGNAL(finished()), this, SLOT(slotSearchJobFinished()));
}

void GoogleImagesPictureService::dropSearch()
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

bool GoogleImagesPictureService::imageInfo(int idx, QString * url, QString * title, int * width, int * height)
{
  if(idx < 0 || idx >= m_searchResults.size()) return false;
  GoogleImagesInternal::Photo * photo = m_searchResults[idx];
  *url = photo->description.url_o;
  *title = QString(); // TODO
  *width = photo->description.width_o;
  *height = photo->description.height_o;
  return true;
}

QNetworkReply * GoogleImagesPictureService::download(int idx)
{
  if(idx < 0 || idx >= m_searchResults.size()) return 0;

  // return an existing prefetch, if any
  if(m_prefetches.contains(idx)) return m_prefetches.take(idx);

  // or start a new transfer
  return get(m_searchResults[idx]->description.url_o);
}

void GoogleImagesPictureService::startPrefetch(int idx)
{
#if QT_VERSION >= 0x040600
  if(idx < 0 || idx >= m_searchResults.size() || m_prefetches.contains(idx)) return;
  m_prefetches[idx] = get(m_searchResults[idx]->description.url_o);
#else
  // no precaching with <= 4.5 since QNetworkReply::isFinished doesn't exist
  Q_UNUSED(idx);
#endif
}

void GoogleImagesPictureService::stopPrefetch(int idx)
{
  if(!m_prefetches.contains(idx)) return;
  QNetworkReply * reply = m_prefetches.take(idx);
  reply->disconnect(0, 0, 0);
  reply->abort();
  reply->deleteLater();
}

bool GoogleImagesPictureService::parseGoogleSearchReply(const QByteArray & data)
{
  int missingImages = 99;

  // find out the start
  QString html = data;
  int pos = m_rxStart.indexIn(html);
  if(pos == -1)
  {
    qWarning("GoogleImagesPictureService::parseSearchReply: Something changed in the google image page html "
             "source.\nThis function must be rewritten");
    qWarning() << html;
    return false;
  }

  // process the html
  int idx = 0;
  html.remove(0, pos + m_rxStart.cap(0).size());
  pos = 0;
  while((pos = m_rxData.indexIn(html, pos)) != -1 && missingImages--)
  {
    int pos2 = 0;
    int i = 0;

    GoogleImagesInternal::G_PhotoDescription pd;

    QString dataString = m_rxData.cap(0);
    dataString.replace("\\", "\\\\");
    pos += m_rxData.matchedLength();

    // skip https images
    if(dataString.contains("https:")) continue;

    // Get image hrefs: (http://.*)[\\\\|\"]
    // 0 occurrence = source image href
    // 1 occurrence = href to attach to google image to retrieve thumbnail
    // 2 occurrence = source image href
    // 3 occurrence = as the 1st
    // 4 occurrence = google href for thumbnails
    while((pos2 = m_rxHref.indexIn(dataString, pos2)) != -1)
    {
      switch(i)
      {
        case 0:
          pd.href_thumbnail_at_google = m_rxHref.cap(1);
          break;
        case 1:
          pd.href_original_page = m_rxHref.cap(1);
          break;
        case 2:
          pd.url_o = m_rxHref.cap(1);
          pd.url_o = QUrl(pd.url_o).fromPercentEncoding(pd.url_o.toLocal8Bit());
          break;
        case 3:
          pd.href_google_thumb_download = m_rxHref.cap(1);
          break;
      }
      pos2 += m_rxHref.matchedLength();
      i++;
    }

    // Get other data: ,[\"](.*)[\"]
    // cap 1 = ID image at google download image thumb
    // cap 2 = source image href
    // cap 4 = thumb x res
    // cap 5 = thumb y res
    // cap 6 = image description with some strange characters
    // cap 9 = source image values ( with x height x Kb )
    // cap 10= kind of image (jpg, png ecc)
    // cap 11= source image site
    // cap 14= source image site (same as href_google_thumb_download)
    pos2 = 0;
    i = 0;
    while((pos2 = m_rxOther.indexIn(dataString, pos2)) != -1)
    {
      // qDebug() << m_rxOther.cap(0);
      switch(i)
      {
        case 1:
          pd.ID_google_thumb = m_rxOther.cap(1);
          // Example to combine data for thumbnail:
          // http://tbn1.google.com/images?q=tbn:jKItZeo94DSqzM:http://www.scott-eaton.com/news/uploaded_images/ecorcheZbrush-723429.jpg
          pd.url_t = pd.href_google_thumb_download + "?q=tbn:" + pd.ID_google_thumb + pd.href_thumbnail_at_google;
          break;
        case 2:
          break;
        case 3:
          pd.width_t = m_rxOther.cap(1).toInt();
          break;
        case 4:
          pd.height_t = m_rxOther.cap(1).toInt();
          break;
        case 5:
          break;
        case 6:
          break;
        case 7:
          break;
        case 8:
          pd.desc = m_rxOther.cap(1);
          pd.width_o = pd.desc.section(' ', 0, 0).toInt();
          pd.height_o = pd.desc.section(' ', 2, 2).toInt();
          break;
        case 9:
          pd.kind = m_rxOther.cap(1);
          break;
      }
      pos2 += m_rxOther.matchedLength();
      i++;
    }

    // create a new Photo
    GoogleImagesInternal::Photo * photo = new GoogleImagesInternal::Photo();
    photo->idx = idx++;
    photo->description = pd;
    m_searchResults.append(photo);

    // notify about the new item
    emit searchResult(photo->idx, pd.desc, pd.width_t, pd.height_t);
  }
  return true;
}

bool GoogleImagesPictureService::startNextThumbnailJobs(int count)
{
  bool finished = true;
  foreach(GoogleImagesInternal::Photo * photo, m_searchResults)
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

void GoogleImagesPictureService::slotSearchJobFinished()
{
  // QByteArray replyContent = QByteArray::fromPercentEncoding(m_searchJob->readAll());
  QByteArray replyContent = m_searchJob->readAll();
  m_searchJob->disconnect(0, 0, 0);
  m_searchJob->deleteLater();
  m_searchJob = 0;

  // TODO add a logic for pre/post image count (to fetch other images)

  // parse the search reply
  parseGoogleSearchReply(replyContent);

  // start 2 thumbnail jobs
  if(startNextThumbnailJobs(2)) emit searchEnded(false);
}

void GoogleImagesPictureService::slotThumbJobFinished()
{
  QNetworkReply * job = static_cast<QNetworkReply *>(sender());
  QByteArray replyContent = job->readAll();
  job->deleteLater();

  // find the Photo that owns the job
  foreach(GoogleImagesInternal::Photo * photo, m_searchResults)
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
