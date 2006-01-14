/***************************************************************************
    date                 : Jan 12 2006
    version              : 0.20
    copyright            : (C) 2004-2006 by Holger Danielsson
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
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qframe.h>
#include <qwhatsthis.h>
#include <qstringlist.h>

#include "configcodecompletion.h"
#include "kileconfig.h"

ConfigCodeCompletion::ConfigCodeCompletion(KConfig *config, QWidget *parent, const char *name )
   : QWidget(parent,name), m_config(config)
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

   list1 = new KListView(page1);
   list1->addColumn(i18n("Complete Files for TeX/LaTeX Mode"));
   grid1->addWidget(list1,0,0);

   // page 2: Dictionary
   page2 = new QWidget(tab);
   QGridLayout *grid2 = new QGridLayout(page2, 1,1, 10,10);

   list2 = new KListView(page2);
   list2->addColumn(i18n("Complete Files for Dictionary Mode"));
   grid2->addWidget(list2,0,0);

   // page 3: Abbreviation
   page3 = new QWidget(tab);
   QGridLayout *grid3 = new QGridLayout(page3, 1,1, 10,10);

   list3 = new KListView(page3);
   list3->addColumn(i18n("Complete Files for Abbreviation Mode"));
   grid3->addWidget(list3,0,0);

   // add all pages to TabWidget
   tab->addTab(page1,i18n("TeX/LaTeX"));
   tab->addTab(page2,i18n("Dictionary"));
   tab->addTab(page3,i18n("Abbreviation"));

   // add two centered button
   add = new KPushButton(i18n("Add..."),gb_tab);
   remove = new KPushButton(i18n("Remove"),gb_tab);

   grid_tab->addMultiCellWidget(tab,1,1,0,1);
   grid_tab->addWidget(add,2,0,Qt::AlignRight);
   grid_tab->addWidget(remove,2,1,Qt::AlignLeft);

	// below: OptionBox
	QButtonGroup *bg_options = new QButtonGroup( this, "bgOptions" );
	bg_options->setColumnLayout(0, Qt::Vertical );
	bg_options->layout()->setSpacing( 6 );
	bg_options->layout()->setMargin( 11 );
	QGridLayout *bg_optionsLayout = new QGridLayout( bg_options->layout() );
	bg_optionsLayout->setAlignment( Qt::AlignTop );

	cb_setcursor = new QCheckBox(i18n("Place cursor"),bg_options);
	cb_setbullets = new QCheckBox(i18n("Insert bullets"),bg_options);
	cb_closeenv = new QCheckBox(i18n("Close environments"),bg_options);
	cb_usecomplete = new QCheckBox(i18n("Use complete"),bg_options);
	cb_autocomplete = new QCheckBox(i18n("Auto completion (LaTeX)"),bg_options);
	lb_latexthreshold = new QLabel("Threshold:",bg_options);
	sp_latexthreshold = new QSpinBox(1,9,1,bg_options);
	QLabel *lb_latexletters = new QLabel("letters",bg_options);
	cb_autocompletetext = new QCheckBox(i18n("Auto completion (text)"),bg_options);
	lb_textthreshold = new QLabel("Threshold:",bg_options);
	sp_textthreshold = new QSpinBox(1,9,1,bg_options);
	QLabel *lb_textletters = new QLabel("letters",bg_options);

	bg_optionsLayout->addWidget(cb_setcursor,0,0);
	bg_optionsLayout->addWidget(cb_setbullets,1,0);
	bg_optionsLayout->addWidget(cb_closeenv,2,0);
	bg_optionsLayout->addWidget(cb_usecomplete,0,2);
	bg_optionsLayout->addWidget(cb_autocomplete,1,2);
	bg_optionsLayout->addWidget(lb_latexthreshold,1,4);
	bg_optionsLayout->addWidget(sp_latexthreshold,1,6);
	bg_optionsLayout->addWidget(lb_latexletters,1,7);
	bg_optionsLayout->addWidget(cb_autocompletetext,2,2);
	bg_optionsLayout->addWidget(lb_textthreshold,2,4);
	bg_optionsLayout->addWidget(sp_textthreshold,2,6);
	bg_optionsLayout->addWidget(lb_textletters,2,7);

	// tune layout
	bg_optionsLayout->setColSpacing(1,20);
	bg_optionsLayout->setColSpacing(3,12);
	bg_optionsLayout->setColSpacing(5,8);
	bg_optionsLayout->setColStretch(7,1); 
	
	QWhatsThis::add(cb_setcursor,i18n("Try to place the cursor."));
	QWhatsThis::add(cb_setbullets,i18n("Insert bullets, where the user must input data."));
	QWhatsThis::add(cb_closeenv,i18n("Also close an environment, when an opening command is inserted."));
	QWhatsThis::add(cb_usecomplete,i18n("Enable components of word completion."));
	QWhatsThis::add(cb_autocomplete,i18n("Directional or popup-based completion with TeX/LaTeX commands, which are given in all selected word completion lists. This mode can only be selected, if no other plugin for autocompletion is active."));
	QWhatsThis::add(cb_autocompletetext,i18n("Directional or popup-based completion from words in the current document. This mode can only be selected, if no other plugin for autocompletion is active."));
	QWhatsThis::add(sp_latexthreshold,i18n("Automatically show a completion list of TeX/LaTeX commands, when the word has this length."));
	QWhatsThis::add(sp_textthreshold,i18n("Automatically show a completion list, when the word has this length."));

	// bottom: warning
	QLabel *lb_automodes = new QLabel("Warning: both autocompletion modes will be disabled, if you enable KTextEditor plugin word completion.",this);
	
	// add OptionBox and TabDialog into the layout
	vbox->addWidget(gb_tab);
	vbox->addWidget(bg_options);
	vbox->addWidget(lb_automodes);
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

	// set checkboxes and thresholds for autocompletion modes
	if ( kateCompletionPlugin() )
	{
   	cb_autocomplete->setChecked( false );
		cb_autocompletetext->setChecked( false );
	}
	else
	{
   	cb_autocomplete->setChecked( KileConfig::completeAuto() );
		cb_autocompletetext->setChecked( KileConfig::completeAutoText() );
	}
	sp_latexthreshold->setValue( KileConfig::completeAutoThreshold() );
	sp_textthreshold->setValue( KileConfig::completeAutoTextThreshold() );

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

	// read autocompletion settings
	bool autoModeLatex = cb_autocomplete->isChecked();
	bool autoModeText = cb_autocompletetext->isChecked();
	if ( kateCompletionPlugin() )
	{
		if ( autoModeLatex || autoModeText )
		{
			QString msg = i18n("You enabled the KTextEditor-Plugin for word completion, "
			                   "but this conflicts with the auto completion modes of Kile. "
			                   "As only one of these completion modes can be used, the "
			                   "autocompletion modes of Kile will be disabled.");
			KMessageBox::information( 0L,"<center>" + msg + "</center>",i18n("Autocomplete warning") );

			// disable Kile autocomplete modes
			autoModeLatex = false;
			autoModeText = false;
		}
	}

	// save settings for Kile autocompletion modes
	KileConfig::setCompleteAuto( autoModeLatex );
	KileConfig::setCompleteAutoText( autoModeText );
	KileConfig::setCompleteAutoThreshold( sp_latexthreshold->value() );
	KileConfig::setCompleteAutoTextThreshold( sp_textthreshold->value() );

   // save changed wordlists?
   KileConfig::setCompleteChangedLists(changed);
}

// read kate plugin configuration
bool ConfigCodeCompletion::kateCompletionPlugin()
{
	m_config->setGroup("Kate Document Defaults");
	return m_config->readBoolEntry("KTextEditor Plugin ktexteditor_docwordcompletion",false);
}

//////////////////// listview ////////////////////

// ListView fr den Konfigurationsdialog einstellen

void ConfigCodeCompletion::setListviewEntries(KListView *listview, const QStringList &files)
{
   // Daten aus der Konfigurationsliste in das ListView-Widget eintragen
   listview->setUpdatesEnabled(false);
   listview->clear();
   for (uint i=0; i<files.count(); ++i) {
      QString s = files[i];
      QCheckListItem *item = new QCheckListItem(listview,s.right(s.length()-2),QCheckListItem::CheckBox);
      item->setOn( s.at(0) == '1' ? true : false );
      listview->insertItem(item);
   }
   listview->setUpdatesEnabled(true);
}

bool ConfigCodeCompletion::getListviewEntries(KListView *listview, QStringList &files)
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
      ++index;
   }

   // only update if there are changes
   if ( changed )
      files = newfiles;

   return changed;
}

bool ConfigCodeCompletion::isListviewEntry(KListView *listview, const QString &filename)
{
	QCheckListItem *item = (QCheckListItem *)listview->firstChild();
	while ( item ) 
	{
		if ( item->text() == filename )
			return true;
		item = (QCheckListItem *)item->nextSibling();
	}
	return false;
}

//////////////////// tabpages parameter ////////////////////

KListView *ConfigCodeCompletion::getListview(QWidget *page)
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
   KListView *list = getListview(page);
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

   QStringList filenames =KFileDialog::getOpenFileNames( basedir,i18n("*.cwl|Complete Files"),this,i18n("Select File"));

	KListView *list = getListview(tab->currentPage());     // get current page
	for ( QStringList::Iterator it = filenames.begin(); it != filenames.end(); ++it )
	{
		QString filename = *it;
	
		// could we accept the wordlist?
		QFileInfo fi(filename);
		if ( !filename.isEmpty() && fi.exists() && fi.isReadable() )
		{
			// check basedir
			if ( fi.dirPath(true) == basedir )
			{
				int len = basedir.length() + 1;
				QString basename = filename.mid(len,filename.length()-len-4);
			
				// check if this entry already exists
				if ( isListviewEntry(list,basename) )
				{
					if ( KMessageBox::questionYesNo(0,i18n("Wordlist '%1' is already used.").arg(basename),i18n("Duplicate Files"),i18n("&Skip"),KStdGuiItem::cancel()) == KMessageBox::Yes )
						continue;     // skip this entry
					else
						break;        // cancel all following entries
				}
				// add new entry
				QCheckListItem *item = new QCheckListItem(list,basename,QCheckListItem::CheckBox);
				item->setOn(true);
				list->insertItem(item);
			}
		else
			KMessageBox::information(0,i18n("Maybe you have changed the directory?"));
	
		}
	}

}

// delete a selected entry

void ConfigCodeCompletion::removeClicked()
{
   QWidget *page = tab->currentPage();
   KListView *list = getListview(page);                              // determine page
   QCheckListItem *item = (QCheckListItem *)list->selectedItem();    // determine entry

   if ( item ) {
      list->takeItem(item);
      // Button enabled/disabled?
      showPage(page);
   }
}

#include "configcodecompletion.moc"
