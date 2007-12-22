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

#include <KPageDialog>

#include <QMap>

#include "kiledocmanager.h"
#include "kiledocumentinfo.h"

class KileProject;
class KileWidgetStatistics;

class KileStatsDlg : public KPageDialog
{
	public:
		KileStatsDlg(KileProject *project, KileDocument::TextInfo* docinfo, QWidget* parent = 0,  const char* name = 0, const QString &caption = QString::null);
		~KileStatsDlg();

	public Q_SLOTS:
		void slotButtonClicked(int button);

	private:
		void fillWidget(const long* stats, KileWidgetStatistics* widget);
		void convertText(QString* text, bool forLaTeX);

	protected:
		KileProject *m_project;
		KileDocument::TextInfo *m_docinfo;
		long *m_summarystats;
		bool m_hasSelection;
		bool m_notAllFilesOpenWarning;
		QMap<KPageWidgetItem*, KileWidgetStatistics*> m_pagetowidget;
		QMap<KPageWidgetItem*, QString> m_pagetoname;
};

#endif
