/***********************************************************************************
  Copyright (C) 2011-2012 by Holger Danielsson (holger.danielsson@versanet.de)
 ***********************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <QFile>
#include <QRegExp>

#include <KTemporaryFile>
#include <KXMLGUIFactory>
#include <KMenuBar>
#include <KAction>
#include <KStandardDirs>
#include <KFileDialog>
#include <KMessageBox>

#include "kileactions.h"
#include "editorextension.h"
#include "kileviewmanager.h"
#include "usermenu/usermenu.h"

#include "kileconfig.h"
#include "kiledebug.h"


namespace KileMenu {

// The UserMenu uses six values/data structures:
//
//  - m_usermenu: the menu with its entries/actions itself (QMenu *)
//    (actions for menu items are named 'useraction-n', where n is a
//     number starting at 0. It is also used as index for the m_menudata list.)
//
//  - m_menudata: a list, containing all info for menu item (QList<UserMenuData>)
//
//  - m_actioncollection: KActionCollection of KileMainWindow (KActionCollection *)
//
//  - m_actionlist: a list with all actions of the menu (QList<KAction *>)
//
//  - m_actionlistContextMenu: a list with all actions of the context menu for selected text (QList<KAction *>)
//
//  - a menu is defined in an xml file, which is placed in KGlobal::dirs()->findResource("appdata","usermenu/")

UserMenu::UserMenu(KileInfo *ki, QObject *receiver)
	: m_ki(ki), m_receiver(receiver), m_proc(NULL)
{
	KXmlGuiWindow *mainwindow = m_ki->mainWindow();
	m_actioncollection = mainwindow->actionCollection();

	// add actions and menu entries
	QMenu *wizard_menu = dynamic_cast<QMenu*>(mainwindow->guiFactory()->container("wizard", mainwindow));
	m_wizardAction1 = wizard_menu->addSeparator();
	m_wizardAction2 = createAction("wizard_usermenu");
	wizard_menu->addAction(m_wizardAction2);

	QMenu *latex_menu  = dynamic_cast<QMenu*>(mainwindow->guiFactory()->container("menu_latex", mainwindow));
	m_latexAction1 = latex_menu->addSeparator();
	m_latexAction2 = createAction("wizard_usermenu2");
	latex_menu->addAction(m_latexAction2);

	m_latexMenuEntry = new QMenu(i18n("User Menu"));
	m_latexMenuEntry->setObjectName("usermenu-submenu");
	latex_menu->addMenu(m_latexMenuEntry);

	// prepare menu position
	m_menuLocation = KileConfig::menuLocation();
	m_usermenu = (m_menuLocation == StandAloneLocation)
	            ? dynamic_cast<QMenu*>(mainwindow->guiFactory()->container("menu_usermenu", mainwindow))
	            : m_latexMenuEntry;

	// look for an existing menufile:
	// if filename matches 'basename.ext' then the file is placed in 'KILE-LOCAL-DIR/usermenu' directory
	m_currentXmlFile = KileConfig::menuFile();
	if ( !m_currentXmlFile.isEmpty() ) {
		if ( !m_currentXmlFile.contains("/") ) {
			m_currentXmlFile = KGlobal::dirs()->findResource("appdata","usermenu/") + m_currentXmlFile;
		}

		if ( QFile(m_currentXmlFile).exists() ) {
			KILE_DEBUG() << "install menufile: " << m_currentXmlFile;
			installXml(m_currentXmlFile);
		}
		else {
			m_currentXmlFile.clear();
		}
	}

	updateUsermenuPosition();
}

UserMenu::~UserMenu()
{
	delete m_proc;
}

bool UserMenu::isEmpty()
{
	return (m_usermenu->actions().size() == 0);
}
/////////////////////// install usermenu//////////////////////////////

KAction *UserMenu::createAction(const QString &name)
{
	KAction *action = m_actioncollection->addAction(name, m_receiver, SLOT(quickUserMenuDialog()));
	action->setText(i18n("Edit User Menu"));
	action->setIcon(KIcon("wizard_usermenu"));
	return action;
}

void UserMenu::updateUsermenuPosition()
{
	// and set the new one
	const bool show = !isEmpty() && m_ki->viewManager()->currentTextView();
	if(m_menuLocation == StandAloneLocation) {
		setStandAloneMenuVisible(true, show);
	}
	else {
		setStandAloneMenuVisible(false, show);
	}
}

void UserMenu::changeMenuLocation(int newPosition)
{
	// clear old usermenu, wherever it is
	clear();

	// set new usermenu position
	KXmlGuiWindow *mainwindow = m_ki->mainWindow();
	m_menuLocation = newPosition;
	m_usermenu = (m_menuLocation == StandAloneLocation)
	            ? dynamic_cast<QMenu*>(mainwindow->guiFactory()->container("menu_usermenu", mainwindow))
	            : m_latexMenuEntry;

	installXmlFile(m_currentXmlFile);
	updateUsermenuPosition();
}

void UserMenu::setStandAloneMenuVisible(bool state, bool show)
{
	m_wizardAction1->setVisible(state);
	m_wizardAction2->setVisible(state);

	m_latexAction1->setVisible(!state);
	m_latexAction2->setVisible(!state);
	m_latexMenuEntry->menuAction()->setVisible(!state && show);

	KXmlGuiWindow *mainwindow = m_ki->mainWindow();
	QMenu *standAloneMenu = dynamic_cast<QMenu*>(mainwindow->guiFactory()->container("menu_usermenu", mainwindow));
	if(standAloneMenu) {
		standAloneMenu->menuAction()->setVisible(state && show);
	}
}

///////////////////////////// clear all data //////////////////////////////

// clear all lists and data for an existing usermenu
void UserMenu::clear()
{
	// clear usermenu and menudata
	if ( m_usermenu ) {
		m_usermenu->clear();
	}
	m_menudata.clear();

	// remove all actions from actioncollection
	foreach ( KAction *action, m_actionlist ) {
		m_actioncollection->removeAction(action);
	}

	// clear actionlists
	m_actionlist.clear();
	m_actionlistContextMenu.clear();
}

///////////////////////////// update GUI //////////////////////////////

// GUI was updated and all menu items disappeared
void UserMenu::updateGui()
{
	KILE_DEBUG() << "update usermenu ...";

	if(m_menuLocation == StandAloneLocation) {
		KXmlGuiWindow *mainwindow = m_ki->mainWindow();
		m_usermenu = dynamic_cast<QMenu*>(mainwindow->guiFactory()->container("menu_usermenu", mainwindow));
	}

	// like installXmlFile(), but without updating KileConfig::menuFile
	// first clear old usermenu, menudata, actions and actionlists
	clear();

	// then install
	if ( installXml(m_currentXmlFile) ) {
		// add changed context menu to all existing views
	   KileView::Manager* viewManager = m_ki->viewManager();
		int views = viewManager->textViewCount();
		for ( int i=0; i<views; ++i ) {
			viewManager->installContextMenu( viewManager->textView(i) );
		}
	}

}

///////////////////////////// update key bindings //////////////////////////////

// shortcut dialog was called, so key bindings may have been changed
void UserMenu::updateKeyBindings()
{
	if ( m_currentXmlFile.isEmpty() && !QFile(m_currentXmlFile).exists() ) {
		return;
	}

	// new key bindings are found in kileui.rc (ActionProperties)
	// remove them, as they will be written into usermenu xml file
	removeActionProperties();

	// update xml file of current usermenu
	updateXmlFile(m_currentXmlFile);
}

void UserMenu::removeActionProperties()
{
	QString xmlfile = "kileui.rc";
	QString xml(KXMLGUIFactory::readConfigFile(xmlfile));
	if ( xml.isEmpty() ) {
		KILE_DEBUG() << "STOP: xmlfile not found: " << xmlfile;
		return;
	}

	QDomDocument doc;
	doc.setContent( xml );

	// process XML data in section 'ActionProperties'
	QDomElement actionPropElement = KXMLGUIFactory::actionPropertiesElement( doc );
	if ( actionPropElement.isNull() ) {
		KILE_DEBUG() << "QDomElement actionPropertiesElement not found ";
		return;
	}

	// search for all actions of the user defined UserMenu
	KILE_DEBUG() << "QDomElement actionPropertiesElement found ";
	bool changed = false;
	QRegExp re("useraction-(\\d+)$");
	QDomElement e = actionPropElement.firstChildElement();
	while(!e.isNull()) {
		QString tag = e.tagName();
		if(tag != "Action") {
			continue;
		}

		QString shortcut = e.attribute("shortcut");
		QString name = e.attribute("name");

		QDomElement removeElement;
		if ( re.indexIn(name) == 0) {
			int index = re.cap(1).toInt();
			KILE_DEBUG() << "action property was changed: old=" << m_menudata[index].shortcut << " new=" << name << " actionIndex=" << index;
			removeElement = e;
			changed = true;
		}

		e = e.nextSiblingElement();

		// finally delete element
		if ( !removeElement.isNull() ) {
			KILE_DEBUG() << "remove ActionProperty: shortcut=" << shortcut << " name=" << name;
			actionPropElement.removeChild(removeElement);
		}
	}

	// Write back to XML file
	if ( changed ) {
		KXMLGUIFactory::saveConfigFile(doc,xmlfile);
	}
}

///////////////////////////// update action properties (shortcuts) //////////////////////////////

// Calling m_mainWindow->guiFactory()->refreshActionProperties() in kile.cpp removes all
// user defined action shortcuts and icons. Here they will be refreshed again.
void UserMenu::refreshActionProperties()
{
	KILE_DEBUG() << "refresh action properties";

	QRegExp re("useraction-(\\d+)$");
	foreach ( KAction *action, m_actionlist ) {
		if ( re.indexIn(action->objectName()) == 0 ) {
			int actionIndex = re.cap(1).toInt();
			if ( !m_menudata[actionIndex].icon.isEmpty() ) {
				action->setIcon( KIcon(m_menudata[actionIndex].icon) );
			}
			if ( !m_menudata[actionIndex].shortcut.isEmpty() ) {
				action->setShortcut( QKeySequence(m_menudata[actionIndex].shortcut,QKeySequence::NativeText) );
			}
		}
	}
}

// Before calling usermenu dialog, all user defined action shortcuts must be removed,
// or the dialog will give a lot of warnings. All shortcuts (even if changed) in the usermenu
// will be refreshed again, when the dialog is finished
void UserMenu::removeShortcuts()
{
	foreach ( KAction *action, m_actionlist ) {
		action->setShortcut( KShortcut() );
	}
}

///////////////////////////// install/remove xml //////////////////////////////

// call from the menu: no xml file given
void UserMenu::installXmlMenufile()
{
	KILE_DEBUG() << "install xml file with KFileDialog::getOpenFileName";

	QString directory = selectUserMenuDir();
	QString filter = i18n("*.xml|Latex Menu Files");

	QString filename = KFileDialog::getOpenFileName(directory, filter, m_ki->mainWindow(), i18n("Select Menu File"));
	if(filename.isEmpty()) {
		return;
	}

	if( !QFile::exists(filename) ) {
		KMessageBox::error(m_ki->mainWindow(), i18n("File '%1' does not exist.", filename));
	}
	else {
		installXmlFile(filename);
	}
}

// SIGNAL from usermenu dialog: install new usermenu (xml file given)
//
// use 'basename.ext' if the file is placed in 'KILE-LOCAL-DIR/usermenu' directory and full filepath else
void UserMenu::installXmlFile(const QString &filename)
{
	KILE_DEBUG() << "install xml file" << filename;

	// clear old usermenu, menudata, actions and actionlists
	clear();

	if ( installXml(filename) ) {
		// update current xml filename (with path)
		m_currentXmlFile = filename;

		// save xml file in config (with or without path)
		QString xmlfile = filename;
		QString dir = KGlobal::dirs()->findResource("appdata","usermenu/");
		if ( filename.startsWith(dir) ) {
			QString basename = filename.right( filename.length()-dir.length() );
			if ( !basename.isEmpty() && !basename.contains("/") )  {
				xmlfile = basename;
			}
		}
		KileConfig::setMenuFile(xmlfile);
		emit (updateStatus());

		// add changed context menu to all existing views
	   KileView::Manager* viewManager = m_ki->viewManager();
		int views = viewManager->textViewCount();
		for ( int i=0; i<views; ++i ) {
			viewManager->installContextMenu( viewManager->textView(i) );
		}
	}
}

void UserMenu::removeXmlFile()
{
	KILE_DEBUG() << "remove xml file";

	clear();
	m_currentXmlFile.clear();

	KileConfig::setMenuFile(m_currentXmlFile);
	emit (updateStatus());
}

///////////////////////////// install usermenu from XML //////////////////////////////

// pre: usermenu is already cleared
bool UserMenu::installXml(const QString &filename)
{
	KILE_DEBUG() << "install: start";

	if ( !m_usermenu ) {
		KILE_DEBUG() << "Hmmmm: found no usermenu";
		return false;
	}

	// read content of xml file
	QDomDocument doc("UserMenu");
	QFile file(filename);
	if ( !file.open(QFile::ReadOnly | QFile::Text) ) {
		// TODO KMessageBox
		KILE_DEBUG() << "STOP: can't open xml file " << filename;
		return false;
	}

	if( !doc.setContent( &file ) ) {
		file.close();
		return false;
	}
	file.close();

	KILE_DEBUG() << "parse xml ...";
	m_actionsContextMenu = 0;

	// parse toplevelitems
	int actionnumber = 0;
	QDomElement root = doc.documentElement();
	QDomElement e = root.firstChildElement();
	while ( !e.isNull()) {
		QString tag = e.tagName();

		if ( tag=="submenu" || tag=="separator") {
			if ( tag == "submenu" ) {
				installXmlSubmenu(e,m_usermenu,actionnumber);
			}
			else /* tag=="separator" */ {
				m_usermenu->addSeparator();
			}

			// try to get some structure into to the context menu
			if ( m_actionsContextMenu > 0 ) {
				m_actionlistContextMenu.append(NULL);
				m_actionsContextMenu = 0;
			}
		}
		else /* if ( tag == "menu" ) */ {
			installXmlMenuentry(e,m_usermenu,actionnumber);
		}

		e = e.nextSiblingElement();
	}
	KILE_DEBUG() << "install: finished ";

	return true;
}

