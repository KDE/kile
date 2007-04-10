//
// C++ Interface: kiledocmanager
//
// Description: 
//
//
// Author: Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>, (C) 2004
//         Michel Ludwig <michel.ludwig@kdemail.net>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef KILEDOCUMENTKILEDOCMANAGER_H
#define KILEDOCUMENTKILEDOCMANAGER_H

#include <qobject.h>

#include "kileconstants.h"

class KURL;
class KFileItem;
class KProgressDialog;
namespace Kate { class Document; class View;}

class TemplateItem;
class KileInfo;
class KileProject;
class KileProjectItem;
class KileProjectItemList;

namespace KileDocument 
{

class Info;
class TextInfo;

/**
@author Jeroen Wijnhout
*/
class Manager : public QObject
{
	Q_OBJECT
public:
	Manager(KileInfo *info, QObject *parent = 0, const char *name = 0);
	~Manager();

public slots:
	Kate::View* createNewJScript();
	Kate::View* createNewLaTeXDocument();

//files
	void newDocumentStatus(Kate::Document *doc);

	/**
	 * Creates a new file on disk.
	 **/
	void fileNew(const KURL &);
	void fileNew();

	void fileSelected(const KURL &);
	void fileSelected(const KileProjectItem * item);
	void fileSelected(const KFileItem *file);

	void fileOpen();
	void fileOpen(const KURL& url, const QString & encoding = QString::null, int index = -1);

	void fileSave();
	void fileSaveAs();

	void saveURL(const KURL &);
	void fileSaveAll(bool amAutoSaving = false, bool disUntitled = false);

	bool fileCloseAllOthers();
	bool fileCloseAll();
	bool fileClose(const KURL & url);
	bool fileClose(Kate::Document *doc = 0L, bool closingproject = false);

//templates
	Kate::View* loadTemplate(TemplateItem*);
	void createTemplate();
	void removeTemplate();
	void replaceTemplateVariables(QString &line);

//projects
	void projectNew();
	KileProject* projectOpen();
	
	/**
	 * @param openProjectItemViews Opens project files in the editor iff openProjectItemViews is set to 'true'.
	 **/
	KileProject* projectOpen(const KURL&, int = 0, int = 1, bool openProjectItemViews = true);

	/**
	 * @param openProjectItemViews Opens project files in the editor iff openProjectItemViews is set to 'true'.
	 **/
	void projectOpenItem(KileProjectItem *item, bool openProjectItemViews = true);

	/**
	 * Saves the state of the project, if @param project is zero, the active project is saved.
	 **/
	void projectSave(KileProject * project = 0);
	void projectAddFiles(const KURL &);
	void projectAddFiles(KileProject * project = 0,const KURL & url = KURL());
	void toggleArchive(KileProjectItem *);
	void buildProjectTree(KileProject *project = 0);
	void buildProjectTree(const KURL &);
	void projectOptions(const KURL &);
	void projectOptions(KileProject *project = 0);
	bool projectClose(const KURL & url = KURL());
	bool projectCloseAll();

	void projectShow();
	void projectRemoveFiles();
	void projectShowFiles();
	void projectAddFile(QString filename, bool graphics=false);
	void projectOpenAllFiles();
	void projectOpenAllFiles(const KURL &);

	KileProject* selectProject(const QString &);

	void addProject(const KileProject *project);
	void addToProject(const KURL &);
	void addToProject(KileProject *, const KURL &);
	void removeFromProject(const KileProjectItem *);
	void storeProjectItem(KileProjectItem *item, Kate::Document *doc);

	void cleanUpTempFiles(const KURL &url, bool silent = false);

	void openDroppedURLs(QDropEvent *e);

signals:
	void projectTreeChanged(const KileProject *);
	void closingDocument(KileDocument::Info *);
	void documentInfoCreated(KileDocument::Info *);

	void updateStructure(bool needToParse, KileDocument::Info*);
	void updateModeStatus();
	void updateReferences(KileDocument::Info *);

	void documentStatusChanged(Kate::Document *, bool, unsigned char reason);

