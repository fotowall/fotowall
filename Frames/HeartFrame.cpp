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

#include "HeartFrame.h"

#include "Shared/RenderOpts.h"

#include <QLinearGradient>
#include <QPainter>

static QPainterPath heartPath(const QRect & r)
{
  double left = r.left(), top = r.top(), height = r.height(), width = r.width(), p1x = left + width / 2,
         p1y = top + height / 3, p2x = p1x, p2y = top + height, h4 = height / 4, skew = 0 * width / 20,
         offs = height / 20;
  QPainterPath path;
  path.moveTo(p1x, p1y - offs);
  path.cubicTo(p1x, top, left + width, top, left + width, p1y);
  path.cubicTo(left + width, p1y + h4, p2x + skew, p2y - h4, p2x, p2y);
  path.cubicTo(p2x + skew, p2y - h4, left, p1y + h4, left, p1y);
  path.cubicTo(left, top, p1x, top, p1x, p1y - offs);
  return path;
}

QRect HeartFrame::frameRect(const QRect & contentsRect) const
{
  int xM = contentsRect.width() / 20;
  int yM = contentsRect.height() / 20;
  return contentsRect.adjusted(-xM, -yM, xM, yM);
}

bool HeartFrame::clipContents() const
{ return true; }

QPainterPath HeartFrame::contentsClipPath(const QRect & contentsRect) const
{ return heartPath(contentsRect); }

bool HeartFrame::isShaped() const
{ return true; }

QPainterPath HeartFrame::frameShape(const QRect & frameRect) const
{ return heartPath(frameRect); }

void HeartFrame::layoutButtons(QList<ButtonItem *> buttons, const QRect & frameRect) const
{
  double x[] = {0.92, 0.84, 0.95, 0.93, 0.84, 0.5, 0.80};
  double y[] = {0.22, 0.15, 0.33, 0.44, 0.55, 0.17, 0.45};
  for(int i = 0; i < buttons.size() && i < 7; i++)
  {

    QGraphicsItem * button = buttons[i];
    button->setPos(frameRect.left() + (int)((float)frameRect.width() * x[i]),
                   frameRect.top() + (int)((float)frameRect.height() * y[i]));
  }
}

void HeartFrame::layoutText(QGraphicsItem * textItem, const QRect & /*frameRect*/) const
{
  textItem->hide();
  // textItem->setPos( frameRect.left() + d->padL, frameRect.center().y() - textItem->boundingRect().size().height() /
  // 2
  // );
}

void HeartFrame::drawFrame(QPainter * painter, const QRect & frameRect, bool selected, bool /*opaqueContents*/)
{
  QLinearGradient lg(0, frameRect.top(), frameRect.width() / 8, frameRect.height() / 2);
  lg.setColorAt(0.0, QColor(196, 00, 00));
  lg.setColorAt(0.3, Qt::red);
  lg.setColorAt(1.0, QColor(128, 00, 00));

  QPainterPath path = heartPath(frameRect);
  if(selected)
    painter->setPen(QPen(RenderOpts::hiColor, 2.0));
  else
    painter->setPen(QPen(QColor(64, 0, 0), 1.0));
  painter->setBrush(lg);
  painter->drawPath(path);
}
