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

#include <kdebug.h>
#include <klistview.h>
#include <kurl.h>
#include <kdialogbase.h>
#include <kconfig.h>

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
	enum  {
		None = 0x1, Label = 0x2, Sect = 0x4, Input = 0x8,
		BibItem = 0x10, Bibliography = 0x12, Package = 0x14
	};
}

/**
 * A convenience class to store info about how LaTeX elements should appear in the
 * structure view. A QMap<QString, KileStructData> should be created, so that the
 * actual LaTeX elements can be mapped to this class.
 **/
class KileStructData
{
public:
	KileStructData(int lvl = 0, int tp = KileStruct::None, QString px = QString::null)  : level(lvl), type(tp), pix(px) {}
	/** At which level the element should be visible **/
	int				level;
	/** The type of element (see @ref KileStruct) **/
	int 			type;
	/** The name of the icon that goes with this element. The icon is located using SmallIcon(pix). **/
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
	KileListViewItem(QListView * parent, QString label) : KListViewItem(parent,label), m_title(label), m_line(0),  m_column(0),m_type(KileStruct::None) {}
	KileListViewItem(QListViewItem * parent, QString label) : KListViewItem(parent,label), m_title(label), m_line(0),  m_column(0), m_type(KileStruct::None) {}

	/** @returns the title of this element (for a label it return the label), without the (line ...) part **/
	const QString& title() { return m_title; }
	/** @returns the line number of the structure element. **/
	const uint line() { return m_line; }
	/** @returns the column number of the structure element, right after the { **/
	const uint column() { return m_column; }
	/** @returns the type of element, see @ref KileStruct **/
	const int type() { return m_type; }

private:
	QString		m_title;
	uint			m_line;
	uint			m_column;
	int			m_type;
};

/**
 * KileDocumentInfo is a decorator class for the Document class. We can't derive a class from an interface
 * without implementing the interface, a decorator class is a way to add some functionality to the Document class.
 **/
class KileDocumentInfo : public QObject
{
	Q_OBJECT

public:
	KileDocumentInfo(Kate::Document *doc = 0L);
	~KileDocumentInfo() {kdDebug() << "DELETING DOCINFO" << m_url.path() << endl;}

	/**
	 * @returns the document for which this class is a decorator
	 **/
	Kate::Document* getDoc() { return m_doc; }
	void setDoc(Kate::Document *doc) { m_doc = doc; m_url=m_oldurl=doc->url();}
	void detach() { m_doc = 0L; }

	/**
	 * Used by @ref KileDocInfoDlg to display statistics of the Document.
	 * @returns an array with some statistical data of the document.
	 * The array is filled as follows: #chars in words, #chars in LaTeX commands,
	 * #chars in whitespace, #words, #commands.
	 **/
	const long* getStatistics();

	const QStringList* labels() const{ return &m_labels; }
	const QStringList* bibItems() const { return &m_bibItems; }
	const QStringList* dependencies() const {return &m_deps; }
	const QStringList* bibliographies() const { return &m_bibliography; }
	const QStringList* packages() const { return &m_packages; }

	KileListViewItem* structViewItem() { return m_struct; }
	void setListView(KListView *lv) { m_structview = lv;}

	bool isLaTeXRoot() { return m_bIsRoot; }

	void setURL(const KURL& url) { m_oldurl = m_url; m_url = url; emit nameChanged(url); }
	const KURL& url() {return m_url;}
	const KURL& oldURL() {return m_oldurl;}
	
	void cleanTempFiles(const QStringList &  );

public slots:
	/**
	 * Never call this function directly, use KileWidget::Structure::update(KileDocumentInfo *, bool) instead
	 **/
	void updateStruct();
	void updateBibItems();
	void emitNameChanged(Kate::Document *);
	void stopUpdate() { m_bContinueUpdate = false; }

signals:
	void nameChanged(const KURL &);
	void nameChanged(Kate::Document *);
	void isrootChanged(bool);

	void foundItem(const QString &title, uint line, uint m_column, int type, int level, const QString & pix);

private:
	void count(const QString line, long *stat);
	QString matchBracket(uint&, uint&);
	bool okToContinue() { return m_bContinueUpdate; }

protected:
	enum State
	{
    	stStandard=0, stComment=1, stControlSequence=3, stControlSymbol=4,
    	stCommand=5
	};

private:
	Kate::Document				*m_doc;
	long							*m_arStatistics;
	bool							m_bIsRoot;
	QStringList					m_labels;
	QStringList					m_bibItems;
	QStringList					m_deps;
	QStringList					m_bibliography;
	QStringList					m_packages;
	KListView						*m_structview;
	KileListViewItem				*m_struct;
	QMap<QString,KileStructData>	m_dictStructLevel;
	KURL						m_url, m_oldurl;
	bool							m_bContinueUpdate;
	KConfig						*m_config;
};

class KileDocInfoDlg : public KDialogBase
{
public:
	KileDocInfoDlg(KileDocumentInfo* docinfo, QWidget* parent = 0,  const char* name = 0, const QString &caption = QString::null);
	~KileDocInfoDlg() {}
};

#endif