// install a submenu item
void UserMenu::installXmlSubmenu(const QDomElement &element, QMenu *parentmenu, int &actionnumber)
{
	QMenu *submenu = parentmenu->addMenu(QString());

	QString title;
	if ( element.hasChildNodes() ) {
		QDomElement e = element.firstChildElement();
		while ( !e.isNull()) {

			QString tag = e.tagName();
			if ( tag == "title" ) {
				title = e.text();
				submenu->setTitle(title);
			}
			else if ( tag == "submenu" ) {
				installXmlSubmenu(e,submenu,actionnumber);
			}
			else if ( tag == "separator" ) {
				submenu->addSeparator();
			}
			else /* if ( tag == "menu" ) */ {
				installXmlMenuentry(e,submenu,actionnumber);
			}

			e = e.nextSiblingElement();
		}
	}
}

// install a standard menu item
void UserMenu::installXmlMenuentry(const QDomElement &element, QMenu *parentmenu, int &actionnumber)
{
	UserMenuData menudata;

	menudata.menutype  = UserMenuData::xmlMenuType( element.attribute("type") );

	// read values
	if ( element.hasChildNodes() ) {
		QDomElement e = element.firstChildElement();
		while ( !e.isNull()) {
			QString tag = e.tagName();
			QString text = e.text();

			int index = UserMenuData::xmlMenuTag(tag);
			switch (index) {
				case  UserMenuData::XML_TITLE:            menudata.menutitle = text;                   break;
				case  UserMenuData::XML_PLAINTEXT:        menudata.text = text.replace("\\n","\n");    break;
				case  UserMenuData::XML_FILENAME:         menudata.filename = text;                    break;
				case  UserMenuData::XML_PARAMETER:        menudata.parameter = text;                   break;
				case  UserMenuData::XML_ICON:             menudata.icon = text;                        break;
				case  UserMenuData::XML_SHORTCUT:         menudata.shortcut = text;                    break;
				case  UserMenuData::XML_NEEDSSELECTION:   menudata.needsSelection   = str2bool(text);  break;
				case  UserMenuData::XML_USECONTEXTMENU:   menudata.useContextMenu   = str2bool(text);  break;
				case  UserMenuData::XML_REPLACESELECTION: menudata.replaceSelection = str2bool(text);  break;
				case  UserMenuData::XML_SELECTINSERTION:  menudata.selectInsertion  = str2bool(text);  break;
				case  UserMenuData::XML_INSERTOUTPUT:     menudata.insertOutput     = str2bool(text);  break;
			}

			e = e.nextSiblingElement();
		}
	}

	// add menu item, if its title is not empty
	if ( !menudata.menutitle.isEmpty() ) {

		KAction *action = m_actioncollection->addAction(QString("useraction-%1").arg(actionnumber), this, SLOT(slotUserMenuAction()) );
		if ( action ) {
			 action->setText(menudata.menutitle);

			if ( !menudata.icon.isEmpty() ) {
				action->setIcon( KIcon(menudata.icon) );
			}

			if ( !menudata.shortcut.isEmpty() ) {
				action->setShortcut( QKeySequence(menudata.shortcut,QKeySequence::PortableText) );
			}

			parentmenu->addAction(action);
			m_menudata.append(menudata);
			m_actionlist.append(action);
			if ( menudata.useContextMenu )  {
				m_actionlistContextMenu.append(action);
				m_actionsContextMenu++;
			}

			actionnumber++;
		}
	}
}

