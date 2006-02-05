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

#include "kilestatsdlg.h"

// A dialog that displays statistical information about the active project/file

KileStatsDlg::KileStatsDlg(KileProject *project, KileDocument::Info* docinfo, QWidget* parent,  const char* name, const QString &caption)
	: KDialogBase(KDialogBase::Tabbed,0,parent,name,true,caption, KDialogBase::Help | KDialogBase::Ok | KDialogBase::User1 | KDialogBase::User2 , KDialogBase::Ok, false),
	m_project(project),
	m_docinfo(docinfo)
{	
	setButtonText(KDialogBase::User1,i18n("Copy"));
	setButtonText(KDialogBase::User2,i18n("Copy as LaTeX"));
	setHelp("statistics");

	m_summarystats = new long[SIZE_STAT_ARRAY];
	m_summarystats[0]=m_summarystats[1]=m_summarystats[2]=m_summarystats[3]=m_summarystats[4]=m_summarystats[5]=0;	
	
	const long* stats;
	QString tempName;
	uint index=0;
	KileWidgetStatistics* tempWidget;
	KileWidgetStatistics* summary;
	KileDocument::Info* tempDocinfo;

	m_hasSelection=false; // class variable, if the user has selected text, 
	summary = new KileWidgetStatistics( addPage( i18n("Summary") ) );
	summary->m_commentAboutHelp->setText(i18n("For information about the accuracy see the Help."));
	// we have in every case a summary tab

	m_pagetowidget[index]=summary;
	m_pagetoname[index]=i18n("Summary");
	index++; // used with activePageIndex() to get the active widget and the tabname, arrays would be more efficient, but Maps are less dangerous
	
	if(m_docinfo->getDoc()->hasSelection()) // the user should really have that doc as active in which the selection is
		m_hasSelection=true;
	
	
	if(!m_project) // the active doc doesn't belong to a project
	{	
		setCaption(i18n("Statistics for %1").arg(m_docinfo->getDoc()->url().fileName()));
		stats = m_docinfo->getStatistics();
		fillWidget(stats,summary);
	}
	else // active doc belongs to a project
	{
		setCaption(i18n("Statistics for the Project %1").arg(m_project->name()));
		kdDebug() << "Project file is " << project->baseURL() << endl;

		KileProjectItemList *items = project->items();

		if(m_hasSelection) // if the active doc has a selection
		{
			stats = m_docinfo->getStatistics();
			fillWidget(stats,summary); // if yes we fill the summary widget and are finished
		} 
		else
		{	
			for(uint k = 0; k < items->count()  ; k++)
			{
				if(items->at(k)->type() ==  KileProjectItem::ProjectFile) // ignore project files
					continue;
				
				tempDocinfo = items->at(k)->getInfo();
				if(tempDocinfo && tempDocinfo->getDoc()) // closed items don't have a doc
				{	
					tempName = tempDocinfo->getDoc()->url().fileName();
					stats = tempDocinfo->getStatistics();
	
					for(uint j = 0; j < SIZE_STAT_ARRAY; j++)
						m_summarystats[j]+=stats[j];
					
					tempWidget = new KileWidgetStatistics( addPage(tempName) );
					kdDebug() << "TempName is " << tempName << endl;
					m_pagetowidget[index]=tempWidget;
					m_pagetoname[index]=tempName;
					index++;
					fillWidget(stats, tempWidget);
				}
				else
					m_notAllFilesOpenWarning=true; // print warning
			}

			fillWidget(m_summarystats,summary);
			if(m_notAllFilesOpenWarning)
				summary->m_warning->setText(i18n("To get statistics for all project files, you have to open them all."));

			kdDebug() << "All keys in name " << m_pagetoname.keys() << " Nr. of keys " << m_pagetowidget.count() << endl;
			kdDebug() << "All keys in widget " << m_pagetowidget.keys() << " Nr. of keys " << m_pagetowidget.count() << endl;
		}
	}
// 	setInitialSize( QSize(550,560), true);
}

KileStatsDlg::~KileStatsDlg()
{
	delete [] m_summarystats;
}


void KileStatsDlg::fillWidget(const long* stats, KileWidgetStatistics* widget)
{

// we don't have to write 0's in the number labels because this is the default value
if (!stats || !widget)
	return;

if(m_hasSelection)
	widget->m_warning->setText(i18n("WARNING: These are the statistics for the selected text only."));

widget->m_wordChar->setText(QString::number(stats[0]));
widget->m_commandChar->setText(QString::number(stats[1]));
widget->m_whitespaceChar->setText(QString::number(stats[2]));
widget->m_totalChar->setText(QString::number(stats[0]+stats[1]+stats[2]));

widget->m_wordString->setText(QString::number(stats[3]));
widget->m_commandString->setText(QString::number(stats[4]));
widget->m_environmentString->setText(QString::number(stats[5]));
widget->m_totalString->setText(QString::number(stats[3]+stats[4]+stats[5]));

widget->updateColumns();
}

