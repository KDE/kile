/***************************************************************************
    begin                : Oct 03 2011
    author               : dani
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <KShortcut>
#include <KLocale>
#include <KIconDialog>
#include <KInputDialog>
#include <KFileDialog>
#include <KStandardDirs>
#include <KMessageBox>
#include <KXMLGUIClient>
#include <KXMLGUIFactory>

#include "dialogs/latexmenu/latexmenudialog.h"
#include "dialogs/latexmenu/latexmenutree.h"
#include "dialogs/latexmenu/latexmenuitem.h"
#include "latexmenu/latexmenu.h"

#include "kiledebug.h"

namespace KileMenu {

#define CHOOSABLE_MENUTYPES   3

LatexmenuDialog::LatexmenuDialog(KConfig *config, KileInfo *ki, QObject *latexusermenu, const QString &xmlfile, QWidget *parent)
	: KileDialog::Wizard(config, parent), m_ki(ki)
{
	QWidget *page = new QWidget(this);
	setMainWidget(page);
	m_LatexmenuDialog.setupUi(page);
	
	m_menutree = m_LatexmenuDialog.m_twLatexMenu;
	m_menutree->setHeaderLabels( QStringList() << i18n("Menu Entry") << i18n("Shortcut") );
	
	// Indexes must be identical to MenuType. Only the first three of them are choosable (see CHOOSABLE_MENUTYPES)
	m_listMenutypes << i18n("Text") << i18n("Insert file contents") << i18n("Execute program") << i18n("Separator") << i18n("Submenu");

	// search for all action collections (needed for shortcut conflicts)
	QList<KActionCollection *> allCollections;
	foreach ( KXMLGUIClient *client, m_ki->mainWindow()->guiFactory()->clients() ) {
		KILE_DEBUG() << "collection count: " << client->actionCollection()->count() ;
		allCollections += client->actionCollection();
	}
	m_LatexmenuDialog.m_keyChooser->setCheckActionCollections(allCollections);
	KILE_DEBUG() << "total collections: " << allCollections.count();
	
	m_LatexmenuDialog.m_pbInsertBelow->setIcon(KIcon("latexmenu-insert-below.png")); 
	m_LatexmenuDialog.m_pbInsertSubmenu->setIcon(KIcon("latexmenu-submenu-below.png")); 
	m_LatexmenuDialog.m_pbInsertSeparator->setIcon(KIcon("latexmenu-separator-below.png")); 
	m_LatexmenuDialog.m_pbDelete->setIcon(KIcon("latexmenu-delete.png")); 
	m_LatexmenuDialog.m_pbUp->setIcon(KIcon("latexmenu-up.png")); 
	m_LatexmenuDialog.m_pbDown->setIcon(KIcon("latexmenu-down.png")); 
	m_LatexmenuDialog.m_pbIconDelete->setIcon(KIcon("edit-clear-locationbar-rtl.png"));

	connect(m_LatexmenuDialog.m_pbInsertBelow, SIGNAL(clicked()), this, SLOT(slotInsertMenuItem()));
	connect(m_LatexmenuDialog.m_pbInsertSubmenu, SIGNAL(clicked()), this, SLOT(slotInsertSubmenu()));
	connect(m_LatexmenuDialog.m_pbInsertSeparator, SIGNAL(clicked()), this, SLOT(slotInsertSeparator()));
	connect(m_LatexmenuDialog.m_pbUp, SIGNAL(clicked()), this, SLOT(slotUp()));
	connect(m_LatexmenuDialog.m_pbDown, SIGNAL(clicked()), this, SLOT(slotDown()));
	connect(m_LatexmenuDialog.m_pbDelete, SIGNAL(clicked()), this, SLOT(slotDelete()));

	connect(m_menutree, SIGNAL(currentItemChanged(QTreeWidgetItem *,QTreeWidgetItem *)), 
	        this, SLOT(slotCurrentItemChanged(QTreeWidgetItem *,QTreeWidgetItem *)));
	
	connect(m_LatexmenuDialog.m_pbMenuentryType, SIGNAL(clicked()), this, SLOT(slotMenuentryTypeClicked()));
	connect(m_LatexmenuDialog.m_leMenuEntry, SIGNAL(textEdited (const QString &)), this, SLOT(slotMenuentryTextChanged(const QString &)));
	connect(m_LatexmenuDialog.m_urlRequester, SIGNAL(textChanged (const QString &)), this, SLOT(slotUrlTextChanged(const QString &)));
	connect(m_LatexmenuDialog.m_urlRequester, SIGNAL(urlSelected(const KUrl&)), this, SLOT(slotUrlSelected(const KUrl&)));
	connect(m_LatexmenuDialog.m_leParameter, SIGNAL(textEdited (const QString &)), this, SLOT(slotParameterTextChanged(const QString &)));
	connect(m_LatexmenuDialog.m_teText, SIGNAL(textChanged()), this, SLOT(slotPlainTextChanged()));
	connect(m_LatexmenuDialog.m_pbIcon, SIGNAL(clicked()), this, SLOT(slotIconClicked()));
	connect(m_LatexmenuDialog.m_pbIconDelete, SIGNAL(clicked()), this, SLOT(slotIconDeleteClicked()));
	connect(m_LatexmenuDialog.m_keyChooser,SIGNAL(keySequenceChanged(const QKeySequence &)), this,SLOT(slotKeySequenceChanged(const QKeySequence &)));

	connect(m_LatexmenuDialog.m_cbNeedsSelection,   SIGNAL(stateChanged(int)), this, SLOT(slotSelectionStateChanged(int)));
	connect(m_LatexmenuDialog.m_cbContextMenu,      SIGNAL(stateChanged(int)), this, SLOT(slotCheckboxStateChanged(int)));
	connect(m_LatexmenuDialog.m_cbReplaceSelection, SIGNAL(stateChanged(int)), this, SLOT(slotCheckboxStateChanged(int)));
	connect(m_LatexmenuDialog.m_cbSelectInsertion,  SIGNAL(stateChanged(int)), this, SLOT(slotCheckboxStateChanged(int)));
	connect(m_LatexmenuDialog.m_cbInsertOutput,     SIGNAL(stateChanged(int)), this, SLOT(slotCheckboxStateChanged(int)));

	connect(m_LatexmenuDialog.m_pbInstall, SIGNAL(clicked()), this, SLOT(slotInstallClicked()));
	connect(m_LatexmenuDialog.m_pbNew,     SIGNAL(clicked()), this, SLOT(slotNewClicked()));
	
	connect(m_LatexmenuDialog.m_pbLoad,   SIGNAL(clicked()), this, SLOT(slotLoadClicked()));
	connect(m_LatexmenuDialog.m_pbSave,   SIGNAL(clicked()), this, SLOT(slotSaveClicked()));
	connect(m_LatexmenuDialog.m_pbSaveAs, SIGNAL(clicked()), this, SLOT(slotSaveAsClicked()));

	// connect dialog with latexmenu to install xml file
	connect(this, SIGNAL(installXmlFile(const QString &)), latexusermenu, SLOT(slotInstallXmlFile(const QString &)));

	// set context menu handler for the menutree
	m_menutree->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_menutree, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(slotCustomContextMenuRequested(const QPoint &)));
	
	// adjust some widths
	int w = m_LatexmenuDialog.m_pbInsertBelow->sizeHint().width();
	m_LatexmenuDialog.m_pbUp->setMinimumWidth(w);
	m_LatexmenuDialog.m_pbDown->setMinimumWidth(w);
	m_LatexmenuDialog.m_lbIconChosen->setMinimumWidth( m_LatexmenuDialog.m_pbIcon->sizeHint().width() );
	
	setFocusProxy(m_menutree);
	setModal(false);
	setButtons(Help | Cancel | Ok);
	
	KILE_DEBUG() << "start dialog with xmfile " << xmlfile;
	
	if ( !xmlfile.isEmpty() && QFile::exists(xmlfile) ) {
		m_currentXmlInstalled = true;
		loadXmlFile(xmlfile,false);
	}
	else {
		startDialog();
	}
}

void LatexmenuDialog::startDialog()
{
	initDialog();

	// disable modified/install/save button
	m_LatexmenuDialog.m_pbNew->setEnabled(false);
	updateDialogState(false,false,false);
	m_currentXmlInstalled = false;
	setXmlFile(QString::null);
}

void LatexmenuDialog::initDialog()
{
	updateTreeButtons();

	// zur Kontrolle
	QTreeWidgetItem *current = m_menutree->currentItem();
	if ( current ) {
		m_menutree->setCurrentItem(current);
	} 
	else {
	}
		
	// init first entry
	m_currentIcon = QString::null;
	showMenuentryData( dynamic_cast<LatexmenuItem *>(current) );
	
}

void LatexmenuDialog::setModified()
{
	if ( !m_modified ) {
		m_modified = true;
		m_LatexmenuDialog.m_pbInstall->setEnabled(false);
	}

	bool state = !m_menutree->isEmpty();
	m_LatexmenuDialog.m_pbSave->setEnabled(state && !m_currentXmlFile.isEmpty());
	m_LatexmenuDialog.m_pbSaveAs->setEnabled(state);
	m_LatexmenuDialog.m_pbNew->setEnabled(true);
}

void LatexmenuDialog::setXmlFile(const QString &filename)
{
	m_currentXmlFile = filename;
	m_LatexmenuDialog.m_lbXmlFile->setText( i18n("File:") + "   " + QFileInfo(m_currentXmlFile).fileName() );
	if ( m_currentXmlInstalled ) {
		m_LatexmenuDialog.m_lbXmlInstalled->show();
	} else {
		m_LatexmenuDialog.m_lbXmlInstalled->hide();
	}
}

void LatexmenuDialog::updateDialogState(bool modified, bool install, bool save)
{
	m_modified = modified;
	m_LatexmenuDialog.m_pbInstall->setEnabled(install);
	
	save = save && !m_menutree->isEmpty();
	m_LatexmenuDialog.m_pbSave->setEnabled(save && !m_currentXmlFile.isEmpty());
	m_LatexmenuDialog.m_pbSaveAs->setEnabled(save);
}

///////////////////////////// dialog button slots (//////////////////////////////


void LatexmenuDialog::slotButtonClicked(int button)
{
	if ( button == Ok ) {
		if ( m_modified && KMessageBox::questionYesNo(this, i18n("Current menu tree was modified, but not saved.\nDiscard these changes?"))==KMessageBox::No ) {
			return;
		}
		accept();
	}
	else if ( button == Cancel ) {
		accept();
	}
	else if ( button == Help ) {
		QString message = i18n("<p>You can create, change and install a user defined menu, which will appear as a part of Kile's menu. "
			"To create or change this menu, use the six buttons on the left side. "
			"Even more possible actions are available in the context menu of already existing menu items.</p>"
			"<p>Like a standard menu, three different kinds of menu items are available:</p>"
			"<ul>"
			"<li><i>standard entries</i>, which are assigned to an action</li>"
			"<li><i>submenus</i>, which contain more menu items</li>"
			"<li><i>separators</i>, to get a visible structure of all entries</li>"
			"</ul>"
			"<p>Each standard menu item is assigned to one of three action types:</p>"
			"<ul>"
			"<li><i>insert text</i>: this action will insert your text at the current cursor position. "
			"Two metachars are available: <tt>%S</tt> will denote the selected text and <tt>%C</tt> the new cursor position after insertion.</li>"
			"<li><i>file content</i>: inserts the complete contents of a given file (metachars <tt>%S</tt> and <tt>%C</tt> and more are also available)</li>"
			"<li><i>run an external program</i>: The output of this program can be inserted into the opened document. "
			"Metachar <tt>%S</tt> is also possible in the commandline of this program, as the selected text will be saved in a temporary file. "
			"Use <tt>%S</tt> for the filename of this temporary file.</li>"
			"</ul>"
			"<p>If some  important information for an action is missing, menu items are colored red. More information is available using the <i>What's this</i> feature of most widgets.</p>");
		
		KMessageBox::information(this,message,i18n("Latexmenu Dialog"));
	}
	else {
		Wizard::slotButtonClicked(button);
	}
}

///////////////////////////// Button slots (Install/New) //////////////////////////////

void LatexmenuDialog::slotInstallClicked()
{
	KILE_DEBUG() << "install " << m_currentXmlFile << "...";

	if ( !m_modified && !m_currentXmlFile.isEmpty() ) {
		emit ( installXmlFile(m_currentXmlFile) );
		m_currentXmlInstalled = true;
		setXmlFile(m_currentXmlFile);
		
		// disable modified/install/save buttons
		updateDialogState(false,false,false);
	}
}

void LatexmenuDialog::slotNewClicked()
{
	KILE_DEBUG() << "start new menutree ... ";

	if ( !m_menutree->isEmpty() && m_modified  ) {
		if ( KMessageBox::questionYesNo(this, i18n("Current menu tree was modified, but not saved.\nDiscard this tree?")) == KMessageBox::No ) {
			return;
		}
	}
	
	m_menutree->clear();
	startDialog();
}


///////////////////////////// Button slots (Load/Save) //////////////////////////////

void LatexmenuDialog::slotLoadClicked()
{
	KILE_DEBUG() << "load xml file ";
    
	if ( !m_menutree->isEmpty() && m_modified ) {
		if ( KMessageBox::questionYesNo(this, i18n("Current menu tree was modified, but not saved.\nDiscard this tree?")) == KMessageBox::No ) {
			return;
		}
	}
	
	QString directory = LatexUserMenu::selectLatexmenuDir();   
	QString filter = i18n("*.xml|Latex Menu Files");

	QString filename = KFileDialog::getOpenFileName(directory, filter, this, i18n("Select Menu File"));
	if(filename.isEmpty()) {
		return;
	}

	if( !QFile::exists(filename) ) {
		KMessageBox::error(this, i18n("File '%1' does not exist.", filename));
	}
	else {
		m_currentXmlInstalled = false;
		loadXmlFile(filename,true);
	}
}

void LatexmenuDialog::loadXmlFile(const QString &filename, bool install)
{
	KILE_DEBUG() << "load xml started ...";
	m_menutree->readXml(filename);
	initDialog();
	updateDialogState(false,install,false);
	setXmlFile(filename);
	if ( m_menutree->errorCheck() == false ) {
		KILE_DEBUG() << "STOP: found errors in xml file!";
		m_LatexmenuDialog.m_pbInstall->setEnabled(false);
	}
	KILE_DEBUG() << "load xml finished ...";
}

void LatexmenuDialog::slotSaveClicked()
{
	if ( m_currentXmlFile.isEmpty() ) {
		return;
	}
	KILE_DEBUG() << "save menutree: " << m_currentXmlFile;
	
	// read current entry
	QTreeWidgetItem *current = m_menutree->currentItem();
	if ( current ) {
		kdDebug() << "read current item ...";
		readMenuentryData( dynamic_cast<LatexmenuItem *>(current) );
	}
	
	if ( saveCheck() == false ) {
		return;
	}
	
	// force to save file in local directory
	QStringList dirs = KGlobal::dirs()->findDirs("appdata", "latexmenu/");
	if ( dirs.size() > 1 ) {
		if ( m_currentXmlFile.startsWith(dirs[1]) ) {
			m_currentXmlFile.replace(dirs[1],dirs[0]);
			KILE_DEBUG() << "change filename to local directory: " << m_currentXmlFile;
		}
	}
	
	// save file
	m_menutree->writeXml(m_currentXmlFile);
	m_currentXmlInstalled = false;
	setXmlFile(m_currentXmlFile);
	updateDialogState(false,true,false);
}

void LatexmenuDialog::slotSaveAsClicked()
{
	KILE_DEBUG() << "menutree should be save as ...";

	// read current entry
	QTreeWidgetItem *current = m_menutree->currentItem();
	if ( current ) {
		KILE_DEBUG() << "read current item ...";
		readMenuentryData( dynamic_cast<LatexmenuItem *>(current) );
	}

	if ( saveCheck() == false ) {
		return;
	}

	QString directory = KStandardDirs::locateLocal("appdata", "latexmenu/");
	QString filter = i18n("*.xml|Latex Menu Files");

	QString filename = KFileDialog::getSaveFileName(directory, filter, this, i18n("Save Menu File"));
	if(filename.isEmpty()) {
		return;
	}

	if( QFile::exists(filename) ) {
		if ( KMessageBox::questionYesNo(this, i18n("File '%1' does already exist.\nOverwrite this file?", filename)) == KMessageBox::No ) {
			return;
		}
	}
	
	m_menutree->writeXml(filename);
	m_currentXmlInstalled = false;
	setXmlFile(filename);
	
	updateDialogState(false,true,false);
}

bool LatexmenuDialog::saveCheck()
{
	if ( m_menutree->errorCheck() == false ) {
		if ( KMessageBox::questionYesNo(this, i18n("The menu tree contains some errors and installing this file may lead to unpredictable results.\nDo you really want to save this file?")) == KMessageBox::No ) {
			return false;
		}
	}
	
	return true;
}

///////////////////////////// Button slots (left widget) //////////////////////////////

void LatexmenuDialog::slotCustomContextMenuRequested(const QPoint &pos)
{
	m_menutree->contextMenuRequested(pos);
	updateAfterDelete();
}

void LatexmenuDialog::slotInsertMenuItem()
{
	if ( m_menutree->insertMenuItem(m_menutree->currentItem()) ) {
		updateTreeButtons();
		setModified();
	}
}

void LatexmenuDialog::slotInsertSubmenu() 
{
	QTreeWidgetItem *current = m_menutree->currentItem(); 
	if ( current ) {
		if ( m_menutree->insertSubmenu(current) ) { 
			updateTreeButtons();
			setModified();
		}
	}
}

void LatexmenuDialog::slotInsertSeparator()
{
	QTreeWidgetItem *current = m_menutree->currentItem(); 
	if ( current ) {
		if ( m_menutree->insertSeparator(current) ) { 
			updateTreeButtons();
			setModified();
		}
	}
}

void LatexmenuDialog::slotDelete()
{
	QTreeWidgetItem *current = m_menutree->currentItem(); 
	if ( current ) {
		m_menutree->itemDelete(current);
		updateAfterDelete();
	}
}

void LatexmenuDialog::slotUp()
{
	QTreeWidgetItem *current = m_menutree->currentItem(); 
	if ( current ) {
		m_menutree->itemUp();
		updateTreeButtons();
		setModified();
	}
}

void LatexmenuDialog::slotDown()
{
	QTreeWidgetItem *current = m_menutree->currentItem(); 
	if ( current ) {
		m_menutree->itemDown();
		updateTreeButtons();
		setModified();
	}
}

void LatexmenuDialog::updateTreeButtons()
{
	LatexmenuItem *current = dynamic_cast<LatexmenuItem *>(m_menutree->currentItem());
	if ( current ) {
		bool state = ( current->menutype() == LatexmenuData::Separator ) ? false : true;
		m_LatexmenuDialog.m_pbInsertSeparator->setEnabled(state);
		m_LatexmenuDialog.m_pbDelete->setEnabled(true);
		
		bool upstate = ( m_menutree->indexOfTopLevelItem(current) == 0 ) ? false : true;
		m_LatexmenuDialog.m_pbUp->setEnabled(upstate);
		
		bool downstate = ( m_menutree->itemBelow(current) ) ? true : false;
		if ( !downstate && current->parent() ) {
			downstate = true;
		}
		m_LatexmenuDialog.m_pbDown->setEnabled(downstate);
	} 
	else {
		m_LatexmenuDialog.m_pbInsertSeparator->setEnabled(false);
		m_LatexmenuDialog.m_pbDelete->setEnabled(false);
		m_LatexmenuDialog.m_pbUp->setEnabled(false);
		m_LatexmenuDialog.m_pbDown->setEnabled(false);
	} 
}

void LatexmenuDialog::updateAfterDelete()
{
	if ( m_menutree->isEmpty() ) {
		initDialog();
		// disable modified/install/save button
		updateDialogState(false,false,false);
	}
	else {
		updateTreeButtons();
		setModified();
	}
}

////////////////////////////// TreeWidget slots T(left widget)  //////////////////////////////

void LatexmenuDialog::slotCurrentItemChanged(QTreeWidgetItem *current,QTreeWidgetItem *previous)
{
	QString from = ( previous ) ? previous->text(0) : "---";
	QString to   = ( current )  ? current->text(0)  : "---";

	KILE_DEBUG() << "currentItemChanged: from=" << from << "  to=" << to;
	bool modifiedState = m_modified;
	bool installState = m_LatexmenuDialog.m_pbInstall->isEnabled();
	bool saveState = m_LatexmenuDialog.m_pbSave->isEnabled();

	// read old data
	readMenuentryData( dynamic_cast<LatexmenuItem *>(previous) );

	// set new data
	showMenuentryData( dynamic_cast<LatexmenuItem *>(current) );	

	// update buttons for treewidget 
	updateTreeButtons();
	
	// restore saved states
	updateDialogState(modifiedState,installState,saveState);
}

//////////////////////////////  MenuentryType slots (right widget) //////////////////////////////

void LatexmenuDialog::slotMenuentryTypeClicked()
{
	LatexmenuItem *current = dynamic_cast<LatexmenuItem *>(m_menutree->currentItem());   
	if ( !current ) {
		return;
	}
	
	KILE_DEBUG() << "change menu item type of current item: " << current->text(0);
	QStringList typelist;
	for (int i=0; i<CHOOSABLE_MENUTYPES; i++ ) {
		typelist << m_listMenutypes[i];
	}

	int oldtype = current->menutype();
	QStringList sellist;
	sellist <<  m_listMenutypes[oldtype];
	
	QStringList list = KInputDialog::getItemList(i18n("Menutype"), i18n("Please choose a menutype"),
	                                                  typelist,sellist,false);
	if ( list.isEmpty() ) {
		return;
	}
	
	int newtype = m_listMenutypes.indexOf(list[0]);
	if ( newtype==-1 || newtype==oldtype )  {
		return;
	}
	
	// set new values
	current->setMenutype( LatexmenuData::MenuType(newtype) );
	m_LatexmenuDialog.m_lbMenuentryType->setText(list[0]);
	if ( newtype == LatexmenuData::Text ) {
		setMenuentryFileChooser(current,false);
		setMenuentryFileParameter(current,false);
		setMenuentryTextEdit(current,true);
	} 
	else if ( newtype == LatexmenuData::FileContent ) {
		setMenuentryFileChooser(current,true);
		setMenuentryFileParameter(current,false);
		setMenuentryTextEdit(current,false);
	} 
	else /* if ( newtype == LatexmenuData::Program ) */ {
		setMenuentryFileChooser(current,true);
		setMenuentryFileParameter(current,true);
		setMenuentryTextEdit(current,false);
	}
	
	setModified();
}

