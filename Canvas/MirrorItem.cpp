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

#include "MirrorItem.h"

#include "Shared/RenderOpts.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <math.h>

#define MIRROR_HEIGHT 100

MirrorItem::MirrorItem(QGraphicsItem * sourceItem, QGraphicsItem * parent)
: AbstractDisposeable(false, parent), m_source(sourceItem), m_dirty(false)
{
  // read current values
  sourceChanged();
  setVisible(m_source->isVisible());
  setZValue(m_source->zValue());

  // add to the scame scene
  Q_ASSERT(m_source->scene());
  m_source->scene()->addItem(this);
}

MirrorItem::~MirrorItem() {}
/*
void MirrorItem::dispose()
{
    deleteLater();
}
*/
QRectF MirrorItem::boundingRect() const
{ return m_boundingRect; }

void MirrorItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * /*widget*/)
{
  // TODO: real HQ rendering, skipping the pixmap (better result)!

  // generate Reflection pixmap, if needed
  if(m_dirty || m_pixmap.isNull() || RenderOpts::HQRendering)
  {

    // change pixmap size to match the bounding rect
    if(m_pixmap.size() != m_boundingRect.size().toSize())
      m_pixmap = QPixmap(m_boundingRect.width(), m_boundingRect.height());

    // clear pixmap
    m_pixmap.fill(Qt::transparent);

    // find out the Transform chain to mirror a rotated item
    QRectF sceneRectF = m_source->mapToScene(m_source->boundingRect()).boundingRect();
    QTransform tFromItem = m_source->transform() * QTransform(1, 0, 0, 1, m_source->pos().x(), m_source->pos().y());
    QTransform tFromPixmap = QTransform(1, 0, 0, -1.0, sceneRectF.left(), sceneRectF.bottom());
    QTransform tItemToPixmap = tFromItem * tFromPixmap.inverted();

    // draw the transformed item onto the pixmap
    QPainter p(&m_pixmap);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setTransform(tItemToPixmap, true);
    m_source->paint(&p, 0, 0);
    p.end();

    // add a linear alpha channel to the image
    QPixmap alphaPixmap(m_pixmap.size());
    QPainter alphaPainter(&alphaPixmap);
    QLinearGradient alphaGradient(0, 0, 0, alphaPixmap.height());
    alphaGradient.setColorAt(0.0, QColor(128, 128, 128));
    alphaGradient.setColorAt(0.50, QColor(32, 32, 32));
    alphaGradient.setColorAt(1.0, QColor(0, 0, 0));
    alphaPainter.fillRect(alphaPixmap.rect(), alphaGradient);
    alphaPainter.end();

    // set the Alpha channel, using a deprecated path
    // (pre-Qt5 was: m_pixmap.setAlphaChannel(alphaPixmap))
    QImage pixmapImage = m_pixmap.toImage();
    pixmapImage.setAlphaChannel(alphaPixmap.toImage());
    m_pixmap = QPixmap::fromImage(pixmapImage);

    // reset dirty
    m_dirty = false;
  }

  // draw the reflection pixmap
  if(!option)
    painter->drawPixmap(0, 0, m_pixmap);
  else
    painter->drawPixmap(option->rect, m_pixmap, option->rect);
}

void MirrorItem::sourceMoved()
{
  // find out the item's polygon in scene coordinates
  QRectF itemRect = m_source->boundingRect();
  QPolygonF itemScenePolygon = m_source->mapToScene(itemRect);
  QRect itemSceneRect = itemScenePolygon.boundingRect().toRect();

  // reposition
  setPos(itemSceneRect.bottomLeft());
}

void MirrorItem::sourceChanged()
{
  // find out the item's polygon in scene coordinates
  QRectF itemRect = m_source->boundingRect();
  QPolygonF itemScenePolygon = m_source->mapToScene(itemRect);
  QRect itemSceneRect = itemScenePolygon.boundingRect().toRect();

  // reposition
  setPos(itemSceneRect.bottomLeft());

  // find out the new bounding rect
  QRectF newBr(0.0, 0.0, itemSceneRect.width(), qMin(itemSceneRect.height(), MIRROR_HEIGHT));

  // if the bounding rect changed, resize
  if(newBr != m_boundingRect)
  {
    prepareGeometryChange();
    m_boundingRect = newBr;
  }

  // invalidate current rendering
  m_dirty = true;
  update();
}
