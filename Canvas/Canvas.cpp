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

#include "Canvas.h"

#include "App/App.h" // this violates insulation (only for the pic service)
#include "CanvasModeInfo.h"
#include "CanvasViewContent.h"
#include "PictureConfig.h"
#include "PictureContent.h"
#include "SelectionProperties.h"
#include "Shared/ColorPickerItem.h"
#include "Shared/Commands.h"
#include "Shared/Compat.h"
#include "Shared/HighlightItem.h"
#include "Shared/PictureServices/AbstractPictureService.h"
#include "Shared/RenderOpts.h"
#include "TextConfig.h"
#include "TextContent.h"
#include "WebcamContent.h"
#include "WordcloudContent.h"

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QDate>
#include <QFile>
#include <QFileInfo>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsView>
#include <QImageReader>
#include <QList>
#include <QMessageBox>
#include <QMimeData>
#include <QNetworkReply>
#include <QTextDocument>
#include <QTimer>
#include <QUrl>

#define COLORPICKER_W 200
#define COLORPICKER_H 150

Canvas::Canvas(int sDpiX, int sDpiY, QObject * parent)
: AbstractScene(parent), m_filePath(), m_fileAbsDir(), m_modeInfo(new CanvasModeInfo), m_backMode(BackGradient),
  m_backContent(0), m_backContentRatio(Qt::KeepAspectRatioByExpanding), m_topBarEnabled(false),
  m_bottomBarEnabled(false), m_forceFieldTimer(0), m_embeddedPainting(false), m_pendingChanges(false),
  m_commandStack(new QUndoStack(parent))
{
  // init modeinfo
  m_modeInfo->setScreenDpi(sDpiX, sDpiY);

  // create colorpickers
  m_titleColorPicker = new ColorPickerItem(COLORPICKER_W, COLORPICKER_H, this);
  m_titleColorPicker->setColor(Qt::red);
  m_titleColorPicker->setAnimated(true);
  m_titleColorPicker->setAnchor(ColorPickerItem::AnchorTop);
  m_titleColorPicker->setZValue(10000);
  m_titleColorPicker->setVisible(false);
  connect(m_titleColorPicker, SIGNAL(colorChanged(const QColor &)), this, SLOT(slotTitleColorChanged()));
  addItem(m_titleColorPicker);

  m_foreColorPicker = new ColorPickerItem(COLORPICKER_W, COLORPICKER_H, this);
  m_foreColorPicker->setColor(QColor(128, 128, 128));
  m_foreColorPicker->setAnimated(true);
  m_foreColorPicker->setAnchor(ColorPickerItem::AnchorTopLeft);
  m_foreColorPicker->setZValue(10000);
  m_foreColorPicker->setVisible(false);
  connect(m_foreColorPicker, SIGNAL(colorChanged(const QColor &)), this, SLOT(slotForeColorChanged()));
  addItem(m_foreColorPicker);

  m_grad1ColorPicker = new ColorPickerItem(COLORPICKER_W, COLORPICKER_H, this);
  m_grad1ColorPicker->setColor(QColor(192, 192, 192));
  m_grad1ColorPicker->setAnimated(true);
  m_grad1ColorPicker->setAnchor(ColorPickerItem::AnchorTopRight);
  m_grad1ColorPicker->setZValue(10000);
  connect(m_grad1ColorPicker, SIGNAL(colorChanged(const QColor &)), this, SLOT(slotGradColorChanged()));
  addItem(m_grad1ColorPicker);

  m_grad2ColorPicker = new ColorPickerItem(COLORPICKER_W, COLORPICKER_H, this);
  m_grad2ColorPicker->setColor(QColor(80, 80, 80));
  m_grad2ColorPicker->setAnimated(true);
  m_grad2ColorPicker->setAnchor(ColorPickerItem::AnchorBottomRight);
  m_grad2ColorPicker->setZValue(10000);
  connect(m_grad2ColorPicker, SIGNAL(colorChanged(const QColor &)), this, SLOT(slotGradColorChanged()));
  addItem(m_grad2ColorPicker);

  // selection self-hook
  connect(this, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()));

  // precreate background tile
  m_backTile = QPixmap(100, 100);
  m_backTile.fill(Qt::lightGray);
  QPainter tilePainter(&m_backTile);
  tilePainter.fillRect(0, 0, 50, 50, Qt::darkGray);
  tilePainter.fillRect(50, 50, 50, 50, Qt::darkGray);
  tilePainter.end();

  // set a placeholder size, the real one will be set on resize
  resize(QSize(1, 1));
}

Canvas::~Canvas()
{
  clearContent();
  qDeleteAll(m_highlightItems);
  delete m_titleColorPicker;
  delete m_foreColorPicker;
  delete m_grad1ColorPicker;
  delete m_grad2ColorPicker;
  delete m_forceFieldTimer;
  delete m_modeInfo;
}

/// Add Content
void Canvas::addAutoContent(const QStringList & filePaths)
{
  // simple auto-detection of the content type
  foreach(const QString & localFile, filePaths)
  {
    if(QFile::exists(localFile))
    {
      // Note: should use App.h for this, but dup'ed for isolation
      if(localFile.endsWith(".fotowall", Qt::CaseInsensitive))
        addCanvasViewContent(QStringList() << localFile);
      else
        addPictureContent(QStringList() << localFile);
    }
  }
}

void Canvas::addCanvasViewContent(const QStringList & fwFilePaths)
{
  clearSelection();
  QList<AbstractContent *> addedCanvas;
  int offset = -30 * (fwFilePaths.size() - 1) / 2;
  QPoint pos = visibleCenter() + QPoint(offset, offset);
  m_commandStack->beginMacro(tr("Add Canvas View"));
  foreach(const QString & localFile, fwFilePaths)
  {
    if(!QFile::exists(localFile)) continue;

    // create picture and load the file
    CanvasViewContent * d = createCanvasView(pos, true);
    if(!d->loadFromFile(localFile, true, true))
    {
      m_content.removeAll(d);
      delete d;
    }
    else
    {
      d->setSelected(true);
      pos += QPoint(30, 30);
      m_commandStack->push(new NewContentCommand(d, this));
      addedCanvas << d;
    }
  }
  m_commandStack->endMacro();
  if(addedCanvas.isEmpty()) m_commandStack->undo();
}

QList<AbstractContent *> Canvas::addPictureContent(const QStringList & picFilePaths)
{
  clearSelection();
  QList<AbstractContent *> addedPictures;
  int offset = -30 * (picFilePaths.size() - 1) / 2;
  QPoint pos = visibleCenter() + QPoint(offset, offset);
  m_commandStack->beginMacro(tr("Add pictures"));
  foreach(const QString & localFile, picFilePaths)
  {
    if(!QFile::exists(localFile)) continue;

    // create picture and load the file
    PictureContent * p = createPicture(pos, true);
    if(!p->loadFromFile(localFile, true, true, true))
    {
      m_content.removeAll(p);
      delete p;
    }
    else
    {
      p->setSelected(true);
      pos += QPoint(30, 30);
      m_commandStack->push(new NewContentCommand(p, this));
      addedPictures << p;
    }
  }
  m_commandStack->endMacro();
  if(addedPictures.isEmpty()) m_commandStack->undo();
  return addedPictures;
}

PictureContent * Canvas::addNetworkPictureContent(const QString & url)
{
  clearSelection();
  // create picture and load the file
  PictureContent * p = createPicture(visibleCenter(), true);
  if(!p->loadFromNetwork(url, 0))
  {
    m_content.removeAll(p);
    delete p;
    p = 0;
  }
  else
  {
    p->setSelected(true);
    m_commandStack->beginMacro(tr("Add Network Picture"));
    m_commandStack->push(new NewContentCommand(p, this));
    m_commandStack->endMacro();
  }
  return p;
}

