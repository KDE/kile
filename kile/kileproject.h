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
#include <qregexp.h>

#include <kdebug.h>
#include <kurl.h>

class QString;
class QStringList;
class KSimpleConfig;
class KileDocumentInfo;

/**
 * KileProjectItem
 **/
class KileProject;
class KileProjectItemList;
class KileProjectItem : public QObject
{
	Q_OBJECT

public:
	KileProjectItem(KileProject *project = 0, const KURL &url = KURL(), int type = Source);
	~KileProjectItem() { kdDebug() << "DELETING PROJITEM" << m_path << endl;}

	bool operator==(const KileProjectItem& item) { return m_url  == item.url();}

	enum Type { ProjectFile = 0, Source, Other};

	int type() const { return m_type; }
	void setType(int type) { m_type = type; }

	bool archive() const { return m_archive; }
	void setArchive(bool ar) { m_archive = ar; }

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

	//project tree functions
	void setParent(KileProjectItem * item);

protected:
	void setChild(KileProjectItem *item) { m_child = item; }
	void setSibling(KileProjectItem *item) { m_sibling = item; }

public:
	KileProjectItem* parent() const { return m_parent; }
	KileProjectItem* firstChild() const { return m_child;}
	KileProjectItem* sibling() const { return m_sibling; }

	void allChildren(QPtrList<KileProjectItem> *) const;

	KileProjectItem * print(int level);

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
	bool						m_bOpen, m_archive;
	int							m_type;
	KileDocumentInfo *m_docinfo;
	KileProjectItem		*m_parent, *m_child, *m_sibling;
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

	void setArchiveCommand(const QString &command) { m_archiveCommand = command;}
	const QString& archiveCommand() { return m_archiveCommand;}

	void setExtensions(const QString & ext);
	const QString & extensions() { return m_extensions; }

	const KURL& url() const { return m_projecturl; }
	const KURL& baseURL() const { return m_baseurl; }

	KileProjectItem* item(const KURL &);
	KileProjectItemList* items() { return &m_projectitems; }

	bool contains(const KURL&);
	KileProjectItem *rootItem(KileProjectItem *) const;
	const QPtrList<KileProjectItem>* rootItems() const { return &m_rootItems;}

	void buildProjectTree();

signals:
	void nameChanged(const QString &);
	void projectTreeChanged(const KileProject *);

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
	QString		m_name, m_archiveCommand;
	KURL			m_projecturl;
	KURL			m_baseurl;

	QPtrList<KileProjectItem> m_rootItems;
	KileProjectItemList	m_projectitems;

	QString		m_extensions;
	QRegExp		m_reExtensions;

	KSimpleConfig	*m_config;
};

#endif
