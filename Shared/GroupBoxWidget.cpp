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

#include "GroupBoxWidget.h"
#include <QFontMetrics>
#include <QLayout>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionButton>
#include <QTimer>
#include <QVariant>

GroupBoxWidget::GroupBoxWidget(QWidget * parent)
: QWidget(parent), m_redesignTimer(0)
#if !defined(MOBILE_UI)
  ,
  m_collapsed(false), m_checkable(false), m_checked(true), m_borderFlags(0), m_checkValue(1.0), m_hoverValue(0.0)
#else
  ,
  m_smartPanel(false), m_panelState(0.0)
#endif
{
  // setup groupbox text
  m_titleFont = font();
  m_titleFont.setPixelSize(10);

  // using a fixed HSizePolicy we better integrate with auto-layouting
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

#if !defined(MOBILE_UI)
  // autofill on on desktop (inserted on topbar)
  setAutoFillBackground(true);
#else
  // autofill off on mobile (inserted on canvas)
  setAutoFillBackground(false);
#endif

  // hide junk before initial layouting
  hide();
}

QString GroupBoxWidget::title() const
{ return m_titleText; }

void GroupBoxWidget::setTitle(const QString & title)
{
  m_titleText = title;
  updateDesign();
}

int GroupBoxWidget::titleSize() const
{ return m_titleFont.pixelSize(); }

void GroupBoxWidget::setTitleSize(int titleSize)
{
  m_titleFont.setPointSize(titleSize);
  updateDesign();
}

#if !defined(MOBILE_UI)
#  if QT_VERSION >= 0x040600
#    include <QPropertyAnimation>
#    define ANIMATE_PARAM(propName, duration, endValue, finalizeLayout)                        \
      {                                                                                        \
        QPropertyAnimation * ani = new QPropertyAnimation(this, propName, this);               \
        if(finalizeLayout) connect(ani, SIGNAL(finished()), this, SLOT(slotFinalizeDesign())); \
        ani->setEasingCurve(QEasingCurve::OutQuint);                                           \
        ani->setDuration(duration);                                                            \
        ani->setEndValue(endValue);                                                            \
        ani->start(QPropertyAnimation::DeleteWhenStopped);                                     \
      }
#  else
#    define ANIMATE_PARAM(propName, duration, endValue, finalizeLayout) \
      {                                                                 \
        setProperty(propName, endValue);                                \
        if(finalizeLayout) slotFinalizeDesign();                        \
      }
#  endif

int GroupBoxWidget::calcMinWidth() const
{ return qMax(QFontMetrics(m_titleFont).horizontalAdvance(m_titleText) + 12, QWidget::sizeHint().width()); }

bool GroupBoxWidget::isCheckable() const
{ return m_checkable; }

void GroupBoxWidget::setCheckable(bool checkable)
{
  if(m_checkable == checkable) return;
  m_checkable = checkable;
  updateDesign();
}

bool GroupBoxWidget::isChecked() const
{ return m_checkable ? m_checked : false; }

void GroupBoxWidget::setChecked(bool checked)
{
  if(!m_checkable || m_checked == checked) return;
  m_checked = checked;
  emit toggled(checked);
  updateDesign();
}

int GroupBoxWidget::borderFlags() const
{ return m_borderFlags; }

void GroupBoxWidget::setBorderFlags(int flags)
{
  m_borderFlags = flags;
  update();
}

void GroupBoxWidget::collapse()
{
  if(!m_collapsed)
  {
    m_collapsed = true;
    updateDesign();
  }
}

void GroupBoxWidget::expand()
{
  if(m_collapsed)
  {
    m_collapsed = false;
    updateDesign();
  }
}

void GroupBoxWidget::mousePressEvent(QMouseEvent * /*event*/)
{ setChecked(!isChecked()); }

