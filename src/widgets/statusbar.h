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
    explicit StatusBar(KileErrorHandler *errorHandler, QWidget *parent = Q_NULLPTR);
    ~StatusBar();

public:
    void setHintText(const QString& text);
    void clearHintText();

    void setParserStatus(const QString& text);
    void clearParserStatus();

    void setLineColumn(int line, int column);
    void clearLineColumn();

    void setViewMode(const QString& text);
    void clearViewMode();

    void setSelectionMode(const QString& text);
    void clearSelectionMode();

    void reset();

private:
    KileErrorHandler * const m_errorHandler;
    QLabel *m_hintTextLabel;
    QLabel *m_lineColumnLabel;
    QLabel *m_viewModeLabel;
    QLabel *m_selectionModeLabel;
    QLabel *m_parserStatusLabel;
};
}

#endif
