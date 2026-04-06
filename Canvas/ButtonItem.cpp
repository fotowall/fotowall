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

#include "ButtonItem.h"

#include "Shared/RenderOpts.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

ButtonItem::ButtonItem(Type type, const QBrush & brush, const QIcon & icon, QGraphicsItem * parent)
: QGraphicsItem(parent), m_type(type), m_icon(icon), m_brush(brush), m_selectsParent(true)
{
  setAcceptHoverEvents(true);
#if 0
    if (type == Control)
        setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
#endif
}

ButtonItem::Type ButtonItem::buttonType() const
{ return m_type; }

int ButtonItem::width() const
{ return 16; }

int ButtonItem::height() const
{ return 16; }

void ButtonItem::setSelectsParent(bool selects)
{ m_selectsParent = selects; }

QRectF ButtonItem::boundingRect() const
{ return QRectF(-8, -8, 16, 16); }

void ButtonItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * /*widget*/)
{
  if(RenderOpts::HQRendering) return;
  bool over = option->state & QStyle::State_MouseOver;
  if(over)
  {
    if(m_startPos.isNull())
      painter->fillRect(boundingRect().adjusted(-1, -1, 1, 1), m_brush);
    else
      painter->fillRect(boundingRect().adjusted(-1, -1, 1, 1), Qt::white);
  }
  if(!m_icon.isNull())
  {
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    m_icon.paint(painter, boundingRect().toRect(), Qt::AlignCenter, over ? QIcon::Active : QIcon::Normal);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
  }
}

void ButtonItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
  event->accept();
  if(m_selectsParent)
  {
    scene()->clearSelection();
    parentItem()->setSelected(true);
  }
  m_startPos = event->scenePos();
  update();
  emit pressed();
}

void ButtonItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
  if(m_startPos.isNull()) return;
  event->accept();
  emit dragging(event->scenePos() - m_startPos, event->modifiers());
}

void ButtonItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
  event->accept();
  emit releaseEvent(event);
  bool dragging = !m_startPos.isNull();
  m_startPos = QPointF();
  update();
  if(contains(event->pos()))
    if(dragging) emit clicked();
}

void ButtonItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
  event->accept();
  emit doubleClicked();
}
