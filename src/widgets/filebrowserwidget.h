/********************************************************************************************
    begin                : Wed Aug 14 2002
    copyright            : (C) 2002 - 2003 by Pascal Brachet
                               2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                               2007, 2008 by Michel Ludwig (michel.ludwig@kdemail.net)

from Kate (C) 2001 by Matt Newell

 ********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FILEBROWSERWIDGET_H
#define FILEBROWSERWIDGET_H

#include <QFocusEvent>

#include <KFile>
#include <KDirOperator>
#include <KUrlComboBox>
#include <KUrlCompletion>
#include <KUrl>

#include "kileextensions.h"

class KFileItem;

namespace KileWidget {

class FileBrowserWidget : public QWidget
{
	Q_OBJECT

public: 
	FileBrowserWidget(KileDocument::Extensions *extensions, QWidget *parent = NULL, const char *name = NULL);
	~FileBrowserWidget();

	KUrl currentUrl() const;

public Q_SLOTS:
	void setDir(const KUrl& url);
	void readConfig();
	void writeConfig();

private Q_SLOTS:
	void comboBoxReturnPressed(const QString& u);
	void dirUrlEntered(const KUrl& u);

	void emitFileSelectedSignal();

protected:
	void focusInEvent(QFocusEvent*);

Q_SIGNALS:
	void fileSelected(const KFileItem& fileItem);

private:
	KUrlComboBox	*m_pathComboBox;
	KDirOperator	*m_dirOperator;
	KUrlCompletion	*m_urlCompletion;
};

}

#endif
