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

#include "StyledButtonItem.h"
#include "Shared/Compat.h"
#include <QFontMetrics>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QPainter>

StyledButtonItem::StyledButtonItem(const QString & text, const QFont & font, QGraphicsItem * parent)
: QGraphicsItem(parent), m_font(font), m_hovered(false), m_pressed(false)
{
  // init defaults
  // m_font.setBold(true);
  setText(text);
  setFlag(QGraphicsItem::ItemIsFocusable, true);

  // track mouse events
  setAcceptHoverEvents(true);
}

QString StyledButtonItem::text() const
{ return m_text; }

void StyledButtonItem::setText(const QString & text)
{
  if(text == m_text) return;
  m_text = text;

  QFontMetrics metrics(m_font);
  int width = metrics.horizontalAdvance(text) + 16 * 2;
  int height = metrics.height() + 1 * 2;
  m_rect = QRectF(0, 0, width, height);
}

QRectF StyledButtonItem::boundingRect() const
{ return m_rect; }

void StyledButtonItem::keyPressEvent(QKeyEvent * event)
{
  if(event->key() == Qt::Key_Return) emit clicked();
  QGraphicsItem::keyPressEvent(event);
}

void StyledButtonItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{ drawButton(m_pressed, m_hovered, painter); }

void StyledButtonItem::hoverEnterEvent(QGraphicsSceneHoverEvent * /*event*/)
{
  m_hovered = true;
  update();
}

void StyledButtonItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * /*event*/)
{
  m_hovered = false;
  update();
}

void StyledButtonItem::mousePressEvent(QGraphicsSceneMouseEvent * /*event*/)
{
  m_pressed = true;
  update();
}

void StyledButtonItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
  // check if going inside/outside
  bool inside = boundingRect().contains(event->pos());
  if(inside == m_pressed) return;
  m_pressed = inside;
  update();
}

void StyledButtonItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * /*event*/)
{
  if(!m_pressed) return;
  m_pressed = false;
  update();
  emit clicked();
}

void StyledButtonItem::drawButton(bool pressed, bool hovered, QPainter * painter)
{
  // frame
  QLinearGradient pg(0, 0, 0, 1);
  pg.setCoordinateMode(QGradient::ObjectBoundingMode);
  QLinearGradient bg(0, 0, 0, 1);
  bg.setCoordinateMode(QGradient::ObjectBoundingMode);
  if(pressed || hovered)
  {
    pg.setColorAt(0.00, QColor(249, 123, 123));
    pg.setColorAt(0.70, QColor(251, 167, 167));
    pg.setColorAt(1.00, QColor(231, 111, 111));
    bg.setColorAt(0.00, QColor(184, 0, 0, pressed ? 255 : 220));
    bg.setColorAt(0.55, QColor(250, 160, 160, pressed ? 255 : 220));
    bg.setColorAt(0.95, QColor(112, 0, 0, pressed ? 255 : 220));
    painter->setPen(QPen(pg, 2));
    painter->setBrush(bg);
    painter->drawRoundedRect(m_rect.adjusted(0.5, 0.5, -0.5, -0.5), 4, 4);
    painter->setPen(QPen(QColor(220, 0, 0), 1));
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(m_rect.adjusted(0.5, 0.5, -0.5, -0.5), 4, 4);
  }
  else
  {
    pg.setColorAt(0.00, QColor(226, 226, 226));
    pg.setColorAt(0.70, QColor(237, 237, 237));
    pg.setColorAt(1.00, QColor(164, 164, 164));
    bg.setColorAt(0.00, QColor(115, 115, 115, 128));
    bg.setColorAt(0.55, QColor(206, 206, 206, 128));
    bg.setColorAt(0.90, QColor(58, 58, 58, 128));
    painter->setPen(QPen(pg, 1));
    painter->setBrush(bg);
    painter->drawRoundedRect(m_rect.adjusted(0.5, 0.5, -0.5, -0.5), 4, 4);
  }

  // text
  if(pressed)
  {
    QFont font = m_font;
    font.setBold(true);
    painter->setFont(font);
  }
  else
    painter->setFont(m_font);
  painter->setPen(hovered ? Qt::white : Qt::black);
  painter->drawText(m_rect, Qt::AlignCenter, m_text);
}