////////////////////////////// Menuentry slot (right widget) //////////////////////////////

void LatexmenuDialog::slotMenuentryTextChanged(const QString &text)
{
	LatexmenuItem *current = dynamic_cast<LatexmenuItem *>( m_menutree->currentItem() );
	if ( current ) {
		current->setText(0,text);
	}
	setModified();
}

////////////////////////////// KUrlRequester slots (right widget) //////////////////////////////

void LatexmenuDialog::slotUrlTextChanged(const QString &)
{
	LatexmenuItem *current = dynamic_cast<LatexmenuItem *>(m_menutree->currentItem());   
	if ( !current ) {
		return;
	}
	
	QString file = m_LatexmenuDialog.m_urlRequester->text().trimmed();
	
	QString color = "black";
	int type = current->menutype();
	if ( type == LatexmenuData::FileContent ) {
		if ( !QFile::exists(file) || file.isEmpty() ) {
			color = "red";
		}
	} 
	else if ( type == LatexmenuData::Program ) {
		if ( !m_menutree->isItemExecutable(file) ) {
			color= "red";
		}
	}
	
	m_LatexmenuDialog.m_urlRequester->setStyleSheet( "QLineEdit { color: " + color + "; }" );
	setModified();
}

void LatexmenuDialog::slotUrlSelected(const KUrl &)
{
	setModified();
}

