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

#ifndef __Workflow_h__
#define __Workflow_h__

#include "Shared/AbstractResourceProvider.h"
#include "Shared/PlugGui/AbstractAppliance.h"
#include <QObject>
#include <QVariant>
namespace Wordcloud
{
class Cloud;
}
class BreadCrumbBar;
class Canvas;

// TODO: add checkes to avoid enqueueing while processing a command, or a list

class Workflow : public QObject
{
  Q_OBJECT
public:
  Workflow(PlugGui::Container * container, BreadCrumbBar * bar, QObject * parent = 0);
  ~Workflow();

  // change workflow
  bool loadCanvas_A(const QString & fwFilePath = QString());
  void startCanvas_A();
  void stackSlaveCanvas_A(SingleResourceLoaner *);
#if defined(HAS_WORDCLOUD_APPLIANCE)
  void startWordcloud_A();
  void stackSlaveWordcloud_A(SingleResourceLoaner *);
#endif
  void stackHelpAppliance();
  void popCurrentAppliance();

  // other utilities
  QString applianceName() const;
  bool applianceCommand(int command);
  bool requestExit();

private:
  struct Command
  {
    enum Type
    {
      ResetToLevel,
      MasterCanvas,
      SlaveCanvas
#if defined(HAS_WORDCLOUD_APPLIANCE)
          ,
      MasterWordcloud,
      SlaveWordcloud
#endif
    };

    Type type;
    QVariant param;
    SingleResourceLoaner * loaner;

    Command(Type type, const QVariant & param = QVariant(), SingleResourceLoaner * loaner = 0)
    : type(type), param(param), loaner(loaner)
    {
    }
  };
  void scheduleCommand(const Command & command);
  bool processCommand(const Command & command);

  struct Node
  {
    PlugGui::AbstractAppliance * appliance;
    SingleResourceLoaner * loaner;

    Node(PlugGui::AbstractAppliance * app, SingleResourceLoaner * loaner = 0) : appliance(app), loaner(loaner) {}
  };

  void pushNode(const Node & node);
  bool popNode(bool allowSaving);
  void updateBreadcrumb();

  // external objects
  PlugGui::Container * m_container;
  BreadCrumbBar * m_bar;

  // commands
  QTimer * m_commandTimer;
  QList<Command> m_commands;
  bool m_processingQueue;

  // nodes structure
  QList<Node> m_stack;

private Q_SLOTS:
  void slotNodeClicked(quint32);
  void slotProcessQueue();
};

#endif
