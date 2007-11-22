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

#include "configcodecompletion.h"

#include <kdialog.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <qlayout.h>
#include <qtabwidget.h>
#include <q3groupbox.h>
#include <q3vgroupbox.h>
#include <q3buttongroup.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <q3frame.h>
#include <q3whatsthis.h>
#include <qstringlist.h>
#include <qfileinfo.h>
#include <qdir.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3VBoxLayout>

#include "kilelistselector.h"
#include "kileconfig.h"
#include "kilelogwidget.h"
#include "kiletool_enums.h"

ConfigCodeCompletion::ConfigCodeCompletion(KConfig *config, KileWidget::LogMsg *logwidget, QWidget *parent, const char *name )
   : QWidget(parent,name), m_config(config), m_logwidget(logwidget)
{
   // Layout
    Q3VBoxLayout *vbox = new Q3VBoxLayout(this, 5,KDialog::spacingHint() );

   // Groupbox with TabDialog and two button
   Q3GroupBox *gb_tab= new Q3GroupBox(i18n("Complete Modes"), this );
   Q3GridLayout *grid_tab = new Q3GridLayout( gb_tab, 2,1, 12,8, "" );
   grid_tab->addRowSpacing( 0, 12 );

	// create TabWidget
	tab = new QTabWidget(gb_tab);

	// add three pages: Tex/Latex, Dictionary, Abbreviation
	addPage(tab, TexPage, i18n("TeX/LaTeX"), "tex");
	addPage(tab, DictionaryPage, i18n("Dictionary"), "dictionary");
	addPage(tab, AbbreviationPage, i18n("Abbreviation"), "abbreviation");

   // add two centered button
   add = new KPushButton(i18n("Add..."),gb_tab);
   remove = new KPushButton(i18n("Remove"),gb_tab);

   grid_tab->addMultiCellWidget(tab,1,1,0,1);
   grid_tab->addWidget(add,2,0,Qt::AlignRight);
   grid_tab->addWidget(remove,2,1,Qt::AlignLeft);

	// below: OptionBox
	Q3ButtonGroup *bg_options = new Q3ButtonGroup( this, "bgOptions" );
	bg_options->setColumnLayout(0, Qt::Vertical );
	bg_options->layout()->setSpacing( 6 );
	bg_options->layout()->setMargin( 11 );
	Q3GridLayout *bg_optionsLayout = new Q3GridLayout( bg_options->layout() );
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
	cb_showabbrevview = new QCheckBox(i18n("Show abbreviations"),bg_options);
	cb_autocompleteabbrev = new QCheckBox(i18n("Auto completion (abbrev.)"),bg_options);
	cb_citeoutofbraces = new QCheckBox(i18n("Move out of braces (citation keylists)"),bg_options);

	bg_optionsLayout->addWidget(cb_setcursor,0,0);
	bg_optionsLayout->addWidget(cb_setbullets,1,0);
	bg_optionsLayout->addWidget(cb_closeenv,2,0);
	bg_optionsLayout->addWidget(cb_showabbrevview,3,0);
	bg_optionsLayout->addWidget(cb_usecomplete,0,2);
	bg_optionsLayout->addWidget(cb_autocomplete,1,2);
	bg_optionsLayout->addWidget(lb_latexthreshold,1,4);
	bg_optionsLayout->addWidget(sp_latexthreshold,1,6);
	bg_optionsLayout->addWidget(lb_latexletters,1,7);
	bg_optionsLayout->addWidget(cb_autocompletetext,2,2);
	bg_optionsLayout->addWidget(lb_textthreshold,2,4);
	bg_optionsLayout->addWidget(sp_textthreshold,2,6);
	bg_optionsLayout->addWidget(lb_textletters,2,7);
	bg_optionsLayout->addWidget(cb_autocompleteabbrev,3,2);
	bg_optionsLayout->addMultiCellWidget(cb_citeoutofbraces,4,4,0,7);

	// tune layout
	bg_optionsLayout->setColSpacing(1,20);
	bg_optionsLayout->setColSpacing(3,12);
	bg_optionsLayout->setColSpacing(5,8);
	bg_optionsLayout->setColStretch(7,1); 
	
	Q3WhatsThis::add(cb_setcursor,i18n("Try to place the cursor."));
	Q3WhatsThis::add(cb_setbullets,i18n("Insert bullets, where the user must input data."));
	Q3WhatsThis::add(cb_closeenv,i18n("Also close an environment, when an opening command is inserted."));
	Q3WhatsThis::add(cb_usecomplete,i18n("Enable components of word completion."));
	Q3WhatsThis::add(cb_autocomplete,i18n("Directional or popup-based completion with TeX/LaTeX commands, which are given in all selected word completion lists. This mode can only be selected, if no other plugin for autocompletion is active."));
	Q3WhatsThis::add(cb_autocompletetext,i18n("Directional or popup-based completion from words in the current document. This mode can only be selected, if no other plugin for autocompletion is active."));
	Q3WhatsThis::add(sp_latexthreshold,i18n("Automatically show a completion list of TeX/LaTeX commands, when the word has this length."));
	Q3WhatsThis::add(sp_textthreshold,i18n("Automatically show a completion list, when the word has this length."));
	Q3WhatsThis::add(cb_citeoutofbraces,i18n("Move cursor out of braces after selecting from a citation keylist."));

	// bottom: warning
	QLabel *lb_automodes = new QLabel("Warning: all autocompletion modes will be disabled, if you enable KTextEditor plugin word completion.",this);
	
	// add OptionBox and TabDialog into the layout
	vbox->addWidget(gb_tab);
	vbox->addWidget(bg_options);
	vbox->addWidget(lb_automodes);
	vbox->addStretch();

   connect(tab,SIGNAL(currentChanged(QWidget*)),this,SLOT(showPage(QWidget*)));
   connect(add,SIGNAL(clicked()),this,SLOT(addClicked()));
   connect(remove,SIGNAL(clicked()),this,SLOT(removeClicked()));

   // justify height
   Q3CheckListItem *item = new Q3CheckListItem(m_listview[AbbreviationPage], "Test", Q3CheckListItem::CheckBox);
   int h = 6*(item->height()+1) + 1;
 	for ( uint i=TexPage; i<NumPages; ++i )
		m_listview[i]->setFixedHeight(h);
   delete item;

	// find resource directories for cwl files
	getCwlDirs();
}

