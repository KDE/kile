/***************************************************************************
    begin                : Oct 03 2011
    author               : dani
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LATEXMENUDIALOG_H
#define LATEXMENUDIALOG_H

#include <QKeySequence>
#include <QCheckBox>

#include "kilewizard.h"
#include "kileinfo.h"

#include "dialogs/latexmenu/latexmenuitem.h"

#include "ui_latexmenudialog_base.h"


namespace KileMenu {

class LatexUserMenu;

class LatexmenuDialog : public KileDialog::Wizard
{
	Q_OBJECT

	public:
		LatexmenuDialog(KConfig *config, KileInfo *ki, QObject *latexusermenu, const QString & xmlfile, QWidget *parent);
		~LatexmenuDialog() {}

 	protected Q_SLOTS:
 		virtual void slotButtonClicked(int button);

	private Q_SLOTS:
		void slotCurrentItemChanged(QTreeWidgetItem *current,QTreeWidgetItem *previous);
		void slotInsertMenuItem();
		void slotInsertSubmenu();
		void slotInsertSeparator();
		void slotDelete();
		void slotUp();
		void slotDown();
		
		void slotMenuentryTypeClicked();
		void slotMenuentryTextChanged(const QString &text);
		void slotUrlSelected(const KUrl &url);
		void slotUrlTextChanged(const QString &text);
		void slotParameterTextChanged(const QString &);
		void slotPlainTextChanged();
		void slotIconClicked();
		void slotIconDeleteClicked();
		void slotKeySequenceChanged(const QKeySequence &seq);

		void slotSelectionStateChanged(int state);
		void slotCheckboxStateChanged(int);

		void slotNewClicked();
		void slotInstallClicked();
		void slotLoadClicked();
		void slotSaveClicked();
		void slotSaveAsClicked();

		void slotCustomContextMenuRequested(const QPoint &pos);
		
	Q_SIGNALS:
		void installXmlFile(const QString &);

	private:
		Ui::LatexmenuDialog m_LatexmenuDialog;
		LatexmenuTree *m_menutree;

		KileInfo *m_ki;

		bool m_modified;
		bool m_currentXmlInstalled;
		QString m_currentXmlFile;
		void setXmlFile(const QString &filename);
		
		QString m_currentIcon;
		QStringList m_listMenutypes;

		void startDialog();
		void initDialog();
		void setModified();

		void readMenuentryData(LatexmenuItem *tem);
		void showMenuentryData(LatexmenuItem *item);
		void clearMenuEntryData();
		void disableMenuEntryData();

		void setTextEntry(LatexmenuItem *item);
		void setFileContentEntry(LatexmenuItem *item);
		void setProgramEntry(LatexmenuItem *item);
		void setSeparatorEntry(LatexmenuItem *item);
		void setSubmenuEntry(LatexmenuItem *item);

		void setMenuentryText(LatexmenuItem *item, bool state);
		void setMenuentryType(LatexmenuItem *item, bool state1, bool state2);
		void setMenuentryFileChooser(LatexmenuItem *item, bool state);
		void setMenuentryFileParameter(LatexmenuItem *item, bool state);
		void setMenuentryTextEdit(LatexmenuItem *item, bool state);
		void setMenuentryIcon(LatexmenuItem *item, bool state, const QString &icon=QString::null);
		void setMenuentryShortcut(LatexmenuItem *item, bool state);

		void setParameterGroupbox(bool state);
		void setMenuentryCheckboxes(LatexmenuItem *item, bool useInsertOutput);

		void setMenuentryIcon(const QString &icon);	
		void updateTreeButtons();
		void updateDialogState(bool modified, bool install, bool save);
		void updateAfterDelete();

		void loadXmlFile(const QString &filename, bool install);
		bool saveCheck();
};

}

#endif
