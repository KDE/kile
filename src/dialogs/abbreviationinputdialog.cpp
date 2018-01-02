/**************************************************************************************
    begin                : Feb 24 2007
    copyright            : 2007 by Holger Danielsson (holger.danielsson@versanet.de)
                           2015 by Andreas Cord-Landwerh (cordlandwehr@kde.org)
***************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dialogs/abbreviationinputdialog.h"

#include "kiledebug.h"

#include <KLocalizedString>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QRegExp>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QPushButton>

namespace KileDialog {

//////////////////// add/edit abbreviation ////////////////////

//TODO dialog has non-standard logic
// calling code is reacting on the result (i.e., if dialog accpepts or not) and then uses the
// changed data accordingling; this should be changed

AbbreviationInputDialog::AbbreviationInputDialog(KileWidget::AbbreviationView *listview, QTreeWidgetItem *item, int mode, const char *name)
    : QDialog(listview)
    , m_listview(listview)
    , m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel))
    , m_abbrevItem(item)
    , m_mode(mode)
{
    setWindowTitle(i18n("Add Abbreviation"));
    setModal(true);
    setObjectName(name);
    setMinimumWidth(350);

    QFormLayout *mainLayout = new QFormLayout;
    setLayout(mainLayout);

    if(m_mode == KileWidget::AbbreviationView::ALVedit) {
        setWindowTitle( i18n("Edit Abbreviation") );
        m_abbrev = m_abbrevItem->text(KileWidget::AbbreviationView::ALVabbrev);
        m_expansion = m_abbrevItem->text(KileWidget::AbbreviationView::ALVexpansion);
    }

    m_leAbbrev = new QLineEdit(m_abbrev, this);
    m_leExpansion = new QLineEdit(m_expansion, this);
    QLabel *labelAbbreviation = new QLabel(i18n("&Abbreviation:"), this);
    labelAbbreviation->setBuddy(m_leAbbrev);
    QLabel *labelExpanded = new QLabel(i18n("&Expanded Text:"), this);
    labelExpanded->setBuddy(m_leExpansion);

    mainLayout->addRow(labelAbbreviation, m_leAbbrev);
    mainLayout->addRow(labelExpanded, m_leExpansion);

    QRegExp reg("[a-zA-Z0-9]+");
    QRegExpValidator *abbrevValidator = new QRegExpValidator(reg, this);
    m_leAbbrev->setValidator(abbrevValidator);

    connect(m_leAbbrev, &QLineEdit::textChanged, this, &AbbreviationInputDialog::onTextChanged);
    connect(m_leExpansion, &QLineEdit::textChanged, this, &AbbreviationInputDialog::onTextChanged);

    onTextChanged(QString());
    m_leAbbrev->setFocus();

    QPushButton *okButton = m_buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(m_buttonBox);
    okButton->setDefault(true);
}

AbbreviationInputDialog::~AbbreviationInputDialog()
{
}

void AbbreviationInputDialog::abbreviation(QString &abbrev, QString &expansion)
{
    abbrev = m_leAbbrev->text();
    expansion = m_leExpansion->text().trimmed();
}

void AbbreviationInputDialog::onTextChanged(const QString &)
{
    bool state = (m_mode == KileWidget::AbbreviationView::ALVadd)
                 ? !m_listview->findAbbreviation( m_leAbbrev->text()) : true;
    state = state && !m_leAbbrev->text().isEmpty() && !m_leExpansion->text().isEmpty();
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(state);
}
}

