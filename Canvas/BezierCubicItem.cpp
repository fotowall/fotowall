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

#include "BezierCubicItem.h"
#include "Shared/Compat.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QLineF>
#include <QPainter>
#include <QPainterPath>
#include <QPen>

class BezierControlPoint : public QGraphicsItem
{
public:
  BezierControlPoint(BezierCubicItem *, int index);

  // ::QGraphicsItem
  QRectF boundingRect() const;
  void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
  void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

private:
  BezierCubicItem * m_parent;
  int m_index;
};

class BezierControlLine : public QGraphicsLineItem
{
public:
  BezierControlLine(BezierCubicItem *);
};

class BezierCubicPath : public QGraphicsPathItem
{
public:
  BezierCubicPath(BezierCubicItem *);
};

BezierCubicItem::BezierCubicItem(QGraphicsItem * parent) : QGraphicsItem(parent)
{
  // children items (4 controlpoints, 2 dashed lines, 1 strong line)
  m_path = new BezierCubicPath(this);
  m_l1 = new BezierControlLine(this);
  m_l2 = new BezierControlLine(this);
  m_cps.resize(4);
  for(int i = 0; i < 4; i++)
  {
    m_cps[i] = new BezierControlPoint(this, i);
    m_cps[i]->setPos(-100 + compat::qrand() % 200, -70 + compat::qrand() % 140);
  }
  controlPointMoved(0);
  controlPointMoved(2);
}

QPainterPath BezierCubicItem::shape() const
{ return m_path->path(); }

void BezierCubicItem::setControlPoints(const QList<QPointF> & points)
{
  if(points.size() != 4) return;
  for(int i = 0; i < 4; i++) m_cps[i]->setPos(points[i]);
  controlPointMoved(1);
  controlPointMoved(3);
}

QList<QPointF> BezierCubicItem::controlPoints() const
{
  QList<QPointF> cp;
  for(int i = 0; i < 4; i++) cp.append(m_cps[i]->pos());
  return cp;
}

QRectF BezierCubicItem::boundingRect() const
{ return QRectF(-5, -5, 11, 11); }

void BezierCubicItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
  painter->setPen(QPen(Qt::red, 0.5));
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->drawLine(-5, 0, 5, 0);
  painter->drawLine(0, -5, 0, 5);
}

void BezierCubicItem::controlPointMoved(int index, bool moveFinished)
{
  // solid move
  if(index == 0 || index == 2)
  {
    QLineF l = index == 0 ? m_l1->line() : m_l2->line();
    if(!l.isNull()) m_cps[index + 1]->setPos(m_cps[index]->pos() + QPointF(l.dx(), l.dy()));
  }

  // update line
  if(index < 2)
    m_l1->setLine(QLineF(m_cps[0]->pos(), m_cps[1]->pos()));
  else
    m_l2->setLine(QLineF(m_cps[2]->pos(), m_cps[3]->pos()));

  // update path
  QPainterPath path(m_cps[0]->pos());
  path.cubicTo(m_cps[1]->pos(), m_cps[3]->pos(), m_cps[2]->pos());
  m_path->setPath(path);
  emit shapeChanged(path);

  if(moveFinished)
  {
    QList<QPointF> cps;
    foreach(BezierControlPoint * pt, m_cps) { cps.append(pt->pos()); }
    emit shapeControlPointChanged(cps);
  }
}

/** BezierControlPoint **/

BezierControlPoint::BezierControlPoint(BezierCubicItem * parent, int index)
: QGraphicsItem(parent), m_parent(parent), m_index(index)
{
  setFlag(QGraphicsItem::ItemIsMovable, true);
  setZValue(2.1);
}

QRectF BezierControlPoint::boundingRect() const
{ return QRectF(-7, -7, 15, 15); }

void BezierControlPoint::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
  if(event->buttons() == Qt::LeftButton)
  {
    setPos(mapToParent(event->pos()));
    m_parent->controlPointMoved(m_index);
  }
}

void BezierControlPoint::mouseReleaseEvent(QGraphicsSceneMouseEvent * /* event */)
{ m_parent->controlPointMoved(m_index, true); }

void BezierControlPoint::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
  painter->setPen(QColor(50, 100, 120, 200));
  painter->setBrush((m_index & 0x01) ? QColor(200, 200, 210, 120) : QColor(210, 100, 110, 120));
  painter->drawEllipse(boundingRect());
}

/** BezierControlLine **/

BezierControlLine::BezierControlLine(BezierCubicItem * parent) : QGraphicsLineItem(parent)
{
  QPen pen(QColor(50, 100, 120, 200), 1);
  QVector<qreal> dashes;
  dashes << 8 << 6;
  pen.setDashPattern(dashes);
  setPen(pen);
  setZValue(1.9);
}

/** BezierCubicPath **/

BezierCubicPath::BezierCubicPath(BezierCubicItem * parent) : QGraphicsPathItem(parent)
{
  QPen pen(QColor(200, 0, 0), 1);
  setPen(pen);
  setZValue(2.2);
}
