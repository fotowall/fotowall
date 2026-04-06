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

#include "HomeScene.h"

#include "3rdparty/pencil/PencilItem.h"
#include "App.h"
#include "Canvas/AbstractContent.h"
#include "Frames/StandardFrame.h"
#include "Settings.h"
#include "Shared/Compat.h"
#include "Shared/RenderOpts.h"

#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QTimer>

#if QT_VERSION >= 0x040600
#  include <QPropertyAnimation>
#  define ANIMATE_PARAM(object, propName, duration, endValue)                    \
    QPropertyAnimation * ani = new QPropertyAnimation(object, propName, object); \
    ani->setEasingCurve(QEasingCurve::OutBack);                                  \
    ani->setDuration(duration);                                                  \
    ani->setEndValue(endValue);                                                  \
    ani->start(QPropertyAnimation::DeleteWhenStopped);
#else
#  define ANIMATE_PARAM(instance, propName, duration, endValue) instance->setProperty(propName, endValue);
#endif

// uncomment following to draw a subtle shadow behind the labels
#define HOMELABEL_SHADOWED

/** Home Label **/

class HomeLabel : public AbstractContent
{
public:
  HomeLabel(const QString & title, const QPixmap & pixmap, QGraphicsScene * scene)
  : AbstractContent(scene, false, true, 0), m_pixmap(pixmap)
  {
    // create the standard frame and set title
    setFrame(new StandardFrame2);
    setFrameTextEnabled(true);
    setFrameTextReadonly(true);
    setFrameText(title);
    setMirrored(false);

    // incremental change over AbstractContent
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    const int pixW = pixmap.width();
    const int pixH = pixmap.height();
    resizeContents(QRect(-pixW / 2, -pixH / 2, pixW, pixH), false);
  }

  QString contentName() const { return "HomeLabel"; }

#ifdef HOMELABEL_SHADOWED
  QRectF boundingRect() const
  {
    const QRectF origRect = AbstractContent::boundingRect();
    return origRect.adjusted(0, 0, 0, origRect.height() * 0.2);
  }
#endif

  void focusInEvent(QFocusEvent * event)
  {
    event->accept();
    hoverEnterEvent(0);
  }

  void focusOutEvent(QFocusEvent * event)
  {
    event->accept();
    hoverLeaveEvent(0);
  }

  void hoverEnterEvent(QGraphicsSceneHoverEvent *)
  {
#if !defined(MOBILE_UI)
    ANIMATE_PARAM(this, "scale", 300, 1.2);
    // ANIMATE_PARAM(this, "rotation", 300, (-20 + (compat::qrand() % 41)));
#endif
  }

  void hoverLeaveEvent(QGraphicsSceneHoverEvent *)
  {
#if !defined(MOBILE_UI)
    ANIMATE_PARAM(this, "scale", 300, 1.0);
    // ANIMATE_PARAM(this, "rotation", 300, 0.0);
#endif
  }

  void mousePressEvent(QGraphicsSceneMouseEvent * event)
  {
    // use an already existing signal.. FIXME!
    if(event->button() == Qt::LeftButton)
    {
#if defined(MOBILE_UI)
      ANIMATE_PARAM(this, "scale", 200, 0.8);
      connect(ani, SIGNAL(finished()), this, SIGNAL(requestEditing()));
#else
      requestEditing();
#endif
    }
  }

  void drawContent(QPainter * painter, const QRect & targetRect, Qt::AspectRatioMode /*ratio*/)
  {
    painter->setRenderHint(QPainter::SmoothPixmapTransform, property("scale").toDouble() == 1.2);
    painter->drawPixmap(targetRect, m_pixmap);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
  }

  void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
  {
    // draw a 'shadow-like' gradient
#ifdef HOMELABEL_SHADOWED
    QRadialGradient rg(0.5, 0.5, 0.5);
    rg.setCoordinateMode(QGradient::ObjectBoundingMode);
    int opacity = (int)(24.0 * (1.0 - 3 * (property("scale").toDouble() - 1.0)));
    rg.setColorAt(0.0, QColor(32, 32, 32, opacity));
    rg.setColorAt(1.0, Qt::transparent);
    painter->setBrush(rg);
    painter->setPen(Qt::NoPen);
    QRect shadowRect = boundingRect().toRect();
    shadowRect.setTop(boundingRect().bottom() - 2 * 8);
    painter->drawRect(shadowRect);
#endif

    // paint the AbstractContent
    AbstractContent::paint(painter, option, widget);
  }

private:
  QPixmap m_pixmap;
};

HomeScene::HomeScene(QObject * parent)
: AbstractScene(parent), m_logoPixmap(":/data/home-logo.png")
#if defined(HAVE_PENCIL_ITEM)
  ,
  m_pencil(0)