void GroupBoxWidget::paintEvent(QPaintEvent * /*event*/)
{
  // skip the rest of the painting if no text
  if(m_titleText.isEmpty()) return;

  // draw hovering
  QPainter p(this);
  if(m_hoverValue > 0 && (m_checkValue == 0.0 || m_checkValue == 1.0))
  {
    QRadialGradient rg = m_checkValue == 1.0 ? QRadialGradient(0.5, 0.2, 0.8) : QRadialGradient(0.5, 1.0, 1.5);
    QColor startColor(Qt::white);
    startColor.setAlpha((int)(255.0 * m_hoverValue));
    rg.setColorAt(0.0, startColor);
    rg.setColorAt(1.0, Qt::transparent);
    rg.setColorAt(0.0, startColor);
    rg.setColorAt(1.0, Qt::transparent);
    rg.setCoordinateMode(QGradient::ObjectBoundingMode);
    p.fillRect(0, 0, width(), height() - m_checkValue * (height() - 12), rg);
  }

  // draw left/right lines
  if(m_borderFlags & 0x0001) p.fillRect(0, 0, 1, height(), QColor(230, 230, 230));
  if(m_borderFlags & 0x0002) p.fillRect(width() - 1, 0, 1, height(), Qt::white);

  // precalc text position and move painter
  QStyle * ss = m_checkable ? style() : 0;
  int indW = ss ? ss->pixelMetric(QStyle::PM_IndicatorWidth, 0, 0) : 0;
  int indH = ss ? ss->pixelMetric(QStyle::PM_IndicatorHeight, 0, 0) : 0;
  p.save();
  p.setFont(m_titleFont);
  QFontMetrics metrics(m_titleFont);
  QRect textRect = metrics.boundingRect(m_titleText);
  //    int textHeight = textRect.height();
  int dx = 0;
  if(m_checkValue < 1.0)
  {
    qreal x1 = -textRect.top() + 2, x2 = (width() - textRect.width() - indW - 4) / 2;
    qreal y1 = height() - 2, // height() + textRect.width()) / 2,
        y2 = -textRect.top();
    p.translate(x1 + m_checkValue * (x2 - x1), y1 + m_checkValue * (y2 - y1));
    p.rotate(m_checkValue * 90 - 90);
  }
  else
    p.translate((width() - textRect.width() - indW - 4) / 2, -textRect.top());

  // draw checkbox indicator
  if(m_checkable && indW && indH)
  {
    QStyleOptionButton opt;
    opt.state = QStyle::State_Enabled;
    int styleOffset = (textRect.height() - indH) / 2;
    opt.rect = QRect(0, -indH + 4 - styleOffset, indW, indH);
    dx = indW + 4;
    opt.state |= m_checked ? QStyle::State_On : QStyle::State_Off;
    if(underMouse()) opt.state |= QStyle::State_MouseOver;
    // p.setRenderHint(QPainter::Antialiasing, true);
    style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &opt, &p, this);
  }

  // draw text
  p.drawText(dx, 0, m_titleText);
  p.restore();
}

void GroupBoxWidget::enterEvent(QEvent *)
{ ANIMATE_PARAM("hAnim", 400, 1.0, false); }

void GroupBoxWidget::leaveEvent(QEvent *)
{ ANIMATE_PARAM("hAnim", 400, 0.0, false); }

qreal GroupBoxWidget::checkValue() const
{ return m_checkValue; }

void GroupBoxWidget::setCheckValue(qreal value)
{
  m_checkValue = value;
  update();
}

qreal GroupBoxWidget::hoverValue() const
{ return m_hoverValue; }

void GroupBoxWidget::setHoverValue(qreal value)
{
  m_hoverValue = value;
  update();
}

void GroupBoxWidget::updateDesign()
{
  if(m_collapsed)
  {
    // full collapse: no pre-work
  }
  else if(m_checkable && !m_checked)
  {
    // checkable off: label on left
    setContentsMargins(m_titleFont.pixelSize() + 8, 0, 0, 0);
    setCursor(Qt::PointingHandCursor);
  }
  else
  {
    // normal: label on top
    setContentsMargins(0, m_titleFont.pixelSize() + 8, 0, 0);
    setCursor(Qt::ArrowCursor);
  }

  // defer and accumulate layout calculations
  if(!m_redesignTimer)
  {
    m_redesignTimer = new QTimer(this);
    m_redesignTimer->setSingleShot(true);
    connect(m_redesignTimer, SIGNAL(timeout()), SLOT(slotAnimateDesign()));
  }
  m_redesignTimer->start();
}

