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

#include <qptrlist.h>
#include <qstring.h>
#include <qmap.h>

#include <kdebug.h>

class KURL;

class KileDocumentInfo;
class KileProject;
class KileProjectItem;
class KileProjectItemList;

namespace Kate { class Document;}

class KileInfo
{

public:
	KileInfo() {}
	virtual ~KileInfo() {}

public:
	QString getName(Kate::Document *doc = 0, bool shrt = false);
	QString getShortName(Kate::Document *doc = 0) { return getName(doc, true); }
	QString getCompileName(bool shrt = false);

	virtual Kate::Document* activeDocument() const = 0;

	virtual const QStringList* labels() =0;
	virtual const QStringList* bibItems() =0;
	virtual const QStringList* bibliographies() = 0;

	KileDocumentInfo* getInfo() const;
	KileDocumentInfo* infoFor(const QString &path) const;
	KileDocumentInfo* infoFor(Kate::Document* doc) const;

	bool	projectIsOpen(const KURL & );
	KileProject* projectFor(const KURL &projecturl);
	KileProject* projectFor(const QString & name);

	KileProject*	activeProject();
	KileProjectItem* activeProjectItem();
	KileProjectItem* itemFor(KileDocumentInfo *docinfo, KileProject *project = 0L) const;
	KileProjectItemList* itemsFor(KileDocumentInfo *docinfo) const;

	/**
	 * Finds the project item for the file with URL @param url.
	 * @returns a pointer to the project item, 0 if this file does not belong to a project
	 **/
	KileProjectItem* itemFor(const KURL &url, KileProject *project = 0L) const;
	Kate::Document* docFor(const KURL &url);


	void mapItem(KileDocumentInfo *docinfo, KileProjectItem *item);
	void trashDoc(KileDocumentInfo *docinfo);

protected:
	QPtrList<KileProject> m_projects;
	bool m_singlemode;
	QString m_masterName;

	QPtrList<Kate::Document> m_docList;
	QPtrList<KileDocumentInfo> m_infoList;
};

#endif