ConfigCodeCompletion::~ConfigCodeCompletion()
{
}

void ConfigCodeCompletion::addPage(QTabWidget *tab, CompletionPage page, const QString &title, const QString &dirname)
{
	m_page[page] = new QWidget(tab);

	m_listview[page] = new KListView( m_page[page] );
	m_listview[page]->addColumn(i18n("Complete Files"));
	m_listview[page]->addColumn(i18n("Local File"));
	m_listview[page]->setFullWidth(true);

	Q3GridLayout *grid = new Q3GridLayout(m_page[page], 1,1, 10,10);
	grid->addWidget(m_listview[page],0,0);

	// add Tab
	tab->addTab(m_page[page],title);

	// remember directory name
	m_dirname << dirname;

	connect(m_listview[page], SIGNAL(clicked( Q3ListViewItem *)), this, SLOT(slotListviewClicked(Q3ListViewItem *)));
}

//////////////////// read/write configuration ////////////////////

void ConfigCodeCompletion::readConfig(void)
{
   // read selected and deselected filenames with wordlists
   m_wordlist[TexPage] = KileConfig::completeTex();
   m_wordlist[DictionaryPage]  = KileConfig::completeDict();
   m_wordlist[AbbreviationPage]  = KileConfig::completeAbbrev();

   // set checkbox status
   cb_usecomplete->setChecked( KileConfig::completeEnabled() );
   cb_setcursor->setChecked( KileConfig::completeCursor() );
   cb_setbullets->setChecked( KileConfig::completeBullets() );
   cb_closeenv->setChecked( KileConfig::completeCloseEnv() );
	cb_showabbrevview->setChecked( KileConfig::completeShowAbbrev() );
	cb_citeoutofbraces->setChecked( KileConfig::completeCitationMove() );

	// set checkboxes and thresholds for autocompletion modes
	if ( kateCompletionPlugin() )
	{
   	cb_autocomplete->setChecked( false );
		cb_autocompletetext->setChecked( false );
		cb_autocompleteabbrev->setChecked( false );
	}
	else
	{
   	cb_autocomplete->setChecked( KileConfig::completeAuto() );
		cb_autocompletetext->setChecked( KileConfig::completeAutoText() );
		cb_autocompleteabbrev->setChecked( KileConfig::completeAutoAbbrev() );
	}
	sp_latexthreshold->setValue( KileConfig::completeAutoThreshold() );
	sp_textthreshold->setValue( KileConfig::completeAutoTextThreshold() );

   // insert filenames into listview
	for ( uint i=TexPage; i<NumPages; ++i )
		setListviewEntries( CompletionPage(i) );
}

