/***************************************************************************
                         texdocdialog.h
                         --------------
    date                 : Feb 15 2007
    version              : 0.14
    copyright            : (C) 2005-2007 by Holger Danielsson
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

#ifndef TEXDOCDIALOG_H
#define TEXDOCDIALOG_H

#include <KDialog>

#include <QMap>

class QTreeWidget;
class QTreeWidgetItem;

class K3Process;
class K3ShellProcess;
class KLineEdit;
class KPushButton;
class KTemporaryFile;

namespace KileDialog
{

class TexDocDialog : public KDialog
{
		Q_OBJECT

	public:
		TexDocDialog(QWidget *parent = 0, const char *name = 0);
		~TexDocDialog();

	private:
		QTreeWidget *m_texdocs;
		KLineEdit *m_leKeywords;
		KPushButton *m_pbSearch;

		QString m_texmfPath, m_texmfdocPath, m_texdoctkPath;

		QStringList m_tocList, m_tocSearchList;
		QMap<QString, QString> m_dictDocuments;
		QMap<QString, QString> m_dictStyleCodes;

		void readToc();
		void showToc(const QString &caption, const QStringList &doclist, bool toc = true);

		QString m_filename;
		QString m_output;

		KTemporaryFile *m_tempfile;
		K3ShellProcess *m_proc;

		void callSearch();
		void executeScript(const QString &command);
		void showFile(const QString &filename);

		QString searchFile(const QString &docfilename, const QString &listofpathes,
		                   const QString &subdir = QString::null);
		void decompressFile(const QString &docfile, const QString &command);
		void showStyleFile(const QString &filename, const QString &stylecode);

		QString getMimeType(const QString &filename);
		QString getIconName(const QString &filename);

	protected:
		bool eventFilter(QObject *o, QEvent *e);

	Q_SIGNALS:
		void processFinished();

	protected Q_SLOTS:
		void slotDefault();

	private Q_SLOTS:
		void slotListViewDoubleClicked(QTreeWidgetItem *item);
		void slotTextChanged(const QString &text);
		void slotSearchClicked();

		void slotProcessOutput(K3Process*, char* buf, int len);
		void slotProcessExited(K3Process *proc);

		void slotInitToc();
		void slotShowFile();
};

}

#endif