PictureContent * Canvas::addNetworkPictureContent(const QString & url,
                                                  QNetworkReply * reply,
                                                  QString title,
                                                  int width,
                                                  int height)
{
  clearSelection();
  PictureContent * p = createPicture(visibleCenter(), true);
  if(!p->loadFromNetwork(url, reply, title, width, height))
  {
    m_content.removeAll(p);
    delete p;
    p = 0;
  }
  else
  {
    p->setSelected(true);
    m_commandStack->beginMacro(tr("Add Network Picture"));
    m_commandStack->push(new NewContentCommand(p, this));
    m_commandStack->endMacro();
  }
  return p;
}

TextContent * Canvas::addTextContent()
{
  clearSelection();
  TextContent * t = createText(visibleCenter(), true);
  if(t != 0)
  {
    m_commandStack->beginMacro(tr("Add Text Content"));
    m_commandStack->push(new NewContentCommand(t, this));
    m_commandStack->endMacro();
  }
  t->setSelected(true);
  return t;
}

WebcamContent * Canvas::addWebcamContent(int webcamIndex)
{
  clearSelection();
  WebcamContent * w = createWebcam(webcamIndex, visibleCenter(), true);
  if(w != 0)
  {
    m_commandStack->beginMacro(tr("Add Webcam Content"));
    m_commandStack->push(new NewContentCommand(w, this));
    m_commandStack->endMacro();
  }
  w->setSelected(true);
  return w;
}

WordcloudContent * Canvas::addWordcloudContent()
{
  clearSelection();
  WordcloudContent * w = createWordcloud(visibleCenter(), true);
  if(w != 0)
  {
    m_commandStack->beginMacro(tr("Add Wordcloud Content"));
    m_commandStack->push(new NewContentCommand(w, this));
    m_commandStack->endMacro();
  }
  w->manualInitialization();
  w->setSelected(true);
  return w;
}

void Canvas::addManualContent(AbstractContent * content, const QPoint & pos)
{
  initContent(content, pos);
}

void Canvas::clearContent()
{
  while(!m_content.isEmpty()) deleteContent(m_content.first());
  while(!m_configs.isEmpty()) deleteConfig(m_configs.first());
  // this is not needed, it's here only as extra-safety
  // Q_ASSERT(!m_backContent)
  m_backContent = 0;
}

QPoint Canvas::visibleCenter() const
{
  QPoint center;
  QGraphicsView * view = mainGraphicsView();
  if(view)
  {
    QPoint viewCenter = view->viewport()->rect().center();
    center = view->mapToScene(viewCenter).toPoint();
  }
  else
    center = sceneRect().center().toPoint();
  return center + QPoint(2 - (compat::qrand() % 5), 2 - (compat::qrand() % 5));
}

void Canvas::resize(const QSize & size)
{
  // handle the fixed resizes
  if(m_modeInfo->fixedSize())
  {
    QSize fixedSize = m_modeInfo->fixedScreenPixels();
    if(size != fixedSize) AbstractScene::resize(fixedSize);
    return;
  }
  // handle the normal resizes
  AbstractScene::resize(size);
}

void Canvas::resizeEvent()
{
  // The resizeEvent only occurs when changing project mode since the canvas is contained within a scroll area. Thus
  // we don't need to save the changes occuring here to the undo stack, as they will be recomputed when the next
  // change of project mode occurs.

  // relayout contents
  m_titleColorPicker->setPos((sceneWidth() - COLORPICKER_W) / 2.0, 10);
  m_grad1ColorPicker->setPos(sceneWidth() - COLORPICKER_W, 0);
  m_grad2ColorPicker->setPos(sceneWidth() - COLORPICKER_W, sceneHeight() - COLORPICKER_H);
  foreach(HighlightItem * highlight, m_highlightItems) highlight->reposition(sceneRect());

  if(m_content.size())
  {
    foreach(AbstractContent * content, m_content) { content->ensureVisible(sceneRect()); }
  }
  foreach(AbstractConfig * config, m_configs) { config->keepInBoundaries(sceneRect()); }

  // reblink after mobile relayout
#if defined(MOBILE_UI)
  // if (m_backMode == BackGradient && sceneWidth() > 200 && sceneHeight() > 200)
  //     blinkBackGradients();
#endif
}

QString Canvas::filePath() const
{
  if(m_filePath.isEmpty()) return tr("Unnamed %1").arg(QDate::currentDate().toString()) + ".fotowall";
  return m_filePath;
}

void Canvas::setFilePath(const QString & filePath)
{
  if(filePath != m_filePath)
  {
    // use the right path
    m_filePath = QDir(filePath).canonicalPath();
    if(m_filePath.isEmpty()) m_filePath = filePath;
    m_fileAbsDir = QFileInfo(m_filePath).absoluteDir();
    emit filePathChanged();
  }
}

QString Canvas::prettyBaseName() const
{
  if(m_filePath.isEmpty()) return tr("Unnamed %1").arg(QDate::currentDate().toString()) + ".fotowall";
  return QFileInfo(m_filePath).baseName();
}

/// Item Interaction
void Canvas::selectAllContent(bool selected)
{
  foreach(AbstractContent * content, m_content) content->setSelected(selected);
}

/// Arrangement
void Canvas::setForceFieldEnabled(bool enabled)
{
  if(enabled)
  {
    foreach(AbstractContent * c, m_content) { c->setPreviousPos(c->pos()); }
  }
  if(enabled && !m_forceFieldTimer)
  {
    m_forceFieldTimer = new QTimer(this);
    connect(m_forceFieldTimer, SIGNAL(timeout()), this, SLOT(slotApplyForce()));
    m_forceFieldTimer->start(10);
    m_forceFieldTime.start();
  }

  if(!enabled && m_forceFieldTimer)
  {
    delete m_forceFieldTimer;
    m_forceFieldTimer = 0;
  }

  if(!enabled)
  {
    MotionCommand * mc = 0;
    if(m_content.size())
    {
      m_commandStack->beginMacro(tr("Force Field"));
      foreach(AbstractContent * c, m_content)
      {
        mc = new MotionCommand(c, c->previousPos(), c->pos());
        m_commandStack->push(mc);
      }
      m_commandStack->endMacro();
    }
  }
}

bool Canvas::forceFieldEnabled() const
{
  return m_forceFieldTimer;
}

#if QT_VERSION >= 0x040600
#  include <QPropertyAnimation>
#  define ANIMATE_PARAM(object, propName, duration, endValue)                      \
    {                                                                              \
      QPropertyAnimation * ani = new QPropertyAnimation(object, propName, object); \
      ani->setEasingCurve(QEasingCurve::InOutCubic);                               \
      ani->setDuration(duration);                                                  \
      ani->setEndValue(endValue);                                                  \
      ani->start(QPropertyAnimation::DeleteWhenStopped);                           \
    }
#else
#  define ANIMATE_PARAM(instance, propName, duration, endValue) instance->setProperty(propName, endValue);
#endif

