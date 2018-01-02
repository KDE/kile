/*****************************************************************************************
    begin                : Fri Aug 15 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                           (C) 2007 by Holger Danielsson (holger.danielsson@versanet.de)
                           (C) 2011 by Libor Bukata (lbukata@gmail.com)
                           (C) 2013 by Michel Ludwig (michel.ludwig@kdemail.net)
 *****************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef LISTSELECTOR_H
#define LISTSELECTOR_H

#include <QItemSelection>
#include <QAbstractItemView>
#include <QSet>
#include <QDialog>

class QDialogButtonBox;
class QTreeWidget;
class QStringList;
class KDirWatch;

class KileListSelector : public QDialog
{
    Q_OBJECT

public:
    KileListSelector(const QStringList &list, const QString &caption, const QString &select, bool sort = true,
                     QWidget *parent = Q_NULLPTR, const char *name = Q_NULLPTR);

    bool hasSelection() const;
    void setSelectionMode(QAbstractItemView::SelectionMode mode);
    QStringList selectedItems() const;

protected Q_SLOTS:
    void handleSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

protected:
    QTreeWidget *m_listView;
    QDialogButtonBox *m_buttonBox;
    void insertStringList(const QStringList &list);
};


class ManageCompletionFilesDialog : public QDialog
{
    Q_OBJECT;

public:
    ManageCompletionFilesDialog(const QString &caption,
                                const QString &localCompletionDir, const QString &globalCompletionDir,
                                QWidget* parent = Q_NULLPTR, const char *name = Q_NULLPTR);
    ~ManageCompletionFilesDialog();

    const QSet<QString> selected() const;

protected Q_SLOTS:
    void addCustomCompletionFiles();
    void openLocalCompletionDirectoryInFileManager();
    void fillTreeView();

private:
    KDirWatch *m_dirWatcher;
    QTreeWidget *m_listView;
    QString m_localCompletionDirectory, m_globalCompletionDirectory;
};

#endif
