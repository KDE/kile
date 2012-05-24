/***************************************************************************************************
    begin                : Mon Dec 22 2003
    copyright            : (C) 2001 - 2003 by Brachet Pascal
                               2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                               2007-2011 by Michel Ludwig (michel.ludwig@kdemail.net)
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

#include <QFileInfo>

#include <QShowEvent>
#include <QVBoxLayout>

#include <KLocale>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KService>
#include <KUrl>

#include <kshell.h>

#include <kparts/part.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

namespace KileWidget
{
	Konsole::Konsole(KileInfo * info, QWidget *parent) :
		QFrame(parent),
		m_part(NULL),
		m_term(NULL),
		m_ki(info)
	{
		setLayout(new QVBoxLayout(this));
		layout()->setMargin(0);
		setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
		spawn();
	}

	Konsole::~Konsole()
	{
	}

	void Konsole::spawn()
	{
		KILE_DEBUG() << "void Konsole::spawn()";
                KPluginFactory* factory = 0;
                KService::Ptr service = KService::serviceByDesktopName("konsolepart");
                if (service) {
                    factory = KPluginLoader(service->library()).factory();
                }

		if(!factory) {
			KILE_DEBUG() << "No factory for konsolepart";
			return;
		}

		// the catalogue for translations is added by the Konsole part constructor already
		m_part = static_cast<KParts::ReadOnlyPart*>(factory->create<QObject>(this, this));
		if(!m_part) {
			return;
		}

		m_term = qobject_cast<TerminalInterface*>(m_part);
		if(!m_term){
			KILE_DEBUG() << "Found no TerminalInterface";
			return;
		}

		layout()->addWidget(m_part->widget());
		setFocusProxy(m_part->widget());
		connect(m_part, SIGNAL(destroyed()), this, SLOT(slotDestroyed()));

		m_term->showShellInDir(QString());
	}


	void Konsole::sync()
	{
		if(!KileConfig::syncConsoleDirWithTabs()) {
			return;
		}

		KTextEditor::Document *doc = m_ki->activeTextDocument();
		KTextEditor::View *view = NULL;

		if(doc) {
			view = doc->views().first();
		}

		if(view) {
			QString finame;
			KUrl url = view->document()->url();

			if(url.path().isEmpty()) {
				return;
			}

			QFileInfo fic(url.directory());
			if(fic.isReadable()) {
				setDirectory(url.directory());
			}
		}
	}

	void Konsole::setDirectory(const QString &directory)
	{
		//FIXME: KonsolePart should be extended in such a way that it isn't necessary
		//       anymore to send 'cd' commands
		if(m_term && !directory.isEmpty() && directory != m_currentDir) {
			m_term->sendInput("cd " + KShell::quoteArg(directory) + '\n');
			m_term->sendInput("clear\n");
			m_currentDir = directory;
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
		// there is no need to remove the widget from the layout as this is done
		// automatically when the widget is destroyed
		m_part = NULL;
		m_term = NULL;
		spawn();
	}
}

#include "konsolewidget.moc"
