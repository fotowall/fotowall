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

#include "HelpItem.h"
#if defined(USE_QTWEBKIT)
#  include "Shared/BrowserItem.h"
#else
#  include <QGraphicsTextItem>
#endif
#include "Frames/FrameFactory.h"
#include <QCoreApplication>
#include <QGraphicsSceneMouseEvent>
#include <QLocale>
#include <QPainter>
#include <QString>

HelpItem::HelpItem(QGraphicsItem * parent)
: AbstractDisposeable(true, parent), m_frame(FrameFactory::createFrame(0x1001 /*HARDCODED*/))
{
  QString introduction =
      tr("<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n"
         "<html><!--head><meta name=\"qrichtext\" content=\"1\" /></head-->\n"
         "<body style=\"font-family:'Sans Serif';\">\n"
         "   <span style='font-size: 26px; font-weight: normal; color: #FF0000;'>\n"
         "      <img src=':/data/action-remove.png'>\n"
         "        Welcome\n"
         "      <img src=':/data/action-remove.png'>\n"
         "   </span>\n"
         "   <br />\n"
         "   <div style='font-size: 16px; font-weight: normal;'>\n"
         "      <b>Playing</b> with <b>Fotowall</b> is easy\n"
         "   </div>\n"
         "   <ul>\n"
         "      <li><img src=':/data/action-add.png'><b> add contents</b> using the <i><span style='background-color: "
         "rgb(255, 226, 0);'>ADD NEW</span></i> buttons or <i><span style='background-color: rgb(255, 226, 0);'>drop "
         "them</span></i></li>\n"
         "      <li><img src=':/data/action-move.png'><b> move contents around </b> and <b><img "
         "src=':/data/action-rotate.png'> rotate them</b> <span style='background-color: rgb(255, 226, 0);'>by "
         "<i>dragging the corners</i></span></li>\n"
         "      <li><img src=':/data/action-configure.png'><i> <span style='background-color: rgb(255, 226, 0);'>right "
         "click on the objects</span></i> or <i><span style='background-color: rgb(255, 226, 0);'>use the top "
         "bar</span></i> to <b>modify properties</b></li>\n"
         "      <li><span style='background-color: rgb(255, 226, 0);'><i>double click any content</i></span> to "
         "<b>open the editor</b> or set it as <b>background</b></li>\n"
         "      <li>...and we'll let you discover the rest!</li>\n"
         "   </ul>\n"
         "   <div><font color='#2B2B2B'><b>\n"
         "      Please make sure to share your best work with your friends ;-)\n"
         "   </b></font></div>\n"
         "   <br/>\n"
         "   <div><font color='#616161' style='font-size: 11px; font-weight: normal;'>\n"
         "      Fotowall is licensed under LGPL 3.0, by Enrico Ros, 2009-2017.\n"
         "   </font></div>\n"
         "</body>\n"
         "</html>");
#if defined(USE_QTWEBKIT)
  // show fancy help in internal browser
#  error WebKit was completely disabled, to only have 1 version of the HTML to translate...
  // BrowserItem * bi = new BrowserItem(this);
  // bi->setGeometry(...);
  // bi->write(introduction);
  // bi->setReadOnly(true);
#else
  // create an item to display it
  QGraphicsTextItem * ti = new QGraphicsTextItem(this);
  ti->setHtml(introduction);
  ti->setPos(boundingRect().adjusted(12, 12, -12, -12).topLeft());
  ti->setTextInteractionFlags(Qt::NoTextInteraction);
#endif
}

HelpItem::~HelpItem()
{ delete m_frame; }

QRectF HelpItem::boundingRect() const
{
  QRectF r = childrenBoundingRect().adjusted(-12, -12, 12, 12);
  r.moveCenter(QPointF());
  return r;
}

void HelpItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
  if(m_frame) m_frame->drawFrame(painter, boundingRect().toRect(), false, false);
}

void HelpItem::mousePressEvent(QGraphicsSceneMouseEvent * /*event*/)
{ emit closeMe(); }
