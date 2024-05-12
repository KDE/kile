/**************************************************************************
*   Copyright (C) 2007 by Michel Ludwig (michel.ludwig@kdemail.net)       *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "latexconfigwidget.h"
#include "kileconfig.h"
#include "kiledebug.h"
#include "kileinfo.h"

#include "editorextension.h"

#include "dialogs/latexcommanddialog.h"

KileWidgetLatexConfig::KileWidgetLatexConfig(KConfig* config, KileInfo* ki, QWidget* parent)
    : QWidget(parent),
      m_config(config),
      m_ki(ki)
{
    setupUi(this);
    connect(m_pbCommands, SIGNAL(clicked()), this, SLOT(slotConfigure()));

    m_modifiers[Qt::ShiftModifier] = i18n("Shift");
    m_modifiers[Qt::ControlModifier] = i18n("Ctrl");
    m_modifiers[Qt::AltModifier] = i18n("Alt");
    m_modifiers[Qt::MetaModifier] = i18n("Meta");
    m_modifiers[Qt::ControlModifier | Qt::ShiftModifier] = i18n("Ctrl + Shift");
    m_modifiers[Qt::AltModifier | Qt::ControlModifier] = i18n("Alt + Ctrl");
    m_modifiers[Qt::MetaModifier | Qt::ControlModifier] = i18n("Meta + Ctrl");

// On macOS, the ControlModifier value corresponds to the Command keys on the
// keyboard, and the MetaModifier value corresponds to the Control keys.
#ifdef Q_OS_DARWIN
    m_modifiers[Qt::ControlModifier] = i18n("Cmd");
    m_modifiers[Qt::MetaModifier] = i18n("Ctrl");
    m_modifiers[Qt::ControlModifier | Qt::ShiftModifier] = i18n("Cmd + Shift");
    m_modifiers[Qt::AltModifier | Qt::ControlModifier] = i18n("Alt + Cmd");
    m_modifiers[Qt::MetaModifier | Qt::ControlModifier] = i18n("Ctrl + Cmd");
#endif
}

KileWidgetLatexConfig::~KileWidgetLatexConfig()
{
}

void KileWidgetLatexConfig::slotConfigure()
{
    KileDialog::LatexCommandsDialog *dlg = new KileDialog::LatexCommandsDialog(m_config, m_commands, this);
    dlg->exec();
    delete dlg;
}

void KileWidgetLatexConfig::initModifierComboBox(QComboBox* c, int initial)
{
    int selectedIndex = 0;
    int currentIndex = 0;
    for(auto i = m_modifiers.cbegin(), end = m_modifiers.cend(); i != end; ++i, ++currentIndex) {
        c->addItem(i.value(), i.key());
        if(i.key() == initial) {
            selectedIndex = currentIndex;
        }
    }

    c->setCurrentIndex(selectedIndex);
}

void KileWidgetLatexConfig::readConfig()
{
    kcfg_DoubleQuotes->addItems(m_ki->editorExtension()->doubleQuotesListI18N());
    m_commands = m_ki->latexCommands();

    initModifierComboBox(m_envEventSelectModifier, KileConfig::envEventSelectModifier());
    initModifierComboBox(m_envEventSearchModifier, KileConfig::envEventSearchModifier());
}

void KileWidgetLatexConfig::writeConfig()
{
    KileConfig::setEnvEventSelectModifier(m_envEventSelectModifier->currentData().toInt());
    KileConfig::setEnvEventSearchModifier(m_envEventSearchModifier->currentData().toInt());
}
