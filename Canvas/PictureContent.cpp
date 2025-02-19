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

#include "PictureContent.h"

#include "Shared/Commands.h"
#include "Shared/Compat.h"

#include "ButtonItem.h"
#include "Shared/CPixmap.h"
#include "Shared/CroppingDialog.h"

#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QGraphicsScene>
#include <QGraphicsSceneDragDropEvent>
#include <QMessageBox>
#include <QMimeData>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#if !defined(__EMSCRIPTEN__)
#  include <QProcess>
#endif
#include <QTimer>
#include <QUrl>

#include "App/App.h"
#include "App/Settings.h"
#include "Frames/Frame.h"
#include "PictureProperties.h"
#include "Shared/PropertyEditors.h"
#include "Shared/RenderOpts.h"

PictureContent::PictureContent(bool spontaneous, QGraphicsScene * scene, QGraphicsItem * parent)
: AbstractContent(scene, spontaneous, false, parent), m_photo(0), m_opaquePhoto(false), m_progress(0), m_netWidth(0),
  m_netHeight(0), m_netReply(0), m_watcher(0), m_watcherTimer(0)
{
  // enable frame text
  setFrameTextEnabled(true);
  setFrameText(tr("..."));

  // allow dropping
  setAcceptDrops(true);

  // add flipping buttons
  ButtonItem * bFlipH = new ButtonItem(ButtonItem::FlipH, Qt::blue, QIcon(":/data/action-flip-horizontal.png"), this);
  bFlipH->setToolTip(tr("Flip horizontally"));
  bFlipH->setFlag(QGraphicsItem::ItemIgnoresTransformations, false);
  connect(bFlipH, SIGNAL(clicked()), this, SIGNAL(flipHorizontally()));
  addButtonItem(bFlipH);

  ButtonItem * bFlipV = new ButtonItem(ButtonItem::FlipV, Qt::blue, QIcon(":/data/action-flip-vertical.png"), this);
  bFlipV->setToolTip(tr("Flip vertically"));
  bFlipV->setFlag(QGraphicsItem::ItemIgnoresTransformations, false);
  addButtonItem(bFlipV);
  connect(bFlipV, SIGNAL(clicked()), this, SIGNAL(flipVertically()));

  ButtonItem * bRotate = new ButtonItem(ButtonItem::Rotate, Qt::blue, QIcon(":/data/action-rotate.png"), this);
  bRotate->setToolTip(tr("Rotate"));
  bRotate->setFlag(QGraphicsItem::ItemIgnoresTransformations, false);
  addButtonItem(bRotate);
  connect(bRotate, SIGNAL(clicked()), this, SLOT(slotRotate()));

#if 0
    // add cropping button (TODO: enable this?)
    ButtonItem * bCrop = new ButtonItem(ButtonItem::Control, Qt::blue, QIcon(":/data/action-scale.png"), this);
    bCrop->setToolTip(tr(""));
    bCrop->setFlag(QGraphicsItem::ItemIgnoresTransformations, false);
    addButtonItem(bCrop);
    connect(bCrop, SIGNAL(clicked()), this, SIGNAL(requestCrop()));
#endif
}

PictureContent::~PictureContent()
{
  dropNetworkConnection();
  delete m_watcherTimer;
  delete m_watcher;
  delete m_photo;
}

bool PictureContent::loadFromImage(const QString & picFilePath,
                                   const QImage & img,
                                   bool setUrl,
                                   bool setRatio,
                                   bool setName)
{
  delete m_photo;
  m_cachedPhoto = QPixmap();
  m_opaquePhoto = false;
  m_photo = 0;
  m_netWidth = 0;
  m_netHeight = 0;

  m_photo = new CPixmap(img);
  if(m_photo->isNull())
  {
    delete m_photo;
    m_photo = 0;
    return false;
  }

  m_opaquePhoto = !m_photo->hasAlpha();
  if(setUrl)
  {
    QString prettyUrl = QDir(picFilePath).canonicalPath();
    if(prettyUrl.isEmpty()) prettyUrl = picFilePath;
    m_fileUrls = QStringList(prettyUrl);
  }
  if(setRatio) resetContentsRatio();
  if(setName)
  {
    QString string = QFileInfo(picFilePath).fileName().section('.', 0, 0);
    string = string.mid(0, 10);
    setFrameText(string + tr("..."));
  }
  applyPostLoadEffects();

  // notify image change
  emit contentChanged();
  return true;
}

