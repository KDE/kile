/*****************************************************************************************
    begin                : Sat Apr 26 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                               2007 by Michel Ludwig (michel.ludwig@kdemail.net)
 *****************************************************************************************/

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

#include <QString>
#include <QCheckBox>

#include <kstandarddirs.h>
#include <k3iconview.h>
#include <kdialog.h>
#include <klocale.h>
#include <k3process.h>

#include "kileconstants.h"
#include "templates.h"

class NewDocumentWidget;

class NewFileWizard : public KDialog
{
	Q_OBJECT
public:
	NewFileWizard(KileTemplate::Manager *manager, QWidget *parent=0, const char *name=0);
	~NewFileWizard();

public:
	TemplateItem* getSelection() const;
	bool useWizard();

protected Q_SLOTS:
	void slotOk();
	void slotActivated(int index);

	void restoreSelectedIcon();

protected:
	KileTemplate::Manager *m_templateManager;
	NewDocumentWidget* m_newDocumentWidget;
	int m_currentlyDisplayedType; // not a document type, only a local type!

	QString getConfigKey(int index);

	void storeSelectedIcon();

};

#endif
