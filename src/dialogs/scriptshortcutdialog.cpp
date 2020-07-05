/******************************************************************************
  Copyright (C) 2011-2012 by Holger Danielsson (holger.danielsson@versanet.de)
 ******************************************************************************/

#include "dialogs/scriptshortcutdialog.h"

#include <QKeySequence>

#include <KActionCollection>
#include <KXMLGUIClient>
#include <KXMLGUIFactory>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "kiledebug.h"

namespace KileDialog {


ScriptShortcutDialog::ScriptShortcutDialog(QWidget *parent, KileInfo *ki, int type, const QString &sequence)
    : QDialog(parent)
{
    setWindowTitle(i18n("New Key Sequence"));
    setModal(true);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    okButton->setDefault(true);

    QWidget *page = new QWidget(this);
    mainLayout->addWidget(page);
    m_scriptShortcutDialog.setupUi(page);
    mainLayout->addWidget(buttonBox);

    m_scriptShortcutDialog.m_rbKeySequence->setWhatsThis(i18n("Use a key sequence written in the editor to execute a script."));
    m_scriptShortcutDialog.m_rbShortcut->setWhatsThis(i18n("Use a shortcut to execute a script."));

    if(type == KileScript::Script::KEY_SHORTCUT) {
        m_scriptShortcutDialog.m_rbShortcut->setChecked(true);
        if(sequence.isEmpty()) {
            m_scriptShortcutDialog.m_keyChooser->clearKeySequence();
        }
        else {
            m_scriptShortcutDialog.m_keyChooser->setKeySequence( QKeySequence(sequence) );
        }
    }
    else {
        m_scriptShortcutDialog.m_rbKeySequence->setChecked(true);
        m_scriptShortcutDialog.m_leKeySequence->setText(sequence);
    }
    slotUpdate();

    // search for all action collections (needed for shortcut conflicts)
    QList<KActionCollection *> allCollections;
    foreach ( KXMLGUIClient *client, ki->mainWindow()->guiFactory()->clients() ) {
        allCollections += client->actionCollection();
    }
    m_scriptShortcutDialog.m_keyChooser->setCheckActionCollections(allCollections);

    connect(m_scriptShortcutDialog.m_rbKeySequence, SIGNAL(clicked()), this, SLOT(slotUpdate()));
    connect(m_scriptShortcutDialog.m_rbShortcut, SIGNAL(clicked()), this, SLOT(slotUpdate()));

}

int ScriptShortcutDialog::sequenceType()
{
    return (m_scriptShortcutDialog.m_rbShortcut->isChecked()) ?  KileScript::Script::KEY_SHORTCUT : KileScript::Script::KEY_SEQUENCE;
}

QString ScriptShortcutDialog::sequenceValue()
{
    return m_scriptShortcutDialog.m_rbShortcut->isChecked()
           ? m_scriptShortcutDialog.m_keyChooser->keySequence().toString(QKeySequence::PortableText)
           : m_scriptShortcutDialog.m_leKeySequence->text();
}

void ScriptShortcutDialog::slotUpdate()
{
    bool state = ( m_scriptShortcutDialog.m_rbKeySequence->isChecked() ) ? true : false;
    m_scriptShortcutDialog.m_lbKeySequence->setEnabled(state);
    m_scriptShortcutDialog.m_leKeySequence->setEnabled(state);
    m_scriptShortcutDialog.m_lbShortcut->setEnabled(!state);
    m_scriptShortcutDialog.m_keyChooser->setEnabled(!state);

    if(state) {
        m_scriptShortcutDialog.m_leKeySequence->setFocus();
    }
    else {
        m_scriptShortcutDialog.m_keyChooser->setFocus();
    }
}


}

