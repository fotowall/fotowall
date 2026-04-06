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

#include "PixmapButton.h"

#include <QApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionFocusRect>

PixmapButton::PixmapButton(QWidget * parent) : QAbstractButton(parent), m_fadeInactive(false), m_hovering(false)
{
  m_hoverFont = QApplication::font();
  m_hoverFont.setPixelSize(10);
  setFixedSize(32, 32);
}

void PixmapButton::setPixmap(const QPixmap & pixmap)
{
  m_fixedPixmap = pixmap;
  update();
}

QPixmap PixmapButton::pixmap() const
{ return m_fixedPixmap; }

void PixmapButton::setHoverText(const QString & text)
{
  m_hoverText = text;
  update();
}

QString PixmapButton::hoverText() const
{ return m_hoverText; }

void PixmapButton::setHoverPixmap(const QPixmap & pixmap)
{
  m_hoverPixmap = pixmap;
  update();
}

QPixmap PixmapButton::hoverPixmap() const
{ return m_hoverPixmap; }

void PixmapButton::setFadeInactive(bool fade)
{
  if(m_fadeInactive != fade)
  {
    m_fadeInactive = fade;
    update();
  }
}

bool PixmapButton::fadeInactive() const
{ return m_fadeInactive; }

void PixmapButton::setFixedSize(const QSize & size)
{
  m_fixedSize = size;
  QAbstractButton::setFixedSize(m_fixedSize);
}

void PixmapButton::setFixedSize(int w, int h)
{ setFixedSize(QSize(w, h)); }

QSize PixmapButton::fixedSize() const
{ return m_fixedSize; }

void PixmapButton::enterEvent(QEvent *)
{
  m_hovering = true;
  update();
}

void PixmapButton::leaveEvent(QEvent *)
{
  m_hovering = false;
  update();
}

void PixmapButton::paintEvent(QPaintEvent *)
{
  QPainter p(this);

  // draw background if checked
  if(isChecked())
  {
#if 0
        p.fillRect(rect(), palette().color(QPalette::Button));
#else
    QStyleOption opt(0);
    opt.palette = palette();
    opt.state = QStyle::State_Enabled | QStyle::State_Sunken;
    if(m_hovering) opt.state |= QStyle::State_MouseOver;
    opt.rect = rect();
    style()->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, &p, this);
#endif
  }
  // draw background if hovering
  else if(m_hovering)
    p.fillRect(rect(), palette().color(QPalette::Highlight));

  // draw pixmap, or placeholder
  if(m_fixedPixmap.isNull())
  {
#if 0
        p.fillRect(rect().adjusted(2, 2, -2, -2), palette().color(QPalette::Button));
#endif
  }
  else
  {
    bool fade = m_fadeInactive && !isDown() && !m_hovering && !hasFocus();
    if(fade) p.setOpacity(0.4);
    int offset = (isDown() | isChecked()) ? 1 : 0;
    p.drawPixmap(offset + (width() - m_fixedPixmap.width()) / 2, offset + (height() - m_fixedPixmap.height()) / 2,
                 m_fixedPixmap);
    if(fade) p.setOpacity(1.0);
  }

  // draw hover text, if any
  if(!m_hoverText.isEmpty())
  {
#if 1
    const int h = 13;
    const int w = h * m_hoverText.size();
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
    p.setPen(Qt::NoPen);
    p.setBrush(palette().color(QPalette::Highlight));
    QRectF rect(0, 0, w, h);
    p.drawEllipse(rect.adjusted(0.5, 0.5, -0.5, -0.5));
    p.setPen(palette().color(QPalette::HighlightedText));
    p.setFont(m_hoverFont);
    p.drawText(rect.toRect()
#  if defined(Q_OS_WIN) || defined(Q_OS_OS2)
                   .adjusted(0, -1, 0, -1)
#  endif
                   ,
               Qt::AlignCenter, m_hoverText);
#endif
  }

  // draw hover pixmap, if hovering
  if(m_hovering && !m_hoverPixmap.isNull())
    p.drawPixmap(width() - m_hoverPixmap.width(), height() - m_hoverPixmap.height(), m_hoverPixmap);

  // draw focus, if any
  if(hasFocus())
  {
    QStyleOptionFocusRect opt;
    opt.initFrom(this);
    style()->drawPrimitive(QStyle::PE_FrameFocusRect, &opt, &p, this);
  }
}
