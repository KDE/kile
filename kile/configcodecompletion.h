/***************************************************************************
    date                 : Dez 02 2005
    version              : 0.12
    copyright            : (C) 2004-2005 by Holger Danielsson
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

#ifndef CONFIGCODECOMPLETION_H
#define CONFIGCODECOMPLETION_H

#include <qwidget.h>
#include <qtabwidget.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qstring.h>

#include <klistview.h>
#include <kpushbutton.h>
#include <kconfig.h>

/**
  *@author Holger Danielsson
  */

class ConfigCodeCompletion : public QWidget
{
    Q_OBJECT
public: 
   ConfigCodeCompletion(KConfig *config, bool viewOpened, QWidget *parent=0, const char *name=0);
   ~ConfigCodeCompletion();

   void readConfig(void);
   void writeConfig(void);

private:
    KConfig *m_config;
    bool m_viewOpened;
    bool m_stateAutomode;
    
    // tabs, views and pages
    QTabWidget *tab;
    KListView *list1,*list2,*list3;         
    QWidget *page1,*page2,*page3;           
    KPushButton *add,*remove;                

    // Checkboxes/Spinboxes
    QCheckBox *cb_usecomplete, *cb_autocomplete, *cb_autocompletetext;
    QCheckBox *cb_setcursor, *cb_setbullets;
    QCheckBox *cb_closeenv;
    QSpinBox *sp_latexthreshold, *sp_textthreshold;
    QLabel *lb_latexthreshold, *lb_textthreshold;

    // wordlists
    QStringList m_texlist;
    QStringList m_dictlist;
    QStringList m_abbrevlist;
    
    KListView *getListview(QWidget *page);        
    QString getListname(QWidget *page);         

    void setListviewEntries(KListView *listview, const QStringList &files);
    bool getListviewEntries(KListView *listview, QStringList &files);
    bool isListviewEntry(KListView *listview, const QString &filename);
    
private slots:
   void showPage(QWidget *page);
   void addClicked();
   void removeClicked();

};

#endif
