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

#include "statusbar.h"
#include "errorhandler.h"
#include <KLocalizedString>
#include <KSqueezedTextLabel>
#include <QMap>

KileWidget::StatusBar::StatusBar(KileErrorHandler *errorHandler, QWidget* parent)
	: QStatusBar(parent)
	, m_errorHandler(errorHandler)
{
	reset();
}

KileWidget::StatusBar::~StatusBar()
{
}

void KileWidget::StatusBar::changeItem(StatusBar::StatusMode id, const QString &text)
{
	switch (id) {
	case HintText:
		m_hintTextLabel->setText(text);
		break;
	case ParserStatus:
		m_parserStatusLabel->setText(text);
		break;
	case LineColumn:
		m_lineColumnLabel->setText(text);
		break;
	case ViewMode:
		m_viewModeLabel->setText(text);
		break;
	case SelectionMode:
		m_selectionModeLabel->setText(text);
		break;
	}
}

void KileWidget::StatusBar::reset()
{
	addLabel(HintText, i18n("Normal Mode"), 10);
	addPermanentWidget(m_errorHandler->compilationResultLabel());
	addLabel(ParserStatus, QString(), 0);
	addLabel(LineColumn, QString(), 0);
	addLabel(ViewMode, QString(), 0);
	addLabel(SelectionMode, QString(), 0);
}

void KileWidget::StatusBar::addLabel(StatusMode id, const QString& text, int stretch)
{
	KSqueezedTextLabel *l = new KSqueezedTextLabel(text, this);
	l->setFixedHeight(fontMetrics().height() + 2);
	l->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	switch (id) {
	case HintText:
		m_hintTextLabel = l;
		break;
	case ParserStatus:
		m_parserStatusLabel = l;
		break;
	case LineColumn:
		m_lineColumnLabel = l;
		break;
	case ViewMode:
		m_viewModeLabel = l;
		break;
	case SelectionMode:
		m_selectionModeLabel = l;
		break;
	}
	addPermanentWidget(l, stretch);
	l->show();
}
