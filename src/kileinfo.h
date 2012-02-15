/**************************************************************************************
    begin                : Thu Jul 17 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                               2007 by Michel Ludwig (michel.ludwig@kdemail.net)
 **************************************************************************************/

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

#include <QString>
#include <QMap>

#include "kiledebug.h"
#include <kurl.h>

#include <KParts/MainWindow>

#include "kileconstants.h"
#include "kileextensions.h"
#include "kiletoolmanager.h"
#include "kilestdtools.h"
#include "latexoutputfilter.h"
#include "outputinfo.h"
#include "latexcmd.h"
#include "kileconfig.h"

class QWidget;

namespace KileDocument { class Info; }
class KileProject;
class KileProjectItem;
class KileProjectItemList;

namespace KTextEditor { class Document;}

namespace KileConfiguration{ class Manager; }
namespace KileDocument { class Extensions; class Manager; class EditorExtension; }
namespace KileView { class Manager; }
namespace KileWidget { class StructureWidget; class Konsole; class ScriptsManagement; class PreviewWidget; class ExtendedScrollArea; class FileBrowserWidget; class OutputView; class BottomBar; }
namespace KileTool { class QuickPreview; }
namespace KileHelp { class Help; }
namespace KileScript { class Manager; }
namespace KileEditorKeySequence { class Manager; }
namespace KileTemplate { class Manager; }
namespace KileCodeCompletion { class Manager; }
namespace KileAbbreviation { class Manager; }

class EditorCommands;

class KileInfo
{
public:
	KileInfo(KParts::MainWindow *mainWindow);
	virtual ~KileInfo();

public:
	enum {bibinputs = 0,bstinputs, texinputs};
	QString getName(KTextEditor::Document *doc = NULL, bool shrt = false);
	QString getShortName(KTextEditor::Document *doc = NULL) { return getName(doc, true); }
	QString getCompileName(bool shrt = false);
	QString getFullFromPrettyName(const QString & name);
	KUrl::List getParentsFor(KileDocument::Info *);
	bool getSinglemode() { return m_singlemode; }

	QString getCurrentTarget() const { return m_currentTarget; }
	void setTarget(const QString &target) { m_currentTarget=target; }

	virtual KTextEditor::Document* activeTextDocument() const;

	QString getSelection() const;
	void clearSelection() const;

	virtual QStringList allLabels(KileDocument::Info *info = NULL);
	virtual QStringList allBibItems(KileDocument::Info *info = NULL);
	virtual QStringList allBibliographies(KileDocument::Info *info = NULL);
	virtual QStringList allDependencies(KileDocument::Info *info = NULL);
	virtual QStringList allNewCommands(KileDocument::Info *info = NULL);
	virtual QStringList allAsyFigures(KileDocument::Info *info = NULL);
	virtual QStringList allPackages(KileDocument::Info *info = NULL);

	QString lastModifiedFile(KileDocument::Info * info = NULL);

	static QString documentTypeToString(KileDocument::Type type);

	virtual void focusLog() = 0;
	virtual void focusOutput() = 0;
	virtual void focusKonsole() = 0;
	virtual void focusEditor() = 0;
	virtual void focusPreview() = 0;

protected:
	virtual bool queryExit() = 0;
	virtual bool queryClose() = 0;

private:
	QStringList retrieveList(QStringList (KileDocument::Info::*getit)() const, KileDocument::Info *docinfo = NULL);

public:
	bool similarOrEqualURL(const KUrl &validurl, const KUrl &testurl);
	bool isOpen(const KUrl & url);
	bool projectIsOpen(const KUrl & );

	bool watchFile() { return m_bWatchFile; }
	bool logPresent() { return m_logPresent; }
	void setLogPresent(bool pr) { m_logPresent = pr; }

	LatexOutputFilter * outputFilter() { return m_outputFilter; }
	LatexOutputInfoArray * outputInfo() { return m_outputInfo; }

