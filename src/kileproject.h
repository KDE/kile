/***************************************************************************************
    begin                : Fri Aug 1 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                           (C) 2009 by Michel Ludwig (michel.ludwig@kdemail.net)
 ***************************************************************************************/

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

#include <QObject>
#include <QList>
#include <QRegExp>

#include "kiledebug.h"

#include <KConfig>
#include <KUrl>
#include <KTextEditor/View>

class QString;
class QStringList;
namespace KileDocument { class Info; class TextInfo; class Extensions; }

/**
 * KileProjectItem
 **/
class KileProject;
class KileProjectItem : public QObject
{
	Q_OBJECT

public:
	explicit KileProjectItem(KileProject *project = 0, const KUrl &url = KUrl(), int type = Source);
	~KileProjectItem() { KILE_DEBUG() << "DELETING PROJITEM " << m_path << endl;}

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
	const KUrl&	url() const { return m_url; }

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

	const QString& mode() { return m_mode;}
	void setMode(const QString& mode) {m_mode = mode;}

	uint lineNumber() { return m_nLine; }
	void setLineNumber(uint l) { m_nLine = l; }

	uint columnNumber() { return m_nColumn; }
	void setColumnNumber(uint l) { m_nColumn = l; }

	int order() const { return m_order; }
	void setOrder(int i);

	//project tree functions
	void setParent(KileProjectItem * item);

	void load();
	void save();

	void loadDocumentAndViewSettings();
	void saveDocumentAndViewSettings();

protected:
	void setChild(KileProjectItem *item) { m_child = item; }
	void setSibling(KileProjectItem *item) { m_sibling = item; }

	void loadViewSettings(KTextEditor::View *view, int viewIndex);
	void saveViewSettings(KTextEditor::View *view, int viewIndex);

	void loadDocumentSettings(KTextEditor::Document *document);
	void saveDocumentSettings(KTextEditor::Document *document);

public:
	KileProjectItem* parent() const { return m_parent; }
	KileProjectItem* firstChild() const { return m_child;}
	KileProjectItem* sibling() const { return m_sibling; }

	void allChildren(QList<KileProjectItem*>* list) const;

	void print(int level);

public Q_SLOTS:
	/**
	 * @warning Does nothing if "url" is empty !
	 **/ 
	void changeURL(const KUrl &url);
	void changePath(const QString& path) { m_path = path;}

private Q_SLOTS:
	void slotChangeURL(KileDocument::Info* info, const KUrl& url);

Q_SIGNALS:
	void urlChanged(KileProjectItem*);

private:
	KileProject		*m_project;
	KUrl			m_url;
	QString			m_path;
	QString			m_encoding;
	QString			m_mode;
	QString			m_highlight;
	bool			m_bOpen, m_archive;
	int			m_type;
	KileDocument::TextInfo	*m_docinfo;
	KileProjectItem		*m_parent, *m_child, *m_sibling;
	uint			m_nLine, m_nColumn;
	int			m_order;
};

/**
 * KileProject
 **/
class KileProject : public QObject
{
	Q_OBJECT
	friend class KileProjectItem;

public:
	KileProject(const QString& name, const KUrl& url, KileDocument::Extensions *extensions);
	KileProject(const KUrl& url, KileDocument::Extensions *extensions);

	~KileProject();

	void setName(const QString & name) { m_name = name; emit (nameChanged(name));}
	const QString& name() const { return m_name; }

	void setMasterDocument(const QString & master);
	const QString& masterDocument() const { return m_masterDocument; }

	void setExtensions(KileProjectItem::Type type, const QString & ext);
	const QString & extensions(KileProjectItem::Type type) { return m_extensions[type-1]; }

	void setDefaultGraphicExt(const QString & ext);
	const QString & defaultGraphicExt();

	void setQuickBuildConfig(const QString & cfg) { m_quickBuildConfig = cfg; }
	const QString & quickBuildConfig() { return m_quickBuildConfig; }

	void setLastDocument(const KUrl &url);
	const KUrl& lastDocument() const { return m_lastDocument; }

	void setMakeIndexOptions(const QString & opt) { m_makeIndexOptions = opt; }
	const QString & makeIndexOptions() { return m_makeIndexOptions; }
	void readMakeIndexOptions();
	void setUseMakeIndexOptions(bool use) { m_useMakeIndexOptions = use; }
	void writeUseMakeIndexOptions();
	bool useMakeIndexOptions() { return m_useMakeIndexOptions; }

	const KUrl& url() const { return m_projecturl; }
	void setURL(const KUrl & url ) { m_projecturl = url; }
	const KUrl& baseURL() const { return m_baseurl; }

	KileProjectItem* item(const KUrl &);
	KileProjectItem* item(const KileDocument::Info *info);
	QList<KileProjectItem*> items() { return m_projectItems; }

	KConfig *config() { return m_config; }

	bool contains(const KUrl&);
	bool contains(const KileDocument::Info *info);
	KileProjectItem *rootItem(KileProjectItem *) const;
	const QList<KileProjectItem*>& rootItems() const {return m_rootItems;}
	bool isInvalid(){ return m_invalid;}
	QString archiveFileList() const;

Q_SIGNALS:
	void nameChanged(const QString &);
	void masterDocumentChanged(const QString &);
	void projectTreeChanged(const KileProject *);

public Q_SLOTS:
	bool load();
	bool save();

	void add(KileProjectItem*);
	void remove(KileProjectItem*);

	void itemRenamed(KileProjectItem*);

	void buildProjectTree(); // moved to slots by tbraun

	//debugging
	void dump();

Q_SIGNALS:
	void loadFile(const KUrl &url , const QString & encoding);

private:
	void init(const QString& name, const KUrl& url, KileDocument::Extensions *extensions);
	QString	findRelativePath(const KUrl&);
	QString	findRelativePath(const QString&);

	void setType(KileProjectItem *item);
	QString addBaseURL(const QString &path);
	QString removeBaseURL(const QString &path);
	void writeConfigEntry(const QString &key,const QString &standardExt,KileProjectItem::Type type);

	KConfigGroup configGroupForItem(KileProjectItem *item) const;
	KConfigGroup configGroupForItemDocumentSettings(KileProjectItem *item) const;
	KConfigGroup configGroupForItemViewSettings(KileProjectItem *item, int viewIndex) const;

	void removeConfigGroupsForItem(KileProjectItem *item);

private:
	QString		m_name, m_quickBuildConfig, m_kileversion, m_kileprversion, m_defGraphicExt;
	KUrl		m_projecturl, m_baseurl, m_lastDocument;
	bool		m_invalid;
	QList<KileProjectItem*> m_rootItems;
	QList<KileProjectItem*>	m_projectItems;

	QString		m_extensions[3];
	QRegExp		m_reExtensions[3];

	QString		m_masterDocument, m_makeIndexOptions;
	bool			m_useMakeIndexOptions;

	KConfig	*m_config;
	KileDocument::Extensions *m_extmanager;
};

#endif
