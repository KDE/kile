/***************************************************************************
    begin                : Tue Aug 12 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
                           (C) 2006 by Michel Ludwig
    email                : Jeroen.Wijnhout@kdemail.net
                           michel.ludwig@kdemail.net
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

#include <kmimetypetrader.h>
#include <k3listview.h>

#include "kileproject.h"
//Added by qt3to4:
#include <QDropEvent>

class KUrl;
class KMenu;
class KToggleAction;
class KileDocument::Info;
class KileInfo;

namespace KileType {enum ProjectView { Project=0, ProjectItem, ProjectExtra, File, Folder};}

class KileProjectViewItem : public QObject, public K3ListViewItem
{
	Q_OBJECT

public:
	KileProjectViewItem (Q3ListView *parent, KileProjectItem *item, bool ar = false) : K3ListViewItem(parent, item->url().fileName()), m_folder(-1), m_projectItem(item) { setArchiveState(ar);}
	KileProjectViewItem (Q3ListView *parent, Q3ListViewItem *after, KileProjectItem *item, bool ar = false) : K3ListViewItem(parent, after, item->url().fileName()), m_folder(-1), m_projectItem(item) { setArchiveState(ar);}
	KileProjectViewItem (Q3ListViewItem *parent, KileProjectItem *item, bool ar = false) : K3ListViewItem(parent, item->url().fileName()), m_folder(-1), m_projectItem(item) { setArchiveState(ar);}

	//use this to create folders
	KileProjectViewItem (Q3ListViewItem *parent, const QString & name) : K3ListViewItem(parent, name), m_folder(-1), m_projectItem(0L) {}

	//use this to create non-project files
	KileProjectViewItem (Q3ListView *parent, const QString & name) : K3ListViewItem(parent, name), m_folder(-1), m_projectItem(0L) {}
	
	KileProjectViewItem (Q3ListView *parent, const KileProject *project) : K3ListViewItem(parent, project->name()), m_folder(-1), m_projectItem(0L) {}
	

	~KileProjectViewItem() {KILE_DEBUG() << "DELETING PROJVIEWITEM " << m_url.fileName() << endl;}

	KileProjectItem* projectItem() { return m_projectItem; }

	KileProjectViewItem* parent() { return dynamic_cast<KileProjectViewItem*>(K3ListViewItem::parent()); }
	KileProjectViewItem* firstChild() { return dynamic_cast<KileProjectViewItem*>(K3ListViewItem::firstChild()); }
	KileProjectViewItem* nextSibling() { return dynamic_cast<KileProjectViewItem*>(K3ListViewItem::nextSibling()); }

	void setInfo(KileDocument::Info *docinfo) { m_docinfo = docinfo;}
	KileDocument::Info * getInfo() { return m_docinfo;}

	void setType(KileType::ProjectView type) {m_type = type;}
	KileType::ProjectView type() const { return m_type;}

	int compare(Q3ListViewItem * i, int col, bool ascending) const;

	void setURL(const KUrl & url) { m_url=url;}
	const KUrl& url() { return m_url;}

	void setArchiveState(bool ar) { setText(1,ar ? "*" : "");}

	void setFolder(int folder) { m_folder = folder; }
	int folder() { return m_folder; }

public slots:
	/**
	 * @warning Does nothing if "url" is empty !
	 **/ 
	void urlChanged(const KUrl & url);
	void nameChanged(const QString & name);
	void isrootChanged(bool isroot);

private slots:
	/**
	 * Dummy slot, simply forwarding to urlChanged(const KUrl& url).
	 **/
	void slotURLChanged(KileDocument::Info*, const KUrl & url);


private:
	KUrl	m_url;
	KileType::ProjectView	m_type;
	KileDocument::Info	*m_docinfo;
	int   m_folder;
	KileProjectItem *m_projectItem;
};

class KileProjectView : public K3ListView
{
	Q_OBJECT

public:
	KileProjectView(QWidget *parent, KileInfo *ki);

signals:
	void fileSelected(const KileProjectItem *);
	void fileSelected(const KUrl &);
	void saveURL(const KUrl&);
	void closeURL(const KUrl&);
	void projectOptions(const KUrl &);
	void projectArchive(const KUrl &);
	void addFiles(const KUrl &);
	void openAllFiles(const KUrl &);
	void toggleArchive(KileProjectItem *);
	void closeProject(const KUrl &);
	void addToProject(const KUrl &);
	void removeFromProject(const KileProjectItem *);
	void buildProjectTree(const KUrl &);

public slots:
	void slotClicked(Q3ListViewItem * item = 0);

	void slotFile(int id);
	void slotProjectItem(int id);
	void slotProject(int id);

	void slotRun(int id);

	void refreshProjectTree(const KileProject *);
	void add(const KUrl & url);
	void add(const KileProject *project);
	void remove(const KUrl & url);
	void remove(const KileProject *project);
	void removeItem(const KileProjectItem *, bool);
	KileProjectViewItem* add(KileProjectItem *item, KileProjectViewItem * projvi  = 0);

public:
	void addTree(KileProjectItem *item, KileProjectViewItem * projvi );

	KileProjectViewItem* projectViewItemFor(const KUrl &);
	KileProjectViewItem* itemFor(const KUrl &);
	KileProjectViewItem* parentFor(const KileProjectItem *projitem, KileProjectViewItem *projvi);

protected:
	virtual bool acceptDrag(QDropEvent *e) const;

private slots:
	void popup(K3ListView *, Q3ListViewItem *, const QPoint &);

private:
	void makeTheConnection(KileProjectViewItem *);
	KileProjectViewItem* folder(const KileProjectItem *item, KileProjectViewItem *);

private:
	KileInfo					*m_ki;
	KMenu		*m_popup;
	uint						m_nProjects;
	KToggleAction		*m_toggle;

	KService::List m_offerList;
};

#endif
