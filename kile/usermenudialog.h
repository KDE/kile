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

#include <qdialog.h>
#include <qstring.h>
#include <qstringlist.h>

class QRadioButton;
class QTextEdit;
class QLineEdit;
class QComboBox;
class QLabel;
class QPushButton;


#ifndef KILE_USERITEM
struct userItem
{
	QString name,tag;
};
#define KILE_USERITEM
#endif



class usermenudialog : public QDialog
{
    Q_OBJECT

public:
    usermenudialog( const QValueList<userItem> &list, QWidget* parent = 0, const char* name = 0, const QString &caption = QString::null);
    ~usermenudialog();

    int index() { return previous_index; }
    int result();
    
    QStringList Name,Tag;

    enum Result { Edit, Add, Remove};

private:
    int previous_index;
    QTextEdit *tagedit;
    QLineEdit *itemedit;
    QComboBox *combo1;
    QLabel* label1;
    QLabel* label2;
    QPushButton *buttonOk;
    QPushButton *buttonCancel;
    QRadioButton *radioEdit, *radioRemove, *radioAdd;

public slots:
    void init();

private slots:
    void change(int index);
    void slotOk();

};

#endif // USERMENUDIALOG_H
