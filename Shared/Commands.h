/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2007-2008 by TANGUY Arnaud <arn.tanguy@gmail.com>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __Command__
#define __Command__

#include "Shared/ColorPickerItem.h"
#include "Shared/PictureEffect.h"
#include <QString>
#include <QUndoCommand>

#include "App/CanvasAppliance.h"
#include "Canvas/AbstractContent.h"
#include "Canvas/Canvas.h"
#include "Canvas/PictureContent.h"
#include "Canvas/TextContent.h"
#include "Canvas/WebcamContent.h"
#include "Canvas/WordcloudContent.h"
#include "Frames/FrameFactory.h"
#include <QAction>
#include <QDebug>
#include <QStringList>
#include <QUrl>

// Unique ID for each command that requires mergeWith
enum class CommandsID
{
  PerspectiveCommand = 0,
  SceneRotationCommand
};

class EffectCommand : public QUndoCommand
{
private:
  QList<PictureEffect> m_previousEffects;
  PictureEffect m_newEffect;
  QRect m_previousSize;
  PictureContent * m_content;

public:
  EffectCommand(PictureContent * content, const PictureEffect & effect)
  : QUndoCommand(QObject::tr("New Effects")), m_content(content), m_newEffect(effect)
  {
    m_previousEffects = m_content->effects();
    m_previousSize = m_content->contentRect();
  }
  void redo() override
  {
    PictureContent * c = dynamic_cast<PictureContent *>(m_content);
    if(!c) return;
    m_previousSize = c->contentRect();
    c->addEffect(m_newEffect);
  }
  void undo() override
  {
    PictureContent * c = dynamic_cast<PictureContent *>(m_content);
    if(!c) return;

    c->addEffect(PictureEffect::ClearEffects);
    foreach(PictureEffect effect, m_previousEffects) { c->addEffect(effect); }
    c->resizeContents(m_previousSize);
  }
};

class OpacityCommand : public QUndoCommand
{
private:
  AbstractContent * m_content = nullptr;
  qreal m_pOpacity;
  qreal m_nOpacity;

public:
  OpacityCommand(AbstractContent * content, qreal pOpacity, qreal nOpacity)
  : QUndoCommand(QObject::tr("Opacity command")), m_content(content), m_pOpacity(pOpacity), m_nOpacity(nOpacity)
  {
  }
  void redo() override
  {
    AbstractContent * c = m_content;
    if(!c) return;
    c->setProperty("contentOpacity", m_nOpacity);
    c->update();
  }
  void undo() override
  {
    AbstractContent * c = m_content;
    if(!c) return;
    c->setProperty("contentOpacity", m_pOpacity);
    c->update();
  }
};

// DONE
/* This commands manage the text of the TextContent */
class TextCommand : public QUndoCommand
{
private:
  TextContent * m_content;
  QString m_previousText, m_newText;

public:
  TextCommand(TextContent * content, QString newText)
  : QUndoCommand(QObject::tr("Text content changed")), m_content(content), m_newText(newText)
  { m_previousText = content->toHtml(); }
  void redo() override
  {
    TextContent * c = dynamic_cast<TextContent *>(m_content);
    if(!c) return;
    c->setHtml(m_newText);
  }
  void undo() override
  {
    TextContent * c = dynamic_cast<TextContent *>(m_content);
    if(!c) return;
    c->setHtml(m_previousText);
  }
};

// T is AbstractContent or AbstractScne
template<typename T = AbstractContent>
class PerspectiveCommand : public QUndoCommand
{
private:
  T * m_content;
  QPointF m_previous, m_new;
  bool m_merge = false;

public:
  PerspectiveCommand(T * content, const QPointF & p, const QPointF & n, bool merge = false)
  : QUndoCommand(QObject::tr("Perspective")), m_content(content), m_previous(p), m_new(n), m_merge(merge)
  {
  }

  void redo() override { m_content->setPerspective(m_new); }

  void undo() override { m_content->setPerspective(m_previous); }

  int id() const override { return static_cast<int>(CommandsID::PerspectiveCommand); }

