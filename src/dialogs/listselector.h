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
#include <QSet>

#include <KDialog>

class QTreeWidget;
class QStringList;
class KDirWatch;

class KileListSelectorBase : public KDialog
{
	Q_OBJECT

	public:
		KileListSelectorBase(const QStringList &list, const QString &caption, const QString &select, bool sort = true,
		                     QWidget *parent = NULL, const char *name = NULL);

		bool hasSelection();

	protected Q_SLOTS:
		void handleSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

	protected:
		QTreeWidget *m_listView;
		void insertStringList(const QStringList &list);
};

class KileListSelector : public KileListSelectorBase
{
	public:
		KileListSelector(const QStringList &list, const QString &caption, const QString &select, bool sort = true,
		                 QWidget *parent = NULL, const char *name = NULL);

		QString selected();
};

class KileListSelectorMultiple : public KileListSelectorBase
{
	public:
		KileListSelectorMultiple(const QStringList & list, const QString &caption, const QString &select, bool sort = true,
		                         QWidget *parent = NULL, const char *name = NULL);

	QStringList selected();
};

class ManageCompletionFilesDialog : public KDialog
{
	Q_OBJECT;

	public:
		ManageCompletionFilesDialog(const QString &caption,
		                            const QString &localCompletionDir, const QString &globalCompletionDir,
		                            QWidget* parent = NULL, const char *name = NULL);
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