void Canvas::randomizeContents(bool position, bool rotation, bool opacity)
{
  if(m_content.isEmpty()) return;

  m_commandStack->beginMacro(tr("Randomize Contents"));
  QRectF r = sceneRect();
  r.adjust(r.width() / 6, r.height() / 6, -r.width() / 6, -r.height() / 6);
  foreach(AbstractContent * content, m_content)
  {

    // randomize position
    if(position)
    {
      QPointF pos(r.left() + (compat::qrand() % (int)r.width()), r.top() + (compat::qrand() % (int)r.height()));
      ANIMATE_PARAM(content, "pos", 200, pos);
      m_commandStack->push(new MotionCommand(content, content->pos(), pos));
    }

    // randomize rotation
    if(rotation)
    {
      int rot = -30 + (compat::qrand() % 60);
      ANIMATE_PARAM(content, "rotation", 1000, rot);
      m_commandStack->push(new RotateAndResizeCommand(content, content->rotation(), rot, content->contentRect(),
                                                      content->contentRect()));
    }

    // randomize opacity
    if(opacity)
    {
      // TODO: undo
      qreal opa = 0.5 + (qreal)(compat::qrand() % 100) / 99.0;
      //  ANIMATE_PARAM(content, "contentOpacity", 2000, opacity);
      OpacityCommand * oc = new OpacityCommand(content, content->contentOpacity(), opa);
      oc->redo();
      m_commandStack->push(oc);
    }
  }
  m_commandStack->endMacro();
}

/// Decorations
void Canvas::setBackMode(BackMode mode)
{
  if(m_backMode == mode) return;

  // apply the change to the background
  m_backMode = mode;
  bool isGradient = m_backMode == BackGradient;
  m_grad1ColorPicker->setVisible(isGradient);
  m_grad2ColorPicker->setVisible(isGradient);
  if(isGradient) blinkBackGradients();
  update();

  // notify the change
  emit backConfigChanged();
}

Canvas::BackMode Canvas::backMode() const
{
  return m_backMode;
}

void Canvas::clearBackContent()
{
  auto * command = new BackgroundContentCommand(this, m_backContent, 0);
  command->redo();
  m_commandStack->push(command);
}

bool Canvas::backContent() const
{
  return m_backContent;
}

void Canvas::setBackContentRatio(Qt::AspectRatioMode mode)
{
  if(m_backContentRatio != mode)
  {
    m_backContentRatio = mode;
    m_backCache = QPixmap();
    update();
  }
}

Qt::AspectRatioMode Canvas::backContentRatio() const
{
  return m_backContentRatio;
}

void Canvas::setTopBarEnabled(bool enabled)
{
  if(enabled == m_topBarEnabled) return;
  m_topBarEnabled = enabled;
  m_foreColorPicker->setVisible(m_topBarEnabled || m_bottomBarEnabled);
  update();
}

bool Canvas::topBarEnabled() const
{
  return m_topBarEnabled;
}

void Canvas::setBottomBarEnabled(bool enabled)
{
  if(enabled == m_bottomBarEnabled) return;
  m_bottomBarEnabled = enabled;
  m_foreColorPicker->setVisible(m_topBarEnabled || m_bottomBarEnabled);
  update();
}

bool Canvas::bottomBarEnabled() const
{
  return m_bottomBarEnabled;
}

void Canvas::setTitleText(const QString & text)
{
  m_titleText = text;
  m_titleColorPicker->setVisible(!text.isEmpty());
  update(0, 0, sceneWidth(), 50);
}

QString Canvas::titleText() const
{
  return m_titleText;
}

void Canvas::setCDMarkers()
{
  clearMarkers();
  QPen outerPen(Qt::black, 1, Qt::DashLine);
  QPen innerPen(Qt::lightGray, 1, Qt::DashLine);
  QSize screenPixels = m_modeInfo->fixedScreenPixels();
  QPointF screenDpi = m_modeInfo->screenDpi();

  float xDiameter = 4.75 * screenDpi.x();
  float yDiameter = 4.75 * screenDpi.y();
  QGraphicsEllipseItem * ellipse = addEllipse((screenPixels.width() - xDiameter) / 2.0,
                                              (screenPixels.height() - yDiameter) / 2.0, xDiameter, yDiameter);
  ellipse->setZValue(-1);
  ellipse->setPen(outerPen);
  m_markerItems.push_back(ellipse);

  xDiameter = 0.59 * screenDpi.x();
  yDiameter = 0.59 * screenDpi.y();
  ellipse = addEllipse((screenPixels.width() - xDiameter) / 2.0, (screenPixels.height() - yDiameter) / 2.0, xDiameter,
                       yDiameter);
  ellipse->setZValue(-1);
  ellipse->setPen(innerPen);
  m_markerItems.push_back(ellipse);

  xDiameter = 1.34 * screenDpi.x();
  yDiameter = 1.34 * screenDpi.y();
  ellipse = addEllipse((screenPixels.width() - xDiameter) / 2.0, (screenPixels.height() - yDiameter) / 2.0, xDiameter,
                       yDiameter);
  ellipse->setZValue(-1);
  ellipse->setPen(innerPen);
  m_markerItems.push_back(ellipse);

  xDiameter = 1.73 * screenDpi.x();
  yDiameter = 1.73 * screenDpi.y();
  ellipse = addEllipse((screenPixels.width() - xDiameter) / 2.0, (screenPixels.height() - yDiameter) / 2.0, xDiameter,
                       yDiameter);
  ellipse->setZValue(-1);
  ellipse->setPen(innerPen);
  m_markerItems.push_back(ellipse);
}

void Canvas::setDVDMarkers()
{
  // Add informations items to show the back, front, and side position
  clearMarkers();
  QPen linePen(Qt::black, 1, Qt::DashLine);
  QPointF screenDpi = m_modeInfo->screenDpi();
  int faceW = 5.08 * screenDpi.x();
  int sideW = 0.67 * screenDpi.y();
  int height = m_modeInfo->fixedScreenPixels().height();
  QGraphicsLineItem * line = addLine((qreal)faceW + 0.5, 0, (qreal)faceW + 0.5, height);
  line->setPen(linePen);
  line->setZValue(-1);
  m_markerItems.push_back(line);
  line = addLine((qreal)(faceW + sideW) + 0.5, 0, (qreal)(faceW + sideW) + 0.5, height), line->setPen(linePen);
  line->setZValue(-1);
  m_markerItems.push_back(line);

  QFont markerFont;
  markerFont.setPointSize(18);
  markerFont.setItalic(true);
  QGraphicsTextItem * textBack = addText(tr("Back"), markerFont);
  textBack->setTransform(QTransform(-1, 0, 0, 0, 1, 0, 0, 0, 1));
  textBack->setPos((faceW + textBack->document()->documentLayout()->documentSize().width()) / 2,
                   (height - textBack->document()->documentLayout()->documentSize().height()) / 2);
  textBack->setZValue(-1);
  m_markerItems.push_back(textBack);
  QGraphicsTextItem * textFront = addText(tr("Front"), markerFont);
  textFront->setPos((faceW + sideW) + faceW / 2 - textFront->document()->documentLayout()->documentSize().width() / 2,
                    (height - textFront->document()->documentLayout()->documentSize().height()) / 2);
  textFront->setZValue(-1);
  m_markerItems.push_back(textFront);
}

void Canvas::clearMarkers()
{
  // Remove the information items
  qDeleteAll(m_markerItems);
  m_markerItems.clear();
}

/// Misc: save, restore, help...
bool Canvas::pendingChanges() const
{
  return !m_content.isEmpty() && m_pendingChanges;
}

#define HIGHLIGHT(x, y, del)                         \
  do                                                 \
  {                                                  \
    HighlightItem * highlight = new HighlightItem(); \
    if(!del)                                         \
      m_highlightItems.append(highlight);            \
    else                                             \
      highlight->deleteAfterAnimation();             \
    addItem(highlight);                              \
    highlight->setZValue(10000);                     \
    highlight->setPosF(x, y);                        \
    highlight->show();                               \
  } while(0)

