//
// C++ Implementation: kileautosavejob
//
// Description: 
//
//
// Author: Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>, (C) 2004
//
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>

#include "kileautosavejob.h"

KileAutoSaveJob::KileAutoSaveJob(const KURL &url)
{
	KIO::Job *job = KIO::file_copy(url,KURL(KURL::fromPathOrURL(url.path()+".backup")),-1,true,false,false);
	//let KIO show the error messages
	job->setAutoErrorHandlingEnabled(true);
	connect(job, SIGNAL(result(KIO::Job*)), this, SLOT(slotResult(KIO::Job*)));
}

KileAutoSaveJob::~KileAutoSaveJob()
{
	kdDebug() << "DELETING KileAutoSaveJob" << endl;
}

void KileAutoSaveJob::slotResult(KIO::Job *job)
{
	if (job->error() == 0)
		emit(success());

	deleteLater();
}

#include "kileautosavejob.moc"
