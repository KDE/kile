/*************************************************************************************************
   Copyright (C) 2015 Andreas Cord-Landwehr (cordlandwehr@kde.org)
 *************************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KILESTATUSBAR_H
#define KILESTATUSBAR_H

#include <QStatusBar>

class QLabel;
class KileErrorHandler;

namespace KileWidget {

class StatusBar : public QStatusBar
{
	Q_OBJECT

public:
	StatusBar(KileErrorHandler *errorHandler, QWidget *parent = Q_NULLPTR);
	~StatusBar();

public:
	enum StatusMode {
		HintText,
		LineColumn,
		ViewMode,
		SelectionMode,
		ParserStatus
	};

	void changeItem(StatusMode id, const QString &text);
	void reset();

private:
	void addLabel(StatusMode id, const QString &text, int stretch = 0);
	KileErrorHandler * const m_errorHandler;
	QLabel *m_hintTextLabel;
	QLabel *m_lineColumnLabel;
	QLabel *m_viewModeLabel;
	QLabel *m_selectionModeLabel;
	QLabel *m_parserStatusLabel;
};
}

#endif
