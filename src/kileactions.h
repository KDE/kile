/*************************************************************************************
    begin                :  2003-07-01 17:33:00 CEST 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
 *************************************************************************************/

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

#include <KAction>
#include <KActionCollection>
#include <KSelectAction>
#include <KDialog>
#include <KLineEdit>

class QCheckBox;
class QLineEdit;

class KileInfo;

namespace KileAction
{

enum { KeepHistory=1, ShowAlternative=2, ShowBrowseButton=4, FromLabelList=8, FromBibItemList=16, ShowLabel=32, AddProjectFile=64};
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

Q_SIGNALS:
	//sends along tagdata so that receiver knows what to insert
	void triggered(const KileAction::TagData&);

private Q_SLOTS:
	//emits the triggered(TagData) signal
	virtual void emitData();

protected:
	TagData m_data;
};

/*
	InputTag: adds a history list and options for a input dialog to TagData
*/
class InputTag : public Tag
{
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

private Q_SLOTS:
	//emits the triggered(TagData) signal
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
	InputDialog
*/
class InputDialog : public KDialog
{
	Q_OBJECT

public:
	InputDialog(const QString &caption, uint options, const QStringList& history, const QString &hint, const QString &alter, KileInfo *ki, QWidget *parent=0, const char *name=0);
	~InputDialog();

	bool useAlternative() {return m_useAlternative;}
	bool useLabel() {return m_useLabel;}
	bool useAddProjectFile() {return m_useAddProjectFile;}

public Q_SLOTS:
	void slotBrowse();
	void slotAltClicked();

	void setTag(const QString&);

Q_SIGNALS:
	void setInput(const QString&);

public:
	QString tag() { return m_tag; }
	QString label();
	bool usedSelection() { return m_usedSelection; }
	
	KLineEdit *m_edLabel;

private:
	QString	m_tag;
	QString	m_labelprefix;
	bool		m_useAlternative,m_useLabel,m_usedSelection,m_useAddProjectFile;
	KileInfo	*m_ki;
};

class Select : public KSelectAction
{
	Q_OBJECT

public:
	//constructors
	Select(const QString &text, const KShortcut &cut, KActionCollection *parent, const char *name);

public Q_SLOTS:
	void setItems(const QList<KAction*> &);

};

}

#endif
