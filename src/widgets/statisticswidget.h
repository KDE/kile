/***************************************************************************
    begin                : Tuesday Nov 15 2005
    copyright            : (C) 2005 by Holger Danielsson
    email                : holger.danielsson@t-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef STATISTICSWIDGET_H
#define STATISTICSWIDGET_H

#include <QGroupBox>
#include <QWidget>

class QLabel;
class QGridLayout;

namespace KileDialog {
class StatisticsDialog;
}

namespace KileWidget {

class StatisticsWidget : public QWidget
{
    friend class KileDialog::StatisticsDialog;
    Q_OBJECT

public:
    explicit StatisticsWidget(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = 0);
    ~StatisticsWidget();

    void updateColumns();

private:
    QLabel* m_commentAboutHelp;
    QLabel* m_warning;

    QLabel* m_wordChar;
    QLabel* m_commandChar;
    QLabel* m_whitespaceChar;
    QLabel* m_totalChar;
    QLabel* m_wordCharText;
    QLabel* m_commandCharText;
    QLabel* m_whitespaceCharText;
    QLabel* m_totalCharText;

    QLabel* m_wordString;
    QLabel* m_environmentString;
    QLabel* m_commandString;
    QLabel* m_totalString;
    QLabel* m_wordStringText;
    QLabel* m_environmentStringText;
    QLabel* m_commandStringText;
    QLabel* m_totalStringText;

    QGroupBox *m_charactersGroup, *m_stringsGroup;

    QGridLayout *chargrouplayout;
    QGridLayout *stringgrouplayout;
};

}

#endif
