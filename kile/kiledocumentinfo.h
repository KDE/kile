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

#include <klistview.h>
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

namespace KileStruct
{
	enum  { None = 0x1, Label = 0x2, Sect = 0x4, Input =0x8};
}

class KileStructData
{
public:
	KileStructData(int lvl = 0, int tp = KileStruct::None, QString px = QString::null)  : level(lvl), type(tp), pix(px) {}
	int				level;
	int 			type;
	QString 	pix;
};

/**
 * ListView items that can hold some additional information appropriate for the Structure View. The
 * additional information is: line number, title string.
 **/
class KileListViewItem : public KListViewItem
{
public:
	KileListViewItem(QListViewItem * parent, QListViewItem * after, QString title, uint line, uint m_column, int type);
	KileListViewItem(QListView * parent, QString label) : KListViewItem(parent,label) { m_line=0; m_column=0; m_title=label; m_type = KileStruct::None;}
	KileListViewItem(QListViewItem * parent, QString label) : KListViewItem(parent,label) { m_line=0; m_column=0; m_title=label; m_type = KileStruct::None; }

	const QString& title() { return m_title; }
	const uint line() { return m_line; }
	const uint column() { return m_column; }
	const int type() { return m_type; }

private:
	QString		m_title;
	uint 				m_line;
	uint				m_column;
	int					m_type;
};

class KileDocumentInfo : public QObject
{
	Q_OBJECT

public:
	KileDocumentInfo(Kate::Document *doc);

	Kate::Document* getDoc() { return m_doc; }

	const long* getStatistics();

	const QStringList* getLabelList() const{ return &m_labels; }
	const QStringList* getBibItemList() const { return &m_bibItems; }

	KileListViewItem* structViewItem() { return m_struct; }
	void setListView(KListView *lv) { m_structview = lv;}

public slots:
	void updateStruct();
	void updateBibItems();

private:
	int		matchBracket(const QString&, int);

protected:
	enum State
	{
    	stStandard=0, stComment=1, stControlSequence=3, stControlSymbol=4,
    	stCommand=5
	};

private:
	Kate::Document	*m_doc;
	long						*m_arStatistics;
	QStringList			m_labels;
	QStringList			m_bibItems;
	KListView				*m_structview;
	KileListViewItem	*m_struct;
	QMap<QString,KileStructData>		m_dictStructLevel;
};

class KileDocInfoDlg : public KDialogBase
{
public:
	KileDocInfoDlg(KileDocumentInfo* docinfo, QWidget* parent = 0,  const char* name = 0, const QString &caption = QString::null);
	~KileDocInfoDlg() {}
};

#endif