///////////////////////////// update XML file //////////////////////////////

// key bindings dialog was called, so the usermenu xml file must be updated, if one of these actions was changed
// pre: xml file exists
void UserMenu::updateXmlFile(const QString &filename)
{
	KILE_DEBUG() << "update xml file: " << filename;

	// read content of xml file
	QDomDocument doc("UserMenu");
	QFile file(filename);
	file.open(QFile::ReadOnly | QFile::Text);
	doc.setContent(&file);
	file.close();

	KILE_DEBUG() << "parse xml ...";

	// parse toplevelitems
	bool changed = false;
	int actionnumber = 0;
	QDomElement root = doc.documentElement();
	QDomElement e = root.firstChildElement();
	while ( !e.isNull()) {
		QString tag = e.tagName();
		if ( tag == "submenu" ) {
			changed = changed || updateXmlSubmenu(doc,e,actionnumber);
		}
		else if ( tag == "menu" ) {
			changed = changed || updateXmlMenuentry(doc,e,actionnumber);
		}
		e = e.nextSiblingElement();
	}
	KILE_DEBUG() << "update finished ";

	if ( changed ) {
		KILE_DEBUG() << "found changes, so write updated xml file ";
		QFile outfile(filename);
		outfile.open(QFile::WriteOnly | QFile::Text);
		QTextStream stream(&outfile);
		doc.save(stream,3);
		outfile.close();
	}
}

