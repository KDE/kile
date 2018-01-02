/*************************************************************************************************
   Copyright (C) 2015 Andreas Cord-Landwehr (cordlandwehr@kde.org)
                 2016 by Michel Ludwig (michel.ludwig@kdemail.net)
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

#include <KLocalizedString>

#include "errorhandler.h"
#include "kiledebug.h"

KileWidget::StatusBar::StatusBar(KileErrorHandler *errorHandler, QWidget* parent)
    : QStatusBar(parent),
      m_errorHandler(errorHandler)
{
    m_hintTextLabel = new QLabel();
    m_parserStatusLabel = new QLabel();
    m_lineColumnLabel = new QLabel();
    m_viewModeLabel = new QLabel();
    m_selectionModeLabel = new QLabel();

    addPermanentWidget(m_hintTextLabel, 10);
    addPermanentWidget(m_errorHandler->compilationResultLabel());
    addPermanentWidget(m_parserStatusLabel, 0);
    addPermanentWidget(m_lineColumnLabel, 0);
    addPermanentWidget(m_viewModeLabel, 0);
    addPermanentWidget(m_selectionModeLabel, 0);

    reset();
}

KileWidget::StatusBar::~StatusBar()
{
}

void KileWidget::StatusBar::setHintText(const QString& text)
{
    m_hintTextLabel->setText(text);
}

void KileWidget::StatusBar::clearHintText()
{
    m_hintTextLabel->clear();
}

void KileWidget::StatusBar::setParserStatus(const QString& text)
{
    m_parserStatusLabel->setText(text);
}

void KileWidget::StatusBar::clearParserStatus()
{
    m_parserStatusLabel->clear();
}

void KileWidget::StatusBar::setLineColumn(int line, int column)
{
    m_lineColumnLabel->setText(i18n("Line: %1 Col: %2", line, column));
}

void KileWidget::StatusBar::clearLineColumn()
{
    m_lineColumnLabel->clear();
}

void KileWidget::StatusBar::setViewMode(const QString& text)
{
    m_viewModeLabel->setText(text);
}

void KileWidget::StatusBar::clearViewMode()
{
    m_viewModeLabel->clear();
}

void KileWidget::StatusBar::setSelectionMode(const QString& text)
{
    m_selectionModeLabel->setText(text);
}

void KileWidget::StatusBar::clearSelectionMode()
{
    m_selectionModeLabel->clear();
}

void KileWidget::StatusBar::reset()
{
    clearHintText();
    clearParserStatus();
    clearLineColumn();
    clearViewMode();
    clearSelectionMode();
}
