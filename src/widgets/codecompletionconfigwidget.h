/********************************************************************************
  Copyright (C) 2004-2007 by Holger Danielsson (holger.danielsson@versanet.de)
                2010 by Michel Ludwig (michel.ludwig@liverpool.ac.uk)
 ********************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CODECOMPLETIONCONFIGWIDGET_H
#define CODECOMPLETIONCONFIGWIDGET_H

#include <QWidget>

#include "ui_codecompletionconfigwidget.h"

class QCheckBox;
class QLabel;
class QSpinBox;
class QTabWidget;
class QTreeWidget;
class QTreeWidgetItem;

class KConfig;
class QPushButton;
class QTabWidget;
class KDirWatch;

class KileErrorHandler;

class CodeCompletionConfigWidget : public QWidget, public Ui::KileWidgetCodeCompletionConfig
{
    Q_OBJECT
public:
    CodeCompletionConfigWidget(KConfig *config, KileErrorHandler *errorHandler, QWidget *parent = Q_NULLPTR, const char *name = Q_NULLPTR);
    ~CodeCompletionConfigWidget();

    void readConfig(void);
    void writeConfig(void);

private:
    enum CompletionPage { TexPage = 0, DictionaryPage = 1, AbbreviationPage = 2, NumPages = 3 };

    KConfig *m_config;
    KileErrorHandler *m_errorHandler;

    // tabs, views, pages, wordlists
    QTreeWidget *m_listview[NumPages];
    QWidget *m_page[NumPages];
    QStringList m_wordlist[NumPages];
    QStringList m_dirname;

    QTreeWidget *getListview(QWidget *page);
    QString getListname(QWidget *page);
    void addPage(QTabWidget *tab, CompletionPage page, const QString &title, const QString &dirname);

    void setListviewEntries(CompletionPage page);
    bool getListviewEntries(CompletionPage page);
    QTreeWidgetItem* getListviewEntry(QTreeWidget *listview, const QString &filename);
    void updateColumnWidth(QTreeWidget *listview);

    bool m_configChanged;
    QString m_localCwlDir, m_globalCwlDir;
    KDirWatch *m_dirWatcher;

private Q_SLOTS:
    void showPage(QWidget *page);
    void showPage(int index);
    void addClicked();
    void removeClicked();
    void slotSelectionChanged();
    void updateCompletionFilesTab(const QString& path);
};

#endif
