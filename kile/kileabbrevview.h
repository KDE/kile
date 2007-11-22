/***************************************************************************
    begin                : Feb 24 2007
    copyright            : 2007 by Holger Danielsson
    email                : holger.danielsson@versanet.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KILEABBREVVIEW_H
#define KILEABBREVVIEW_H

#include <qstring.h>
#include <qlabel.h>
#include <q3popupmenu.h>

#include <klineedit.h>
#include <klistbox.h>
#include <klistview.h>
#include <kdialogbase.h>

//////////////////// KlistView for abbreviations ////////////////////

class KileAbbrevView : public KListView  
{
  Q_OBJECT

public:
	enum { ALVabbrev=0, ALVlocal=1, ALVexpansion=2 };
	enum { ALVnone=0, ALVadd=1, ALVedit=2, ALVdelete=3 };

	KileAbbrevView(QWidget *parent=0, const char *name=0);
	~KileAbbrevView();

	void init(const QStringList *globallist, const QStringList *locallist);
	bool findAbbreviation(const QString &abbrev);
	void saveLocalAbbreviation(const QString &filename);

signals:
	void updateAbbrevList(const QString &ds, const QString &as);
	void sendText(const QString &text);
 
private slots:
	void slotMouseButtonClicked(int button, Q3ListViewItem *item, const QPoint &pos, int);
	void slotContextMenu(KListView *, Q3ListViewItem *item, const QPoint &pos);
	void slotPopupAbbreviation(int id);

private:
	Q3PopupMenu* m_popup;
	bool m_changes;

	void addAbbreviation(const QString &abbrev, const QString &expansion);
	void changeAbbreviation(KListViewItem *item,const QString &abbrev, const QString &expansion);
	void deleteAbbreviation(KListViewItem *item);

	void addWordlist(const QStringList *wordlist, bool global);

};

//////////////////// add/edit dialog for abbreviations ////////////////////

class KileAbbrevInputDialog : public KDialogBase  
{
   Q_OBJECT

public: 
	KileAbbrevInputDialog(KileAbbrevView *listview, KListViewItem *item, int mode, const char *name=0);
	~KileAbbrevInputDialog();

	void abbreviation(QString &abbrev, QString &expansion);

private:
	KileAbbrevView *m_listview;
	KListViewItem *m_abbrevItem;
	KLineEdit *m_leAbbrev;
	KLineEdit *m_leExpansion;

	int m_mode;
	QString m_abbrev, m_expansion;

public slots:
	void slotOk();

private slots:
	void slotTextChanged(const QString &text);

};

#endif