bool PictureContent::loadFromFile(const QString & picFilePath, bool setUrl, bool setRatio, bool setName)
{
  dropNetworkConnection();
  auto img = QImage(picFilePath);
  return loadFromImage(picFilePath, img, setUrl, setRatio, setName);
}

bool PictureContent::loadFromNetwork(const QString & url,
                                     QNetworkReply * reply,
                                     const QString & title,
                                     int width,
                                     int height)
{
  dropNetworkConnection();
  delete m_photo;
  m_cachedPhoto = QPixmap();
  m_opaquePhoto = false;
  m_photo = 0;
  if(!m_fileUrls.contains(url)) m_fileUrls.append(url);
  m_netWidth = width;
  m_netHeight = height;

  // start a download if not passed as a paramenter
  if(!reply)
  {
    // the QNAM will be auto-deleted on closure
    QNetworkAccessManager * nam = new QNetworkAccessManager(this);
    QNetworkRequest request(url);
    m_netReply = nam->get(request);
  }
  else
    m_netReply = reply;

  // set title
  if(!title.isEmpty())
  {
    setFrameTextEnabled(true);
    setFrameText(title);
  }

#if QT_VERSION >= 0x040600
  // Immediate Decode: just handle the reply if done
  if(m_netReply->isFinished()) return slotLoadNetworkData();
#else
  // No Precaching ensures signals to be emitted later
#endif

  // Deferred Decode: listen to the network job
  setAcceptHoverEvents(false);
  setControlsVisible(false);
  m_progress = 0.01;
  connect(m_netReply, SIGNAL(finished()), this, SLOT(slotLoadNetworkData()));
  connect(m_netReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotNetworkError()));
  connect(m_netReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(slotNetworkProgress(qint64, qint64)));

  // reset size, if got the network one
  if(m_netWidth > 0 && m_netHeight > 0) resetContentsRatio();
  return true;
}

/**
 * The contract of this function is that the bitmap won't be saved, if exported to XML.
 * That's why the name is complex.
 */
bool PictureContent::loadPixmapForAccelTest(const QPixmap & pixmap, const QString & title)
{
  dropNetworkConnection();
  delete m_photo;
  m_cachedPhoto = QPixmap();
  m_opaquePhoto = false;
  m_photo = 0;
  m_fileUrls = QStringList();
  m_netWidth = 0;
  m_netHeight = 0;

  m_photo = new CPixmap(pixmap.toImage());
  m_opaquePhoto = !pixmap.hasAlpha();
  resetContentsRatio();
  setFrameTextEnabled(!title.isEmpty());
  setFrameText(title);
  applyPostLoadEffects();

  // notify image change
  emit contentChanged();
  return true;
}

void PictureContent::addEffect(const PictureEffect & effect)
{
  if(!m_photo) return;

  m_photo->addEffect(effect);
  // adapt picture ratio after cropping
  if(effect.effect == PictureEffect::ClearEffects) setContentOpacity(1);
  if(effect.effect == PictureEffect::Crop)
  {
    QRect actualContentRect = contentRect();
    if((actualContentRect.height() + actualContentRect.width()) > 0)
    {
      qreal reduceRatio = (qreal)(effect.rect.width() + effect.rect.height())
                          / (qreal)(actualContentRect.height() + actualContentRect.width());
      int newW = (int)((qreal)effect.rect.width() / reduceRatio);
      int newH = (int)((qreal)effect.rect.height() / reduceRatio);
      resizeContents(QRect(-newW / 2, -newH / 2, newW, newH), true);
    }
  }
  else if(effect.effect == PictureEffect::Opacity) { setContentOpacity(effect.param); }
  else if(effect.effect == PictureEffect::Rotate)
  {
    QRect newContentRect = contentRect();
    newContentRect.setWidth(contentRect().height());
    newContentRect.setHeight(contentRect().width());
    resizeContents(newContentRect);
  }
  m_cachedPhoto = QPixmap();
  update();
  GFX_CHANGED();

  // notify image change
  emit contentChanged();
}

QList<PictureEffect> PictureContent::effects() const
{
  return m_photo->effects();
}

