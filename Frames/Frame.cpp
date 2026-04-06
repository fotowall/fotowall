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

#include "Frame.h"
#include <QRectF>

QRect Frame::frameRect(const QRect & contentsRect) const
{ return contentsRect; }

bool Frame::clipContents() const
{ return false; }

QPainterPath Frame::contentsClipPath(const QRect & /*contentsRect*/) const
{ return QPainterPath(); }

bool Frame::isShaped() const
{ return false; }

QPainterPath Frame::frameShape(const QRect & frameRect) const
{
  QPainterPath path;
  path.addRect(frameRect);
  return path;
}

QPixmap Frame::preview(int width, int height)
{
  QPixmap pixmap(128, 128);
  pixmap.fill(Qt::transparent);
  QPainter pixPainter(&pixmap);
  drawFrame(&pixPainter, QRect(0, 0, 128, 128), false, false);
  pixPainter.end();
  return pixmap.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

Frame::~Frame() {}
