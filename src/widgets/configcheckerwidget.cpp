/**************************************************************************
*   Copyright (C) 2007 by Michel Ludwig (michel.ludwig@kdemail.net)       *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "widgets/configcheckerwidget.h"

ConfigCheckerWidget::ConfigCheckerWidget(QWidget *parent) : QWidget(parent)
{
    setupUi(this);
}

ConfigCheckerWidget::~ConfigCheckerWidget()
{
}

QProgressBar* ConfigCheckerWidget::progressBar()
{
    return m_progress;
}


QLabel* ConfigCheckerWidget::label()
{
    return m_lbChecking;
}

QListWidget* ConfigCheckerWidget::listWidget()
{
    return m_lstResults;
}