////////////////////////////// Parameter slot (right widget) //////////////////////////////

void LatexmenuDialog::slotParameterTextChanged(const QString &)
{
	setModified();
}


////////////////////////////// Text slot (right widget) //////////////////////////////

void LatexmenuDialog::slotPlainTextChanged()
{
	setModified();
}

////////////////////////////// Icon slots (right widget) //////////////////////////////

void  LatexmenuDialog::slotIconClicked()
{
	QString iconname = KIconDialog::getIcon(KIconLoader::Small, KIconLoader::Any,true);
	if ( iconname!=m_currentIcon && !iconname.isEmpty() ) {
		QString iconpath = KIconLoader::global()->iconPath(iconname,KIconLoader::Small);
		KILE_DEBUG() << "icon changed: " << iconname << " path=" << iconpath;
		m_currentIcon = iconpath;
		setMenuentryIcon(m_currentIcon);
		setModified();
	}
}

void  LatexmenuDialog::slotIconDeleteClicked()
{
	m_currentIcon = QString::null;
	setMenuentryIcon(m_currentIcon);
	setModified();
}

void LatexmenuDialog::setMenuentryIcon(const QString &icon)
{
	LatexmenuItem *current = dynamic_cast<LatexmenuItem *>( m_menutree->currentItem() );
	if ( current ) {
		if ( icon.isEmpty() ) {
			current->setIcon(0,KIcon());  
		} else {
			current->setIcon(0,KIcon(icon));
		}
		current->setMenuicon(icon);
	
		// update icon widgets
		setMenuentryIcon(current,true,icon);
		setModified();
	}
}

