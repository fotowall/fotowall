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

#include "AbstractDisposeable.h"

AbstractDisposeable::AbstractDisposeable(bool fadeIn, QGraphicsItem * parent)
#if QT_VERSION >= 0x040600
: QGraphicsObject(parent)
#else
: QGraphicsItem(parent)
#endif
  ,
  m_contentOpacity(1.0)
{
  if(fadeIn)
  {
#if QT_VERSION >= 0x040600
    // fade in
    QPropertyAnimation * ao = new QPropertyAnimation(this, "contentOpacity");
    ao->setEasingCurve(QEasingCurve::OutQuad);
    ao->setDuration(150);
    ao->setStartValue(0.01);
    ao->setEndValue(1.0);
    ao->start(QPropertyAnimation::DeleteWhenStopped);
#  if !defined(MOBILE_UI)
    // zoom in
    QPropertyAnimation * as = new QPropertyAnimation(this, "scale");
    as->setEasingCurve(QEasingCurve::OutElastic);
    as->setDuration(500);
    as->setStartValue(0.2);
    as->setEndValue(1.0);
    as->start(QPropertyAnimation::DeleteWhenStopped);
#  endif
#endif
    show();
  }
}

void AbstractDisposeable::dispose()
{
#if QT_VERSION >= 0x040600
  // fade out animation, then delete
  QPropertyAnimation * ani = new QPropertyAnimation(this, "contentOpacity");
  connect(ani, SIGNAL(finished()), this, SLOT(deleteLater()));
  ani->setEasingCurve(QEasingCurve::OutCubic);
  ani->setDuration(300);
  ani->setEndValue(0.0);
  ani->start(QPropertyAnimation::DeleteWhenStopped);
#else
  // delete this now
  deleteLater();
#endif
}

qreal AbstractDisposeable::contentOpacity() const
{ return m_contentOpacity; }

void AbstractDisposeable::setContentOpacity(qreal opacity)
{
  if(m_contentOpacity != opacity)
  {
    m_contentOpacity = opacity;
    update();
    emit contentOpacityChanged();
  }
}
