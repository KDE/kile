/***************************************************************************
    date                 : Feb 18 2005
    version              : 0.12
    copyright            : (C) 2005 by Holger Danielsson
    email                : holger.danielsson@t-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QUICKPREVIEW_H
#define QUICKPREVIEW_H

#include "kileinfo.h"

#include <qobject.h> 
#include <qstring.h>
#include <qstringlist.h>

namespace KileTool
{

class QuickPreview : public QObject
{
	Q_OBJECT

public:
	QuickPreview(KileInfo *ki);
	~QuickPreview();
	
	void run(const QString &text,const QString &textfilename,int startrow);
	void getTaskList(QStringList &tasklist);
	
private slots:
	void destroyed();

private:
	enum { pvLatex=0, pvDvips=1, pvViewer=2, pvViewerCfg=3, pvExtension=4 };
	
	KileInfo *m_ki;
	QString m_tempfile;
	QStringList m_taskList;
	bool m_running;
		
	int createTempfile(const QString &text);
	void removeTempFiles(bool rmdir=false);
	void showError(const QString &text);
};

}

#endif