void ConfigCodeCompletion::writeConfig(void)
{
   // default: no changes in configuration
   bool changed = false;

   // get listview entries
	for ( uint i=TexPage; i<NumPages; ++i )
		changed |= getListviewEntries( CompletionPage(i) );

   // Konfigurationslisten abspeichern
   KileConfig::setCompleteTex( m_wordlist[TexPage] );
   KileConfig::setCompleteDict( m_wordlist[DictionaryPage] );
   KileConfig::setCompleteAbbrev( m_wordlist[AbbreviationPage] );

   // save checkbox status
   KileConfig::setCompleteEnabled(cb_usecomplete->isChecked());
   KileConfig::setCompleteCursor(cb_setcursor->isChecked());
   KileConfig::setCompleteBullets(cb_setbullets->isChecked());
   KileConfig::setCompleteCloseEnv(cb_closeenv->isChecked());
	KileConfig::setCompleteShowAbbrev( cb_showabbrevview->isChecked() );
	KileConfig::setCompleteCitationMove( cb_citeoutofbraces->isChecked() );

	// read autocompletion settings
	bool autoModeLatex = cb_autocomplete->isChecked();
	bool autoModeText = cb_autocompletetext->isChecked();
	bool autoModeAbbrev = cb_autocompleteabbrev->isChecked();
	if ( kateCompletionPlugin() )
	{
		if ( autoModeLatex || autoModeText  || autoModeAbbrev)
		{
			QString msg = i18n("You enabled the KTextEditor-Plugin for word completion, "
			                   "but this conflicts with the auto completion modes of Kile. "
			                   "As only one of these completion modes can be used, the "
			                   "autocompletion modes of Kile will be disabled.");
			KMessageBox::information( 0L,"<center>" + msg + "</center>",i18n("Autocomplete warning") );

			// disable Kile autocomplete modes
			autoModeLatex = false;
			autoModeText = false;
			autoModeAbbrev = false;
		}
	}

	// save settings for Kile autocompletion modes
	KileConfig::setCompleteAuto( autoModeLatex );
	KileConfig::setCompleteAutoText( autoModeText );
	KileConfig::setCompleteAutoAbbrev( autoModeAbbrev );
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

void ConfigCodeCompletion::setListviewEntries(CompletionPage page)
{
	QString listname = m_dirname[page];
	QString localdir = m_localCwlDir + listname + '/';
	QString globaldir = m_globalCwlDir + listname + '/';

	// Daten aus der Konfigurationsliste in das ListView-Widget eintragen
	m_listview[page]->setUpdatesEnabled(false);
	m_listview[page]->clear();
	QStringList::ConstIterator it;
	for ( it=m_wordlist[page].begin(); it!=m_wordlist[page].end(); ++it )
	{
		QString basename = (*it).right( (*it).length()-2 );
		bool localExists = QFileInfo(localdir+basename+".cwl").exists();
		
		Q3CheckListItem *item = new Q3CheckListItem(m_listview[page],basename,Q3CheckListItem::CheckBox);
		if ( localExists )
		{
			item->setOn( (*it).at(0) == '1' ? true : false );
			item->setText(1,"+");
		}
		else if ( QFileInfo(globaldir+basename+".cwl").exists() )
		{	
			item->setOn( (*it).at(0) == '1' ? true : false );
		}
		else
		{
			item->setOn(false);
			item->setText(1,i18n("File not found"));
		}
		m_listview[page]->insertItem(item);
	}
	
	updateColumnWidth(m_listview[page]);
	m_listview[page]->setUpdatesEnabled(true);
}

void ConfigCodeCompletion::updateColumnWidth(KListView *listview)
{
	listview->setColumnWidth(0,listview->columnWidth(0)+60);
}

bool ConfigCodeCompletion::getListviewEntries(CompletionPage page)
{
   bool changed = false;

   // count number of entries
   uint n = m_listview[page]->childCount();

    // there are changes if this number has changed
   if ( n != m_wordlist[page].count() )
      changed = true;

   // clear all stringlist with files, if there are no entries
   if ( n == 0 ) {
      m_wordlist[page].clear();
      return changed;
   }

   // now check all entries if they have changed
   QStringList newfiles;
   int index = 0;
   Q3CheckListItem *item = (Q3CheckListItem *)m_listview[page]->firstChild();
   while ( item ) {
      QString s = ( item->isOn() ) ? "1-" : "0-";
      s += item->text(0);
      newfiles.append(s);

      // check for a change
      if ( m_wordlist[page][index] != s )
         changed = true;

      // go on
      item = (Q3CheckListItem *)item->nextSibling();
      ++index;
   }

   // only update if there are changes
   if ( changed )
      m_wordlist[page] = newfiles;

   return changed;
}

bool ConfigCodeCompletion::isListviewEntry(KListView *listview, const QString &filename)
{
	Q3CheckListItem *item = (Q3CheckListItem *)listview->firstChild();
	while ( item ) 
	{
		if ( item->text() == filename )
			return true;
		item = (Q3CheckListItem *)item->nextSibling();
	}
	return false;
}

//////////////////// tabpages parameter ////////////////////

KListView *ConfigCodeCompletion::getListview(QWidget *page)
{
	for ( uint i=TexPage; i<NumPages; ++i )
	{
		if ( page == m_page[i] )
			return m_listview[i];
	}
	return 0;
}

QString ConfigCodeCompletion::getListname(QWidget *page)
{
	for ( uint i=TexPage; i<NumPages; ++i )
	{
		if ( page == m_page[i] )
			return m_dirname[i];
	}
	return QString::null;
}

//////////////////// shwo tabpages ////////////////////

void ConfigCodeCompletion::showPage(QWidget *page)
{
	KListView *listview = getListview(page);
	if ( listview ) 
		remove->setEnabled( listview->selectedItems().count() > 0 );
}

//////////////////// add/remove new wordlists ////////////////////

// find local and global resource directories

void ConfigCodeCompletion::getCwlDirs()
{
	m_localCwlDir = locateLocal("appdata","complete/");
	m_globalCwlDir = QString::null;

	QStringList dirs = KGlobal::dirs()->findDirs("appdata","complete/");
	for ( QStringList::ConstIterator it=dirs.begin(); it!=dirs.end(); ++it )
	{
		if ( (*it) != m_localCwlDir )
		{
			m_globalCwlDir = (*it);
			break;
		}
	}
}

// find local and global cwl files: global files are not added,
// if there is already a local file with this name. We fill a map
// with filename as key and filepath as value. Additionally all 
// filenames are added to a stringlist.

void ConfigCodeCompletion::getCwlFiles(QMap<QString,QString> &map, QStringList &list, const QString &dir)
{
	QStringList files = QDir(dir,"*.cwl").entryList();
	for ( QStringList::ConstIterator it=files.begin(); it!=files.end(); ++it )
	{
		QString filename = QFileInfo(*it).fileName();
		if ( ! map.contains(filename) )
		{
			map[filename] = dir + '/' + (*it);
			list << filename;
		}
	}
}

void ConfigCodeCompletion::addClicked()
{
	// determine current subdirectory for current tab page	
	QString listname = getListname(tab->currentPage());


	// get a sorted list of all cwl files from both directories
	QMap<QString,QString> filemap;
	QStringList filelist;
	getCwlFiles(filemap,filelist,m_localCwlDir+listname);
	getCwlFiles(filemap,filelist,m_globalCwlDir+listname);
	filelist.sort();

	// dialog to add cwl files
	KileListSelectorMultiple *dlg  = new KileListSelectorMultiple(filelist,i18n("Complete Files"),i18n("Select Files"), this);
	if ( dlg->exec() ) 
	{
		if ( dlg->currentItem() >= 0 ) 
		{
			KListView *listview = getListview(tab->currentPage());     // get current page
			QStringList filenames = dlg->selected();                   // get selected files
			for ( QStringList::ConstIterator it=filenames.begin(); it!=filenames.end(); ++it )
			{
				QString filename = *it;
				// could we accept the wordlist?
				QFileInfo fi( filemap[filename] );
				if ( !filename.isEmpty() && fi.exists() && fi.isReadable() )
				{
					QString basename = filename.left(filename.length()-4);

					// check if this entry already exists
					if ( isListviewEntry(listview,basename) )
					{
						m_logwidget->printMsg(KileTool::Info,i18n("Wordlist '%1' is already used.").arg(basename),i18n("Complete"));
						continue;
					}

					// add new entry
					Q3CheckListItem *item = new Q3CheckListItem(listview,basename,Q3CheckListItem::CheckBox);
					item->setOn(true);
					item->setSelected(true);
					if ( filemap[filename].left(m_localCwlDir.length()) == m_localCwlDir )
						item->setText(1,"+");

					listview->insertItem(item);
				}
			}
			updateColumnWidth(listview);
		}
	}
	delete dlg;

}

// delete a selected entry

void ConfigCodeCompletion::removeClicked()
{
   QWidget *page = tab->currentPage();
   KListView *list = getListview(page);                              // determine page
   Q3CheckListItem *item = (Q3CheckListItem *)list->selectedItem();    // determine entry

	if ( item ) 
	{
		list->takeItem(item);
		delete item;
		// Button enabled/disabled?
		showPage(page);
	}
}

void ConfigCodeCompletion::slotListviewClicked(Q3ListViewItem *)
{
	KListView *listview = getListview(tab->currentPage());     // get current page
	remove->setEnabled( listview->selectedItems().count() > 0 );
}

#include "configcodecompletion.moc"
