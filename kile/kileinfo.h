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

class KileDocumentInfo;
class KileProject;
class KileProjectItem;

namespace Kate { class Document;}

class KileInfo
{

public:
	KileInfo() {}
	virtual ~KileInfo() {}

public:
	QString getName(Kate::Document *doc = 0, bool shrt = false);
	QString getShortName(Kate::Document *doc = 0) { return getName(doc, true); }

	virtual Kate::Document* activeDocument() const = 0;

	virtual const QStringList* labels() const =0;
	virtual const QStringList* bibItems() const =0;

	KileDocumentInfo* getInfo() const {Kate::Document *doc = activeDocument(); if (doc) return m_mapDocInfo[doc]; else return 0;}
	KileDocumentInfo* getInfo(Kate::Document* doc) const { return m_mapDocInfo[doc];}

	KileProject*	activeProject();
	KileProjectItem* activeProjectItem();
	KileProjectItem* itemFor(Kate::Document *doc) { return m_mapDocToItem[doc]; }
	Kate::Document* docFor(KileProjectItem *item) { return m_mapItemToDoc[item]; }

	void mapItem(Kate::Document *doc, KileProjectItem *item) { m_mapDocToItem[doc]=item; m_mapItemToDoc[item]=doc;}
	void removeMap(Kate::Document *doc, KileProjectItem *item) { m_mapDocToItem.remove(doc); m_mapItemToDoc.remove(item); }

protected:
	QMap< Kate::Document*, KileDocumentInfo* >      m_mapDocInfo;
	QPtrList<KileProject>		m_projects;
	QMap<Kate::Document*, KileProjectItem* >	m_mapDocToItem;
	QMap<KileProjectItem*, Kate::Document* >	m_mapItemToDoc;
};

#endif
