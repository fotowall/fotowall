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

#include "VersionCheckDialog.h"
#include "ui_VersionCheckDialog.h"
#include <QCoreApplication>
#include <QDesktopServices>
#include <QPushButton>
#include <QUrl>

VersionCheckDialog::VersionCheckDialog(QWidget * parent)
: QDialog(parent), ui(new Ui::VersionCheckDialog), m_connector(0)
{
  // inject ui
  ui->setupUi(this);
  ui->currVersion->setText(QCoreApplication::applicationVersion());
  ui->nextVersion->setText(tr("checking"));

  // start the network request
  m_connector = new MetaXml::Connector();
  connect(m_connector, SIGNAL(fetched()), this, SLOT(slotFetched()));
  connect(m_connector, SIGNAL(fetchError(const QString &)), this, SLOT(slotError(const QString &)));
}

VersionCheckDialog::~VersionCheckDialog()
{
  delete m_connector;
  delete ui;
}

void VersionCheckDialog::slotFetched()
{
  // parse the xml for the data
  const MetaXml::Reader_1 * reader = m_connector->reader();
  if(!reader || reader->releases.isEmpty())
  {
    slotError(tr("XML Error"));
    return;
  }
  ui->progressBar->hide();

  // show version
  m_release = reader->releases.first();
  ui->nextVersion->setText(tr("%1 (%2)").arg(m_release.version).arg(m_release.name));

  // show download button
  if(m_release.version.compare(QCoreApplication::applicationVersion(), Qt::CaseInsensitive))
  {
    QPushButton * dlButton = ui->buttonBox->addButton(tr("Download"), QDialogButtonBox::NoRole);
    connect(dlButton, SIGNAL(clicked()), this, SLOT(slotDownload()));
  }
}

void VersionCheckDialog::slotError(const QString & error)
{
  ui->progressBar->hide();
  ui->nextVersion->setText(error);
}

void VersionCheckDialog::slotDownload()
{ QDesktopServices::openUrl(QUrl(m_release.url)); }
