/***************************************************************************
                          newfilewizard.h  -  description
                             -------------------
    begin                : Sat Apr 26 2003
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

#ifndef NEWFILEWIZARD_H
#define NEWFILEWIZARD_H

#include <qstring.h>
#include <qcheckbox.h>

#include <kstandarddirs.h>
#include <kiconview.h>
#include <kdialogbase.h>

#include "templates.h"

const QString DEFAULT_EMPTY_CAPTION = i18n("Empty Document");

#define DEFAULT_EMPTY_ICON "type_Empty"

class TemplateItem : public QIconViewItem
{
public:
	TemplateItem( QIconView * parent, const TemplateInfo & info);
	~TemplateItem();

	QString name() { return m_info.name; }
	QString path() { return m_info.path; }
	QString icon() { return m_info.icon; }

private:
	TemplateInfo m_info;
};

class NewFileWidget : public KIconView
{
public:
	NewFileWidget(QWidget *parent = 0, char *name = 0);
	~NewFileWidget() {}
};

class NewFileWizard : public KDialogBase  
{
	Q_OBJECT
public:
	NewFileWizard(QWidget *parent=0, const char *name=0);
	~NewFileWizard();

public:
	TemplateItem* getSelection()const { return static_cast<TemplateItem*>(m_iv->currentItem());}
	bool useWizard();
  
public slots:
	void slotOk();

private:
	QIconView *m_iv;
	QCheckBox *m_ckWizard;
};

#endif