// install a submenu item
bool UserMenu::updateXmlSubmenu(QDomDocument &doc, QDomElement &element, int &actionnumber)
{
	bool changed = false;

	if ( element.hasChildNodes() ) {
		QDomElement e = element.firstChildElement();
		while ( !e.isNull()) {
			QString tag = e.tagName();
			if ( tag == "submenu" ) {
				changed = changed || updateXmlSubmenu(doc,e,actionnumber);
			}
			else if ( tag == "menu" )  {
				changed = changed || updateXmlMenuentry(doc,e,actionnumber);
			}

			e = e.nextSiblingElement();
		}
	}
	return changed;
}

// install a standard menu item
bool UserMenu::updateXmlMenuentry(QDomDocument &doc, QDomElement &element, int &actionnumber)
{
	bool changed = false;

	// read values
	if ( element.hasChildNodes() ) {
		QDomElement oldElement;
		QDomElement e = element.firstChildElement();
		while ( !e.isNull()) {
			QString tag = e.tagName();

			if ( UserMenuData::xmlMenuTag(tag) == UserMenuData::XML_SHORTCUT) {
				oldElement = e;
				//oldText = e.text();    value not needed, is also in m_menudata[]
			}

			e = e.nextSiblingElement();
		}

		// keybindings dialog has already updated all actions
		QString currentShortcut = m_actionlist[actionnumber]->shortcut().toString(QKeySequence::PortableText);
		if ( currentShortcut != m_menudata[actionnumber].shortcut ) {
			// an existing shortcut always needs a new QDomElement
			if ( !currentShortcut.isEmpty() ) {
				// create element with new shortcut
				QDomElement newElement = doc.createElement( UserMenuData::xmlMenuTagName(UserMenuData::XML_SHORTCUT) );
				QDomText newText = doc.createTextNode(currentShortcut);
				newElement.appendChild(newText);

				// replace existing node with new node
				if ( !oldElement.isNull() ) {
					element.replaceChild(newElement,oldElement);
				}
				// or insert a new node
				else {
					element.appendChild(newElement);
				}
			}
			// or delete an existing QDomElement
			else {
				element.removeChild(oldElement);
			}
			changed = true;
		}
	}

	actionnumber++;
	return changed;
}

