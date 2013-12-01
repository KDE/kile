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


#include <QDateTime>
#include <QDir>

#include <KRun>

#include "kiledebug.h"

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

QString KileUtilities::lastModifiedFile(const QStringList& files, const QString& baseDir)
{
	KILE_DEBUG() << "==KileUtilities::lastModifiedFile()=====";

	if(files.empty()) {
		return QString();
	}

	QStringList absoluteFileNames;
	if(baseDir.isEmpty()) {
		absoluteFileNames = files;
	}
	else {
		QDir basePath(baseDir);
		Q_FOREACH(const QString& file, files) {
			absoluteFileNames.append(basePath.absoluteFilePath(file));
		}
	}

	QDateTime lastModifiedTime;
	const QString* lastModifiedFile = NULL;

	Q_FOREACH(const QString& file, absoluteFileNames) {
		QDateTime modificationTime = QFileInfo(file).lastModified();
		if(!lastModifiedTime.isValid() || modificationTime > lastModifiedTime) {
		   lastModifiedFile = &file;
		   lastModifiedTime = modificationTime;
		}
	}

	if(lastModifiedFile) {
		return *lastModifiedFile;
	}
	else {
		return QString();
	}
}

#include "utilities.moc"

