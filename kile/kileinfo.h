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
#include <qmap.h>

#include <kdebug.h>

class KURL;

class KileDocumentInfo;
class KileProject;
class KileProjectItem;

namespace Kate { class Document;}

class KileInfo
{

public:
	KileInfo() : m_currentTarget(QString::null) { }
	virtual ~KileInfo() {}

public:
	QString getName(Kate::Document *doc = 0, bool shrt = false);
	QString getShortName(Kate::Document *doc = 0) { return getName(doc, true); }
	QString getCompileName(bool shrt = false);

	QString getCurrentTarget() const { return m_currentTarget; }
	void setTarget(const QString &target) { m_currentTarget=target; }

	virtual Kate::Document* activeDocument() const = 0;

	QString getSelection() const;
	void clearSelection() const;

	virtual const QStringList* labels(KileDocumentInfo * info = 0) =0;
	virtual const QStringList* bibItems(KileDocumentInfo * info = 0) =0;
	virtual const QStringList* bibliographies(KileDocumentInfo * info = 0) = 0;

	KileDocumentInfo* getInfo() const {Kate::Document *doc = activeDocument(); if (m_mapDocInfo.contains(doc)) return m_mapDocInfo[doc]; else return 0;}
	KileDocumentInfo* infoFor(const QString &path);
	KileDocumentInfo* infoFor(Kate::Document* doc) const { if (m_mapDocInfo.contains(doc) > 0) return m_mapDocInfo[doc]; else return 0;}

	bool	projectIsOpen(const KURL & );
	KileProject* projectFor(const KURL &projecturl);
	KileProject* projectFor(const QString & name);

	KileProject*	activeProject();
	KileProjectItem* activeProjectItem();
	KileProjectItem* itemFor(KileDocumentInfo *docinfo) const { if (m_mapDocInfoToItem.contains(docinfo) > 0) return m_mapDocInfoToItem[docinfo];  else return 0;}
	/**
	 * Finds the project item for the file with URL @param url.
	 * @returns a pointer to the project item, 0 if this file does not belong to a project
	 **/
	KileProjectItem* itemFor(const KURL &url);
	KileDocumentInfo* infoFor(KileProjectItem *item) { if (m_mapItemToDocInfo.contains(item))  return m_mapItemToDocInfo[item];  else return 0;}
	Kate::Document* docFor(const KURL &url);

	void mapInfo(Kate::Document *doc, KileDocumentInfo *info) { m_mapDocInfo[doc] = info; }
	void mapItem(KileDocumentInfo *docinfo, KileProjectItem *item);
	void removeMap(KileDocumentInfo *docinfo, KileProjectItem *item) { m_mapDocInfoToItem.remove(docinfo); m_mapItemToDocInfo.remove(item); }
	void removeMap(Kate::Document *doc) { m_mapDocInfo.remove(doc); }

	void trash(Kate::Document* doc);

	bool checkForRoot() { return m_bCheckForRoot; }
	bool watchFile() { return m_bWatchFile; }
	
	virtual int lineNumber() = 0;
	
	QString relativePath(const QString basepath, const QString & file);

protected:
	QMap< Kate::Document*, KileDocumentInfo* >      m_mapDocInfo;
	QPtrList<KileProject>		m_projects;
	QMap<KileDocumentInfo*, KileProjectItem* >	m_mapDocInfoToItem;
	QMap<KileProjectItem*, KileDocumentInfo* >	m_mapItemToDocInfo;

	bool 			m_singlemode;
	QString	m_masterName;

	QPtrList<Kate::Document> 		m_docList;
	QPtrList<KileDocumentInfo>	m_infoList;

	QString			m_currentTarget;
	
	bool m_bCheckForRoot, m_bWatchFile;
};

#endif
