/******************************************************************************************
date                 : Sep 12 2004
version              : 0.22
copyright            : Thomas Fischer (t-fisch@users.sourceforge.net)
                       restructured, improved and completed by Holger Danielsson
                       (C) 2004 by Holger Danielsson (holger.danielsson@t-online.de)
*******************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QUICKDOCUMENTDIALOG_H
#define QUICKDOCUMENTDIALOG_H

#include "kilewizard.h"

#include <QList>
#include <QMap>

class QLabel;
class QTabWidget;
class QTreeWidget;
class QTreeWidgetItem;

namespace KileWidget {
class CategoryComboBox;
}

class KComboBox;
class KLineEdit;
class KPushButton;

namespace KileDialog
{

// some flags to check the results of the input dialog
enum {
	qd_CheckNotEmpty = 1,
	qd_CheckDocumentClass = 2,
	qd_CheckClassOption = 4,
	qd_CheckPackage = 8,
	qd_CheckPackageOption = 16,
	qd_CheckFontsize = 32,
	qd_CheckPapersize = 64
};

class QuickDocument : public Wizard
{
		Q_OBJECT

	public:
		explicit QuickDocument(KConfig *, QWidget *parent = 0, const char *name = 0, const QString &caption = QString());
		~QuickDocument();

		bool isStandardClass(const QString &classname);
		bool isDocumentClass(const QString &name);
		bool isDocumentClassOption(const QString &option);
		bool isPackage(const QString &package);
		bool isPackageOption(const QString &package, const QString &option);

	private:
		KileWidget::CategoryComboBox *m_cbDocumentClass;
		KileWidget::CategoryComboBox *m_cbTypefaceSize;
		KileWidget::CategoryComboBox *m_cbPaperSize;
		KileWidget::CategoryComboBox *m_cbEncoding;
		QTreeWidget *m_lvClassOptions;
		QTreeWidget *m_lvPackages;
		KLineEdit *m_leAuthor;
		KLineEdit *m_leTitle;
		KLineEdit *m_leDate;
		QLabel    *m_lbPaperSize;

		QString m_currentClass;
		QString m_currentFontsize;
		QString m_currentPapersize;
		QString m_currentEncoding;
		bool m_currentHyperref;
		QString m_hyperrefdriver;
		QString m_hyperrefsetup;
		QStringList m_userClasslist;
		QStringList m_deleteDocumentClasses;

		QMap<QString, QStringList> m_dictDocumentClasses;
		QMap<QString, bool> m_dictStandardClasses;
		QMap<QString, bool> m_currentDefaultOptions;
		QMap<QString, bool> m_currentSelectedOptions;
		QMap<QString, bool> m_dictPackagesEditable;
		QMap<QString, QString> m_dictPackagesDefaultvalues;
		QMap<QString, bool> m_dictHyperrefDriver;

		KPushButton *m_btnDocumentClassAdd;
		KPushButton *m_btnDocumentClassDelete;
		KPushButton *m_btnTypefaceSizeAdd;
		KPushButton *m_btnTypefaceSizeDelete;
		KPushButton *m_btnPaperSizeAdd;
		KPushButton *m_btnPaperSizeDelete;
		KPushButton *m_btnEncodingAdd;
		KPushButton *m_btnEncodingDelete;

		KPushButton *m_btnClassOptionsAdd;
		KPushButton *m_btnClassOptionsEdit;
		KPushButton *m_btnClassOptionsDelete;
		KPushButton *m_btnPackagesAdd;
		KPushButton *m_btnPackagesAddOption;
		KPushButton *m_btnPackagesEdit;
		KPushButton *m_btnPackagesDelete;
		KPushButton *m_btnPackagesReset;

		// GUI
		QWidget *setupClassOptions(QTabWidget *tab);
		QWidget *setupPackages(QTabWidget *tab);
		QWidget *setupProperties(QTabWidget *tab);

		// read/write config files and init data
		void readConfig();
		void readDocumentClassConfig();
		void readPackagesConfig();
		void initHyperref();
		void writeConfig();
		void writeDocumentClassConfig();
		void writePackagesConfig();

		// document class tab
		void initDocumentClass();
		void initStandardClass(const QString &classname, const QString &fontsize,
		                       const QString &papersize, const QString &defaultoptions,
		                       const QString &selectedoptions);
		void initStandardOptions(const QString &classname, QStringList &optionlist);
		void setDefaultClassOptions(const QString &defaultoptions);
		void setSelectedClassOptions(const QString &selectedoptions);
		void setClassOptions(const QStringList &list, uint start);
		void updateClassOptions();
		QString getClassOptions();
		void fillDocumentClassCombobox();
		void fillCombobox(KileWidget::CategoryComboBox *combo, const QString &cslist, const QString &seltext);
		bool addComboboxEntries(KileWidget::CategoryComboBox *combo, const QString &title, const QString &entry);
		QString getComboxboxList(KComboBox *combo);

		bool isDefaultClassOption(const QString &option);
		bool isSelectedClassOption(const QString &option);
		QString stripDefault(const QString &s);

		// packages tab
		void initPackages();
		bool readPackagesListview();
		QTreeWidgetItem *insertTreeWidget(QTreeWidget *treeWidget,
		                                  const QString &entry,
		                                  const QString &description);
		QTreeWidgetItem *insertTreeWidget(QTreeWidgetItem *parent,
		                                  const QString &entry,
		                                  const QString &description);
		QTreeWidgetItem *insertEditableTreeWidget(QTreeWidgetItem *parent,
		    const QString &entry,
		    const QString &description,
		    const QString &value,
		    const QString &defaultvalue);
		bool isTreeWidgetEntry(QTreeWidget *treeWidget, const QString &entry);
		void setPackagesValue(QTreeWidgetItem *item, const QString &option, const QString &val);
		QString getPackagesValue(const QString &value);

		bool isTreeWidgetChild(QTreeWidget *treeWidget, const QString &entry, const QString &option);
		QString addPackageDefault(const QString &option, const QString &description);
		QString stripPackageDefault(const QString &option, const QString &description);
		bool isHyperrefDriver(const QString &name);

		// document template
		void printTemplate();
		void printPackages();
		void printHyperref();
		void printBeamerTheme();

		// input dialog
		bool inputDialog(QStringList &list, int check = qd_CheckNotEmpty);

	private Q_SLOTS:
		void slotDocumentClassAdd();
		void slotDocumentClassDelete();
		void slotDocumentClassChanged(int index);
		void slotTypefaceSizeAdd();
		void slotTypefaceSizeDelete();
		void slotPaperSizeAdd();
		void slotPaperSizeDelete();
		void slotOptionDoubleClicked(QTreeWidgetItem *item, int column);
		void slotClassOptionAdd();
		void slotClassOptionEdit();
		void slotClassOptionDelete();

		void slotCheckParent(QTreeWidgetItem *item);
		void slotPackageDoubleClicked(QTreeWidgetItem *item);
		void slotPackageAdd();
		void slotPackageAddOption();
		void slotPackageEdit();
		void slotPackageDelete();
		void slotPackageReset();

		void slotEnableButtons();

	protected Q_SLOTS:
	virtual void slotButtonClicked(int button);
};

class QuickDocumentInputDialog : public KDialog {
		Q_OBJECT
	public:
		QuickDocumentInputDialog(const QStringList &list, int check = 0,
		                         QuickDocument *parent = 0, const char *name = 0);
		~QuickDocumentInputDialog();

		void getResults(QStringList &list);

	private:
		QuickDocument *m_parent;
		int  m_check;

		QStringList m_description;
		QList<QWidget*> m_objectlist;

		QString getPackageName(const QString &text);
		bool checkListEntries(const QString &title, const QString &textlist, const QString &pattern);

	protected Q_SLOTS:
        	virtual void slotButtonClicked(int button);
};

} // namespace

#endif
