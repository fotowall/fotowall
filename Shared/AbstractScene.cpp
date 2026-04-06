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

#include "AbstractScene.h"
#include "Shared/Commands.h"

AbstractScene::AbstractScene(QObject * parent) : QGraphicsScene(parent), m_rotationAngle(0) {}

void AbstractScene::adjustSceneSize()
{ emit sceneSizeChanged(); }

void AbstractScene::resizeAutoFit()
{
  QPointF bottomRight = itemsBoundingRect().bottomRight();
  resize(QSize(qMax(10, (int)bottomRight.x()), qMax(10, (int)bottomRight.y())));
}

void AbstractScene::resize(const QSize & size)
{
  // skip if already ok
  if(size == m_size) return;

  // save size and the related rect
  // QSize oldSize = m_size;
  m_size = size;
  m_rect = QRectF(0, 0, m_size.width(), m_size.height());

  // change my rect
  setSceneRect(m_rect);

  // call handlers
  // QResizeEvent re(m_size, oldSize);
  resizeEvent(/*&re*/);
}

void AbstractScene::resizeEvent()
{
  // nothing to do here
}

bool AbstractScene::sceneSelectable() const
{ return true; }

void AbstractScene::setPerspective(const QPointF & angles)
{
  if(angles != m_perspectiveAngles)
  {
    m_perspectiveAngles = angles;
    emit scenePerspectiveChanged();
  }
}

void AbstractScene::setPerspective_(const QPointF & angles)
{
  if(angles != m_perspectiveAngles)
  {
    do_canvas_command(this, new PerspectiveCommand<AbstractScene>(this, m_perspectiveAngles, angles));
  }
}

QPointF AbstractScene::perspective() const
{ return m_perspectiveAngles; }

void AbstractScene::setRotation(qreal rotation)
{
  if(rotation != m_rotationAngle)
  {
    m_rotationAngle = rotation;
    emit sceneRotationChanged();
  }
}

void AbstractScene::setRotation_(qreal rotation)
{
  if(rotation != m_rotationAngle)
  {
    do_canvas_command(this, new SceneRotationCommand(this, m_rotationAngle, rotation));
  }
}

qreal AbstractScene::rotation() const
{ return m_rotationAngle; }
