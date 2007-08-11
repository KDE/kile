/***************************************************************************
date                 : Sep 15 2004
version              : 0.23
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

#include "quickdocumentdialog.h"

#include <qstringlist.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <qwidget.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qstyle.h>
#include <qlistview.h>
#include <qwhatsthis.h>
#include <qregexp.h>

#include <kcombobox.h>
#include <klocale.h>
#include <kconfig.h>
#include <klineedit.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include "kileconfig.h"


namespace KileDialog
{
enum {
	qd_Base=1,
	qd_Article=2,
	qd_BookReport=4,
	qd_KomaArticle=8,
	qd_KomaBookReport=16,
	qd_KomaAbstract=32,
	qd_Prosper=64,
	qd_Beamer=128
};

// list with index numbers for the stringlist with all information of a document class
enum {
	qd_Fontsizes,
	qd_Papersizes,
	qd_DefaultOptions,
	qd_SelectedOptions,
	qd_OptionsStart
};

//////////////////// ListBoxSeparator ////////////////////

class ListBoxSeparator : public QListBoxItem
{
public:
	ListBoxSeparator(int h);
protected:
	virtual void paint( QPainter * );
	virtual int width( const QListBox* ) const { return listBox()->width(); }
	virtual int height( const QListBox* ) const { return m_height; }
private:
	int m_height;
};

ListBoxSeparator::ListBoxSeparator(int h) : QListBoxItem(), m_height(h)
{
	// setText("-");          // not necessary, use QString::null
	setCustomHighlighting( true );
	setSelectable(false);    // doesn't work here, so set it again after creating item
}

void ListBoxSeparator::paint(QPainter *painter)
{
//	QRect r( 0, 0, width(listBox()), height(listBox()) );
	painter->setPen(Qt::gray);
	painter->drawLine(0,m_height/2+1,listBox()->width()-10,m_height/2+1);
}

//////////////////// EditableCheckListItem ////////////////////

class EditableCheckListItem : public QCheckListItem
{
public:
	EditableCheckListItem(QCheckListItem *parent, const QString &text);

	virtual void paintCell(QPainter *p, const QColorGroup &cg,
                          int column, int width, int alignment );
};

EditableCheckListItem::EditableCheckListItem(QCheckListItem *parent, const QString &text)
  : QCheckListItem(parent,text,QCheckListItem::CheckBox)
{
}

void EditableCheckListItem::paintCell( QPainter *p, const QColorGroup &cg,
                                       int column, int width, int alignment )
{
	if ( column == 1) {
		QColorGroup colorgroup( cg );

		QListViewItem *item = dynamic_cast<QListViewItem*>(this);
		if ( item && (item->text(1)==i18n("<default>") || item->text(1)==i18n("<empty>")) )  {
			colorgroup.setColor( QColorGroup::Text, Qt::gray );
			colorgroup.setColor( QColorGroup::HighlightedText, Qt::gray );
		}
		QCheckListItem::paintCell( p, colorgroup, column, width, Qt::AlignHCenter );
	} else {
		QCheckListItem::paintCell( p, cg, column, width, alignment );
	}
}

//////////////////// QuickDocument class ////////////////////

QuickDocument::QuickDocument(KConfig *config, QWidget *parent, const char *name, const QString &caption) : Wizard(config, parent,name,caption)
{
	kdDebug() << "==QuickDocument::setupGUI()============" << endl;
	QTabWidget *tabWidget = new QTabWidget( this );
	setMainWidget(tabWidget);

	tabWidget->addTab( setupClassOptions(tabWidget), i18n("Cla&ss Options"));
	tabWidget->addTab( setupPackages(tabWidget), i18n("&Packages"));
	tabWidget->addTab( setupProperties(tabWidget), i18n("&Document Properties"));

	// read config file
	readConfig();

}

QuickDocument::~QuickDocument()
{}

//////////////////// GUI ////////////////////

QWidget *QuickDocument::setupClassOptions(QTabWidget *tab)
{
	kdDebug() << "\tsetupClassOptions" << endl;
	QLabel *label;

	QWidget *classOptions = new QWidget( tab );
	QGridLayout *gl = new QGridLayout(classOptions, 6,4, marginHint(), spacingHint());
	gl->setColStretch(1,1);

	// Document classes
	m_cbDocumentClass = new KComboBox(classOptions);
	m_cbDocumentClass->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
	m_cbDocumentClass->setDuplicatesEnabled(false);
//	m_cbDocumentClass->listBox()->setVariableHeight(true);
	gl->addWidget(m_cbDocumentClass,0,1);
	connect(m_cbDocumentClass, SIGNAL(activated(int)), this, SLOT(slotDocumentClassChanged(int)));

	label = new QLabel(i18n("Doc&ument class:"), classOptions);
	gl->addWidget(label,0,0);
	label->setBuddy(m_cbDocumentClass);	label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

	m_btnDocumentClassAdd = new KPushButton(SmallIcon("edit_add"), "", classOptions);
	QWhatsThis::add(m_btnDocumentClassAdd, i18n("Add current text to this list"));
	gl->addWidget(m_btnDocumentClassAdd,0,2);
	connect(m_btnDocumentClassAdd, SIGNAL(clicked()), this, SLOT(slotDocumentClassAdd()));

	m_btnDocumentClassDelete = new KPushButton(SmallIcon("eraser"), "", classOptions);
	QWhatsThis::add(m_btnDocumentClassDelete, i18n("Remove current element from this list"));
	gl->addWidget(m_btnDocumentClassDelete,0,3);
	connect(m_btnDocumentClassDelete, SIGNAL(clicked()), this, SLOT(slotDocumentClassDelete()));

	// Fontsize
	m_cbTypefaceSize = new KComboBox(classOptions);
	m_cbTypefaceSize->setDuplicatesEnabled(false);
	gl->addWidget(m_cbTypefaceSize,1,1);

	label = new QLabel(i18n("&Typeface size:"), classOptions);
	label->setBuddy(m_cbTypefaceSize);
	label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	gl->addWidget(label,1,0);

	m_btnTypefaceSizeAdd = new KPushButton(SmallIcon("edit_add"), "", classOptions);
	QWhatsThis::add(m_btnTypefaceSizeAdd, i18n("Add current text to this list"));
	gl->addWidget(m_btnTypefaceSizeAdd,1,2);
	connect(m_btnTypefaceSizeAdd, SIGNAL(clicked()), this, SLOT(slotTypefaceSizeAdd()));

	m_btnTypefaceSizeDelete = new KPushButton(SmallIcon("eraser"), "", classOptions);
	QWhatsThis::add(m_btnTypefaceSizeDelete, i18n("Remove current element from this list"));
	gl->addWidget(m_btnTypefaceSizeDelete,1,3);
	connect(m_btnTypefaceSizeDelete, SIGNAL(clicked()), this, SLOT(slotTypefaceSizeDelete()));

	// Papersize
	m_cbPaperSize = new KComboBox(classOptions);
	m_cbPaperSize->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
	m_cbPaperSize->setDuplicatesEnabled(false);
	gl->addWidget(m_cbPaperSize,2,1);

	m_lbPaperSize = new QLabel(i18n("Paper si&ze:"), classOptions);
	m_lbPaperSize->setBuddy(m_cbPaperSize);
	m_lbPaperSize->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	gl->addWidget(m_lbPaperSize,2,0);

	m_btnPaperSizeAdd = new KPushButton(SmallIcon("edit_add"), "", classOptions);
	QWhatsThis::add(m_btnPaperSizeAdd, i18n("Add current text to this list"));
	gl->addWidget(m_btnPaperSizeAdd,2,2);
	connect(m_btnPaperSizeAdd, SIGNAL(clicked()), this, SLOT(slotPaperSizeAdd()));

	m_btnPaperSizeDelete = new KPushButton(SmallIcon("eraser"), "", classOptions);
	QWhatsThis::add(m_btnPaperSizeDelete, i18n("Remove current element from this list"));
	gl->addWidget(m_btnPaperSizeDelete,2,3);
	connect(m_btnPaperSizeDelete, SIGNAL(clicked()), this, SLOT(slotPaperSizeDelete()));

	// Encoding
	m_cbEncoding = new KComboBox(classOptions);
	m_cbEncoding->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
	m_cbEncoding->setDuplicatesEnabled(false);
	gl->addWidget(m_cbEncoding,3,1);

	label = new QLabel(i18n("E&ncoding:"), classOptions);
	label->setBuddy(m_cbEncoding);
	label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	gl->addWidget(label,3,0);

	// Class Options
	m_lvClassOptions = new QListView(classOptions);
	m_lvClassOptions->addColumn(i18n("Option"));
	m_lvClassOptions->addColumn(i18n("Description"));
	m_lvClassOptions->setAllColumnsShowFocus(true);
	gl->addMultiCellWidget(m_lvClassOptions, 4,4, 1,3);
	connect(m_lvClassOptions, SIGNAL(selectionChanged()),
	        this, SLOT(slotEnableButtons()));
	connect(m_lvClassOptions, SIGNAL(doubleClicked(QListViewItem *,const QPoint &,int)),
	        this, SLOT(slotOptionDoubleClicked(QListViewItem *,const QPoint &,int)));

	label = new QLabel(i18n("Cl&ass options:"), classOptions);
	label->setBuddy(m_lvClassOptions);
	label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
	label->setAlignment(Qt::AlignTop);
	gl->addWidget(label,4,0);

	// button
	QWidget *frame = new QWidget(classOptions);
	QHBoxLayout *hl = new QHBoxLayout(frame, 0, spacingHint());
	//hl->addStretch(1);
	gl->addMultiCellWidget(frame, 5,5, 1,3, Qt::AlignCenter);

	m_btnClassOptionsAdd = new KPushButton(SmallIcon("edit_add"), i18n("&Add..."), frame);
	QWhatsThis::add(m_btnClassOptionsAdd, i18n("Add a new class option"));
	hl->addWidget(m_btnClassOptionsAdd);
	connect(m_btnClassOptionsAdd, SIGNAL(clicked()), this, SLOT(slotClassOptionAdd()));

	m_btnClassOptionsEdit = new KPushButton(SmallIcon("edit"), i18n("Ed&it..."), frame);
	QWhatsThis::add(m_btnClassOptionsEdit, i18n("Edit the current class option"));
	hl->addWidget(m_btnClassOptionsEdit);
	connect(m_btnClassOptionsEdit, SIGNAL(clicked()), this, SLOT(slotClassOptionEdit()));

	m_btnClassOptionsDelete = new KPushButton(SmallIcon("eraser"), i18n("De&lete"), frame);
	QWhatsThis::add(m_btnClassOptionsDelete, i18n("Remove the current class option"));
	hl->addWidget(m_btnClassOptionsDelete);
	connect(m_btnClassOptionsDelete, SIGNAL(clicked()), this, SLOT(slotClassOptionDelete()));

	return classOptions;
}

QWidget *QuickDocument::setupPackages(QTabWidget *tab)
{
	kdDebug() << "\tsetupPackages" << endl;

	QWidget *packages = new QWidget( tab );
	QVBoxLayout *vl = new QVBoxLayout(packages, marginHint(), spacingHint());

	QLabel *label = new QLabel(i18n("LaTe&X packages:"), packages);
	vl->addWidget(label);
	m_lvPackages = new QListView(packages);
	vl->addWidget(m_lvPackages);
	m_lvPackages->setRootIsDecorated(true);
	m_lvPackages->addColumn(i18n("Package"));
	m_lvPackages->addColumn(i18n("Value"));
	m_lvPackages->addColumn(i18n("Description"));
	m_lvPackages->setAllColumnsShowFocus(true);
	label->setBuddy(m_lvPackages);
	connect(m_lvPackages, SIGNAL(clicked(QListViewItem *)),
	        this, SLOT(slotCheckParent(QListViewItem *)));
	connect(m_lvPackages, SIGNAL(spacePressed(QListViewItem *)),
	        this, SLOT(slotCheckParent(QListViewItem *)));
	connect(m_lvPackages, SIGNAL(selectionChanged()),
	        this, SLOT(slotEnableButtons()));
	connect(m_lvPackages, SIGNAL(doubleClicked(QListViewItem *,const QPoint &,int)),
	        this, SLOT(slotPackageDoubleClicked(QListViewItem *,const QPoint &,int)));

	QWidget *frame = new QWidget(packages);
	vl->addWidget(frame);
	QHBoxLayout *hl = new QHBoxLayout(frame, 0, spacingHint());
	hl->addStretch(1);

	m_btnPackagesAdd = new KPushButton(SmallIcon("edit_add"), "&Add Package...", frame);
	QWhatsThis::add(m_btnPackagesAdd, i18n("Add a new package"));
	connect(m_btnPackagesAdd, SIGNAL(clicked()), this, SLOT(slotPackageAdd()));
	hl->addWidget(m_btnPackagesAdd);
	m_btnPackagesAddOption = new KPushButton(SmallIcon("edit_add"), i18n("Add Op&tion..."), frame);
	QWhatsThis::add(m_btnPackagesAddOption, i18n("Add a new package option"));
	connect(m_btnPackagesAddOption, SIGNAL(clicked()), this, SLOT(slotPackageAddOption()));
	hl->addWidget(m_btnPackagesAddOption);
	m_btnPackagesEdit = new KPushButton(SmallIcon("edit"), "Ed&it...", frame);
	QWhatsThis::add(m_btnPackagesEdit, i18n("Edit the current package option"));
	connect(m_btnPackagesEdit, SIGNAL(clicked()), this, SLOT(slotPackageEdit()));
	hl->addWidget(m_btnPackagesEdit);
	m_btnPackagesDelete = new KPushButton(SmallIcon("eraser"), i18n("De&lete"), frame);
	QWhatsThis::add(m_btnPackagesDelete, i18n("Remove the current package option"));
	connect(m_btnPackagesDelete, SIGNAL(clicked()), this, SLOT(slotPackageDelete()));
	hl->addWidget(m_btnPackagesDelete);
	m_btnPackagesReset = new KPushButton(SmallIcon("reload"), i18n("&Reset to Defaults"), frame);
	QWhatsThis::add(m_btnPackagesReset, i18n("Reset to the default list of packages"));
	connect(m_btnPackagesReset, SIGNAL(clicked()), this, SLOT(slotPackageReset()));
	hl->addWidget(m_btnPackagesReset);
	hl->addStretch(1);

	return packages;
}

QWidget *QuickDocument::setupProperties(QTabWidget *tab)
{
	kdDebug() << "\tsetupProperties" << endl;
	QLabel *label;

	QWidget *personalInfo = new QWidget( tab );
	QGridLayout *gl = new QGridLayout(personalInfo, 4,2, marginHint(),spacingHint());
	gl->setRowStretch(gl->numRows()-1, 1);

	m_leAuthor = new KLineEdit(personalInfo);
	gl->addWidget(m_leAuthor, 0, 1);
	label = new QLabel(i18n("&Author:"), personalInfo);
	gl->addWidget(label, 0, 0);
	label->setBuddy(m_leAuthor);

	m_leTitle = new KLineEdit(personalInfo);
	gl->addWidget(m_leTitle, 1, 1);
	label = new QLabel(i18n("&Title:"), personalInfo);
	gl->addWidget(label, 1, 0);
	label->setBuddy(m_leTitle);

	m_leDate = new KLineEdit(personalInfo);
	gl->addWidget(m_leDate, 2, 1);
	label = new QLabel(i18n("Dat&e:"), personalInfo);
	gl->addWidget(label, 2, 0);
	label->setBuddy(m_leDate);

	// set current date
	m_leDate->setText( KGlobal::locale()->formatDate(QDate::currentDate(), true) );
	// For KDE4:
	//m_leDate->setText( KGlobal::locale()->formatDate(QDate::currentDate(), KLocale::ShortDate) );

	return personalInfo;
}

//////////////////// read configuration ////////////////////

void QuickDocument::readConfig()
{
	kdDebug() << "==QuickDocument::readConfig()============" << endl;

	// read config for document class
	readDocumentClassConfig();
	// init the current document class
	initDocumentClass();

	// read config for packages
	readPackagesConfig();
	initHyperref();

	// read author
	m_leAuthor->setText(KileConfig::author());

}

//////////////////// write configuration ////////////////////

void QuickDocument::writeConfig()
{
	kdDebug() << "==QuickDocument::writeConfig()============" << endl;

	// write document class to config file
	writeDocumentClassConfig();

	// write packages to config file
	writePackagesConfig();

	// set author
	KileConfig::setAuthor(m_leAuthor->text());
}

////////////////////////////// document class tab //////////////////////////////

void QuickDocument::readDocumentClassConfig()
{
	kdDebug() << "\tread config: document class" << endl;

	// read standard options
	m_userClasslist = KileConfig::userClasses();
	m_currentClass = KileConfig::documentClass();
	m_currentEncoding = KileConfig::encoding();

	// init standard classes
	QString stdFontsize = "10pt,11pt,12pt";
	QString stdPapersize = "a4paper,a5paper,b5paper,executivepaper,legalpaper,letterpaper";
	QString beamerThemes = "bars;boxes;classic;lined;plain;sidebar;sidebar (dark);sidebar (tab);"
	                       "sidebar (dark,tab);shadow;split;tree;tree (bar)";

	initStandardClass( "article",stdFontsize,stdPapersize,
	                   "10pt,letterpaper,oneside,onecolumn,final",
	                   KileConfig::optionsArticle() );
	initStandardClass( "book",stdFontsize,stdPapersize,
	                   "10pt,letterpaper,twoside,onecolumn,final,openright",
	                   KileConfig::optionsBook() );
	initStandardClass( "letter",stdFontsize,stdPapersize,
	                   "10pt,letterpaper,oneside,onecolumn,final",
	                   KileConfig::optionsLetter() );
	initStandardClass( "report",stdFontsize,stdPapersize,
	                   "10pt,letterpaper,oneside,onecolumn,final,openany",
	                   KileConfig::optionsReport() );
	initStandardClass( "scrartcl",stdFontsize,stdPapersize,
	                   "11pt,a4paper,abstractoff,bigheadings,final,headnosepline,"
	                   "footnosepline,listsindent,onelinecaption,notitlepage,onecolumn,"
	                   "oneside,openany,parindent,tablecaptionbelow,tocindent",
	                   KileConfig::optionsScrartcl() );
	initStandardClass( "scrbook",stdFontsize,stdPapersize,
	                   "11pt,a4paper,bigheadings,final,headnosepline,footnosepline,"
	                   "listsindent,nochapterprefix,onelinecaption,onecolumn,"
	                   "openright,parindent,tablecaptionbelow,titlepage,tocindent,twoside",
	                   KileConfig::optionsScrbook() );
	initStandardClass( "scrreprt",stdFontsize,stdPapersize,
	                   "11pt,a4paper,abstractoff,bigheadings,final,headnosepline,"
	                   "footnosepline,listsindent,nochapterprefix,onelinecaption,onecolumn,"
	                   "oneside,openany,parindent,tablecaptionbelow,titlepage,tocindent",
	                   KileConfig::optionsScrreprt() );
	initStandardClass( "prosper",QString::null,QString::null,
	                   "final,slideBW,total,nocolorBG,ps,noaccumulate,ps2pdf",
	                   KileConfig::optionsProsper() );
	initStandardClass( "beamer","8pt,9pt,10pt,11pt,12pt,14pt,17pt,20pt",beamerThemes,
	                   "11pt,blue,notes=show,sans,slidescentered",
	                   KileConfig::optionsBeamer() );

	// init all user classes
	for ( uint i=0; i<m_userClasslist.count(); ++i )
	{
		kdDebug() << "\tinit user class: " << m_userClasslist[i] << endl;
		QStringList list;
		// read dour default entries for this user class
		m_config->setGroup( QString("QuickDocument/")+m_userClasslist[i] );
		list.append( m_config->readEntry("fontsizesList") );
		list.append( m_config->readEntry("pagesizesList") );
		list.append( m_config->readEntry("defaultOptions") );
		list.append( m_config->readEntry("selectedOptions") );
		// now read all user defined options
		QStringList options = QStringList::split(",", m_config->readEntry("options") );
		for ( uint j=0; j<options.count(); ++j ) {
			list.append( options[j] + " => " + m_config->readEntry(options[j]) );
		}

		// save all information of this class into the documentClass-dictionary
		m_dictDocumentClasses[ m_userClasslist[i] ] = list;
	}

	// set classes combobox (standard and user defined classes)
	fillDocumentClassCombobox();

	// set encoding combobox
	fillCombobox(m_cbEncoding,
	             "ansinew,applemac,ascii,cp1252,cp1250,cp1251,cp1257,cp437,cp437de,cp850,cp858,"
				     "cp852,cp865,decmulti,koi8-r,latin1,latin2,latin3,latin4,latin5,latin9,latin10,next,utf8,utf8x,utf-8,utfcyr",
	             m_currentEncoding);
}

void QuickDocument::fillDocumentClassCombobox()
{
	QString stdClasses = "article,book,letter,report,-,scrartcl,scrbook,scrreprt,-";
	QString stdUserClasses = "beamer,prosper";

	// set classes combobox (standard and user defined classes)
	QStringList classlist = QStringList::split(",",stdUserClasses);
	for ( uint i=0; i< m_userClasslist.count(); ++i )
		classlist.append( m_userClasslist[i] );
	classlist.sort();
	fillCombobox(m_cbDocumentClass,stdClasses + ',' + classlist.join(","),m_currentClass);
}

void QuickDocument::writeDocumentClassConfig()
{
	kdDebug() << "\twrite config: document class" << endl;

	// first delete all marked document classes
	for ( uint i=0; i<m_deleteDocumentClasses.count(); ++i ) {
		kdDebug() << "\tdelete class: " << m_deleteDocumentClasses[i] << endl;
		m_config->deleteGroup( QString("QuickDocument/")+m_deleteDocumentClasses[i] );
	}

	// write document classes and encoding
	QStringList userclasses;
	for ( int i=0; i<m_cbDocumentClass->count(); ++i) {
		if ( !m_cbDocumentClass->text(i).isEmpty() && !isStandardClass(m_cbDocumentClass->text(i)) ) {
			userclasses.append( m_cbDocumentClass->text(i) );
		}
	}
	KileConfig::setUserClasses(userclasses);
	KileConfig::setDocumentClass(m_cbDocumentClass->currentText());
	KileConfig::setEncoding(m_cbEncoding->currentText());

	// write checked options of standard classes
	kdDebug() << "\twrite standard classes" << endl;
	KileConfig::setOptionsArticle( m_dictDocumentClasses["article"][qd_SelectedOptions] );
	KileConfig::setOptionsBook( m_dictDocumentClasses["book"][qd_SelectedOptions] );
	KileConfig::setOptionsLetter( m_dictDocumentClasses["letter"][qd_SelectedOptions] );
	KileConfig::setOptionsReport( m_dictDocumentClasses["report"][qd_SelectedOptions] );
	KileConfig::setOptionsScrartcl( m_dictDocumentClasses["scrartcl"][qd_SelectedOptions] );
	KileConfig::setOptionsScrbook( m_dictDocumentClasses["scrbook"][qd_SelectedOptions] );
	KileConfig::setOptionsScrreprt( m_dictDocumentClasses["scrreprt"][qd_SelectedOptions] );
	KileConfig::setOptionsProsper( m_dictDocumentClasses["prosper"][qd_SelectedOptions] );
	KileConfig::setOptionsBeamer( m_dictDocumentClasses["beamer"][qd_SelectedOptions] );

	// write config of user packages
	QRegExp reg("(\\S+)\\s+=>\\s+(.*)");
	for ( uint i=0; i< userclasses.count(); ++i ) {
		// get the stringlist with all information
		kdDebug() << "\twrite user class: " << userclasses[i] << endl;
		QStringList list = m_dictDocumentClasses[ userclasses[i] ];

		// write the config group and the default entries
		m_config->setGroup( QString("QuickDocument/")+userclasses[i] );
		m_config->writeEntry("fontsizesList",list[qd_Fontsizes]);
		m_config->writeEntry("pagesizesList",list[qd_Papersizes]);
		m_config->writeEntry("defaultOptions",list[qd_DefaultOptions]);
		m_config->writeEntry("selectedOptions",list[qd_SelectedOptions]);

		// write user defined options
		QString options;
		for ( uint j=qd_OptionsStart; j<list.count(); ++j ) {
			int pos = reg.search( list[j] );
			if ( pos != -1 ) {
				m_config->writeEntry( reg.cap(1),reg.cap(2) );
				if ( ! options.isEmpty() )
					options += ',';
				options += reg.cap(1);
			}
		}
		m_config->writeEntry("options",options);
	}
}

void QuickDocument::initDocumentClass()
{
	kdDebug() << "==QuickDocument::initDocumentClass()============" << endl;
	kdDebug() << "\tset class: " << m_currentClass << endl;

	// get the stringlist of this class with all information
	QStringList classlist = m_dictDocumentClasses[m_currentClass];

	// First of all, we have to set the defaultOptions-dictionary and the
	// selectedOptions-dictionary for this class, before inserting options
	// into the listview. The function setClassOptions() will look
	// into both dictionaries to do some extra work.
	setDefaultClassOptions( classlist[qd_DefaultOptions] );
	setSelectedClassOptions( classlist[qd_SelectedOptions] );

	// set comboboxes for fontsizes and papersizes
	fillCombobox(m_cbTypefaceSize,classlist[qd_Fontsizes],m_currentFontsize);
	fillCombobox(m_cbPaperSize,classlist[qd_Papersizes],m_currentPapersize);

	// now we are ready to set the class options
	if ( isStandardClass(m_currentClass) ) {
		QStringList optionlist;
		initStandardOptions(m_currentClass,optionlist);
		setClassOptions(optionlist,0);
	} else {
		setClassOptions(classlist,qd_OptionsStart);
	}

	// there is no papersize with class beamer, but a theme
	if ( m_currentClass == "beamer" )
		m_lbPaperSize->setText( i18n("&Theme:") );
	else
		m_lbPaperSize->setText( i18n("Paper si&ze:") );

	// enable/disable buttons to add or delete entries
	slotEnableButtons();
}

void QuickDocument::initStandardClass(const QString &classname,
                                      const QString &fontsize, const QString &papersize,
                                      const QString &defaultoptions, const QString &selectedoptions)
{
	kdDebug() << "\tinit standard class: " << classname << endl;

	// remember that this is a standard class
	m_dictStandardClasses[ classname ]  =  true;

	// save all entries
	QStringList list;
	list << fontsize << papersize << defaultoptions << selectedoptions;

	// save in documentClass-dictionary
	m_dictDocumentClasses[ classname ] = list;
}

// build all option for the current standard class
void QuickDocument::initStandardOptions(const QString &classname,QStringList &optionlist)
{
	// build the bitcode for all options of this class
	int options;
	if ( classname == "article" )
		options = qd_Base + qd_Article;
	else if ( classname == "book" )
		options = qd_Base + qd_Article + qd_BookReport;
	else if ( classname == "letter" )
		options = qd_Base;
	else if ( classname == "report" )
		options = qd_Base + qd_Article + qd_BookReport;
	else if ( classname == "scrartcl" )
		options = qd_Base + qd_Article + qd_KomaArticle + qd_KomaAbstract;
	else if ( classname == "scrbook" )
		options = qd_Base + qd_Article + qd_BookReport + qd_KomaArticle + qd_KomaBookReport;
	else if ( classname == "scrreprt" )
		options = qd_Base + qd_Article + qd_BookReport + qd_KomaArticle + qd_KomaAbstract + qd_KomaBookReport;
	else if ( classname == "prosper" )
		options = qd_Prosper;
	else if ( classname == "beamer" )
		options = qd_Beamer;
	else
		return;

	// insert all options into the list
	if ( options & qd_Base ) {
		optionlist
			<< QString("landscape => ") + i18n("Sets the document's orientation to landscape")
			<< QString("oneside => ") + i18n("Margins are set for single side output")
			<< QString("twoside => ") + i18n("Left and right pages differ in page margins")
			<< QString("draft => ") + i18n("Marks \"overfull hboxes\" on the output with black boxes")
			<< QString("final => ") + i18n("No special marks for \"overfull hboxes\" on the output")
			<< QString("leqno => ") + i18n("Puts formula numbers on the left side")
			<< QString("fleqn => ") + i18n("Aligns formulas on the left side")
			;
	}

	if ( options & qd_Article ) {
		optionlist
			<< QString("titlepage => ") + i18n("Puts title and abstract on an extra page")
			<< QString("notitlepage => ") + i18n("Puts title and abstract on the same page as the text")
			<< QString("onecolumn => ") + i18n("Puts the text in one column")
			<< QString("twocolumn => ") + i18n("Puts the text in two columns")
			<< QString("openbib => ") + i18n("Formats the bibliography in open style")
			;
	}

	if ( options & qd_BookReport ) {
		optionlist
			<< QString("openany => ") + i18n("Chapters may start on top of every page")
			<< QString("openright => ") + i18n("Chapters may only start on top of right pages")
			;
	}

	if ( options & qd_KomaArticle ) {
		optionlist
			<< QString("headinclude => ") + i18n("Cause the header to be counted as text")
			<< QString("headexclude => ") + i18n("Cause the header to be counted as border")
			<< QString("footinclude => ") + i18n("Cause the footer to be counted as text")
			<< QString("footexclude => ") + i18n("Cause the footer to be counted as border")
			<< QString("mpinclude => ") + i18n("Cause the margin-note to be counted to the text body")
			<< QString("mpexclude => ") + i18n("The normal margin is used for the margin-note area")
			<< QString("dvips => ") + i18n("Writes the paper size as a special into the DVI-file")
			<< QString("pdftex => ") + i18n("Writes the paper size into the pdftex page register")
			<< QString("pagesize => ") + i18n("Uses the correct mechanism with PDF- or DVI-file")
			<< QString("cleardoubleempty => ") + i18n("Enables the default for an empty left page")
			<< QString("cleardoubleplain => ") + i18n("An empty left page will set with the plain-pagestyle")
			<< QString("cleardoublestandard => ") + i18n("An empty left page will set with the empty-pagestyle")
			<< QString("headsepline => ") + i18n("Use a line to separate the header from the text body")
			<< QString("headnosepline => ") + i18n("Use no line to separate the header from the text body")
			<< QString("footsepline => ") + i18n("Use a line to separate the footer from the text body")
			<< QString("footnosepline => ") + i18n("Use no line to separate the footer from the text body")
			<< QString("parskip => ") + i18n("Normal paragraph spacing of one line")
			<< QString("parskip- => ") + i18n("Normal spacing, at least 1/3 of the last line is free")
			<< QString("parskip+ => ") + i18n("Normal spacing, at least 1/4 of the last line is free")
			<< QString("parskip* => ") + i18n("Normal spacing, no special provision for the last line")
			<< QString("halfparskip => ") + i18n("Paragraph spacing of half a line")
			<< QString("halfparskip- => ") + i18n("Spacing 1/2 line, at least 1/3 of the last line is free")
			<< QString("halfparskip+ => ") + i18n("Spacing 1/2 line, at least 1/4 of the last line is free")
			<< QString("halfparskip* => ") + i18n("Spacing 1/2 line, no special provision for the last line")
			<< QString("parindent => ") + i18n("No spacing between paragraphs, indent the first line by 1 em")
			<< QString("onelinecaption => ") + i18n("One-line captions are centered, multi-line left-justified")
			<< QString("noonelinecaption => ") + i18n("No special handling of one-line captions")
			<< QString("bigheading => ") + i18n("Normal great title font sizes")
			<< QString("normalheadings => ") + i18n("Small font sizes for titles")
			<< QString("smallheadings => ") + i18n("Even smaller font sizes for titles")
			<< QString("liststotoc => ") + i18n("Include lists of figures and tables in the TOC")
			<< QString("bibtotoc => ") + i18n("Include the bibliography in the TOC")
			<< QString("idxtotoc => ") + i18n("Include the index in the TOC")
			<< QString("liststotocnumbered => ") + i18n("Number the lists of figures and tables in the TOC")
			<< QString("bibtotocnumbered => ") + i18n("Number the bibliography in the TOC")
			<< QString("tocleft => ") + i18n("All numbers and titles are set in a left-justified column")
			<< QString("tocindent => ") + i18n("Different sectional units have different indentations")
			<< QString("listsleft => ") + i18n("All numbers and captions are set in a left-justified column")
			<< QString("listsindent => ") + i18n("All Numbers uses a fixed space")
			<< QString("pointednumbers => ") + i18n("Numbering of sectional units have a point at the end")
			<< QString("pointlessnumbers => ") + i18n("Numbering of sectional units have no point at the end")
			<< QString("tablecaptionabove => ") + i18n("Caption command acts like \\captionabove")
			<< QString("tablecaptionbelow => ") + i18n("Caption command acts like \\captionbelow")
			<< QString("origlongtable => ") + i18n("Captions of the longtable package should not be redefined")
			;
	}

	if ( options & qd_KomaBookReport ) {
		optionlist
			<< QString("chapterprefix => ") + i18n("Use a separate line for the chapter number")
			<< QString("nochapterprefix => ") + i18n("Use the same line for the chapter number and title")
			<< QString("appendixprefix => ") + i18n("Use a separate line for the appendix name")
			<< QString("noappendixprefix  => ") + i18n("No separate line for the appendix name")
			;
	}

	if ( options & qd_KomaAbstract ) {
		optionlist
			<< QString("abstracton => ") + i18n("Include the abstract's title")
			<< QString("abstractoff => ") + i18n("Exclude the abstract's title")
			;
	}

	if ( options & qd_Prosper ) {
		optionlist
			<< QString("draft => ") + i18n("The file is compiled in draft mode")
			<< QString("final => ") + i18n("The file is compiled in final mode")
			<< QString("slideColor => ") + i18n("Slides will use many colors")
			<< QString("slideBW => ") + i18n("Slides will use a restricted set of colors")
			<< QString("total => ") + i18n("Display the number of the current slide and the total number")
			<< QString("nototal => ") + i18n("Display only the number of the current slide")
			<< QString("nocolorBG => ") + i18n("The background of the slide is always white")
			<< QString("colorBG => ") + i18n("The color of the background depends on the current style")
			<< QString("ps => ") + i18n("The LaTeX file is compiled to produce a PostScript file")
			<< QString("pdf => ") + i18n("The LaTeX file is compiled to produce a PDF file")
			<< QString("accumulate => ") + i18n("Some macros interpret their argument in ps mode")
			<< QString("noaccumulate => ") + i18n("Some macros do not interpret their argument in ps mode")
			<< QString("distiller => ") + i18n("The PS file is to be translated into a PDF file using Adobe Distiller")
			<< QString("YandY => ") + i18n("The LaTeX file is to be processed with YandY LaTeX")
			<< QString("ps2pdf => ") + i18n("The PS file is to be translated into a PDF file using ps2pdf")
			<< QString("vtex => ") + i18n("The LaTeX file is to be processed with MicroPress VTeX")
			<< QString("noFooter => ") + i18n("Do not add any caption at the bottom of the slides")
			;
	}

	if ( options & qd_Beamer ) {
		optionlist
			<< QString("slidestop => ") + i18n("Place text of slides at the (vertical) top of the slides")
			<< QString("slidescentered => ") + i18n("Place text of slides at the (vertical) center of the slides")
			<< QString("draft => ") + i18n("Headlines, footlines, and sidebars are replaced by gray rectangles")
			<< QString("compress => ") + i18n("Make all navigation bars as small as possible")
			<< QString("usepdftitle=false => ") + i18n("Suppresses generation of some entries in the pdf information")
			<< QString("notheorems => ") + i18n("Switches off the definition of default blocks like theorem")
			<< QString("noamsthm => ") + i18n("Does not load amsthm and amsmath")
			<< QString("CJK => ") + i18n("Needed when using the CJK package for Asian fonts")
			<< QString("sans => ") + i18n("Use a sans-serif font during the presentation")
			<< QString("serif => ") + i18n("Use a serif font during the presentation")
			<< QString("mathsans => ") + i18n("Override the math font to be a sans-serif font")
			<< QString("mathserif => ") + i18n("Override the math font to be a serif font")
			<< QString("professionalfont => ") + i18n("Deactivate internal font replacements for math text")
			<< QString("handout => ") + i18n("Create a PDF handout")
			<< QString("trans => ") + i18n("For PDF transparency")
			<< QString("blue => ") + i18n("All structure elements are typeset in blue")
			<< QString("red => ") + i18n("All structure elements are typeset in red")
			<< QString("blackandwhite => ") + i18n("All structure elements are typeset in black and white")
			<< QString("brown => ") + i18n("All structure elements are typeset in brown")
			<< QString("notes=hide => ") + i18n(" Notes are not shown")
			<< QString("notes=show => ") + i18n(" Include notes in the output file")
			<< QString("notes=only => ") + i18n(" Include only notes and suppress frames")
			;
	}
}

// check for a standard class
bool QuickDocument::isStandardClass(const QString &classname)
{
	return m_dictStandardClasses.contains(classname);
}

// check for a default option
bool QuickDocument::isDefaultClassOption(const QString &option)
{
	return m_currentDefaultOptions.contains(option);
}

// check for an user option
bool QuickDocument::isSelectedClassOption(const QString &option)
{
	return m_currentSelectedOptions.contains(option);
}

// insert all default options of the current class into the defaultOptions-dictionary
void QuickDocument::setDefaultClassOptions(const QString &defaultoptions)
{
	QStringList list = QStringList::split(",",defaultoptions);
	m_currentDefaultOptions.clear();
	for ( uint i=0; i<list.count(); ++i ) {
		if ( ! list[i].isEmpty() )
			m_currentDefaultOptions[ list[i] ] = true;
	}
}

// insert all checked options of the current class into the selectedOptions-dictionary
void QuickDocument::setSelectedClassOptions(const QString &selectedoptions)
{
	kdDebug() << "\tset options: " << selectedoptions << endl;

	QStringList list = QStringList::split(",",selectedoptions);
	uint nlist = list.count();

	m_currentFontsize  = ( nlist >= 1 ) ? list[0] : "";
	m_currentPapersize = ( nlist >= 2 ) ? list[1] : "";

	m_currentSelectedOptions.clear();
	for ( uint i=0; i<nlist; ++i ) {
		if ( ! list[i].isEmpty() )
			m_currentSelectedOptions[ list[i] ] = true;
	}
}

// show all options of the current class
//  - split this string into option and description (option => description)
//  - if the option is in the defaultOptions-dictionary, add 'default'
//  - if the option is in the selectedOptions-dictionary, set the 'checked' symbol
void QuickDocument::setClassOptions(const QStringList &list, uint start)
{
	QRegExp reg("(\\S+)\\s+=>\\s+(.*)");

	m_lvClassOptions->clear();
	for (uint i=start; i<list.count(); ++i) {
		int pos = reg.search( list[i] );
		if ( pos != -1 ) {
			QCheckListItem *cli = new QCheckListItem(m_lvClassOptions, reg.cap(1), QCheckListItem::CheckBox);

			// see if it is a default option
			if ( isDefaultClassOption(reg.cap(1)) )
				cli->setText(1, reg.cap(2)+" [default]");
			else
				cli->setText(1, reg.cap(2));

			// check it if this option is set by th user
			if ( isSelectedClassOption(reg.cap(1)) )
				cli->setOn(true);
		}
	}
}

// get all options of the current class as a comma separated list
//  - first entry: always the current fontsize
//  - second entry: always the current papersize
//  - followed by all other checked options
QString QuickDocument::getClassOptions()
{
	QString fontsize = stripDefault( m_cbTypefaceSize->currentText() );
	QString papersize = stripDefault( m_cbPaperSize->currentText() );

	QString options =  fontsize + ',' + papersize;

	for (QListViewItem *cur = m_lvClassOptions->firstChild(); cur; cur=cur->nextSibling()) {
		QCheckListItem *cli = dynamic_cast<QCheckListItem*>(cur);
		if (cli && cli->isOn()) {
			options += ',' + cur->text(0);
		}
	}

	return options;
}

// Some changes were made in the listview: add, edit oder delete entries.
// This means that the defaultOptions-dictionary, the selectedOptions-dictionary
// and the list of all options may be                                                                     . So the documentClass-dictionary,
// the defaultOptions-dictionary and the selectedOptions-dictionary must be updated.
void QuickDocument::updateClassOptions()
{
	kdDebug() << "==QuickDocument::updateClassOptions()============" << endl;
	kdDebug() << "\tclass: " << m_currentClass << endl;

	QString defaultoptions;
	QStringList newlist;
	QStringList oldlist = m_dictDocumentClasses[m_currentClass];

	// read the first four static entries
	newlist << oldlist[qd_Fontsizes];
	newlist << oldlist[qd_Papersizes];
	newlist << QString::null;        // dummy entry: will be changed
	newlist << getClassOptions();

	// read all options
	for (QListViewItem *cur = m_lvClassOptions->firstChild(); cur; cur=cur->nextSibling()) {
		QCheckListItem *cli = dynamic_cast<QCheckListItem*>(cur);
		if ( cli ) {
			QString description = cur->text(1);
			if ( description.right(10) == " [default]" ) {
				description = stripDefault(description);
				if ( ! defaultoptions.isEmpty() )
					defaultoptions += ',';
				defaultoptions += cur->text(0);
			}
			newlist += cur->text(0) + " => " + description;
		}
	}

	// update list entry with defaultoptions
	newlist[qd_DefaultOptions] = defaultoptions;

	// insert this changed list into the documentClass-dictionary
	m_dictDocumentClasses[m_currentClass] = newlist;

	// update other dictionaries
	setDefaultClassOptions(newlist[qd_DefaultOptions]);
	setSelectedClassOptions(newlist[qd_SelectedOptions]);
}


// Insert all entries from a comma separated list into a combobox.
// If this entry matches a given text, this entry will be activated.
void QuickDocument::fillCombobox(KComboBox *combo, const QString &cslist, const QString &seltext)
{
	bool documentclasscombo = ( combo == m_cbDocumentClass );
	QListBox *listbox = combo->listBox();

	QString sep = ( m_currentClass=="beamer" && combo==m_cbPaperSize ) ? ";" : ",";
	QStringList list = QStringList::split(sep,cslist);
	if ( ! documentclasscombo )
		list.sort();

	combo->clear();
	for (uint i=0; i<list.count(); ++i) {
		if ( !documentclasscombo &&  isDefaultClassOption(list[i]) )
			combo->insertItem( QString(list[i]) + " [default]" );
		else if ( list[i] != "-" )
			combo->insertItem( list[i] );
		else
        {
            ListBoxSeparator *separator = new ListBoxSeparator(listbox->item(0)->height(listbox));
			listbox->insertItem(separator);
			// doesn't work in constructor, so set it again here
			separator->setSelectable(false);
		}

		// should this entry be selected?
		if ( !seltext.isEmpty() && list[i]==seltext )
			combo->setCurrentItem(i);
	}
}

// Add some entries from a comma separated list to a sorted combobox.
// The new entries must match a regular expression or will be denied.
bool QuickDocument::addComboboxEntries(KComboBox *combo, const QString &title,const QString &entry)
{
	// read current comboxbox entries
	QStringList combolist;
	for (int i=0; i<combo->count(); ++i)
		combolist += combo->text(i);

	// add new entries (one or a comma separated list)
	QStringList list = QStringList::split(",",entry);
	for ( uint i=0; i<list.count(); ++i ) {
		QString s = list[i].stripWhiteSpace();
		// entries must match a regular expression
		if ( combolist.findIndex(s) != -1 )
			KMessageBox::error( this, i18n("%1 '%2' already exists.").arg(title).arg(s) );
		else {
			combolist += s;
			kdDebug() << "\tinsert new " << title << ": " << s << endl;
		}
	}

	// insert list, if there are more entries than before
	if ( combolist.count() > (uint)combo->count() ) {
		fillCombobox(combo,combolist.join(","),list[0]);
		return true;
	} else {
		return false;
	}

}

QString QuickDocument::getComboxboxList(KComboBox *combo)
{
	QStringList list;
	for ( int i=0; i<combo->count(); ++i ) {
		list += combo->text(i);
	}

	return ( list.count() > 0 ) ? list.join(",") : QString::null;
}

// strip an optional default-tag from the string
QString QuickDocument::stripDefault(const QString &s)
{
	return ( s.right(10) == " [default]" ) ? s.left( s.length()-10 ) : s;
}

////////////////////////////// packages tab //////////////////////////////

void QuickDocument::readPackagesConfig()
{
	kdDebug() << "\tread config: packages" << endl;

	if ( ! readPackagesListview() )
		initPackages();
}

// init default values for packages tab
void QuickDocument::initPackages()
{
	kdDebug() << "read config: init standard packages" << endl;
	QCheckListItem *cli;
	QCheckListItem *clichild;

	m_lvPackages->clear();
	cli = insertListview(m_lvPackages,"amsmath", i18n("Special math environments and commands (AMS)") );
	cli = insertListview(m_lvPackages,"amsfonts",i18n("Collection of fonts and symbols for math mode (AMS)") );
	cli = insertListview(m_lvPackages,"amssymb",i18n("Defines symbol names for all math symbols in MSAM and MSBM (AMS)") );
	cli = insertListview(m_lvPackages,"amsthm",i18n("Improved theorem setup (AMS)"));
	cli = insertListview(m_lvPackages,"caption",i18n("Extends caption capabilities for figures and tables"));

	cli = insertListview(m_lvPackages,"hyperref",i18n("Hypertext marks in LaTeX") );
	cli->setOpen(true);
	clichild = insertListview(cli,"dvips",i18n("Use dvips as hyperref driver") );
	clichild->setOn(true);
	clichild = insertListview(cli,"pdftex",i18n("Use pdftex as hyperref driver") );
	clichild = insertEditableListview(cli,"bookmarks",i18n("Make bookmarks"),"true","true" );
	clichild = insertEditableListview(cli,"bookmarksnumbered",i18n("Put section numbers in bookmarks"),"false","false" );
	clichild = insertEditableListview(cli,"bookmarksopen",i18n("Open up bookmark tree"),QString::null,QString::null );
	clichild = insertEditableListview(cli,"pdfauthor",i18n("Text for PDF Author field"),QString::null,QString::null );
	clichild = insertEditableListview(cli,"pdfcreator",i18n("Text for PDF Creator field"),i18n("LaTeX with hyperref package"),i18n("LaTeX with hyperref package") );
	clichild = insertEditableListview(cli,"pdffitwindow",i18n("Resize document window to fit document size"),"false","false" );
	clichild = insertEditableListview(cli,"pdfkeywords",i18n("Text for PDF Keywords field"),QString::null,QString::null );
	clichild = insertEditableListview(cli,"pdfproducer",i18n("Text for PDF Producer field"),QString::null,QString::null );
	clichild = insertEditableListview(cli,"pdfstartview",i18n("Starting view of PDF document"),"/Fit","/Fit" );
	clichild = insertEditableListview(cli,"pdfsubject",i18n("Text for PDF Subject field"),QString::null,QString::null );
	clichild = insertEditableListview(cli,"pdftitle",i18n("Text for PDF Title field"),QString::null,QString::null );

	cli = insertListview(m_lvPackages,"mathpazo",i18n("Use Palatino font as roman font (both text and math mode)") );
	cli = insertListview(m_lvPackages,"mathptmx",i18n("Use Times font as roman font (both text and math mode)") );
	cli = insertListview(m_lvPackages,"makeidx",i18n("Enable index generation") );
	cli = insertListview(m_lvPackages,"multicol",i18n("Enables multicolumn environments") );
	cli = insertListview(m_lvPackages,"pst-all",i18n("Load all pstricks packages") );
	cli = insertListview(m_lvPackages,"rotating",i18n("Rotates text") );
	cli = insertListview(m_lvPackages,"subfigure",i18n("Enables subfigures inside figures") );
	cli = insertListview(m_lvPackages,"upgreek",i18n("Typesetting capital Greek letters") );
	cli = insertListview(m_lvPackages,"xcolor",i18n("Extending LaTeX's color facilities") );

	cli = insertListview(m_lvPackages,"babel",i18n("Adds language specific support") );
	cli->setOn(true);
	cli->setOpen(true);
	clichild = new QCheckListItem(cli,"acadian" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"afrikaans" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"american" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"australian" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"austrian" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"bahasa" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"basque" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"brazil" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"brazilian" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"breton" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"british" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"bulgarian" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"canadian" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"canadien" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"catalan" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"croatian" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"czech" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"danish" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"dutch" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"english" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"esperanto" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"estonian" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"finnish" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"francais" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"frenchb" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"french" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"galician" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"german" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"germanb" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"greek" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"polutonikogreek" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"hebrew" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"hungarian" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"icelandic" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"interlingua" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"irish" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"italian" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"latin" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"lowersorbian" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"magyar" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"naustrian" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"newzealand" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"ngerman" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"norsk" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"samin" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"nynorsk" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"polish" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"portuges" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"portuguese" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"romanian" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"russian" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"scottish" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"serbian" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"slovak" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"slovene" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"spanish" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"swedish" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"turkish" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"ukrainian" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"uppersorbian" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"welsh" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"UKenglish" ,QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli,"USenglish" ,QCheckListItem::CheckBox);

	cli = insertListview(m_lvPackages,"fontenc",i18n("Use a font encoding scheme") );
	cli->setOn(true);
	cli->setOpen(true);
	clichild = new QCheckListItem(cli, "HE8",QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "IL2",QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "LCH",QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "LCY",QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "LGR",QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "LHE",QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "LIT",QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "LO1",QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "LY1",QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "MTT",QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "OML",QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "OMS",QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "OT1",QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "OT2",QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "OT4",QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "PD1",QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "PU",QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "QX",QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "T1",QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "T2A",QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "T2B",QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "T2C",QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "T5",QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "TS1",QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "UT1",QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "X2",QCheckListItem::CheckBox);

	cli = insertListview(m_lvPackages,"graphicx",i18n("Support for including graphics") );
	cli->setOn(true);
	cli->setOpen(true);
	clichild = insertListview(cli,"dvips",i18n("Specialize on graphic inclusion for dvips") );
	clichild = insertListview(cli,"pdftex",i18n("Specialize on graphic inclusion for pdftex") );
	clichild = insertListview(cli,"draft",i18n("Show only frames of graphics") );
}

// Try to read values from the config file:
//  - main entry:  selected,open,empty,empty,description
//  - child entry: selected,editable,defaultvalue,value,description

bool QuickDocument::readPackagesListview()
{
	kdDebug() << "\tread config: packages from config file" << endl;

	QStringList elements = KileConfig::packagesList();

	// clear packages dictionaries and listview
	m_dictPackagesEditable.clear();
	m_dictPackagesDefaultvalues.clear();
	m_lvPackages->clear();

	if ( elements.empty() )
		return false;

	// regular expression to split the string from the config file
	QRegExp reg("([^,]*),([^,]*),([^,]*),([^,]*),(.*)");

	m_config->setGroup( "QuickDocument/Packages" );
	for ( QStringList::Iterator it=elements.begin(); it!=elements.end(); ++it ) {
		QCheckListItem *cli;

		// look, if this is a main or a child entry
		kdDebug() << "\tread config entry: " << *it << endl;
		int pos = (*it).find('!');
		if ( pos == -1 ) {                    // main entry
			cli = new QCheckListItem(m_lvPackages, *it, QCheckListItem::CheckBox);
			if ( reg.exactMatch(m_config->readEntry(*it)) ) {
				if ( reg.cap(1) == "1" )        // selected state (entry 1)
					cli->setOn(true);
				if ( reg.cap(2) == "1" )        // open state (entry 2)
					cli->setOpen(true);
				cli->setText(2,reg.cap(5));     // description (entry 5)
			} else {
				kdDebug() << "\twrong config entry for package " << cli->text(0) << endl;
			}
		} else {                              // child entry
			cli = dynamic_cast<QCheckListItem*>(m_lvPackages->findItem((*it).left(pos), 0));
			if ( cli ) {
				QCheckListItem *clichild;
				if ( reg.exactMatch(m_config->readEntry(*it)) ) {
					if ( reg.cap(2) == "1" ) {                                     // editable state
						clichild = insertEditableListview(cli,(*it).mid(pos+1),reg.cap(5),reg.cap(4),reg.cap(3) );
					} else {
						clichild = new QCheckListItem(cli, (*it).mid(pos+1), QCheckListItem::CheckBox);
						clichild->setText(2,reg.cap(5));                            // description
					}
					if ( reg.cap(1) == "1" )                                       // selected state
						clichild->setOn(true);
				} else {
					kdDebug() << "\twrong config entry for package option " << cli->text(0) << endl;
				}
			} else {
				kdDebug() << "\tlistview entry for package " << (*it).left(pos) << " not found" << endl;
			}
		}
	}

	return true;
}

void QuickDocument::writePackagesConfig()
{
	kdDebug() << "\twrite config: packages" << endl;

	QStringList packagesList;

	m_config->setGroup( "QuickDocument/Packages" );
	for (QListViewItem *cur=m_lvPackages->firstChild(); cur; cur=cur->nextSibling()) {
		kdDebug() << "\twrite config: " << cur->text(0) << endl;
		// add to packages list
		packagesList += cur->text(0);

		// determine config entry
		QString packageentry;

		// look for selected entries
		QCheckListItem *cli = dynamic_cast<QCheckListItem*>(cur);
		if ( cli && cli->isOn() )
			packageentry = "1,";
		else
			packageentry = "0,";

		// look if this listitem is opened
		if ( cli && cli->isOpen() )
			packageentry += "1,";
		else
			packageentry += "0,";

		// two dummy entries and finally the description
		packageentry += ",," + cur->text(2);

		// write listview entry
		m_config->writeEntry(cur->text(0),packageentry);

		// look for children
		for (QListViewItem *curchild=cur->firstChild(); curchild; curchild=curchild->nextSibling()) {
			// add child to packages list
			QString option = cur->text(0) + '!' + curchild->text(0);
			packagesList += option;
			kdDebug() << "\twrite config: " << option << endl;

			// determine config entry
			QString optionentry;

			// look for selected options
			QCheckListItem *clichild = dynamic_cast<QCheckListItem*>(curchild);
			if ( clichild && clichild->isOn() )
				optionentry = "1,";
			else
				optionentry = "0,";

			// look, if this child is editable
			if ( clichild && m_dictPackagesEditable.contains(option) ) {
				optionentry += "1,";
				if ( m_dictPackagesDefaultvalues.contains(option) )
					optionentry += m_dictPackagesDefaultvalues[option] + ',';
				else
					optionentry += ',';
			} else
				optionentry += "0,,";

			// add a value and a description
			optionentry += getPackagesValue(curchild->text(1))
			                    + ',' + stripPackageDefault(option,curchild->text(2));

			// write listview entry
			m_config->writeEntry(option,optionentry);
		}
	}

	// write the list of all packages
	KileConfig::setPackagesList(packagesList);
}

// insert package
QCheckListItem *QuickDocument::insertListview(QListView *listview,
                                              const QString &entry,
                                              const QString &description)
{
	QCheckListItem *item = new QCheckListItem(listview,entry,QCheckListItem::CheckBox);
	if ( ! description.isEmpty() )
		item->setText(2,description);

	return item;
}

// insert package option (not editable)
QCheckListItem *QuickDocument::insertListview(QCheckListItem *parent,
                                              const QString &entry,
                                              const QString &description)
{
	QCheckListItem *item = new QCheckListItem(parent,entry,QCheckListItem::CheckBox);
	if ( ! description.isEmpty() )
		item->setText(2,description);

	return item;
}

// insert package option (editable)
QCheckListItem *QuickDocument::insertEditableListview(QCheckListItem *parent,
	                                       const QString &entry,const QString &description,
	                                       const QString value,const QString defaultvalue)
{
	QCheckListItem *item = new EditableCheckListItem(parent,entry);
	QString option = parent->text(0) + '!' + entry;
	m_dictPackagesEditable[option] = true;
	if ( ! defaultvalue.isEmpty() )
		m_dictPackagesDefaultvalues[option] = defaultvalue;
	setPackagesValue(item,option,value);
	if ( ! description.isEmpty() )
		item->setText( 2,addPackageDefault(option,description) );

	return item;
}

void QuickDocument::setPackagesValue(QListViewItem *item,const QString &option,const QString &val)
{
	QString defaultvalue = ( m_dictPackagesDefaultvalues.contains(option) )
	                         ? m_dictPackagesDefaultvalues[option] : QString::null;
	QString value = ( ! val.isEmpty() ) ? val : QString::null;

	if ( value == defaultvalue )
		item->setText(1,i18n("<default>") );
	else if ( value.isEmpty() )
		item->setText(1,i18n("<empty>") );
	else
		item->setText(1,value);
}

QString QuickDocument::getPackagesValue(const QString &value)
{
	return ( value==i18n("<default>") || value==i18n("<empty>") ) ? QString::null : value;
}


bool QuickDocument::isListviewEntry(QListView *listview,const QString &entry)
{
	for ( QListViewItem *cur=listview->firstChild(); cur; cur=cur->nextSibling() ) {
		if ( cur->text(0) == entry )
			return true;
	}

	return false;
}

bool QuickDocument::isListviewChild(QListView *listview,const QString &entry, const QString &option)
{
	for ( QListViewItem *cur=listview->firstChild(); cur; cur=cur->nextSibling() ) {
		// look for the main entry
		if ( cur->text(0) == entry ) {
			// look for children
			for (QListViewItem *curchild=cur->firstChild(); curchild; curchild=curchild->nextSibling()) {
				if ( option == curchild->text(0) )
					return true;
			}
			return false;
		}
	}

	return false;
}

QString QuickDocument::addPackageDefault(const QString &option,const QString &description)
{
	return ( m_dictPackagesDefaultvalues.contains(option) )
	       ? description + " [" + m_dictPackagesDefaultvalues[option] + ']'
			 : description + " [ ]";
}

QString QuickDocument::stripPackageDefault(const QString &option,const QString &description)
{
	QRegExp reg("(.*) \\[([^\\[]*)\\]");

	if ( description.right(4) == " [ ]" )
		return description.left(description.length()-4);

	if ( ! reg.exactMatch(description) )
		return description;

	return ( reg.cap(2).isEmpty() ||
		     ( m_dictPackagesDefaultvalues.contains(option) && m_dictPackagesDefaultvalues[option]==reg.cap(2) )
		    ) ? reg.cap(1) : description;
}

////////////////////////////// hyperref tab //////////////////////////////

void QuickDocument::initHyperref()
{
	kdDebug() << "\tread config: init hyperref" << endl;


	QString driver =  "dvipdf,dvipdfm,dvips,dvipsone,"
	                  "dviwindo,hypertex,latex2html,pdftex,"
	                  "ps2pdf,tex4ht,textures,vtex";
	QStringList list = QStringList::split(",",driver);

	m_dictHyperrefDriver.clear();
	for ( uint i=0; i<list.count(); ++i )
		m_dictHyperrefDriver[list[i]] = true;
}

bool QuickDocument::isHyperrefDriver(const QString &name)
{
	return m_dictHyperrefDriver.contains(name);
}

////////////////////////////// check for existing exntries //////////////////////////////

bool QuickDocument::isDocumentClass(const QString &name)
{
	for ( int i=0; i<m_cbDocumentClass->count(); ++i ) {
		if ( m_cbDocumentClass->text(i) == name )
			return true;
	}
	return false;
}

bool QuickDocument::isDocumentClassOption(const QString &option)
{
	return isListviewEntry(m_lvClassOptions,option);
}

bool QuickDocument::isPackage(const QString &package)
{
	return isListviewEntry(m_lvPackages,package);
}

bool QuickDocument::isPackageOption(const QString &package, const QString &option)
{
	return isListviewChild(m_lvPackages,package,option);
}


////////////////////////////// print document template //////////////////////////////

void QuickDocument::printTemplate()
{
	kdDebug() << "==QuickDocument::printTemplate()============" << endl;

	// get current document class
	QString documentclass = m_cbDocumentClass->currentText();
	kdDebug() << "\tdocument class: " << documentclass << endl;

	// build template
	m_td.tagBegin = "\\documentclass";

	// build options
	QString options;
	if ( documentclass != "beamer" ) {
		if ( !m_cbPaperSize->currentText().isEmpty() )
			options += stripDefault( m_cbPaperSize->currentText() ) + ',';
	}

	if ( !m_cbTypefaceSize->currentText().isEmpty() )
		options += stripDefault( m_cbTypefaceSize->currentText() ) + ',';

	for (QListViewItem *cur=m_lvClassOptions->firstChild(); cur; cur=cur->nextSibling()) {
		QCheckListItem *cli=dynamic_cast<QCheckListItem*>(cur);
		if ( cli && cli->isOn() )
			options += cur->text(0) + ',';
	}

	if ( ! options.isEmpty() )
		m_td.tagBegin += '[' + options.left( options.length()-1 ) + ']';
	m_td.tagBegin += '{' + documentclass + "}\n\n";

	
	QString enc = m_cbEncoding->currentText();
	if (!enc.isEmpty())
	{
		if( enc.find("utf") != -1 )
			m_td.tagBegin += "\\usepackage{ucs}\n";
		m_td.tagBegin += "\\usepackage[" + enc + "]{inputenc}\n";
	}
	if ( documentclass != "beamer" ) {
		printPackages();
		printHyperref();
	} else {
		printBeamerTheme();
		printPackages();
	}

	if (!m_leAuthor->text().isEmpty())
		m_td.tagBegin += "\\author{"+m_leAuthor->text()+"}\n";
	if (!m_leTitle->text().isEmpty())
		m_td.tagBegin += "\\title{"+m_leTitle->text()+"}\n";
	if (!m_leDate->text().isEmpty())
		m_td.tagBegin += "\\date{"+m_leDate->text()+"}\n";
	m_td.tagBegin += '\n';

	m_td.tagBegin += "\\begin{document}\n%E%C";

	m_td.tagEnd = "\n\\end{document}\n";
}

void QuickDocument::printPackages()
{
	kdDebug() << "\tpackages" << endl;

	m_currentHyperref = false;
	m_hyperrefdriver = QString::null;
	m_hyperrefsetup = QString::null;

	for (QListViewItem *cur=m_lvPackages->firstChild(); cur; cur=cur->nextSibling()) {
		QCheckListItem *cli = dynamic_cast<QCheckListItem*>(cur);
		if ( ! cli )
			continue;

		if ( cur->text(0) == "hyperref" ) {          // manage hyperref package
			m_currentHyperref = cli->isOn();
			for (QListViewItem *curchild = cur->firstChild(); curchild; curchild=curchild->nextSibling()) {
				QCheckListItem *clichild = dynamic_cast<QCheckListItem*>(curchild);
				if (clichild && clichild->isOn() ) {              // manage hyperref option
					if ( isHyperrefDriver(curchild->text(0)) ) {   // either hyperref driver
						if ( ! m_hyperrefdriver.isEmpty() )
							m_hyperrefdriver += ',';
						m_hyperrefdriver += curchild->text(0);
					} else {
						QString value = curchild->text(1);          // or another option
						if ( value != i18n("<default>") ) {
							if ( ! m_hyperrefsetup.isEmpty() )
								m_hyperrefsetup += ',';
							m_hyperrefsetup += "%\n   " + curchild->text(0) + '=' + getPackagesValue(curchild->text(1));
						}
					}
				}
			}
		} else if ( cli->isOn() ) {                   // manage other package options
			QString packageOptions;
			for (QListViewItem *curchild = cur->firstChild(); curchild; curchild=curchild->nextSibling()) {
				QCheckListItem *clichild = dynamic_cast<QCheckListItem*>(curchild);
				if (clichild && clichild->isOn()) {
					QString optiontext;
					if ( m_dictPackagesEditable.contains(cur->text(0) + '!' + curchild->text(0)) ) {
						QString value = curchild->text(1);
						if ( value != i18n("<default>") )
							optiontext = curchild->text(0) + '=' + getPackagesValue(curchild->text(1));
					} else
						optiontext = curchild->text(0);

					if ( ! optiontext.isEmpty() ) {
						if (!packageOptions.isEmpty())
							packageOptions += ',';
						packageOptions += optiontext;
					}
				}
			}

			m_td.tagBegin += "\\usepackage";
			if (!packageOptions.isEmpty())
				m_td.tagBegin += '[' + packageOptions + ']';
			m_td.tagBegin += '{' + cur->text(0) + "}\n";
		}
	}
	m_td.tagBegin += '\n';
}

void QuickDocument::printHyperref()
{
	if ( ! m_currentHyperref )
		return;

	kdDebug() << "\thyperref" << endl;

	// output hyperref package
	m_td.tagBegin += "\\usepackage";
	if ( ! m_hyperrefdriver.isEmpty()  )
		m_td.tagBegin += '[' + m_hyperrefdriver + ']';
	m_td.tagBegin += "{hyperref}\n";

	// output hyperref options
	if ( ! m_hyperrefsetup.isEmpty() ) {
		m_td.tagBegin += "\\hypersetup{" + m_hyperrefsetup + "%\n}\n";
	}

	m_td.tagBegin += '\n';


}

void QuickDocument::printBeamerTheme()
{
	kdDebug() << "\tbeamer theme" << endl;

	QString theme = m_cbPaperSize->currentText();
	QRegExp reg("(\\w+)\\s+\\((.*)\\)$");

	if ( reg.search(theme) >= 0 ) {
		QStringList optionlist = QStringList::split(",",reg.cap(2));
		m_td.tagBegin += "\\usepackage[" + optionlist.join(",") + "]{beamertheme" + reg.cap(1) + "}\n\n";
	} else {
		m_td.tagBegin += "\\usepackage{beamertheme" + theme + "}\n\n";
	}
}

////////////////////////////// Slots //////////////////////////////

void QuickDocument::slotOk()
{
	kdDebug() << "==QuickDocument::slotOk()============" << endl;

	// get current class options
	m_currentClass = m_cbDocumentClass->currentText();
	kdDebug() << "\tcurrent class: " << m_currentClass << endl;

	// save the checked options
	m_dictDocumentClasses[m_currentClass][qd_SelectedOptions] = getClassOptions();
	kdDebug() << "\tsave options: " << m_dictDocumentClasses[m_currentClass][qd_SelectedOptions] << endl;

	// build template
	printTemplate();

	// update config file
	writeConfig();

	accept();
}

////////////////////////////// slots: document class //////////////////////////////

void QuickDocument::slotDocumentClassAdd()
{
	kdDebug() << "==QuickDocument::slotDocumentClassAdd()============" << endl;
	QStringList list;
	list << i18n("Document Class")
	     << "label,edit,label,combobox,checkbox,checkbox"
	     << i18n("Please enter the new document &class:")
	     << QString::null                                     // 3
	     << i18n("&Set all options from this standard class (optional):")
	     << ",article,book,letter,report,scrartcl,scrbook,scrreprt"    // 5
	     << i18n("Use standard &fontsizes")                   // 6
	     << i18n("Use standard &papersizes")                  // 7
	     ;

	if ( inputDialog(list,qd_CheckNotEmpty | qd_CheckDocumentClass) ) {
		QString classname = list[3];

		QStringList classlist;
		if ( list[5].isEmpty() ) {             // no base class
			QString useFontsizes = ( list[6] == "true" )
			           ? "10pt,11pt,12pt" : "";
			QString usePapersizes = ( list[7] == "true" )
			           ? "a4paper,a5paper,b5paper,executivepaper,legalpaper,letterpaper" : "";
			kdDebug() << "\tadd document class: " << classname
		 	         << " fontsize=" << list[6] << " papersize=" << list[7] << endl;

			// set default entries for the documentClass-dictionary
			classlist <<  useFontsizes << usePapersizes << "" << "";
		} else {                              // based on a standard class
			// first get the first four parameters
			classlist = m_dictDocumentClasses[list[5]];
			// then add all baseclass options
			QStringList optionlist;
			initStandardOptions(list[5],optionlist);
			for (uint i=0; i<optionlist.count(); ++i)
				classlist.append(optionlist[i]);
		}

		// insert the stringlist for this new document class
		m_dictDocumentClasses[ classname ] = classlist;

		// add the new document class into the userClasslist and the documentClass-combobox
		m_userClasslist.append(classname);
		fillDocumentClassCombobox();

		// activate the new document class
		m_cbDocumentClass->setCurrentText(classname);
		slotDocumentClassChanged( m_cbDocumentClass->currentItem() );
	}
}

void QuickDocument::slotDocumentClassDelete()
{
	// get the name of the current class
	QString documentclass = m_cbDocumentClass->currentText();

	kdDebug() << "==QuickDocument::slotDocumentClassDelete()============" << endl;
	if (KMessageBox::warningContinueCancel(this, i18n("Do you want to remove \"%1\" from the document class list?").arg(documentclass), i18n("Remove Document Class"))==KMessageBox::Continue)
	{
		kdDebug() << "\tlazy delete class: " << documentclass << endl;

		// remove this document class from the documentClass-dictionary
		m_dictDocumentClasses.remove(documentclass);

		// mark this document class for deleting from config file (only with OK-Button)
		if ( m_deleteDocumentClasses.findIndex(documentclass) == -1 )
			m_deleteDocumentClasses.append(documentclass);

		// remove it from the list of userclasses
		m_userClasslist.remove(documentclass);

		// and finally remove it from the combobox
		int i = m_cbDocumentClass->currentItem();
		m_cbDocumentClass->removeItem(i);
		m_cbDocumentClass->setCurrentItem(0);

		// init a new document class
		m_currentClass = m_cbDocumentClass->currentText();
		kdDebug() << "\tchange class:  --> " << m_currentClass << endl;
		initDocumentClass();
	}
}

void QuickDocument::slotDocumentClassChanged(int index)
{
	kdDebug() << "==QuickDocument::slotDocumentClassChanged()============" << endl;
	if ( m_cbDocumentClass->text(index).isNull() ) {
		kdDebug() << "\tnull" << endl;
		return;
	}

	// get old and new document class
	QString oldclass = m_currentClass;
	m_currentClass = m_cbDocumentClass->text(index);
	kdDebug() << "\tchange class: " << oldclass << " --> " << m_currentClass << endl;

	// save the checked options
	m_dictDocumentClasses[oldclass][qd_SelectedOptions] = getClassOptions();
	kdDebug() << "\tsave options: " << m_dictDocumentClasses[oldclass][qd_SelectedOptions] << endl;

	// init the new document class
	initDocumentClass();
}

void QuickDocument::slotTypefaceSizeAdd()
{
	kdDebug() << "==QuickDocument::slotTypefaceSizeAdd()============" << endl;
	QStringList list;
	list << i18n("Add Fontsize")
	     << "label,edit"
	     << i18n("Please enter the &fontsizes (comma-separated list):")
	     << QString::null             // 3
	     ;

	if ( inputDialog(list,qd_CheckNotEmpty |qd_CheckFontsize) ) {
		kdDebug() << "\tadd fontsize: " << list[3] << endl;
		addComboboxEntries(m_cbTypefaceSize,"fontsize",list[3]);

		// save the new list of fontsizes
		m_dictDocumentClasses[m_currentClass][qd_Fontsizes] = getComboxboxList(m_cbTypefaceSize);
	}
}

void QuickDocument::slotTypefaceSizeDelete()
{
	if (KMessageBox::warningContinueCancel(this, i18n("Do you want to remove \"%1\" from the fontsize list?").arg(m_cbPaperSize->currentText()), i18n("Remove Fontsize"))==KMessageBox::Continue)
	{
		int i=m_cbPaperSize->currentItem();
		m_cbPaperSize->removeItem(i);

		// save the new list of fontsizes
		m_dictDocumentClasses[m_currentClass][qd_Fontsizes] = getComboxboxList(m_cbTypefaceSize);
	}
}

void QuickDocument::slotPaperSizeAdd()
{
	kdDebug() << "==QuickDocument::slotPaperSizeAdd()============" << endl;
	QStringList list;
	list << i18n("Add Papersize")
	     << "label,edit"
	     << i18n("Please enter the &papersizes (comma-separated list):")
	     << QString::null                 // 3
	     ;

	if ( inputDialog(list,qd_CheckNotEmpty |qd_CheckPapersize) ) {
		kdDebug() << "\tadd papersize: " << list[3] << endl;
		addComboboxEntries(m_cbPaperSize,"papersize",list[3]);

		// save the new list of papersizes
		m_dictDocumentClasses[m_currentClass][qd_Papersizes] = getComboxboxList(m_cbPaperSize);
	}
}

void QuickDocument::slotPaperSizeDelete()
{
	if (KMessageBox::warningContinueCancel(this, i18n("Do you want to remove \"%1\" from the papersize list?").arg(m_cbPaperSize->currentText()), i18n("Remove Papersize"))==KMessageBox::Continue)
	{
		int i=m_cbPaperSize->currentItem();
		m_cbPaperSize->removeItem(i);

		// save the new list of papersizes
		m_dictDocumentClasses[m_currentClass][qd_Papersizes] = getComboxboxList(m_cbPaperSize);
	}
}

////////////////////////////// slots: document class button //////////////////////////////

void QuickDocument::slotClassOptionAdd()
{
	kdDebug() << "==QuickDocument::slotClassOptionAdd()============" << endl;
	QStringList list;
	list << i18n("Add Option")
	     << "label,edit,label,edit,checkbox"
	     << i18n("Name of &option:")
	     << QString::null                  // 3
	     << i18n("&Description:")
	     << QString::null                  // 5
	     << i18n("&Select this option")    // 6
	     ;

	if ( inputDialog(list,qd_CheckNotEmpty | qd_CheckClassOption) ) {
		// get results
		QString option = list[3];
		QString description = list[5];
		bool check = ( list[6] == "true" );

		// add class option
		kdDebug() << "\tadd option: " << option << " (" << description << ") checked=" << list[6] << endl;
		QCheckListItem *cli = new QCheckListItem(m_lvClassOptions, option, QCheckListItem::CheckBox);
		cli->setText(1,description);

		if ( check )
			cli->setOn(true);

		// update dictionary
		updateClassOptions();
	}
}

void QuickDocument::slotClassOptionEdit()
{
	QListViewItem *cur = m_lvClassOptions->selectedItem();
	if ( ! cur )
		return;

	kdDebug() << "==QuickDocument::slotClassOptionEdit()============" << endl;
	QStringList list;
	list << i18n("Edit Option")
	     << "label,edit-r,label,edit"
	     << i18n("Name of &option:")
	     << cur->text(0)
	     << i18n("&Description:")
	     << stripDefault(cur->text(1))           // 5
	     ;

	//if ( inputDialog(list,qd_CheckNotEmpty | qd_CheckClassOption) ) {
	if ( inputDialog(list) ) {
		// get results
		//QString option = list[3];
		QString description = list[5];

		// set changed class option
		kdDebug() << "\tedit option: " << cur->text(0) << " (" << description << ")" << endl;
		//cur->setText(0, option);
		cur->setText(1, description);

		// update dictionary
		updateClassOptions();
	}
}

void QuickDocument::slotClassOptionDelete()
{
	kdDebug() << "==QuickDocument::slotClassOptionDelete()============" << endl;
	if (m_lvClassOptions->selectedItem() && (KMessageBox::warningContinueCancel(this, i18n("Do you want to delete this class option?"), i18n("Delete"))==KMessageBox::Continue)) {
		QListViewItem *cur = m_lvClassOptions->selectedItem();

		kdDebug() << "\tdelete option: " << cur->text(0) << " (" << cur->text(1) << ")" << endl;
		m_lvClassOptions->takeItem(m_lvClassOptions->selectedItem());

		// update dictionary
		updateClassOptions();
	}
}

void QuickDocument::slotOptionDoubleClicked(QListViewItem *listViewItem,const QPoint &,int)
{
	QCheckListItem *cli = dynamic_cast<QCheckListItem*>(listViewItem);
	if ( cli ) {
		if ( ! cli->isOn() ) {
			cli->setOn(true);
		}
		else
			cli->setOn(false);
	}
}

////////////////////////////// slots: packages //////////////////////////////

void QuickDocument::slotPackageAdd()
{
	kdDebug() << "==QuickDocument::slotPackageAdd()============" << endl;
	QStringList list;
	list << i18n("Add Package")
	     << "label,edit,label,edit,checkbox"
	     << i18n("&Package:")
	     << QString::null                        // 3
	     << i18n("&Description:")
	     << QString::null                        // 5
	     << i18n("&Select this package")         // 6
	     ;

	if ( inputDialog(list,qd_CheckNotEmpty | qd_CheckPackage) ) {
		kdDebug() << "\tadd package: " << list[3] << " (" << list[5] << ") checked=" << list[6] << endl;
		QCheckListItem *cli = new QCheckListItem(m_lvPackages, list[3], QCheckListItem::CheckBox);
		cli->setText(2, list[5]);
		if ( list[6] == "true" )
			cli->setOn(true);
	}
}

void QuickDocument::slotPackageAddOption()
{
	QListViewItem *cur = m_lvPackages->selectedItem();
	if ( !cur )
		return;

	kdDebug() << "==QuickDocument::packageAddOption()============" << endl;
	QStringList list;
	list << i18n("Add Option")
	     << "label,edit,checkbox,label,edit,label,edit,label,edit,checkbox"
	     << i18n("&Option:") + " (" + i18n("package:") + ' ' + cur->text(0) + ')'
	     << QString::null                   // 3
	     << i18n("&Editable")               // 4
	     << i18n("De&fault value:")
	     << QString::null                   // 6
	     << i18n("&Value:")
	     << QString::null                   // 8
	     << i18n("&Description:")
	     << QString::null                   // 10
	     << i18n("&Select this option")     // 11
	     ;

	if ( !cur->parent() && inputDialog(list,qd_CheckNotEmpty | qd_CheckPackageOption) ) {
		kdDebug() << "\tadd option: " << list[3] << " (" << list[10] << ") checked=" << list[11] << endl;

		QCheckListItem *cli;
		if ( list[4] == "true" ) {
			cli = insertEditableListview((QCheckListItem *)cur,list[3],list[10],list[8],list[6]);
		} else {
			cli = new QCheckListItem(cur, list[3], QCheckListItem::CheckBox);
			cli->setText(2,list[10]);
		}
		if ( list[11] == "true" )
			cli->setOn(true);
		cur->setOpen(true);
	}

}

void QuickDocument::slotPackageEdit()
{
	QListViewItem *cur = m_lvPackages->selectedItem();
	if ( !cur )
		return;

	kdDebug() << "==QuickDocument::slotPackageEdit()============" << endl;
	bool editableOption;
	QString caption,labelText,optionname;

	if ( cur->parent() ) {
//		checkmode = qd_CheckPackageOption;
		caption = i18n("Edit Option");
		labelText = i18n("Op&tion:")  + " (" + i18n("package:") + ' ' + cur->parent()->text(0) + ')';
		optionname = cur->parent()->text(0) + '!' + cur->text(0);
		editableOption = m_dictPackagesEditable.contains(optionname);
	} else {
//		checkmode = qd_CheckPackage;
		caption = i18n("Edit Package");
		labelText = i18n("&Package:");
		optionname = QString::null;
		editableOption = false;
	}

	// create one of three different dialogs; edit package, edit editable option, edit option
	QStringList list;
	list << caption;
	if ( editableOption ) {
		QString defaultvalue = ( m_dictPackagesDefaultvalues.contains(optionname) )
		                     ? m_dictPackagesDefaultvalues[optionname]
		                     : QString::null;
		QString value = ( cur->text(1) == i18n("<default>") )
		                     ? defaultvalue : getPackagesValue(cur->text(1));

		list << "label,edit-r,label,edit-r,label,edit,label,edit"
		     << labelText
		     << cur->text(0)                           // 3
		     << i18n("De&fault value:")
		     << defaultvalue                           // 5
		     << i18n("&Value:")
		     << value                                  // 7
		     << i18n("&Description:")
		     << stripPackageDefault(optionname,cur->text(2))      // 9
		     ;
	} else {
		list << "label,edit-r,label,edit"
		     << labelText
		     << cur->text(0)                           // 3
		     << i18n("&Description:")
		     << cur->text(2)                           // 5
		     ;
	}

	if ( inputDialog(list) ) {
		if ( editableOption ) {
			kdDebug() << "\tedit package: "
			          << list[3]
			          << " (" << list[7] << ") "
			          << " (" << list[9] << ")"
			          << endl;
			cur->setText(0, list[3]);
			setPackagesValue(cur,optionname,list[7]);
			cur->setText(2, addPackageDefault(optionname,list[9]));
		} else {
			kdDebug() << "\tedit package: " << list[3] << " (" << list[5] << ")" << endl;
			cur->setText(0, list[3]);
			cur->setText(2, list[5]);
		}
	}
}

void QuickDocument::slotPackageDelete()
{
	QListViewItem *cur = m_lvPackages->selectedItem();
	if ( !cur )
		return;

	bool packageoption;
	QString message,optionname;
	if ( cur->parent() ) {
		packageoption = true;
		message = i18n("Do you want do delete this package option?");
		optionname = cur->parent()->text(0) + '!' + cur->text(0);
	} else {
		packageoption = false;
		message = i18n("Do you want to delete this package?");
		optionname = cur->text(0);
	}

	if (KMessageBox::warningContinueCancel(this, message, i18n("Delete"))==KMessageBox::Continue) {
		QListViewItem *childcur = cur->firstChild();
		while (childcur) {
			QListViewItem *nextchildcur=childcur->nextSibling();
			delete childcur;
			childcur = nextchildcur;
		}
		delete cur;

		// also delete entries for editable package option
		if ( packageoption && m_dictPackagesEditable.contains(optionname) ) {
			m_dictPackagesEditable.remove(optionname);
			if ( m_dictPackagesDefaultvalues.contains(optionname) )
				m_dictPackagesDefaultvalues.remove(optionname);
		}
	}
}

void QuickDocument::slotPackageReset()
{
	if (KMessageBox::warningContinueCancel(this, i18n("Do you want to reset this package list?"), i18n("Reset Package List"))==KMessageBox::Continue)
	{
		kdDebug() << "\treset packages" << endl;

		initPackages();
		slotEnableButtons();
	}
}

void QuickDocument::slotCheckParent(QListViewItem *listViewItem)
{
	QCheckListItem *cli = dynamic_cast<QCheckListItem*>(listViewItem);
	if (cli && listViewItem->parent() && cli->isOn()) {
		QCheckListItem *cliparent=dynamic_cast<QCheckListItem*>(listViewItem->parent());
		if (cliparent)
			cliparent->setOn(true);
	}
}

void QuickDocument::slotPackageDoubleClicked(QListViewItem *listViewItem,const QPoint &,int)
{
	if ( listViewItem && listViewItem->parent() ) {
		QCheckListItem *parentitem = dynamic_cast<QCheckListItem*>(listViewItem->parent());
		QString option = parentitem->text(0) + '!' + listViewItem->text(0);
		if ( m_dictPackagesEditable.contains(option) )
			slotPackageEdit();
	}
}

////////////////////////////// button states //////////////////////////////

void QuickDocument::slotEnableButtons()
{
	bool enable;

	enable = ( ! isStandardClass(m_currentClass) );

	// add/delete button
	m_btnDocumentClassDelete->setEnabled(enable);
	m_btnTypefaceSizeAdd->setEnabled(enable);
	m_btnTypefaceSizeDelete->setEnabled(enable && m_cbTypefaceSize->count()>0);
	m_btnPaperSizeAdd->setEnabled(enable);
	m_btnPaperSizeDelete->setEnabled(enable && m_cbPaperSize->count()>0);

	// class options
	m_btnClassOptionsAdd->setEnabled(enable);
	enable = ( enable && (m_lvClassOptions->selectedItem() != NULL) );
	m_btnClassOptionsEdit->setEnabled(enable);
	m_btnClassOptionsDelete->setEnabled(enable);

	// packeges
	QListViewItem *cur = m_lvPackages->selectedItem();
	if ( cur && cur->text(0)!= "hyperref" ) {
		m_btnPackagesEdit->setEnabled(true);
		m_btnPackagesDelete->setEnabled(true);
		if ( cur->parent() )
			m_btnPackagesAddOption->setEnabled(false);
		else
			m_btnPackagesAddOption->setEnabled(true);
	} else {
		m_btnPackagesEdit->setEnabled(false);
		m_btnPackagesDelete->setEnabled(false);
		m_btnPackagesAddOption->setEnabled(false);
	}

}

////////////////////////////// input dialog //////////////////////////////

// A variable input dialog, whose widgets are determind by the entries of a stringlist.
// Entry 1 is always the label for the main lineedit, entry 2 the main lineedit. All
// other objects are optionale and their return values are not checked.
//  0 :   caption    (input:  always)
//  1 :   comma separated list of Qt widgets (label,checkbox,edit,edit-r)
//  2ff : strings for Qt widgets

bool QuickDocument::inputDialog(QStringList &list, int check)
{
	QuickDocumentInputDialog *dialog = new QuickDocumentInputDialog(list,check,this,"inputDialog");

	bool result = false;
	if ( dialog->exec() ) {
		dialog->getResults(list);
		result = true;
	}

   delete dialog;
	return result;

}

QuickDocumentInputDialog::QuickDocumentInputDialog(const QStringList &list,int check,
	                                                QuickDocument *parent,
	                                                const char *name )
	: KDialogBase(parent,name,true,list[0],KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true),
	m_parent(parent),
	m_check(check)
{

	QWidget *page = new QWidget(this);
	setMainWidget(page);
	QVBoxLayout *vl = new QVBoxLayout(page, 0, spacingHint());

	int firstlinedit = -1;
	m_description = QStringList::split(",",list[1]);
	for ( uint i=0; i<m_description.count(); ++i ) {
		// create the object
		if ( m_description[i] == "label" ) {
			m_objectlist.append( new QLabel(list[i+2],page) );
		} else if ( m_description[i]=="checkbox" ) {
			m_objectlist.append( new QCheckBox(list[i+2],page) );
		} else if ( m_description[i]=="combobox" ) {
			KComboBox *combobox = new KComboBox(page);
			combobox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
			combobox->setDuplicatesEnabled(false);
			combobox->insertStringList( QStringList::split(",",list[i+2],true) );
			if ( i>0 && m_description[i-1]=="label" )
				((QLabel *)m_objectlist[i-1])->setBuddy(combobox);
			m_objectlist.append( combobox );
		} else  {
			m_objectlist.append( new KLineEdit(list[i+2],page) );
			if ( m_description[i] == "edit-r" )
				((KLineEdit *)m_objectlist[i])->setReadOnly(true);
			else if ( firstlinedit == -1 )
				firstlinedit = i;
			if ( i>0 && m_description[i-1]=="label" )
				((QLabel *)m_objectlist[i-1])->setBuddy(m_objectlist[i]);
		}

		// insert the new object into the layout
		vl->addWidget(m_objectlist[i]);
	}

	if ( firstlinedit != -1 )
		m_objectlist[firstlinedit]->setFocus();
	vl->addStretch(1);
	page->setMinimumWidth(350);
}

QuickDocumentInputDialog::~QuickDocumentInputDialog()
{}

void QuickDocumentInputDialog::getResults(QStringList &list)
{
	for ( uint i=0; i<m_description.count(); ++i ) {
		if ( m_description[i] == "label" ) {
			list[i+2] = ((QLabel *)m_objectlist[i])->text();
		} else if ( m_description[i] == "checkbox" ) {
			list[i+2] = ( ((QCheckBox *)m_objectlist[i])->isOn() ) ? "true" : "false";
		} else if ( m_description[i] == "combobox" ) {
		   list[i+2] = ((KComboBox *)m_objectlist[i])->currentText();
		} else  {
			list[i+2] = ((KLineEdit *)m_objectlist[i])->text().simplifyWhiteSpace();
		}
	}
}

// get the package name from string 'Option: (package: name)'
QString QuickDocumentInputDialog::getPackageName(const QString &text)
{
	QRegExp reg( i18n("package:") + " ([^\\)]+)" );
	return ( reg.search(text) >= 0 ) ? reg.cap(1) : QString::null;
}

bool QuickDocumentInputDialog::checkListEntries(const QString &title, const QString &textlist,
                                                const QString &pattern)
{
	// split entries (one or a comma separated list)
	QStringList list = QStringList::split(",",textlist);

	for ( uint i=0; i<list.count(); ++i ) {
		QString s = list[i].stripWhiteSpace();
		// entries must match a regular expression
		QRegExp reg(pattern);
		if ( ! reg.exactMatch(s) ) {
			KMessageBox::error( this, i18n("%1 '%2' is not allowed.").arg(title).arg(s) );
			return false;
		}
	}
	return true;
}

// check the main result of the input dialog
void QuickDocumentInputDialog::slotOk()
{
	if ( m_check ) {
		// get the label and main input string from the first label/linedit
		QString inputlabel = ((QLabel *)m_objectlist[0])->text();
		QString input = ((KLineEdit *)m_objectlist[1])->text().simplifyWhiteSpace();

		// should we check for an empty string
		if ( (m_check & qd_CheckNotEmpty) && input.isEmpty() ) {
			KMessageBox::error( this, i18n("An empty string is not allowed.") );
			return;
		}

		// should we check for an existing document class
		if ( m_check & qd_CheckDocumentClass ) {
			if ( m_parent->isDocumentClass(input) ) {
				KMessageBox::error( this, i18n("This document class already exists.") );
				return;
			}

			QRegExp reg("\\w+");
			if ( ! reg.exactMatch(input) ) {
				KMessageBox::error( this, i18n("This is not an allowed name for a document class.") );
				return;
			}
		}

		// should we check for an existing document class option
		if ( (m_check & qd_CheckClassOption) && m_parent->isDocumentClassOption(input) ) {
			KMessageBox::error( this, i18n("This document class option already exists.") );
			return;
		}

		// should we check for an existing package
		if ( (m_check & qd_CheckPackage) && m_parent->isPackage(input) ) {
			KMessageBox::error( this, i18n("This package already exists.") );
			return;
		}

		// should we check for an existing package option
		if ( m_check & qd_CheckPackageOption ) {
			QString package = getPackageName(inputlabel);
			if ( package.isEmpty() ) {
				KMessageBox::error( this, i18n("Could not identify the package name.") );
				return;
			}
			if ( m_parent->isPackageOption(package,input) ) {
				KMessageBox::error( this, i18n("This package option already exists.") );
				return;
			}
		}

		// should we check for a (list of) fontsizes
		if ( (m_check & qd_CheckFontsize) && !checkListEntries("Fontsize",input,"\\d+pt") ) {
			return;
		}

		// should we check for a (list of) papersizes
		if ( (m_check & qd_CheckPapersize) && !checkListEntries("Papersize",input,"\\w+") ) {
			return;
		}
	}

	accept();
}

} // namespace

#include "quickdocumentdialog.moc"