void Canvas::blinkBackGradients()
{
  HIGHLIGHT(1.0, 0.0, true);
  HIGHLIGHT(1.0, 1.0, true);
}

void Canvas::setEmbeddedPainting(bool embedded)
{
  m_embeddedPainting = embedded;
}

/// Modes
CanvasModeInfo * Canvas::modeInfo() const
{
  return m_modeInfo;
}

void Canvas::renderVisible(QPainter * painter,
                           const QRectF & target,
                           const QRectF & source,
                           Qt::AspectRatioMode aspectRatioMode,
                           bool hideTools)
{
  if(hideTools)
  {
    clearSelection();
    foreach(QGraphicsItem * item, m_markerItems) item->hide();
    foreach(AbstractConfig * conf, m_configs) conf->hide();
  }

  RenderOpts::HQRendering = true;
  QGraphicsScene::render(painter, target, source, aspectRatioMode);
  RenderOpts::HQRendering = false;

  if(hideTools)
  {
    foreach(AbstractConfig * conf, m_configs) conf->show();
    foreach(QGraphicsItem * item, m_markerItems) item->show();
  }
}

QImage Canvas::renderedImage(const QSize & iSize, Qt::AspectRatioMode aspectRatioMode, bool hideTools)
{
  QImage result(iSize, QImage::Format_ARGB32);
  result.fill(0);

  QPainter painter(&result);
  painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform, true);

  QSize targetSize = sceneSize();
  targetSize.scale(iSize, aspectRatioMode);
  int offsetX = (iSize.width() - targetSize.width()) / 2;
  int offsetY = (iSize.height() - targetSize.height()) / 2;

  QRect targetRect = QRect(offsetX, offsetY, targetSize.width(), targetSize.height());
  renderVisible(&painter, targetRect, sceneRect(), Qt::IgnoreAspectRatio, hideTools);
  painter.end();

  return result;
}

/// Load & Save
void Canvas::saveToXml(QDomElement & canvasElement) const
{
  QDomDocument doc = canvasElement.ownerDocument();

  // META
  {
    QDomElement metaElement = doc.createElement("meta");
    canvasElement.appendChild(metaElement);

    QDomElement fpElement = doc.createElement("filepath");
    metaElement.appendChild(fpElement);
    fpElement.appendChild(doc.createTextNode(m_filePath));
  }

  // MODEINFO
  {
    QDomElement modeElement = doc.createElement("mode");
    canvasElement.appendChild(modeElement);

    // save modeInfo
    m_modeInfo->toXml(modeElement);
  }

  // BACKGROUND
  {
    QDomElement backgroundElement = doc.createElement("background");
    canvasElement.appendChild(backgroundElement);

    // save back Mode
    QDomElement backModeElement = doc.createElement("mode");
    backgroundElement.appendChild(backModeElement);
    backModeElement.appendChild(doc.createTextNode(QString::number((int)m_backMode)));

    // save back Content Ratio
    QDomElement backRatioElement = doc.createElement("back-properties");
    backRatioElement.setAttribute("ratio", (int)m_backContentRatio);
    backgroundElement.appendChild(backRatioElement);

    // save Title
    QDomElement titleElement = doc.createElement("title");
    backgroundElement.appendChild(titleElement);
    titleElement.appendChild(doc.createTextNode(m_titleText));

    // save background Colors
    QColor color;
    QDomElement redElement = doc.createElement("red"), greenElement = doc.createElement("green"),
                blueElement = doc.createElement("blue"), redElement2 = doc.createElement("red"),
                greenElement2 = doc.createElement("green"), blueElement2 = doc.createElement("blue"),
                rElement = doc.createElement("red"), gElement = doc.createElement("green"),
                bElement = doc.createElement("blue"), rElement2 = doc.createElement("red"),
                gElement2 = doc.createElement("green"), bElement2 = doc.createElement("blue");
    QDomElement bgColorElement = doc.createElement("background-color");
    backgroundElement.appendChild(bgColorElement);

    QDomElement topColor = doc.createElement("top");
    color = m_grad1ColorPicker->color();
    redElement.appendChild(doc.createTextNode(QString::number(color.red())));
    greenElement.appendChild(doc.createTextNode(QString::number(color.green())));
    blueElement.appendChild(doc.createTextNode(QString::number(color.blue())));
    topColor.appendChild(redElement);
    topColor.appendChild(greenElement);
    topColor.appendChild(blueElement);
    bgColorElement.appendChild(topColor);

    QDomElement bottomColor = doc.createElement("bottom");
    color = m_grad2ColorPicker->color();
    redElement2.appendChild(doc.createTextNode(QString::number(color.red())));
    greenElement2.appendChild(doc.createTextNode(QString::number(color.green())));
    blueElement2.appendChild(doc.createTextNode(QString::number(color.blue())));
    bottomColor.appendChild(redElement2);
    bottomColor.appendChild(greenElement2);
    bottomColor.appendChild(blueElement2);
    bgColorElement.appendChild(bottomColor);

    QDomElement titleColor = doc.createElement("title-color");
    color = m_titleColorPicker->color();
    rElement.appendChild(doc.createTextNode(QString::number(color.red())));
    gElement.appendChild(doc.createTextNode(QString::number(color.green())));
    bElement.appendChild(doc.createTextNode(QString::number(color.blue())));
    titleColor.appendChild(rElement);
    titleColor.appendChild(gElement);
    titleColor.appendChild(bElement);
    backgroundElement.appendChild(titleColor);

    QDomElement foreColor = doc.createElement("foreground-color");
    color = m_foreColorPicker->color();
    rElement2.appendChild(doc.createTextNode(QString::number(color.red())));
    gElement2.appendChild(doc.createTextNode(QString::number(color.green())));
    bElement2.appendChild(doc.createTextNode(QString::number(color.blue())));
    foreColor.appendChild(rElement2);
    foreColor.appendChild(gElement2);
    foreColor.appendChild(bElement2);
    backgroundElement.appendChild(foreColor);
  }

  // CONTENT
  {
    QDomElement contentElement = doc.createElement("content");
    canvasElement.appendChild(contentElement);

    // save contents
    foreach(const AbstractContent * content, m_content)
    {
      // save content
      QDomElement cEl = doc.createElement("dummy-renamed-element");
      contentElement.appendChild(cEl);
      content->toXml(cEl, m_fileAbsDir);

      // add a flag to the background element
      if(m_backContent == content)
      {
        QDomElement bgEl = doc.createElement("set-as-background");
        cEl.appendChild(bgEl);
      }
    }
  }

  // PREVIEW
  {
    // make up the PNG image
    Canvas * rwCanvas = (Canvas *)this;
    QImage previewImage = rwCanvas->renderedImage(QSize(48, 48), Qt::KeepAspectRatio, true);
    if(!previewImage.isNull())
    {
      QBuffer saveData;
      saveData.open(QIODevice::ReadWrite);
      previewImage.save(&saveData, "PNG");
      saveData.close();

      // create a canvas.preview[cdata] element
      QByteArray encodedData = saveData.buffer().toBase64();
      QDomElement previewEl = doc.createElement("preview");
      canvasElement.appendChild(previewEl);
      previewEl.appendChild(doc.createCDATASection(encodedData));
    }
  }

  // reset the 'needs saving' flag
  Canvas * rwCanvas = (Canvas *)this;
  rwCanvas->slotResetChanges();
}

