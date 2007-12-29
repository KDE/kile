/***************************************************************************************************
    begin                : Mon Dec 22 2003
    copyright            : (C) 2001 - 2003 by Brachet Pascal
                               2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                               2007 by Michel Ludwig (michel.ludwig@kdemail.net)
 ***************************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "widgets/konsolewidget.h"

#include "kileinfo.h"

#include <qfileinfo.h>

#include <QShowEvent>
#include <QVBoxLayout>

#include <KLibLoader>
#include <KLocale>
#include <KPluginFactory>
#include <KUrl>

#include <kde_terminal_interface.h>

#include <kparts/part.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include "kileuntitled.h"

namespace KileWidget
{
	Konsole::Konsole(KileInfo * info, QWidget *parent) :
		QWidget(parent),
		m_part(NULL),
		m_ki(info)
	{
		setLayout(new QVBoxLayout(this));
		spawn();
	}

	Konsole::~Konsole()
	{
	}

	void Konsole::spawn()
	{
		KPluginFactory *factory = KLibLoader::self()->factory("libkonsolepart");

		if(!factory) {
			return;
		}

		m_part = static_cast<KParts::ReadOnlyPart*>(factory->create<QObject>(this, this));

		if(!m_part) {
			return;
		}

		layout()->addWidget(m_part->widget());

		// start the konsole part
		qobject_cast<TerminalInterface*>(m_part)->showShellInDir(QString());

		setFocusProxy(m_part->widget());

		if (m_part->widget()->inherits("QFrame")) {
			((QFrame*)m_part->widget())->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
		}

		connect(m_part, SIGNAL(destroyed()), this, SLOT(slotDestroyed()));
	}


	void Konsole::sync()
	{
		KTextEditor::Document *doc = m_ki->activeTextDocument();
		KTextEditor::View *view = NULL;

		if(doc) {
			view = doc->views().first();
		}

		if(view) {
			QString finame;
			KUrl url = view->document()->url();

			if(url.path().isEmpty() || KileUntitled::isUntitled(url.path())) {
				return;
			}

			QFileInfo fic(url.directory());
			if(fic.isReadable()) {
				setDirectory(url.directory());
				activate();
			}

			view->setFocus();
		}
	}

	void Konsole::setDirectory(const QString &dirname)
	{
		if(m_part) {
			KUrl url(dirname);
			if (m_part->url() != url) {
				m_part->openUrl(url);
			}
		}
	}

	void Konsole::showEvent(QShowEvent *ev)
	{
		QWidget::showEvent(ev);
		activate();
	}

	void Konsole::activate()
	{
		if (m_part) {
			m_part->widget()->show();
			m_part->widget()->setFocus();
		}
	}

	void Konsole::slotDestroyed ()
	{
		m_part = NULL;
		spawn();
	}
}

#include "konsolewidget.moc"