void PictureContent::crop()
{
  if(!m_photo) return;
  CroppingDialog dial(m_photo);
#if defined(MOBILE_UI)
  dial.showMaximized();
#endif
  if(dial.exec() != QDialog::Accepted) return;
  QRect cropRect = dial.getCroppingRect();
  if(!cropRect.isNull())
  {
    EffectCommand * command = new EffectCommand(this, PictureEffect(PictureEffect::Crop, 0, cropRect));
    do_canvas_command(scene(), command);
  }
}

QWidget * PictureContent::createPropertyWidget(ContentProperties * __p)
{
  PictureProperties * pp = __p ? (PictureProperties *)__p : new PictureProperties;
  AbstractContent::createPropertyWidget(pp);

  // properties link
#if defined(Q_OS_WIN) || defined(Q_OS_OS2)
  delete pp->gimpButton;
#else
  new PE_AbstractButton(pp->gimpButton, this, "externalEdit", pp);
#endif
  connect(pp->cropButton, SIGNAL(clicked()), this, SIGNAL(requestCrop()));

  return pp;
}

static bool hasChildTags(const QDomElement & element, const QString & tagName)
{
  return !element.elementsByTagName(tagName).isEmpty();
}

bool PictureContent::fromXml(const QDomElement & contentElement, const QDir & baseDir)
{
  AbstractContent::fromXml(contentElement, baseDir);

  // build the afterload effects list
  m_afterLoadEffects.clear();
  QDomElement effectsE = contentElement.firstChildElement("effects");
  for(QDomElement effectE = effectsE.firstChildElement("effect"); effectE.isElement();
      effectE = effectE.nextSiblingElement("effect"))
  {
    PictureEffect fx;
    fx.effect = (PictureEffect::Effect)effectE.attribute("type").toInt();
    fx.param = effectE.attribute("param").toDouble();
    if(fx.effect == PictureEffect::Crop)
    {
      QString rect = effectE.attribute("croppingRect");
      QStringList coordinates = rect.split(" ");
      if(coordinates.size() >= 3)
      {
        QRect croppingRect(coordinates.at(0).toInt(), coordinates.at(1).toInt(), coordinates.at(2).toInt(),
                           coordinates.at(3).toInt());
        fx.rect = croppingRect;
      }
    }
    else if(fx.effect == PictureEffect::Opacity)
      setContentOpacity(fx.param);
    m_afterLoadEffects.append(fx);
  }

  // get all the absolute paths/urls
  m_relativePath = contentElement.firstChildElement("relativeFilePath").text();
  m_fileUrls.clear();
  QDomNodeList urlElements = contentElement.elementsByTagName("fileUrl");
  for(int i = 0; i < urlElements.count(); i++) m_fileUrls.append(urlElements.at(i).toElement().text());

  // RETROCOMP <= 0.8
  if(m_fileUrls.isEmpty() && hasChildTags(contentElement, "path"))
    m_fileUrls.append(contentElement.firstChildElement("path").text());

  // A. load all non-relative urls until one works
  foreach(const QString & url, m_fileUrls)
  {
    // load Network image
    if(url.startsWith("http:", Qt::CaseInsensitive) || url.startsWith("https:", Qt::CaseInsensitive)
       || url.startsWith("ftp:", Qt::CaseInsensitive))
      return loadFromNetwork(url, 0);

    // load File
    if(loadFromFile(url, false, false, false)) return true;
  }

  // B. search from the relative path (used to always try this first)
  if(!m_relativePath.isEmpty() && baseDir.exists(m_relativePath))
  {
    QString cleanedFilePath = QDir::cleanPath(baseDir.filePath(m_relativePath));
    if(loadFromFile(cleanedFilePath, false, false, false))
    {
      m_fileUrls.append(cleanedFilePath);
      return true;
    }
  }

  // C. automatic file search from the relative name
  QStringList candidateNames;
  if(!m_relativePath.isEmpty())
  {
    candidateNames.append(m_relativePath);
    const QString cleanedUp = QFileInfo(m_relativePath).fileName();
    if(!cleanedUp.isEmpty() && cleanedUp != m_relativePath) candidateNames.append(cleanedUp);
  }
  if(candidateNames.isEmpty() && !m_fileUrls.isEmpty()) candidateNames.append(m_fileUrls.first());

  if(!candidateNames.isEmpty())
  {
    qWarning("PictureContent::fromXml: file '%s' not found, scanning for replacements",
             qPrintable(candidateNames.first()));

    // find all replacements from the current basepath
    QStringList matches, insensitiveMatches;
    QDirIterator dIt(baseDir, QDirIterator::Subdirectories);
    while(dIt.hasNext())
    {
      dIt.next();
      foreach(const QString & match, candidateNames)
      {
        const QString fileName = dIt.fileName();
        const QString filePath = dIt.filePath();
        const QString absoluteFilePath = dIt.fileInfo().absoluteFilePath();
        if(fileName == match || filePath == match) matches.append(absoluteFilePath);
        if(fileName.compare(match, Qt::CaseInsensitive) == 0) insensitiveMatches.append(absoluteFilePath);
      }
    }

    // enqueue the insensitive to the precise matches
    matches.append(insensitiveMatches);

    // try to load each replacement, until we find one
    foreach(const QString & absoluteFilePath, matches)
    {
      if(loadFromFile(absoluteFilePath, false, false, false))
      {
        m_fileUrls.append(absoluteFilePath);
        qWarning("PictureContent::fromXml:    found and using '%s'", qPrintable(absoluteFilePath));
        return true;
      }
      qWarning("PictureContent::fromXml:    found, but couldn't load '%s'", qPrintable(absoluteFilePath));
    }
  }

  // D. offer a manual selection dialog
  QMessageBox::warning(
      0, tr("Missing image file"),
      tr("I looked everywhere but I could not find '%1'. Please select a replacement.").arg(m_relativePath));

  const QString defaultLoadPath = App::settings->value("Fotowall/LoadImagesDir").toString();
  const QString replacementFilePath =
      QFileDialog::getOpenFileName(0, tr("Select replacement for %1").arg(m_relativePath), defaultLoadPath,
                                   tr("Images (%1)").arg(App::supportedImageFormats()));
  if(replacementFilePath.isEmpty()) return false;
  App::settings->setValue("Fotowall/LoadImagesDir", QFileInfo(replacementFilePath).absolutePath());

  if(loadFromFile(replacementFilePath, false, false, false))
  {
    m_fileUrls.append(replacementFilePath);
    qWarning("PictureContent::fromXml:    replaced and using '%s'", qPrintable(replacementFilePath));
    return true;
  }

  // give up
  QMessageBox::information(0, tr("Skipping file"),
                           tr("We will remove the image '%1' from the Canvas.").arg(m_relativePath));
  return false;
}

