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

#ifndef __BreadCrumbBar_h__
#define __BreadCrumbBar_h__

#include <QWidget>
struct InternalNode;

class BreadCrumbBar : public QWidget
{
  Q_OBJECT
public:
  BreadCrumbBar(QWidget * parent = 0);

  enum
  {
    InvalidNode = 0
  };
  quint32 addNode(quint32 id, const QString & text, quint32 parentId = 0);
  void deleteNode(quint32 id);
  void clearNodes();

  // enable clickable leaves, active by default
  void setClickableLeaves(bool);
  bool clickableLeaves() const;

  // enable background drawing, active by default
  void setDrawBackground(bool);
  bool drawBackground() const;

  // invert background offset (used by right-side bcbars)
  void setBackgroundOffset(int side = 0);
  int backgroundOffset() const;

  // ::QWidget
  void paintEvent(QPaintEvent * event);

Q_SIGNALS:
  void nodeClicked(quint32 id);

private:
  void processLayout();
  InternalNode * m_root;
  bool m_clickableLeaves;
  bool m_drawBackground;
  int m_backgroundOffset;

private Q_SLOTS:
  void slotLabelClicked(quint32 id);
  void slotExpanderClicked(quint32 id);
};

// Move the following classes to a _p.h
#include <QLabel>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
using EnterEvent = QEnterEvent;
#else
using EnterEvent = QEvent;
#endif

class BcLabel : public QLabel
{
  Q_OBJECT
public:
  BcLabel(quint32 labelId, QWidget * parent = 0);

  void setLast(bool last);
  bool last() const;

  // ::QWidget
  void enterEvent(EnterEvent * event);
  void leaveEvent(QEvent * event);
  void keyPressEvent(QKeyEvent * event);
  void mousePressEvent(QMouseEvent * event);
  void paintEvent(QPaintEvent * event);

Q_SIGNALS:
  void labelClicked(quint32 id);

private:
  int m_labId;
  bool m_last;
  bool m_hover;
};

class BcExpander : public QWidget
{
  Q_OBJECT
public:
  BcExpander(quint32 expanderId, QWidget * parent = 0);

  void setCount(int count);
  int count() const;

  // ::QWidget
  void mousePressEvent(QMouseEvent * event);
  void paintEvent(QPaintEvent * event);

Q_SIGNALS:
  void expanderClicked(quint32 id);

private:
  int m_exId;
  int m_count;
};

#endif
