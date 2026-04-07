/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2009 by TANGUY Arnaud <arn.tanguy@gmail.com>            *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "CroppingWidget.h"
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#if QT_VERSION >= 0x040600
#  include <QPropertyAnimation>
#endif

void MaskItem::resizeToScene()
{
  if(scene()) setGeometry(scene()->sceneRect());
}

void MaskItem::setHole(const QRect & rect)
{
  m_hole = rect;
  update();
}

MaskItem::MaskItem() : m_strength(0)
{
#if QT_VERSION >= 0x040600
  // appear
  QPropertyAnimation * as = new QPropertyAnimation(this, "strength");
  as->setEasingCurve(QEasingCurve::OutQuad);
  as->setDuration(1000);
  as->setStartValue(0.01);
  as->setEndValue(1.0);
  as->start(QPropertyAnimation::DeleteWhenStopped);
#else
  m_strength = 1.0;
#endif
}

void MaskItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
  // fill each rect of the tassellated subtracted region
  foreach(const QRect & rect, QRegion(geometry().toRect()).subtracted(m_hole))
    painter->fillRect(rect, QColor(0, 0, 0, (int)(128.0 * m_strength)));
}

qreal MaskItem::strength() const
{ return m_strength; }

void MaskItem::setStrength(qreal strength)
{
  if(strength != m_strength)
  {
    m_strength = strength;
    update();
  }
}

#define HandleSize 12

RubberBandItem::RubberBandItem() : QGraphicsWidget(0), m_handle(-1) {}

void RubberBandItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
  int idx = 0;
  foreach(const QRect & rect, m_corners)
  {
    if(rect.contains(event->pos().toPoint())) break;
    idx++;
  }
  m_handle = idx < 8 ? idx : -1;
  m_displace = event->scenePos().toPoint() - geometry().toRect().center();
  mouseMoveEvent(event);
}

void RubberBandItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
  QRect newGeom = geometry().toRect();
  QRect sceneRect = scene()->sceneRect().toRect();

  // move center if dragging
  if(m_handle == -1)
  {
    QPoint pt = event->scenePos().toPoint() - m_displace;
    if(pt.x() < 2 * HandleSize)
      pt.setX(2 * HandleSize);
    else if(pt.x() >= sceneRect.right() - 2 * HandleSize)
      pt.setX(sceneRect.right() - 2 * HandleSize - 1);
    if(pt.y() < 2 * HandleSize)
      pt.setY(2 * HandleSize);
    else if(pt.y() >= sceneRect.bottom() - 2 * HandleSize)
      pt.setY(sceneRect.bottom() - 2 * HandleSize - 1);
    newGeom.translate(pt - newGeom.center());
  }

  // or resize if moving a handle
  else
  {
    bool left = false, top = false, right = false, bottom = false;
    switch(m_handle)
    {
      case 0:
        left = true;
        top = true;
        break;
      case 1:
        top = true;
        break;
      case 2:
        right = true;
        top = true;
        break;
      case 3:
        right = true;
        break;
      case 4:
        right = true;
        bottom = true;
        break;
      case 5:
        bottom = true;
        break;
      case 6:
        left = true;
        bottom = true;
        break;
      case 7:
        left = true;
        break;
    }
    if(left)
    {
      newGeom.setLeft(event->scenePos().x());
      if(newGeom.width() < (2 * HandleSize)) newGeom.setLeft(newGeom.right() - 2 * HandleSize);
    }
    if(top)
    {
      newGeom.setTop(event->scenePos().y());
      if(newGeom.height() < (2 * HandleSize)) newGeom.setTop(newGeom.bottom() - 2 * HandleSize);
    }
    if(right)
    {
      newGeom.setRight(event->scenePos().x());
      if(newGeom.width() < (2 * HandleSize)) newGeom.setRight(newGeom.left() + 2 * HandleSize);
    }
    if(bottom)
    {
      newGeom.setBottom(event->scenePos().y());
      if(newGeom.height() < (2 * HandleSize)) newGeom.setBottom(newGeom.top() + 2 * HandleSize);
    }
  }

  // keep in bounds
  if(newGeom.left() < sceneRect.left()) newGeom.setLeft(sceneRect.left());
  if(newGeom.top() < sceneRect.top()) newGeom.setTop(sceneRect.top());
  if(newGeom.right() >= sceneRect.right()) newGeom.setRight(sceneRect.right() - 1);
  if(newGeom.bottom() >= sceneRect.bottom()) newGeom.setBottom(sceneRect.bottom() - 1);
  setGeometry(newGeom);

  // change mask too
  emit geometryChanged(newGeom);
}

