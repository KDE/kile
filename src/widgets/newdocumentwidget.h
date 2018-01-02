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

#ifndef NEWDOCUMENTWIDGET_H
#define NEWDOCUMENTWIDGET_H

#include <QWidget>

#include "ui_newdocumentwidget.h"

class NewDocumentWidget : public QWidget, public Ui::NewDocumentWidget
{
    Q_OBJECT

public:
    NewDocumentWidget(QWidget *parent = 0);
    ~NewDocumentWidget();
};

#endif
