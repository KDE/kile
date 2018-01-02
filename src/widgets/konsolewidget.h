/**********************************************************************
 Copyright (C) 2001 - 2003 by Brachet Pascal
               2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
               2007-2012 by Michel Ludwig (michel.ludwig@kdemail.net)
 **********************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KONSOLEWIDGET_H
#define KONSOLEWIDGET_H

#include <QFrame>
#include <QShowEvent>

namespace KParts {
class ReadOnlyPart;
}

class KileInfo;

namespace KileWidget
{
class Konsole : public QFrame
{
    Q_OBJECT

public:
    Konsole(KileInfo *, QWidget* parent);
    ~Konsole();

public Q_SLOTS:
    void setDirectory(const QString& dir);
    void activate();
    void sync();

private Q_SLOTS:
    void slotDestroyed();

protected:
    void showEvent(QShowEvent *ev);
    void spawn();

private:
    KParts::ReadOnlyPart	*m_part;
    bool			m_bPresent;
    KileInfo		*m_ki;
    QString 		m_currentDir;
};
}

#endif
