/***************************************************************************
                          kileproject.h -  description
                             -------------------
    begin                : Tue Aug 12 2003
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
#ifndef KILEPROJECTVIEW_H
#define KILEPROJECTVIEW_H

#include <ktrader.h>
#include <klistview.h>

#include "kileproject.h"

class KURL;
class KPopupMenu;
class KToggleAction;
class KileDocument::Info;
class KileInfo;

namespace KileType {enum ProjectView { Project=0, ProjectItem, ProjectExtra, File, Folder};}

class KileProjectViewItem : public QObject, public KListViewItem
{
	Q_OBJECT

public:
	KileProjectViewItem (QListView *parent, KileProjectItem *item, bool ar = false) : KListViewItem(parent, item->url().fileName()), m_folder(-1), m_projectItem(item) { setArchiveState(ar);}
	KileProjectViewItem (QListView *parent, QListViewItem *after, KileProjectItem *item, bool ar = false) : KListViewItem(parent, after, item->url().fileName()), m_folder(-1), m_projectItem(item) { setArchiveState(ar);}
	KileProjectViewItem (QListViewItem *parent, KileProjectItem *item, bool ar = false) : KListViewItem(parent, item->url().fileName()), m_folder(-1), m_projectItem(item) { setArchiveState(ar);}

	//use this to create folders
	KileProjectViewItem (QListViewItem *parent, const QString & name) : KListViewItem(parent, name), m_folder(-1), m_projectItem(0L) {}

	//use this to create non-project files
	KileProjectViewItem (QListView *parent, const QString & name) : KListViewItem(parent, name), m_folder(-1), m_projectItem(0L) {}

	~KileProjectViewItem() {kdDebug() << "DELETING PROJVIEWITEM " << m_url.fileName() << endl;}

	KileProjectItem* projectItem() { return m_projectItem; }

	KileProjectViewItem* parent() { return dynamic_cast<KileProjectViewItem*>(KListViewItem::parent()); }
	KileProjectViewItem* firstChild() { return dynamic_cast<KileProjectViewItem*>(KListViewItem::firstChild()); }
	KileProjectViewItem* nextSibling() { return dynamic_cast<KileProjectViewItem*>(KListViewItem::nextSibling()); }

	void setInfo(KileDocument::Info *docinfo) { m_docinfo = docinfo;}
	KileDocument::Info * getInfo() { return m_docinfo;}

	void setType(KileType::ProjectView type) {m_type = type;}
	KileType::ProjectView type() const { return m_type;}

	int compare(QListViewItem * i, int col, bool ascending) const;

	void setURL(const KURL & url) { m_url=url;}
	const KURL& url() { return m_url;}

	void setArchiveState(bool ar) { setText(1,ar ? "*" : "");}

	void setFolder(int folder) { m_folder = folder; }
	int folder() { return m_folder; }

public slots:
	void urlChanged(const KURL & url);
	void nameChanged(const QString & name);
	void isrootChanged(bool isroot);

private:
	KURL	m_url;
	KileType::ProjectView	m_type;
	KileDocument::Info	*m_docinfo;
	int   m_folder;
	KileProjectItem *m_projectItem;
};

class KileProjectView : public KListView
{
	Q_OBJECT

public:
	KileProjectView(QWidget *parent, KileInfo *ki);

signals:
	void fileSelected(const KileProjectItem *);
	void fileSelected(const KURL &);
	void saveURL(const KURL&);
	void closeURL(const KURL&);
	void projectOptions(const KURL &);
	void projectArchive(const KURL &);
	void addFiles(const KURL &);
	void toggleArchive(KileProjectItem *);
	void closeProject(const KURL &);
	void addToProject(const KURL &);
	void removeFromProject(const KileProjectItem *);
	void buildProjectTree(const KURL &);

public slots:
	void slotClicked(QListViewItem * item = 0);

	void slotFile(int id);
	void slotProjectItem(int id);
	void slotProject(int id);

	void slotRun(int id);

	void refreshProjectTree(const KileProject *);

public:
	void add(const KileProject *project);
	KileProjectViewItem* add(KileProjectItem *item, KileProjectViewItem * projvi  = 0);
	const KileProjectViewItem* addTree(KileProjectItem *item, KileProjectViewItem * projvi );
	void add(const KURL & url);

	void remove(const KileProject *project);
	void removeItem(const KileProjectItem *, bool);
	void remove(const KURL & url);

	KileProjectViewItem* projectViewItemFor(const KURL &);
	KileProjectViewItem* itemFor(const KURL &);
	KileProjectViewItem* parentFor(const KileProjectItem *projitem, KileProjectViewItem *projvi);

private slots:
	void popup(KListView *, QListViewItem *, const QPoint &);

private:
	void makeTheConnection(KileProjectViewItem *);
	KileProjectViewItem* folder(const KileProjectItem *item, KileProjectViewItem *);

private:
	KileInfo					*m_ki;
	KPopupMenu		*m_popup;
	uint						m_nProjects;
	KToggleAction		*m_toggle;

	KTrader::OfferList m_offerList;
};

#endif
