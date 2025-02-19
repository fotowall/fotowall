/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2007-2009 by Enrico Ros <enrico.ros@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "SceneView.h"

#include "Shared/AbstractScene.h"

#include <QAction>
#include <QApplication>
#include <QCommonStyle>
#include <QPainter>
#include <QPixmap>
#include <QRectF>
#include <QScrollBar>
#include <QStyleOption>
#include <QTimer>
#include <QVBoxLayout>
#include <QWheelEvent>

/// The style used by the SceneView's rubberband selection
class RubberBandStyle : public QCommonStyle
{
public:
  void drawControl(ControlElement element,
                   const QStyleOption * option,
                   QPainter * painter,
                   const QWidget * widget = 0) const
  {
    if(element != CE_RubberBand) return QCommonStyle::drawControl(element, option, painter, widget);
    painter->save();
    // ### Qt WORKAROUND this unbreaks the OpenGL rubberband drawing
    painter->resetTransform();
    QColor color = option->palette.color(QPalette::Highlight);
    painter->setPen(color);
    color.setAlpha(80);
    painter->setBrush(color);
    painter->drawRect(option->rect.adjusted(0, 0, -1, -1));
    painter->restore();
  }
  int styleHint(StyleHint hint,
                const QStyleOption * option,
                const QWidget * widget,
                QStyleHintReturn * returnData) const
  {
    if(hint == SH_RubberBand_Mask) return false;
    return QCommonStyle::styleHint(hint, option, widget, returnData);
  }
};

SceneView::SceneView(QWidget * parent)
: QGraphicsView(parent), m_viewScale(1.0), m_openGL(false), m_abstractScene(0), m_style(0), m_shadowTile(0),
  m_heavyTimer(0), m_heavyCounter(0)
{
  // customize widget
  setInteractive(true);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
  setDragMode(RubberBandDrag);
  setAcceptDrops(true);
  setFrameStyle(QFrame::NoFrame);

  // don't autofill the view with the Base brush
  QPalette pal;
  pal.setBrush(QPalette::Base, Qt::NoBrush);
  setPalette(pal);

  // use own style for drawing the RubberBand
  m_style = new RubberBandStyle;
  viewport()->setStyle(m_style);

  // general actions
  QAction * pasteAction = new QAction(tr("Paste"), this);
  pasteAction->setShortcut(tr("CTRL+V"));
  addAction(pasteAction);
  connect(pasteAction, SIGNAL(triggered()), this, SLOT(slotPasteActionTriggered()));

  // can't activate the cache mode by default, since it inhibits dynamical background picture changing
  // setCacheMode(CacheBackground);
}

SceneView::~SceneView()
{
  delete m_shadowTile;
  delete m_style;
}

void SceneView::setScene(AbstractScene * scene)
{
  if(m_abstractScene == scene) return;

  // disconnect previous scene
  if(m_abstractScene) disconnect(m_abstractScene, 0, this, 0);

  // use scene
  m_abstractScene = scene;
  QGraphicsView::setScene(m_abstractScene);
  if(m_abstractScene)
  {
    connect(m_abstractScene, SIGNAL(destroyed(QObject *)), this, SLOT(slotSceneDestroyed(QObject *)));
    connect(m_abstractScene, SIGNAL(sceneSizeChanged()), this, SLOT(slotLayoutScene()));
    connect(m_abstractScene, SIGNAL(scenePerspectiveChanged()), this, SLOT(slotUpdateViewTransform()));
    connect(m_abstractScene, SIGNAL(sceneRotationChanged()), this, SLOT(slotUpdateViewTransform()));
    setViewScale(1.0);
    slotLayoutScene();
  }
}

AbstractScene * SceneView::scene() const
{
  return m_abstractScene;
}

AbstractScene * SceneView::takeScene()
{
  AbstractScene * m_scene = m_abstractScene;
  setScene(0);
  return m_scene;
}

