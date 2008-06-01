/**************************************************************************
*   Copyright (C) 2008 by Michel Ludwig (michel.ludwig@kdemail.net)       *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "utilities.h"

#include <KRun>

ServiceRunAction::ServiceRunAction(const KService& service,
		                   const KUrl::List& urls,
		                   QWidget* window,
		                   bool tempFiles,
		                   const QString& suggestedFileName,
		                   const QByteArray& asn,
		                   QObject *parent)
: KAction(KIcon(service.icon()), service.genericName(), parent),
m_service(service),
m_urlList(urls),
m_window(window),
m_tempFiles(tempFiles),
m_suggestedFileName(suggestedFileName),
m_asn(asn)
{
	connect(this, SIGNAL(triggered()), this, SLOT(runService()));
}

ServiceRunAction::~ServiceRunAction()
{
}

void ServiceRunAction::runService()
{
	KRun::run(m_service, m_urlList, m_window,
	                                m_tempFiles,
	                                m_suggestedFileName,
	                                m_asn);
}

#include "utilities.moc"

