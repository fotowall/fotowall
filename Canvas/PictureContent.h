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

#ifndef __PictureContent_h__
#define __PictureContent_h__

#include "AbstractContent.h"
#include "Shared/PictureEffect.h"
class CPixmap;
class QFileSystemWatcher;
class QNetworkReply;
class QByteArray;

/**
    \brief Transformable picture, with lots of gadgets
*/
class PictureContent : public AbstractContent
{
  Q_OBJECT
  Q_PROPERTY(bool externalEdit READ externalEdit WRITE setExternalEdit)
public:
  PictureContent(bool spontaneous, QGraphicsScene * scene, QGraphicsItem * parent = 0);
  ~PictureContent();

  bool loadFromFile(const QString & picFilePath, bool setUrl, bool setRatio, bool setName);
  bool loadFromImage(const QString & picFilePath, const QImage & img, bool setUrl, bool setRatio, bool setName);
  bool loadFromNetwork(const QString & url,
                       QNetworkReply * reply = 0,
                       const QString & title = QString(),
                       int width = -1,
                       int height = -1);
  bool loadPixmapForAccelTest(const QPixmap & pixmap, const QString & title = QString());
  void addEffect(const PictureEffect & effect);
  QList<PictureEffect> effects() const;
  void crop();

  // ::AbstractContent
  QString contentName() const { return tr("Picture"); }
  QWidget * createPropertyWidget(ContentProperties * p = 0);
  bool fromXml(const QDomElement & contentElement, const QDir & baseDir);
  void toXml(QDomElement & contentElement, const QDir & baseDir) const;
  void drawContent(QPainter * painter, const QRect & targetRect, Qt::AspectRatioMode ratio);
  QPixmap toPixmap(const QSize & size, Qt::AspectRatioMode ratio);
  int contentHeightForWidth(int width) const;
  bool contentOpaque() const;

  // ::QGraphicsItem
  void dropEvent(QGraphicsSceneDragDropEvent * event);
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);

Q_SIGNALS:
  void flipHorizontally();
  void flipVertically();
  void requestCrop();

protected:
  // properties
  void setExternalEdit(bool);
  bool externalEdit() const;

private:
  void dropNetworkConnection();
  void applyPostLoadEffects();
  QString m_relativePath;
  QStringList m_fileUrls;
  CPixmap * m_photo;
  QPixmap m_cachedPhoto;
  bool m_opaquePhoto;
  double m_progress;
  int m_netWidth;
  int m_netHeight;
  QNetworkReply * m_netReply;
  QList<PictureEffect> m_afterLoadEffects;
  QFileSystemWatcher * m_watcher;
  QTimer * m_watcherTimer;

private Q_SLOTS:
  void slotGimpCompressNotifies();
  void slotGimpFinished();
  bool slotLoadNetworkData();
  void slotNetworkError();
  void slotNetworkProgress(qint64, qint64);
  void slotRotate();
};

#endif
