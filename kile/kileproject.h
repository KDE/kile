/***************************************************************************
                          kileproject.h -  description
                             -------------------
    begin                : Fri Aug 1 2003
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
#ifndef KILEPROJECT_H
#define KILEPROJECT_H

#include <qobject.h>
#include <qptrlist.h>

#include <kdebug.h>
#include <kurl.h>

class QString;
class QStringList;
class KSimpleConfig;
class KileDocumentInfo;

/**
 * KileURLTree
 **/
class KileURLTree
{
public:
	KileURLTree(KileURLTree *parent, const KURL & url);
	~KileURLTree();

	KileURLTree*	firstChild() {return m_child; }
	KileURLTree*	sibling() { return m_sibling;}
	KileURLTree*	parent() { return m_parent;}

	void setChild(KileURLTree *item) { m_child = item; }
	void setSibling(KileURLTree *item) { m_sibling = item; }

private:
	KileURLTree	*m_parent;
	KURL				m_url;
	KileURLTree	*m_sibling;
	KileURLTree	*m_child;
};

/**
 * KileProjectItem
 **/
class KileProject;
class KileProjectItem : public QObject
{
	Q_OBJECT

public:
	KileProjectItem(KileProject *project = 0, const KURL &url = KURL());
	~KileProjectItem() { kdDebug() << "DELETING " << m_path << endl;}

	bool operator==(const KileProjectItem& item) { return m_url  == item.url();}

	void setInfo(KileDocumentInfo * docinfo);
	KileDocumentInfo*	getInfo() { return m_docinfo; }

	const KileProject* project() const{ return m_project;}

	/**
	 * @returns absolute URL of this item
	 **/
	const KURL&	url() const { return m_url; }
	/**
	 * @returns path of this item relative to the project file
	 **/
	const QString& path() const { return m_path; }

	bool	isOpen() const { return m_bOpen; }
	void setOpenState(bool state) { m_bOpen = state; }

	const QString& encoding() { return m_encoding;}
	void setEncoding(const QString& encoding) {m_encoding = encoding;}

	const QString& highlight() { return m_highlight;}
	void setHighlight(const QString& highlight) {m_highlight = highlight;}

	void setParent(KileProjectItem * item) { m_parent = item;}
	KileProjectItem* parent() { return m_parent; }

public slots:
	void changeURL(const KURL &url) { m_url = url;  kdDebug() << "changeURL " << url.path() << endl; emit(urlChanged(this));}
	void changePath(const QString& path) { m_path = path;}

signals:
	void urlChanged(KileProjectItem*);

private:
	KileProject			*m_project;
	KURL					m_url;
	QString				m_path;
	QString				m_encoding;
	QString				m_highlight;
	bool						m_bOpen;
	KileDocumentInfo *m_docinfo;
	KileProjectItem		*m_parent;
};

class  KileProjectItemList : public QPtrList<KileProjectItem>
{
public:
	KileProjectItemList() { setAutoDelete(true); }
	~KileProjectItemList() { kdDebug() << "DELETING KILEPROJECTITEMLIST" << endl;}
};

/**
 * KileProject
 **/
class KileProject : public QObject
{
	Q_OBJECT

public:
	KileProject(const QString& name, const KURL& url);
	KileProject(const KURL& url);

	~KileProject() {}

	void setName(const QString & name) { m_name = name; emit (nameChanged(name));}
	const QString& name() const { return m_name; }

	const KURL& url() const { return m_projecturl; }
	const KURL& baseURL() const { return m_baseurl; }

	KileProjectItem* item(const KURL &);
	KileProjectItemList* items() { return &m_projectitems; }

	bool contains(const KURL&);
	KileProjectItem *rootItem();

	void buildProjectTree();
	const KileURLTree* projectTree() { return m_projecttree; }

signals:
	void nameChanged(const QString &);

public slots:
	bool load();
	bool save();
	
	void add(KileProjectItem*);
	void remove(KileProjectItem*);

	void itemRenamed(KileProjectItem*);

	//debugging
	void dump();

signals:
	void loadFile(const KURL &url , const QString & encoding);

private:
	void 	init(const QString& name, const KURL& url);
	QString	findRelativePath(const KURL&);

private:
	QString		m_name;
	KURL			m_projecturl;
	KURL			m_baseurl;

	KileProjectItem			*m_rootItem;
	KileProjectItemList	m_projectitems;
	KileURLTree			*m_projecttree;

	KSimpleConfig	*m_config;
};

#endif
