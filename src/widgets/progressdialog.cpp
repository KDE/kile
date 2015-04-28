/**************************************************************************************
    Copyright (C) 2009 by Michel Ludwig (michel.ludwig@kdemail.net)
 **************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "progressdialog.h"

#include "kiledebug.h"

#include <QLabel>

namespace KileWidget {

ProgressDialog::ProgressDialog(QWidget* parent, const QString& caption, const QString& text, Qt::WindowFlags flags) :
	  QProgressDialog(parent, flags)
{
	setWindowTitle(caption);

	QLabel *label = new QLabel(this);
	label->setText(caption);
	setLabel(label);
}

ProgressDialog::~ProgressDialog()
{
}

void ProgressDialog::closeEvent(QCloseEvent *event)
{
//TODO KF5
// 	if(allowCancel()) {
// 		QProgressDialog::closeEvent(event);
// 	}
// 	else {
// 		event->ignore();
// 	}
}

}
