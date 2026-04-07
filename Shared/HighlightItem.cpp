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

#include "HighlightItem.h"

#include "Shared/RenderOpts.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QTimerEvent>
#include <math.h>

HighlightItem::HighlightItem(QGraphicsItem * parent)
: QGraphicsItem(parent), m_unset(true), m_xn(0), m_yn(0), m_phase(0), m_radius(1), m_closing(false)
{ m_timer.start(30, this); }

void HighlightItem::setPos(double x, double y)
{
  QRectF pr = parentRect();
  m_xn = (x - pr.left()) / (double)pr.width();
  m_yn = (y - pr.top()) / (double)pr.height();
  reposition();
}

void HighlightItem::setPosF(double xn, double yn)
{
  m_xn = xn;
  m_yn = yn;
  reposition();
}

void HighlightItem::reposition(const QRectF & rect)
{
  QRectF pr = rect.isNull() ? parentRect() : rect;
  QGraphicsItem::setPos((int)(pr.left() + m_xn * pr.width()), (int)(pr.top() + m_yn * pr.height()));
}

void HighlightItem::deleteAfterAnimation()
{ m_closing = true; }

QRectF HighlightItem::boundingRect() const
{ return QRectF(-m_radius, -m_radius, 2 * m_radius, 2 * m_radius); }

void HighlightItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
  if(RenderOpts::HQRendering) return;
  int alpha = qBound(0, (100 - (int)m_radius) * 3, 255);
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setPen(QPen(QColor(255, 255, 255, alpha), 3));
  painter->drawEllipse(boundingRect().adjusted(1, 1, -1, -1));
}

void HighlightItem::timerEvent(QTimerEvent * event)
{
  if(event->timerId() != m_timer.timerId()) return QObject::timerEvent(event);

  // advance phase
  m_phase++;

  // resize
  prepareGeometryChange();
  m_radius = sqrt((double)(m_phase * 200));
  update();

  // wrap around @50 {rad=100}
  if(m_phase > 50)
  {
    m_phase = 0;
    if(m_closing)
    {
      m_timer.stop();
      deleteLater();
    }
  }
}

QRectF HighlightItem::parentRect() const
{
  if(parentItem())
    return parentItem()->boundingRect();
  else if(scene())
    return scene()->sceneRect();
  return QRectF(0, 0, 10, 10);
}