void KileStatsDlg::slotUser1() // Copy
{
	kdDebug() << "Copy Button was clicked" << endl;
	kdDebug() << "Open tab is " << activePageIndex() << " " + ( m_pagetoname.contains(activePageIndex()) ?  m_pagetoname[activePageIndex()] : "No such entry" )<< endl;

	QClipboard *clip = KApplication::clipboard();
	QString text;
	convertText(&text,false);
	clip->setText(text,QClipboard::Selection); // the text will be avaible with the middle mouse button
}

void KileStatsDlg::slotUser2() // CopyAsLaTeX
{
	kdDebug() << "CopyAsLateX Button was clicked" << endl;
	kdDebug() << "Open tab is " << activePageIndex() << " " + ( m_pagetoname.contains(activePageIndex()) ?  m_pagetoname[activePageIndex()] : "No such entry" )<< endl;

	QClipboard *clip = KApplication::clipboard();
	QString text;
	convertText(&text,true);
	clip->setText(text,QClipboard::Selection);
}

void KileStatsDlg::convertText(QString* text, bool forLaTeX) // the bool determines if we want plainText or LaTeXCode
{
	KileWidgetStatistics* widget = m_pagetowidget[activePageIndex()];
	QString name = m_pagetoname[activePageIndex()];
	QString charGroupName = i18n("Characters"); // always ensure that these are the same than in kilestatswidget.ui, there is no way to get the label of a button group, so this ugly hack is needed
	QString stringGroupName = i18n("Strings");

	if (forLaTeX)
		text->append("\\begin{tabular}{ll}\n");

if(m_project && activePageIndex())
	text->append(i18n("Statistics for project %1, file %2").arg(m_project->name()).arg(name));
else if(m_project)
	text->append(i18n("Statistics for project %1").arg(m_project->name()));
else if(m_docinfo->getDoc()->url().isValid())
	text->append(i18n("Statistics for %1").arg(m_docinfo->getDoc()->url().fileName()));
else
	text->append(i18n("Statistics for Untitled"));
			
	if (forLaTeX)	
		text->append(" & \\\\\\hline\n");
	else
		text->append("\n\n");
	text->append(charGroupName + ( forLaTeX ? " &  \\\\\n" : "\n") );
 	text->append(widget->m_wordCharText->text() + ( forLaTeX ? " & " : "\t" ) + widget->m_wordChar->text() + ( forLaTeX ? " \\\\\n" : "\n"));
	text->append(widget->m_commandCharText->text() + ( forLaTeX ? " & " : "\t" ) + widget->m_commandChar->text() + ( forLaTeX ? " \\\\\n" : "\n"));
	text->append(widget->m_whitespaceCharText->text() + ( forLaTeX ? " & " : "\t" ) + widget->m_whitespaceChar->text() + ( forLaTeX ? " \\\\\n" : "\n"));
	text->append(widget->m_totalCharText->text() + ( forLaTeX ? " & " : "\t" ) + widget->m_totalChar->text() + ( forLaTeX ? " \\\\\n" : "\n"));
	
	text->append( ( forLaTeX ? " & \\\\\n" : "\n") );
	text->append(stringGroupName + ( forLaTeX ? " &  \\\\\n" : "\n") );
	text->append(widget->m_wordStringText->text() + ( forLaTeX ? " & " : "\t" ) + widget->m_wordString->text() + ( forLaTeX ? " \\\\\n" : "\n"));
	text->append(widget->m_commandStringText->text() + ( forLaTeX ? " & " : "\t" ) + widget->m_commandString->text() + ( forLaTeX ? " \\\\\n" : "\n"));
	text->append(widget->m_environmentStringText->text() + ( forLaTeX ? " & " : "\t" ) + widget->m_environmentString->text() + ( forLaTeX ? " \\\\\n" : "\n"));
	text->append(widget->m_totalStringText->text() + ( forLaTeX ? " & " : "\t" ) + widget->m_totalString->text() + ( forLaTeX ? " \\\\\\hline\n": "\n"));
	
	if (forLaTeX)
		text->append("\\end{tabular}\n");

	if(m_hasSelection) // we can't have both cases
		text->append( ( forLaTeX? "\\par\\bigskip\n": "\n") + widget->m_warning->text() + "\n");
	else if(m_notAllFilesOpenWarning)
		text->append( ( forLaTeX? "\\par\\bigskip\n": "\n") + widget->m_warning->text() + "\n");
}



