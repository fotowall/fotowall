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

#include "PictureConfig.h"
#include "PictureContent.h"
#include "Shared/GlowEffectDialog.h"
#include "ui_PictureConfig.h"
#include <QGraphicsSceneMouseEvent>
#include <QInputDialog>
#include <QListWidgetItem>

PictureConfig::PictureConfig(PictureContent * pictureContent, AbstractConfig_PARENT * parent)
: AbstractConfig(pictureContent, parent), m_pictureUi(new Ui::PictureConfig()), m_pictureContent(pictureContent)
{
  // WIDGET setup
  QWidget * pictureTab = new QWidget();
  m_pictureUi->setupUi(pictureTab);
  m_pictureUi->applyEffects->setEnabled(false);
  addTab(pictureTab, tr("Picture"));

  // add effects items to the listview
  QListWidgetItem * no_effect =
      new QListWidgetItem(QIcon(":/data/effects-icons/no-effect.png"), tr("No effects"), m_pictureUi->effectsList);
  no_effect->setData(Qt::UserRole, PictureEffect::ClearEffects);
  QListWidgetItem * item_invert = new QListWidgetItem(QIcon(":/data/effects-icons/invert-effect.png"),
                                                      tr("Invert colors"), m_pictureUi->effectsList);
  item_invert->setToolTip(tr("Invert the colors of the picture"));
  item_invert->setData(Qt::UserRole, PictureEffect::InvertColors);
  QListWidgetItem * autoBlend = new QListWidgetItem(QIcon(":/data/effects-icons/autoblend-effect.png"),
                                                    tr("Auto Blend"), m_pictureUi->effectsList);
  autoBlend->setToolTip(tr("Change dark pixels to transparent, leaving bright ones only"));
  autoBlend->setData(Qt::UserRole, PictureEffect::AutoBlend);
  QListWidgetItem * item_nvg =
      new QListWidgetItem(QIcon(":/data/effects-icons/nvg-effect.png"), tr("NVG"), m_pictureUi->effectsList);
  item_nvg->setToolTip(tr("Set the colors to levels of gray"));
  item_nvg->setData(Qt::UserRole, PictureEffect::NVG);
  QListWidgetItem * item_black = new QListWidgetItem(QIcon(":/data/effects-icons/black-and-white-effect.png"),
                                                     tr("Black and White"), m_pictureUi->effectsList);
  item_black->setData(Qt::UserRole, PictureEffect::BlackAndWhite);
  QListWidgetItem * glow =
      new QListWidgetItem(QIcon(":/data/effects-icons/glow-effect.png"), tr("Glow effect"), m_pictureUi->effectsList);
  glow->setData(Qt::UserRole, PictureEffect::Glow);
  QListWidgetItem * sepia =
      new QListWidgetItem(QIcon(":/data/effects-icons/sepia-effect.png"), tr("Sepia"), m_pictureUi->effectsList);
  sepia->setData(Qt::UserRole, PictureEffect::Sepia);
  QListWidgetItem * opacity =
      new QListWidgetItem(QIcon(":/data/effects-icons/opacity-effect.png"), tr("Opacity"), m_pictureUi->effectsList);
  opacity->setData(Qt::UserRole, PictureEffect::Opacity);

  connect(m_pictureUi->invertButton, SIGNAL(clicked()), m_pictureContent, SIGNAL(flipVertically()));
  connect(m_pictureUi->flipButton, SIGNAL(clicked()), m_pictureContent, SIGNAL(flipHorizontally()));
  connect(m_pictureUi->cropButton, SIGNAL(clicked()), m_pictureContent, SIGNAL(requestCrop()));
  // autoconnection doesn't work because we don't do ->setupUi(this), so here we connect manually
  connect(m_pictureUi->applyEffects, SIGNAL(clicked()), this, SLOT(on_applyEffects_clicked()));
  connect(m_pictureUi->effectsList, SIGNAL(itemActivated(QListWidgetItem *)), this,
          SLOT(on_effectsList_itemActivated(QListWidgetItem *)));
}

PictureConfig::~PictureConfig()
{ delete m_pictureUi; }

void PictureConfig::on_applyEffects_clicked()
{
  if(m_currentEffect.effect != PictureEffect::ClearEffects) emit applyEffect(m_currentEffect, true);
}

void PictureConfig::on_effectsList_itemActivated(QListWidgetItem * item)
{
  // get the effect class
  if(!item) return;

  m_pictureUi->applyEffects->setEnabled(false);
  int effect = item->data(Qt::UserRole).toUInt();
  qreal param = 0.0;

  // show glow editing dialog
  if(effect == PictureEffect::Glow)
  {
    QPixmap preview = m_pictureContent->toPixmap(QSize(300, 300), Qt::KeepAspectRatio);
    GlowEffectDialog dialog(preview.toImage());
    if(dialog.exec() != QDialog::Accepted) return;
    param = (qreal)dialog.currentRadius();
  }
  // show opacity dialog
  else if(effect == PictureEffect::Opacity)
  {
    int opacity = QInputDialog::getInt(0, tr("Opacity"), tr("Opacity value.\n\n0: transparent\n100: opaque"),
                                       m_pictureContent->contentOpacity() * 100, 0, 100);
    param = (float)opacity / 100.f;
  }
  // show autoblend dialog
  else if(effect == PictureEffect::AutoBlend)
  {
    int strongness =
        QInputDialog::getInt(0, tr("Auto Blending"), tr("Strength value.\n\n0: weak\n100: strong"), 50, 0, 100);
    param = (float)strongness / 100.f;
  }

  // apply the effect
  m_currentEffect = PictureEffect((PictureEffect::Effect)effect, param);
  emit applyEffect(m_currentEffect, false);
  m_pictureUi->applyEffects->setEnabled(true);
}