  bool mergeWith(const QUndoCommand * command) override
  {
    if(!m_merge) return false;
    if(command->id() != id()) // make sure other is also a PerspectiveCommand
      return false;
    auto * other = static_cast<const PerspectiveCommand<T> *>(command);
    m_new = other->m_new;
    return true;
  }
};

class SceneRotationCommand : public QUndoCommand
{
private:
  AbstractScene * m_scene = nullptr;
  qreal m_previous, m_new;

public:
  SceneRotationCommand(AbstractScene * scene, qreal p, qreal n)
  : QUndoCommand(QObject::tr("Scene Rotation")), m_scene(scene), m_previous(p), m_new(n)
  {
  }

  int id() const override { return static_cast<int>(CommandsID::SceneRotationCommand); }

  void redo() override { m_scene->setRotation(m_new); }
  void undo() override { m_scene->setRotation(m_previous); }

  bool mergeWith(const QUndoCommand * command) override
  {
    if(command->id() != id()) // make sure other is also a SceneRotationCommand
      return false;
    auto * other = static_cast<const SceneRotationCommand *>(command);
    m_new = other->m_new;
    return true;
  }
};

// DONE
/* This commands manages transformations: rotations and scaling */
class TransformCommand : public QUndoCommand
{
private:
  AbstractContent * m_content;
  QTransform m_previous, m_new;

public:
  TransformCommand(AbstractContent * content, QTransform & previous_matrix, QTransform & new_matrix)
  : QUndoCommand(QObject::tr("Transformation")), m_content(content), m_previous(previous_matrix), m_new(new_matrix)
  {
  }
  void redo() override { m_content->setTransform(m_new); }
  void undo() override { m_content->setTransform(m_previous); }
};

// DONE
class RotateAndResizeCommand : public QUndoCommand
{
private:
  AbstractContent * m_content;
  const qreal m_pAngle, m_nAngle;
  const QRect m_pRect, m_nRect;

public:
  RotateAndResizeCommand(AbstractContent * content,
                         const qreal pAngle,
                         const qreal nAngle,
                         const QRect pRect,
                         const QRect nRect)
  : QUndoCommand(QObject::tr("Rotation and Resize")), m_content(content), m_pAngle(pAngle), m_nAngle(nAngle),
    m_pRect(pRect), m_nRect(nRect)
  {
  }
  void redo() override
  {
    m_content->setRotation(m_nAngle);
    m_content->resizeContents(m_nRect);
  }
  void undo() override
  {
    m_content->setRotation(m_pAngle);
    m_content->resizeContents(m_pRect);
  }
};

// DONE
/* This command manages movements of the content */
class MotionCommand : public QUndoCommand
{
private:
  AbstractContent * m_content;
  QPointF m_previous, m_newMotion;

public:
  MotionCommand(AbstractContent * content, QPointF previous, QPointF newMotion)
  : QUndoCommand(QObject::tr("Move")), m_content(content), m_previous(previous), m_newMotion(newMotion)
  {
  }
  void redo() override { m_content->setPos(m_newMotion); }
  void undo() override { m_content->setPos(m_previous); }
  void setPrevious(const QPointF & prev) { m_previous = prev; }
  void setCurrent(const QPointF & current) { m_newMotion = current; }
};

// DONE
/**
 * @brief DeleteContentCommand handles the deletion and reloading of contents
 *
 * WARNING: Deleting a large amount of content will keep them in memory until the command stack is cleared. This can
 * lead to high memory consumption
 */
class DeleteContentCommand : public QUndoCommand
{
  AbstractContent * m_content;
  Canvas * m_canvas;

public:
  DeleteContentCommand(AbstractContent * content, Canvas * canvas)
  : QUndoCommand(QObject::tr("Delete content(s)")), m_content(content), m_canvas(canvas)
  {
  }

  void redo() override { m_content->setVisible(false); }

  void undo() override { m_content->setVisible(true); }
};

/**
 * @brief NewContentCommand handles the creation/removal of content.
 * It delegates most of its functionalities to the opposite
 * DeleteContentCommand.
 */
