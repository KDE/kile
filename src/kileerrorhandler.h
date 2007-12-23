/***************************************************************************
    begin                : Tue May 25 2004
    copyright            : (C) 2003 by Jeroen Wijnhout
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

#ifndef KILEERRORHANDLER_H
#define KILEERRORHANDLER_H

#include <qobject.h>

/**
@author Jeroen Wijnhout
*/

class KileInfo;
class OutputInfo;

class KileErrorHandler : public QObject
{
	Q_OBJECT
public:
	KileErrorHandler(QObject *parent, KileInfo *info, const char *name = 0);

	~KileErrorHandler();

/* log view, error handling */
private slots:
	void ViewLog();
	void NextError();
	void PreviousError();
	void NextWarning();
	void PreviousWarning();
	void NextBadBox();
	void PreviousBadBox();

private:
	void jumpToProblem(int type, bool);
	void jumpToProblem(OutputInfo *);

public slots:
	void jumpToFirstError();
	void reset();
	void showLogResults(const QString &);

private:
	KileInfo		*m_ki;
	int			m_nCurrentError;
};

#endif
