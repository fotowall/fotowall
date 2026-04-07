/***************************************************************************
 *                                                                         *
 *   This file is part of the Wordcloud project,                           *
 *       http://www.enricoros.com/opensource/wordcloud                     *
 *                                                                         *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "Cloud.h"

#include "WordItem.h"

#include "Shared/Compat.h"
#include "math.h"
#include <QCoreApplication>
#include <QGraphicsScene>
#include <QProgressDialog>

using namespace Wordcloud;

Appearance::Appearance() : textGradient(false), minFontSize(8), maxFontSize(72), quadraticFont(false) {}

Cloud::Cloud(QObject * parent)
: QObject(parent), m_scene(0), m_dirty(false), m_placement(MostlyHorizontal), m_accurate(false),
  m_busyMode(ShowProgress)
{
  // default appearance
  m_appearance.textColors << Qt::lightGray << Qt::darkGray << Qt::darkRed << Qt::black /*### Qt::white */;
  m_appearance.backColor = Qt::transparent;
  m_appearance.font.setFamily("Teen"); // Berylium
}

void Cloud::newCloud(const WordList & words)
{
  // gether stats info over the words
  qDeleteAll(m_wordItems);
  m_wordItems.clear();
  int minCount = 0;
  int maxCount = 0;
  foreach(const Word & word, words)
  {
    if(word.count < minCount || !minCount) minCount = word.count;
    if(word.count > maxCount || !maxCount) maxCount = word.count;
  }

  // create items
  foreach(const Word & word, words)
  {
#if 1
    double rot = ((compat::qrand() % 4) == 1) ? 90 : 0.0;
#else
    double rot = compat::qrand() % 360;
#endif
    WordItem * wordItem = new WordItem(word, m_appearance.font, rot, minCount, maxCount);
    wordItem->show();

    m_wordItems.append(wordItem);
    if(m_scene) m_scene->addItem(wordItem);
  }

  // GO!
  m_dirty = true;
  process();
}

void Cloud::regenCloud()
{ process(); }

void Cloud::randomCloud()
{
  // randomize placement
  m_placement = (Placement)(compat::qrand() % Placement_COUNT);

  // randomize appearance
  m_appearance.quadraticFont = !(compat::qrand() & 0x1);
  m_appearance.textGradient = !(compat::qrand() & 0x1);
  // TODO: randomize more

  process();
}

void Cloud::setScene(QGraphicsScene * scene)
{
  if(scene != m_scene)
  {
    // set the scene
    removeFromScene();
    m_scene = scene;

    // add existing items to the new scene
    if(m_scene && !m_wordItems.isEmpty())
      foreach(WordItem * word, m_wordItems) m_scene->addItem(word);
  }
}

QGraphicsScene * Cloud::scene() const
{ return m_scene; }

void Cloud::removeFromScene()
{
  if(!m_scene || m_wordItems.isEmpty()) return;
  foreach(WordItem * word, m_wordItems) m_scene->removeItem(word);
}

void Cloud::setPlacement(Wordcloud::Placement placement)
{
  if(m_placement != placement)
  {
    m_placement = placement;
    m_dirty = true;
  }
}

Wordcloud::Placement Cloud::placement() const
{ return m_placement; }

void Cloud::setAccurate(bool accurate)
{
  if(m_accurate != accurate)
  {
    m_accurate = accurate;
    m_dirty = true;
  }
}

bool Cloud::accurate() const
{ return m_accurate; }

void Cloud::setAppearance(const Wordcloud::Appearance & appearance)
{
  m_appearance = appearance;
  m_dirty = true;
}

Wordcloud::Appearance Cloud::appearance() const
{ return m_appearance; }

void Cloud::setBusyMode(Wordcloud::BusyMode mode)
{ m_busyMode = mode; }

Wordcloud::BusyMode Cloud::busyMode() const
{ return m_busyMode; }

bool Cloud::loadFromXml(QDomElement & cloudElement)
{
  // create all words
  for(QDomElement we = cloudElement.firstChildElement("word"); !we.isNull(); we = we.nextSiblingElement("word"))
  {
    WordItem * wordItem = new WordItem(we);
    wordItem->show();
    m_wordItems.append(wordItem);
    if(m_scene) m_scene->addItem(wordItem);
  }
  return true;
}

void Cloud::saveToXml(QDomElement & cloudElement) const
{
  // dump all Words
  QDomDocument doc = cloudElement.ownerDocument();
  foreach(WordItem * word, m_wordItems)
  {
    // create the Word element
    QDomElement wordElement = doc.createElement("word");
    cloudElement.appendChild(wordElement);
    word->saveToXml(wordElement);
  }
}

