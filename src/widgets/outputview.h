/**************************************************************************************
    begin                : Sun Dec 21 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                           (C) 2009 by Michel Ludwig (michel.ludwig@kdemail.net)
 **************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ****************************************************************************/
#ifndef OUTPUTVIEW_H
#define OUTPUTVIEW_H

#include <KTextEdit>

namespace KileWidget {

class OutputView : public KTextEdit
{
    Q_OBJECT

public:
    OutputView(QWidget *parent);
    ~OutputView();

public Q_SLOTS:
    void receive(const QString &);

protected:
    virtual void paintEvent(QPaintEvent *ev);
};

}

#endif