////////////////////////////// load dir for xml files (static) //////////////////////////////

// - start with search for xml files in the local directory
// - if no files are present, but in global directory, start with global
// - if not a single file was found: back to local directory

QString UserMenu::selectUserMenuDir()
{
	QStringList dirs = KGlobal::dirs()->findDirs("appdata", "usermenu/");
	if ( dirs.size() < 2 ) {
		return dirs.at(0);
	}

	QStringList namefilter = QStringList() << "*.xml";
	QString localDirName = dirs.at(0);
	QDir localDir = QDir(localDirName);
	QStringList localList = localDir.entryList (namefilter,QDir::Files | QDir::Readable);
	if ( localList.size() > 0 ) {
		return localDirName;
	}

	QDir globalDir = QDir(dirs.at(1));
	QStringList globalList = globalDir.entryList (namefilter,QDir::Files | QDir::Readable);
	return ( globalList.size() > 0 ) ? dirs.at(1) : localDirName;
}

////////////////////////////// execUserMenuAction //////////////////////////////

// an action was called from the usermenu:
//  - identify action
//  - find textview
//  - execute action
void UserMenu::slotUserMenuAction()
{
	KILE_DEBUG() << "want to start an action from usermenu ...";

	KAction *action = dynamic_cast<KAction *>(sender());
	if ( !action ) {
		return;
	}

	QString actionName = action->objectName();
	KILE_DEBUG() << "action name: " << actionName << "classname=" << action->metaObject()->className();

	QRegExp re("useraction-(\\d+)$");
	if ( re.indexIn(actionName) != 0) {
			KILE_DEBUG() << "STOP: found wrong action name: " << actionName;
		return;
	}

	bool ok;
	int actionIndex = re.cap(1).toInt(&ok);
	if ( actionIndex < 0 || actionIndex >= m_menudata.size() ) {
		KILE_DEBUG() << "STOP: invalid action (range error): " << actionIndex << "  list size: " << m_menudata.size();
		return;
	}

	// check view and action requirements
	KTextEditor::View *view = m_ki->viewManager()->currentTextView();

	if ( !view ) {
		return;
	}

	if ( !view->selection() && m_menudata[actionIndex].needsSelection ) {
		return;
	}

	UserMenuData::MenuType type = m_menudata[actionIndex].menutype;

	if ( type == UserMenuData::Text ) {
		execActionText(view,m_menudata[actionIndex]);
	}
	else if ( type == UserMenuData::FileContent ) {
		execActionFileContent(view,m_menudata[actionIndex]);
	}
	else if ( type == UserMenuData::Program ) {
		execActionProgramOutput(view,m_menudata[actionIndex]);
	}
	else {
		KILE_DEBUG() << "STOP: unknown action type: " << type;
	}
}

