/*
 *  Copyright 2015  Andreas Cord-Landwehr <cordlandwehr@kde.org>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KILESTATUSBAR_H
#define KILESTATUSBAR_H

#include <QMap>
#include <QStatusBar>

class KSqueezedTextLabel;
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
	KSqueezedTextLabel * m_hintTextLabel;
	KSqueezedTextLabel * m_lineColumnLabel;
	KSqueezedTextLabel * m_viewModeLabel;
	KSqueezedTextLabel * m_selectionModeLabel;
	KSqueezedTextLabel * m_parserStatusLabel;
};
}

#endif
