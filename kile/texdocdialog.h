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

#include <kdialogbase.h>
#include <klineedit.h>
#include <kpushbutton.h>
#include <kprocess.h>
#include <ktempfile.h>
#include <klistview.h>

#include <qlabel.h>
#include <qstringlist.h>
#include <qmap.h>

namespace KileDialog
{

class TexDocDialog : public KDialogBase  
{
	Q_OBJECT

public:
	TexDocDialog(QWidget *parent=0, const char *name=0);
	~TexDocDialog();

private:	
	KListView *m_texdocs;
	KLineEdit *m_leKeywords;
	KPushButton *m_pbSearch;

	QString m_texmfPath, m_texmfdocPath, m_texdoctkPath;
	
	QStringList m_tocList,m_tocSearchList;
	QMap<QString,QString> m_dictDocuments;
	QMap<QString,QString> m_dictStyleCodes;
	
	void readToc();
	void showToc(const QString &caption,const QStringList &doclist, bool toc=true);

	QString m_filename;
	QString m_output;
	
	KTempFile *m_tempfile;
	KShellProcess *m_proc;
	
	void callSearch();
	void executeScript(const QString &command);
	void showFile(const QString &filename);

	QString searchFile(const QString &docfilename,const QString &listofpathes, 
	                   const QString &subdir=QString::null);
	void decompressFile(const QString &docfile,const QString &command);
	void showStyleFile(const QString &filename,const QString &stylecode);
	
	QString getMimeType(const QString &filename);
	QString getIconName(const QString &filename);

protected:
	bool eventFilter(QObject *o, QEvent *e); 

signals:
	void processFinished();

protected slots:
	void slotHelp(); 
	
private slots:
	void slotListViewDoubleClicked(QListViewItem *item,const QPoint &,int); 
	void slotTextChanged(const QString &text);
	void slotSearchClicked();
	
	void slotProcessOutput(KProcess*,char* buf,int len);
	void slotProcessExited (KProcess *proc);

	void slotInitToc();
	void slotShowFile();
};

}

#endif