////////////////////////////// execActionText //////////////////////////////

// execute an action: insert text
void UserMenu::execActionText(KTextEditor::View *view, const UserMenuData &menudata)
{
	KILE_DEBUG() << "want to insert text ... ";
	insertText(view, menudata.text, menudata.replaceSelection, menudata.selectInsertion);
}

////////////////////////////// execActionFileContent //////////////////////////////

// execute an action: insert file contents
void UserMenu::execActionFileContent(KTextEditor::View *view, const UserMenuData &menudata)
{
	KILE_DEBUG() << "want to insert contents of a file: " << menudata.filename;

	QFile file(menudata.filename);
	if ( !file.open(QFile::ReadOnly | QFile::Text) ) {
		KILE_DEBUG() << "STOP: could not open file " << menudata.filename;
		return;
	}

	QTextStream stream( &file );
	QString text = stream.readAll();
	file.close();

	if ( !text.isEmpty() ) {
		insertText(view, text, menudata.replaceSelection, menudata.selectInsertion);
	}
}

////////////////////////////// execActionFileContent //////////////////////////////

// execute an action: run a program
void UserMenu::execActionProgramOutput(KTextEditor::View *view, const UserMenuData &menudata)
{
	KILE_DEBUG() << "want to start a program ... ";

	// delete old process
	if (m_proc) {
		delete m_proc;
		m_proc = NULL;
	}

	// build commandline
	QString cmdline = menudata.filename + " " + menudata.parameter;
	bool useTemporaryFile = cmdline.contains("%M");

	bool needsSelection = menudata.needsSelection;
	bool hasSelection = view->selection();

	// check parameter
	if ( needsSelection && !hasSelection ) {
		KILE_DEBUG() << "STOP: this program needs selected text";
		return;
	}

	// do we need a temporary file for the selected text?
	if ( hasSelection && useTemporaryFile ) {
		KILE_DEBUG() << "selection and 'placeholder' %M found --> create temporary file";

		// create temporary file
		KTemporaryFile tempfile;
		tempfile.setSuffix(".txt");
		tempfile.setAutoRemove(false);

		if ( !tempfile.open() ) {
			KILE_DEBUG() << "STOP: could not create tempfile for selected text" ;
			return;
		}

		// get filename
		QString selfile = tempfile.fileName();

		// write selection
		QTextStream stream( &tempfile );
		stream << view->selectionText() << "\n";
		tempfile.close();

		// update comamndline with temporary filename of selection
		cmdline.replace("%M",selfile);
	}

	// replace %F with the complete base name of the file without the path.
	// The complete base name consists of all characters in the file up to
	// (but not including) the last '.' character.
	if (  cmdline.contains("%S") ) {
		QFileInfo fi(view->document()->url().toLocalFile());
		QString basename = fi.completeBaseName();
		cmdline.replace("%S",basename);
	}

	m_proc = new KProcess(this);
	m_proc->setShellCommand(cmdline);
	m_proc->setOutputChannelMode(KProcess::MergedChannels);
	m_proc->setReadChannel(QProcess::StandardOutput);

	connect(m_proc, SIGNAL(readyReadStandardOutput()), this, SLOT(slotProcessOutput()));
	connect(m_proc, SIGNAL(readyReadStandardError()),  this, SLOT(slotProcessOutput()));
	connect(m_proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotProcessExited(int, QProcess::ExitStatus)));

	KILE_DEBUG() << "... start proc: " << cmdline;
	// init and/or save important data
	m_procOutput.clear();
	m_procView = view;
	m_procMenudata = &menudata;

	m_proc->start();
}

