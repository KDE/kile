/**************************************************************************
*   Copyright (C) 2007 by Michel Ludwig (michel.ludwig@kdemail.net)       *
*                 2011 by Felix Mauch (felix_mauch@web.de)                *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "widgets/helpconfigwidget.h"

#include <KFileDialog>
#include <KUrlCompletion>
#include <KMessageBox>

KileWidgetHelpConfig::KileWidgetHelpConfig(QWidget *parent) : QWidget(parent)
{
	setupUi(this);

	m_helpLocationButton->setIcon(KIcon("folder-open"));
	m_pbInformation->setIcon(KIcon("help-about"));

	connect(m_pbConfigure, SIGNAL(clicked()), this, SLOT(slotConfigure()));
	connect(m_helpLocationButton, SIGNAL(clicked()), this, SLOT(selectHelpLocation()));
	connect(m_pbInformation, SIGNAL(clicked()), this, SLOT(slotHelpInformation()));

	KUrlCompletion *dirCompletion = new KUrlCompletion();
	dirCompletion->setMode(KUrlCompletion::DirCompletion);
	kcfg_location->setCompletionObject(dirCompletion);
	kcfg_location->setAutoDeleteCompletionObject(true);
}

KileWidgetHelpConfig::~KileWidgetHelpConfig()
{
}

void KileWidgetHelpConfig::slotConfigure()
{
	m_help->userHelpDialog();
}

void KileWidgetHelpConfig::setHelp(KileHelp::Help *help)
{
	m_help = help;
}

void KileWidgetHelpConfig::selectHelpLocation()
{
	QString newLocation = KFileDialog::getExistingDirectory(kcfg_location->text(), this);
	if (!newLocation.isEmpty()) {
		kcfg_location->setText(newLocation);
	}
}

void KileWidgetHelpConfig::slotHelpInformation()
{
		QString message = i18n("<p>LaTeX distributions use very different locations for the base directory of documentation.<br>"
			"Here are some suggestions, which may help.</p>"
			"<ul>"
			"<li><i>Debian:&nbsp;</i> /usr/share/doc/texlive-doc</li>"
			"<li><i>Ubuntu:&nbsp;</i> /usr/share/doc/texlive-doc</li>"
			"<li><i>OpenSuse:&nbsp;</i> /usr/share/texmf/doc</li>"
			"<li><i>TexLive 2009:&nbsp;</i> /usr/share/doc/texlive-doc</li>"
			"<li><i>TexLive 2010 (TUG):&nbsp;</i> /usr/local/texlive/2010/texmf-dist/doc</li>"
			"<li><i>TexLive 2011 (TUG):&nbsp;</i> /usr/local/texlive/2011/texmf-dist/doc</li>"
			"</ul>"
			"<p>For documentation, the comprehensive links in the top-level file <code>doc.html</code> may be helpful,<br>"
			"if you use TeXLive 2010 or above (<code>/usr/local/texlive/2011/doc.html</code> or similar).<br>"
			"Consider to place it in the <it>User Help</it> section of the help menu.</p>"
		);

		KMessageBox::information(this,message,i18n("Latexmenu Dialog"));
}

#include "helpconfigwidget.moc"