	virtual int lineNumber() = 0;

	KileWidget::StructureWidget *structureWidget() { return m_kwStructure; }
	KileWidget::Konsole *texKonsole() { return m_texKonsole; }
	KileWidget::OutputView *outputWidget() { return m_outputWidget; }
	KileWidget::BottomBar *outputView() { return m_bottomBar; }
	KileWidget::LogWidget *logWidget() { return m_logWidget; }
	KileWidget::PreviewWidget *previewWidget () { return m_previewWidget; }

	KileConfiguration::Manager* configurationManager() const { return m_configurationManager; }
	KileDocument::Manager* docManager() const { return m_docManager; }
	KileView::Manager* viewManager() const { return m_viewManager; }
	KileTool::Manager* toolManager() const { return m_manager; }
	KileScript::Manager* scriptManager() const { return m_jScriptManager; }
	KileEditorKeySequence::Manager* editorKeySequenceManager() const { return m_editorKeySequenceManager; }
	KileTool::Factory* toolFactory() const { return m_toolFactory; }
	KileDocument::EditorExtension *editorExtension() const { return m_edit; }
	KileDocument::LatexCommands *latexCommands() const { return m_latexCommands; }
	KileHelp::Help *help() const { return m_help; }
	KileTool::QuickPreview *quickPreview() const { return m_quickPreview; }
	KileDocument::Extensions *extensions() const { return m_extensions; }
	KileTemplate::Manager *templateManager() const { return m_templateManager; }
	KileCodeCompletion::Manager *codeCompletionManager() const { return m_codeCompletionManager; }
	KileAbbreviation::Manager* abbreviationManager() const { return m_abbreviationManager; }

	//FIXME:refactor
	KileWidget::FileBrowserWidget* fileSelector() const { return m_fileBrowserWidget; }

	KParts::MainWindow* mainWindow() const { return m_mainWindow; }

	static QString expandEnvironmentVars(const QString &variable);
	static QString checkOtherPaths(const QString &path,const QString &file, int type);
	static QString checkOtherPaths(const KUrl &url,const QString &file, int type){ return checkOtherPaths(url.toLocalFile(),file, type); }

protected:
	KileConfiguration::Manager	*m_configurationManager;
	KParts::MainWindow		*m_mainWindow;
	KileDocument::Manager		*m_docManager;
	KileView::Manager		*m_viewManager;
	KileTool::Manager		*m_manager;
	KileTemplate::Manager		*m_templateManager;
	KileScript::Manager		*m_jScriptManager;
	KileEditorKeySequence::Manager	*m_editorKeySequenceManager;
	KileTool::Factory		*m_toolFactory;
	KileWidget::Konsole		*m_texKonsole;
	KileWidget::OutputView		*m_outputWidget;
	KileWidget::LogWidget		*m_logWidget;
	KileWidget::ScriptsManagement	*m_scriptsManagementWidget;
	KileWidget::BottomBar		*m_bottomBar;
	KileWidget::PreviewWidget	*m_previewWidget;
	KileWidget::ExtendedScrollArea	*m_previewScrollArea;
	KileCodeCompletion::Manager	*m_codeCompletionManager;
	KileAbbreviation::Manager	*m_abbreviationManager;

	EditorCommands				*m_editorCommands;

	KileHelp::Help		*m_help;
	KileDocument::EditorExtension 	*m_edit;
	KileDocument::LatexCommands *m_latexCommands;
	KileDocument::Extensions *m_extensions;
	KileTool::QuickPreview *m_quickPreview;

	bool 		m_singlemode;
	QString	m_masterName;

	QString	m_currentTarget;

	bool m_bWatchFile, m_logPresent;

	LatexOutputFilter		*m_outputFilter;
	LatexOutputInfoArray	*m_outputInfo;

	KileWidget::StructureWidget	*m_kwStructure;
	KileWidget::FileBrowserWidget 			*m_fileBrowserWidget;
};

#endif
