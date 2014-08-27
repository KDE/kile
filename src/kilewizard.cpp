/***************************************************************************
    begin                : Tue Dec 23 2003
    copyright            : (C) 2003 Jeroen Wijnhout
    email                : Jeroen.Wijnhout@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kilewizard.h"

#include <QPushButton>

namespace KileDialog
{
	Wizard::Wizard(KConfig *config, QWidget *parent, const char *name, const QString &caption) :
		QDialog(parent),
		m_td(QString(), QString(), QString(), 0, 0, QString()),
		m_config(config)
	{
		setObjectName(name);
		setWindowTitle(caption);
		setModal(true);
//TODO KF5
// 		QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
// 		QWidget *mainWidget = new QWidget(this);
// 		QVBoxLayout *mainLayout = new QVBoxLayout;
// 		setLayout(mainLayout);
// 		mainLayout->addWidget(mainWidget);
// 		QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
// 		okButton->setDefault(true);
// 		okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
// 		connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
// 		connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
		//PORTING SCRIPT: WARNING mainLayout->addWidget(buttonBox) must be last item in layout. Please move it.
// 		mainLayout->addWidget(buttonBox);
// 		okButton->setDefault(true);
	}

	Wizard::~Wizard()
	{}
}
