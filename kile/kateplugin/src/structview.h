/***************************************************************************
 *   Copyright (C) 2003 by Roland Schulz                                   *
 *   mail@r2s2.de                                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef STRUCTVIEW_H
#define STRUCTVIEW_H

#include <qlistview.h>
#include <qstringlist.h>
#include <kate/document.h>
#include <kate/viewmanager.h>

/**
@author Roland Schulz
*/
class StructView : public QListView
{
Q_OBJECT
public:
    StructView(Kate::ViewManager* viewManager, QWidget *parent = 0, const char *name = 0);
    ~StructView();

public slots:
    void UpdateStructure();

private:
	Kate::ViewManager* m_viewManager;
	QListViewItem *level[6],*lastChild, *Child;
   QStringList structlist, labelitem, structitem;
   QString struct_level[5];

private slots:
	void ClickedOnStructure(QListViewItem *);
	void DoubleClickedOnStructure(QListViewItem *);
};

#endif
