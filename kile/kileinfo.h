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

#include "kiletoolmanager.h"
#include "kilestdtools.h"
#include "latexoutputfilter.h"
#include "latexoutputinfo.h"

class QWidget;

class KURL;

namespace KileDocument { class Info; }
class KileProject;
class KileProjectItem;
class KileProjectItemList;
class KileFileSelect;
class KileEventFilter;

namespace Kate { class Document;}
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

	QString getCurrentTarget() const { return m_currentTarget; }
	void setTarget(const QString &target) { m_currentTarget=target; }

	virtual Kate::Document* activeDocument() const;

	QString getSelection() const;
	void clearSelection() const;

	virtual const QStringList* labels(KileDocument::Info * info = 0) =0;
	virtual const QStringList* bibItems(KileDocument::Info * info = 0) =0;
	virtual const QStringList* bibliographies(KileDocument::Info * info = 0) = 0;

	bool isOpen(const KURL & url);
	bool	projectIsOpen(const KURL & );

	bool watchFile() { return m_bWatchFile; }

	LatexOutputFilter * outputFilter() { return m_outputFilter; }
	LatexOutputInfoArray * outputInfo() { return m_outputInfo; }
	
	virtual int lineNumber() = 0;
	
	QString relativePath(const QString basepath, const QString & file);

	KileWidget::Structure *structureWidget() { return m_kwStructure; }
	KileWidget::Konsole *texKonsole() { return m_texKonsole; }

	KileDocument::Manager* docManager() const { return m_docManager; }
	KileView::Manager* viewManager() const { return m_viewManager; }
	KileTool::Manager* toolManager() const { return m_manager; }
	KileTool::Factory* toolFactory() const { return m_toolFactory; }
	KileDocument::EditorExtension *editorExtension() const { return m_edit; }

	//FIXME:refactor
	KileFileSelect* fileSelector() const { return KileFS; }
	KileEventFilter* eventFilter() const { return m_eventFilter; }

	QWidget *parentWidget() const { return m_parentWidget; }

	//FIXME: should be in separate template class
	const QString & templAuthor() const { return m_templAuthor; }
	const QString & templDocClassOpt() const { return m_templDocClassOpt; }
	const QString & templEncoding() const { return m_templEncoding; }

protected:
	KileDocument::Manager	*m_docManager;
	KileView::Manager		*m_viewManager;
	KileTool::Manager		*m_manager;
	KileTool::Factory		*m_toolFactory;
	KileWidget::Konsole		*m_texKonsole;

	KileDocument::EditorExtension *m_edit;

	QWidget *m_parentWidget;

	bool 		m_singlemode;
	QString	m_masterName;

	QString	m_currentTarget;
	
	bool m_bWatchFile;

	LatexOutputFilter		*m_outputFilter;
	LatexOutputInfoArray	*m_outputInfo;

	KileWidget::Structure	*m_kwStructure;
	KileFileSelect 			*KileFS;
	KileEventFilter*		m_eventFilter;

	QString m_templAuthor, m_templDocClassOpt, m_templEncoding;
};

#endif
