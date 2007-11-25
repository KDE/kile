/***************************************************************************
    begin                : Tuesday Nov 1 2005
    copyright            : (C) 2005 by Thomas Braun
    email                : braun@physik.fu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KILESTATSDLG_H
#define KILESTATSDLG_H


#include <kpagedialog.h>
#include <klocale.h>
#include <kapplication.h>
#include <qclipboard.h>
#include <qlabel.h>

#include "kileproject.h"
#include "kiledocumentinfo.h"
#include "kilestatswidget.h"

class KileStatsDlg : public KPageDialog
{
public:
	KileStatsDlg(KileProject *project, KileDocument::TextInfo* docinfo, QWidget* parent = 0,  const char* name = 0, const QString &caption = QString::null);
	~KileStatsDlg();

private:
	void fillWidget (const long* stats, KileWidgetStatistics* widget);
	void slotUser1();
	void slotUser2();
	void convertText(QString* text, bool forLaTeX);

protected:
	KileProject				*m_project;
	KileDocument::TextInfo			*m_docinfo;
	long					*m_summarystats;
	bool 					m_hasSelection;
	bool					m_notAllFilesOpenWarning;
	QMap<int, KileWidgetStatistics*> 	m_pagetowidget;
	QMap<int, QString> 			m_pagetoname;
};

#endif
