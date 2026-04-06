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

#include "CornerItem.h"

#include "AbstractContent.h"
#include "Shared/RenderOpts.h"

#include "Shared/Commands.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <math.h>

CornerItem::CornerItem(Qt::Corner corner, bool rotateOnly, AbstractContent * content)
: QGraphicsItem(content), m_content(content), m_corner(corner), m_opMask(rotateOnly ? Rotate | FixRotate : AllowAll),
  m_side(8), m_operation(Off), m_startRotation(content->rotation()), m_hidden(false)
{ setAcceptHoverEvents(true); }

void CornerItem::relayout(const QRect & rect)
{
  // change side, if needed
  int side = 1 + (int)sqrt((float)qMin(rect.width(), rect.height()));
  if(side != m_side)
  {
    prepareGeometryChange();
    m_side = side;
  }

  // place at the right corner
  switch(m_corner)
  {
    case Qt::TopLeftCorner:
      setPos(rect.topLeft() + QPoint(m_side, m_side));
      break;
    case Qt::TopRightCorner:
      setPos(rect.topRight() + QPoint(-m_side + 1, m_side));
      break;
    case Qt::BottomLeftCorner:
      setPos(rect.bottomLeft() + QPoint(m_side, -m_side + 1));
      break;
    case Qt::BottomRightCorner:
      setPos(rect.bottomRight() + QPoint(-m_side + 1, -m_side + 1));
      break;
  }
}

QRectF CornerItem::boundingRect() const
{ return QRectF(-m_side, -m_side, 2 * m_side, 2 * m_side); }

void CornerItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
  event->accept();

  // do the right op
  switch(event->button())
  {
    case Qt::LeftButton:
      if(m_opMask & Scale) m_content->resetContentsRatio();
      break;
    case Qt::RightButton:
      if(m_opMask & Rotate) m_content->setRotation(0);
      break;
    default:
      break;
  }
}

void CornerItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
  event->accept();

  // do the right op
  switch(event->button())
  {
    case Qt::LeftButton:
      m_operation = Rotate | Scale | FixScale;
      break;
    case Qt::RightButton:
      m_operation = Scale | FixScale;
      break;
    case Qt::MiddleButton:
      m_operation = Scale;
      break;
    default:
      m_operation = Off;
      return;
  }

  // filter out unwanted operations
  m_operation &= m_opMask;

  // intial parameters
  m_startContentRect = m_content->contentRect();
  m_startRatio = (double)m_startContentRect.width() / (double)m_startContentRect.height();

  m_startRotation = m_content->rotation();

  update();
}

void CornerItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
  if(m_operation == Off) return;
  event->accept();

  // modify the operation using the shortcuts
  int op = m_operation;
  if(event->modifiers() & Qt::ShiftModifier) op &= ~FixScale & ~Rotate;
  if(event->modifiers() & Qt::ControlModifier)
    // op |= FixRotate;
    op |= Rotate;
  if(event->modifiers() & Qt::AltModifier)
    // op &= ~(Scale | FixScale);
    op |= FixRotate;
  op &= m_opMask;
  if((op & (Rotate | Scale)) == (Rotate | Scale)) op |= FixScale;
  if((op & (Rotate | Scale)) == Off) return;

  // vector relative to the centre
  QPointF v = pos() + event->pos();
  if(v.isNull()) return;

  // do scaling
  m_content->delayedDirty();
  if(op & Scale)
  {
    if(op & FixScale)
    {
      const double r = m_startRatio;
      const double D = sqrt(v.x() * v.x() + v.y() * v.y());
      const double K = sqrt(1 + 1 / (r * r));
      int W = qMax((int)((2 * D) / (K)), 50);
      int H = qMax((int)((2 * D) / (r * K)), 40);
      m_content->resizeContents(QRect(-W / 2, -H / 2, W, H));
    }
    else
    {
      int W = qMax(2 * fabs(v.x()), 50.0); //(m_contentRect.width() * v.x()) / oldPos.x();
      int H = qMax(2 * fabs(v.y()), 40.0); //(m_contentRect.height() * v.y()) / oldPos.y();
                                           // if (W != (int)cRect.width() || H != (int)cRect.height())
      m_content->resizeContents(QRect(-W / 2, -H / 2, W, H));
    }
  }

  // do rotation
  if(op & Rotate)
  {
    QPointF refPos = pos();

    // set item rotation (set rotation relative to current)
    qreal refAngle = atan2(refPos.y(), refPos.x());
    qreal newAngle = atan2(v.y(), v.x());
    double dZr = 57.29577951308232 * (newAngle - refAngle); // 180 * a / M_PI
    double zr = m_content->rotation() + dZr;

    // snap to M_PI/4
    if(op & FixRotate)
    {
      int fracts = (int)((zr - 7.5) / 15.0);
      zr = (qreal)fracts * 15.0;
    }

    // apply rotation
    m_content->setRotation(zr);
  }
}

void CornerItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
  event->accept();
  bool accepted = m_operation != Off;
  m_operation = Off;
  update();

  RotateAndResizeCommand * command = new RotateAndResizeCommand(m_content, m_startRotation, m_content->rotation(),
                                                                m_startContentRect, m_content->contentRect());
  do_canvas_command(m_content->scene(), command);

  // clicked
  if(accepted) {}
}

void CornerItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * /*widget*/)
{
  if(RenderOpts::HQRendering) return;

  QColor color = RenderOpts::hiColor;
  if(option->state & QStyle::State_MouseOver)
  {
    if(m_operation != Off)
      color.setAlpha(250);
    else
      color.setAlpha(196);
  }
  else
    color.setAlpha(128);
  painter->fillRect(boundingRect(), color);
}
