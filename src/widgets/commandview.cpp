/********************************************************************************
*   Copyright (C) 2018 by Michel Ludwig (michel.ludwig@kdemail.net)             *
*                 2009 by Holger Danielsson (holger.danielsson@versanet.de)     *
*********************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "commandview.h"

#include <KLocalizedString>
#include <KTextEditor/View>

#include <QComboBox>
#include <QVBoxLayout>

#include <algorithm>

#include "kileconfig.h"
#include "kileviewmanager.h"
#include "kiledebug.h"


namespace KileWidget {

//-------------------- CommandView --------------------

CommandView::CommandView(QWidget *parent)
    : QListWidget(parent)
{
    setViewMode(ListMode);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setSortingEnabled(true);
    setDragDropMode(NoDragDrop);

    connect(this, SIGNAL(itemActivated(QListWidgetItem*)), parent, SLOT(slotItemActivated(QListWidgetItem*)));
}

CommandView::~CommandView()
{
}

//-------------------- CommandViewToolBox --------------------

CommandViewToolBox::CommandViewToolBox(KileInfo *ki, QWidget *parent)
    : QWidget(parent), m_ki(ki)
{
    // we need a completion model for some auxiliary functions
    m_latexCompletionModel = new KileCodeCompletion::LaTeXCompletionModel(this,
            m_ki->codeCompletionManager(),
            m_ki->editorExtension());
    auto wrapperLayout = new QVBoxLayout;
    wrapperLayout->setContentsMargins(
        style()->pixelMetric(QStyle::PM_LayoutLeftMargin),
        style()->pixelMetric(QStyle::PM_LayoutTopMargin),
        style()->pixelMetric(QStyle::PM_LayoutRightMargin),
        style()->pixelMetric(QStyle::PM_LayoutBottomMargin)
    );
    m_cwlFilesComboBox = new QComboBox(this);
    wrapperLayout->addWidget(m_cwlFilesComboBox);
    connect(m_cwlFilesComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
    [=](int index) {
        populateCommands(m_cwlFilesComboBox->itemData(index).toString());
    });

    m_commandView = new CommandView(this);
    m_commandView->setProperty("_breeze_borders_sides", QVariant::fromValue(QFlags{Qt::TopEdge}));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins({});
    layout->addLayout(wrapperLayout);
    layout->addWidget(m_commandView);

    clearItems();
}

CommandViewToolBox::~CommandViewToolBox()
{
}

void CommandViewToolBox::readCommandViewFiles()
{
    clearItems();

    KileCodeCompletion::Manager *manager = m_ki->codeCompletionManager();

    QStringList validCwlFiles;

    const QStringList files = KileConfig::completeTex();
    for (const QString& file : files) {
        // check, if the wordlist has to be read
        const QString validCwlFile = manager->validCwlFile(file);

        if(!validCwlFile.isEmpty()) {
            validCwlFiles << validCwlFile;
        }
    }

    std::sort(validCwlFiles.begin(), validCwlFiles.end());

    for(const QString &cwlFile : std::as_const(validCwlFiles)) {
        m_cwlFilesComboBox->addItem(cwlFile, cwlFile);
    }

    if(m_cwlFilesComboBox->count() > 0) {
        m_commandView->setEnabled(true);
        m_cwlFilesComboBox->setEnabled(true);
        m_cwlFilesComboBox->setCurrentIndex(0);
    }
}

void CommandViewToolBox::populateCommands(const QString& cwlFile)
{
    KileCodeCompletion::Manager *manager = m_ki->codeCompletionManager();

    m_commandView->clear();

    const QStringList wordlist = manager->readCWLFile("tex/" + cwlFile + ".cwl");

    for(const QString &string : wordlist) {
        m_commandView->addItem(string);
    }
}

void CommandViewToolBox::clearItems()
{
    m_commandView->clear();
    m_cwlFilesComboBox->clear();

    m_commandView->setEnabled(false);
    m_cwlFilesComboBox->setEnabled(false);
}

void CommandViewToolBox::slotItemActivated(QListWidgetItem *item)
{
    KTextEditor::View *view = m_ki->viewManager()->currentTextView();
    if(view) {
        KTextEditor::Cursor cursor = view->cursorPosition();

        //insert text
        int xpos,ypos;
        QString text = m_latexCompletionModel->filterLatexCommand(item->text(),ypos,xpos);
        if(!text.isEmpty()) {
            Q_EMIT(sendText(text));

            // place cursor
            if(KileConfig::completeCursor() && (xpos > 0 || ypos > 0) ) {
                view->setCursorPosition(KTextEditor::Cursor(cursor.line() + (ypos >= 0 ? ypos : 0),
                                        cursor.column() + (xpos >= 0 ? xpos : 0)));
            }
        }
    }
}

}
