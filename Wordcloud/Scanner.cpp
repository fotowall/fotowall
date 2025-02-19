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

#include "Scanner.h"

#include <QDebug>
#include <QFile>
#include <QHeaderView>
#include <QRegularExpression>
#include <QStringList>
#include <QTableWidget>
#include <QTextStream>

using namespace Wordcloud;

Scanner::Scanner() : m_minimumLength(0), m_maximumLength(-1) {}

void Scanner::setMinimumWordLength(int minimum)
{
  m_minimumLength = minimum;
}

void Scanner::setMaximumWordLength(int maximum)
{
  m_maximumLength = maximum;
}

bool Scanner::addFromFile(const QString & txtFilePath)
{
  QFile file(txtFilePath);
  if(!file.open(QIODevice::ReadOnly)) return false;

  // read all the words from file
  QRegularExpression splitNonWords("\\W");
  QTextStream ts(&file);
  while(!ts.atEnd())
  {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    QStringList words = ts.readLine().split(splitNonWords, Qt::SkipEmptyParts);
#else
    QStringList words = ts.readLine().split(splitNonWords, QString::SkipEmptyParts);
#endif

    foreach(const QString & word, words) addWord(word);
  }
  return true;
}

bool Scanner::addFromString(const QString & string)
{
  QRegularExpression splitNonWords("\\W");

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
  QStringList words = string.split(splitNonWords, Qt::SkipEmptyParts);
#else
  QStringList words = string.split(splitNonWords, QString::SkipEmptyParts);
#endif

  foreach(const QString & word, words) addWord(word);
  return true;
}

bool Scanner::addFromUrl(const QUrl & url)
{
  qWarning() << "Scanner::addFromUrl(" << url.toString() << ") not implemented";
  return false;
}

bool Scanner::addFromRss(const QUrl & rss)
{
  qWarning() << "Scanner::addFromRss(" << rss.toString() << ") not implemented";
  return false;
}

void Scanner::clear()
{
  m_words.clear();
}

static bool wordFrequencySorter(const Word & w1, const Word & w2)
{
  return w1.count > w2.count;
}

WordList Scanner::takeWords(bool cleanList, int maxCount)
{
  // remove common words, single ones, and sort by frequency
  if(cleanList)
  {
    removeWordsByLanguage(QLocale::Italian);
    removeWordsByLanguage(QLocale::English);
    removeWordsByLanguage(QLocale::French);
    int count = 2;
    while(m_words.size() >= maxCount)
    {
      removeWordsBelowCount(count);
      ++count;
    }
    std::sort(m_words.begin(), m_words.end(), wordFrequencySorter);
  }

  // FIXME find out common words (FAKE: use the first..)
  WordList::iterator wIt = m_words.begin();
  for(; wIt != m_words.end(); ++wIt) wIt->commonString = wIt->variants.begin().key();

  // clear private list and return
  WordList wl = m_words;
  m_words.clear();
  return wl;
}

int Scanner::wordCount() const
{
  return m_words.count();
}

bool Scanner::isEmpty() const
{
  return m_words.isEmpty();
}

void Scanner::dumpOnTable(QTableWidget * table)
{
  // setup the table
  table->clear();
  table->setColumnCount(2);
  table->setRowCount(m_words.size());
  table->horizontalHeader()->setVisible(true);
  table->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Word")));
  table->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("#")));
  table->verticalHeader()->setVisible(false);
  // populate the table
  int row = 0;
  WordList::iterator i = m_words.begin();
  for(; i != m_words.end(); ++i)
  {
    table->setItem(row, 0, new QTableWidgetItem(i->variants.begin().key()));
    table->setItem(row++, 1, new QTableWidgetItem(QString::number(i->count)));
  }
}

void Scanner::dumpWords() const
{
  QString dumpString;
  bool first = true;
  foreach(const Word & word, m_words)
  {
    if(first)
      first = false;
    else
      dumpString += ", ";
    dumpString += QString("\"%1\"").arg(word.lowerString);
  }
  qWarning("WordList: %s", qPrintable(dumpString));
}

bool Scanner::addWord(const QString & word)
{
  int length = word.length();
  if(length < m_minimumLength || (length > m_maximumLength && m_maximumLength > 0)) return false;
  QString lowerWord = word.toLower();

  // update existing entries
  WordList::iterator i = m_words.begin();
  for(; i != m_words.end(); ++i)
  {
    if(i->lowerString == lowerWord)
    {
      i->count++;
      if(i->variants.contains(word))
        i->variants[word]++;
      else
        i->variants[word] = 1;
      return true;
    }
  }

  // add a new entry
  Word w;
  w.lowerString = lowerWord;
  w.count = 1;
  w.variants[word] = 1;
  m_words.append(w);
  return true;
}

void Scanner::removeWordsByLanguage(QLocale::Language language)
{
  const char ** regExps;
  int regExpCount = 0;

  switch(language)
  {
    case QLocale::English:
    {
      static const char * er[] = {"and", "are", "has", "to", "by",  "for", "or",
                                  "the", "I",   "you", "on", "off", "of",  "with"};
      regExps = er;
      regExpCount = sizeof(er) / sizeof(const char *);
    }
    break;

    case QLocale::French:
    {
      static const char * er[] = {"le",  "d[ue]", "un",   "être",  "et",   "a",       "je",   "tu",
                                  "il.", "nous",  "vous", "me",    "te",   "[mts]on", "lui",  "nous",
                                  "ne",  "sont",  "que",  "[sc]e", "qui",  "dans",    "elle", "au",
                                  "le",  "pour",  "par",  "y",     "avec", "si",      "là",   "ça"};
      regExps = er;
      regExpCount = sizeof(er) / sizeof(const char *);
    }
    break;

    case QLocale::Italian:
    {
      static const char * r[] = {
          ".",    "a.",    "all",  "alla",  "anche", "anzich.", "che",    "ci",     "cio.",  "come", "con",   "cos.",
          "cui",  "da",    "da.",  "dall.", "degli", "de.",     "dell",   "della",  "delle", "di",   "dove",  "due",
          "ed",   "far.",  "fino", "fra",   "gli",   "i.",      "l.",     "loro",   "nel",   "nell", "nella", "nelle",
          "non",  "per",   "pi.",  "poi",   "pu.",   "quale",   "quell.", "quest.", "sar.",  "s.",   "senza", "su.",
          "sull", "sull.", "tali", "tra",   "un",    "un.",     "uso",    "ti",     "sei",   "ma",   "tu.",   "tutt."};
      regExps = r;
      regExpCount = sizeof(r) / sizeof(const char *);
    }
    break;

    default:
      qWarning("Scanner::removeWordsByLanguage: language unsupported");
      return;
  }

  // erase all words matching regexps
  WordList::iterator wIt = m_words.begin();
  while(wIt != m_words.end())
  {
    bool found = false;
    for(int i = 0; i < regExpCount; i++)
    {
      if(QRegularExpression(QRegularExpression::anchoredPattern(regExps[i])).match(wIt->lowerString).hasMatch())
      {
        found = true;
        break;
      }
    }
    if(found)
      wIt = m_words.erase(wIt);
    else
      ++wIt;
  }
}

void Scanner::removeWordsBelowCount(int count)
{
  WordList::iterator i = m_words.begin();
  while(i != m_words.end())
  {
    if(i->count < count)
      i = m_words.erase(i);
    else
      ++i;
  }
}