void Canvas::loadFromXml(QDomElement & canvasElement)
{
  // remove all content
  clearContent();

  // META
  {
    // find the 'meta' element
    QDomElement metaElement = canvasElement.firstChildElement("meta");
    if(metaElement.isElement())
    {
      // set file path if not already set (i.e. if loading sub-canvases)
      QDomElement fpElement = metaElement.firstChildElement("filepath");
      if(m_filePath.isEmpty()) setFilePath(fpElement.text());
    }
  }

  // MODEINFO
  {
    // find the 'mode' element
    QDomElement modeElement = canvasElement.firstChildElement("mode");
    if(!modeElement.isElement()) // 'Format 1'
      modeElement = canvasElement.firstChildElement("project").firstChildElement("mode");

    // read and apply properties
    if(modeElement.isElement()) m_modeInfo->fromXml(modeElement);
  }

  // BACKGROUND
  {
    // find the 'background' element
    QDomElement backgroundElement = canvasElement.firstChildElement("background");
    if(!backgroundElement.isElement()) // 'Format 1'
      backgroundElement = canvasElement.firstChildElement("desk");

    // read and apply properties
    if(backgroundElement.isElement())
    {
      // back Mode
      QDomElement domElement = backgroundElement.firstChildElement("mode");
      if(domElement.isElement()) setBackMode((Canvas::BackMode)domElement.text().toInt());

      // back ratio mode
      domElement = backgroundElement.firstChildElement("back-properties");
      if(domElement.isElement()) m_backContentRatio = (Qt::AspectRatioMode)domElement.attribute("ratio").toInt();

      // title text
      setTitleText(backgroundElement.firstChildElement("title").text());

      // colors
      domElement = backgroundElement.firstChildElement("background-color").firstChildElement("top");
      int r = domElement.firstChildElement("red").text().toInt();
      int g = domElement.firstChildElement("green").text().toInt();
      int b = domElement.firstChildElement("blue").text().toInt();
      m_grad1ColorPicker->setColor(QColor(r, g, b));
      domElement = backgroundElement.firstChildElement("background-color").firstChildElement("bottom");
      r = domElement.firstChildElement("red").text().toInt();
      g = domElement.firstChildElement("green").text().toInt();
      b = domElement.firstChildElement("blue").text().toInt();
      m_grad2ColorPicker->setColor(QColor(r, g, b));
      domElement = backgroundElement.firstChildElement("title-color");
      r = domElement.firstChildElement("red").text().toInt();
      g = domElement.firstChildElement("green").text().toInt();
      b = domElement.firstChildElement("blue").text().toInt();
      m_titleColorPicker->setColor(QColor(r, g, b));
      domElement = backgroundElement.firstChildElement("foreground-color");
      r = domElement.firstChildElement("red").text().toInt();
      g = domElement.firstChildElement("green").text().toInt();
      b = domElement.firstChildElement("blue").text().toInt();
      m_foreColorPicker->setColor(QColor(r, g, b));
    }
  }

  // CONTENT
  {
    // find the 'content' element
    QDomElement contentElement = canvasElement.firstChildElement("content");

    // create all content
    for(QDomElement ce = contentElement.firstChildElement(); !ce.isNull(); ce = ce.nextSiblingElement())
    {
      addContentFromXml(ce);
    }
  }

  // refresh all
  slotResetChanges();
  update();
  adjustSceneSize();
}

/// Drag & Drop image files
void Canvas::pasteFromClipboard()
{
  QString text = QApplication::clipboard()->text();
  if(text.trimmed().isEmpty()) return;

  QPoint pos = sceneCenter().toPoint();
  QGraphicsView * view = mainGraphicsView();
  if(view)
  {
    QPoint viewPoint = view->mapFromGlobal(QCursor::pos());
    pos = view->mapToScene(viewPoint).toPoint();
  }

  TextContent * t = createText(pos, true);
  t->setPlainText(text);
}

void Canvas::dragEnterEvent(QGraphicsSceneDragDropEvent * event)
{
  // dispatch to children but accept it only for image files
  QGraphicsScene::dragEnterEvent(event);
  event->ignore();

  // skip bad mimes
  if(!event->mimeData()) return;

  // check files drop
  if(event->mimeData()->hasUrls())
  {

    // get supported images extensions
    QStringList extensions;
    foreach(const QByteArray & format, QImageReader::supportedImageFormats()) extensions.append("." + format);

    // match local and remote urls against all supported extensions
    foreach(const QUrl & url, event->mimeData()->urls())
    {
      if(url.scheme() == "http" || url.scheme() == "ftp" || !url.toString().isEmpty())
      {
        QString urlString = url.toString();
        foreach(const QString & extension, extensions)
        {
          if(urlString.endsWith(extension, Qt::CaseInsensitive))
          {
            event->accept();
            return;
          }
        }
      }
    }
  }

  // check content drop
  if(event->mimeData()->hasFormat("picturesearch/idx"))
  {
    event->accept();
    return;
  }
}

void Canvas::dragMoveEvent(QGraphicsSceneDragDropEvent * event)
{
  // dispatch to children
  event->ignore();
  QGraphicsScene::dragMoveEvent(event);

  // or continue accepting event for this
  if(!event->isAccepted())
  {
    event->setDropAction(Qt::CopyAction);
    event->accept();
  }
}

void Canvas::dropEvent(QGraphicsSceneDragDropEvent * event)
{
  // handle by children
  event->ignore();
  QGraphicsScene::dropEvent(event);
  if(event->isAccepted()) return;

  // handle as an own file drop event
  if(event->mimeData()->hasUrls())
  {
    event->accept();
    QPoint pos = event->scenePos().toPoint();
    foreach(const QUrl & url, event->mimeData()->urls())
    {
      // handle network images (dropped from an extern application)
      if(url.scheme() == "http" || url.scheme() == "ftp")
      {
        // TODO: undo/redo for network pictures
        PictureContent * p = addNetworkPictureContent(url.toString());
        if(p != 0)
        {
          pos += QPoint(30, 30);
          p->setPos(pos);
        }
        else
        {
          m_content.removeAll(p);
          delete p;
        }
      }

      // handle local files
      QString picFilePath = url.toString(QUrl::RemoveScheme);
      if(QFile::exists(picFilePath))
      {
        PictureContent * p = createPicture(pos, true);
        if(!p->loadFromFile(picFilePath, true, true, true))
        {
          m_content.removeAll(p);
          delete p;
        }
      }
    }
    return;
  }

  // handle as an own content drop event
  if(event->mimeData()->hasFormat("picturesearch/idx") && App::pictureService)
  {

    // download each picture
    QPoint insertPos = event->scenePos().toPoint();
    QStringList sIndexes = QString(event->mimeData()->data("picturesearch/idx")).split(",");
    QStringList networkPaths;
    foreach(const QString & sIndex, sIndexes)
    {
      int index = sIndex.toUInt();

      // get picture description
      QString url;
      QString title;
      int width = 0;
      int height = 0;
      if(!App::pictureService->imageInfo(index, &url, &title, &width, &height)) continue;

      // get the download
      QNetworkReply * reply = App::pictureService->download(index);
      if(!reply) continue;

      networkPaths.append(url);
      // create PictureContent from network
      PictureContent * p = addNetworkPictureContent(url, reply, title, width, height);
      if(p != 0)
      {
        p->setPos(insertPos);
        insertPos += QPoint(30, 30);
      }
      else
      {
        m_content.removeAll(p);
        delete p;
      }
    }
    event->accept();
  }
}

void Canvas::keyPressEvent(QKeyEvent * keyEvent)
{
  QGraphicsScene::keyPressEvent(keyEvent);
  if(!keyEvent->isAccepted() && keyEvent->key() == Qt::Key_Delete) slotDeleteContent();
}

