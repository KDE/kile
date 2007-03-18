/***************************************************************************
    begin                : Thu Jul 17 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
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

#ifndef KILEINFO_H
#define KILEINFO_H

#include <qstring.h>
#include <qptrlist.h>
#include <qmap.h>
#include <qscrollview.h>

#include <kdebug.h>
#include <kurl.h>

#include "kileextensions.h"
#include "kiletoolmanager.h"
#include "kilestdtools.h"
#include "latexoutputfilter.h"
#include "latexoutputinfo.h"
#include "latexcmd.h"         

class QWidget;

namespace KileDocument { class Info; }
class KileProject;
class KileProjectItem;
class KileProjectItemList;
class KileFileSelect;
class KileEventFilter;

namespace Kate { class Document;}

class KileBottomBar;
namespace KileDocument { class Extensions; class Manager; class EditorExtension; }
namespace KileView { class Manager; }
namespace KileWidget { class Structure; class Konsole; class ScriptsManagement; class PreviewWidget; }
namespace KileTool { class QuickPreview; }
namespace KileHelp { class Help; }
namespace KileJScript { class Manager; }
namespace KileEditorKeySequence { class Manager; }

class KileInfo
{

public:
	KileInfo(QWidget *parent);
	virtual ~KileInfo();

public:
	QString getName(Kate::Document *doc = 0, bool shrt = false);
	QString getShortName(Kate::Document *doc = 0) { return getName(doc, true); }
	QString getCompileName(bool shrt = false);
	QString getFullFromPrettyName(const QString & name);
	KURL::List getParentsFor(KileDocument::Info *);
	bool getSinglemode() { return m_singlemode; }

	QString getCurrentTarget() const { return m_currentTarget; }
	void setTarget(const QString &target) { m_currentTarget=target; }

	virtual Kate::Document* activeTextDocument() const;

	QString getSelection() const;
	void clearSelection() const;

	virtual const QStringList* allLabels(KileDocument::Info * info = 0L);
	virtual const QStringList* allBibItems(KileDocument::Info * info = 0L);
	virtual const QStringList* allBibliographies(KileDocument::Info * info = 0L);
	virtual const QStringList* allDependencies(KileDocument::Info * info = 0L);
	virtual const QStringList* allNewCommands(KileDocument::Info * info = 0L);
	virtual const QStringList* allPackages(KileDocument::Info * info = 0L);

	QString lastModifiedFile(KileDocument::Info * info = 0L);

private:
	const QStringList* retrieveList(const QStringList* (KileDocument::Info::*getit)() const, KileDocument::Info * docinfo = 0L);
	QStringList m_listTemp;

public:
	bool similarOrEqualURL(const KURL &validurl, const KURL &testurl);
	bool isOpen(const KURL & url);
	bool projectIsOpen(const KURL & );

	bool watchFile() { return m_bWatchFile; }
	bool logPresent() { return m_logPresent; }
	void setLogPresent(bool pr) { m_logPresent = pr; }

	LatexOutputFilter * outputFilter() { return m_outputFilter; }
	LatexOutputInfoArray * outputInfo() { return m_outputInfo; }
	
	virtual int lineNumber() = 0;
	
	QString relativePath(const QString basepath, const QString & file);

	KileWidget::Structure *structureWidget() { return m_kwStructure; }
	KileWidget::Konsole *texKonsole() { return m_texKonsole; }
	KileWidget::Output *outputWidget() { return m_outputWidget; }
	KileBottomBar *outputView() { return m_bottomBar; }
	KileWidget::LogMsg *logWidget() { return m_logWidget; }
	KileWidget::PreviewWidget *previewWidget () { return m_previewWidget; } 

	KileDocument::Manager* docManager() const { return m_docManager; }
	KileView::Manager* viewManager() const { return m_viewManager; }
	KileTool::Manager* toolManager() const { return m_manager; }
	KileJScript::Manager* scriptManager() const { return m_jScriptManager; }
	KileEditorKeySequence::Manager* editorKeySequenceManager() const { return m_editorKeySequenceManager; }
	KileTool::Factory* toolFactory() const { return m_toolFactory; }
	KileDocument::EditorExtension *editorExtension() const { return m_edit; }
	KileDocument::LatexCommands *latexCommands() const { return m_latexCommands; }
	KileHelp::Help *help() const { return m_help; }
	KileTool::QuickPreview *quickPreview() const { return m_quickPreview; }
	KileDocument::Extensions *extensions() const { return m_extensions; }

	//FIXME:refactor
	KileFileSelect* fileSelector() const { return m_fileSelector; }
	KileEventFilter* eventFilter() const { return m_eventFilter; }

	QWidget *parentWidget() const { return m_parentWidget; }

protected:
	KileDocument::Manager		*m_docManager;
	KileView::Manager		*m_viewManager;
	KileTool::Manager		*m_manager;
	KileJScript::Manager		*m_jScriptManager;
	KileEditorKeySequence::Manager	*m_editorKeySequenceManager;
	KileTool::Factory		*m_toolFactory;
	KileWidget::Konsole		*m_texKonsole;
	KileWidget::Output		*m_outputWidget;
	KileWidget::LogMsg		*m_logWidget;
	KileWidget::ScriptsManagement	*m_scriptsManagementWidget;
	KileBottomBar			*m_bottomBar;
	KileWidget::PreviewWidget *m_previewWidget; 
	QScrollView *m_previewView;

	KileHelp::Help		*m_help;
	KileDocument::EditorExtension 	*m_edit;
	KileDocument::LatexCommands *m_latexCommands;
	KileDocument::Extensions *m_extensions;
	KileTool::QuickPreview *m_quickPreview;

	QWidget *m_parentWidget;

	bool 		m_singlemode;
	QString	m_masterName;

	QString	m_currentTarget;
	
	bool m_bWatchFile, m_logPresent;

	LatexOutputFilter		*m_outputFilter;
	LatexOutputInfoArray	*m_outputInfo;

	KileWidget::Structure	*m_kwStructure;
	KileFileSelect 			*m_fileSelector;
	KileEventFilter*		m_eventFilter;
};

#endif