void PictureContent::toXml(QDomElement & contentElement, const QDir & baseDir) const
{
  // save AbstractContent properties and rename to 'picture'
  AbstractContent::toXml(contentElement, baseDir);
  contentElement.setTagName("picture");

  // save picture properties
  QDomDocument doc = contentElement.ownerDocument();
  QDomElement domElement;

  // save image urls (whether are local paths or remote urls)
  bool firstRelative = true;
  foreach(const QString & fileUrl, m_fileUrls)
  {

    // if file, save relative path (mandatory for relocating files)
    if(firstRelative && !fileUrl.startsWith("http:", Qt::CaseInsensitive)
       && !fileUrl.startsWith("https:", Qt::CaseInsensitive) && !fileUrl.startsWith("ftp:", Qt::CaseInsensitive))
    {
      firstRelative = false;
      QString relativePath = m_relativePath;
      if(relativePath.isEmpty()) relativePath = baseDir.relativeFilePath(fileUrl);
      if(!relativePath.isEmpty())
      {
        domElement = doc.createElement("relativeFilePath");
        contentElement.appendChild(domElement);
        domElement.appendChild(doc.createTextNode(relativePath));
      }
    }

    // save the url (mostly useful for non-local files)
    domElement = doc.createElement("fileUrl");
    contentElement.appendChild(domElement);
    domElement.appendChild(doc.createTextNode(fileUrl));
  }

  // save the effects
  domElement = doc.createElement("effects");
  contentElement.appendChild(domElement);
  QList<PictureEffect> effectsList = m_afterLoadEffects;
  if(m_photo) effectsList.append(m_photo->effects());
  foreach(const PictureEffect & effect, effectsList)
  {
    QDomElement effectElement = doc.createElement("effect");
    effectElement.setAttribute("type", effect.effect);
    effectElement.setAttribute("param", effect.param);
    if(effect.effect == PictureEffect::Crop)
    {
      QString croppingRectStr;
      croppingRectStr = QString::number(effect.rect.x()) + " " + QString::number(effect.rect.y()) + " "
                        + QString::number(effect.rect.width()) + " " + QString::number(effect.rect.height());

      effectElement.setAttribute("croppingRect", croppingRectStr);
    }
    domElement.appendChild(effectElement);
  }
}