bool SceneView::supportsOpenGL() const
{
#ifdef QT_OPENGL_LIB
  return true;
#else
  return false;
#endif
}

bool SceneView::openGL() const
{
  return m_openGL;
}

#ifdef QT_OPENGL_LIB
#  include <QOpenGLWidget>
void SceneView::setOpenGL(bool enabled)
{
  // skip if already ok
  if(m_openGL == enabled) return;
  m_openGL = enabled;

  // change viewport widget and transfer style
  QWidget * newViewport = m_openGL ? new QOpenGLWidget() : new QWidget();
  newViewport->setStyle(m_style);
  setViewport(newViewport);
  setViewportUpdateMode(m_openGL ? FullViewportUpdate : MinimalViewportUpdate);

  // transparent background for raster, standard Base on opengl
  QPalette pal = qApp->palette();
#  if QT_VERSION < 0x040600
  // WORKAROUND Qt <= 4.6-beta1
  if(m_openGL)
    pal.setBrush(QPalette::Base, pal.window());
  else
#  endif
    pal.setBrush(QPalette::Base, Qt::NoBrush);
  setPalette(pal);

  // issue an update, just in case..
  update();
}
#else
void SceneView::setOpenGL(bool) {};
#endif

qreal SceneView::viewScale() const
{
  return m_viewScale;
}

void SceneView::setViewScale(qreal scale)
{
  if(m_viewScale != scale)
  {
    m_viewScale = scale;
    if(m_viewScale > 0.98 && m_viewScale < 1.02) m_viewScale = 1.0;
    slotUpdateViewTransform();
    emit viewScaleChanged();
  }
}

static void drawVerticalShadow(QPainter * painter, int width, int height)
{
  QLinearGradient lg(0, 0, 0, height);
  lg.setColorAt(0.0, QColor(0, 0, 0, 64));
  lg.setColorAt(0.4, QColor(0, 0, 0, 16));
  lg.setColorAt(0.7, QColor(0, 0, 0, 5));
  lg.setColorAt(1.0, QColor(0, 0, 0, 0));
  painter->fillRect(0, 0, width, height, lg);
}

void SceneView::drawForeground(QPainter * painter, const QRectF & rect)
{
  // base impl: draw Scene's foreground
  QGraphicsView::drawForeground(painter, rect);

  // the first time create the Shadow Tile
  if(!m_shadowTile)
  {
    m_shadowTile = new QPixmap(64, 8);
    m_shadowTile->fill(Qt::transparent);
    QPainter shadowPainter(m_shadowTile);
    drawVerticalShadow(&shadowPainter, 64, 8);
  }

  // if scaled, draw untransformed (full shadow tile + zoom)
  if(!transform().isIdentity())
  {
    // draw untransformed (we're drawing to the viewport())
    painter->resetTransform();

    // draw shadow
    QRect viewportRect = viewport()->contentsRect();
    int viewportHeight = viewportRect.height();
    viewportRect.setHeight(8);
    painter->drawTiledPixmap(viewportRect, *m_shadowTile);

    // draw text
    QString text = tr("%1%").arg(qRound(m_viewScale * 100));
    QRect textRect = QFontMetrics(painter->font()).boundingRect(text).adjusted(-2, -1, 2, 1);
    if(QApplication::isLeftToRight())
      textRect.moveBottomLeft(QPoint(5, viewportHeight - 5));
    else
      textRect.moveBottomRight(QPoint(viewportRect.width() - 5, viewportHeight - 5));
    painter->fillRect(textRect, palette().color(QPalette::Highlight));
    painter->setPen(palette().color(QPalette::HighlightedText));
    painter->drawText(textRect, Qt::AlignCenter, text);
    return;
  }

  // find out if we have a drawing offset (we draw in Scene coords, and scene may be translated)
  int y = mapToScene(0, 0).y();

  // blend the shadow tile
  if(rect.top() < (y + 8)) painter->drawTiledPixmap(rect.left(), y, rect.width(), 8, *m_shadowTile);
}

