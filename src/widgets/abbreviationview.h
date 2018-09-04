/****************************************************************************************
    begin                : Feb 24 2007
    copyright            : 2007 by Holger Danielsson (holger.danielsson@versanet.de)
                           2008 - 2009 by Michel Ludwig (michel.ludwig@kdemail.net)
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

namespace KileAbbreviation {
class Manager;
}

namespace KileWidget {

class AbbreviationView : public QTreeWidget
{
    Q_OBJECT

public:
    enum {ALVabbrev = 0, ALVlocal = 1, ALVexpansion = 2};
    enum {ALVnone = 0, ALVadd = 1, ALVedit = 2, ALVdelete = 3};

    explicit AbbreviationView(KileAbbreviation::Manager *manager, QWidget *parent = Q_NULLPTR);
    ~AbbreviationView();

    bool findAbbreviation(const QString &abbrev);

Q_SIGNALS:
    void sendText(const QString &text);

public Q_SLOTS:
    void updateAbbreviations();

private Q_SLOTS:
    void slotItemClicked(QTreeWidgetItem *item, int column);
    void slotCustomContextMenuRequested(const QPoint& p);
    void slotAddAbbreviation();
    void slotChangeAbbreviation();
    void slotDeleteAbbreviation();

private:
    KileAbbreviation::Manager *m_abbreviationManager;

};

}

#endif
