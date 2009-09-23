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
 
#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QCloseEvent>

#include <KProgressDialog>

namespace KileWidget {

/**
 * Our version of the progress dialog ignores close events from the window manager if the user
 * is not allowed to cancel the progress dialog.
 **/
class ProgressDialog : public KProgressDialog
{
	Q_OBJECT

public:
	explicit ProgressDialog(QWidget* parent = NULL, const QString& caption = QString(),
	                        const QString& text = QString(), Qt::WFlags flags = 0);
	virtual ~ProgressDialog();

	virtual void closeEvent(QCloseEvent *event);

};

}

#endif
