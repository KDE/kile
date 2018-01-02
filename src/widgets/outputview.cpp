/*************************************************************************************
    begin                : Sun Dec 21 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                           (C) 2009-2012 by Michel Ludwig (michel.ludwig@kdemail.net)
 *************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ****************************************************************************/

#include "widgets/outputview.h"

#include <KColorScheme>

#include "kiledebug.h"

namespace KileWidget {

OutputView::OutputView(QWidget *parent) : KTextEdit(parent)
{
    setReadOnly(true);
    setAcceptRichText(false);
}

OutputView::~OutputView()
{
}

void OutputView::receive(const QString& str)
{
    static QString line;

    //find newline symbol
    //only output if we have receive one or more
    //full lines of text
    int newLineAt = str.lastIndexOf('\n');
    if(newLineAt != -1) {
        line += str.left(newLineAt); //don't copy the newline char
        append(line);
        line = str.mid(newLineAt + 1);
    }
    else {
        line += str;
    }
}

void OutputView::paintEvent(QPaintEvent *ev)
{
    QPalette customPalette = palette();
    KColorScheme::adjustBackground(customPalette, KColorScheme::NormalBackground,
                                   QPalette::Base, KColorScheme::View);
    setPalette(customPalette);
    KTextEdit::paintEvent(ev);
}

}