void PictureContent::drawContent(QPainter * painter, const QRect & targetRect, Qt::AspectRatioMode ratio)
{
  // draw progress
  if(m_progress > 0.0 && m_progress < 1.0 && !RenderOpts::HQRendering)
  {
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::blue);
    painter->drawPie(QRect(targetRect.center(), QSize(20, 20)), 90 * 16, (int)(-5760.0 * m_progress));
  }

  // skip if no photo
  if(!m_photo) return;

    // blit if opaque picture
#if QT_VERSION >= 0x040600
    // disabled for 4.6 too, since it relies on raster.
    // if (m_opaquePhoto)
    //     painter->setCompositionMode(QPainter::CompositionMode_Source);
#endif

  // draw high-resolution photo when exporting png
  if(RenderOpts::HQRendering || ratio != Qt::IgnoreAspectRatio)
  {
    QSize scaledSize = m_photo->size();
    scaledSize.scale(targetRect.size(), ratio);
    int offX = targetRect.left() + (targetRect.width() - scaledSize.width()) / 2;
    int offY = targetRect.top() + (targetRect.height() - scaledSize.height()) / 2;
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true);
    painter->drawPixmap(offX, offY, scaledSize.width(), scaledSize.height(), *m_photo);
    return;
  }

  // draw photo using caching and deferred rescales
  if(beingTransformed())
  {
    if(!m_cachedPhoto.isNull()) painter->drawPixmap(targetRect, m_cachedPhoto);
  }
  else
  {
    if(m_cachedPhoto.isNull() || m_cachedPhoto.size() != targetRect.size())
      m_cachedPhoto = m_photo->scaled(targetRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter->drawPixmap(targetRect.topLeft(), m_cachedPhoto);
  }

#if QT_VERSION >= 0x040600
//    if (m_opaquePhoto)
//        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
#endif
}

QPixmap PictureContent::toPixmap(const QSize & size, Qt::AspectRatioMode ratio)
{
  if(m_photo) return ratioScaledPixmap(m_photo, size, ratio);
  return AbstractContent::toPixmap(size, ratio);
}

int PictureContent::contentHeightForWidth(int width) const
{
  if(m_netWidth > 0 && m_netHeight > 0) return (m_netHeight * width) / m_netWidth;
  if(!m_photo || m_photo->width() < 1) return -1;
  return (m_photo->height() * width) / m_photo->width();
}

bool PictureContent::contentOpaque() const
{
  return m_opaquePhoto;
}

void PictureContent::dropEvent(QGraphicsSceneDragDropEvent * event)
{
  // load the first valid picture
  foreach(const QUrl & url, event->mimeData()->urls())
  {
    // handle network drops
    if(url.scheme() == "http" || url.scheme() == "https" || url.scheme() == "ftp")
    {
      QStringList prevUrls = m_fileUrls;
      m_fileUrls.clear();
      if(loadFromNetwork(url.toString(), 0))
      {
        event->accept();
        return;
      }
      m_fileUrls = prevUrls;
    }
    // handle local drops
    QString picFilePath = url.toString();
    if(QFile::exists(picFilePath))
    {
      if(loadFromFile(picFilePath, true, true, true))
      {
        event->accept();
        return;
      }
    }
  }
}

void PictureContent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
  AbstractContent::mouseDoubleClickEvent(event);
  emit requestBackgrounding();
}