void Canvas::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
  foreach(QGraphicsItem * item, selectedItems())
  {
    AbstractContent * content = dynamic_cast<AbstractContent *>(item);
    if(!forceFieldEnabled() && content != nullptr) { content->setPreviousPos(content->pos()); }
  }
  AbstractScene::mousePressEvent(event);
}

void Canvas::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
  AbstractScene::mouseReleaseEvent(event);
  if(selectedItems().isEmpty()) return;

  m_commandStack->beginMacro(tr("Move Items"));
  bool moved = false;
  foreach(QGraphicsItem * item, selectedItems())
  {
    AbstractContent * content = dynamic_cast<AbstractContent *>(item);
    if(content != nullptr)
    {
      if(content->previousPos() != content->pos())
      {
        m_commandStack->push(new MotionCommand(content, content->previousPos(), content->pos()));
        moved = true;
      }
    }
  }
  m_commandStack->endMacro();
  if(!moved) m_commandStack->undo();
}

void Canvas::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * mouseEvent)
{
  // first dispatch doubleclick to items
  mouseEvent->ignore();
  QGraphicsScene::mouseDoubleClickEvent(mouseEvent);
  if(mouseEvent->isAccepted()) return;

  // unset the background picture, if present
  clearBackContent();
}

void Canvas::contextMenuEvent(QGraphicsSceneContextMenuEvent * event)
{
  // context menu on empty area
  // if (items(event->scenePos()).isEmpty()) {
  //}
  QGraphicsScene::contextMenuEvent(event);
}

/// Scene Background & Foreground
void Canvas::drawBackground(QPainter * painter, const QRectF & exposedRect)
{
  // clip exposedRect to the scene
  QRect sceneRect = this->sceneRect().toAlignedRect();
  QRect expRect = sceneRect.intersected(exposedRect.toAlignedRect());

  // draw background if have any uncovered area
  bool contentOnly = m_backContent && m_backContent->contentOpaque() && m_backContentRatio == Qt::IgnoreAspectRatio;
  if(!contentOnly)
  {
    switch(m_backMode)
    {
      case BackNone:
        // draw checkboard to simulate a transparent background
        if(!RenderOpts::ARGBWindow && !RenderOpts::HQRendering && !m_embeddedPainting)
          painter->drawTiledPixmap(expRect, m_backTile, QPointF(expRect.left() % 100, expRect.top() % 100));
        break;

      case BackBlack:
        painter->fillRect(expRect, Qt::black);
        break;

      case BackWhite:
        painter->fillRect(expRect, Qt::white);
        break;

      case BackGradient:
      {
        // draw background gradient
        QLinearGradient lg(0, 0, 0, sceneHeight());
        lg.setColorAt(0.0, m_grad1ColorPicker->color());
        lg.setColorAt(1.0, m_grad2ColorPicker->color());
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        painter->fillRect(expRect, lg);
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
      }
      break;
    }
  }

  // draw content if set
  if(m_backContent)
  {
    // do the HQ painting by hand, to improve quality on scaled views
    if(RenderOpts::HQRendering) m_backContent->drawContent(painter, sceneRect, m_backContentRatio);

    // use the lower quality cached (but faster) rendering
    else
    {
      // regenerate full-size cache if needed
      if(m_backCache.isNull() || m_backCache.size() != sceneSize())
        m_backCache = m_backContent->toPixmap(sceneSize(), m_backContentRatio);

      // paint cached background
      if(m_backContent->contentOpaque() && m_backContentRatio == Qt::IgnoreAspectRatio)
        painter->setCompositionMode(QPainter::CompositionMode_Source);
      painter->drawPixmap(expRect, m_backCache, expRect);
      painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    }
  }
}

void Canvas::drawForeground(QPainter * painter, const QRectF & exposedRect)
{
  // clip exposedRect to the scene
  QRect targetRect = sceneRect().toAlignedRect().intersected(exposedRect.toAlignedRect());

  // draw header and footer 50px bars
  if(m_topBarEnabled || m_bottomBarEnabled)
  {
    QColor hColor = m_foreColorPicker->color();
    hColor.setAlpha(128);
    if(m_topBarEnabled && targetRect.top() < 50)
      painter->fillRect(targetRect.left(), 0, targetRect.width(), 50, hColor);
    if(m_bottomBarEnabled && targetRect.bottom() >= sceneHeight() - 50)
      painter->fillRect(targetRect.left(), sceneHeight() - 50, targetRect.width(), 50, hColor);
  }

  // draw Title text
  if(!m_titleText.isEmpty())
  {
    painter->setFont(QFont("Courier 10 Pitch", 28));
    QLinearGradient lg(0, 15, 0, 35);
    QColor titleColor = m_titleColorPicker->color();
    lg.setColorAt(0.0, titleColor);
    lg.setColorAt(0.49, titleColor.lighter(150));
    lg.setColorAt(0.51, titleColor.darker(150));
    lg.setColorAt(1.0, titleColor);
    painter->setPen(QPen(lg, 0));
    painter->drawText(QRect(0, 0, sceneWidth(), 50), Qt::AlignCenter, m_titleText);
  }
}

void Canvas::initContent(AbstractContent * content, const QPoint & pos)
{
  // listen to AbstractContent signals. dangerous ops (deletion, editing) have queued connections
  connect(content, SIGNAL(changeStack(int)), this, SLOT(slotStackContent(int)));
  connect(content, SIGNAL(requestBackgrounding()), this, SLOT(slotBackgroundContent()));
  connect(content, SIGNAL(requestConfig(const QPoint &)), this, SLOT(slotConfigureContent(const QPoint &)));
  connect(content, SIGNAL(requestEditing()), this, SLOT(slotEditContent()), Qt::QueuedConnection);
  connect(content, SIGNAL(requestRemoval()), this, SLOT(slotDeleteContent()), Qt::QueuedConnection);

  if(!pos.isNull()) content->setPos(pos);
  content->setZValue(m_content.isEmpty() ? 1 : (m_content.last()->zValue() + 1));
  // content->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
  content->show();

  m_content.append(content);

  // mark the change
  slotMarkChanges();
}

void Canvas::setBackContent(AbstractContent * content)
{
  // skip if unchanged
  if(content == m_backContent) return;

  // re-show previous background
  if(m_backContent)
  {
    disconnect(m_backContent, SIGNAL(contentChanged()), this, SLOT(slotBackContentChanged()));
    m_backContent->show();
  }

  // hide content item
  m_backContent = content;
  if(m_backContent)
  {
    connect(m_backContent, SIGNAL(contentChanged()), this, SLOT(slotBackContentChanged()));
    m_backContent->hide();
  }

  // hide gradient controls
  m_grad1ColorPicker->hide();
  m_grad2ColorPicker->hide();

  // update GUI
  slotMarkChanges();
  m_backCache = QPixmap();
  update();
  emit backConfigChanged();
}

CanvasViewContent * Canvas::createCanvasView(const QPoint & pos, bool spontaneous)
{
  CanvasViewContent * d = new CanvasViewContent(spontaneous, this);
  initContent(d, pos);
  return d;
}

