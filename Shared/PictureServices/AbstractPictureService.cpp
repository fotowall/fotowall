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

#include "AbstractPictureService.h"

#include <QNetworkAccessManager>

AbstractPictureService::AbstractPictureService(QNetworkAccessManager * manager, QObject * parent)
: QObject(parent), m_nam(manager)
{
}

QNetworkReply * AbstractPictureService::get(const QNetworkRequest & request)
{ return m_nam->get(request); }

QNetworkReply * AbstractPictureService::get(const QUrl & url)
{
  QNetworkRequest request(url);
  return m_nam->get(request);
}