	void addToRecentFiles(const KURL &);
	void addToRecentProjects(const KURL &);
	void removeFromRecentProjects(const KURL &);

	void startWizard();

	void printMsg(int type, const QString & message, const QString &tool = "Kile" );

	void removeFromProjectView(const KURL &);
	void removeFromProjectView(const KileProject *);
	void removeItemFromProjectView(const KileProjectItem *, bool);
	void addToProjectView(const KURL &);
	void addToProjectView(KileProjectItem *item);
	void addToProjectView(const KileProject *);

public:
	QPtrList<KileProject>* projects() { return &m_projects; }
	QPtrList<TextInfo>* textDocumentInfos() { return &m_textInfoList; }

	Kate::Document* docFor(const KURL &url);

	Info* getInfo() const;
	// FIXME: "path" should be changed to a URL, i.e. only the next but one function 
	//        should be used
	TextInfo* textInfoFor(const QString &path) const;
	TextInfo* textInfoForURL(const KURL& url);
	TextInfo* textInfoFor(Kate::Document* doc) const;
	void updateInfos();

	KileProject* projectFor(const KURL &projecturl);
	KileProject* projectFor(const QString & name);
	KileProject* activeProject();
	bool isProjectOpen();
	void updateProjectReferences(KileProject *project);
	QStringList getProjectFiles();

	KileProjectItem* activeProjectItem();
	/**
	 * Finds the project item for the file with URL @param url.
	 * @returns a pointer to the project item, 0 if this file does not belong to a project
	 **/
	KileProjectItem* itemFor(const KURL &url, KileProject *project = 0) const;
	KileProjectItem* itemFor(Info *docinfo, KileProject *project = 0) const;
	KileProjectItem* selectProjectFileItem(const QString &label);
	KileProjectItemList* selectProjectFileItems(const QString &label);

	KileProjectItemList* itemsFor(Info *docinfo) const;

	static const KURL symlinkFreeURL(const KURL& url);

protected:
	void mapItem(TextInfo *docinfo, KileProjectItem *item);

	void trashDoc(TextInfo *docinfo, Kate::Document *doc = 0L);
	Type determineFileType(const KURL& url);

	TextInfo* createTextDocumentInfo(KileDocument::Type type, const KURL &url, const KURL& baseDirectory = KURL());
	void recreateTextDocumentInfo(TextInfo *oldinfo);

	/**
	 * Tries to remove and delete a TextInfo object. The TextInfo object will only be deleted if it isn't referenced
	 * by any project item or if is is only referenced by a project that should be closed.
	 * @param closingproject Indicates whether the TextInfo object should be removed as part of a project close
	 *                       operation.
	 * @warning This method does not close or delete any Kate documents that are associated with the TextInfo object !
	 **/
	bool removeTextDocumentInfo(TextInfo *docinfo, bool closingproject = false);
	Kate::Document* createDocument(const QString& name, const KURL& url, TextInfo *docinfo, const QString & encoding, const QString & highlight);

	/**
	 *  Creates a document with the specified text.
	 * 
	 *  @param extension The extension of the file that should be created without leading "."
	 **/
	Kate::View* createDocumentWithText(const QString& text, KileDocument::Type type = KileDocument::Text, const QString& extension = QString::null, const KURL& baseDirectory = KURL());

	Kate::View* loadText(KileDocument::Type type, const QString& name, const KURL &url, const QString & encoding = QString::null, bool create = true, const QString & highlight  = QString::null, const QString &text = QString::null, int index = -1, const KURL& baseDirectory = KURL());
	Kate::View* loadItem(KileDocument::Type type, KileProjectItem *item, const QString & text = QString::null, bool openProjectItemViews = true);

private:
	QPtrList<TextInfo>				m_textInfoList;
	KileInfo					*m_ki;
	QPtrList<KileProject>		m_projects;
	KProgressDialog				*m_kpd;
	
	void dontOpenWarning(KileProjectItem *item, const QString &action, const QString &filetype);
	void cleanupDocumentInfoForProjectItems(KileDocument::Info *info);

	QStringList autosaveWarnings;

};

}

#endif
