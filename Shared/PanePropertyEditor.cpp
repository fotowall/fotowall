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

#include "PanePropertyEditor.h"

#include <QLinearGradient>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>

#if QT_VERSION >= 0x040600
#  include <QPropertyAnimation>
#endif

PaneWidget::PaneWidget(QWidget * parent)
: QWidget(parent), m_range(-1.0, -1.0, 2.0, 2.0), m_value(0.0, 0.0), m_hovered(false), m_pressed(false)
{
}

QPointF PaneWidget::value() const
{ return m_value; }

QPointF PaneWidget::endValue() const
{ return m_endValue; }

void PaneWidget::setValue(const QPointF & value)
{
  if(value != m_value)
  {
    m_value = value;
    emit valueChanged(value);
    update();
  }
}

QRectF PaneWidget::range() const
{ return m_range; }

void PaneWidget::setRange(const QRectF & range)
{
  if(range != m_range)
  {
    m_range = range;
    emit rangeChanged(m_range);
    update();
  }
}

void PaneWidget::enterEvent(QEvent *)
{
  m_hovered = true;
  update();
}

void PaneWidget::leaveEvent(QEvent *)
{
  m_hovered = false;
  update();
}

void PaneWidget::mousePressEvent(QMouseEvent * event)
{
  if(event->button() != Qt::LeftButton) return;
  m_pressed = true;
  emit pressed();
  pressing(event->localPos());
}

void PaneWidget::mouseMoveEvent(QMouseEvent * event)
{
  if(m_pressed) pressing(event->localPos());
}

void PaneWidget::mouseReleaseEvent(QMouseEvent *)
{
  m_pressed = false;
  emit released();
  update();
}

void PaneWidget::mouseDoubleClickEvent(QMouseEvent *)
{
  // using setValue directly breaks animations
  pressing(screenMap(QPointF(0, 0)));
}

void PaneWidget::paintEvent(QPaintEvent * event)
{
  // draw background
  QPainter p(this);
  if(m_hovered)
  {
    QLinearGradient lg(0, 0, 0, height());
    lg.setColorAt(0.0, QColor(128, 128, 128, 64));
    lg.setColorAt(1.0, QColor(255, 255, 255, 128));
    p.fillRect(event->rect(), lg);
  }
#if 1
  else
    p.fillRect(event->rect(), QColor(64, 64, 64, 16));
#endif

  // draw axis
  QPointF pt = screenMap(QPointF(0, 0));
  p.setPen(QPen(Qt::lightGray, 1.0));
#if 0
    p.drawRect(rect().adjusted(0, 0, -1, -1));
#endif
  p.setRenderHint(QPainter::Antialiasing, true);
  p.drawLine(pt.x(), 0, pt.x(), height());
  p.drawLine(0, pt.y(), width(), pt.y());

  // draw point
  pt = screenMap(m_value);
  p.setPen(m_pressed ? Qt::red : QColor(50, 100, 120, 200));
  p.setBrush(QColor(210, 100, 110, 120));
  p.drawEllipse(pt, 5, 5);
}

void PaneWidget::pressing(const QPointF & pos)
{
  if(width() < 2 || height() < 2) return;
  double px = m_range.left() + m_range.width() * pos.x() / (double)(width() - 1);
  double py = m_range.top() + m_range.height() * pos.y() / (double)(height() - 1);
  m_endValue =
      QPointF(qBound(m_range.left(), (qreal)px, m_range.right()), qBound(m_range.top(), (qreal)py, m_range.bottom()));
#if QT_VERSION >= 0x040600
  // animate the change
  QPropertyAnimation * ani = new QPropertyAnimation(this, "value");
  ani->setEasingCurve(QEasingCurve::OutCubic);
  ani->setDuration(500);
  ani->setEndValue(m_endValue);
  ani->start(QPropertyAnimation::DeleteWhenStopped);
#else
  // set the final value
  setValue(m_endValue);
#endif
}

QPointF PaneWidget::screenMap(const QPointF & value) const
{
  if(m_range.width() <= 0 || m_range.height() <= 0) return QPointF();
  double sx = (value.x() - m_range.left()) * (double)(width() - 1) / m_range.width();
  double sy = (value.y() - m_range.top()) * (double)(height() - 1) / m_range.height();
  return QPointF(sx, sy);
}

PE_PaneWidget::PE_PaneWidget(PaneWidget * _pane, QObject * _target, const char * propertyName, QObject * parent)
: PE_TypeControl<PaneWidget>(_pane, _target, propertyName, parent)
{
  // read initial value and link to property changes
  slotPropertyChanged();
  PE_LISTEN_TO_PROPERTY(slotPropertyChanged());

  // link to the slider changes
  connect(m_control.data(), SIGNAL(valueChanged(const QPointF &)), this, SLOT(slotPaneValueChanged(const QPointF &)));

  // allow Int and Double properties only
  if(m_property.type() != QVariant::PointF)
    qWarning("PE_PaneWidget: unhandled property '%s' of type %d", propertyName, (int)m_property.type());
  else
    m_isValid = true;
}

void PE_PaneWidget::slotPaneValueChanged(const QPointF & pointfValue)
{
  // set the property to the current value of the PaneWidget
  if(m_control && m_target && m_property.type() == QVariant::PointF) m_property.write(m_target.data(), pointfValue);
}

void PE_PaneWidget::slotPropertyChanged()
{
  // set the control pointf as the property value
  if(m_control && m_target && m_property.type() == QVariant::PointF)
  {
    QPointF pointfValue = m_property.read(m_target.data()).toPointF();
    m_control->setValue(pointfValue);
  }
}
