/***************************************************************************
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
namespace KileDocument { class Info; class TextInfo; }

const QString SOURCE_EXTENSIONS = ".tex .ltx .bib .mp";
const QString PACKAGE_EXTENSIONS = ".cls .sty .dtx";
const QString IMAGE_EXTENSIONS = ".eps .pdf .dvi .ps .fig .gif .jpg .jpeg .png";

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
	~KileProjectItem() { kdDebug() << "DELETING PROJITEM " << m_path << endl;}

	bool operator==(const KileProjectItem& item) { return m_url  == item.url();}

	enum Type { ProjectFile = 0, Source, Package, Image, Other /* should be the last item*/ };

	int type() const { return m_type; }
	void setType(int type) { m_type = type; }

	bool archive() const { return m_archive; }
	void setArchive(bool ar) { m_archive = ar; }

	void setInfo(KileDocument::TextInfo * docinfo);
	KileDocument::TextInfo* getInfo() { return m_docinfo; }

	KileProject* project() const { return m_project;}

	/**
	 * @returns absolute URL of this item
	 **/
	const KURL&	url() const { return m_url; }

	/**
	 * @returns path of this item relative to the project file
	 **/
	const QString& path() const { return m_path; }

	bool isOpen() const { return m_bOpen; }
	void setOpenState(bool state) { m_bOpen = state; }

	const QString& encoding() const { return m_encoding;}
	void setEncoding(const QString& encoding) {m_encoding = encoding;}

	const QString& highlight() { return m_highlight;}
	void setHighlight(const QString& highlight) {m_highlight = highlight;}

	uint lineNumber() { return m_nLine; }
	void setLineNumber(uint l) { m_nLine = l; }

	uint columnNumber() { return m_nColumn; }
	void setColumnNumber(uint l) { m_nColumn = l; }

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

	void print(int level);

public slots:
	/**
	 * @warning Does nothing if "url" is empty !
	 **/ 
	void changeURL(const KURL &url);
	void changePath(const QString& path) { m_path = path;}

signals:
	void urlChanged(KileProjectItem*);

private:
	KileProject		*m_project;
	KURL			m_url;
	QString			m_path;
	QString			m_encoding;
	QString			m_highlight;
	bool			m_bOpen, m_archive;
	int			m_type;
	KileDocument::TextInfo	*m_docinfo;
	KileProjectItem		*m_parent, *m_child, *m_sibling;
	uint			m_nLine, m_nColumn;
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

	~KileProject();

	void setName(const QString & name) { m_name = name; emit (nameChanged(name));}
	const QString& name() const { return m_name; }

	void setMasterDocument(const QString & master);
	const QString& masterDocument() const { return m_masterDocument; }

	void setExtensions(KileProjectItem::Type type, const QString & ext);
	const QString & extensions(KileProjectItem::Type type) { return m_extensions[type-1]; }
	void setExtIsRegExp(KileProjectItem::Type type, bool is) { m_extIsRegExp[type-1] = is; }
	bool extIsRegExp(KileProjectItem::Type type) { return m_extIsRegExp[type-1]; }

	void setQuickBuildConfig(const QString & cfg) { m_quickBuildConfig = cfg; }
	const QString & quickBuildConfig() { return m_quickBuildConfig; }

    void setLastDocument(const KURL &url);
    const KURL & lastDocument() const { return m_lastDocument; }

	void setMakeIndexOptions(const QString & opt) { m_makeIndexOptions = opt; }
	const QString & makeIndexOptions() { return m_makeIndexOptions; }
	void readMakeIndexOptions();
	void setUseMakeIndexOptions(bool use) { m_useMakeIndexOptions = use; }
	void writeUseMakeIndexOptions();
	bool useMakeIndexOptions() { return m_useMakeIndexOptions; }

	const KURL& url() const { return m_projecturl; }
	void setURL(const KURL & url ) { m_projecturl = url; }
	const KURL& baseURL() const { return m_baseurl; }

	KileProjectItem* item(const KURL &);
	KileProjectItem* item(const KileDocument::Info *info);
	KileProjectItemList* items() { return &m_projectitems; }

	KSimpleConfig *config() { return m_config; }

	bool contains(const KURL&);
	bool contains(const KileDocument::Info *info);
	KileProjectItem *rootItem(KileProjectItem *) const;
	const QPtrList<KileProjectItem>* rootItems() const {return &m_rootItems;}
	bool isInvalid(){ return m_invalid;}

signals:
	void nameChanged(const QString &);
	void masterDocumentChanged(const QString &);
	void projectTreeChanged(const KileProject *);

public slots:
	bool load();
	bool save();

	void add(KileProjectItem*);
	void remove(const KileProjectItem*);

	void itemRenamed(KileProjectItem*);

	void buildProjectTree(); // moved to slots by tbraun

	//debugging
	void dump();

signals:
	void loadFile(const KURL &url , const QString & encoding);

private:
	void init(const QString& name, const KURL& url);
	QString	findRelativePath(const KURL&);
	void setType(KileProjectItem *item);
  	QString addBaseURL(const QString &path);
  	QString removeBaseURL(const QString &path);

private:
	QString		m_name, m_quickBuildConfig, m_kileversion, m_kileprversion;
	KURL		m_projecturl, m_baseurl, m_lastDocument;
	bool		m_invalid;
	QPtrList<KileProjectItem> m_rootItems;
	KileProjectItemList	m_projectitems;

	QString		m_extensions[3];
	QRegExp		m_reExtensions[3];
	bool			m_extIsRegExp[3];

	QString		m_masterDocument, m_makeIndexOptions;
	bool			m_useMakeIndexOptions;

	KSimpleConfig	*m_config;
};

#endif