////////////////////////////// Shortcut slots (right widget) //////////////////////////////

void LatexmenuDialog::slotKeySequenceChanged(const QKeySequence &seq)
{
	QString shortcut = seq.toString(QKeySequence::NativeText);	
	KILE_DEBUG() << "key sequence changed: " << shortcut;
		
	LatexmenuItem *current = dynamic_cast<LatexmenuItem *>( m_menutree->currentItem() );
	if ( current ) {
		current->setText(1,shortcut);
		current->setShortcut(shortcut); 

		m_LatexmenuDialog.m_keyChooser->applyStealShortcut();
		setModified();
	}
}

//////////////////////////////  Selection checkbox slots (right widget) //////////////////////////////

void LatexmenuDialog::slotSelectionStateChanged(int state)
{
	m_LatexmenuDialog.m_cbReplaceSelection->setEnabled(state);
	m_LatexmenuDialog.m_cbContextMenu->setEnabled(state);
	if ( !state ) {
		m_LatexmenuDialog.m_cbReplaceSelection->setChecked(state);
		m_LatexmenuDialog.m_cbContextMenu->setChecked(state);
	}
	setModified();
}

void LatexmenuDialog::slotCheckboxStateChanged(int)
{
	setModified();
}

////////////////////////////// read menu item data //////////////////////////////

