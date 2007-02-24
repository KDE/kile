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
#include <qvgroupbox.h>

#include <kdialogbase.h>
#include <klineedit.h>
#include <kpushbutton.h>

#include "templates.h"

class NewFileWidget;
class QLabel;
class KileProject;
class KComboBox;
class QVGroupBox;

class KileProjectDlgBase : public KDialogBase
{
	Q_OBJECT

public:
	KileProjectDlgBase(const QString &caption = QString::null, QWidget *parent = 0, const char * name = 0);
	virtual ~KileProjectDlgBase();

	void setProject(KileProject *project, bool override);
	virtual KileProject* project();

	void setProjectTitle(const QString &title) { m_title->setText(title); }
	const QString projectTitle() { return m_title->text(); }

	void setExtensions(KileProjectItem::Type type, const QString & ext);
	const QString extensions(KileProjectItem::Type type)
		{ return m_val_extensions[type-1]; }

	void setExtIsRegExp(KileProjectItem::Type type, bool is);
	bool extIsRegExp(KileProjectItem::Type type)
		{ return m_val_isregexp[type-1]; }

protected slots:
	virtual void slotOk() = 0;
	virtual void fillProjectDefaults();

private slots:
	void slotExtensionsHighlighted(int index);
	void slotExtensionsTextChanged(const QString &text);
	void slotRegExpToggled(bool on);

protected:
	QVGroupBox *m_pgroup, *m_egroup;
	QGridLayout	*m_pgrid, *m_egrid;
	QLabel *m_plabel;

	KLineEdit	*m_title, *m_extensions;
	QCheckBox	*m_isregexp;
	KileProject	*m_project;
	KComboBox	*m_sel_extensions;

	QString		m_val_extensions[KileProjectItem::Other - 1];
	bool		m_val_isregexp[KileProjectItem::Other - 1];
};

class KileNewProjectDlg : public KileProjectDlgBase
{
	Q_OBJECT

public:
	KileNewProjectDlg(QWidget* parent = 0, const char* name = 0);
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
	KLineEdit			*m_location, *m_file, *m_name;
	NewFileWidget		*m_nfw;
	QCheckBox		*m_cb;
	QLabel 			*m_lb;

	KPushButton *m_pbChooseDir;
	QString			m_dir, m_filename;
};

class KileProjectOptionsDlg : public KileProjectDlgBase
{
	Q_OBJECT

public:
	KileProjectOptionsDlg(KileProject *project, QWidget *parent = 0, const char * name = 0);
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