void PictureContent::setExternalEdit(bool enabled)
{
  if(!m_photo) return;

  // start gimp if requested
  if(enabled && !m_watcher)
  {
    // save the pic to a file
    QString tmpFile =
        QDir::tempPath() + QDir::separator() + "TEMP" + QString::number(compat::qrand() % 999999) + ".png";
    if(!m_photo->save(tmpFile, "PNG"))
    {
      qWarning("PictureContent::slotGimpEdit: can't save the image");
      return;
    }

#if !defined(__EMSCRIPTEN__)
    // open it with the gimp
#  if defined(Q_OS_WIN) || defined(Q_OS_OS2)
    QString executable = "gimp.exe";
#  else
    QString executable = "gimp";
#  endif
    if(!QProcess::startDetached(executable, QStringList() << tmpFile))
    {
      qWarning("PictureContent::slotGimpEdit: can't start The Gimp");
      return;
    }
#endif

    // start a watcher over it
    delete m_watcher;
    m_watcher = new QFileSystemWatcher(this);
    m_watcher->setProperty("fullName", tmpFile);
    m_watcher->addPath(tmpFile);
    connect(m_watcher, SIGNAL(fileChanged(const QString &)), this, SLOT(slotGimpCompressNotifies()));
    return;
  }

  // delete if requested
  if(m_watcher && !enabled)
  {
    delete m_watcherTimer;
    m_watcherTimer = 0;
    delete m_watcher;
    m_watcher = 0;
  }
}

bool PictureContent::externalEdit() const
{
  return m_watcher;
}

void PictureContent::dropNetworkConnection()
{
  if(m_netReply)
  {
    m_netReply->disconnect(0, 0, 0);
#if QT_VERSION >= 0x040600
    if(!m_netReply->isFinished())
#endif
      m_netReply->abort();
    m_netReply->deleteLater();
    m_netReply = 0;
  }
  m_progress = 0.0;
}

void PictureContent::applyPostLoadEffects()
{
  foreach(const PictureEffect & effect, m_afterLoadEffects) m_photo->addEffect(effect);
  m_afterLoadEffects.clear();
  update();
  GFX_CHANGED();
}

void PictureContent::slotRotate()
{
  do_canvas_command(scene(), new EffectCommand(this, PictureEffect::Rotate));
  update();
}

void PictureContent::slotGimpCompressNotifies()
{
  if(!m_watcherTimer)
  {
    m_watcherTimer = new QTimer(this);
    m_watcherTimer->setSingleShot(true);
    connect(m_watcherTimer, SIGNAL(timeout()), this, SLOT(slotGimpFinished()));
  }
  m_watcherTimer->start(500);
}

void PictureContent::slotGimpFinished()
{
  // get the file name and dispose the watcher
  if(!m_watcher) return;
  QString gimpFilePath = m_watcher->property("fullName").toString();

  // reload the file
  CPixmap * newPhoto = new CPixmap(gimpFilePath);
  if(newPhoto->isNull())
  {
    qWarning("PictureContent::slotGimpFinished: can't load the modified picture");
    delete newPhoto;
    return;
  }
  delete m_photo;
  m_photo = newPhoto;
  m_cachedPhoto = QPixmap();
  m_opaquePhoto = !m_photo->hasAlpha();
  // UNBREAK URL (pic will disappear!) copy locally
  qWarning("PictureContent::slotGimpFinished: saving the picture won't work. fix it.");
  m_fileUrls = QStringList();
  update();

  // notify image change
  emit contentChanged();
}

bool PictureContent::slotLoadNetworkData()
{
  // get the data
  if(!m_netReply) return false;
  QByteArray replyData = m_netReply->readAll();
  dropNetworkConnection();

  // make the QImage from data
  QImage image = QImage::fromData(replyData);
  if(image.isNull()) return false;

  // update contents
  m_netWidth = image.width();
  m_netHeight = image.height();
  resetContentsRatio();
  m_progress = 1.0;
  m_photo = new CPixmap(image);
  m_opaquePhoto = !m_photo->hasAlpha();
  applyPostLoadEffects();
  setAcceptHoverEvents(true);

  // notify image change
  emit contentChanged();
  return true;
}

void PictureContent::slotNetworkError()
{
  // clear state
  if(!m_netReply) return;
  dropNetworkConnection();
  m_progress = 0.0;
  setAcceptHoverEvents(true);
}

void PictureContent::slotNetworkProgress(qint64 a, qint64 b)
{
  m_progress = b > 0 ? (double)a / (double)b : 0.0;
  update();
}
