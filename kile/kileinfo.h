/***************************************************************************
                          kileinfointerface.h  -  description
                             -------------------
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
#include <qptrlist.h>

#include <kdebug.h>
#include <kurl.h>

#include "kiletoolmanager.h"
#include "kilestdtools.h"
#include "latexoutputfilter.h"
#include "latexoutputinfo.h"

class QWidget;

namespace KileDocument { class Info; }
class KileProject;
class KileProjectItem;
class KileProjectItemList;
class KileFileSelect;
class KileEventFilter;

namespace Kate { class Document;}

class KileBottomBar;
namespace KileDocument { class Manager; class EditorExtension; }
namespace KileView { class Manager; }
namespace KileWidget { class Structure; class Konsole; }

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

	QString getCurrentTarget() const { return m_currentTarget; }
	void setTarget(const QString &target) { m_currentTarget=target; }

	virtual Kate::Document* activeDocument() const;

	QString getSelection() const;
	void clearSelection() const;

	virtual const QStringList* allLabels(KileDocument::Info * info = 0L);
	virtual const QStringList* allBibItems(KileDocument::Info * info = 0L);
	virtual const QStringList* allBibliographies(KileDocument::Info * info = 0L);
	virtual const QStringList* allDependencies(KileDocument::Info * info = 0L);
	virtual const QStringList* allNewCommands(KileDocument::Info * info = 0L);

	QString lastModifiedFile(KileDocument::Info * info = 0L);

private:
	const QStringList* retrieveList(const QStringList* (KileDocument::Info::*getit)() const, KileDocument::Info * docinfo = 0L);
	QStringList m_listTemp;

public:
	bool isOpen(const KURL & url);
	bool	projectIsOpen(const KURL & );

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

	KileDocument::Manager* docManager() const { return m_docManager; }
	KileView::Manager* viewManager() const { return m_viewManager; }
	KileTool::Manager* toolManager() const { return m_manager; }
	KileTool::Factory* toolFactory() const { return m_toolFactory; }
	KileDocument::EditorExtension *editorExtension() const { return m_edit; }

	//FIXME:refactor
	KileFileSelect* fileSelector() const { return m_fileSelector; }
	KileEventFilter* eventFilter() const { return m_eventFilter; }

	QWidget *parentWidget() const { return m_parentWidget; }

protected:
	KileDocument::Manager		*m_docManager;
	KileView::Manager		*m_viewManager;
	KileTool::Manager		*m_manager;
	KileTool::Factory		*m_toolFactory;
	KileWidget::Konsole		*m_texKonsole;
	KileWidget::Output		*m_outputWidget;
	KileWidget::LogMsg		*m_logWidget;
	KileBottomBar			*m_bottomBar;

	KileDocument::EditorExtension 	*m_edit;

	QWidget *m_parentWidget;

	bool 		m_singlemode;
	QString	m_masterName;

	QString	m_currentTarget;
	
	bool m_bWatchFile, m_logPresent;

	LatexOutputFilter		*m_outputFilter;
	LatexOutputInfoArray		*m_outputInfo;

	KileWidget::Structure		*m_kwStructure;
	KileFileSelect 			*m_fileSelector;
	KileEventFilter*		m_eventFilter;

	KileProject			*m_defaultProject;
};

#endif
