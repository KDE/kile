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

namespace KileWidget {

ProgressDialog::ProgressDialog(QWidget* parent, const QString& caption, const QString& text, Qt::WFlags flags) :
	  KProgressDialog(parent, caption, text, flags)
{
}

ProgressDialog::~ProgressDialog()
{
}

void ProgressDialog::closeEvent(QCloseEvent *event)
{
	if(allowCancel()) {
		KProgressDialog::closeEvent(event);
	}
	else {
		event->ignore();
	}
}

}
#include "progressdialog.moc"