void SceneView::paintEvent(QPaintEvent * event)
{
  // start the measuring time
  const bool measureTime = true;
  if(measureTime) m_paintTime.start();

  // do painting
  QGraphicsView::paintEvent(event);

  // handle measurement
  if(!measureTime) return;
  if(m_paintTime.elapsed() < 100)
  {
    m_heavyCounter = 0;
    return;
  }

  // handle slow painting
  if(++m_heavyCounter > 6)
  {
    m_heavyCounter = -100;
    if(!m_heavyTimer)
    {
      m_heavyTimer = new QTimer(this);
      m_heavyTimer->setSingleShot(true);
      connect(m_heavyTimer, SIGNAL(timeout()), this, SIGNAL(heavyRepaint()));
    }
  }
  if(m_heavyTimer) m_heavyTimer->start(1000);
}

void SceneView::resizeEvent(QResizeEvent * event)
{
  QGraphicsView::resizeEvent(event);
  slotLayoutScene();
}

void SceneView::wheelEvent(QWheelEvent * event)
{
  if(event->modifiers() == Qt::ControlModifier && m_abstractScene && m_abstractScene->sceneSelectable())
  {
    if(event->angleDelta().y() < 0 && m_viewScale > 0.1)
      setViewScale(m_viewScale * 0.707106781187);
    else if(event->angleDelta().y() > 0 && m_viewScale < 15)
      setViewScale(m_viewScale * 1.41421356237);
    event->accept();
    return;
  }
  QGraphicsView::wheelEvent(event);
}

void SceneView::slotLayoutScene()
{
  if(!m_abstractScene) return;

  // change size
  QSize viewportSize = viewport()->contentsRect().size();
  m_abstractScene->resize(viewportSize);

  // use scrollbars if scene screen size is bigger than viewport's
  QSize sceneSize = m_abstractScene->sceneSize();
  if(!transform().isIdentity()) sceneSize = transform().mapRect(m_abstractScene->sceneRect()).size().toSize();
  bool scrollbarsNeeded = (sceneSize.width() > viewportSize.width()) || (sceneSize.height() > viewportSize.height());
  Qt::ScrollBarPolicy sPolicy = scrollbarsNeeded ? Qt::ScrollBarAlwaysOn : Qt::ScrollBarAlwaysOff;
  setVerticalScrollBarPolicy(sPolicy);
  setHorizontalScrollBarPolicy(sPolicy);

  // change the selection/scrolling policy
  if(m_abstractScene->sceneSelectable())
    setDragMode(RubberBandDrag);
  else
    setDragMode(scrollbarsNeeded ? ScrollHandDrag : NoDrag);

  // update screen
  update();
}

void SceneView::slotUpdateViewTransform()
{
  QTransform viewTransform;
  if(m_viewScale != 1.0) viewTransform.scale(m_viewScale, m_viewScale);
  if(m_abstractScene)
  {
    const QPointF perspective = m_abstractScene->perspective();
    const qreal rotation = m_abstractScene->rotation();
    QPointF offset = m_abstractScene->sceneCenter();
    viewTransform.translate(offset.x(), offset.y());
    if(rotation) viewTransform.rotate(rotation, Qt::ZAxis);
    if(!perspective.isNull())
    {
      viewTransform.rotate(perspective.y(), Qt::XAxis);
      viewTransform.rotate(perspective.x(), Qt::YAxis);
    }
    viewTransform.translate(-offset.x(), -offset.y());
  }
  setTransform(viewTransform, false);
  slotLayoutScene();
}

void SceneView::slotPasteActionTriggered()
{
  if(m_abstractScene) m_abstractScene->pasteFromClipboard();
}

void SceneView::slotSceneDestroyed(QObject * object)
{
  // if there is a scene and it's being deleted, don't reference to it anymore
  if(m_abstractScene && static_cast<AbstractScene *>(object) == m_abstractScene) m_abstractScene = 0;
}
