/***************************************************************************
    begin                :  2003-07-01 17:33:00 CEST 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
    email                :  Jeroen.Wijnhout@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KILEACTIONS_H
#define KILEACTIONS_H

#include <qdict.h>

#include <kaction.h>
#include <kdialogbase.h>


class QCheckBox;
class QLineEdit;

class KileInfo;

namespace KileAction
{

enum { KeepHistory=1, ShowAlternative=2, ShowBrowseButton=4, FromLabelList=8, FromBibItemList = 16, ShowFigureInput=32};
/*
	TagData
*/

class TagData
{
public:
	TagData(const QString &t, const QString &tB = QString::null, const QString &tE = QString::null, int x = 0, int y = 0, const QString &desc = QString::null)
		: text(t), tagBegin(tB), tagEnd(tE), dx(x), dy(y), description(desc) {}

	TagData() : text(QString::null), tagBegin(QString::null), tagEnd(QString::null), dx(0), dy(0), description(QString::null) {}

	QString		text;
	QString		tagBegin, tagEnd;
	int			dx,dy;
	QString		description;
};

class Tag : public KAction
{
	Q_OBJECT

public:
	//constructors
	Tag(const QString &text, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name
			, const QString &tagBegin, const QString &tagEnd = QString::null, int dx=0, int dy=0, const QString &description = QString::null);

	Tag(const QString &text, const QString& pix, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name
			, const QString &tagBegin, const QString &tagEnd = QString::null, int dx=0, int dy=0, const QString &description = QString::null);

	Tag(const QString &text, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name
			, const TagData& data);

	Tag(const QString &text, const QString& pix, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name
			, const TagData& data);

	~Tag();

private:
	void init(const QObject *receiver = 0, const char *slot = 0);

signals:
	//sends along tagdata so that receiver knows what to insert
	void activated(const KileAction::TagData&);

private slots:
	//emits the activated(TagData) signal
	virtual void emitData();

protected:
	TagData m_data;
};

/*
	InputTag: adds a history list and options for a input dialog to TagData
*/
class InputTag : public Tag
{
friend class InputFigure;

	Q_OBJECT

public:
	//constructors
	InputTag(KileInfo* ki, const QString &text, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name, QWidget *wparent,uint options
			, const QString &tagBegin, const QString &tagEnd = QString::null, int dx=0, int dy=0, const QString &description = QString::null, const QString &hint = QString::null, const QString &alter = QString::null);

	InputTag(KileInfo* ki, const QString &text, const QString& pix, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name, QWidget *wparent,uint options
			, const QString &tagBegin, const QString &tagEnd = QString::null, int dx=0, int dy=0, const QString &description = QString::null, const QString &hint = QString::null, const QString &alter = QString::null);

	InputTag(KileInfo* ki, const QString &text, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name, QWidget *wparent,uint options
			, const TagData& data, const QString &hint = QString::null, const QString &alter = QString::null);

	InputTag(KileInfo* ki, const QString &text, const QString& pix, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name, QWidget *wparent,uint options
			, const TagData& data, const QString &hint = QString::null, const QString &alter = QString::null);

	~InputTag();

	bool hasHistory() {return (m_options & KeepHistory); }
	bool hasAlternative() { return (m_options & ShowAlternative); }
	bool hasBrowseButton() { return (m_options & ShowBrowseButton); }

	void addToHistory(const QString& str) { if ( m_history.first() != str ) m_history.prepend(str); }

private:
	void init();

private slots:
	//emits the activated(TagData) signal
	virtual void emitData();

private:
	KileInfo	*m_ki;
	QStringList			m_history;
	QWidget				*m_parent;
	uint				m_options;
	QString				m_hint;
	QString				m_alter;
};


/*
	InputFigure
*/
class InputFigure : public InputTag
{
	Q_OBJECT

public:
	//constructors
	InputFigure(KileInfo* ki, const QString &text, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name, QWidget *wparent,uint options
			, const QString &tagBegin, const QString &tagEnd = QString::null, int dx=0, int dy=0, const QString &description = QString::null, const QString &hint = QString::null, const QString &alter = QString::null);

private slots:
	//emits the activated(TagData) signal
	virtual void emitData();

};


/*
	InputDialog
*/
class InputDialog : public KDialogBase
{
	Q_OBJECT

public:
	InputDialog(const QString &caption, uint options, const QStringList& history, const QString &hint, const QString &alter, KileInfo *ki, QWidget *parent=0, const char *name=0);
	~InputDialog();

	bool useAlternative() {return m_useAlternative;}

public slots:
	void slotBrowse();
	void slotAltClicked();
	void slotEnvClicked();

	void setTag(const QString&);

signals:
	void setInput(const QString&);

public:
	QString tag() { return m_tag; }
	bool usedSelection() { return m_usedSelection; }
	
	QLineEdit *figLabel;
	QLineEdit *figCaption;
	QCheckBox *Env;

private:
	QString 			m_tag;
	bool				m_useAlternative, m_usedSelection;
	KileInfo	*m_ki;
	QLabel *Text2;
	QLabel *Text3;
};

class Select : public KSelectAction
{
	Q_OBJECT

public:
	//constructors
	Select(const QString &text, const KShortcut &cut, KActionCollection *parent, const char *name);

private:
	void init();

signals:
	void activated(const KAction&);

public slots:
	void setItems(QPtrList<KAction> &);

private slots:
	void emitData(const QString&);

private:
	QDict<KAction> m_dict;
};

}

#endif

