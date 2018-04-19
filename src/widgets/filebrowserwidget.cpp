/******************************************************************************************
    begin                : Wed Aug 14 2002
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                               2007-2014 by Michel Ludwig (michel.ludwig@kdemail.net)

from Kate (C) 2001 by Matt Newell

 ******************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// 2007-03-12 dani
//  - use KileDocument::Extensions

#include "widgets/filebrowserwidget.h"

#include <QAbstractItemView>
#include <QFocusEvent>
#include <QLayout>
#include <QToolTip>
#include <QVBoxLayout>

#include <KActionCollection>
#include <KActionMenu>
#include <KCharsets>
#include <KFilePlacesModel>
#include <KLocalizedString>
#include <KToolBar>
#include <KConfig>

#include "kileconfig.h"
#include "kiledebug.h"

namespace KileWidget {

FileBrowserWidget::FileBrowserWidget(KileDocument::Extensions *extensions, QWidget *parent)
    : QWidget(parent), m_extensions(extensions)
{
    m_configGroup = KConfigGroup(KSharedConfig::openConfig(),"FileBrowserWidget");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    m_toolbar = new KToolBar(this);
    m_toolbar->setMovable(false);
    m_toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_toolbar->setContextMenuPolicy(Qt::NoContextMenu);
    layout->addWidget(m_toolbar);

    KFilePlacesModel* model = new KFilePlacesModel(this);
    m_urlNavigator = new KUrlNavigator(model, QUrl::fromLocalFile(QDir::homePath()), this);
    layout->addWidget(m_urlNavigator);
    connect(m_urlNavigator, SIGNAL(urlChanged(const QUrl&)), SLOT(setDir(const QUrl&)));

    m_dirOperator = new KDirOperator(QUrl(), this);
    m_dirOperator->setViewConfig(m_configGroup);
    m_dirOperator->readConfig(m_configGroup);
    m_dirOperator->setView(KFile::Tree);
    m_dirOperator->setMode(KFile::Files);
    setFocusProxy(m_dirOperator);

    connect(m_urlNavigator, SIGNAL(urlChanged(const QUrl&)), m_dirOperator, SLOT(setFocus()));
    connect(m_dirOperator, SIGNAL(fileSelected(const KFileItem&)), this, SIGNAL(fileSelected(const KFileItem&)));
    connect(m_dirOperator, SIGNAL(urlEntered(const QUrl&)), this, SLOT(dirUrlEntered(const QUrl&)));



    setupToolbar();

    layout->addWidget(m_dirOperator);
    layout->setStretchFactor(m_dirOperator, 2);
    readConfig();
}

FileBrowserWidget::~FileBrowserWidget()
{
}

void FileBrowserWidget::readConfig()
{
    QString lastDir = KileConfig::lastDir();
    QFileInfo ldi(lastDir);
    if (!ldi.isReadable()) {
        KILE_DEBUG_MAIN << "lastDir is not readable";
        m_dirOperator->home();
    }
    else {
        setDir(QUrl::fromLocalFile(lastDir));
    }

    bool filterLatex = KileConfig::showLaTeXFilesOnly();
    toggleShowLaTeXFilesOnly(filterLatex);
}

void FileBrowserWidget::writeConfig()
{
    KileConfig::setLastDir(m_dirOperator->url().toLocalFile());
    m_dirOperator->writeConfig(m_configGroup);
}

void FileBrowserWidget::setupToolbar()
{
    KActionCollection *coll = m_dirOperator->actionCollection();
    m_toolbar->addAction(coll->action("back"));
    m_toolbar->addAction(coll->action("forward"));

    QAction *action = new QAction(this);
    action->setIcon(QIcon::fromTheme("document-open"));
    action->setText(i18n("Open selected"));
    connect(action, SIGNAL(triggered()), this, SLOT(emitFileSelectedSignal()));
    m_toolbar->addAction(action);

    QAction *showOnlyLaTexFilesAction = new QAction(this);
    showOnlyLaTexFilesAction->setText(i18n("Show LaTeX Files Only"));
    showOnlyLaTexFilesAction->setCheckable(true);
    showOnlyLaTexFilesAction->setChecked(KileConfig::showLaTeXFilesOnly());
    connect(showOnlyLaTexFilesAction, SIGNAL(triggered(bool)), this, SLOT(toggleShowLaTeXFilesOnly(bool)));

    // section for settings menu
    KActionMenu *optionsMenu = new KActionMenu(QIcon::fromTheme("configure"), i18n("Options"), this);
    optionsMenu->setDelayed(false);
    optionsMenu->addAction(m_dirOperator->actionCollection()->action("short view"));
    optionsMenu->addAction(m_dirOperator->actionCollection()->action("detailed view"));
    optionsMenu->addAction(m_dirOperator->actionCollection()->action("tree view"));
    optionsMenu->addAction(m_dirOperator->actionCollection()->action("detailed tree view"));
    optionsMenu->addSeparator();
    optionsMenu->addAction(showOnlyLaTexFilesAction);
    optionsMenu->addAction(m_dirOperator->actionCollection()->action("show hidden"));

    m_toolbar->addSeparator();
    m_toolbar->addAction(optionsMenu);
}

void FileBrowserWidget::toggleShowLaTeXFilesOnly(bool filter)
{
    KileConfig::setShowLaTeXFilesOnly(filter);

    if(filter) {
        // FileBrowserWidget filter for sidebar
        QString filter =  m_extensions->latexDocuments()
                          + ' ' + m_extensions->latexPackages()
                          + ' ' + m_extensions->bibtex()
                          + ' ' +  m_extensions->metapost();
        filter.replace('.', "*.");
        m_dirOperator->setNameFilter(filter);
    }
    else {
        m_dirOperator->clearFilter();
    }
    m_dirOperator->rereadDir();
}

QUrl FileBrowserWidget::currentUrl() const
{
    return m_dirOperator->url();
}

void FileBrowserWidget::dirUrlEntered(const QUrl &u)
{
    m_urlNavigator->setLocationUrl(u);
}

void FileBrowserWidget::setDir(const QUrl &url)
{
    m_dirOperator->setUrl(url, true);
}

void FileBrowserWidget::emitFileSelectedSignal()
{
    KFileItemList itemList = m_dirOperator->selectedItems();
    for(KFileItemList::iterator it = itemList.begin(); it != itemList.end(); ++it) {
        emit(fileSelected(*it));
    }

    m_dirOperator->view()->clearSelection();
}

}

