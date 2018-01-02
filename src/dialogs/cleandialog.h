/***************************************************************************
    date                 : Jan 22 2004
    version              : 0.10
    copyright            : (C) 2004 by Holger Danielsson
    email                : holger.danielsson@t-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CLEANDIALOG_H
#define CLEANDIALOG_H

#include <QDialog>

#include <QStringList>

class QTreeWidget;

/**
 * @author Holger Danielsson
 */

namespace KileDialog
{
class Clean : public QDialog
{
    Q_OBJECT

public:
    Clean(QWidget *parent, const QString &filename, const QStringList &extlist);
    ~Clean();
    QStringList cleanList();

private:
    QTreeWidget *m_listview;
    QStringList m_extlist;
};
}

#endif
