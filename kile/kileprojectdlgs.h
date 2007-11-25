/***************************************************************************
    begin                : Sun Aug 3 2003
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

#ifndef KILEPROJECTDLGS_H
#define KILEPROJECTDLGS_H

#include <qcheckbox.h>
#include <qlayout.h>
#include <q3vgroupbox.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <QLabel>

#include <kdialog.h>
#include <klineedit.h>
#include <kpushbutton.h>

#include "kileproject.h"
#include "templates.h"

class TemplateIconView;
class QLabel;
class KileProject;
class KComboBox;
class Q3VGroupBox;
class TemplateItem;

namespace KileDocument { class Extensions; }
namespace KileTemplate { class Manager; }

class KileProjectDlgBase : public KDialog
{
	Q_OBJECT

public:
	KileProjectDlgBase(const QString &caption, KileDocument::Extensions *extensions, QWidget *parent = 0, const char * name = 0);
	virtual ~KileProjectDlgBase();

	void setProject(KileProject *project, bool override);
	virtual KileProject* project();

	void setProjectTitle(const QString &title) { m_title->setText(title); }
	const QString projectTitle() { return m_title->text(); }

	void setExtensions(KileProjectItem::Type type, const QString & ext);
	const QString extensions(KileProjectItem::Type type)
		{ return m_val_extensions[type-1]; }

protected slots:
	virtual void slotOk() = 0;
	virtual void fillProjectDefaults();

private slots:
	void slotExtensionsHighlighted(int index);
	void slotExtensionsTextChanged(const QString &text);

protected:
	KileDocument::Extensions *m_extmanager;

	Q3VGroupBox *m_pgroup, *m_egroup;
	Q3GridLayout	*m_pgrid, *m_egrid;
	QLabel *m_plabel;

	KLineEdit	*m_title, *m_extensions;
	QLabel *m_lbPredefinedExtensions, *m_lbStandardExtensions;
	KileProject	*m_project;
	KComboBox	*m_sel_extensions;

	QString		m_val_extensions[KileProjectItem::Other - 1];
	QString 		m_val_standardExtensions[KileProjectItem::Other - 1];

	bool acceptUserExtensions();

};

class KileNewProjectDlg : public KileProjectDlgBase
{
	Q_OBJECT

public:
	KileNewProjectDlg(KileTemplate::Manager *templateManager, KileDocument::Extensions *extensions, QWidget* parent = 0, const char* name = 0);
	~KileNewProjectDlg();

	KileProject* project();

	QString bare();
	QString location() { return m_location->text(); }

	TemplateItem* getSelection() const;
	QString file() { return m_file->text();}
	bool createNewFile() { return m_cb->isChecked(); }

private slots:
	void clickedCreateNewFileCb();
	void browseLocation();
	void makeProjectPath();
	void slotOk();
	void fillProjectDefaults();

private:
	KileTemplate::Manager	*m_templateManager;
	KLineEdit		*m_location, *m_file, *m_name;
	TemplateIconView	*m_templateIconView;
	QCheckBox		*m_cb;
	QLabel 			*m_lb;

	KPushButton *m_pbChooseDir;
	QString			m_dir, m_filename;
};

class KileProjectOptionsDlg : public KileProjectDlgBase
{
	Q_OBJECT

public:
	KileProjectOptionsDlg(KileProject *project, KileDocument::Extensions *extensions, QWidget *parent = 0, const char * name = 0);
	~KileProjectOptionsDlg();

private slots:
	void slotOk();
	void toggleMakeIndex(bool);

private:
	KComboBox	*m_master, *m_cbQuick;
	KLineEdit		*m_leMakeIndex;
	QCheckBox	*m_ckMakeIndex;
};

#endif
