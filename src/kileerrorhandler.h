/***************************************************************************
    begin                : Tue May 25 2004
    Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                  2011 by Michel Ludwig (michel.ludwig@kdemail.net)
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

#include <QObject>

#include "outputinfo.h"

class KileInfo;
class OutputInfo;

class KileErrorHandler : public QObject
{
	Q_OBJECT
public:
	KileErrorHandler(QObject *parent, KileInfo *info, const char *name = NULL);

	~KileErrorHandler();

	void setMostRecentLogInformation(const QString& logFile, const LatexOutputInfoArray& outputInfoList);

/* log view, error handling */
private Q_SLOTS:
	void ViewLog();
	void NextError();
	void PreviousError();
	void NextWarning();
	void PreviousWarning();
	void NextBadBox();
	void PreviousBadBox();

private:
	void jumpToProblem(int type, bool);

public Q_SLOTS:
	void jumpToFirstError();
	void jumpToProblem(const OutputInfo& info);
	void reset();

private:
	KileInfo		*m_ki;
	int			m_nCurrentError;
	QString			m_mostRecentLogFile;
	LatexOutputInfoArray	m_mostRecentLaTeXOutputInfoList;

	void displayProblemsInLogWidget(const LatexOutputInfoArray& infoList);
};

#endif
