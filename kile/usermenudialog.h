/***************************************************************************
    begin                : Sun Jun 3 2001
    copyright            : (C) 2001 - 2003 by Brachet Pascal, 2003 Jeroen Wijnhout
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
#ifndef USERMENUDIALOG_H
#define USERMENUDIALOG_H

#include <qstring.h>
#include <qstringlist.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <QLabel>

#include <kdialogbase.h>

#include "kileactions.h"

class QRadioButton;
class KTextEdit;
class QLineEdit;
class QComboBox;
class QLabel;
class KPushButton;

namespace KileDialog
{

class UserTags: public KDialogBase
{
	Q_OBJECT

public:
	UserTags( const Q3ValueList<KileAction::TagData> &list, QWidget* parent = 0, const char* name = 0, const QString &caption = QString::null);
	~UserTags();

	int index() { return m_prevIndex; }
	const Q3ValueList<KileAction::TagData>& result() {return m_list; }

	static QString completeTag(const KileAction::TagData & td);
	static KileAction::TagData splitTag(const QString & name, const QString & tag);

private slots:
	void change(int index);
	void redraw();

	void slotAdd();
	void slotInsert();
	void slotRemove();

	void slotApply();

private:
	int 				m_prevIndex;
	KTextEdit 		*m_editTag;
	QLineEdit 		*m_editName;
	QComboBox 		*m_combo;
	QLabel			*m_labelName;
	QLabel			*m_labelTag;
	KPushButton		*m_buttonRemove, *m_buttonAdd, *m_buttonInsert;

	Q3ValueList<KileAction::TagData> 	m_list;
};

}

#endif // USERMENUDIALOG_H
