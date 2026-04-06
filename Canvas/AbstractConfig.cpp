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

#include "AbstractConfig.h"

#include "AbstractContent.h"
#include "Frames/FrameFactory.h"
#include "Shared/RenderOpts.h"
#include "StyledButtonItem.h"
#include "ui_AbstractConfig.h"

#include <QSettings>

#include <QFileDialog>
#include <QGraphicsSceneMouseEvent>
#include <QListWidgetItem>
#include <QPainter>
#include <QPixmapCache>
#include <QPushButton>
#include <QStyle>
#include <QWidget>

#if QT_VERSION >= 0x040600
#  include <QPropertyAnimation>
#endif

AbstractConfig::AbstractConfig(AbstractContent * content, AbstractConfig_PARENT * parent)
: AbstractConfig_TYPE(parent), m_content(content), m_commonUi(new Ui::AbstractConfig())
#if !defined(MOBILE_UI)
  ,
  m_closeButton(0), m_okButton(0)
#endif
  ,
  m_frame(FrameFactory::defaultPanelFrame())
{
#if !defined(MOBILE_UI)
  // close button
  m_closeButton = new StyledButtonItem(tr(" x "), font(),
                                       this); // this, ":/data/button-close.png", ":/data/button-close-hovered.png",
                                              // ":/data/button-close-pressed.png");
  connect(m_closeButton, SIGNAL(clicked()), this, SIGNAL(requestClose()));

  // WIDGET setup (populate contents and set base palette (only) to transparent)
  QWidget * widget = new QWidget();
  m_commonUi->setupUi(widget);
  QPalette pal;
  QColor oldColor = pal.window().color();
  pal.setBrush(QPalette::Window, Qt::transparent);
  widget->setPalette(pal);
  pal.setBrush(QPalette::Window, oldColor);
  m_commonUi->tab->setPalette(pal);
#else
  m_commonUi->setupUi(this);
#endif

  populateFrameList();

  // select the frame
  quint32 frameClass = m_content->frameClass();
  if(frameClass != Frame::NoFrame)
  {
    for(int i = 0; i < m_commonUi->framesList->count(); ++i)
    {
      QListWidgetItem * item = m_commonUi->framesList->item(i);
      if(item->data(Qt::UserRole).toUInt() == frameClass)
      {
        item->setSelected(true);
        break;
      }
    }
  }

  // read other properties
  m_commonUi->reflection->setChecked(m_content->mirrored());
  connect(m_commonUi->front, SIGNAL(clicked()), m_content, SLOT(slotStackFront()));
  connect(m_commonUi->raise, SIGNAL(clicked()), m_content, SLOT(slotStackRaise()));
  connect(m_commonUi->lower, SIGNAL(clicked()), m_content, SLOT(slotStackLower()));
  connect(m_commonUi->back, SIGNAL(clicked()), m_content, SLOT(slotStackBack()));
  connect(m_commonUi->save, SIGNAL(clicked()), m_content, SLOT(slotSaveAs()));
  connect(m_commonUi->background, SIGNAL(clicked()), m_content, SIGNAL(requestBackgrounding()));
  connect(m_commonUi->del, SIGNAL(clicked()), m_content, SIGNAL(requestRemoval()));
  connect(m_commonUi->newFrame, SIGNAL(clicked()), this, SLOT(slotAddFrame()));
  connect(m_commonUi->lookApplyAll, SIGNAL(clicked()), this, SLOT(slotLookApplyAll()));
  connect(m_commonUi->framesList, SIGNAL(itemSelectionChanged()), this, SLOT(slotFrameSelectionChanged()));
  connect(m_commonUi->reflection, SIGNAL(toggled(bool)), this, SLOT(slotReflectionToggled(bool)));

  // ITEM setup
#if !defined(MOBILE_UI)
  setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
  setWidget(widget);
  static qreal s_propZBase = 99999;
  setZValue(s_propZBase++);
#endif

#if !defined(MOBILE_UI) && QT_VERSION >= 0x040600
  // fade in animation
  QPropertyAnimation * ani = new QPropertyAnimation(this, "opacity");
  ani->setEasingCurve(QEasingCurve::OutCubic);
  ani->setDuration(400);
  ani->setStartValue(0.0);
  ani->setEndValue(1.0);
  ani->start(QPropertyAnimation::DeleteWhenStopped);
#endif
}

AbstractConfig::~AbstractConfig()
{
  delete m_frame;
  delete m_commonUi;
}

void AbstractConfig::dispose()
{
#if !defined(MOBILE_UI) && QT_VERSION >= 0x040600
  // fade out animation, then delete
  QPropertyAnimation * ani = new QPropertyAnimation(this, "opacity");
  connect(ani, SIGNAL(finished()), this, SLOT(deleteLater()));
  ani->setEasingCurve(QEasingCurve::OutCubic);
  ani->setDuration(200);
  ani->setEndValue(0.0);
  ani->start(QPropertyAnimation::DeleteWhenStopped);
#else
  // delete this now
  deleteLater();
#endif
}

AbstractContent * AbstractConfig::content() const
{ return m_content; }

void AbstractConfig::keepInBoundaries(const QRectF & rect)
{
#if !defined(MOBILE_UI)
  QRectF r = mapToScene(boundingRect()).boundingRect();
  r.setLeft(qBound(rect.left(), r.left(), rect.right() - r.width()));
  r.setTop(qBound(rect.top(), r.top(), rect.bottom() - r.height()));
  // CHECK SUBPIXELS
  setPos(r.topLeft().toPoint());
#else
  Q_UNUSED(rect);
#endif
}

