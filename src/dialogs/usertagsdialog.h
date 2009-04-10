/************************************************************************************
    begin                : Sun Jun 3 2001
    copyright            : (C) 2001 - 2003 by Brachet Pascal
                               2003 Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
 ************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef USERTAGSDIALOG_H
#define USERTAGSDIALOG_H

#include <QLabel>
#include <QList>
#include <QString>
#include <QStringList>

#include <KDialog>

#include "kileactions.h"

class QRadioButton;
class QLabel;

class KComboBox;
class KLineEdit;
class KPushButton;
class KTextEdit;

namespace KileDialog
{

class UserTags: public KDialog
{
	Q_OBJECT

public:
	explicit UserTags(const QList<KileAction::TagData> &list, QWidget* parent = NULL, const char* name = NULL, const QString &caption = QString());
	~UserTags();

	int index() { return m_prevIndex; }
	const QList<KileAction::TagData>& result() {return m_list; }

	static QString completeTag(const KileAction::TagData & td);
	static KileAction::TagData splitTag(const QString & name, const QString & tag);

private Q_SLOTS:
	void change(int index);
	void redraw();

	void slotAdd();
	void slotInsert();
	void slotRemove();

	void slotApply();

private:
	int 			m_prevIndex;
	KTextEdit 		*m_editTag;
	KLineEdit 		*m_editName;
	KComboBox 		*m_combo;
	QLabel			*m_labelName;
	QLabel			*m_labelTag;
	KPushButton		*m_buttonRemove, *m_buttonAdd, *m_buttonInsert;

	QList<KileAction::TagData>	m_list;
};

}

#endif
