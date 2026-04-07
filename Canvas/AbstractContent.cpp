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

#include "AbstractContent.h"

#include "Shared/Commands.h"

#include "ButtonItem.h"
#include "ContentProperties.h"
#include "CornerItem.h"
#include "Frames/FrameFactory.h"
#include "MirrorItem.h"
#include "Shared/RenderOpts.h"

#include <QApplication>
#include <QDate>
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsTextItem>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPainter>
#include <QSettings>
#include <QTimer>
#include <QUrl>
#include <math.h>
#if QT_VERSION >= 0x040600
#  include <QGraphicsBlurEffect>
#  include <QGraphicsDropShadowEffect>
#endif

#include "Canvas/Canvas.h" // for CommandStack helpers

bool do_canvas_command(QObject * maybeCanvas, QUndoCommand * command)
{
  if(maybeCanvas == nullptr)
  {
    qDebug() << "Failed to add the do command " << command->text() << " to the command stack: parent is null";
    command->redo();
  }

  if(auto * canvas = dynamic_cast<Canvas *>(maybeCanvas); canvas != nullptr)
  {
    ;
    canvas->commandStack().push(command);
    return true;
  }
  else
  {
    qDebug() << "Failed to add command " << command->text()
             << " to the command stack: element is not a Canvas and does not have a command stack";
    command->redo();
    return false;
  }
}

AbstractContent::AbstractContent(QGraphicsScene * scene, bool fadeIn, bool noRescale, QGraphicsItem * parent)
: AbstractDisposeable(fadeIn, parent), m_scene(scene), m_contentRect(-100, -75, 200, 150), m_frame(0),
  m_frameTextItem(0), m_controlsVisible(false), m_dirtyTransforming(false), m_transformRefreshTimer(0),
  m_gfxChangeTimer(0), m_mirrorItem(0)
#if QT_VERSION < 0x040600
  ,
  m_rotationAngle(0)
#endif
  ,
  m_fxIndex(0)
{
  // the buffered graphics changes timer
  m_gfxChangeTimer = new QTimer(this);
  m_gfxChangeTimer->setInterval(0);
  m_gfxChangeTimer->setSingleShot(true);

  // customize item's behavior
  setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsSelectable);
#if QT_VERSION >= 0x040600
  setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
#endif
  // allow some items (eg. the shape controls for text) to be shown
  setFlag(QGraphicsItem::ItemClipsChildrenToShape, false);
  setAcceptHoverEvents(true);

  // create child controls
  createCorner(Qt::TopLeftCorner, noRescale);
  createCorner(Qt::TopRightCorner, noRescale);
  createCorner(Qt::BottomLeftCorner, noRescale);
  createCorner(Qt::BottomRightCorner, noRescale);

  // ButtonItem * bFront = new ButtonItem(ButtonItem::Control, Qt::blue, QIcon(":/data/action-order-front.png"),
  // this); bFront->setToolTip(tr("Raise")); connect(bFront, SIGNAL(clicked()), this, SLOT(slotStackRaise()));
  // addButtonItem(bFront);

  ButtonItem * bConf = new ButtonItem(ButtonItem::Control, Qt::green, QIcon(":/data/action-configure.png"), this);
  bConf->setToolTip(tr("Change properties..."));
  connect(bConf, SIGNAL(clicked()), this, SLOT(slotConfigure()));
  addButtonItem(bConf);

  ButtonItem * bPersp = new ButtonItem(ButtonItem::Control, Qt::red, QIcon(":/data/action-perspective.png"), this);
  bPersp->setToolTip(tr(
      "Drag around to change the perspective.\nHold SHIFT to move faster.\nUse CTRL to cancel the transformations."));
  connect(bPersp, SIGNAL(dragging(const QPointF &, Qt::KeyboardModifiers)), this,
          SLOT(slotSetPerspective(const QPointF &, Qt::KeyboardModifiers)));
  connect(bPersp, SIGNAL(releaseEvent(QGraphicsSceneMouseEvent *)), this,
          SLOT(slotReleasePerspectiveButton(QGraphicsSceneMouseEvent *)));
  connect(bPersp, SIGNAL(doubleClicked()), this, SLOT(slotClearPerspective()));
  addButtonItem(bPersp);

  ButtonItem * bDelete = new ButtonItem(ButtonItem::Control, Qt::red, QIcon(":/data/action-delete.png"), this);
  bDelete->setSelectsParent(false);
  bDelete->setToolTip(tr("Remove"));
  connect(bDelete, SIGNAL(clicked()), this, SIGNAL(requestRemoval()));
  addButtonItem(bDelete);

  // create default frame
  Frame * frame = FrameFactory::defaultPictureFrame();
  setFrame(frame);

  // hide and layoutChildren buttons
  layoutChildren();

  // add to the scene
  scene->addItem(this);

  // display mirror
