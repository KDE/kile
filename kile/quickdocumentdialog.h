/***************************************************************************
                          quickdocumentdialog.h  -  description
                             -------------------
    begin                : Tue Oct 30 2001
    copyright            : (C) 2001 by Brachet Pascal, (C) 2003 Jeroen Wijnhout
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

#ifndef QUICKDOCUMENTDIALOG_H
#define QUICKDOCUMENTDIALOG_H

#include <qstringlist.h>

#include "kilewizard.h"

class QLabel;
class QCheckBox;
class QGridLayout;

class KPushButton;
class KConfig;
class KComboBox;
class KLineEdit;
class KListBox;

class AddOptionDialog;

/**
  *@author Brachet Pascal
  *@author Jeroen Wijnhout
  */

namespace KileDialog
{
	class QuickDocument : public Wizard
	{
		Q_OBJECT

	public:
		QuickDocument(KConfig *, QWidget *parent=0, const char *name=0, const QString &caption = QString::null);
		~QuickDocument();

	public slots:
		void init();
		void slotOk();

	private slots:
		void addUserClass();
		void addUserPaper();
		void addUserEncoding();
		void addUserOptions();

	private:
		void add(QStringList & list);
		void readConfig();
		void writeConfig();

	protected:
		QGridLayout	*m_layout;
		KLineEdit		*m_leAuthor,*m_leTitle ;
		KComboBox	*m_cbDocClass,  *m_cbFontSize, *m_cbPaperSize,  *m_cbEncoding;
		QCheckBox	*m_ckAMS, *m_ckIdx;
		KPushButton	*userClassBtn, *userPaperBtn, *userEncodingBtn, *userOptionsBtn;
		QLabel		*m_lbDocClass, *m_lbAuthor, *m_lbTitle, *m_lbOptions;

		KListBox		*m_bxOptions;
		QStringList	m_otherClassList, m_otherPaperList, m_otherEncodingList, m_otherOptionsList;
	};
}

#endif
