/******************************************************************************************
    begin                : Wed Aug 14 2002
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                               2007, 2008 by Michel Ludwig (michel.ludwig@kdemail.net)

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
#include <KCharsets>
#include <KLocale>
#include <KToolBar>

#include "kileconfig.h"
#include "kiledebug.h"

namespace KileWidget {

FileBrowserWidget::FileBrowserWidget(KileDocument::Extensions *extensions, QWidget *parent, const char *name) : QWidget(parent)
{
	setObjectName(name);
	QVBoxLayout* layout = new QVBoxLayout(this);
	setLayout(layout);

	KToolBar *toolbar = new KToolBar(this);
	toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
	toolbar->setIconDimensions(KIconLoader::SizeSmall);
	layout->addWidget(toolbar);

	m_pathComboBox = new KUrlComboBox(KUrlComboBox::Directories, this);
	m_pathComboBox->setEditable(true);
	m_pathComboBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
	m_urlCompletion = new KUrlCompletion(KUrlCompletion::DirCompletion);
	m_pathComboBox->setCompletionObject(m_urlCompletion);
	layout->addWidget(m_pathComboBox);
	connect(m_pathComboBox, SIGNAL(urlActivated(const KUrl&)), this, SLOT(setDir(const KUrl&)));
	connect(m_pathComboBox, SIGNAL(returnPressed(const QString&)), this, SLOT(comboBoxReturnPressed(const QString&)));

	m_dirOperator = new KDirOperator(KUrl(), this);
	m_dirOperator->setView(KFile::Simple);
	m_dirOperator->setMode(KFile::Files);
	connect(m_dirOperator, SIGNAL(fileSelected(const KFileItem&)), this, SIGNAL(fileSelected(const KFileItem&)));
	connect(m_dirOperator, SIGNAL(urlEntered(const KUrl&)), this, SLOT(dirUrlEntered(const KUrl&)));

	// FileBrowserWidget filter for sidebar 
	QString filter =  extensions->latexDocuments() 
	                    + ' ' + extensions->latexPackages() 
	                    + ' ' + extensions->bibtex() 
	                    + ' ' +  extensions->metapost();
	filter.replace(".", "*.");
	m_dirOperator->setNameFilter(filter);

	KActionCollection *coll = m_dirOperator->actionCollection();
#ifdef __GNUC__
#warning Check whether these shortcut settings are needed!
#endif
//FIXME: port for KDE4
/*
	// some shortcuts of diroperator that clashes with Kate
	coll->action("delete")->setShortcut(KShortcut(Qt::ALT + Qt::Key_Delete));
	coll->action("reload")->setShortcut(KShortcut(Qt::ALT + Qt::Key_F5));
	coll->action("back")->setShortcut(KShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_Left));
	coll->action("forward")->setShortcut(KShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_Right));
	// some consistency - reset up for m_dirOperator too
	coll->action("up")->setShortcut(KShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_Up));
	coll->action("home")->setShortcut(KShortcut(Qt::CTRL + Qt::ALT + Qt::Key_Home));
*/
	toolbar->addAction(coll->action("home"));
	toolbar->addAction(coll->action("up"));
	toolbar->addAction(coll->action("back"));
	toolbar->addAction(coll->action("forward"));

	KAction *action = new KAction(this);
	action->setIcon(SmallIcon("document-open"));
	action->setText(i18n("Open selected"));
	connect(action, SIGNAL(triggered()), this, SLOT(emitFileSelectedSignal()));
	toolbar->addAction(action);

	layout->addWidget(m_dirOperator);
	layout->setStretchFactor(m_dirOperator, 2);
}

FileBrowserWidget::~FileBrowserWidget()
{
	delete m_urlCompletion;
}

void FileBrowserWidget::readConfig()
{
	QString lastDir = KileConfig::lastDir();
	QFileInfo ldi(lastDir);
	if (!ldi.isReadable()) {
		m_dirOperator->home();
	}
	else {
		setDir(KUrl::fromPathOrUrl(lastDir));
	}
}

void FileBrowserWidget::writeConfig()
{
	KileConfig::setLastDir(m_dirOperator->url().path());
}

KUrl FileBrowserWidget::currentUrl() const
{
	return m_dirOperator->url();
}

void FileBrowserWidget::comboBoxReturnPressed(const QString& u)
{
	m_dirOperator->setFocus();
	setDir(KUrl(u));
}

void FileBrowserWidget::dirUrlEntered(const KUrl& u)
{
	m_pathComboBox->removeUrl(u);
	QStringList urls = m_pathComboBox->urls();
	urls.prepend(u.url());
	while(urls.count() >= m_pathComboBox->maxItems()) {
		urls.removeAll(urls.last());
	}
	m_pathComboBox->setUrls(urls);
}

void FileBrowserWidget::focusInEvent(QFocusEvent* /* e */)
{
	m_dirOperator->setFocus();
}

void FileBrowserWidget::setDir(const KUrl& url)
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

#include "filebrowserwidget.moc"