AbstractContent * Canvas::addContentFromXml(const QDomElement & contentElt)
{
  // create the right kind of content
  AbstractContent * content = 0;
  if(contentElt.tagName() == "picture")
    content = createPicture(QPoint(), false);
  else if(contentElt.tagName() == "text")
    content = createText(QPoint(), false);
  else if(contentElt.tagName() == "webcam")
    content = createWebcam(contentElt.attribute("input").toInt(), QPoint(), false);
  else if(contentElt.tagName() == "embedded-canvas")
    content = createCanvasView(QPoint(), false);
  else if(contentElt.tagName() == "wordcloud")
    content = createWordcloud(QPoint(), false);
  if(!content)
  {
    qWarning("Canvas::fromXml: unknown content type '%s'", qPrintable(contentElt.tagName()));
    return NULL;
  }

  // load item properties, and delete it if something goes wrong
  if(!content->fromXml(contentElt, m_fileAbsDir))
  {
    m_content.removeAll(content);
    delete content;
    return NULL;
  }

  // restore the background element of the canvas
  if(contentElt.firstChildElement("set-as-background").isElement())
  {
    if(m_backContent)
      qWarning("Canvas::fromXml: only 1 element with <set-as-background/> allowed");
    else
      setBackContent(content);
  }
  return content;
}

PictureContent * Canvas::createPicture(const QPoint & pos, bool spontaneous)
{
  PictureContent * p = new PictureContent(spontaneous, this);
  initContent(p, pos);
  connect(p, SIGNAL(flipHorizontally()), this, SLOT(slotFlipHorizontally()));
  connect(p, SIGNAL(flipVertically()), this, SLOT(slotFlipVertically()));
  connect(p, SIGNAL(requestCrop()), this, SLOT(slotCrop()));
  return p;
}

TextContent * Canvas::createText(const QPoint & pos, bool spontaneous)
{
  TextContent * t = new TextContent(spontaneous, this);
  initContent(t, pos);
  return t;
}

WebcamContent * Canvas::createWebcam(int webcamIndex, const QPoint & pos, bool spontaneous)
{
  WebcamContent * w = new WebcamContent(webcamIndex, spontaneous, this);
  initContent(w, pos);
  return w;
}

WordcloudContent * Canvas::createWordcloud(const QPoint & pos, bool spontaneous)
{
  WordcloudContent * w = new WordcloudContent(spontaneous, this);
  initContent(w, pos);
  return w;
}

void Canvas::deleteContent(AbstractContent * content)
{
  if(content)
  {
    // unset background if deleting its content
    if(m_backContent == content) setBackContent(0);

    // remove related property if deleting its content
    foreach(AbstractConfig * config, m_configs)
    {
      if(config->content() == content)
      {
        deleteConfig(config);
        break;
      }
    }

    // unlink content from lists, myself(the Scene) and memory
    m_content.removeAll(content);
    content->dispose();
  }
}

void Canvas::deleteConfig(AbstractConfig * config)
{
  if(config)
  {
    m_configs.removeAll(config);
    config->dispose();
  }
}

QGraphicsView * Canvas::mainGraphicsView() const
{
  QList<QGraphicsView *> vList = views();
  return vList.isEmpty() ? 0 : vList.first();
}

/// Slots
void Canvas::slotSelectionChanged()
{
  // show the config widget if 1 AbstractContent is selected
  QList<QGraphicsItem *> selection = selectedItems();
  if(selection.size() == 1)
  {
    AbstractContent * content = dynamic_cast<AbstractContent *>(selection.first());
    if(content)
    {
      QWidget * pWidget = content->createPropertyWidget();
      if(pWidget) pWidget->setWindowTitle(tr("%1").arg(content->contentName().toUpper()));
      emit showPropertiesWidget(pWidget);
      return;
    }
  }

  // show a 'selection' properties widget
  QList<AbstractContent *> selectedContent = projectList<QGraphicsItem, AbstractContent>(selection);
  if(!selectedContent.isEmpty())
  {
    SelectionProperties * pWidget = new SelectionProperties(selectedContent);
    connect(pWidget, SIGNAL(collateSelection()), this, SLOT(slotCollateContent()));
    connect(pWidget, SIGNAL(deleteSelection()), this, SLOT(slotDeleteContent()), Qt::QueuedConnection);
    emit showPropertiesWidget(pWidget);
    return;
  }

  // or don't show anything
  emit showPropertiesWidget(0);
}

void Canvas::slotBackgroundContent()
{
  AbstractContent * back = dynamic_cast<AbstractContent *>(sender());
  auto * command = new BackgroundContentCommand(this, m_backContent, back);
  command->redo();
  m_commandStack->push(command);
}

void Canvas::slotConfigureContent(const QPoint & scenePoint)
{
#if defined(MOBILE_UI)
  Q_UNUSED(scenePoint)
#endif
  // get the content and ensure it hasn't already a property window
  AbstractContent * content = dynamic_cast<AbstractContent *>(sender());
  foreach(AbstractConfig * config, m_configs)
  {
    if(config->content() == content) return;
    // force only 1 property instance
    deleteConfig(config);
  }
  AbstractConfig * p = 0;

  // picture config (dialog and connections)
  if(PictureContent * picture = dynamic_cast<PictureContent *>(content))
  {
    p = new PictureConfig(picture);
    connect(p, SIGNAL(applyEffect(const PictureEffect &, bool)), this,
            SLOT(slotApplyEffect(const PictureEffect &, bool)));
  }

  // text config (dialog and connections)
  if(TextContent * text = dynamic_cast<TextContent *>(content)) p = new TextConfig(text);

  // generic config
  if(!p) p = new AbstractConfig(content);

  // common links
  m_configs.append(p);
#if !defined(MOBILE_UI)
  addItem(p);
#endif
  connect(p, SIGNAL(requestClose()), this, SLOT(slotDeleteConfig()));
  connect(p, SIGNAL(applyLook(quint32, bool, bool)), this, SLOT(slotApplyLook(quint32, bool, bool)));
#if !defined(MOBILE_UI)
  p->show();
  p->setPos(scenePoint - QPoint(10, 10));
#else
  p->show();
  // p->showFullScreen();
#endif
  QGraphicsView * mainView = mainGraphicsView();
  if(mainView)
  {
    QRect bounds = mainView->mapToScene(mainView->viewport()->rect()).boundingRect().toRect();
    p->keepInBoundaries(bounds);
  }
  else
    p->keepInBoundaries(sceneRect().toRect());
  p->setFocus();
}

void Canvas::slotEditContent()
{
  // get content
  AbstractContent * content = dynamic_cast<AbstractContent *>(sender());
  if(content)
  {
    // handle internally if possible
    // ...

    // or handle externally
    emit requestContentEditing(content);
  }
}

void Canvas::slotStackContent(int op)
{
  AbstractContent * content = dynamic_cast<AbstractContent *>(sender());
  if(!content || m_content.size() < 2) return;
  int size = m_content.size();
  int index = m_content.indexOf(content);

  // find out insertion indexes over the stacked items
  QList<QGraphicsItem *> stackedItems = items(content->sceneBoundingRect(), Qt::IntersectsItemShape);
  int prevIndex = 0;
  int nextIndex = size - 1;
  foreach(QGraphicsItem * item, stackedItems)
  {
    // operate only on different Content
    AbstractContent * c = dynamic_cast<AbstractContent *>(item);
    if(!c || c == content) continue;

    // refine previous/next indexes (close to 'index')
    int cIdx = m_content.indexOf(c);
    if(cIdx < nextIndex && cIdx > index)
      nextIndex = cIdx;
    else if(cIdx > prevIndex && cIdx < index)
      prevIndex = cIdx;
  }

  // move items
  switch(op)
  {
    case 1: // front
      m_content.append(m_content.takeAt(index));
      break;
    case 2: // raise
      if(index >= size - 1) return;
      m_content.insert(nextIndex, m_content.takeAt(index));
      break;
    case 3: // lower
      if(index <= 0) return;
      m_content.insert(prevIndex, m_content.takeAt(index));
      break;
    case 4: // back
      m_content.prepend(m_content.takeAt(index));
      break;
  }

  // reassign z-levels
  int z = 1;
  m_commandStack->beginMacro(tr("Change content stack order"));
  foreach(AbstractContent * content, m_content)
  {
    auto * command = new StackCommand(content, content->zValue(), z++);
    m_commandStack->push(command);
  }
  m_commandStack->endMacro();
}