#if QT_VERSION >= 0x040600
  // WORKAROUND with Qt 4.6-tp1 there are crashes activating a mirror before setting the scene
  // need to rethink this anyway
  setMirrored(false);
#else
  setMirrored(RenderOpts::LastMirrored);
#endif
}

AbstractContent::~AbstractContent()
{
  qDeleteAll(m_cornerItems);
  qDeleteAll(m_controlItems);
  delete m_mirrorItem;
  delete m_frameTextItem;
  delete m_frame;
}

void AbstractContent::dispose()
{
  // stick this item
  setFlags((GraphicsItemFlags)0x00);

  // fade out mirror too
  setMirrored(false);

  // unselect if selected
  setSelected(false);

  // pre-remove controls
  qDeleteAll(m_cornerItems);
  m_cornerItems.clear();
  qDeleteAll(m_controlItems);
  m_controlItems.clear();
  m_controlsVisible = false;

  // little rotate animation
#if !defined(MOBILE_UI) && QT_VERSION >= 0x040600
  QPropertyAnimation * ani = new QPropertyAnimation(this, "rotation");
  ani->setEasingCurve(QEasingCurve::InQuad);
  ani->setDuration(300);
  ani->setEndValue(-30.0);
  ani->start(QPropertyAnimation::DeleteWhenStopped);
#endif

  // standard disposition
  AbstractDisposeable::dispose();
}

QRect AbstractContent::contentRect() const
{ return m_contentRect; }

void AbstractContent::resizeContents(const QRect & rect, bool keepRatio)
{
  if(!rect.isValid()) return;

  prepareGeometryChange();

  m_contentRect = rect;
  if(keepRatio)
  {
    int hfw = contentHeightForWidth(rect.width());
    if(hfw > 1)
    {
      m_contentRect.setTop(-hfw / 2);
      m_contentRect.setHeight(hfw);
    }
  }

  m_frameRect = m_frame ? m_frame->frameRect(m_contentRect) : m_contentRect;

  layoutChildren();
  update();
  GFX_CHANGED();
}

void AbstractContent::resetContentsRatio()
{ resizeContents(m_contentRect, true); }

void AbstractContent::delayedDirty(int ms)
{
  // tell rendering that we're changing stuff
  m_dirtyTransforming = true;

  // start refresh timer
  if(!m_transformRefreshTimer)
  {
    m_transformRefreshTimer = new QTimer(this);
    connect(m_transformRefreshTimer, SIGNAL(timeout()), this, SLOT(slotDirtyEnded()));
    m_transformRefreshTimer->setSingleShot(true);
  }
  m_transformRefreshTimer->start(ms);
}

QPointF AbstractContent::previousPos() const
{ return m_previousPos; }

void AbstractContent::setPreviousPos(const QPointF & previousPos)
{ m_previousPos = previousPos; }

void AbstractContent::setPosUndo(const QPointF & pos)
{
  m_previousPos = this->pos();
  do_canvas_command(scene(), new MotionCommand(this, m_previousPos, pos));
}

void AbstractContent::setFrame(Frame * frame)
{
  delete m_frame;
  m_frame = frame;
  FrameFactory::setDefaultPictureClass(frameClass());
  if(!m_frame && m_frameTextItem) m_frameTextItem->hide();
  resizeContents(m_contentRect);
  layoutChildren();
  update();
  GFX_CHANGED();
}

quint32 AbstractContent::frameClass() const
{ return m_frame ? m_frame->frameClass() : (quint32)Frame::NoFrame; }

class FrameTextItem : public QGraphicsTextItem
{
public:
  FrameTextItem(AbstractContent * content) : QGraphicsTextItem(content), m_content(content) {}

  void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0)
  {
    painter->setOpacity(m_content->contentOpacity());
    QGraphicsTextItem::paint(painter, option, widget);
  }

  // prevent the TextItem from listening to global shortcuts
  bool eventFilter(QObject * object, QEvent * event)
  {
    if(event->type() == QEvent::Shortcut || event->type() == QEvent::ShortcutOverride)
    {
      if(!object->inherits("QGraphicsView"))
      {
        event->accept();
        return true;
      }
    }
    return false;
  }

