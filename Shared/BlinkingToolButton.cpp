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

#include "BlinkingToolButton.h"

#include <QApplication>
#include <QPainter>
#include <QPalette>
#include <QStyle>
#include <QVariant>

#if QT_VERSION >= 0x040600
#  include <QPropertyAnimation>
#  define ANIMATE_PARAM(object, propName, duration, endValue)                      \
    {                                                                              \
      QPropertyAnimation * ani = new QPropertyAnimation(object, propName, object); \
      ani->setEasingCurve(QEasingCurve::Linear);                                   \
      ani->setDuration(duration);                                                  \
      ani->setEndValue(endValue);                                                  \
      ani->start(QPropertyAnimation::DeleteWhenStopped);                           \
    }
#else
#  define ANIMATE_PARAM(instance, propName, duration, endValue) instance->setProperty(propName, endValue);
#endif

BlinkingToolButton::BlinkingToolButton(QWidget * parent)
: QToolButton(parent), m_palette(palette()),
  m_markPixmap(style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(16, 16)), m_markOpacity(0)
{ connect(this, SIGNAL(clicked()), this, SLOT(clearAttenction())); }

void BlinkingToolButton::drawAttenction()
{
  setMarkOpacity(1.0);
#if 0
    ANIMATE_PARAM(this, "backColor", 1000, QColor(255, 20, 20));
    ANIMATE_PARAM(this, "textColor", 1000, QColor(255, 255, 255));
#else
  ANIMATE_PARAM(this, "backColor", 1000, QApplication::palette().color(QPalette::Highlight));
  ANIMATE_PARAM(this, "textColor", 1000, QApplication::palette().color(QPalette::HighlightedText));
#endif
}

void BlinkingToolButton::clearAttenction()
{
  setMarkOpacity(0.0);
  ANIMATE_PARAM(this, "backColor", 200, QApplication::palette().color(QPalette::Button));
  ANIMATE_PARAM(this, "textColor", 200, QApplication::palette().color(QPalette::ButtonText));
}

void BlinkingToolButton::paintEvent(QPaintEvent * event)
{
  // unbreak painting
  QToolButton::paintEvent(event);

  // draw mark
  if(m_markOpacity > 0.0 && !m_markPixmap.isNull())
  {
    QPainter p(this);
    if(m_markOpacity < 1.0) p.setOpacity(m_markOpacity);
    p.drawPixmap(width() - m_markPixmap.width(), 0, m_markPixmap);
  }
}

QSize BlinkingToolButton::sizeHint() const
{
  QSize s = QToolButton::sizeHint();
  if(s.isValid() && !m_markPixmap.isNull()) s.rwidth() += m_markPixmap.width();
  return s;
}

QColor BlinkingToolButton::backColor() const
{ return m_palette.color(QPalette::Button); }

void BlinkingToolButton::setBackColor(const QColor & color)
{
  if(color != m_palette.color(QPalette::Button))
  {
    m_palette.setColor(QPalette::Button, color);
    setPalette(m_palette);
    update();
  }
}

QColor BlinkingToolButton::textColor() const
{ return m_palette.color(QPalette::ButtonText); }

void BlinkingToolButton::setTextColor(const QColor & color)
{
  if(color != m_palette.color(QPalette::ButtonText))
  {
    m_palette.setColor(QPalette::ButtonText, color);
    setPalette(m_palette);
    update();
  }
}

qreal BlinkingToolButton::markOpacity() const
{ return m_markOpacity; }

void BlinkingToolButton::setMarkOpacity(qreal opacity)
{
  if(opacity != m_markOpacity)
  {
    m_markOpacity = opacity;
    update();
  }
}
