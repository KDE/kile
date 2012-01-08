/******************************************************************************
  Copyright (C) 2011-2012 by Holger Danielsson (holger.danielsson@versanet.de)
 ******************************************************************************/

#include "dialogs/scriptshortcutdialog.h"

#include <QKeySequence>

#include <KActionCollection>
#include <KXMLGUIClient>
#include <KXMLGUIFactory>

#include "kiledebug.h"

namespace KileDialog {


ScriptShortcutDialog::ScriptShortcutDialog(QWidget *parent, KileInfo *ki, const QString &shortcut)
	: KDialog(parent)
{
	setCaption(i18n("New Key Sequence"));
	setModal(true);
	setButtons(Ok | Cancel);
	setDefaultButton(Ok);
	showButtonSeparator(true);

	QWidget *page = new QWidget(this);
	setMainWidget(page);
	m_scriptShortcutDialog.setupUi(page);

	if ( shortcut.isEmpty() ) {
		m_scriptShortcutDialog.m_keyChooser->clearKeySequence();
	}
	else {
		m_scriptShortcutDialog.m_keyChooser->setKeySequence( QKeySequence(shortcut) );
	}

	// search for all action collections (needed for shortcut conflicts)
	QList<KActionCollection *> allCollections;
	foreach ( KXMLGUIClient *client, ki->mainWindow()->guiFactory()->clients() ) {
		allCollections += client->actionCollection();
	}
	m_scriptShortcutDialog.m_keyChooser->setCheckActionCollections(allCollections);

}

QString ScriptShortcutDialog::keySequence()
{
	return m_scriptShortcutDialog.m_keyChooser->keySequence().toString(QKeySequence::PortableText);
}



}

#include "scriptshortcutdialog.moc"
