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

#include "CanvasViewContent.h"

// ### FIXME! bad crossing!
#include "App/FotowallFile.h"
#include "Canvas.h"

#include <QDebug>
#include <QFileInfo>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QPainter>

CanvasViewContent::CanvasViewContent(bool spontaneous, QGraphicsScene * scene, QGraphicsItem * parent)
: AbstractContent(scene, spontaneous, false, parent), m_canvas(0), m_canvasTaken(false)
{
}

CanvasViewContent::~CanvasViewContent()
{
  if(m_canvasTaken) qWarning("CanvasViewContent::~CanvasViewContent: canvas still exported");
  delete m_canvas;
}

bool CanvasViewContent::loadFromFile(const QString & fwFilePath, bool /*keepRatio*/, bool setName)
{
  // create a Canvas
  Canvas * canvas = new Canvas(96, 96, this);
  connect(canvas, SIGNAL(changed(const QList<QRectF> &)), this, SLOT(slotRepaintCanvas(const QList<QRectF> &)));
  bool ok = FotowallFile::read(fwFilePath, canvas, false);
  canvas->resizeAutoFit();

  // set the canvas
  m_canvas = canvas;
  m_canvas->setEmbeddedPainting(true);
  m_canvasCachedSize = m_canvas->sceneSize();
  resizeContents(contentRect(), true);
  update();

  // customize the item
  setFrameTextEnabled(setName);
  setFrameText(QFileInfo(fwFilePath).baseName());
  return ok;
}

bool CanvasViewContent::fromXml(const QDomElement & contentElement, const QDir & baseDir)
{
  AbstractContent::fromXml(contentElement, baseDir);

  // sanity check
  if(m_canvas)
  {
    qWarning("CanvasViewContent::fromXml: canvas already set. skipping.");
    return false;
  }

  // get the canvas element
  QDomElement canvasElement = contentElement.firstChildElement("canvas");
  if(!canvasElement.isElement())
  {
    qWarning("CanvasViewContent::fromXml: no embedded canvas element in here");
    return false;
  }

  // restore canvas from the element
  m_canvas = new Canvas(96, 96, this);
  m_canvas->setEmbeddedPainting(true);
  connect(m_canvas, SIGNAL(changed(const QList<QRectF> &)), this, SLOT(slotRepaintCanvas(const QList<QRectF> &)));
  m_canvas->loadFromXml(canvasElement);
  m_canvas->resizeAutoFit();

  // set the canvas
  m_canvasCachedSize = m_canvas->sceneSize();
  update();

  // TODO: use the canvas name as text for the text frame
  // setFrameTextEnabled(setName);
  // setFrameText(QFileInfo(filePath).baseName());
  return true;
}

void CanvasViewContent::toXml(QDomElement & contentElement, const QDir & baseDir) const
{
  // save AbstractContent properties and rename to 'embedded-canvas'
  AbstractContent::toXml(contentElement, baseDir);
  contentElement.setTagName("embedded-canvas");

  // sanity check
  if(!m_canvas)
  {
    qWarning("CanvasViewContent::toXml: can't save canvas to xml, taken or not present");
    return;
  }

  // save embedded canvas
  QDomDocument doc = contentElement.ownerDocument();
  QDomElement canvasElement = doc.createElement("canvas");
  contentElement.appendChild(canvasElement);
  m_canvas->saveToXml(canvasElement);
}

void CanvasViewContent::drawContent(QPainter * painter, const QRect & targetRect, Qt::AspectRatioMode ratio)
{
  Q_UNUSED(ratio)

  // shouldn't paint if canvas is taken.. use a scary red
  if(m_canvasTaken)
  {
    painter->fillRect(targetRect, Qt::red);
    return;
  }

  // render canvas
  if(m_canvas) m_canvas->render(painter, targetRect, m_canvas->sceneRect(), Qt::IgnoreAspectRatio);
}

int CanvasViewContent::contentHeightForWidth(int width) const
{
  // keep aspect ratio of the canvas
  if(m_canvas && m_canvas->width() > 0) return (m_canvas->height() * width) / m_canvas->width();
  if(m_canvasCachedSize.width() > 0) return (m_canvasCachedSize.height() * width) / m_canvasCachedSize.width();
  return width;
}

QVariant CanvasViewContent::takeResource()
{
  // sanity check
  if(m_canvasTaken)
  {
    qWarning("CanvasViewContent::takeResource: already taken");
    return QVariant();
  }

  // discard reference
  m_canvas->setEmbeddedPainting(false);
  m_canvasTaken = true;
  Canvas * canvas = m_canvas;
  m_canvas = 0;
  update();
  return QVariant::fromValue((void *)canvas);
}

void CanvasViewContent::returnResource(const QVariant & resource)
{
  // sanity checks
  if(!m_canvasTaken) qWarning("CanvasViewContent::returnResource: not taken");
  if(m_canvas)
  {
    qWarning("CanvasViewContent::returnResource: we already have one canvas, shouldn't return one");
    delete m_canvas;
  }

  // store reference
  m_canvas = static_cast<Canvas *>(resource.value<void *>());
  m_canvas->setEmbeddedPainting(true);
  m_canvasCachedSize = m_canvas->sceneSize();
  m_canvasTaken = false;
  resizeContents(contentRect(), true);
  update();
}

void CanvasViewContent::slotRepaintCanvas(const QList<QRectF> &)
{ update(); }
