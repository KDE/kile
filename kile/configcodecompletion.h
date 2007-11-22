/***************************************************************************
    date                 : Mar 30 2007
    version              : 0.24
    copyright            : (C) 2004-2007 by Holger Danielsson
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

#ifndef CONFIGCODECOMPLETION_H
#define CONFIGCODECOMPLETION_H

#include <qwidget.h>
#include <qtabwidget.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qstring.h>

#include <kdeversion.h>
#include <k3listview.h>
#include <kpushbutton.h>
#include <kconfig.h>

/**
  *@author Holger Danielsson
  */

namespace KileWidget { class LogMsg; }

class ConfigCodeCompletion : public QWidget
{
    Q_OBJECT
public: 
   ConfigCodeCompletion(KConfig *config, KileWidget::LogMsg *logwidget, QWidget *parent=0, const char *name=0);
   ~ConfigCodeCompletion();

   void readConfig(void);
   void writeConfig(void);

private:
	enum CompletionPage { TexPage=0, DictionaryPage=1, AbbreviationPage=2, NumPages=3 };

	KConfig *m_config;
	KileWidget::LogMsg *m_logwidget;
 
	// tabs, views, pages, wordlists
	QTabWidget *tab;
	K3ListView *m_listview[NumPages];
	QWidget *m_page[NumPages];
	QStringList m_wordlist[NumPages];
	QStringList m_dirname;

	// button
	KPushButton *add,*remove;

    // Checkboxes/Spinboxes
    QCheckBox *cb_usecomplete, *cb_autocomplete;
    QCheckBox *cb_setcursor, *cb_setbullets;
    QCheckBox *cb_closeenv;
    QSpinBox *sp_latexthreshold;
    QLabel *lb_latexthreshold;
	QCheckBox *cb_autocompletetext;
	QSpinBox *sp_textthreshold;
	QLabel *lb_textthreshold;
	QCheckBox *cb_autocompleteabbrev;
	QCheckBox *cb_showabbrevview;
	QCheckBox *cb_citeoutofbraces;

	bool kateCompletionPlugin();

	K3ListView *getListview(QWidget *page);
	QString getListname(QWidget *page);
	void addPage(QTabWidget *tab, CompletionPage page, const QString &title, const QString &dirname);

	void setListviewEntries(CompletionPage page);
	bool getListviewEntries(CompletionPage page);
	bool isListviewEntry(K3ListView *listview, const QString &filename);
	void updateColumnWidth(K3ListView *listview);

	QString m_localCwlDir, m_globalCwlDir;
	void getCwlFiles(QMap<QString,QString> &map, QStringList &list, const QString &dir);
	void getCwlDirs();

private slots:
   void showPage(QWidget *page);
   void addClicked();
   void removeClicked();
	void slotListviewClicked(Q3ListViewItem *);
};

#endif