protected:
  void focusInEvent(QFocusEvent * event)
  {
    QGraphicsTextItem::focusInEvent(event);
    qApp->installEventFilter(this);
  }

  void focusOutEvent(QFocusEvent * event)
  {
    QGraphicsTextItem::focusOutEvent(event);
    qApp->removeEventFilter(this);
  }

private:
  AbstractContent * m_content;
};

void AbstractContent::setFrameTextEnabled(bool enabled)
{
  // create the Text Item, if enabled...
  if(enabled && !m_frameTextItem)
  {
    m_frameTextItem = new FrameTextItem(this);
    m_frameTextItem->setTextInteractionFlags(Qt::TextEditorInteraction);
    QFont f("Sans Serif");
    // f.setPointSizeF(7.5);
    m_frameTextItem->setFont(f);
    m_frameTextItem->setZValue(1.0);
    layoutChildren();
  }

  // ...or destroy it if disabled
  else if(!enabled && m_frameTextItem)
  {
    delete m_frameTextItem;
    m_frameTextItem = 0;
  }
}

bool AbstractContent::frameTextEnabled() const
{ return m_frameTextItem; }

void AbstractContent::setFrameTextReadonly(bool read)
{
  if(m_frameTextItem)
    m_frameTextItem->setTextInteractionFlags(read ? Qt::NoTextInteraction : Qt::TextEditorInteraction);
}

bool AbstractContent::frameTextReadonly() const
{
  if(!m_frameTextItem) return false;
  return m_frameTextItem->textInteractionFlags() == Qt::NoTextInteraction;
}

void AbstractContent::setFrameText(const QString & text)
{
  if(!m_frameTextItem) return;
  m_frameTextItem->setPlainText(text);
}

QString AbstractContent::frameText() const
{
  if(!m_frameTextItem) return QString();
  return m_frameTextItem->toPlainText();
}

void AbstractContent::addButtonItem(ButtonItem * button)
{
  m_controlItems.append(button);
  button->setVisible(m_controlsVisible);
  button->setZValue(3.0);
  layoutChildren();
}

void AbstractContent::setMirrored(bool enabled)
{
  if(m_mirrorItem && !enabled)
  {
    m_mirrorItem->dispose();
    m_mirrorItem = 0;
    emit mirroredChanged();
  }
  if(enabled && !m_mirrorItem)
  {
    m_mirrorItem = new MirrorItem(this);
    connect(m_gfxChangeTimer, SIGNAL(timeout()), m_mirrorItem, SLOT(sourceChanged()));
    connect(this, SIGNAL(destroyed()), m_mirrorItem, SLOT(deleteLater()));
    emit mirroredChanged();
  }
}

bool AbstractContent::mirrored() const
{ return m_mirrorItem; }

void AbstractContent::setPerspective(const QPointF & angles)
{
  if(angles != m_perspectiveAngles)
  {
    m_perspectiveAngles = angles;
    applyTransforms();
    emit perspectiveChanged();
  }
}

QPointF AbstractContent::perspective() const
{ return m_perspectiveAngles; }

#if QT_VERSION < 0x040600
void AbstractContent::setRotation(qreal angle)
{
  if(m_fixedRotation) return;
  if(m_rotationAngle != angle)
  {
    m_rotationAngle = angle;
    applyTransforms();
    emit rotationChanged();
  }
}

qreal AbstractContent::rotation() const
{ return m_rotationAngle; }
#endif

void AbstractContent::setFxIndex_(int index)
{
  if(m_fxIndex == index) return;
  do_canvas_command(scene(), new FxCommand(this, m_fxIndex, index));
}

void AbstractContent::setFxIndex(int index)
{
  m_fxIndex = index;
  // apply graphics effect
#if QT_VERSION >= 0x040600
  switch(m_fxIndex)
  {
    default:
      setGraphicsEffect(0);
      break;
    case 1:
    {
      QGraphicsDropShadowEffect * ds = new QGraphicsDropShadowEffect(this);
      ds->setColor(Qt::black);
      ds->setBlurRadius(7);
      ds->setOffset(1, 1);
      setGraphicsEffect(ds);
    }
    break;
    case 2:
    {
      QGraphicsDropShadowEffect * ds = new QGraphicsDropShadowEffect(this);
      ds->setColor(Qt::white);
      ds->setBlurRadius(7);
      ds->setOffset(1, 1);
      setGraphicsEffect(ds);
    }
    break;
    case 3:
    {
      QGraphicsBlurEffect * b = new QGraphicsBlurEffect(this);
      b->setBlurRadius(5);
      b->setBlurHints(QGraphicsBlurEffect::QualityHint);
      setGraphicsEffect(b);
    }
    break;
    case 4:
    {
      QGraphicsBlurEffect * b = new QGraphicsBlurEffect(this);
      b->setBlurRadius(16);
      b->setBlurHints(QGraphicsBlurEffect::QualityHint);
      setGraphicsEffect(b);
    }
    break;
  }
#endif
  emit fxIndexChanged();
}

