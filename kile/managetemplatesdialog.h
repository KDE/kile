/***************************************************************************
                          managetemplatesdialog.h  -  description
                             -------------------
    begin                : Sun Apr 27 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
    email                : wijnhout@science.uva.nl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MANAGETEMPLATESDIALOG_H
#define MANAGETEMPLATESDIALOG_H

#include <qfileinfo.h>
#include <qstring.h>
#include <klistview.h>
#include <kdialogbase.h>
#include <klineedit.h>

#include "templates.h"

/**
  *@author Jeroen Wijnhout
  */

class ManageTemplatesDialog : public KDialogBase  {
   Q_OBJECT
public: 
	ManageTemplatesDialog(QFileInfo src, const QString &caption,QWidget *parent=0, const char *name=0);
	ManageTemplatesDialog(const QString &caption,QWidget *parent=0, const char *name=0);	
	~ManageTemplatesDialog();

public slots:
   void slotSelectedTemplate(QListViewItem *item);
   void slotSelectedTemplate();
   void slotSelectIcon();
   void addTemplate();
   void removeTemplate();

private:
	bool selected;
   TemplateInfo m_sourceTemplate;
   KLineEdit *m_nameEdit, *m_iconEdit;
   KListView *tlist;
   Templates *m_Templates;
};

#endif