class NewContentCommand : public QUndoCommand
{
private:
  DeleteContentCommand * m_command;

public:
  NewContentCommand(AbstractContent * content, Canvas * canvas) : QUndoCommand(QObject::tr("New content"))
  { m_command = new DeleteContentCommand(content, canvas); }

  void redo() override { m_command->undo(); }

  void undo() override { m_command->redo(); }
};

// DONE
/* This command manages movements of the content */
class FrameCommand : public QUndoCommand
{
private:
  AbstractContent * m_content;
  quint32 m_previousClass, m_newClass;
  bool m_previousMirror, m_newMirror;

public:
  FrameCommand(AbstractContent * content, quint32 newClass, bool mirrored)
  : QUndoCommand(QObject::tr("Change frame/mirror")), m_content(content), m_newClass(newClass), m_newMirror(mirrored)
  {
    m_previousClass = content->frameClass();
    m_previousMirror = content->mirrored();
  }
  void redo() override
  {
    m_content->setFrame(FrameFactory::createFrame(m_newClass));
    m_content->setMirrored(m_newMirror);
  }
  void undo() override
  {
    m_content->setFrame(FrameFactory::createFrame(m_previousClass));
    m_content->setMirrored(m_previousMirror);
  }
  bool hasEffect() { return m_previousClass != m_newClass || m_previousMirror != m_newMirror; }
};

// DONE
/* Manage background content */
class BackgroundContentCommand : public QUndoCommand
{
private:
  Canvas * m_canvas;
  AbstractContent * m_previousContent = nullptr;
  AbstractContent * m_newContent = nullptr;

public:
  BackgroundContentCommand(Canvas * canvas, AbstractContent * previousContent, AbstractContent * newContent)
  : QUndoCommand(QObject::tr("Change background")), m_canvas(canvas), m_previousContent(previousContent),
    m_newContent(newContent)
  {
  }
  void redo() override { m_canvas->setBackContent(m_newContent); }
  void undo() override { m_canvas->setBackContent(m_previousContent); }
};

// DONE
class BackgroundRatioCommand : public QUndoCommand
{
private:
  Canvas * m_canvas;
  const Qt::AspectRatioMode m_previousRatio, m_newRatio;

public:
  BackgroundRatioCommand(Canvas * canvas,
                         const Qt::AspectRatioMode & previousRatio,
                         const Qt::AspectRatioMode & newRatio)
  : QUndoCommand(QObject::tr("Change background ratio")), m_canvas(canvas), m_previousRatio(previousRatio),
    m_newRatio(newRatio)
  {
  }
  void redo() override { m_canvas->setBackContentRatio(m_newRatio); }
  void undo() override { m_canvas->setBackContentRatio(m_previousRatio); }
};

// DONE
class BackgroundModeCommand : public QUndoCommand
{
private:
  Canvas * m_canvas;
  const Canvas::BackMode m_previousMode, m_newMode;

public:
  BackgroundModeCommand(Canvas * desk, const Canvas::BackMode & previousMode, const Canvas::BackMode & newMode)
  : QUndoCommand(QObject::tr("Change background mode")), m_canvas(desk), m_previousMode(previousMode),
    m_newMode(newMode)
  {
  }
  void redo() override { m_canvas->setBackMode(m_newMode); }
  void undo() override { m_canvas->setBackMode(m_previousMode); }
};

// DONE
class DecoTopBarCommand : public QUndoCommand
{
  Canvas * m_canvas;
  QAction * m_action;
  bool m_state;

public:
  DecoTopBarCommand(Canvas * canvas, QAction * action, bool state)
  : QUndoCommand(QObject::tr("Top bar enabled/disabled")), m_canvas(canvas), m_action(action), m_state(state)
  {
  }
  void redo() override
  {
    m_canvas->setTopBarEnabled(m_state);
    m_action->setChecked(m_state);
  }
  void undo() override
  {
    m_canvas->setTopBarEnabled(!m_state);
    m_action->setChecked(!m_state);
  }
};