int AbstractContent::fxIndex() const
{ return m_fxIndex; }

void AbstractContent::ensureVisible(const QRectF & rect)
{
  m_previousPos = pos();
  // keep the center inside the scene rect
  QPointF center = pos();
  if(!rect.contains(center))
  {
    center.setX(qBound(rect.left(), center.x(), rect.right()));
    center.setY(qBound(rect.top(), center.y(), rect.bottom()));
    setPos(center);
  }
}

bool AbstractContent::beingTransformed() const
{ return m_dirtyTransforming; }

bool AbstractContent::fromXml(const QDomElement & contentElement, const QDir & /*baseDir*/)
{
  // restore content properties
  QDomElement domElement;

  // Load image size saved in the rect node
  domElement = contentElement.firstChildElement("rect");
  qreal x, y, w, h;
  x = domElement.firstChildElement("x").text().toDouble();
  y = domElement.firstChildElement("y").text().toDouble();
  w = domElement.firstChildElement("w").text().toDouble();
  h = domElement.firstChildElement("h").text().toDouble();
  resizeContents(QRect(x, y, w, h));

  // Load position coordinates
  domElement = contentElement.firstChildElement("pos");
  x = domElement.firstChildElement("x").text().toDouble();
  y = domElement.firstChildElement("y").text().toDouble();
  setPos(x, y);

  qreal zvalue = contentElement.firstChildElement("zvalue").text().toDouble();
  setZValue(zvalue);

  bool visible = contentElement.firstChildElement("visible").text().toInt();
  setVisible(visible);

  qreal opacity = contentElement.firstChildElement("opacity").text().toDouble();
  if(opacity > 0.0 && opacity < 1.0) setContentOpacity(opacity);

  int fxIdx = contentElement.firstChildElement("fxindex").text().toInt();
  if(fxIdx > 0) setFxIndex(fxIdx);

  bool hasText = contentElement.firstChildElement("frame-text-enabled").text().toInt();
  setFrameTextEnabled(hasText);
  if(hasText)
  {
    QString text = contentElement.firstChildElement("frame-text").text();
    setFrameText(text);
  }

  quint32 frameClass = contentElement.firstChildElement("frame-class").text().toInt();
  setFrame(frameClass ? FrameFactory::createFrame(frameClass) : 0);

  // restore transformation
  QDomElement te = contentElement.firstChildElement("transformation");
  if(!te.isNull())
  {
    m_perspectiveAngles = QPointF(te.attribute("xRot").toDouble(), te.attribute("yRot").toDouble());
#if QT_VERSION < 0x040600
    m_rotationAngle = te.attribute("zRot").toDouble();
#else
    setRotation(te.attribute("zRot").toDouble());
#endif
    applyTransforms();
  }
  domElement = contentElement.firstChildElement("mirror");
  setMirrored(domElement.attribute("state").toInt());

  return true;
}