void Canvas::slotCollateContent()
{
  QList<AbstractContent *> selectedContent = projectList<QGraphicsItem, AbstractContent>(selectedItems());
  if(selectedContent.isEmpty()) return;

  // TODO implement collation
#if 0
    QGraphicsItemGroup * group = new QGraphicsItemGroup;
    foreach (AbstractContent * content, selectedContent)
    group->addToGroup(content);
    group->setFlags(QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
    addItem(group);
#endif
}

void Canvas::slotDeleteContent()
{
  QList<AbstractContent *> selectedContent = projectList<QGraphicsItem, AbstractContent>(selectedItems());
  AbstractContent * senderContent = dynamic_cast<AbstractContent *>(sender());
  if(senderContent && !selectedContent.contains(senderContent))
  {
    selectedContent.clear();
    selectedContent.append(senderContent);
  }
  if(selectedContent.size() > 1)
    if(QMessageBox::question(
           0, tr("Delete content"),
           tr("All the %1 selected content will be deleted, do you want to continue ?").arg(selectedContent.size()),
           QMessageBox::Yes | QMessageBox::No)
       == QMessageBox::No)
      return;

  // Undo/redo delete code
  m_commandStack->beginMacro(tr("Delete %1 content").arg(selectedContent.size()));
  for(const auto content : selectedContent)
  {
    auto * command = new DeleteContentCommand(content, this);
    m_commandStack->push(command);
  }
  m_commandStack->endMacro();
}

void Canvas::slotDeleteConfig()
{
  deleteConfig(dynamic_cast<AbstractConfig *>(sender()));
}

void Canvas::slotApplyLook(quint32 frameClass, bool mirrored, bool all)
{
  QList<AbstractContent *> selectedContent = projectList<QGraphicsItem, AbstractContent>(selectedItems());
  m_commandStack->beginMacro(tr("Frame Look"));
  bool frameChanged = false;
  foreach(AbstractContent * content, m_content)
  {
    if(all || content->isSelected())
    {
      auto * command = new FrameCommand(content, frameClass, mirrored);
      if(command->hasEffect())
      {
        frameChanged = true;
        command->redo();
        m_commandStack->push(command);
      }
    }
  }
  m_commandStack->endMacro();
  if(!frameChanged) m_commandStack->undo();
}

void Canvas::slotApplyEffect(const PictureEffect & effect, bool all)
{
  QList<PictureContent *> pictures = projectList<AbstractContent, PictureContent>(m_content);
  m_commandStack->beginMacro(tr("Change effects"));
  bool changed = false;
  foreach(PictureContent * picture, pictures)
  {
    if(all || picture->isSelected())
    {
      auto * command = new EffectCommand(picture, effect);
      m_commandStack->push(command);
      changed = true; // XXX: should detect if we do an actual effect change or a no-op
    }
  }
  m_commandStack->endMacro();
  if(!changed) m_commandStack->undo();
}

void Canvas::slotCrop()
{
  QList<PictureContent *> pictures = projectList<QGraphicsItem, PictureContent>(selectedItems());
  foreach(PictureContent * picture, pictures) { picture->crop(); }
}

void Canvas::slotFlipHorizontally()
{
  QList<PictureContent *> pictures = projectList<QGraphicsItem, PictureContent>(selectedItems());
  if(pictures.isEmpty()) return;

  m_commandStack->beginMacro(tr("Horizontally flip pictures"));
  foreach(PictureContent * picture, pictures)
  {
    auto * command = new EffectCommand(picture, PictureEffect::FlipH);
    m_commandStack->push(command);
  }
  m_commandStack->endMacro();
}

void Canvas::slotFlipVertically()
{
  QList<PictureContent *> pictures = projectList<QGraphicsItem, PictureContent>(selectedItems());
  if(pictures.isEmpty()) return;

  m_commandStack->beginMacro(tr("Vertically flip pictures"));
  foreach(PictureContent * picture, pictures)
  {
    auto * command = new EffectCommand(picture, PictureEffect::FlipV);
    m_commandStack->push(command);
  }
  m_commandStack->endMacro();
}

void Canvas::slotTitleColorChanged()
{
  update(0, 0, sceneWidth(), 50);
}

void Canvas::slotForeColorChanged()
{
  update(0, 0, sceneWidth(), 50);
  update(0, sceneHeight() - 50, sceneWidth(), 50);
}

void Canvas::slotGradColorChanged()
{
  update();
}

void Canvas::slotBackContentChanged()
{
  m_backCache = QPixmap();
  update();
}

void Canvas::slotMarkChanges()
{
  m_pendingChanges = true;
}

void Canvas::slotResetChanges()
{
  m_pendingChanges = false;
}

void Canvas::slotApplyForce()
{
  // initial consts
  const QRectF sRect = sceneRect();
  if(sRect.width() < 10 || sRect.height() < 10) return;
  const qreal W = sRect.width();
  const qreal H = sRect.height();
  const qreal dT = 4.0 * qBound((qreal)0.001, (qreal)m_forceFieldTime.restart() / (qreal)1000.0, (qreal)0.10);

  // pass 0
  QList<AbstractContent *>::iterator it1, it2, end = m_content.end();
  for(it1 = m_content.begin(); it1 != end; ++it1)
  {
    AbstractContent * t = *it1;
    t->vPos = Vector2(t->pos().x(), t->pos().y());
    double fx = W / (t->vPos.x() - sRect.left() + 10.0) + W / (t->vPos.x() - sRect.right() - 10.0);
    double fy = H / (t->vPos.y() - sRect.top() + 10.0) + H / (t->vPos.y() - sRect.bottom() - 10.0);
    t->vForce = Vector2(fx, fy);
  }

  // pass 1: item-vs-item force
  for(it1 = m_content.begin(); it1 != end; ++it1)
  {
    for(it2 = m_content.begin(); it2 != end; ++it2)
    {
      AbstractContent * t = *it1;
      AbstractContent * s = *it2;
      Vector2 r = t->vPos - s->vPos;
      double mod = r.module();
      if(mod > 0.707)
      {
        r *= s->boundingRect().width() / (mod * mod);
        t->vForce += r;
      }
    }
  }

  // pass 2: apply force
  for(it1 = m_content.begin(); it1 != end; ++it1)
  {
    AbstractContent * t = *it1;
    if(t->isSelected()) continue;

    Vector2 vStart = t->vVel;

    // add friction
    t->vForce += -0.2 * t->vVel;

    t->vVel += t->vForce * dT;
    t->vPos += (vStart + t->vVel) * dT / 2.0;
    t->setPos(t->vPos.x(), t->vPos.y());
  }
}

void Canvas::undoSlot()
{
  // qDebug() << "[Canvas] Undo command: " << commandStack().undoText();
  commandStack().undo();
}

void Canvas::redoSlot()
{
  // qDebug() << "[Canvas] Redo command: " << commandStack().redoText();
  commandStack().redo();
}

void Canvas::pushCurrentContentPositionToCommandStack()
{
  commandStack().beginMacro("Save current content position");
  for(const auto & c : m_content) { commandStack().push(new MotionCommand(c, c->pos(), c->pos())); }
  commandStack().endMacro();
}
