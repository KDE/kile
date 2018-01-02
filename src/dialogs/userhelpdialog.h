/*****************************************************************************************
                           userhelpdialog.h
----------------------------------------------------------------------------
    date                 : Jul 22 2005
    version              : 0.20
    copyright            : (C) 2005 by Holger Danielsson (holger.danielsson@t-online.de)
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

#ifndef USERHELPDIALOG_H
#define USERHELPDIALOG_H

#include <QList>
#include <QStringList>

#include <QDialog>
#include <QLineEdit>

class QListWidget;
class QPushButton;

namespace KileDialog
{

class UserHelpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserHelpDialog(QWidget *parent = Q_NULLPTR, const char *name = Q_NULLPTR);
    ~UserHelpDialog() {}

    void setParameter(const QStringList &menuentries, const QList<QUrl> &helpfiles);
    void getParameter(QStringList &userhelpmenulist, QList<QUrl> &userhelpfilelist);
private:
    QListWidget *m_menulistbox;
    QLineEdit *m_fileedit;
    QPushButton *m_add, *m_remove, *m_addsep, *m_up, *m_down;

    QList<QUrl> m_filelist;

    void updateButton();

private Q_SLOTS:
    void slotChange();
    void slotAdd();
    void slotRemove();
    void slotAddSep();
    void slotUp();
    void slotDown();
};

class UserHelpAddDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserHelpAddDialog(QListWidget *menulistbox, QWidget *parent = Q_NULLPTR);
    ~UserHelpAddDialog() {}

private:
    QLineEdit *m_leMenuEntry, *m_leHelpFile;
    QPushButton *m_pbChooseFile;
    QListWidget *m_menulistbox;

public:
    QString getMenuitem() {
        return m_leMenuEntry->text();
    }
    QString getHelpfile() {
        return m_leHelpFile->text();
    }

private Q_SLOTS:
    void onShowLocalFileSelection();
    void onAccepted();
};

}

#endif