void AbstractContent::toXml(QDomElement & contentElement, const QDir & /*baseDir*/) const
{
  // Save general item properties
  contentElement.setTagName("abstract");
  QDomDocument doc = contentElement.ownerDocument();
  QDomElement domElement;
  QDomText text;
  QString valueStr;

  // Save item position and size
  QDomElement rectParent = doc.createElement("rect");
  QDomElement xElement = doc.createElement("x");
  rectParent.appendChild(xElement);
  QDomElement yElement = doc.createElement("y");
  rectParent.appendChild(yElement);
  QDomElement wElement = doc.createElement("w");
  rectParent.appendChild(wElement);
  QDomElement hElement = doc.createElement("h");
  rectParent.appendChild(hElement);

  QRectF rect = m_contentRect;
  xElement.appendChild(doc.createTextNode(QString::number(rect.left())));
  yElement.appendChild(doc.createTextNode(QString::number(rect.top())));
  wElement.appendChild(doc.createTextNode(QString::number(rect.width())));
  hElement.appendChild(doc.createTextNode(QString::number(rect.height())));
  contentElement.appendChild(rectParent);

  // Save the position
  domElement = doc.createElement("pos");
  xElement = doc.createElement("x");
  yElement = doc.createElement("y");
  valueStr.setNum(pos().x());
  xElement.appendChild(doc.createTextNode(valueStr));
  valueStr.setNum(pos().y());
  yElement.appendChild(doc.createTextNode(valueStr));
  domElement.appendChild(xElement);
  domElement.appendChild(yElement);
  contentElement.appendChild(domElement);

  // Save the stacking position
  domElement = doc.createElement("zvalue");
  contentElement.appendChild(domElement);
  valueStr.setNum(zValue());
  text = doc.createTextNode(valueStr);
  domElement.appendChild(text);

  // Save the visible state
  domElement = doc.createElement("visible");
  contentElement.appendChild(domElement);
  valueStr.setNum(isVisible());
  text = doc.createTextNode(valueStr);
  domElement.appendChild(text);

  // Save the opacity
  if(contentOpacity() < 1.0)
  {
    domElement = doc.createElement("opacity");
    contentElement.appendChild(domElement);
    domElement.appendChild(doc.createTextNode(QString::number(contentOpacity())));
  }

  // Save the Fx Index
  if(fxIndex() > 0)
  {
    domElement = doc.createElement("fxindex");
    contentElement.appendChild(domElement);
    domElement.appendChild(doc.createTextNode(QString::number(fxIndex())));
  }

  // Save the frame class
  valueStr.setNum(frameClass());
  domElement = doc.createElement("frame-class");
  contentElement.appendChild(domElement);
  text = doc.createTextNode(valueStr);
  domElement.appendChild(text);

  domElement = doc.createElement("frame-text-enabled");
  contentElement.appendChild(domElement);
  valueStr.setNum(frameTextEnabled());
  text = doc.createTextNode(valueStr);
  domElement.appendChild(text);

  if(frameTextEnabled())
  {
    domElement = doc.createElement("frame-text");
    contentElement.appendChild(domElement);
    text = doc.createTextNode(frameText());
    domElement.appendChild(text);
  }

  // save transformation
  const QTransform t = transform();
  if(!t.isIdentity() || rotation() != 0)
  {
    domElement = doc.createElement("transformation");
    domElement.setAttribute("xRot", m_perspectiveAngles.x());
    domElement.setAttribute("yRot", m_perspectiveAngles.y());
#if QT_VERSION < 0x040600
    domElement.setAttribute("zRot", m_rotationAngle);
#else
    domElement.setAttribute("zRot", rotation());
#endif
    contentElement.appendChild(domElement);
  }
  domElement = doc.createElement("mirror");
  domElement.setAttribute("state", mirrored());
  contentElement.appendChild(domElement);
}

QPixmap AbstractContent::toPixmap(const QSize & size, Qt::AspectRatioMode ratio)
{
  // allocate a fixed size pixmap and draw the content over it
  QPixmap pixmap(size);
  pixmap.fill(Qt::transparent);
  QPainter painter(&pixmap);
  drawContent(&painter, pixmap.rect(), ratio);
  painter.end();
  return pixmap;
}

int AbstractContent::contentHeightForWidth(int width) const
{ return width; }

bool AbstractContent::contentOpaque() const
{ return false; }

QWidget * AbstractContent::createPropertyWidget(ContentProperties * __p)
{
  ContentProperties * cp = __p ? __p : new ContentProperties;

  // connect actions
  connect(cp->cFront, SIGNAL(clicked()), this, SLOT(slotStackFront()));
  connect(cp->cRaise, SIGNAL(clicked()), this, SLOT(slotStackRaise()));
  connect(cp->cLower, SIGNAL(clicked()), this, SLOT(slotStackLower()));
  connect(cp->cBack, SIGNAL(clicked()), this, SLOT(slotStackBack()));
  connect(cp->cConfigure, SIGNAL(clicked()), this, SLOT(slotConfigure()));

  // properties link
  m_opacitySlider = new PE_AbstractSlider(cp->cOpacity, this, "contentOpacity", cp);
  connect(m_opacitySlider, SIGNAL(sliderReleased()), this, SLOT(slotOpacityChanged()));
  connect(m_opacitySlider, SIGNAL(sliderPressed()), this, SLOT(slotOpacityChanging()));
  new PE_Combo(cp->cFxCombo, this, "fxIndex", cp);
  cp->cPerspWidget->setRange(QRectF(-70.0, -70.0, 140.0, 140.0));
  connect(cp->cPerspWidget, SIGNAL(released()), this, SLOT(slotReleasePerspectivePane()));
  new PE_PaneWidget(cp->cPerspWidget, this, "perspective", cp);

  return cp;
}

QRectF AbstractContent::boundingRect() const
{ return m_frameRect; }

void AbstractContent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
  // emitting the edit request by default. some subclasses request backgrounding
  emit requestEditing();
  event->accept();
}