void LatexmenuDialog::readMenuentryData(LatexmenuItem *item)
{
	KILE_DEBUG() << "read current menu item ...";
	if ( !item ) {
		return; 
	}
	
	LatexmenuData::MenuType type = LatexmenuData::MenuType( m_listMenutypes.indexOf(m_LatexmenuDialog.m_lbMenuentryType->text()) );
	item->setMenutype(type); 
	if ( type == LatexmenuData::Separator ) {
		return;
	}
	
	item->setMenutitle( m_LatexmenuDialog.m_leMenuEntry->text().trimmed() );
	item->setFilename( m_LatexmenuDialog.m_urlRequester->text().trimmed() ); 
	item->setParameter( m_LatexmenuDialog.m_leParameter->text().trimmed() ); 
	item->setPlaintext( m_LatexmenuDialog.m_teText->toPlainText() ); 

	item->setMenuicon( m_currentIcon );
	item->setShortcut(m_LatexmenuDialog.m_keyChooser->keySequence().toString(QKeySequence::NativeText) );	
	
	item->setNeedsSelection( m_LatexmenuDialog.m_cbNeedsSelection->checkState() );
	item->setUseContextMenu( m_LatexmenuDialog.m_cbContextMenu->checkState() );
	item->setReplaceSelection( m_LatexmenuDialog.m_cbReplaceSelection->checkState() );
	item->setSelectInsertion( m_LatexmenuDialog.m_cbSelectInsertion->checkState() );
	item->setInsertOutput( m_LatexmenuDialog.m_cbInsertOutput->checkState() );

	bool executable = ( type==LatexmenuData::Program && m_menutree->isItemExecutable(item->filename()) ); 
	item->setModelData(executable);

	item->setText(0, item->updateMenutitle());
}

