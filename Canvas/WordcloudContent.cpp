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

#include "WordcloudContent.h"

#include "Wordcloud/Scanner.h"

#include <QDebug>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QPainter>

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>

WordCloudContentInputDialog::WordCloudContentInputDialog(QWidget * parent)
{
  setWindowTitle(tr("Wordcloud options"));
  QFormLayout * lytMain = new QFormLayout(this);
  wordLength = new QSpinBox(this);
  wordLength->setRange(1, 20);
  wordLength->setValue(3);
  lytMain->addRow(new QLabel(tr("Minimum word length longer or equal to: "), this), wordLength);
  maxCount = new QSpinBox(this);
  maxCount->setRange(1, 10000);
  maxCount->setValue(100);
  lytMain->addRow(new QLabel(tr("Maximum number of words: "), this), maxCount);
  resize(300, 100);

  QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
  lytMain->addWidget(buttonBox);

  bool conn = connect(buttonBox, &QDialogButtonBox::accepted, this, &WordCloudContentInputDialog::accept);
  Q_ASSERT(conn);

  setLayout(lytMain);
}

void WordCloudContentInputDialog::accept()
{
  emit acceptedOptions(wordLength->value(), maxCount->value());
  QDialog::accept();
}

WordcloudContent::WordcloudContent(bool spontaneous, QGraphicsScene * scene, QGraphicsItem * parent)
: AbstractContent(scene, spontaneous, false, parent), m_cloudScene(new QGraphicsScene), m_cloud(new Wordcloud::Cloud),
  m_cloudTaken(false)
{
  connect(m_cloudScene, SIGNAL(changed(const QList<QRectF> &)), this, SLOT(slotRepaintScene(const QList<QRectF> &)));
  m_cloud->setScene(m_cloudScene);
}

WordcloudContent::~WordcloudContent()
{
  if(m_cloudTaken) qWarning("WordcloudContent::~WordcloudContent: Wordcloud still exported");
  // this deletes even all the items of the cloud
  delete m_cloudScene;
  // this deletes only the container (BAD DONE, IMPROVE)
  delete m_cloud;
}

void WordcloudContent::manualInitialization()
{
  auto createWordCloud = [this](QString txtContent)
  {
    if(txtContent.isEmpty()) { txtContent = tr("Welcome to Wordcloud. Change options on the sidebar."); }

    auto * dlg = new WordCloudContentInputDialog(nullptr);

    connect(dlg, &WordCloudContentInputDialog::acceptedOptions,
            [txtContent, this](int minLength, int maxCount)
            {
              Wordcloud::Scanner scanner;
              scanner.setMinimumWordLength(minLength);
              scanner.addFromString(txtContent);
              m_cloud->newCloud(scanner.takeWords(true, maxCount));
            });
    dlg->open();

    return;
  };

#if defined(__EMSCRIPTEN__)
  auto fileContentReady = [this, createWordCloud](const QString & fileName, const QByteArray & fileContent)
  {
    if(!fileName.isEmpty())
    {
      // Use fileName and fileContent
      QString txtContent = QString(fileContent);
      createWordCloud(txtContent);
    }
  };
  QFileDialog::getOpenFileContent(tr("Text files"), fileContentReady);
#else
  QString txtFilePath = QFileDialog::getOpenFileName(0, tr("Create a Wordcloud from a text file"));
  QFile file(txtFilePath);
  if(!file.open(QIODevice::ReadOnly)) return;
  createWordCloud(file.readAll());
#endif
}

bool WordcloudContent::fromXml(const QDomElement & contentElement, const QDir & baseDir)
{
  AbstractContent::fromXml(contentElement, baseDir);

  // restore the cloud
  QDomElement cloudElement = contentElement.firstChildElement("cloud");
  return m_cloud->loadFromXml(cloudElement);
}

void WordcloudContent::toXml(QDomElement & contentElement, const QDir & baseDir) const
{
  AbstractContent::toXml(contentElement, baseDir);
  contentElement.setTagName("wordcloud");

  // sanity check
  if(m_cloudTaken)
  {
    qWarning("WordcloudContent::toXml: resource taken, can't save it");
    return;
  }

  // dump cloud
  QDomDocument doc = contentElement.ownerDocument();
  QDomElement cloudElement = doc.createElement("cloud");
  contentElement.appendChild(cloudElement);
  m_cloud->saveToXml(cloudElement);
}

void WordcloudContent::drawContent(QPainter * painter, const QRect & targetRect, Qt::AspectRatioMode ratio)
{
  Q_UNUSED(ratio);
  if(m_cloud) m_cloudScene->render(painter, targetRect, m_cloudScene->sceneRect(), Qt::KeepAspectRatio);
}

QVariant WordcloudContent::takeResource()
{
  // sanity check
  if(m_cloudTaken)
  {
    qWarning("WordcloudContent::takeResource: resource already taken");
    return QVariant();
  }

  // discard reference
  m_cloudTaken = true;
  Wordcloud::Cloud * cloud = m_cloud;
  m_cloud = 0;
  return QVariant::fromValue((void *)cloud);
}

void WordcloudContent::returnResource(const QVariant & resource)
{
  // sanity checks
  if(!m_cloudTaken) qWarning("WordcloudContent::returnResource: not taken");
  if(m_cloud)
  {
    qWarning("WordcloudContent::returnResource: we already have a cloud, shouldn't return one");
    delete m_cloud;
  }

  // store reference back
  m_cloudTaken = false;
  m_cloud = static_cast<Wordcloud::Cloud *>(resource.value<void *>());
  if(m_cloud) m_cloud->setScene(m_cloudScene);
  update();
}

void WordcloudContent::slotRepaintScene(const QList<QRectF> & /*exposed*/)
{
  update();
}
