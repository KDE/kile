/***************************************************************************
    begin                : Wed Aug 14 2002
    copyright            : (C) 2002 - 2003 by Pascal Brachet, 2003 Jeroen Wijnhout
    email                : Jeroen.Wijnhout@kdemail.net

from Kate (C) 2001 by Matt Newell

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KILEFILESELECT_H
#define KILEFILESELECT_H

#include <qwidget.h>
#include <qtoolbutton.h>
//Added by qt3to4:
#include <QFocusEvent>
#include <kfile.h>
#include <kdiroperator.h>
#include <kurlcombobox.h>
#include <kurlcompletion.h>
#include <kurl.h>

#include "kileextensions.h"

/**
  *@author Jeroen Wijnhout
  */

class KFileItem;
class KComboBox;

class KileFileSelect : public QWidget
{
	Q_OBJECT

public: 
	KileFileSelect(KileDocument::Extensions *extensions, QWidget *parent=0, const char *name=0);
	~KileFileSelect();

	void setView(KFile::FileView);
	KDirOperator * dirOperator(){return dir;}
	KComboBox* comboEncoding() { return m_comboEncoding; }

public slots:
	void setDir(KURL);
	void readConfig();
	void writeConfig();

private slots:
	void cmbPathActivated( const KURL& u );
	void cmbPathReturnPressed( const QString& u );
	void dirUrlEntered( const KURL& u );

	void clickedToolbar(int);

protected:
	void focusInEvent(QFocusEvent*);

signals:
	void fileSelected(const KFileItem*);

private:
	KURLComboBox	*cmbPath;
	KDirOperator	* dir;
	KComboBox	*m_comboEncoding;
	QToolButton	*home, *up, *back, *forward;
	KURLCompletion	*cmpl;
};

#endif