void GroupBoxWidget::slotAnimateDesign()
{
  if(m_collapsed)
  {
    // full collapse: shrink to zero
    ANIMATE_PARAM("fixedWidth", 200, 0, true)
  }
  else if(m_checkable && !m_checked)
  {
    // checkable off: shrink to the target width
    ANIMATE_PARAM("fixedWidth", 200, m_titleFont.pixelSize() + 8, true);
    ANIMATE_PARAM("cAnim", 200, 0.0, false);
  }
  else
  {
    // normal: expand to the full size
    ANIMATE_PARAM("fixedWidth", 300, calcMinWidth(), true);
    ANIMATE_PARAM("cAnim", 200, 1.0, false);
  }

  // it was hidden in the constructor for not showing junk before initial layouting
  show();
}

void GroupBoxWidget::slotFinalizeDesign()
{
  if(m_collapsed)
  {
    // full collapse: zero width
    setFixedWidth(0);
  }
  else if(m_checkable && !m_checked)
  {
    // checkable off: left label
    setFixedWidth(m_titleFont.pixelSize() + 8);
    setCheckValue(0.0);
  }
  else
  {
    // normal: layout driven
    setMinimumWidth(calcMinWidth());
    setMaximumWidth(QWIDGETSIZE_MAX);
    setCheckValue(1.0);
  }
}
#endif

//
// Mobile Version customizations
//
#if defined(MOBILE_UI)
#  include <QPaintEvent>
#  include <QPropertyAnimation>
#  define SP_MARGIN 2
#  define SP_SPACING 2

int GroupBoxWidget::calcMinWidth() const
{ return qMax(m_labelRect.width(), QWidget::sizeHint().width()); }

void GroupBoxWidget::setSmartPanel(bool smart)
{
  if(smart != m_smartPanel)
  {
    m_smartPanel = smart;
    updateDesign();
  }
}

QPoint GroupBoxWidget::basePos() const
{ return m_basePos; }

void GroupBoxWidget::setBasePos(const QPoint & pos)
{
  if(m_basePos != pos)
  {
    m_basePos = pos;
    updateDesign();
  }
}

int GroupBoxWidget::labelWidth() const
{ return m_labelRect.width(); }

void GroupBoxWidget::disappear()
{
  if(m_panelState >= 0.01) emit panelLowering();
  m_panelState = 0.0;
  m_titleText = QString();
  updateDesign();
}

void GroupBoxWidget::smartFall()
{
  if(m_smartPanel)
  {
    if(m_panelState >= 0.01) emit panelLowering();
    QPropertyAnimation * ani = new QPropertyAnimation(this, "panelState", this);
    ani->setEndValue((qreal)-1.0);
    ani->setEasingCurve(QEasingCurve::OutQuad);
    ani->setDuration(200);
    ani->start(QAbstractAnimation::DeleteWhenStopped);
  }
}

void GroupBoxWidget::smartRaise()
{
  if(m_smartPanel)
  {
    QPropertyAnimation * ani = new QPropertyAnimation(this, "panelState", this);
    ani->setEndValue((qreal)0.0);
    ani->setEasingCurve(QEasingCurve::OutQuad);
    ani->setDuration(200);
    ani->start(QAbstractAnimation::DeleteWhenStopped);
  }
}

qreal GroupBoxWidget::panelState() const
{ return m_panelState; }

void GroupBoxWidget::setPanelState(qreal state)
{
  if(state != m_panelState)
  {
    m_panelState = state;
    move(m_basePos + QPoint(0, verticalOffset()));
  }
}

void GroupBoxWidget::mousePressEvent(QMouseEvent * event)
{
  if(!m_smartPanel || m_titleText.isEmpty() || !m_labelRect.contains(event->pos())) return;
  if(m_panelState <= 0.01)
  {
    QPropertyAnimation * ani = new QPropertyAnimation(this, "panelState", this);
    ani->setEndValue((qreal)1.0);
    ani->setEasingCurve(QEasingCurve::OutQuad);
    ani->setDuration(200);
    ani->start(QAbstractAnimation::DeleteWhenStopped);
    emit panelRaising();
  }
  else
  {
    QPropertyAnimation * ani = new QPropertyAnimation(this, "panelState", this);
    ani->setEndValue((qreal)0.0);
    ani->setEasingCurve(QEasingCurve::OutQuad);
    ani->setDuration(200);
    ani->start(QAbstractAnimation::DeleteWhenStopped);
    emit panelLowering();
  }
}

