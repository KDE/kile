/***************************************************************************
  Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PROJECTDIALOGS_H
#define PROJECTDIALOGS_H

#include <KDialog>

#include <QCheckBox>

#include <KLineEdit>
#include <KUrlRequester>

#include "kileextensions.h"
#include "kileproject.h"
#include "templates.h"

class QLabel;
class QGridLayout;
class QGroupBox;

class KComboBox;

class KileProject;
class TemplateItem;
class TemplateIconView;

namespace KileDocument {
class Extensions;
}
namespace KileTemplate {
class Manager;
}

class KileProjectDlgBase : public KDialog
{
		Q_OBJECT

	public:
		KileProjectDlgBase(const QString &caption, KileDocument::Extensions *extensions, QWidget *parent = 0, const char * name = 0);
		virtual ~KileProjectDlgBase();

		void setProject(KileProject *project, bool override);
		virtual KileProject* project();

		void setProjectTitle(const QString &title) {
			m_title->setText(title);
		}
		const QString projectTitle() {
			return m_title->text();
		}

		void setExtensions(KileProjectItem::Type type, const QString & ext);
		const QString extensions(KileProjectItem::Type type)
		{
			return m_val_extensions[type-1];
		}

	protected Q_SLOTS:
		virtual void fillProjectDefaults();

	private Q_SLOTS:
		void slotExtensionsHighlighted(int index);
		void slotExtensionsTextChanged(const QString &text);

	protected:
		KileDocument::Extensions *m_extmanager;

		QGroupBox *m_pgroup, *m_egroup;
		QGridLayout *m_pgrid, *m_egrid;
		QLabel *m_plabel;

		KLineEdit *m_title, *m_extensions;
		QLabel *m_lbPredefinedExtensions, *m_lbStandardExtensions, *m_lbDefGraphicExt;
		KileProject *m_project;
		KComboBox *m_sel_extensions;
		KComboBox *m_sel_defGraphicExt;

		QString  m_val_extensions[KileProjectItem::Other - 1];
		QString   m_val_standardExtensions[KileProjectItem::Other - 1];

		bool acceptUserExtensions();

};

class KileNewProjectDlg : public KileProjectDlgBase
{
		Q_OBJECT

	public:
		KileNewProjectDlg(KileTemplate::Manager *templateManager, KileDocument::Extensions *extensions, QWidget* parent = 0, const char* name = 0);
		~KileNewProjectDlg();

		KileProject* project();

		QString cleanProjectFile();
		QString folder() {
			return m_folder->lineEdit()->text();
		}

		TemplateItem* getSelection() const;
		QString file() {
			return m_file->text();
		}
		bool createNewFile() {
			return m_cb->isChecked();
		}

	private Q_SLOTS:
		void clickedCreateNewFileCb();
		void fillProjectDefaults();

	protected Q_SLOTS:
		virtual void slotButtonClicked(int button);

	private:
		KileTemplate::Manager *m_templateManager;
		KLineEdit *m_file, *m_name;
		KUrlRequester *m_folder;
		TemplateIconView *m_templateIconView;
		QCheckBox  *m_cb;
		QLabel    *m_lb;
		KUrl m_projectFileWithPath;
};

class KileProjectOptionsDlg : public KileProjectDlgBase
{
		Q_OBJECT

	public:
		KileProjectOptionsDlg(KileProject *project, KileDocument::Extensions *extensions, QWidget *parent = 0, const char * name = 0);
		~KileProjectOptionsDlg();

	private Q_SLOTS:
		void toggleMakeIndex(bool);

	protected Q_SLOTS:
		virtual void slotButtonClicked(int button);

	private:
		KComboBox *m_master, *m_cbQuick;
		KLineEdit *m_leMakeIndex;
		QCheckBox *m_ckMakeIndex;
		QString m_toolDefaultString;
};

#endif
