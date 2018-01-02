/***************************************************************************
    begin                : Tuesday Nov 15 2005
    copyright            : (C) 2005 by Holger Danielsson
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

#include "widgets/statisticswidget.h"

#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QVariant>
#include <QVBoxLayout>

#include <QDialog>
#include <KLocalizedString>
#include <KConfigGroup>

#include "kiledebug.h"

namespace KileWidget {

StatisticsWidget::StatisticsWidget(QWidget* parent, const char *name, Qt::WindowFlags fl)
    : QWidget(parent, fl)
{
    setObjectName(name);
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setMargin(0);
//TODO PORT QT5 	vbox->setSpacing(QDialog::spacingHint());
    setLayout(vbox);

    // characters groupbox
    m_charactersGroup = new QGroupBox(i18n("Characters"), this);
    chargrouplayout = new QGridLayout();
//TODO PORT QT5 	chargrouplayout->setMargin(QDialog::marginHint());
//TODO PORT QT5 	chargrouplayout->setSpacing(QDialog::spacingHint());
    chargrouplayout->setAlignment(Qt::AlignTop);
    m_charactersGroup->setLayout(chargrouplayout);

    m_wordCharText = new QLabel(i18n("Words and numbers:"), m_charactersGroup);
    m_commandCharText = new QLabel(i18n("LaTeX commands and environments:"), m_charactersGroup);
    m_whitespaceCharText = new QLabel(i18n("Punctuation, delimiter and whitespaces:"), m_charactersGroup);
    m_totalCharText = new QLabel(i18n("Total characters:"), m_charactersGroup);
    m_wordChar = new QLabel(m_charactersGroup);
    m_wordChar->setObjectName("m_wordChar");
    m_commandChar = new QLabel(m_charactersGroup);
    m_commandChar->setObjectName("m_commandChar");
    m_whitespaceChar = new QLabel(m_charactersGroup);
    m_whitespaceChar->setObjectName("m_whitespaceChar");
    m_totalChar = new QLabel(m_charactersGroup);
    m_totalChar->setObjectName("m_totalChar");

    QFrame *charframe = new QFrame(m_charactersGroup);
    charframe->setFrameShape(QFrame::HLine);
    charframe->setFrameShadow(QFrame::Sunken);
    charframe->setLineWidth(1);

    chargrouplayout->addWidget(m_wordCharText, 0, 0);
    chargrouplayout->addWidget(m_commandCharText, 1, 0);
    chargrouplayout->addWidget(m_whitespaceCharText, 2, 0);
    chargrouplayout->addWidget(m_totalCharText, 4, 0);
    chargrouplayout->addWidget(m_wordChar, 0, 2, Qt::AlignRight);
    chargrouplayout->addWidget(m_commandChar, 1, 2, Qt::AlignRight);
    chargrouplayout->addWidget(m_whitespaceChar, 2, 2, Qt::AlignRight);
    chargrouplayout->addWidget(charframe, 3, 3, 1, 2);
    chargrouplayout->addWidget(m_totalChar, 4, 2, Qt::AlignRight);
    chargrouplayout->setColumnMinimumWidth(1, 16);
    chargrouplayout->setColumnMinimumWidth(3, 1);
    chargrouplayout->setColumnStretch(3, 1);

    // string groupbox
    m_stringsGroup = new QGroupBox(i18n("Strings"), this);
    stringgrouplayout = new QGridLayout();
//TODO PORT QT5 	stringgrouplayout->setMargin(QDialog::marginHint());
//TODO PORT QT5 	stringgrouplayout->setSpacing(QDialog::spacingHint());
    stringgrouplayout->setAlignment(Qt::AlignTop);
    m_stringsGroup->setLayout(stringgrouplayout);

    m_wordStringText = new QLabel(i18n("Words:"), m_stringsGroup);
    m_commandStringText = new QLabel(i18n("LaTeX commands:"), m_stringsGroup);
    m_environmentStringText = new QLabel(i18n("LaTeX environments:"), m_stringsGroup);
    m_totalStringText = new QLabel(i18n("Total strings:"), m_stringsGroup);
    m_wordString = new QLabel(m_stringsGroup);
    m_wordString->setObjectName("m_wordString");
    m_commandString = new QLabel(m_stringsGroup);
    m_commandString->setObjectName("m_commandStringText");
    m_environmentString = new QLabel(m_stringsGroup);
    m_environmentString->setObjectName("m_environmentStringText");
    m_totalString = new QLabel(m_stringsGroup);
    m_totalString->setObjectName("m_totalStringText");

    QFrame *stringframe = new QFrame(m_stringsGroup);
    stringframe->setFrameShape(QFrame::HLine);
    stringframe->setFrameShadow(QFrame::Sunken);
    stringframe->setLineWidth(1);

    stringgrouplayout->addWidget(m_wordStringText, 0, 0);
    stringgrouplayout->addWidget(m_commandStringText, 1, 0);
    stringgrouplayout->addWidget(m_environmentStringText, 2, 0);
    stringgrouplayout->addWidget(m_totalStringText, 4, 0);
    stringgrouplayout->addWidget(m_wordString, 0, 2, Qt::AlignRight);
    stringgrouplayout->addWidget(m_commandString, 1, 2, Qt::AlignRight);
    stringgrouplayout->addWidget(m_environmentString, 2, 2, Qt::AlignRight);
    stringgrouplayout->addWidget(stringframe, 3, 3, 1, 2);
    stringgrouplayout->addWidget(m_totalString, 4, 2, Qt::AlignRight);
    stringgrouplayout->setColumnMinimumWidth(1, 16);
    stringgrouplayout->setColumnMinimumWidth(3, 1);
    stringgrouplayout->setColumnStretch(3, 1);

    m_commentAboutHelp = new QLabel(parent);
    m_warning =  new QLabel(parent);

    vbox->addWidget(m_charactersGroup);
    vbox->addWidget(m_stringsGroup);
    vbox->addSpacing(12);
    vbox->addWidget(m_commentAboutHelp);
    vbox->addWidget(m_warning);
    vbox->addStretch(1);

    int w = m_commandCharText->sizeHint().width();
    if (m_whitespaceCharText->sizeHint().width() > w) {
        w = m_whitespaceCharText->sizeHint().width();
    }
    stringgrouplayout->setColumnMinimumWidth(0, w);

}

StatisticsWidget::~StatisticsWidget()
{
}

void StatisticsWidget::updateColumns()
{
    int w = m_totalChar->sizeHint().width();
    if (m_totalString->sizeHint().width() > w) {
        w = m_totalString->sizeHint().width();
    }
    chargrouplayout->setColumnMinimumWidth(2, w);
    stringgrouplayout->setColumnMinimumWidth(2, w);
}

}