void GroupBoxWidget::paintEvent(QPaintEvent * event)
{
  // skip the rest of the painting if no text
  if(m_titleText.isEmpty()) return;

  QPainter p(this);
  p.setFont(m_titleFont);

  // compatibility drawing
  if(!m_smartPanel)
  {
    p.fillRect(event->rect(), palette().color(QPalette::Window));
    p.setPen(Qt::black);
    p.drawText(QRect(0, 0, width(), m_labelRect.height()), Qt::AlignCenter, m_titleText);
    return;
  }

  // draw label
  QLinearGradient lg(0, 0, 0, height());
  QColor wColor(200, 200, 200, 200);
  lg.setColorAt(0.0, wColor.lighter());
  lg.setColorAt(1.0, wColor);
  if(m_panelState < 0.99)
  {
    p.setPen(QPen(Qt::white, 0));
    p.setBrush(lg);
    QPainterPath path;
    path.moveTo(m_labelRect.left(), m_labelRect.bottom() + 1);
    path.lineTo(m_labelRect.left(), 3);
    path.quadTo(m_labelRect.left(), 0, m_labelRect.left() + 3, 0);
    path.lineTo(m_labelRect.right() - 4, 0);
    path.quadTo(m_labelRect.right() - 1, 0, m_labelRect.right() - 1, 3);
    path.lineTo(m_labelRect.right() - 1, m_labelRect.bottom() + 1);
    p.drawPath(path);
  }
  else
    p.fillRect(m_labelRect, lg);

  // draw background
  if(m_panelState > 0.0)
  {
    p.fillRect(1, m_labelRect.bottom() + 1, width() - 1, height() - m_labelRect.bottom() - 1, lg);
    p.fillRect(0, m_labelRect.bottom() + 1, 1, height() - m_labelRect.bottom() - 1, Qt::white);
  }

  // draw text
  p.setPen(Qt::black);
  p.drawText(m_labelRect, Qt::AlignCenter, m_titleText);
}

void GroupBoxWidget::updateDesign()
{
  // calc text box size
  QFontMetrics fm(m_titleFont);
  int textWidth = fm.width(m_titleText) + 32;
  QRect newLabelRect = QRect(0, 0, textWidth, 20);
  bool emitLabelChanged = false;
  if(newLabelRect != m_labelRect)
  {
    emitLabelChanged = true;
    m_labelRect = newLabelRect;
    setContentsMargins(0, m_labelRect.height() + SP_SPACING, 0, 0);
    update();
  }

  // move me
  move(m_basePos + QPoint(0, verticalOffset()));

  // emit label changed
  if(emitLabelChanged) emit labelSizeChanged();

  // defer and accumulate layout calculations
  if(!m_redesignTimer)
  {
    m_redesignTimer = new QTimer(this);
    m_redesignTimer->setSingleShot(true);
    connect(m_redesignTimer, SIGNAL(timeout()), SLOT(slotAnimateDesign()));
  }
  m_redesignTimer->start();
}

int GroupBoxWidget::verticalOffset() const
{
  if(!m_panelState) return -m_labelRect.height();
  if(m_panelState > 0) return -m_labelRect.height() - (int)((qreal)(height() - m_labelRect.height()) * m_panelState);
  return -(int)((qreal)m_labelRect.height() * (1.0 + m_panelState));
}

void GroupBoxWidget::slotAnimateDesign()
{
  // no animation for the design
  slotFinalizeDesign();

  // it was hidden in the constructor for not showing junk before initial layouting
  show();
}

void GroupBoxWidget::slotFinalizeDesign()
{
  setMinimumWidth(calcMinWidth());
  setMaximumWidth(QWIDGETSIZE_MAX);
}
#endif
