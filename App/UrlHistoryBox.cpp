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

#include "UrlHistoryBox.h"

#include "Canvas/Canvas.h"
#include "FotowallFile.h"
#include "Shared/Compat.h"
#include "Shared/GlowEffectWidget.h"
#include "Shared/PixmapButton.h"

#include <QFile>
#include <QHBoxLayout>
#include <QMenu>
#include <QTimer>
#include <QVariant>

UrlHistoryBox::UrlHistoryBox(const QList<QUrl> & urls, QWidget * parent) : GroupBoxWidget(parent), m_previewIndex(0)
{
  // create this layout
  QHBoxLayout * lay = new QHBoxLayout(this);
  lay->setContentsMargins(2, 0, 0, 0);
  lay->setSpacing(0);
  setLayout(lay);

  // set initial urls, if given
  if(!urls.isEmpty()) changeUrls(urls, true);
}

UrlHistoryBox::~UrlHistoryBox()
{
  qDeleteAll(m_entries);
  m_entries.clear();
}

void UrlHistoryBox::changeUrls(const QList<QUrl> & urls, bool delayPreview)
{
  // remove previous icons, if any
  qDeleteAll(m_entries);
  m_entries.clear();

  if(urls.isEmpty()) return;

  // add buttons
#if defined(MOBILE_UI)
  const int maxButtons = 3;
#else
  const int maxButtons = 5;
#endif
  for(int i = 0; i < qMin(maxButtons, urls.size()); i++)
  {
    const QUrl & url = urls[i];
    PixmapButton * button = new PixmapButton(this);
    connect(button, SIGNAL(clicked()), this, SLOT(slotClicked()));
    connect(button, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(slotContextMenu(const QPoint &)));
    button->setFixedSize(QSize(64, 60));
    button->setContextMenuPolicy(Qt::CustomContextMenu);
    button->setProperty("url", url);
    button->setHoverText(QString::number(i + 1));
    button->setToolTip(url.toString());
    int mag = (compat::qrand() % 7) + (compat::qrand() % 7);
    static bool dir = true;
    dir = !dir;
    int angle = dir ? mag : -mag;
    button->setProperty("angle", angle);
    layout()->addWidget(button);
    m_entries.append(button);
    if(delayPreview)
    {
#if 0
            QImage img(48, 48, QImage::Format_ARGB32_Premultiplied);
            img.fill(0xFF808080);
            button->setPixmap(prettyPixmap(img, angle));
#endif
    }
    else
      genPreview(button);
  }

  // start preview jobs
  if(delayPreview)
  {
    m_previewIndex = 0;
    QTimer::singleShot(100, this, SLOT(slotNextPreview()));
  }
}

QUrl UrlHistoryBox::urlForEntry(int index) const
{
  if(index < 0 || index >= m_entries.size()) return QUrl();
  return m_entries[index]->property("url").toUrl();
}

QPixmap UrlHistoryBox::prettyPixmap(const QImage & image, int angle)
{
  QTransform rot;
  rot.rotate(angle);
  const QImage rotated = image.transformed(rot, Qt::SmoothTransformation);
  const QImage shadowed = GlowEffectWidget::dropShadow(rotated, QColor(64, 64, 64), 6, 1, 1);
  return QPixmap::fromImage(shadowed);
}

void UrlHistoryBox::genPreview(PixmapButton * button)
{
  QUrl currentUrl = button->property("url").toUrl();
  QString fwFilePath = currentUrl.toString();

  // get the embedded preview
  QImage previewImage = FotowallFile::embeddedPreview(fwFilePath);

  // generate preview
  if(previewImage.isNull())
  {
    Canvas * canvas = new Canvas(physicalDpiX(), physicalDpiY(), this);
    if(FotowallFile::read(fwFilePath, canvas, false))
    {
      // render canvas, rotate, drop shadow and set
      canvas->resizeAutoFit();
      canvas->setEmbeddedPainting(true);
      previewImage = canvas->renderedImage(QSize(48, 48), Qt::KeepAspectRatio, true);
    }
    delete canvas;
  }

  // make a pretty preview (rotated and shadowed)
  if(!previewImage.isNull())
  {
    int angle = button->property("angle").toInt();
    button->setPixmap(prettyPixmap(previewImage, angle));
  }
}

void UrlHistoryBox::slotClicked()
{
  emit urlClicked(sender()->property("url").toUrl());
}

void UrlHistoryBox::slotContextMenu(const QPoint & widgetPos)
{
  // get data from the sender button
  PixmapButton * button = static_cast<PixmapButton *>(sender());
  QPoint screenPos = button->mapToGlobal(widgetPos);
  QUrl fileUrl = button->property("url").toUrl();
  QString fwFilePath = fileUrl.toString();
  if(!QFile::exists(fwFilePath)) return;

  // build menu
  QMenu menu;
  QAction * openAction = menu.addAction(tr("Open"));
  menu.addSeparator();
  QAction * removeAction = menu.addAction(tr("Remove From History"));
  QMenu * fileSubMenu = menu.addMenu(tr("File"));
  QAction * deleteAction = fileSubMenu->addAction(QIcon(":/data/action-delete.png"), tr("Delete File"));

  // popup menu and handle actions
  QAction * action = menu.exec(screenPos);
  if(action == openAction) { emit urlClicked(fileUrl); }
  else if(action == removeAction)
  {
    m_entries.removeAll(button);
    button->deleteLater();
    emit urlRemoved(QUrl(fwFilePath));
  }
  else if(action == deleteAction)
  {
    if(QFile::remove(fwFilePath))
    {
      m_entries.removeAll(button);
      button->deleteLater();
      emit urlRemoved(QUrl(fwFilePath));
    }
    else
      qWarning("UrlHistoryBox::slotContextMenu: cannot remove file");
  }
}

void UrlHistoryBox::slotNextPreview()
{
  if(m_previewIndex < m_entries.size())
  {
    PixmapButton * btn = m_entries[m_previewIndex];
    genPreview(btn);
    m_previewIndex++;

    // start next job right after
    if(m_previewIndex < m_entries.size()) QTimer::singleShot(10, this, SLOT(slotNextPreview()));
  }
}
