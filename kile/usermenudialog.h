/***************************************************************************
                          usermenudialog.h  -  description
                             -------------------
    begin                : Sun Jun 3 2001
    copyright            : (C) 2001 by Brachet Pascal
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef USERMENUDIALOG_H
#define USERMENUDIALOG_H

#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qtextedit.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qstring.h>

typedef QString userlist[10];

class usermenudialog : public QDialog
{
    Q_OBJECT

public:
    usermenudialog( QWidget* parent = 0, const char* name = 0);
    ~usermenudialog();

    userlist Name,Tag;

private:
    int previous_index;
    QTextEdit *tagedit;
    QLineEdit *itemedit;
    QComboBox *combo1;
    QLabel* label1;
    QLabel* label2;
	  QPushButton *buttonOk;
	  QPushButton *buttonCancel;

public slots:
    void init();        

private slots:
    void change(int index);
    void slotOk();

};

#endif // USERMENUDIALOG_H
