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

/**
 * KileProjectItem
 **/
class KileProjectItem : public QObject
{
	Q_OBJECT

public:
	KileProjectItem(const KURL &url = KURL()) : m_url(url) { m_encoding=QString::null; m_bOpen = true;}
	~KileProjectItem() { kdDebug() << "DELETING " << m_path << endl;}

	bool operator==(const KileProjectItem& item) { return m_url  == item.url();}

	/**
	 * @returns absolute URL of this item
	 **/
	const KURL&	url() const { return m_url; }
	/**
	 * @returns path of this item relative to the project file
	 **/
	const QString& path() { return m_path; }

	bool	isOpen() const { return m_bOpen; }
	void setOpenState(bool state) { m_bOpen = state; }

	const QString& encoding() { return m_encoding;}
	void setEncoding(const QString& encoding) {m_encoding = encoding;}

	void changeURL(const KURL &url) { m_url = url;  emit(urlChanged(this));}
	void changePath(const QString& path) { m_path = path;}

signals:
	void urlChanged(KileProjectItem*);

private:
	KURL		m_url;
	QString	m_path;
	QString	m_encoding;
	bool			m_bOpen;
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

	const QString& name() { return m_name; }
	const KURL& url() { return m_projecturl; }
	const KURL& baseURL() { return m_baseurl; }

	KileProjectItemList* items() { return &m_projectitems; }

	bool contains(const KURL&);
	KileProjectItem *rootItem() { return m_rootItem; }

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

	KSimpleConfig	*m_config;
};

#endif
