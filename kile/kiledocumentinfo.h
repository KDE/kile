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
		BibItem = 0x10, Bibliography = 0x20, Package = 0x40, NewCommand = 0x80
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
 * KileDocument::Info is a decorator class for the Document class. We can't derive a class from an interface
 * without implementing the interface, a decorator class is a way to add some functionality to the Document class.
 **/
 
namespace KileDocument
{

struct BracketResult 
{
	BracketResult() : option(QString::null), value(QString::null) {}
	QString option, value;
};

class Info : public QObject
{
	Q_OBJECT

public:
	static bool isTeXFile(const KURL &);
	static bool isBibFile(const KURL &);

public:
	Info(Kate::Document *doc = 0L);
	~Info() {kdDebug() << "DELETING DOCINFO" << m_url.path() << endl;}

	/**
	 * @returns the document for which this class is a decorator
	 **/
	Kate::Document* getDoc() const { return m_doc; }
	void setDoc(Kate::Document *doc) { m_doc = doc; m_url=m_oldurl=doc->url();}
	void detach() { m_doc = 0L; }

	/**
	 * Used by @ref KileDocInfoDlg to display statistics of the Document.
	 * @returns an array with some statistical data of the document.
	 * The array is filled as follows: #chars in words, #chars in LaTeX commands,
	 * #chars in whitespace, #words, #commands.
	 **/
	virtual const long* getStatistics();

	const QStringList* labels() const{ return &m_labels; }
	const QStringList* bibItems() const { return &m_bibItems; }
	const QStringList* dependencies() const {return &m_deps; }
	const QStringList* bibliographies() const { return &m_bibliography; }
	const QStringList* packages() const { return &m_packages; }
	const QStringList* newCommands() const { return &m_newCommands; }

	QString lastModifiedFile(const QStringList *list = 0L);

	const QString & preamble() const { return m_preamble; }

	virtual bool isLaTeXRoot() { return m_bIsRoot; }

	void setURL(const KURL& url) { m_oldurl = m_url; m_url = url; emit nameChanged(url); }
	const KURL& url() {return m_url;}
	const KURL& oldURL() {return m_oldurl;}
	
	void cleanTempFiles(const QStringList &  );

public slots:
	/**
	 * Never call this function directly, use KileWidget::Structure::update(KileDocument::Info *, bool) instead
	 **/
	virtual void updateStruct();
	virtual void updateBibItems();
	void emitNameChanged(Kate::Document *);

signals:
	void nameChanged(const KURL &);
	void nameChanged(Kate::Document *);
	void isrootChanged(bool);

	void foundItem(const QString &title, uint line, uint column, int type, int level, const QString & pix, const QString & folder);

protected:
	void count(const QString line, long *stat);
	QString matchBracket(QChar c, uint &, uint &);

protected:
	enum State
	{
		stStandard=0, stComment=1, stControlSequence=3, stControlSymbol=4,
	 	stCommand=5
	};

protected:
	Kate::Document				*m_doc;
	long							*m_arStatistics;
	bool							m_bIsRoot;
	QStringList					m_labels;
	QStringList					m_bibItems;
	QStringList					m_deps;
	QStringList					m_bibliography;
	QStringList					m_packages;
	QStringList					m_newCommands;
	QString						m_preamble;
	QMap<QString,KileStructData>	m_dictStructLevel;
	KURL						m_url, m_oldurl;
	KConfig						*m_config;
};

class TeXInfo : public Info
{
	Q_OBJECT

public:
	TeXInfo ( Kate::Document * doc ) : Info(doc) {}

public:
	const long* getStatistics();

public slots:
	void updateStruct();

private:
	BracketResult matchBracket(uint &, uint &);
};

class BibInfo : public Info
{
	Q_OBJECT

public:
	BibInfo ( Kate::Document * doc ) : Info(doc) {}
	bool isLaTeXRoot() { return false; }

public slots:
	void updateStruct();
};

}

class KileDocInfoDlg : public KDialogBase
{
public:
	KileDocInfoDlg(KileDocument::Info* docinfo, QWidget* parent = 0,  const char* name = 0, const QString &caption = QString::null);
	~KileDocInfoDlg() {}
};

#endif

