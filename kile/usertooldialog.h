/***************************************************************************
                          usertooldialog.h  -  description
                             -------------------
    begin                : mer avr 9 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
    email                : Jeroen.Wijnhout@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef USERTOOLDIALOG_H
#define USERTOOLDIALOG_H

#include <qdialog.h>
#include <qstring.h>
#include <qstringlist.h>

class QLineEdit;
class QComboBox;
class QLabel;
class QPushButton;
class QRadioButton;

#ifndef KILE_USERITEM
struct userItem
{
	QString name,tag;
};
#define KILE_USERITEM
#endif

class usertooldialog : public QDialog  {
   Q_OBJECT
public:
	usertooldialog(const QValueList<userItem> &list, QWidget *parent=0, const char *name=0, const QString &caption = QString::null);
	~usertooldialog();

	int index() { return previous_index; }
	int result();
	enum Result { Edit, Add, Remove};
	
      QStringList Name,Tool;

private:
    int previous_index;
    QLineEdit *itemedit, *tooledit;
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

#endif