////////////////////////////// show menu item data //////////////////////////////

void LatexmenuDialog::showMenuentryData(LatexmenuItem *item)
{ 
	KILE_DEBUG() << "show new menu item ...";
	if ( !item ) {
		disableMenuEntryData();
		return;
	}
	
	LatexmenuData::MenuType type = item->menutype();   
	
	blockSignals(true);
	switch ( type ) {
		case LatexmenuData::Text:        setTextEntry(item);        break; 
		case LatexmenuData::FileContent: setFileContentEntry(item); break; 
		case LatexmenuData::Program:     setProgramEntry(item);     break; 
		case LatexmenuData::Separator:   setSeparatorEntry(item);   break; 
		case LatexmenuData::Submenu:     setSubmenuEntry(item);     break; 
		default:                         disableMenuEntryData();    // should not happen  
	}
	blockSignals(false);
}

void LatexmenuDialog::setTextEntry(LatexmenuItem *item)
{
	setMenuentryText(item,true);
	setMenuentryType(item,true,true);
	setMenuentryFileChooser(item,false);
	setMenuentryFileParameter(item,false);
	setMenuentryTextEdit(item,true);
	setMenuentryIcon(item,true);
	setMenuentryShortcut(item,true);
	setParameterGroupbox(true);
	setMenuentryCheckboxes(item,false);
}

