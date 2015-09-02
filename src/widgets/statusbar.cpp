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

#include "statusbar.h"

#include <QLabel>
#include <QMap>

#include <KLocalizedString>

#include "errorhandler.h"
#include "kiledebug.h"

KileWidget::StatusBar::StatusBar(KileErrorHandler *errorHandler, QWidget* parent)
	: QStatusBar(parent),
	m_errorHandler(errorHandler)
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
	QLabel *l = new QLabel(this);
	l->setText(text);
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