#endif
{
  // create the 3 buttons
#if defined(HAS_WORDCLOUD_APPLIANCE)
  HomeLabel * newWordcloud = new HomeLabel(tr("Wordcloud"), QPixmap(":/data/home-newwordcloud.png"), this);
#  if 0
     connect(newWordcloud, SIGNAL(requestEditing()), this, SIGNAL(startWordcloud()));
#  else
  // NOTE: this is not ready for 1.0, so disable it
  newWordcloud->setEnabled(false);
  newWordcloud->setContentOpacity(0.2);
#  endif
#else
  HomeLabel * newWordcloud = new HomeLabel(tr("coming soon"), QPixmap(":/data/home-newwordcloud.png"), this);
  newWordcloud->setEnabled(false);
  newWordcloud->setContentOpacity(0.2);
#endif
  newWordcloud->setZValue(0.0);
  m_labels.append(newWordcloud);

  HomeLabel * newCanvas = new HomeLabel(tr("Create"), QPixmap(":/data/home-newcanvas.png"), this);
  connect(newCanvas, SIGNAL(requestEditing()), this, SLOT(slotStartCanvas()));
  newCanvas->setZValue(1.0);
  m_labels.append(newCanvas);

  HomeLabel * wizard = new HomeLabel(tr("coming soon"), QPixmap(":/data/home-wizard.png"), this);
  connect(wizard, SIGNAL(requestEditing()), this, SIGNAL(startWizard()));
  wizard->setEnabled(false);
  wizard->setContentOpacity(0.2);
  wizard->setZValue(0.0);
  m_labels.append(wizard);

#if defined(HAVE_PENCIL_ITEM)
  // create the pencil item the first time
  if(App::settings->firstTime() || !(compat::qrand() % 5)) QTimer::singleShot(1000, this, SLOT(slotCreatePencil()));
#endif
}

HomeScene::~HomeScene()
{ qDeleteAll(m_labels); }

void HomeScene::drawBackground(QPainter * painter, const QRectF & rect)
{
  // draw a gray shade as background
  painter->setCompositionMode(QPainter::CompositionMode_Source);
  QLinearGradient lg(0, 0, 0, sceneHeight());
  lg.setColorAt(0.0, QColor(192, 192, 192, RenderOpts::ARGBWindow ? 200 : 255));
  lg.setColorAt(1.0, QColor(128, 128, 128, RenderOpts::ARGBWindow ? 200 : 255));
  painter->fillRect(rect, lg);
  painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
#if 0
    QRadialGradient rg(sceneWidth() / 2, 0, 200, sceneWidth() / 2, -100);
    rg.setColorAt(0.0, Qt::white);
    rg.setColorAt(1.0, Qt::transparent);
    painter->fillRect(rect, rg);
#endif
}

void HomeScene::drawForeground(QPainter * painter, const QRectF & rect)
{
  // draw top-center logo
  if(m_logoRect.isValid() && m_logoRect.intersects(rect.toRect()))
    painter->drawPixmap(m_logoRect.topLeft(), m_logoPixmap);
}

void HomeScene::keyPressEvent(QKeyEvent * event)
{
  AbstractScene::keyPressEvent(event);
  emit keyPressed(event->key());
}

void HomeScene::resize(const QSize & size)
{
  // resize but ensure a minimum size
#if !defined(MOBILE_UI)
  AbstractScene::resize(size.expandedTo(QSize(600, 200)));
#else
  AbstractScene::resize(size);
#endif

  // grid placement
  const int margin = 20;
  const int count = m_labels.size();
  const int width = sceneWidth() - margin * 2;
#if !defined(MOBILE_UI)
  const int height = sceneHeight() - margin;
#else
  const int height = sceneHeight() + 16;
#endif
  const int rows = (int)sqrt((qreal)count);
  const int cols = (int)ceil((qreal)count / (qreal)rows);
  int rIdx = 0, cIdx = 0;
  for(int i = 0; i < count; i++)
  {
    double xPos = margin + (int)(((qreal)cIdx + 0.50) * (qreal)width / (qreal)cols);
    double yPos = margin + (int)(((qreal)rIdx + 0.55) * (qreal)height / (qreal)rows);
    if(cols == 3 && cIdx != 1) yPos += 4;
    m_labels[i]->setPos(QPointF(xPos, yPos) - m_labels[i]->boundingRect().center());
    if(++cIdx >= cols)
    {
      cIdx = 0;
      ++rIdx;
    }
  }

  // logo rectangle
  if(!m_logoPixmap.isNull())
  {
    int top = 0;
    if(m_labels.size() < 2)
      top = (height - rows * 200) / 10;
    else
      top = qMax((qreal)0, (m_labels[1]->sceneBoundingRect().top() - m_logoPixmap.height()) / 2);
    m_logoRect = QRect((sceneWidth() - m_logoPixmap.width()) / 2, top, m_logoPixmap.width(), m_logoPixmap.height());
  }

  // pencil item
#if defined(HAVE_PENCIL_ITEM)
  if(m_pencil)
  {
    QRectF pRect = m_pencil->boundingRect();
    m_pencil->setPos(QPointF((sceneWidth() - pRect.width()) / 2, 9 * (sceneHeight() - pRect.height()) / 10));
  }
#endif
}

bool HomeScene::sceneSelectable() const
{ return false; }

void HomeScene::slotCreatePencil()
{
#if defined(HAVE_PENCIL_ITEM)
  if(!m_pencil)
  {
    m_pencil = new PencilItem(":/data/home-art.svg");
    QRectF pRect = m_pencil->boundingRect();
    m_pencil->setZValue(999);
    m_pencil->setPos(QPointF((sceneWidth() - pRect.width()) / 2, 9 * (sceneHeight() - pRect.height()) / 10));
    addItem(m_pencil);
  }
#endif
}

void HomeScene::slotStartCanvas()
{
  emit startCanvas();

  // reset the scale of the pressed button
  sender()->setProperty("scale", (qreal)1.0);
}