void LatexmenuDialog::setFileContentEntry(LatexmenuItem *item)
{
	setMenuentryText(item,true);
	setMenuentryType(item,true,true);
	setMenuentryFileChooser(item,true);
	setMenuentryFileParameter(item,false);
	setMenuentryTextEdit(item,false);
	setMenuentryIcon(item,true);
	setMenuentryShortcut(item,true);
	setParameterGroupbox(true);
	setMenuentryCheckboxes(item,false);
}

void LatexmenuDialog::setProgramEntry(LatexmenuItem *item)
{
	setMenuentryText(item,true);
	setMenuentryType(item,true,true);
	setMenuentryFileChooser(item,true);
	setMenuentryFileParameter(item,true);
	setMenuentryTextEdit(item,false);
	setMenuentryIcon(item,true);
	setMenuentryShortcut(item,true);
	setParameterGroupbox(true);
	setMenuentryCheckboxes(item,true);
}

void LatexmenuDialog::setSeparatorEntry(LatexmenuItem *item)
{
	disableMenuEntryData();
	setMenuentryType(item,true,false);
}

void LatexmenuDialog::setSubmenuEntry(LatexmenuItem *item)
{
	setMenuentryText(item,true);
	setMenuentryType(item,true,false);
	setMenuentryFileChooser(0L,false);
	setMenuentryFileParameter(0L,false);
	setMenuentryTextEdit(0L,false);
	setMenuentryIcon(0L,false);
	setMenuentryShortcut(0L,false);
	setParameterGroupbox(false);
	setMenuentryCheckboxes(0L,false); 
}

////////////////////////////// update data widgets//////////////////////////////

void LatexmenuDialog::setMenuentryType(LatexmenuItem *item, bool state1, bool state2)
{
	QString s = ( item && state1 ) ? m_listMenutypes[item->menutype()] : QString::null;
	m_LatexmenuDialog.m_lbMenuentryType->setText(s);
	m_LatexmenuDialog.m_lbMenuentryType->setEnabled(state1);
	m_LatexmenuDialog.m_pbMenuentryType->setEnabled(state2);
}

void LatexmenuDialog::setMenuentryText(LatexmenuItem *item, bool state)
{
	QString s = ( item && state ) ? item->menutitle() : QString::null;
	m_LatexmenuDialog.m_leMenuEntry->setText(s);
	
	m_LatexmenuDialog.m_lbMenuEntry->setEnabled(state);
	m_LatexmenuDialog.m_leMenuEntry->setEnabled(state);
}

void LatexmenuDialog::setMenuentryFileChooser(LatexmenuItem *item, bool state)
{
	QString s = ( item && state ) ? item->filename() : QString::null;
	m_LatexmenuDialog.m_urlRequester->setText(s);
	
	m_LatexmenuDialog.m_lbFile->setEnabled(state);
	m_LatexmenuDialog.m_urlRequester->setEnabled(state);
}

void LatexmenuDialog::setMenuentryFileParameter(LatexmenuItem *item, bool state)
{
	QString s = ( item && state ) ? item->parameter() : QString::null;
	m_LatexmenuDialog.m_leParameter->setText(s);
	
	m_LatexmenuDialog.m_lbParameter->setEnabled(state);
	m_LatexmenuDialog.m_leParameter->setEnabled(state);
		
}

void LatexmenuDialog::setMenuentryTextEdit(LatexmenuItem *item, bool state)
{
	QString s = ( item && state ) ? item->plaintext() : QString::null;
	m_LatexmenuDialog.m_teText->setPlainText(s);
	
	m_LatexmenuDialog.m_lbText->setEnabled(state);
	m_LatexmenuDialog.m_teText->setEnabled(state);
}

