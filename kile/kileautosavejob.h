//
// C++ Interface: kileautosavejob
//
// Description: 
//
//
// Author: Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KILEAUTOSAVEJOB_H
#define KILEAUTOSAVEJOB_H

#include <qobject.h>

#include <kio/job.h>

class KileAutoSaveJob : public QObject
{
	Q_OBJECT

public:
	KileAutoSaveJob(const KURL& from);
	~KileAutoSaveJob();

protected slots:
	void slotResult(KIO::Job *);

signals:
	void success();
};

#endif
