/***************************************************************************
    begin                : Mon Dec 22 2003
    copyright            : (C) 2001 - 2003 by Brachet Pascal, 2003 by Jeroen Wijnhout
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

#include "kilekonsolewidget.h"
#include "kileinfo.h"

#include <qfileinfo.h>
#include <q3frame.h>
//Added by qt3to4:
#include <QShowEvent>

#include <klocale.h>
#include <klibloader.h>
#include <kurl.h>
#include <kparts/part.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include "kileuntitled.h"

namespace KileWidget
{
	Konsole::Konsole(KileInfo * info, QWidget *parent, const char *name) : 
		Q3VBox(parent, name),
		m_bPresent(false),
		m_ki(info)
	{
		spawn();
	}

	Konsole::~Konsole()
	{
	}

	void Konsole::spawn()
	{
		KLibFactory *factory = KLibLoader::self()->factory("libkonsolepart");

		if (!factory) return;
		m_part = (KParts::ReadOnlyPart *) factory->create(this);

		if (!m_part) return;

		if (m_part->widget()->inherits("QFrame"))
			((Q3Frame*)m_part->widget())->setFrameStyle(Q3Frame::Panel|Q3Frame::Sunken);

		m_bPresent=true;
		connect ( m_part, SIGNAL(destroyed()), this, SLOT(slotDestroyed()) );
	
		m_part->widget()->show();
		show();
	}


	void Konsole::sync()
	{
		KTextEditor::Document *doc = m_ki->activeTextDocument();
		KTextEditor::View *view = 0;

		if (doc)
			view = doc->views().first();

		if (view)
		{
			QString finame;
			KUrl url = view->getDoc()->url();

			if ( url.path().isEmpty() || KileUntitled::isUntitled(url.path()) ) return;

			QFileInfo fic(url.directory());
			if ( fic.isReadable() )
			{
				setDirectory(url.directory());
				activate();
			}

			view->setFocus();
		}
	}

	void Konsole::setDirectory(const QString &dirname)
	{
		if (m_bPresent)
		{
			KUrl url(dirname);
			if (m_part->url() != url)
				m_part->openURL(url);
		}
	}

	void Konsole::showEvent(QShowEvent *ev)
	{
		QWidget::showEvent(ev);
		activate();
	}

	void Konsole::activate()
	{
		if (m_bPresent)
		{
			m_part->widget()->show();
			this->setFocusProxy(m_part->widget());
			m_part->widget()->setFocus();
		}
	}

	void Konsole::slotDestroyed ()
	{
		m_bPresent=false;
		spawn();
	}
}

#include "kilekonsolewidget.moc"