void LatexmenuDialog::setMenuentryIcon(LatexmenuItem *item, bool state, const QString &icon) 
{
	if ( item && state ) { 
		m_currentIcon = ( icon.isEmpty() ) ? item->menuicon() : icon;
	} 
	else {
		m_currentIcon = QString::null;
	}

	// update widgets
	if ( m_currentIcon.isEmpty() ) {
		m_LatexmenuDialog.m_lbIconChosen->setText(m_currentIcon);
		m_LatexmenuDialog.m_lbIconChosen->hide(); 
		m_LatexmenuDialog.m_pbIcon->show();
	} 
	else {
		QString iconpath = KIconLoader::global()->iconPath(m_currentIcon,KIconLoader::Small);
		m_LatexmenuDialog.m_lbIconChosen->setText("<img src=\"" +  iconpath +"\" />");
		m_LatexmenuDialog.m_lbIconChosen->show(); 
		m_LatexmenuDialog.m_pbIcon->hide(); 
	}
	
	m_LatexmenuDialog.m_lbIcon->setEnabled(state);
	m_LatexmenuDialog.m_pbIcon->setEnabled(state);
	m_LatexmenuDialog.m_lbIconChosen->setEnabled(state);
	bool deleteIconState = ( state && !m_currentIcon.isEmpty() ); 
	m_LatexmenuDialog.m_pbIconDelete->setEnabled(deleteIconState); 
}

void LatexmenuDialog::setMenuentryShortcut(LatexmenuItem *item, bool state) 
{
	if ( item && state ) { 
		QString shortcut = item->shortcut();
		if ( shortcut.isEmpty() ) {
			m_LatexmenuDialog.m_keyChooser->clearKeySequence();
		}
		else {
			m_LatexmenuDialog.m_keyChooser->setKeySequence( QKeySequence(shortcut) );
		}
		item->setText(1,shortcut);
	} 
	else {
		m_LatexmenuDialog.m_keyChooser->clearKeySequence();
	}
 
	m_LatexmenuDialog.m_lbShortcut->setEnabled(state);
	m_LatexmenuDialog.m_keyChooser->setEnabled(state);
}

void LatexmenuDialog::setParameterGroupbox(bool state)
{
	m_LatexmenuDialog.m_gbParameter->setEnabled(state);
}

void LatexmenuDialog::setMenuentryCheckboxes(LatexmenuItem *item, bool useInsertOutput)
{
	bool selectionState, insertionState, outputState, replaceState, contextState;
	if ( item) { 
		selectionState = item->needsSelection();
		replaceState   = (selectionState) ? item->replaceSelection() : false;
		insertionState = item->selectInsertion();
		outputState    = (useInsertOutput) ? item->insertOutput() : false;
		contextState   = (selectionState) ? item->useContextMenu() : false;
	} 
	else {
		selectionState = false;
		replaceState   = false;
		insertionState = false;
		outputState    = false;
		contextState   = false;
	}

	// m_cbNeedsSelection and m_cbSelectInsertion are always enabled
	m_LatexmenuDialog.m_cbNeedsSelection->setChecked(selectionState);
	m_LatexmenuDialog.m_cbContextMenu->setChecked(contextState);
	m_LatexmenuDialog.m_cbReplaceSelection->setChecked(replaceState);
	m_LatexmenuDialog.m_cbSelectInsertion->setChecked(insertionState);
	m_LatexmenuDialog.m_cbInsertOutput->setChecked(outputState);
	
	m_LatexmenuDialog.m_cbInsertOutput->setEnabled(useInsertOutput);
}

void LatexmenuDialog::clearMenuEntryData()
{
	m_LatexmenuDialog.m_leMenuEntry->setText(QString::null);
	m_LatexmenuDialog.m_lbMenuentryType->setText(QString::null);
	m_LatexmenuDialog.m_urlRequester->setText(QString::null);
	m_LatexmenuDialog.m_teText->setPlainText(QString::null);
	m_LatexmenuDialog.m_pbIcon->setIcon(KIcon(i18n("Choose")));
	m_LatexmenuDialog.m_keyChooser->clearKeySequence();

	m_LatexmenuDialog.m_cbNeedsSelection->setChecked(false);
	m_LatexmenuDialog.m_cbReplaceSelection->setChecked(false);
	m_LatexmenuDialog.m_cbContextMenu->setChecked(false);
	m_LatexmenuDialog.m_cbSelectInsertion->setChecked(false);	
	m_LatexmenuDialog.m_cbInsertOutput->setChecked(false);	
}

void LatexmenuDialog::disableMenuEntryData()
{
	setMenuentryText(0L,false);
	setMenuentryType(0L,false,false);
	setMenuentryFileChooser(0L,false);
	setMenuentryFileParameter(0L,false);
	setMenuentryTextEdit(0L,false);
	setMenuentryIcon(0L,false);
	setMenuentryShortcut(0L,false);
	setParameterGroupbox(false);
	setMenuentryCheckboxes(0L,false); 
}


}

#include "latexmenudialog.moc"
