/***************************************************************************
                          quickdocumentdialog.h  -  description
                             -------------------
    begin                : Tue Oct 30 2001
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

#ifndef QUICKDOCUMENTDIALOG_H
#define QUICKDOCUMENTDIALOG_H
#include <qdialog.h>
#include <qstringlist.h>
#include "addoptiondialog.h"


class QLabel;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QPushButton;

class QListBox;

/**
  *@author Brachet Pascal
  */

class quickdocumentdialog : public QDialog  {
    Q_OBJECT
public:
    quickdocumentdialog(QWidget *parent=0, const char *name=0);
    ~quickdocumentdialog();

public:
    QLabel *QLabel_1;
    QLabel *QLabel_2;
    QLabel *QLabel_3;
    QLabel *QLabel_4;
    QLabel *QLabel_5;
    QLabel *QLabel_6;
    QLabel *QLabel_7;
    QLineEdit *LineEdit1,*LineEdit2 ;
    QComboBox *combo1;
    QComboBox *combo2;
    QComboBox *combo3;
    QComboBox *combo4;
    QCheckBox* checkbox1, *checkbox2;
    QPushButton *buttonOk;
    QPushButton *buttonCancel;
    QPushButton *userClassBtn, *userPaperBtn, *userEncodingBtn, *userOptionsBtn;

    QListBox *availableBox;
    QStringList otherClassList, otherPaperList, otherEncodingList, otherOptionsList;

public slots:
    void Init();
private slots:
    void addUserClass();
    void addUserPaper();
    void addUserEncoding();
    void addUserOptions();
private:
    AddOptionDialog *dlg;
};


#endif
