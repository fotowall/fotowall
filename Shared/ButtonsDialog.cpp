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

#include "ButtonsDialog.h"
#include <QAbstractButton>
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>

ButtonsDialog::ButtonsDialog(const QString & dialogId,
                             const QString & title,
                             const QString & message,
                             QDialogButtonBox::StandardButtons buttons,
                             bool buttonsCentered,
                             bool memorize)
: m_dialogId(dialogId), m_memorize(false), m_iconLabel(0), m_messageLabel(0), m_memorizeCheckbox(0), m_extraLayout(0),
  m_buttonBox(0), m_defaultButton(QDialogButtonBox::NoButton), m_pressedButton(QDialogButtonBox::NoButton)
{
  // create contents
  QGridLayout * mainLay = new QGridLayout(this);

  m_iconLabel = new QLabel(this);
  m_iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  mainLay->addWidget(m_iconLabel, 0, 0, 3, 1);

  m_messageLabel = new QLabel(this);
  m_messageLabel->setContentsMargins(2, 0, 0, 0);
  m_messageLabel->setTextFormat(Qt::RichText);
  m_messageLabel->setTextInteractionFlags(Qt::NoTextInteraction);
  // m_messageLabel->setWordWrap(true);
  mainLay->addWidget(m_messageLabel, 0, 1, 1, 1, Qt::AlignVCenter);

  m_extraLayout = new QVBoxLayout();
  mainLay->addLayout(m_extraLayout, 1, 1, 1, 1);

  m_memorizeCheckbox = new QCheckBox(tr("ask again next time"), this);
  m_memorizeCheckbox->setChecked(true);
  m_memorizeCheckbox->setVisible(m_memorize);
  mainLay->addWidget(m_memorizeCheckbox, 2, 1, 1, 1);

  m_buttonBox = new QDialogButtonBox(this);
  connect(m_buttonBox, SIGNAL(clicked(QAbstractButton *)), this, SLOT(slotButtonClicked(QAbstractButton *)));
  m_buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  m_buttonBox->setFocus();
  mainLay->addWidget(m_buttonBox, 3, 0, 1, 2);

  // apply initial values
  if(!title.isEmpty()) setTitle(title);
  if(!message.isEmpty()) setMessage(message);
  if(buttons != QDialogButtonBox::NoButton) setButtons(buttons);
  if(buttonsCentered) setButtonsCentered(true);
  if(memorize) setMemorizeEnabled(true);

#if 0
    // useful if the message label word-wraps, so we have to set a minimum size
    // by hand
    setModal(true);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    QSize screenSize = QApplication::desktop()->availableGeometry(QCursor::pos()).size();
    setMinimumWidth(qMin(screenSize.width() - 480, screenSize.width()/2));
    setMinimumHeight(190);
    mainLay->activate();
    adjustSize();
    resize(minimumSize());
#endif
}

QDialogButtonBox::StandardButton ButtonsDialog::execute()
{
  // handle a previous answer if memorize is on
  QString configKey = "ButtonsDialogSkip/" + m_dialogId;
  if(m_memorize)
  {
    // return the previous answer if such answer exists
    QSettings s;
    if(s.contains(configKey)) return (QDialogButtonBox::StandardButton)s.value(configKey).toInt();
  }

  // pop up the dialog
  m_pressedButton = QDialogButtonBox::NoButton;
  int retCode = exec();
  if(retCode == QDialog::Rejected || m_pressedButton == QDialogButtonBox::NoButton) return QDialogButtonBox::NoButton;

  // save the answer if requested
  if(m_memorize && !m_memorizeCheckbox->isChecked() && m_pressedButton != QDialogButtonBox::Cancel)
    QSettings().setValue(configKey, (int)m_pressedButton);

  // return the choosen key
  return m_pressedButton;
}

void ButtonsDialog::setButtons(QDialogButtonBox::StandardButtons buttons)
{
  m_buttonBox->setStandardButtons(buttons);
  setDefaultButton(m_defaultButton);
  foreach(QDialogButtonBox::StandardButton sb, m_buttonNames.keys()) setButtonText(sb, m_buttonNames[sb]);
}

QDialogButtonBox::StandardButtons ButtonsDialog::buttons() const
{ return m_buttonBox->standardButtons(); }

void ButtonsDialog::setButtonText(QDialogButtonBox::StandardButton sb, const QString & text)
{
  m_buttonNames[sb] = text;
  if(QPushButton * pButton = m_buttonBox->button(sb)) pButton->setText(text);
}

QString ButtonsDialog::buttonText(QDialogButtonBox::StandardButton sb) const
{
  QPushButton * pButton = m_buttonBox->button(sb);
  return pButton ? pButton->text() : QString();
}

void ButtonsDialog::setDefaultButton(QDialogButtonBox::StandardButton sb)
{
  m_defaultButton = sb;
  foreach(QAbstractButton * aButton, m_buttonBox->buttons())
    if(QPushButton * pButton = dynamic_cast<QPushButton *>(aButton))
      pButton->setDefault(m_buttonBox->standardButton(aButton) == m_defaultButton);
}

QDialogButtonBox::StandardButton ButtonsDialog::defaultButton() const
{ return m_defaultButton; }

void ButtonsDialog::setButtonsCentered(bool centered)
{ m_buttonBox->setCenterButtons(centered); }

bool ButtonsDialog::buttonsCentered() const
{ return m_buttonBox->centerButtons(); }

void ButtonsDialog::setMessage(const QString & message)
{ m_messageLabel->setText(message); }

QString ButtonsDialog::message() const
{ return m_messageLabel->text(); }

void ButtonsDialog::setTitle(const QString & title)
{ setWindowTitle(title); }

QString ButtonsDialog::title() const
{ return windowTitle(); }

void ButtonsDialog::setIcon(const QIcon & icon)
{
  m_icon = icon;
  setWindowIcon(m_icon);
  m_iconLabel->setPixmap(m_icon.pixmap(32, 32));
}

void ButtonsDialog::setIcon(QStyle::StandardPixmap standardIcon)
{ setIcon(style()->standardIcon(standardIcon)); }

QIcon ButtonsDialog::icon() const
{ return m_icon; }

void ButtonsDialog::addExtraWidget(QWidget * widget)
{
  if(widget)
  {
    m_extraLayout->addWidget(widget);
    m_extraWidgets.append(widget);
    connect(widget, SIGNAL(destroyed()), this, SLOT(slotExtraWidgetDeleted(QObject *)));
  }
}

QList<QWidget *> ButtonsDialog::extraWidgets() const
{ return m_extraWidgets; }

void ButtonsDialog::setMemorizeEnabled(bool enabled)
{
  m_memorize = enabled;
  m_memorizeCheckbox->setVisible(m_memorize);
}

bool ButtonsDialog::memorizeEnabled() const
{ return m_memorize; }

void ButtonsDialog::slotButtonClicked(QAbstractButton * button)
{
  // save the pressed button and return from the 'exec()'
  m_pressedButton = m_buttonBox->standardButton(button);
  accept();
}

void ButtonsDialog::slotExtraWidgetDeleted(QObject * object)
{
  QWidget * widget = static_cast<QWidget *>(object);
  m_extraWidgets.removeAll(widget);
}