void AbstractConfig::addTab(QWidget * widget, const QString & label, bool front, bool setCurrent)
{
  // insert on front/back
  int idx = m_commonUi->tab->insertTab(front ? 0 : m_commonUi->tab->count(), widget, label);

  // show if requested
  if(setCurrent) m_commonUi->tab->setCurrentIndex(idx);

  // adjust size after inserting the tab
  if(m_commonUi->tab->parentWidget()) m_commonUi->tab->parentWidget()->adjustSize();
}

void AbstractConfig::showOkButton(bool show)
{
#if !defined(MOBILE_UI)
  if(show)
  {
    if(!m_okButton)
    {
      m_okButton = new StyledButtonItem(tr("ok"), font(), this);
      connect(m_okButton, SIGNAL(clicked()), this, SLOT(slotOkClicked()));
      layoutButtons();
    }
    m_okButton->show();
  }
  else if(m_okButton)
    m_okButton->hide();
#endif
}

#if !defined(MOBILE_UI)
void AbstractConfig::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
  QGraphicsProxyWidget::mousePressEvent(event);
  if(!event->isAccepted() && event->button() == Qt::RightButton) emit requestClose();
  event->accept();
}

void AbstractConfig::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
  QGraphicsProxyWidget::mouseDoubleClickEvent(event);
  event->accept();
}

void AbstractConfig::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
  // speed up svg drawing and unbreak proxy widget
  painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, false);

  // draw cached frame background
  QRect rect = boundingRect().toRect();
  if(m_backPixmap.isNull() || m_backPixmap.size() != rect.size())
  {
    m_backPixmap = QPixmap(rect.size());
    m_backPixmap.fill(Qt::transparent);
    QPainter backPainter(&m_backPixmap);
    if(m_frame) m_frame->drawFrame(&backPainter, m_backPixmap.rect(), false, false);
  }
  painter->drawPixmap(rect.topLeft(), m_backPixmap);

  // unbreak parent
  QGraphicsProxyWidget::paint(painter, option, widget);
}

void AbstractConfig::resizeEvent(QGraphicsSceneResizeEvent * event)
{
  layoutButtons();
  QGraphicsProxyWidget::resizeEvent(event);
}
#endif

void AbstractConfig::populateFrameList()
{
  m_commonUi->framesList->clear();
  // add frame items to the listview
  foreach(quint32 frameClass, FrameFactory::classes())
  {
    // make icon from frame preview
    QIcon icon;
    Frame * frame = FrameFactory::createFrame(frameClass);
    if(!frame)
    {
      // generate the 'empty frame' preview
      QPixmap emptyPixmap(32, 32);
      emptyPixmap.fill(Qt::transparent);
      QPainter pixPainter(&emptyPixmap);
      pixPainter.drawLine(4, 4, 27, 27);
      pixPainter.drawLine(4, 27, 27, 4);
      pixPainter.end();
      icon = emptyPixmap;
    }
    else
    {
      icon = frame->preview(32, 32);
      delete frame;
    }

    // add the item to the list (and attach it the class)
    QListWidgetItem * item = new QListWidgetItem(icon, QString(), m_commonUi->framesList);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setData(Qt::UserRole, frameClass);
  }
}

void AbstractConfig::layoutButtons()
{
#if !defined(MOBILE_UI)
  // layout the close button
  QRect cRect = boundingRect().toRect().adjusted(12, 12, -12, -12);
  if(QApplication::isLeftToRight())
  {
    m_closeButton->setPos(cRect.right() - m_closeButton->boundingRect().width(), cRect.top());
    if(m_okButton) m_okButton->setPos(m_closeButton->pos().x() - m_okButton->boundingRect().width() - 8, cRect.top());
  }
  else
  {
    m_closeButton->setPos(cRect.left(), cRect.top());
    if(m_okButton) m_okButton->setPos(cRect.left() + m_closeButton->boundingRect().width() + 8, cRect.top());
  }
#endif
}

void AbstractConfig::slotAddFrame()
{
  QStringList framesPath = QFileDialog::getOpenFileNames(
      0, tr("Choose frame images"), QString(), tr("Images (*.svg)") /*, 0, QFileDialog::DontResolveSymlinks*/);
  if(!framesPath.isEmpty())
  {
    foreach(QString frame, framesPath) FrameFactory::addSvgFrame(frame);
    populateFrameList();
  }
}

void AbstractConfig::slotRemoveFrame()
{
  QList<QListWidgetItem *> selectedFrames = m_commonUi->framesList->selectedItems();
  foreach(QListWidgetItem * item, selectedFrames) { FrameFactory::removeFrame(item->data(Qt::UserRole).toInt()); }
  populateFrameList();
}

void AbstractConfig::slotLookApplyAll()
{ emit applyLook(m_content->frameClass(), m_content->mirrored(), true); }

void AbstractConfig::slotFrameSelectionChanged()
{
  // get the frameClass
  QList<QListWidgetItem *> items = m_commonUi->framesList->selectedItems();
  if(items.isEmpty()) return;
  QListWidgetItem * item = items.first();
  quint32 frameClass = item->data(Qt::UserRole).toUInt();
  emit applyLook(frameClass, m_content->mirrored(), false);
}

void AbstractConfig::slotReflectionToggled(bool checked)
{
  RenderOpts::LastMirrored = checked;
  emit applyLook(m_content->frameClass(), checked, false);
}
