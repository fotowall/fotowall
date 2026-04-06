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

#include "Stacker.h"

#include "AbstractAppliance.h"

using namespace PlugGui;

Stacker::Stacker() : m_container(0) {}

Stacker::~Stacker()
{
  if(!m_appliances.isEmpty())
  {
    qWarning("Appliance::Stacker::~Stacker: not empty!");
    clearAppliances();
  }
  m_container = 0;
}

void Stacker::setContainer(Container * container)
{
  if(m_container == container) return;

  m_container = container;
  if(AbstractAppliance * app = currentAppliance())
  {
    if(m_container)
      app->addToApplianceContainer(m_container);
    else
      app->removeFromApplianceContainer();
  }
}

Container * Stacker::container() const
{ return m_container; }

void Stacker::stackAppliance(AbstractAppliance * appliance)
{
  if(!appliance) return;
  if(AbstractAppliance * app = currentAppliance()) app->removeFromApplianceContainer();
  m_appliances.append(appliance);
  if(m_container) appliance->addToApplianceContainer(m_container);

  // notify about the change
  emit structureChanged();
}

QList<AbstractAppliance *> Stacker::stackedAppliances() const
{ return m_appliances; }

AbstractAppliance * Stacker::currentAppliance() const
{ return m_appliances.isEmpty() ? 0 : m_appliances.last(); }

int Stacker::applianceCount() const
{ return m_appliances.size(); }

void Stacker::popAppliance()
{
  // delete last
  bool removed = false;
  if(!m_appliances.isEmpty())
  {
    AbstractAppliance * app = m_appliances.takeLast();
    if(m_container) app->removeFromApplianceContainer();
    // TODO - SAVE/SERIALIZE/OTHER LINK HERE
    delete app;
    removed = true;
  }

  // show the last-1
  if(m_container && !m_appliances.isEmpty()) m_appliances.last()->addToApplianceContainer(m_container);

  // notify about the change
  emit structureChanged();
}

void Stacker::clearAppliances()
{
  // remove appliances
  while(!m_appliances.isEmpty()) popAppliance();
}

void Stacker::structureChanged()
{
  // nothing to do here...
}
