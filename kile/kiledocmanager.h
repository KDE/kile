//
// C++ Interface: kiledocmanager
//
// Description: 
//
//
// Author: Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>, (C) 2004
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

class KURL;
class KFileItem;
namespace Kate { class Document; class View;}

class TemplateItem;
class KileInfo;
class KileProject;
class KileProjectItem;
class KileProjectItemList;

namespace KileDocument 
{
class Info;
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

//files
	Info* createDocumentInfo(const KURL &url);
	bool removeDocumentInfo(Info *docinfo, bool closingproject = false);
	Kate::Document* createDocument(Info *docinfo, const QString & encoding, const QString & highlight);

	Kate::View* createDocumentWithText(const QString & text);

	Kate::View* load( const KURL &url , const QString & encoding = QString::null, bool create = true, const QString & highlight  = QString::null, const QString &text = QString::null);
	Kate::View* loadItem(KileProjectItem *item, const QString & text = QString::null);

	void setHighlightMode(Kate::Document * doc, const QString & highlight = QString::null);
	void slotNameChanged(Kate::Document *);
	void newDocumentStatus(Kate::Document *doc);

	void fileNew(const KURL &);
	void fileNew();

	void fileSelected(const KURL &);
	void fileSelected(const KileProjectItem * item);
	void fileSelected(const KFileItem *file);

	void fileOpen();
	void fileOpen(const KURL& url, const QString & = QString::null);

	void saveURL(const KURL &);
	void fileSaveAll(bool amAutoSaving = false);

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
	void projectOpen();
	void projectOpen(const KURL&, int = 0, int = 1);
	void projectOpenItem(KileProjectItem *item);

	/**
	 * Saves the state of the project, if @param project is zero, the active project is saved.
	 **/
	void projectSave(KileProject * project = 0);
	void projectAddFiles(const KURL &);
	void projectAddFiles(KileProject * project = 0);
	void toggleArchive(KileProjectItem *);
	bool projectArchive(const KURL &);
	bool projectArchive(KileProject *project  = 0);
	void buildProjectTree(KileProject *project = 0);
	void buildProjectTree(const KURL &);
	void projectOptions(const KURL &);
	void projectOptions(KileProject *project = 0);
	bool projectClose(const KURL & url = KURL());
	bool projectCloseAll();

	KileProject* selectProject(const QString &);

	void addProject(const KileProject *project);
	void addToProject(const KURL &);
	void addToProject(KileProject *, const KURL &);
	void removeFromProject(const KileProjectItem *);
	void storeProjectItem(KileProjectItem *item, Kate::Document *doc);

signals:
	void projectTreeChanged(const KileProject *);
	void closingDocument(KileDocument::Info *);

	void updateStructure(bool, KileDocument::Info*);
	void updateModeStatus();

	void addToRecentFiles(const KURL &);
	void addToRecentProjects(const KURL &);
	void removeFromRecentProjects(const KURL &);

	void startWizard();

public:
	QPtrList<KileProject>* projects() { return &m_projects; }
	QPtrList<Info>* documentInfos() { return &m_infoList; }

	void trashDoc(Info *docinfo);
	Kate::Document* docFor(const KURL &url);

	Info* getInfo() const;
	Info* infoFor(const QString &path) const;
	Info* infoFor(Kate::Document* doc) const;

	KileProject* projectFor(const KURL &projecturl);
	KileProject* projectFor(const QString & name);
	KileProject* activeProject();

	KileProjectItem* activeProjectItem();
	/**
	 * Finds the project item for the file with URL @param url.
	 * @returns a pointer to the project item, 0 if this file does not belong to a project
	 **/
	KileProjectItem* itemFor(const KURL &url, KileProject *project = 0) const;
	KileProjectItem* itemFor(Info *docinfo, KileProject *project = 0) const;

	KileProjectItemList* itemsFor(Info *docinfo) const;

	void mapItem(Info *docinfo, KileProjectItem *item);

private:
	QPtrList<Info>	m_infoList;
	KileInfo					*m_ki;
	QPtrList<KileProject>		m_projects;
};

}

#endif
