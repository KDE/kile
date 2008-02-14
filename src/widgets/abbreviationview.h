/****************************************************************************************
    begin                : Feb 24 2007
    copyright            : 2007 by Holger Danielsson (holger.danielsson@versanet.de)
                           2008 by Michel Ludwig (michel.ludwig@kdemail.net)
 ****************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ABBREVIATIONVIEW_H
#define ABBREVIATIONVIEW_H

#include <QLabel>
#include <QString>
#include <QTreeWidget>

//////////////////// KlistView for abbreviations ////////////////////

namespace KileWidget {

class AbbreviationView : public QTreeWidget
{
  Q_OBJECT

public:
	enum {ALVabbrev = 0, ALVlocal = 1, ALVexpansion = 2};
	enum {ALVnone = 0, ALVadd = 1, ALVedit = 2, ALVdelete = 3};

	AbbreviationView(QWidget *parent = NULL, const char *name = NULL);
	~AbbreviationView();

	void init(const QStringList *globallist, const QStringList *locallist);
	bool findAbbreviation(const QString &abbrev);
	void saveLocalAbbreviation(const QString &filename);

Q_SIGNALS:
	void updateAbbrevList(const QString &ds, const QString &as);
	void sendText(const QString &text);
 
private Q_SLOTS:
	void slotItemClicked(QTreeWidgetItem *item, int column);
	void slotCustomContextMenuRequested(const QPoint& p);
	void slotAddAbbreviation();
	void slotChangeAbbreviation();
	void slotDeleteAbbreviation();

private:
	bool m_changes;

	void addAbbreviation(const QString &abbrev, const QString &expansion);
	void changeAbbreviation(QTreeWidgetItem *item, const QString &abbrev, const QString &expansion);
	void deleteAbbreviation(QTreeWidgetItem *item);

	void addWordlist(const QStringList *wordlist, bool global);

};

}

#endif