// DONE
class DecoBottomBarCommand : public QUndoCommand
{
  Canvas * m_canvas;
  QAction * m_action;
  bool m_state;

public:
  DecoBottomBarCommand(Canvas * canvas, QAction * action, bool state)
  : QUndoCommand(QObject::tr("Bottom bar enabled/disabled")), m_canvas(canvas), m_action(action), m_state(state)
  {
  }
  void redo() override
  {
    m_canvas->setBottomBarEnabled(m_state);
    m_action->setChecked(m_state);
  }
  void undo() override
  {
    m_canvas->setBottomBarEnabled(!m_state);
    m_action->setChecked(!m_state);
  }
};

// DONE
class DecoTitleCommand : public QUndoCommand
{
private:
  Canvas * m_canvas;
  QString m_previousText, m_newText;

public:
  DecoTitleCommand(Canvas * canvas, QString newText)
  : QUndoCommand(QObject::tr("Title changed")), m_canvas(canvas), m_newText(newText)
  { m_previousText = m_canvas->titleText(); }
  void redo() override { m_canvas->setTitleText(m_newText); }
  void undo() override { m_canvas->setTitleText(m_previousText); }
};

// DONE
class ColorPickerCommand : public QUndoCommand
{
  ColorPickerItem * m_cItem;
  const QColor m_pColor, m_nColor;

public:
  ColorPickerCommand(ColorPickerItem * item, const QColor pCol, const QColor nCol)
  : QUndoCommand(QObject::tr("Color changed")), m_cItem(item), m_pColor(pCol), m_nColor(nCol)
  {
  }

  void redo() override { m_cItem->setColor(m_nColor); }
  void undo() override { m_cItem->setColor(m_pColor); }
};

// DONE
class StackCommand : public QUndoCommand
{
  AbstractContent * m_content;
  int m_pZ, m_nZ;

public:
  StackCommand(AbstractContent * c, int pZ, int nZ)
  : QUndoCommand(QObject::tr("Stack order changed")), m_content(c), m_pZ(pZ), m_nZ(nZ)
  {
  }

  void redo() override { m_content->setZValue(m_nZ); }
  void undo() override { m_content->setZValue(m_pZ); }
};

// DONE
class ShapeCommand : public QUndoCommand
{
  TextContent * m_content = nullptr;
  QList<QPointF> m_pCps, m_nCps;

public:
  ShapeCommand(TextContent * c, const QList<QPointF> & pCps, const QList<QPointF> & nCps)
  : QUndoCommand(QObject::tr("Text Shape changed")), m_content(c), m_pCps(pCps), m_nCps(nCps)
  {
  }

  void redo() override
  {
    TextContent * c = dynamic_cast<TextContent *>(m_content);
    if(!c) return;
    c->setControlPoints(m_nCps);
  }
  void undo() override
  {
    TextContent * c = dynamic_cast<TextContent *>(m_content);
    if(!c) return;
    c->setControlPoints(m_pCps);
  }
};

class ProjectModeCommand : public QUndoCommand
{
  CanvasAppliance * m_app;
  CanvasModeInfo m_pMode, m_nMode;

public:
  ProjectModeCommand(CanvasAppliance * app, const CanvasModeInfo pMode, const CanvasModeInfo nMode)
  : QUndoCommand(QObject::tr("Project mode changed")), m_app(app), m_pMode(pMode), m_nMode(nMode)
  {
  }

  void redo() override { m_app->setProjectMode(m_nMode); }

  void undo() override { m_app->setProjectMode(m_pMode); }
};

class FxCommand : public QUndoCommand
{
  AbstractContent * m_content;
  int m_pIndex, m_nIndex;

public:
  FxCommand(AbstractContent * content, int pIndex, int nIndex)
  : QUndoCommand(QObject::tr("Content Fx")), m_content(content), m_pIndex(pIndex), m_nIndex(nIndex)
  {
  }
  void redo() override { m_content->setFxIndex(m_nIndex); }

  void undo() override { m_content->setFxIndex(m_pIndex); }
};

#endif
