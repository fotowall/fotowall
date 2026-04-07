/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2009 by TANGUY Arnaud <arn.tanguy@gmail.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "CroppingDialog.h"

CroppingDialog::CroppingDialog(QPixmap * pix) : QDialog()
{
  ui.setupUi(this);
  ui.croppingWidget->setPixmap(pix);

  connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

QRect CroppingDialog::getCroppingRect() const
{ return ui.croppingWidget->getCroppingRect(); }
