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

	virtual const QStringList* getLabelList() const =0;
	virtual const QStringList* getBibItemList() const =0;

	KileDocumentInfo* getInfo() const {Kate::Document *doc = activeDocument(); if (doc) return m_mapDocInfo[doc]; else return 0;}

protected:
	QMap< Kate::Document*, KileDocumentInfo* >      m_mapDocInfo;
};

#endif
