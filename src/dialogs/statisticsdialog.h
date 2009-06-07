/***************************************************************************
    begin                : Tuesday Nov 1 2005
    copyright            : (C) 2005 by Thomas Braun
    email                : thomas.braun@virtuell-zuhause.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef STATISTICSDIALOG_H
#define STATISTICSDIALOG_H

#include <KPageDialog>

#include <QMap>

#include "kiledocmanager.h"
#include "documentinfo.h"

class KileProject;

namespace KileWidget {
	class StatisticsWidget;
}

namespace KileDialog {

class StatisticsDialog : public KPageDialog
{
	public:
		StatisticsDialog(KileProject *project, KileDocument::TextInfo* docinfo,
		                 QWidget* parent = NULL, KTextEditor::View *view = NULL,
		                 const char* name = NULL, const QString &caption = QString());
		~StatisticsDialog();

	public Q_SLOTS:
		void slotButtonClicked(int button);

	private:
		void fillWidget(const long* stats, KileWidget::StatisticsWidget* widget);
		void convertText(QString* text, bool forLaTeX);

	protected:
		KileProject *m_project;
		KileDocument::TextInfo *m_docinfo;
		KTextEditor::View *m_view;
		long *m_summarystats;
		bool m_hasSelection;
		bool m_notAllFilesOpenWarning;
		QMap<KPageWidgetItem*, KileWidget::StatisticsWidget*> m_pagetowidget;
		QMap<KPageWidgetItem*, QString> m_pagetoname;
};

}

#endif
