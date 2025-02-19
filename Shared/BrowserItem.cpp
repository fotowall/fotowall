/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Inspired from Plasma Web Applet <www.kde.org> and Qt LGPL sources     *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "BrowserItem.h"
#include <QGraphicsSceneResizeEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWebFrame>
#include <QWebPage>

// this class 'unprotects' QWebPage from BrowserItem
class WebPage : public QWebPage
{
public:
  friend class BrowserItem;
  WebPage(QObject * parent = 0) : QWebPage(parent) {}
};

/// Little QGraphicsWidget that renders HTML
BrowserItem::BrowserItem(QGraphicsItem * parent)
: QGraphicsWidget(parent), m_webPage(new WebPage(this)), m_readOnly(false)
{
  // set the page as transparent and link it
  QPalette pal;
#if QT_VERSION < 0x040500
  pal.setBrush(QPalette::Window, Qt::transparent);
#else
  pal.setBrush(QPalette::Base, Qt::transparent);
#endif
  m_webPage->setPalette(pal);
  connect(m_webPage, SIGNAL(linkClicked(const QUrl &)), this, SIGNAL(linkClicked(const QUrl &)));
  connect(m_webPage, SIGNAL(scrollRequested(int, int, const QRect &)), this,
          SLOT(slotScrollRequested(int, int, const QRect &)));
  connect(m_webPage, SIGNAL(repaintRequested(const QRect &)), this, SLOT(slotUpdateRequested(const QRect &)));

  // enable keyboard focus and mouse tracking
  setFlag(QGraphicsItem::ItemIsFocusable, true);
  setAcceptHoverEvents(true);
}

void BrowserItem::write(const QString & html, const QUrl & url)
{
  m_webPage->mainFrame()->setHtml(html, url);
  update();
}

void BrowserItem::browse(const QString & url)
{
  m_webPage->mainFrame()->load(QUrl(url));
  update();
}

void BrowserItem::historyBack()
{
  m_webPage->triggerAction(QWebPage::Back);
}

void BrowserItem::historyForward()
{
  m_webPage->triggerAction(QWebPage::Forward);
}

bool BrowserItem::readOnly() const
{
  return m_readOnly;
}

void BrowserItem::setReadOnly(bool on)
{
  m_readOnly = on;
}

void BrowserItem::resizeEvent(QGraphicsSceneResizeEvent * event)
{
  m_webPage->setViewportSize(event->newSize().toSize());
  update();
}

void BrowserItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * /*widget*/)
{
  painter->setRenderHints(QPainter::TextAntialiasing);
  m_webPage->mainFrame()->render(painter, option->exposedRect.toAlignedRect());
}

void BrowserItem::slotScrollRequested(int dx, int dy, const QRect & scrollViewRect)
{
  scroll(dx, dy, scrollViewRect);
}

void BrowserItem::slotUpdateRequested(const QRect & rect)
{
  update(rect);
}

// utility function for passing events / calling method's base impl
#define PASS2(__orig, __fctn)        \
  if(m_readOnly)                     \
    QGraphicsWidget::__fctn(__orig); \
  else                               \
    m_webPage->event(__orig)

#define PASS3(__e, __orig, __fctn)            \
  if(m_readOnly)                              \
    QGraphicsWidget::__fctn(__orig);          \
  else                                        \
  {                                           \
    (__e)->setAccepted(__orig->isAccepted()); \
    m_webPage->event(__e);                    \
  }

/** Focusing **/
void BrowserItem::focusInEvent(QFocusEvent * event)
{
  PASS2(event, focusInEvent);
}

void BrowserItem::focusOutEvent(QFocusEvent * event)
{
  PASS2(event, focusOutEvent);
}

/** Keyboard **/
void BrowserItem::keyPressEvent(QKeyEvent * event)
{
  PASS2(event, keyPressEvent);
}

void BrowserItem::keyReleaseEvent(QKeyEvent * event)
{
  PASS2(event, keyReleaseEvent);
}

void BrowserItem::inputMethodEvent(QInputMethodEvent * event)
{
  PASS2(event, inputMethodEvent);
}

QVariant BrowserItem::inputMethodQuery(Qt::InputMethodQuery query) const
{
  return m_webPage->inputMethodQuery(query);
}

/** Mouse **/
void BrowserItem::hoverMoveEvent(QGraphicsSceneHoverEvent * event)
{
  QMouseEvent e(QEvent::MouseMove, event->pos().toPoint(), event->screenPos(), Qt::NoButton, Qt::NoButton,
                event->modifiers());
  PASS3(&e, event, hoverMoveEvent);
}

void BrowserItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
  QMouseEvent e(QEvent::MouseMove, event->pos().toPoint(), event->screenPos(), event->button(), event->buttons(),
                event->modifiers());
  PASS3(&e, event, mouseMoveEvent);
}

void BrowserItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
  QMouseEvent e(QEvent::MouseButtonPress, event->pos().toPoint(), event->screenPos(), event->button(), event->buttons(),
                event->modifiers());
  PASS3(&e, event, mousePressEvent);
}

void BrowserItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
  QMouseEvent e(QEvent::MouseButtonRelease, event->pos().toPoint(), event->screenPos(), event->button(),
                event->buttons(), event->modifiers());
  PASS3(&e, event, mouseReleaseEvent);
}

void BrowserItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
  QMouseEvent e(QEvent::MouseButtonDblClick, event->pos().toPoint(), event->screenPos(), event->button(),
                event->buttons(), event->modifiers());
  PASS3(&e, event, mouseDoubleClickEvent);
}

void BrowserItem::contextMenuEvent(QGraphicsSceneContextMenuEvent * event)
{
  QContextMenuEvent e((QContextMenuEvent::Reason)event->reason(), event->pos().toPoint(), event->screenPos());
  PASS3(&e, event, contextMenuEvent);
}

void BrowserItem::wheelEvent(QGraphicsSceneWheelEvent * event)
{

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
  qDebug()
      << "BrowserItem::wheelEvent has not been ported to Qt 5.15 and will not work as expecte. Please fix the code";

  // FIXME I Have no idea how to do this in this Qt version.
  // As the BrowserItem is currently unused, I will leave it unfixed here
  // QWheelEvent(
  // const QPointF &pos,
  // const QPointF &globalPos,
  // QPoint pixelDelta,
  // QPoint angleDelta,
  // Qt::MouseButtons buttons,
  // Qt::KeyboardModifiers modifiers,
  // Qt::ScrollPhase phase,
  // bool inverted,
  // Qt::MouseEventSource source = Qt::MouseEventNotSynthesized,
  // const QPointingDevice *device = QPointingDevice::primaryPointingDevice())

  // QWheelEvent e(event->pos().toPoint(), event->screenPos(), event->delta(), event->angleDelta(), event->buttons(),
  // event->modifiers(), event->scrollPhase(), event->isInverted());
#else
  QWheelEvent e(event->pos().toPoint(), event->screenPos(), event->delta(), event->buttons(), event->modifiers(),
                event->orientation());
  PASS3(&e, event, wheelEvent);
#endif
}

/** Drag & Drop **/
void BrowserItem::dragEnterEvent(QGraphicsSceneDragDropEvent * event)
{
  QDragEnterEvent e(event->pos().toPoint(), event->dropAction(), event->mimeData(), event->buttons(),
                    event->modifiers());
  PASS3(&e, event, dragEnterEvent);
}

void BrowserItem::dragLeaveEvent(QGraphicsSceneDragDropEvent * event)
{
  QDragLeaveEvent e;
  PASS3(&e, event, dragLeaveEvent);
}

void BrowserItem::dragMoveEvent(QGraphicsSceneDragDropEvent * event)
{
  QDragMoveEvent e(event->pos().toPoint(), event->dropAction(), event->mimeData(), event->buttons(),
                   event->modifiers());
  PASS3(&e, event, dragMoveEvent);
}

void BrowserItem::dropEvent(QGraphicsSceneDragDropEvent * event)
{
  QDropEvent e(event->pos().toPoint(), event->dropAction(), event->mimeData(), event->buttons(), event->modifiers());
  PASS3(&e, event, dropEvent);
}

/** some Magic **/
bool BrowserItem::sceneEvent(QEvent * event)
{
  // directly dispatch key events (to avoid tab eating in QGraphicsItem)
  if(event->type() == QEvent::KeyPress)
  {
    QKeyEvent * ke = static_cast<QKeyEvent *>(event);
    if(ke->key() == Qt::Key_Tab)
    {
      m_webPage->focusNextPrevChild(true);
      ke->setAccepted(true);
    }
    else if(ke->key() == Qt::Key_Backtab)
    {
      m_webPage->focusNextPrevChild(false);
      ke->setAccepted(true);
    }
    else
    {
      ke->setAccepted(false);
      keyPressEvent(ke);
    }
    return true;
  }

  // unbreak GW
  return QGraphicsWidget::sceneEvent(event);
}
