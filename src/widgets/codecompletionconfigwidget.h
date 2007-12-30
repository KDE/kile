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

#include <QWidget>

class QTabWidget;
class QCheckBox;
class QLabel;
class QSpinBox;
class QTreeWidget;
class QTreeWidgetItem;

class KConfig;
class KPushButton;

/**
  *@author Holger Danielsson
  */

namespace KileWidget {
class LogMsg;
}

class CodeCompletionConfigWidget : public QWidget
{
		Q_OBJECT
	public:
		CodeCompletionConfigWidget(KConfig *config, KileWidget::LogMsg *logwidget, QWidget *parent = 0, const char *name = 0);
		~CodeCompletionConfigWidget();

		void readConfig(void);
		void writeConfig(void);

	private:
		enum CompletionPage { TexPage = 0, DictionaryPage = 1, AbbreviationPage = 2, NumPages = 3 };

		KConfig *m_config;
		KileWidget::LogMsg *m_logwidget;

		// tabs, views, pages, wordlists
		QTabWidget *tab;
		QTreeWidget *m_listview[NumPages];
		QWidget *m_page[NumPages];
		QStringList m_wordlist[NumPages];
		QStringList m_dirname;

		// button
		KPushButton *add, *remove;

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

		QTreeWidget *getListview(QWidget *page);
		QString getListname(QWidget *page);
		void addPage(QTabWidget *tab, CompletionPage page, const QString &title, const QString &dirname);

		void setListviewEntries(CompletionPage page);
		bool getListviewEntries(CompletionPage page);
		bool isListviewEntry(QTreeWidget *listview, const QString &filename);
		void updateColumnWidth(QTreeWidget *listview);

		QString m_localCwlDir, m_globalCwlDir;
		void getCwlFiles(QMap<QString, QString> &map, QStringList &list, const QString &dir);
		void getCwlDirs();

	private Q_SLOTS:
		void showPage(QWidget *page);
		void addClicked();
		void removeClicked();
		void slotSelectionChanged();
};

#endif
