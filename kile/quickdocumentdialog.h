//
// C++ Interface: quickdocheader
//
// Description:
//
//
// Author: Thomas Fischer <t-fisch@users.sourceforge.net>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KILEDIALOGQUICKDOCHEADER_H
#define KILEDIALOGQUICKDOCHEADER_H

#include "kilewizard.h"

class KComboBox;
class QListView;
class KLineEdit;

namespace KileDialog
{

/**
@author Jeroen Wijnhout
*/
class QuickDocument : public Wizard
{
	Q_OBJECT

public:
	QuickDocument(KConfig *, QWidget *parent=0, const char *name=0, const QString &caption = QString::null);
	~QuickDocument();

	void packageEdit(QListViewItem *cur);

public slots:
	void init();
	void slotOk();

private:
	KComboBox *m_cbDocumentClass;
	KComboBox *m_cbTypefaceSize;
	KComboBox *m_cbPaperSize;
	KComboBox *m_cbEncoding;
	QListView *m_lvClassOptions;
	QListView *m_lvPackagesCommon;
	QListView *m_lvPackagesExotic;
	KLineEdit *m_leAuthor;
	KLineEdit *m_leTitle;
	KLineEdit *m_leDate;
	void setupGUI();
	void writeConfig();
	void writeListView(QString key, QListView *listView, bool saveSelected=true);
	void readConfig();
	bool readListView(QString key, QListView *listView, bool readSelected=true);
	bool inputDialogDouble(QString caption, QString label1, QString& text1, QString label2, QString& text2);
	void packageDelete(QListViewItem *cur);
	void packageAddOption(QListViewItem *cur);
	void packageAdd(QListView *listView);
	void initClassOption();
	void initDocumentClass();
	void initPaperSize();
	void initEncoding();
	void initPackageCommon();
	void initPackageExotic();
	void printTemplate();
	void printPackage(QListView *listView);

private slots:
	void slotClassOptionReset();
	void slotClassOptionAdd();
	void slotClassOptionEdit();
	void slotClassOptionDelete();
	void slotCommonPackageReset();
	void slotCommonPackageAdd();
	void slotCommonPackageAddOption();
	void slotCommonPackageEdit();
	void slotCommonPackageDelete();
	void slotExoticPackageReset();
	void slotExoticPackageAdd();
	void slotExoticPackageAddOption();
	void slotExoticPackageEdit();
	void slotExoticPackageDelete();
	void slotDocumentClassAdd();
	void slotDocumentClassDelete();
	void slotDocumentClassReset();
	void slotPaperSizeAdd();
	void slotPaperSizeDelete();
	void slotPaperSizeReset();
	void slotEncodingAdd();
	void slotEncodingDelete();
	void slotEncodingReset();
	void slotCheckParent(QListViewItem *listViewItem);
};

};

#endif
