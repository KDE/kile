/***************************************************************************
    date                 : Feb 09 2004
    version              : 0.10.0
    copyright            : (C) 2004 by Holger Danielsson
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

#ifndef CONFIGENVIRONMENT_H
#define CONFIGENVIRONMENT_H

#include <qstringlist.h>
#include <qmap.h>

#include <kdialogbase.h>

class QRadioButton;
class KPushButton;
class KListBox;

/**
  *@author Holger Danielsson
  */

class ConfigEnvironment : public QWidget
{
	Q_OBJECT
public: 
	ConfigEnvironment(QWidget *parent=0, const char *name=0);
	~ConfigEnvironment() {}

	void readConfig(void);
	void writeConfig(void);

private:
	QRadioButton *rb_listenv, *rb_mathenv, *rb_tabularenv;
	KPushButton *pb_add, *pb_remove;
	KListBox *listbox;

	QMap<QString,bool> m_dictenvlatex;
	QMap<QString,bool> m_dictenvlist;
	QMap<QString,bool> m_dictenvmath;
	QMap<QString,bool> m_dictenvtab;

	QStringList envlist,envmath,envtab;

	void fillListbox(const QMap<QString,bool> *map);
	void setEnvironments(const QStringList &list, QMap<QString,bool> &map);
	QStringList getEnvironments(const QMap<QString,bool> &map);
	QMap<QString,bool> *getDictionary();

private slots:
	void clickedEnvtype();
	void highlightedListbox(int index);
	void clickedAdd();
	void clickedRemove();
};

#endif
