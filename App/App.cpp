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

#include "App.h"

#include <QFile>
#include <QImageReader>

/// Global Instances

Settings * App::settings = 0;
Workflow * App::workflow = 0;
OnlineServices * App::onlineServices = 0;
AbstractPictureService * App::pictureService = 0;

/// Functions

QString App::supportedImageFormats()
{
  static QString extensions;
  if(extensions.isEmpty())
    foreach(const QByteArray & format, QImageReader::supportedImageFormats())
      extensions += "*." + format + " *." + format.toUpper() + " ";
  return extensions;
}

bool App::isPictureFile(const QString & picFilePath)
{
  QString extension = picFilePath.section('.', -1);
  foreach(const QByteArray & format, QImageReader::supportedImageFormats())
    if(!extension.compare(QString(format), Qt::CaseInsensitive)) return true;
  return false;
}

bool App::isFotowallFile(const QString & fwFilePath)
{ return fwFilePath.endsWith(".fotowall", Qt::CaseInsensitive); }

bool App::isContentUrl(const QString & url)
{
  // check file domain
  if(QFile::exists(url) || url.startsWith("http:/", Qt::CaseInsensitive)
     || url.startsWith("ftp:/", Qt::CaseInsensitive))
  {

    // check extension
    if(isFotowallFile(url) || isPictureFile(url)) return true;
  }

  // unsupported local or remote content url
  return false;
}

bool App::validateFotowallUrl(const QString & url)
{
  if(isFotowallFile(url) && QFile::exists(url)) return true;
  if(url.startsWith("http:/", Qt::CaseInsensitive) || url.startsWith("ftp:/", Qt::CaseInsensitive))
  {
    qWarning("App::validateFotowallUrl: http or ftp urls not allowed for now");
    return false;
  }
  return false;
}
