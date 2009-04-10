/*****************************************************************************************
    begin                : Sat Apr 26 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                               2007-2008 by Michel Ludwig (michel.ludwig@kdemail.net)
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

#include <KStandardDirs>
#include <KDialog>
#include <KLocale>

#include "kileconstants.h"
#include "templates.h"

class NewDocumentWidget;

class NewFileWizard : public KDialog
{
	Q_OBJECT
public:
	explicit NewFileWizard(KileTemplate::Manager *manager, KileDocument::Type startType = KileDocument::LaTeX,
	              QWidget *parent = NULL, const char *name = NULL);
	~NewFileWizard();

public:
	TemplateItem* getSelection() const;
	bool useWizard();

protected Q_SLOTS:
	virtual void slotButtonClicked(int button);
	void slotClickOKButton();

	void slotActivated(int index);

	void restoreSelectedIcon();

protected:
	KileTemplate::Manager *m_templateManager;
	NewDocumentWidget* m_newDocumentWidget;
	int m_currentlyDisplayedType; // not a document type, only a local type!

	QString getConfigKey(int index);

	void storeSelectedIcon();
	void displayType(int index);
};

#endif
