/***************************************************************************
date                 : Sep 12 2004
version              : 0.22
copyright            : Thomas Fischer <t-fisch@users.sourceforge.net>
                       restructured, improved and completed by Holger Danielsson
                       (C) 2004 by Holger Danielsson
email                : holger.danielsson@t-online.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KILEDIALOGQUICKDOCHEADER_H
#define KILEDIALOGQUICKDOCHEADER_H

#include <qmap.h>
#include <q3valuelist.h>
//Added by qt3to4:
#include <QLabel>
#include "kilewizard.h"

class KComboBox;
class Q3ListView;
class Q3CheckListItem;
class KLineEdit;
class KPushButton;

namespace KileDialog
{

// some flags to check the results of the input dialog
enum {
	qd_CheckNotEmpty=1,        
	qd_CheckDocumentClass=2,
	qd_CheckClassOption=4,
	qd_CheckPackage=8,
	qd_CheckPackageOption=16,
	qd_CheckFontsize=32,
	qd_CheckPapersize=64
};

class QuickDocument : public Wizard
{
	Q_OBJECT

public:
	QuickDocument(KConfig *, QWidget *parent=0, const char *name=0, const QString &caption = QString::null);
	~QuickDocument();

	bool isStandardClass(const QString &classname);             
	bool isDocumentClass(const QString &name);
	bool isDocumentClassOption(const QString &option);
	bool isPackage(const QString &package);
	bool isPackageOption(const QString &package, const QString &option);
	
public slots:
	void slotOk();

private:
	KComboBox *m_cbDocumentClass;
	KComboBox *m_cbTypefaceSize;
	KComboBox *m_cbPaperSize;
	KComboBox *m_cbEncoding;
	Q3ListView *m_lvClassOptions;
	Q3ListView *m_lvPackages;
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
	
	QMap<QString,QStringList> m_dictDocumentClasses; 
	QMap<QString,bool> m_dictStandardClasses;    
	QMap<QString,bool> m_currentDefaultOptions;   
	QMap<QString,bool> m_currentSelectedOptions;   
	QMap<QString,bool> m_dictPackagesEditable;   
	QMap<QString,QString> m_dictPackagesDefaultvalues;     
	QMap<QString,bool> m_dictHyperrefDriver;   
	
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
	void initStandardClass(const QString &classname,const QString &fontsize,      
	                       const QString &papersize,const QString &defaultoptions,
	                       const QString &selectedoptions);
	void initStandardOptions(const QString &classname,QStringList &optionlist); 	
	void setDefaultClassOptions(const QString &defaultoptions);            
	void setSelectedClassOptions(const QString &selectedoptions);             
	void setClassOptions(const QStringList &list,uint start);            
	void updateClassOptions(); 
	QString getClassOptions();  
	void fillDocumentClassCombobox();   
	void fillCombobox(KComboBox *combo, const QString &cslist,const QString &seltext);    
	bool addComboboxEntries(KComboBox *combo, const QString &title,const QString &entry);
	QString getComboxboxList(KComboBox *combo);
		   
	bool isDefaultClassOption(const QString &option);                   
	bool isSelectedClassOption(const QString &option);                  
	QString stripDefault(const QString &s);          
	
	// packages tab
	void initPackages();
	bool readPackagesListview();        
	Q3CheckListItem *insertListview(Q3ListView *listview,               
                                  const QString &entry,
                                  const QString &description);
	Q3CheckListItem *insertListview(Q3CheckListItem *parent,               
                                  const QString &entry,
                                  const QString &description);
	Q3CheckListItem *insertEditableListview(Q3CheckListItem *parent,               
	                                       const QString &entry,const QString &description,
	                                       const QString value,const QString defaultvalue);
	bool isListviewEntry(Q3ListView *listview,const QString &entry);
	void setPackagesValue(Q3ListViewItem *item,const QString &option,const QString &val);
	QString getPackagesValue(const QString &value);

	bool isListviewChild(Q3ListView *listview,const QString &entry, const QString &option);
	QString addPackageDefault(const QString &option,const QString &description);
	QString stripPackageDefault(const QString &option,const QString &description);
	bool isHyperrefDriver(const QString &name);
	
	// document template
	void printTemplate();
	void printPackages();
	void printHyperref();
	void printBeamerTheme();
	
	// input dialog
	bool inputDialog(QStringList &list,int check=qd_CheckNotEmpty);

private slots:
	void slotDocumentClassAdd();
	void slotDocumentClassDelete();
	void slotDocumentClassChanged(int index);  
	void slotTypefaceSizeAdd();  
	void slotTypefaceSizeDelete();  
	void slotPaperSizeAdd();
	void slotPaperSizeDelete();
	void slotOptionDoubleClicked(Q3ListViewItem *listViewItem,const QPoint &,int); 
	void slotClassOptionAdd();
	void slotClassOptionEdit();
	void slotClassOptionDelete();
	
	void slotCheckParent(Q3ListViewItem *listViewItem);
	void slotPackageDoubleClicked(Q3ListViewItem *listViewItem,const QPoint &,int);
	void slotPackageAdd();
	void slotPackageAddOption();
	void slotPackageEdit();
	void slotPackageDelete();
	void slotPackageReset();
	
	void slotEnableButtons();
};

class QuickDocumentInputDialog : public KDialogBase  {
   Q_OBJECT
public: 
	QuickDocumentInputDialog(const QStringList &list,int check=0,
	                         QuickDocument *parent=0, const char *name=0);
	~QuickDocumentInputDialog();

	void getResults(QStringList &list);
	
private:
	QuickDocument *m_parent;
	int  m_check;
	
	QStringList m_description;
	Q3ValueList<QWidget *> m_objectlist;
		
	QString getPackageName(const QString &text);
	bool checkListEntries(const QString &title, const QString &textlist,const QString &pattern);
	
private slots:
	void slotOk();
};

} // namespace

#endif
