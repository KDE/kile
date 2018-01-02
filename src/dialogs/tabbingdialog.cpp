/***************************************************************************
    begin                : dim jui 14 2002
    copyright            : (C) 2002 - 2003 by Pascal Brachet, 2003 Jeroen Wijnhout
    email                : Jeroen.Wijnhout@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dialogs/tabbingdialog.h"
#include "editorextension.h"

#include <KLocalizedString>
#include <KConfigGroup>
#include <QDialogButtonBox>

namespace KileDialog
{

QuickTabbing::QuickTabbing(KConfig *config, KileInfo *info, QWidget *parent,
                           const char *name, const QString &caption)
    : Wizard(config, parent, name, caption)
    , m_info(info)
{
    QWidget *page = new QWidget(this);
    m_tabbingDialog.setupUi(page);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(page);
    mainLayout->addWidget(buttonBox());

    connect(this, &Wizard::accepted, this, &QuickTabbing::onAccepted);
}

QuickTabbing::~QuickTabbing()
{
}

void QuickTabbing::onAccepted()
{
    int x = m_tabbingDialog.m_spCols->value();
    int y = m_tabbingDialog.m_spRows->value();
    QString s = m_tabbingDialog.m_leSpacing->text();
    QString indent = m_info->editorExtension()->autoIndentEnvironment();

    m_td.tagBegin = "\\begin{tabbing}\n";
    m_td.tagBegin += indent;

    for (int j = 1; j < x ; ++j) {
        m_td.tagBegin += "\\hspace{" + s + "}\\=";
    }

    m_td.tagBegin += "\\kill\n";

    for (int i = 0; i < y - 1; ++i) {
        m_td.tagBegin += indent;
        for (int j = 1; j < x; ++j)
            m_td.tagBegin += " \\> ";
        m_td.tagBegin += "\\\\ \n";
    }

    m_td.tagBegin += indent;
    for (int j = 1; j < x; ++j) {
        m_td.tagBegin += " \\> ";
    }

    m_td.tagEnd = "\n\\end{tabbing}";
    m_td.dy = 1;
    m_td.dx = indent.length();

}
}