void UserMenu::slotProcessOutput()
{
	m_procOutput += m_proc->readAll();
}

void UserMenu::slotProcessExited(int /* exitCode */, QProcess::ExitStatus exitStatus)
{
	KILE_DEBUG() << "... finish proc ";
	KILE_DEBUG() << "output:  " << m_procOutput;

	if ( exitStatus == QProcess::NormalExit && m_procMenudata->insertOutput && !m_procOutput.isEmpty() ) {
		insertText(m_procView, m_procOutput, m_procMenudata->replaceSelection, m_procMenudata->selectInsertion);
	}
}

////////////////////////////// auxiliary //////////////////////////////

// action is finished, now insert some text
void UserMenu::insertText(KTextEditor::View *view, const QString &text, bool replaceSelection, bool selectInsertion)
{
	KILE_DEBUG() << "insert text from action: " << text;
	// metachars: %R - references (like \ref{%R}, \pageref{%R} ...)
	//            %T - citations  (like \cite{%T} ...)
	QString metachar,label;
	int actiontype =0;

	if ( text.contains("%R") )  {
		metachar = "%R";
		label = i18n("Label");
		actiontype = KileAction::FromLabelList;
	}
	else if ( text.contains("%T") )  {
		metachar = "%T";
		label = i18n("Reference");
		actiontype = KileAction::FromBibItemList;
	}
	if ( !metachar.isEmpty() ) {
		QStringList list = text.split(metachar);

		KileAction::InputTag tag(m_ki, i18n("Input Dialog"), QString(), KShortcut(), m_receiver, SLOT(insertTag(const KileAction::TagData&)), m_actioncollection,"tag_temporary_action", m_ki->mainWindow(), actiontype, list.at(0)+metachar, list.at(1), list.at(0).length(), 0, QString(), label);

		tag.activate(QAction::Trigger);
		return;
	}

	// metachars: %B - bullet
	//            %M - selected text (replaced by %C, if no selection is present)
	//            %C - place cursor
	//            %E - indent in environment
	QString ins = text;
	bool bullet = ins.contains("%B");

	// deselect and/or remove current selection
	if ( view->selection() ) {
		if ( ins.contains("%M") )  {
			if ( ins.contains("%M%C") ) {
				ins.replace("%M%C",view->selectionText());
			}
			else {
				ins.replace("%M",view->selectionText());
			}
		}
		if ( replaceSelection ) {
			view->removeSelectionText();
		}
		else {
			view->removeSelection();
		}
	}
	else {
		ins.replace("%M", QString());
	}
	KILE_DEBUG() << " ---> " << ins;

	// insert new text
	KTextEditor::Cursor cursor1 = view->cursorPosition();
	emit( sendText(ins) );

	// select inserted text
	if ( selectInsertion ) {
		KTextEditor::Cursor cursor2 = view->cursorPosition();
		view->setSelection(KTextEditor::Range(cursor1,cursor2));
	}

	// text with bullet metachar %B
	if ( bullet ) {
		view->setCursorPosition(cursor1);
		m_ki->editorExtension()->gotoBullet(false,view);
	}
}

////////////////////////////// auxiliary //////////////////////////////

bool UserMenu::str2bool(const QString &value)
{
	return ( value == "true" );
}


}

#include "usermenu.moc"