void RubberBandItem::resizeEvent(QGraphicsSceneResizeEvent * /*event*/)
{
  // place corners
  m_corners.clear();
  m_corners.append(QRect(0, 0, HandleSize, HandleSize));
  m_corners.append(QRect((size().width() - HandleSize) / 2, 0, HandleSize, HandleSize));
  m_corners.append(QRect(size().width() - HandleSize, 0, HandleSize, HandleSize));
  m_corners.append(QRect(size().width() - HandleSize, (size().height() - HandleSize) / 2, HandleSize, HandleSize));
  m_corners.append(QRect(size().width() - HandleSize, size().height() - HandleSize, HandleSize, HandleSize));
  m_corners.append(QRect((size().width() - HandleSize) / 2, size().height() - HandleSize, HandleSize, HandleSize));
  m_corners.append(QRect(0, size().height() - HandleSize, HandleSize, HandleSize));
  m_corners.append(QRect(0, (size().height() - HandleSize) / 2, HandleSize, HandleSize));
}

void RubberBandItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
  QColor col = palette().brush(QPalette::Highlight).color();
  QRect fullRect = boundingRect().toRect();
  painter->setBrush(Qt::NoBrush);
  painter->setPen(col);
  painter->drawRect(fullRect);
  col.setAlpha(64);
  painter->setBrush(col);
  foreach(const QRect & rect, m_corners) painter->drawRect(rect);
  painter->drawPoint(fullRect.center());
  painter->drawPoint(fullRect.center() + QPoint(1, 0));
  painter->drawPoint(fullRect.center() + QPoint(-1, 0));
  painter->drawPoint(fullRect.center() + QPoint(0, 1));
  painter->drawPoint(fullRect.center() + QPoint(0, -1));
}

CroppingWidget::CroppingWidget(QWidget * parent)
: QGraphicsView(parent), m_scene(0), m_maskItem(0), m_rubberBand(0), m_previewScale(1)
{
  m_scene = new QGraphicsScene(this);
  setScene(m_scene);
  setFrameStyle(QFrame::NoFrame);

  m_maskItem = new MaskItem;
  m_maskItem->setZValue(1);
  m_scene->addItem(m_maskItem);
  connect(m_scene, SIGNAL(sceneRectChanged(QRectF)), m_maskItem, SLOT(resizeToScene()));

  m_rubberBand = new RubberBandItem;
  m_rubberBand->setZValue(2);
  m_scene->addItem(m_rubberBand);
  connect(m_rubberBand, SIGNAL(geometryChanged(QRect)), m_maskItem, SLOT(setHole(QRect)));
}

void CroppingWidget::setPixmap(QPixmap * pix)
{
  m_previewPixmap = pix->scaled(500, 500, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  m_previewScale = (float)pix->width() / (float)m_previewPixmap.width();
  setFixedSize(m_previewPixmap.size());
  int hw = m_previewPixmap.width() / 2;
  int hh = m_previewPixmap.height() / 2;
  m_rubberBand->resize(hw, hh);
  m_rubberBand->setPos(hw / 2, hh / 2);
  m_maskItem->setHole(m_rubberBand->geometry().toRect());
}

QRect CroppingWidget::getCroppingRect() const
{
  QRectF cropRect = m_rubberBand->geometry();
  return QRect(cropRect.x() * m_previewScale, cropRect.y() * m_previewScale, cropRect.width() * m_previewScale,
               cropRect.height() * m_previewScale);
}

void CroppingWidget::drawBackground(QPainter * painter, const QRectF & /*rect*/)
{
  // blit background pixmap
  painter->setCompositionMode(QPainter::CompositionMode_Source);
  painter->drawPixmap(sceneRect().toAlignedRect(), m_previewPixmap);
  painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
}

void CroppingWidget::resizeEvent(QResizeEvent * /*event*/)
{
  QRect sceneRect(0, 0, width(), height());
  m_scene->setSceneRect(sceneRect);
}
