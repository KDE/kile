/***************************************************************************
                          kiledocumentinfo.h -  description
                             -------------------
    begin                : Sun Jul 20 2003
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

#ifndef KILEDOCUMENTINFO_H
#define KILEDOCUMENTINFO_H

#include <kate/document.h>

#include <kdialogbase.h>

#define TEX_CAT0 '\\'
#define TEX_CAT1 '{'
#define TEX_CAT2 '}'
#define TEX_CAT3 '$'
#define TEX_CAT4 '&'
#define TEX_CAT6 '#'
#define TEX_CAT7 '^'
#define TEX_CAT8 '_'
#define TEX_CAT13 '~'
#define TEX_CAT14 '%'

class KileDocumentInfo : public QObject
{
	Q_OBJECT

public:
	KileDocumentInfo(Kate::Document *doc);

	Kate::Document* getDoc() { return m_doc; }

	const long* getStatistics();

protected:
	enum State
	{
    	stStandard=0, stComment=1, stControlSequence=3, stControlSymbol=4,
    	stCommand=5
	};

private:
	Kate::Document	*m_doc;
	long*			m_arStatistics;
};

class KileDocInfoDlg : public KDialogBase
{
public:
	KileDocInfoDlg(KileDocumentInfo* docinfo, QWidget* parent = 0,  const char* name = 0, const QString &caption = QString::null);
	~KileDocInfoDlg() {}
};

#endif

