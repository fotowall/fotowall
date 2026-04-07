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

#include "FrameFactory.h"

#include "HeartFrame.h"
#include "PlasmaFrame.h"
#include "StandardFrame.h"

#include <QDirIterator>
#include <QFile>
#include <QSettings>

Q_GLOBAL_STATIC(FrameFactory, factoryInstance)
#define FRAME_SVG_1 ":/plasma-frames/1.svg"
#define FRAME_SVG_2 ":/plasma-frames/2.svg"
#define FRAME_SVG_3 ":/plasma-frames/3.svg"
#define FRAME_SVG_4 ":/plasma-frames/4.svg"

// STATICS
QList<quint32> FrameFactory::classes()
{
  QList<quint32> classes = factoryInstance()->m_svgMap.keys();
  classes.prepend(Frame::HeartFrame);
  classes.prepend(Frame::StandardFrame2);
  classes.prepend(Frame::StandardFrame);
  classes.prepend(Frame::NoFrame);
  return classes;
}

Frame * FrameFactory::createFrame(quint32 frameClass)
{
  if(frameClass == Frame::NoFrame)
    return 0;
  else if(frameClass == Frame::StandardFrame2)
    return new StandardFrame2();
  else if(frameClass == Frame::StandardFrame)
    return new StandardFrame();
  else if(frameClass == Frame::HeartFrame)
    return new HeartFrame();
  else if(factoryInstance()->m_svgMap.contains(frameClass))
    return new PlasmaFrame(frameClass, factoryInstance()->m_svgMap[frameClass]);
  qWarning("FrameFactory::createFrame: unknown frame for class %x, falling back to default", frameClass);
  return 0;
}

Frame * FrameFactory::defaultPanelFrame()
{ return createFrame(factoryInstance()->m_defaultPanel); }

Frame * FrameFactory::defaultPictureFrame()
{ return createFrame(factoryInstance()->m_defaultPicture); }

void FrameFactory::addSvgFrame(const QString & frameFilePath)
{ factoryInstance()->m_svgMap[factoryInstance()->m_svgClassIndex++] = frameFilePath; }

void FrameFactory::removeFrame(const quint32 frameClass)
{ factoryInstance()->m_svgMap.remove(frameClass); }

quint32 FrameFactory::defaultPanelClass()
{ return factoryInstance()->m_defaultPanel; }

void FrameFactory::setDefaultPanelClass(quint32 frameClass)
{ factoryInstance()->m_defaultPanel = frameClass; }

quint32 FrameFactory::defaultPictureClass()
{ return factoryInstance()->m_defaultPicture; }

void FrameFactory::setDefaultPictureClass(quint32 frameClass)
{ factoryInstance()->m_defaultPicture = frameClass; }

// class impl
FrameFactory::FrameFactory() : m_defaultPanel(Frame::BasePlasmaFrame), m_defaultPicture(Frame::StandardFrame)
{
  // init default frames
  m_svgClassIndex = Frame::BasePlasmaFrame;
  QDirIterator dIt(":/plasma-frames", QDir::Files | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
  while(dIt.hasNext()) m_svgMap[m_svgClassIndex++] = dIt.next();

  // add stored frames
  QSettings s;
  int values = s.value("frames/count", 0).toInt();
  for(int i = 0; i < values; i++)
  {
    QString frameFilePath = s.value(QString("frames/frame%1").arg(i)).toString();
    if(QFile::exists(frameFilePath)) m_svgMap[m_svgClassIndex++] = frameFilePath;
  }
}

FrameFactory::~FrameFactory()
{
  QSettings s;

  // store each path
  int count = 0;
  QMap<quint32, QString>::iterator it = m_svgMap.begin(), end = m_svgMap.end();
  for(; it != end; ++it)
  {
    QString frameFilePath = *it;
    if(frameFilePath.startsWith(":/")) continue;
    s.setValue(QString("frames/frame%1").arg(count++), frameFilePath);
  }
  // store the count
  if(count) s.setValue("frames/count", count);
}