void AbstractContent::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
  // find out whether to draw the selection
  const bool drawSelection = RenderOpts::HQRendering ? false : isSelected();

  // change opacity
  qreal opacity = contentOpacity();
  if(opacity < 1.0) painter->setOpacity(opacity);

  if(m_frame)
  {
    // draw the Frame
    m_frame->drawFrame(painter, m_frameRect.toRect(), drawSelection, contentOpaque());

    // use clip path for contents, if set
    if(m_frame->clipContents()) painter->setClipPath(m_frame->contentsClipPath(m_contentRect));
  }

#if 0
    if (RenderOpts::OpenGLWindow && drawSelection)
        painter->setCompositionMode(QPainter::CompositionMode_Plus);
#endif

  // paint the inner contents
  const QRect tcRect = QRect(0, 0, m_contentRect.width(), m_contentRect.height());
  painter->translate(m_contentRect.topLeft());
  drawContent(painter, tcRect, Qt::IgnoreAspectRatio);

  // restore opacity
  if(opacity < 1.0) painter->setOpacity(1.0);

  // overlay a selection
  if(drawSelection && !m_frame)
  {
    painter->setRenderHint(QPainter::Antialiasing, true);
    QPen selPen(RenderOpts::hiColor, 2.0);
    selPen.setJoinStyle(Qt::MiterJoin);
    painter->setPen(selPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(tcRect.adjusted(1, 1, -1, -1));
  }
}

void AbstractContent::selectionChanged(bool /*selected*/)
{
  // nothing to do here.. only used by subclasses
}

void AbstractContent::GFX_CHANGED() const
{
  if(m_gfxChangeTimer && m_mirrorItem) m_gfxChangeTimer->start();
}

void AbstractContent::setControlsVisible(bool visible)
{
  m_controlsVisible = visible;
  foreach(CornerItem * corner, m_cornerItems) corner->setVisible(visible);
  foreach(ButtonItem * button, m_controlItems) button->setVisible(visible);
}

QPixmap AbstractContent::ratioScaledPixmap(const QPixmap * source, const QSize & size, Qt::AspectRatioMode ratio) const
{
  QPixmap scaledPixmap = source->scaled(size, ratio, Qt::SmoothTransformation);
  if(scaledPixmap.size() != size)
  {
    int offX = (scaledPixmap.width() - size.width()) / 2;
    int offY = (scaledPixmap.height() - size.height()) / 2;
    if(ratio == Qt::KeepAspectRatio)
    {
      QPixmap rightSizePixmap(size);
      rightSizePixmap.fill(Qt::transparent);
      QPainter p(&rightSizePixmap);
      p.drawPixmap(-offX, -offY, scaledPixmap);
      p.end();
      return rightSizePixmap;
    }
    if(ratio == Qt::KeepAspectRatioByExpanding) { return scaledPixmap.copy(offX, offY, size.width(), size.height()); }
  }
  return scaledPixmap;
}

void AbstractContent::hoverEnterEvent(QGraphicsSceneHoverEvent * /*event*/)
{ setControlsVisible(true); }

void AbstractContent::hoverLeaveEvent(QGraphicsSceneHoverEvent * /*event*/)
{ setControlsVisible(false); }

void AbstractContent::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{ QGraphicsItem::mouseReleaseEvent(event); }

void AbstractContent::dragMoveEvent(QGraphicsSceneDragDropEvent * event)
{ event->accept(); }

void AbstractContent::dropEvent(QGraphicsSceneDragDropEvent * event)
{ event->accept(); }

void AbstractContent::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
  QGraphicsItem::mousePressEvent(event);
  if(event->button() == Qt::RightButton)
  {
    scene()->clearSelection();
    setSelected(true);
    emit requestConfig(event->scenePos().toPoint());
  }
  m_previousPos = scenePos();
}

void AbstractContent::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
  QGraphicsItem::mouseMoveEvent(event);
  event->accept();
}

void AbstractContent::keyPressEvent(QKeyEvent * event)
{
  // discard key events for unselectable items
  if(!(flags() & ItemIsSelectable))
  {
    event->ignore();
    return;
  }
  event->accept();
  int step = (event->modifiers() & Qt::ShiftModifier) ? 50 : (event->modifiers() & Qt::ControlModifier ? 1 : 10);
  switch(event->key())
  {
    // cursor keys: 10px, 50 if Shift pressed, 1 if Control pressed
    case Qt::Key_Left:
      setPosUndo(pos() - QPointF(step, 0));
      break;
    case Qt::Key_Up:
      setPosUndo(pos() - QPointF(0, step));
      break;
    case Qt::Key_Right:
      setPosUndo(pos() + QPointF(step, 0));
      break;
    case Qt::Key_Down:
      setPosUndo(pos() + QPointF(0, step));
      break;
    // deletion
    case Qt::Key_Delete:
      emit requestRemoval();
      break;

    // editing
    case Qt::Key_Return:
      emit requestEditing();
      break;

    // ignore unhandled key
    default:
      event->ignore();
  }
}

