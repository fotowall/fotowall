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

#ifndef __Canvas_h__
#define __Canvas_h__

#include "Shared/AbstractScene.h"
#include <QDataStream>
#include <QDir>
#include <QDomElement>
#include <QElapsedTimer>
#include <QImage>
#include <QPainter>
#include <QPair>
#include <QPixmap>
#include <QRect>
#include <QTime>
#include <QUndoStack>
#include <QVector>

class AbstractContent;
class AbstractConfig;
struct PictureEffect;
class ColorPickerItem;
class CanvasModeInfo;
class CanvasViewContent;
class HelpItem;
class HighlightItem;
class PictureContent;
class QTimer;
class TextContent;
class WebcamContent;
class WordcloudContent;
class QNetworkReply;

class Canvas : public AbstractScene
{
  Q_OBJECT
  Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged)
public:
  Canvas(int sDpiX, int sDpiY, QObject * parent = 0);
  ~Canvas();

  // add/remove content
  void addAutoContent(const QStringList & filePaths);
  void addCanvasViewContent(const QStringList & fwFilePaths);
  QList<AbstractContent *> addPictureContent(const QStringList & fileNames);
  QList<AbstractContent *> addPictureContent(const QString & fileName, const QImage & picData);

  QList<AbstractContent *> addPictureContent(const QVector<QPair<QString, QImage>> & pictures);
  PictureContent * addPictureContent(const QString & url);
  PictureContent * addNetworkPictureContent(const QString & url);
  PictureContent * addNetworkPictureContent(const QString & url,
                                            QNetworkReply * reply,
                                            QString title,
                                            int width,
                                            int height);
  TextContent * addTextContent();
  WebcamContent * addWebcamContent(int input);
  WordcloudContent * addWordcloudContent();

  void addManualContent(AbstractContent * content, const QPoint & pos);
  void deleteContent(AbstractContent * content);
  void clearContent();
  QPoint visibleCenter() const;

  // ::AbstractScene
  void resize(const QSize & size);
  void resizeEvent();

  // properties
  QString filePath() const;
  void setFilePath(const QString & filePath);
  QString prettyBaseName() const;

  // item interaction
  void selectAllContent(bool selected = true);

  // arrangement
  void setForceFieldEnabled(bool enabled);
  bool forceFieldEnabled() const;
  void randomizeContents(bool position, bool rotation, bool opacity);

  // decorations
  void setBackContent(AbstractContent * content);

  enum BackMode
  {
    BackNone = 0,
    BackBlack = 1,
    BackWhite = 2,
    BackGradient = 3
  };
  void setBackMode(BackMode mode);
  BackMode backMode() const;
  void clearBackContent();
  bool backContent() const;

  void setBackContentRatio(Qt::AspectRatioMode mode);
  Qt::AspectRatioMode backContentRatio() const;
  void setTopBarEnabled(bool enabled);
  bool topBarEnabled() const;
  void setBottomBarEnabled(bool enabled);
  bool bottomBarEnabled() const;
  void setTitleText(const QString & text);
  QString titleText() const;
  void setCDMarkers();
  void setDVDMarkers();
  void clearMarkers();

  // misc
  bool pendingChanges() const;
  void blinkBackGradients();
  void setEmbeddedPainting(bool);

  // change size and project mode (CD, DVD, etc)
  CanvasModeInfo * modeInfo() const;

  // render contents, but not the invisible items
  void renderVisible(QPainter * painter,
                     const QRectF & target = QRectF(),
                     const QRectF & source = QRectF(),
                     Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio,
                     bool hideTools = true);
  QImage renderedImage(const QSize & size,
                       Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio,
                       bool hideTools = true);

  // load & save
  void saveToXml(QDomElement & canvasElement) const;
  void loadFromXml(QDomElement & canvasElement);
  AbstractContent * addContentFromXml(const QDomElement & contentElt);

  inline QUndoStack & commandStack() { return *m_commandStack; }
  inline const QUndoStack & commandStack() const { return *m_commandStack; }
  inline QUndoStack * commandStackPtr() { return m_commandStack; }
  inline const QUndoStack * commandStackPtr() const { return m_commandStack; }
  void pushCurrentContentPositionToCommandStack();

Q_SIGNALS:
  void backConfigChanged();
  void requestContentEditing(AbstractContent * content);
  void showPropertiesWidget(QWidget * widget);
  void filePathChanged();

protected:
  void pasteFromClipboard();
  void dragEnterEvent(QGraphicsSceneDragDropEvent * event);
  void dragMoveEvent(QGraphicsSceneDragDropEvent * event);
  void dropEvent(QGraphicsSceneDragDropEvent * event);
  void keyPressEvent(QKeyEvent * keyEvent);

  void mousePressEvent(QGraphicsSceneMouseEvent * event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);

  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
  void contextMenuEvent(QGraphicsSceneContextMenuEvent * event);
  void drawBackground(QPainter * painter, const QRectF & rect);
  void drawForeground(QPainter * painter, const QRectF & rect);

private:
  void initContent(AbstractContent * content, const QPoint & pos);
  CanvasViewContent * createCanvasView(const QPoint & pos, bool spontaneous);
  PictureContent * createPicture(const QPoint & pos, bool spontaneous);
  TextContent * createText(const QPoint & pos, bool spontaneous);
  WebcamContent * createWebcam(int webcamIndex, const QPoint & pos, bool spontaneous);
  WordcloudContent * createWordcloud(const QPoint & pos, bool spontaneous);
  void deleteConfig(AbstractConfig * config);
  QGraphicsView * mainGraphicsView() const;

  QString m_filePath;
  QDir m_fileAbsDir;
  CanvasModeInfo * m_modeInfo;
  QList<AbstractContent *> m_content;
  QList<AbstractConfig *> m_configs;
  QList<HighlightItem *> m_highlightItems;
  ColorPickerItem * m_titleColorPicker;
  ColorPickerItem * m_foreColorPicker;
  ColorPickerItem * m_grad1ColorPicker;
  ColorPickerItem * m_grad2ColorPicker;
  BackMode m_backMode;
  AbstractContent * m_backContent;
  Qt::AspectRatioMode m_backContentRatio;
  bool m_topBarEnabled;
  bool m_bottomBarEnabled;
  QString m_titleText;
  QPixmap m_backTile;
  QPixmap m_backCache;
  QList<QGraphicsItem *> m_markerItems; // used by some modes to show information items, which won't be rendered
  QTimer * m_forceFieldTimer;
  QElapsedTimer m_forceFieldTime;
  bool m_embeddedPainting;
  bool m_pendingChanges;

  QUndoStack * m_commandStack;

private Q_SLOTS:
  friend class AbstractConfig; // HACK here, only to call 1 method
  friend class PixmapButton; // HACK here, only to call 1 method
  void slotSelectionChanged();
  void slotBackgroundContent();
  void slotConfigureContent(const QPoint & scenePoint);
  void slotEditContent();
  void slotStackContent(int);
  void slotCollateContent();
  void slotDeleteContent();
  void slotDeleteConfig();
  void slotApplyLook(quint32 frameClass, bool mirrored, bool allContent);
  void slotApplyEffect(const PictureEffect & effect, bool allPictures);
  void slotCrop();
  void slotFlipHorizontally();
  void slotFlipVertically();

  void slotTitleColorChanged();
  void slotForeColorChanged();
  void slotGradColorChanged();
  void slotBackContentChanged();

  void slotMarkChanges();
  void slotResetChanges();
  void slotApplyForce();

  void undoSlot();
  void redoSlot();
};

#endif
