/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2009 by TANGUY Arnaud <arn.tanguy@gmail.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "CanvasModeInfo.h"

#define DEFAULT_SCREEN_DPI 96
#define DEFAULT_PRINT_DPI 300

CanvasModeInfo::CanvasModeInfo()
: m_canvasDpi(DEFAULT_SCREEN_DPI, DEFAULT_SCREEN_DPI), m_printDpi(DEFAULT_PRINT_DPI), m_landscape(false),
  m_projectMode(ModeNormal), m_previousProjectMode(ModeNormal)
{
}

void CanvasModeInfo::setFixedSizeInches(const QSizeF & size)
{
  // 1 inch = 2.54 cm
  // w /= (float)2.54; h /= (float)2.54;
  // m_realSizeInches = QSizeF(w, h);
  m_realSizeInches = size;
}

bool CanvasModeInfo::fixedSize() const
{ return !m_realSizeInches.isEmpty(); }

QSizeF CanvasModeInfo::fixedSizeInches() const
{ return m_realSizeInches; }

QSize CanvasModeInfo::fixedScreenPixels() const
{ return QSize(m_realSizeInches.width() * m_canvasDpi.x(), m_realSizeInches.height() * m_canvasDpi.y()); }

QSize CanvasModeInfo::fixedPrinterPixels() const
{ return QSize(m_realSizeInches.width() * m_printDpi, m_realSizeInches.height() * m_printDpi); }

void CanvasModeInfo::setScreenDpi(float dpiX, float dpiY)
{ m_canvasDpi = QPointF(dpiX, dpiY); }

QPointF CanvasModeInfo::screenDpi() const
{ return m_canvasDpi; }

void CanvasModeInfo::setPrintDpi(float dpi)
{
  if(dpi > 0) m_printDpi = dpi;
}

float CanvasModeInfo::printDpi() const
{ return m_printDpi; }

void CanvasModeInfo::setPrintLandscape(bool landscape)
{ m_landscape = landscape; }

bool CanvasModeInfo::printLandscape() const
{ return m_landscape; }

void CanvasModeInfo::setProjectMode(Mode mode)
{
  m_previousProjectMode = m_projectMode;
  m_projectMode = mode;
}

CanvasModeInfo::Mode CanvasModeInfo::projectMode() const
{ return m_projectMode; }

CanvasModeInfo::Mode CanvasModeInfo::previousProjectMode() const
{ return m_previousProjectMode; }

void CanvasModeInfo::toXml(QDomElement & canvasModeElement) const
{
  QDomDocument doc = canvasModeElement.ownerDocument();

  // <mode><id>m_projectmode</id>...
  QDomElement modeIdElement = doc.createElement("id");
  canvasModeElement.appendChild(modeIdElement);
  modeIdElement.appendChild(doc.createTextNode(QString::number((int)m_projectMode)));

  // additional saving if not ModeNormal
  if(!m_realSizeInches.isEmpty())
  {
    QDomElement modeSizeElement = doc.createElement("size");
    canvasModeElement.appendChild(modeSizeElement);
    QDomElement wElement = doc.createElement("w");
    modeSizeElement.appendChild(wElement);
    wElement.appendChild(doc.createTextNode(QString::number(m_realSizeInches.width())));
    QDomElement hElement = doc.createElement("h");
    modeSizeElement.appendChild(hElement);
    hElement.appendChild(doc.createTextNode(QString::number(m_realSizeInches.height())));
    QDomElement dpiXElement = doc.createElement("dpiX");
    modeSizeElement.appendChild(dpiXElement);
    dpiXElement.appendChild(doc.createTextNode(QString::number(m_canvasDpi.x())));
    QDomElement dpiYElement = doc.createElement("dpiY");
    modeSizeElement.appendChild(dpiYElement);
    dpiYElement.appendChild(doc.createTextNode(QString::number(m_canvasDpi.y())));
  }

  // printing information
  if(m_printDpi != DEFAULT_PRINT_DPI || m_landscape)
  {
    QDomElement printElement = doc.createElement("print");
    canvasModeElement.appendChild(printElement);
    QDomElement dpiElement = doc.createElement("dpi");
    printElement.appendChild(dpiElement);
    dpiElement.appendChild(doc.createTextNode(QString::number(m_printDpi)));
    QDomElement landscapeElement = doc.createElement("landscape");
    printElement.appendChild(landscapeElement);
    landscapeElement.appendChild(doc.createTextNode(QString::number(m_landscape)));
  }
}

void CanvasModeInfo::fromXml(QDomElement & canvasModeElement)
{
  // eventual size and dpi
  QDomElement sizeElement = canvasModeElement.firstChildElement("size");
  if(!sizeElement.isNull())
  {
    float w = sizeElement.firstChildElement("w").text().toFloat();
    float h = sizeElement.firstChildElement("h").text().toFloat();
    m_realSizeInches = QSizeF(w, h);
    float dpiX = sizeElement.firstChildElement("dpiX").text().toFloat();
    float dpiY = sizeElement.firstChildElement("dpiY").text().toFloat();
    if(dpiX > 1 && dpiY > 1) m_canvasDpi = QPointF(dpiX, dpiY);
  }

  // printing information
  QDomElement printElement = canvasModeElement.firstChildElement("print");
  if(!printElement.isNull())
  {
    m_printDpi = printElement.firstChildElement("dpi").text().toDouble();
    m_landscape = printElement.firstChildElement("landscape").text().toInt();
  }

  // restore mode
  int mode = canvasModeElement.firstChildElement("id").text().toInt();
  m_projectMode = (CanvasModeInfo::Mode)mode;
  // ### notify the changes?
}
