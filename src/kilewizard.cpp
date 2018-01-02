/******************************************************************************
  Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
		2017 by Michel Ludwig (michel.ludwig@kdemail.net)
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kilewizard.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QShowEvent>

#include "utilities.h"

namespace KileDialog
{
Wizard::Wizard(KConfig *config, QWidget *parent, const char *name, const QString &caption)
    : QDialog(parent)
    , m_td(QString(), QString(), QString(), 0, 0, QString())
    , m_config(config)
    , m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel))
{
    setObjectName(name);
    setWindowTitle(caption);
    setModal(true);

    // add buttons
    QPushButton *okButton = m_buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    okButton->setDefault(true);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

Wizard::~Wizard()
{}

KConfig* Wizard::config() const
{
    return m_config;
}

QDialogButtonBox * Wizard::buttonBox() const
{
    return m_buttonBox;
}

void Wizard::showEvent(QShowEvent *event)
{
    // even with 'showEvent' the dialog might not be shown yet, only about to be shown
    // so we have to 'schedule' a centering (still does not work all the time)
    KileUtilities::scheduleCenteringOfWidget(this);
    QDialog::showEvent(event);
}

}