QVariant AbstractContent::itemChange(GraphicsItemChange change, const QVariant & value)
{
  // keep the AbstractContent's center inside the scene rect..
  if(change == ItemPositionChange && scene())
  {
    QPointF newPos = value.toPointF();
    QRectF sceneRect = scene()->sceneRect();
    if(!sceneRect.contains(newPos) && sceneRect.width() > 100 && sceneRect.height() > 100)
    {
      newPos.setX(qBound(sceneRect.left(), newPos.x(), sceneRect.right()));
      newPos.setY(qBound(sceneRect.top(), newPos.y(), sceneRect.bottom()));
      return newPos;
    }
  }

  // tell subclasses about selection changes
  if(change == ItemSelectedHasChanged) selectionChanged(value.toBool());

  // changes that affect the mirror item
  if(m_mirrorItem)
  {
    switch(change)
    {
      // notify about setPos
      case ItemPositionHasChanged:
        m_mirrorItem->sourceMoved();
        break;

      // notify about graphics changes
      // case ItemMatrixChange:
      // case ItemTransformChange:
      case ItemTransformHasChanged:
      case ItemEnabledHasChanged:
      case ItemSelectedHasChanged:
      case ItemParentHasChanged:
      case ItemOpacityHasChanged:
        GFX_CHANGED();
        break;

      case ItemZValueHasChanged:
        m_mirrorItem->setZValue(zValue());
        break;

      case ItemVisibleHasChanged:
        m_mirrorItem->setVisible(isVisible());
        break;

      default:
        break;
    }
  }

  // ..or just apply the value
  return QGraphicsItem::itemChange(change, value);
}

void AbstractContent::slotConfigure()
{
  ButtonItem * item = dynamic_cast<ButtonItem *>(sender());
  if(item)
    emit requestConfig(item->scenePos().toPoint());
  else
    emit requestConfig(scenePos().toPoint());
}

void AbstractContent::slotStackFront()
{ emit changeStack(1); }

void AbstractContent::slotStackRaise()
{ emit changeStack(2); }

void AbstractContent::slotStackLower()
{ emit changeStack(3); }

void AbstractContent::slotStackBack()
{ emit changeStack(4); }

void AbstractContent::slotSaveAs()
{
  // make up the default save path (stored as 'Fotowall/ExportDir')
  QSettings s;
  QString defaultSavePath = tr("Unnamed %1.png").arg(QDate::currentDate().toString());
  if(s.contains("Fotowall/ExportDir"))
    defaultSavePath.prepend(s.value("Fotowall/ExportDir").toString() + QDir::separator());

  // ask the file name, validate it, store back to settings
  QString imgFilePath = QFileDialog::getSaveFileName(0, tr("Choose the Image file"), defaultSavePath,
                                                     tr("Images (*.jpeg *.jpg *.png *.bmp *.tif *.tiff)"));
  if(imgFilePath.isNull()) return;
  s.setValue("Fotowall/ExportDir", QFileInfo(imgFilePath).absolutePath());
  if(QFileInfo(imgFilePath).suffix().isEmpty()) imgFilePath += ".png";

  // find out the Transform chain to mirror a rotated item
  QRectF sceneRectF = mapToScene(boundingRect()).boundingRect();
  QTransform tFromItem = transform() * QTransform(1, 0, 0, 1, pos().x(), pos().y());
  QTransform tFromPixmap = QTransform(1, 0, 0, 1, sceneRectF.left(), sceneRectF.top());
  QTransform tItemToPixmap = tFromItem * tFromPixmap.inverted();

  // render on the image
  int iHeight = (int)sceneRectF.height();
  if(m_mirrorItem) iHeight += (int)m_mirrorItem->boundingRect().height();
  QImage image((int)sceneRectF.width(), iHeight, QImage::Format_ARGB32);
  image.fill(Qt::transparent);

  // enable hi-q rendering
  bool prevHQ = RenderOpts::HQRendering;
  RenderOpts::HQRendering = true;

  // draw the transformed item onto the pixmap
  QPainter p(&image);
  p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true);
  p.setTransform(tItemToPixmap);
  paint(&p, 0, 0);
  if(m_mirrorItem)
  {
    p.resetTransform();
    p.translate(0, (qreal)((int)sceneRectF.height()));
    m_mirrorItem->paint(&p, 0, 0);
  }
  p.end();
  RenderOpts::HQRendering = prevHQ;

  // save image and check errors
  if(!image.save(imgFilePath) || !QFile::exists(imgFilePath))
  {
    QMessageBox::warning(0, tr("File Error"), tr("Error saving the Object to '%1'").arg(imgFilePath));
    return;
  }
}