void Cloud::process()
{
  // reapply all the properties to the items here
  if(m_dirty)
  {
    foreach(WordItem * item, m_wordItems)
    {

      // randomize the color
      QColor color = m_appearance.textColors.isEmpty()
                         ? Qt::black
                         : m_appearance.textColors.at(compat::qrand() % m_appearance.textColors.size());
      if(m_appearance.textGradient)
      {
        QLinearGradient lg(0, 0, 0, 1);
        lg.setCoordinateMode(QGradient::StretchToDeviceMode);
        lg.setColorAt(0.0, color.lighter());
        lg.setColorAt(1.0, color.darker());
        item->setBrush(lg);
      }
      else
        item->setBrush(color);
    }
  }

  // common constants
  double tBoundingArea = 0.0;
  foreach(WordItem * word, m_wordItems) tBoundingArea += word->boundingRect().width() * word->boundingRect().height();
  const double tRatio = 3.0 / 4.0;
  const int tExtWidth = (int)sqrt(tBoundingArea / tRatio);
  const int tExtHeight = (int)((double)tExtWidth * tRatio);

  // random placement
  if(m_placement == RandomPosition)
  {
    foreach(WordItem * word, m_wordItems)
      word->setPos((compat::qrand() % tExtWidth) - tExtWidth / 2, (compat::qrand() % tExtHeight) - tExtHeight / 2);
    return;
  }

  // show progress dialog if requested
  QProgressDialog * progress = 0;
  if(m_busyMode == ShowProgress)
  {
    progress = new QProgressDialog(tr("Placing Words..."), tr("Cancel"), 0, m_wordItems.size(), 0);
    progress->setWindowModality(Qt::WindowModal);
    progress->setValue(0);
    progress->show();
    qApp->processEvents();
  }

  QList<WordItem *> placed;
  foreach(WordItem * word, m_wordItems)
  {

    // TODO: handle this better
    if(progress && progress->wasCanceled())
    {
      delete progress;
      break;
    }

    // randomize the first word inside the area
    if(placed.isEmpty())
    {
      word->setPos((compat::qrand() % tExtWidth) - tExtWidth / 2, (compat::qrand() % tExtHeight) - tExtHeight / 2);
      word->tmpPlacedRect = word->boundingRect().translated(word->pos().x(), word->pos().y());
      placed.append(word);
      continue;
    }

    // place the word outside of other's boudaries
    int pDir = 0, pSide = 1;
    int pX = 0, pY = 1, pCount = 0;
    while(true)
    {
      switch(pDir)
      {
        case 0:
          pY++;
          break;
        case 1:
          pX--;
          break;
        case 2:
          pY--;
          break;
        case 3:
          pX++;
          break;
      }
      if(++pCount == pSide)
      {
        if(++pDir == 4) pDir = 0;
        if(pDir == 0 || pDir == 2) ++pSide;
        pCount = 0;
      }

      // find out the list of 'maybe intersecting items'
      int posX = pX * 10;
      int posY = pY * 10;
      const QRectF wordRealRect = word->boundingRect().translated(posX, posY);
      QPainterPath wordRealPath = word->path();
#if QT_VERSION >= 0x040600
      wordRealPath.translate(posX, posY);
#else
      wordRealPath = QTransform::fromTranslate(posX, posY).map(wordRealPath);
#endif

      bool intersections = false;
      foreach(WordItem * clItem, placed)
      {
        // check for boundary intersection
        if(!clItem->tmpPlacedRect.intersects(wordRealRect)) continue;

        // check for precise intersection
        if(m_accurate)
        {
          QPainterPath placedPath = clItem->path();
#if QT_VERSION >= 0x040600
          placedPath.translate(clItem->pos());
#else
          placedPath = QTransform::fromTranslate(clItem->pos().x(), clItem->pos().y()).map(placedPath);
#endif
          if(!wordRealPath.intersects(placedPath)) continue;
        }

        // there is the intersection
        intersections = true;
        break;
      }

      // place the word
      if(!intersections)
      {
        word->setPos(posX, posY);
        word->tmpPlacedRect = wordRealRect;
        placed.append(word);
        break;
      }
    }

    if(progress)
    {
      int value = (100 * placed.size()) / m_wordItems.size();
      value = (value * value) / 100;
      progress->setValue(value);
    }
    if(m_busyMode == ShowPlacement) qApp->processEvents();
  }

  // remove the progress dialog (if any)
  delete progress;
  m_dirty = false;
}
