/***************************************************************************
                           configcodecompletion.cpp 
----------------------------------------------------------------------------
    date                 : Jan 17 2004
    version              : 0.10.2
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

#include <kdialog.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <qlayout.h>
#include <qtabwidget.h>
#include <qgroupbox.h>
#include <qvgroupbox.h>
#include <qframe.h>
#include <qstringlist.h>

#include "configcodecompletion.h"
#include "kileconfig.h"

ConfigCodeCompletion::ConfigCodeCompletion(QWidget *parent, const char *name )
   : QWidget(parent,name)
{
   // Layout
    QVBoxLayout *vbox = new QVBoxLayout(this, 5,KDialog::spacingHint() );

   // Groupbox with TabDialog and two button
   QGroupBox *gb_tab= new QGroupBox(i18n("Complete Modes"), this );
   QGridLayout *grid_tab = new QGridLayout( gb_tab, 2,1, 12,8, "" );
   grid_tab->addRowSpacing( 0, 12 );

   tab = new QTabWidget(gb_tab);

   // page 1: Tex/Latex
   page1 = new QWidget(tab);
   QGridLayout *grid1 = new QGridLayout(page1, 1,1, 10,10);

   list1 = new QListView(page1);
   list1->addColumn(i18n("Complete files for TeX/LaTeX mode"));
   grid1->addWidget(list1,0,0);

   // page 2: Dictionary
   page2 = new QWidget(tab);
   QGridLayout *grid2 = new QGridLayout(page2, 1,1, 10,10);

   list2 = new QListView(page2);
   list2->addColumn(i18n("Complete files for dictionary mode"));
   grid2->addWidget(list2,0,0);

   // page 3: Abbreviation
   page3 = new QWidget(tab);
   QGridLayout *grid3 = new QGridLayout(page3, 1,1, 10,10);

   list3 = new QListView(page3);
   list3->addColumn(i18n("Complete files for abbreviation mode"));
   grid3->addWidget(list3,0,0);

   // add all pages to TabWidget 
   tab->addTab(page1,i18n("TeX/LaTeX"));
   tab->addTab(page2,i18n("Dictionary"));
   tab->addTab(page3,i18n("Abbreviation"));

   // add two centered button
   add = new QPushButton(i18n("Add"),gb_tab);
   remove = new QPushButton(i18n("Remove"),gb_tab);

   grid_tab->addMultiCellWidget(tab,1,1,0,1);
   grid_tab->addWidget(add,2,0,Qt::AlignRight);
   grid_tab->addWidget(remove,2,1,Qt::AlignLeft);

   // below: OptionBox
   QGroupBox *gb_opt= new QGroupBox(2,Qt::Horizontal,i18n("Options"), this );

   cb_setcursor = new QCheckBox(i18n("place cursor"),gb_opt);
   cb_usecomplete = new QCheckBox(i18n("use complete"),gb_opt);
   cb_setbullets = new QCheckBox(i18n("insert bullets"),gb_opt);
   cb_autocomplete = new QCheckBox(i18n("auto completion"),gb_opt);
   cb_closeenv = new QCheckBox(i18n("close environments"),gb_opt);
   
   // add OptionBox and TabDialog into the layout
   vbox->addWidget(gb_tab);
   vbox->addWidget(gb_opt);
   vbox->addStretch();

   connect(tab,SIGNAL(currentChanged(QWidget*)),this,SLOT(showPage(QWidget*)));
   connect(add,SIGNAL(clicked()),this,SLOT(addClicked()));
   connect(remove,SIGNAL(clicked()),this,SLOT(removeClicked()));

   // justify height
   QCheckListItem *item = new QCheckListItem(list3,"Test",QCheckListItem::CheckBox);
   int h = 6*(item->height()+1) + 1;
   list1->setFixedHeight(h);
   list2->setFixedHeight(h);
   list3->setFixedHeight(h);
   delete item;

}

ConfigCodeCompletion::~ConfigCodeCompletion()
{
}

//////////////////// read/write configuration ////////////////////

void ConfigCodeCompletion::readConfig(void)
{   
   // read selected and deselected filenames with wordlists
   m_texlist = KileConfig::completeTex();
   m_dictlist = KileConfig::completeDict();
   m_abbrevlist = KileConfig::completeAbbrev();

   // set checkbox status
   cb_usecomplete->setChecked( KileConfig::completeEnabled() );
   cb_setcursor->setChecked( KileConfig::completeCursor() );
   cb_setbullets->setChecked( KileConfig::completeBullets() );
   cb_closeenv->setChecked( KileConfig::completeCloseEnv() );
   cb_autocomplete->setChecked( KileConfig::completeAuto() );

   // insert filenames into listview 
   setListviewEntries(list1,m_texlist);
   setListviewEntries(list2,m_dictlist);
   setListviewEntries(list3,m_abbrevlist);

}

void ConfigCodeCompletion::writeConfig(void)
{
   // default: no changes in configuration
   bool changed = false;

   // get listview entries
   changed |= getListviewEntries(list1,m_texlist);
   changed |= getListviewEntries(list2,m_dictlist);
   changed |= getListviewEntries(list3,m_abbrevlist);
   
   // Konfigurationslisten abspeichern
   KileConfig::setCompleteTex(m_texlist);
   KileConfig::setCompleteDict(m_dictlist);
   KileConfig::setCompleteAbbrev(m_abbrevlist);
   
   // save checkbox status
   KileConfig::setCompleteEnabled(cb_usecomplete->isChecked());
   KileConfig::setCompleteCursor(cb_setcursor->isChecked());
   KileConfig::setCompleteBullets(cb_setbullets->isChecked());
   KileConfig::setCompleteCloseEnv(cb_closeenv->isChecked());
   KileConfig::setCompleteAuto(cb_autocomplete->isChecked());

   // sind die Wortlisten geändert?
   KileConfig::setCompleteChangedLists(changed);

}

//////////////////// listview ////////////////////

// ListView für den Konfigurationsdialog einstellen

void ConfigCodeCompletion::setListviewEntries(QListView *listview, const QStringList &files)
{
   // Daten aus der Konfigurationsliste in das ListView-Widget eintragen
   listview->setUpdatesEnabled(false);
   listview->clear();
   for (uint i=0; i<files.count(); i++) {
      QString s = files[i];
      QCheckListItem *item = new QCheckListItem(listview,s.right(s.length()-2),QCheckListItem::CheckBox);
      item->setOn( s.at(0) == '1' ? true : false );
      listview->insertItem(item);
   }
   listview->setUpdatesEnabled(true);
}

bool ConfigCodeCompletion::getListviewEntries(QListView *listview, QStringList &files)
{
   bool changed = false;
   
   // count number of entries
   uint n = listview->childCount();

    // there are changes if this number has changed
   if ( n != files.count() )
      changed = true;

   // clear all stringlist with files, if there are no entries
   if ( n == 0 ) {
      files.clear();
      return changed;
   }

   // now check all entries if they have changed
   QStringList newfiles;
   int index = 0;
   QCheckListItem *item = (QCheckListItem *)listview->firstChild();
   while ( item ) {
      QString s = ( item->isOn() ) ? "1-" : "0-";
      s += item->text(0);
      newfiles.append(s);         
 
      // check for a change
      if ( files[index] != s )
         changed = true;

      // go on
      item = (QCheckListItem *)item->nextSibling();
      index++;
   }

   // only update if there are changes
   if ( changed )
      files = newfiles;

   return changed;
}

//////////////////// tabpages parameter ////////////////////

QListView *ConfigCodeCompletion::getListview(QWidget *page)      
{
   if ( page == page1 )
      return list1;
   else if ( page == page2 )
      return list2;
   else if ( page == page3 )
      return list3;
   else
      return 0;
}

QString ConfigCodeCompletion::getListname(QWidget *page) 
{
   if ( page == page1 )
      return "tex";
   else if ( page == page2 )
      return "dictionary";
   else if ( page == page3 )
      return "abbreviation";
   else
      return QString::null;
}

//////////////////// shwo tabpages ////////////////////

void ConfigCodeCompletion::showPage(QWidget *page)    
{             
   QListView *list = getListview(page);
   if ( list ) {
      if ( list->childCount() == 0 )
         remove->setEnabled(false);
      else
         remove->setEnabled(true);
   }
}

//////////////////// add/remove new wordlists ////////////////////

void ConfigCodeCompletion::addClicked()    
{

   QString listname = getListname(tab->currentPage());   // determine name
   QString basedir = locate("appdata","complete/") + listname;

   QString filename = KFileDialog::getOpenFileName( basedir,i18n("*.cwl|complete files"),
                                                    this,i18n("Select a File") );

   // could we accept the wordlist?
   QFileInfo fi(filename);
   if ( !filename.isEmpty() && fi.exists() && fi.isReadable() )  {
        // check basedir
        if ( fi.dirPath(true) == basedir )
        {
           int len = basedir.length() + 1;
           QString basename = filename.mid(len,filename.length()-len-4);

           // add new entry
           QListView *list = getListview(tab->currentPage());     // get current page
           QCheckListItem *item = new QCheckListItem(list,basename,QCheckListItem::CheckBox);
           item->setOn(true);
           list->insertItem(item);
        }
        else
           KMessageBox::information(0,i18n("Maybe you have changed the directory?"));

   }
}

// delete a selected entry

void ConfigCodeCompletion::removeClicked()
{
   QWidget *page = tab->currentPage();
   QListView *list = getListview(page);                              // determine page
   QCheckListItem *item = (QCheckListItem *)list->selectedItem();    // determine entry

   if ( item ) {                                                     
      list->takeItem(item);
      // Button enabled/disabled?
      showPage(page);
   }
}

#include "configcodecompletion.moc"