void AbstractContent::createCorner(Qt::Corner corner, bool noRescale)
{
  CornerItem * c = new CornerItem(corner, noRescale, this);
  c->setVisible(m_controlsVisible);
  c->setZValue(2.0);
  c->setToolTip(
      tr("Drag with Left or Right mouse button.\n - Hold down SHIFT for free resize\n - Hold down CTRL to rotate "
         "only\n - "
         "Hold down ALT to snap rotation\n - Double click (with LMB/RMB) to restore the aspect ratio/rotation"));
  m_cornerItems.append(c);
}

void AbstractContent::layoutChildren()
{
  // layout corners
  foreach(CornerItem * corner, m_cornerItems) corner->relayout(m_contentRect);

  // layout controls and text
  if(m_frame)
  {
    m_frame->layoutButtons(m_controlItems, m_frameRect.toRect());
    if(m_frameTextItem) m_frame->layoutText(m_frameTextItem, m_frameRect.toRect());
  }
  else
  {
    // layout controls
    const int spacing = 4;
    int left = m_frameRect.left();
    int top = m_frameRect.top();
    int right = m_frameRect.right();
    int bottom = m_frameRect.bottom();
    int offset = right;
    foreach(ButtonItem * button, m_controlItems)
    {
      switch(button->buttonType())
      {
        case ButtonItem::FlipH:
          button->setPos(right - button->width() / 2, (top + bottom) / 2);
          break;

        case ButtonItem::FlipV:
          button->setPos((left + right) / 2, top + button->height() / 2);
          break;

        case ButtonItem::Rotate:
          button->setPos(left + button->width() / 2, (top + bottom) / 2);
          break;

        default:
          button->setPos(offset - button->width() / 2, bottom - button->height() / 2);
          offset -= button->width() + spacing;
          break;
      }
    }
    // hide text, if present
    if(m_frameTextItem) m_frameTextItem->hide();
  }
}

void AbstractContent::slotReleasePerspectiveButton(QGraphicsSceneMouseEvent *)
{
  do_canvas_command(scene(), new PerspectiveCommand(this, m_previousPerspectiveAngles, m_perspectiveAngles, false));
  m_previousPerspectiveAngles = m_perspectiveAngles;
}

void AbstractContent::slotReleasePerspectivePane()
{
  PaneWidget * w = qobject_cast<PaneWidget *>(sender());
  do_canvas_command(scene(), new PerspectiveCommand(this, m_previousPerspectiveAngles, w->endValue(), false));
  m_previousPerspectiveAngles = w->endValue();
}

void AbstractContent::slotOpacityChanging()
{ m_opacity = contentOpacity(); }

void AbstractContent::slotOpacityChanged()
{
  do_canvas_command(scene(), new OpacityCommand(this, m_opacity, contentOpacity()));
  m_opacity = contentOpacity();
}

void AbstractContent::applyTransforms()
{
  setTransform(QTransform()
                   .rotate(m_perspectiveAngles.y(), Qt::XAxis)
                   .rotate(m_perspectiveAngles.x(), Qt::YAxis)
#if QT_VERSION < 0x040600
                   .rotate(m_rotationAngle, Qt::ZAxis)
#endif
                   ,
               false);
}

void AbstractContent::slotSetPerspective(const QPointF & sceneRelPoint, Qt::KeyboardModifiers modifiers)
{
  if(modifiers & Qt::ControlModifier) return slotClearPerspective();
  qreal k = modifiers == Qt::NoModifier ? 0.2 : 0.5;
  setPerspective(QPointF(qBound((qreal)-70.0, (qreal)sceneRelPoint.x() * k, (qreal)70.0),
                         qBound((qreal)-70.0, (qreal)sceneRelPoint.y() * k, (qreal)70.0)));
}

void AbstractContent::slotClearPerspective()
{ setPerspective(QPointF(0, 0)); }

void AbstractContent::slotDirtyEnded()
{
  m_dirtyTransforming = false;
  update();
  GFX_CHANGED();
}
