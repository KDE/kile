/***************************************************************************************
date                 : Sep 15 2004
version              : 0.23
copyright            : Thomas Fischer <t-fisch@users.sourceforge.net>
                       restructured, improved and completed by Holger Danielsson
                       (C) 2004 by Holger Danielsson (holger.danielsson@t-online.de)
****************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dialogs/quickdocumentdialog.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QItemDelegate>
#include <QLabel>
#include <QLayout>
#include <QPainter>
#include <QPalette>
#include <QRegExp>
#include <QStringList>
#include <QStyle>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>

#include <KComboBox>
#include <KConfig>
#include <KIconLoader>
#include <KLineEdit>
#include <KLocale>
#include <KMessageBox>
#include <KPushButton>
#include <KTabWidget>

#include "widgets/categorycombobox.h"
#include "kiledebug.h"
#include "kileconfig.h"

namespace KileDialog
{
enum {
	qd_Base = 1,
	qd_Article = 2,
	qd_BookReport = 4,
	qd_KomaArticle = 8,
	qd_KomaBookReport = 16,
	qd_KomaAbstract = 32,
	qd_Prosper = 64,
	qd_Beamer = 128
};

// list with index numbers for the stringlist with all information of a document class
enum {
	qd_Fontsizes,
	qd_Papersizes,
	qd_DefaultOptions,
	qd_SelectedOptions,
	qd_OptionsStart
};

//////////////////// EditableItemDelegate ////////////////////

class EditableItemDelegate : public QItemDelegate {
	public:
		EditableItemDelegate(QObject *parent = 0) : QItemDelegate(parent) {}

		virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex& index) const
		{
			drawBackground(painter, option, index);

			QColor textColor = option.palette.color(QPalette::Text);
			QString text = index.data(Qt::DisplayRole).toString();
			if (text == "<default>" || text == "<empty>") {
				textColor = Qt::gray;
			} else
				if (option.state & QStyle::State_Selected) {
					textColor = option.palette.color(QPalette::HighlightedText);
				}
			painter->setPen(textColor);
			painter->drawText(option.rect, Qt::AlignCenter | Qt::AlignVCenter, text);
			//drawDisplay(painter, option, option.rect, index.data(Qt::DisplayRole).toString());
			drawFocus(painter, option, option.rect);
		}
};

//////////////////// QuickDocument class ////////////////////

QuickDocument::QuickDocument(KConfig *config, QWidget *parent, const char *name, const QString &caption) : Wizard(config, parent, name, caption)
{
	KILE_DEBUG() << "==QuickDocument::setupGUI()============";
	KTabWidget *tabWidget = new KTabWidget(this);
	setMainWidget(tabWidget);

	tabWidget->addTab(setupClassOptions(tabWidget), i18n("Cla&ss Options"));
	tabWidget->addTab(setupPackages(tabWidget), i18n("&Packages"));
	tabWidget->addTab(setupProperties(tabWidget), i18n("&Document Properties"));

	// read config file
	readConfig();
	m_lvClassOptions->resizeColumnToContents(0);
	m_lvPackages->resizeColumnToContents(0);
}

QuickDocument::~QuickDocument()
{}

//////////////////// GUI ////////////////////

QWidget *QuickDocument::setupClassOptions(QTabWidget *tab)
{
	KILE_DEBUG() << "\tsetupClassOptions";
	QLabel *label;

	QWidget *classOptions = new QWidget(tab);
	QGridLayout *gl = new QGridLayout();
	gl->setMargin(marginHint());
	gl->setSpacing(spacingHint());
	classOptions->setLayout(gl);

	// Document classes
	m_cbDocumentClass = new KileWidget::CategoryComboBox(classOptions);
	m_cbDocumentClass->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
	m_cbDocumentClass->setDuplicatesEnabled(false);
// m_cbDocumentClass->listBox()->setVariableHeight(true);
	gl->addWidget(m_cbDocumentClass, 0, 1);
	connect(m_cbDocumentClass, SIGNAL(activated(int)), this, SLOT(slotDocumentClassChanged(int)));

	label = new QLabel(i18n("Doc&ument class:"), classOptions);
	gl->addWidget(label, 0, 0);
	label->setBuddy(m_cbDocumentClass);
	label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

	m_btnDocumentClassAdd = new KPushButton(classOptions);
	m_btnDocumentClassAdd->setIcon(KIcon("list-add"));
	m_btnDocumentClassAdd->setWhatsThis(i18n("Add an entry to this combo box"));
	gl->addWidget(m_btnDocumentClassAdd, 0, 2);
	connect(m_btnDocumentClassAdd, SIGNAL(clicked()), this, SLOT(slotDocumentClassAdd()));

	m_btnDocumentClassDelete = new KPushButton(classOptions);
	m_btnDocumentClassDelete->setIcon(KIcon("list-remove"));
	m_btnDocumentClassDelete->setWhatsThis(i18n("Remove current entry from this combo box"));
	gl->addWidget(m_btnDocumentClassDelete, 0, 3);
	connect(m_btnDocumentClassDelete, SIGNAL(clicked()), this, SLOT(slotDocumentClassDelete()));

	// Fontsize
	m_cbTypefaceSize = new KileWidget::CategoryComboBox(classOptions);
	m_cbTypefaceSize->setDuplicatesEnabled(false);
	gl->addWidget(m_cbTypefaceSize, 1, 1);

	label = new QLabel(i18n("&Typeface size:"), classOptions);
	label->setBuddy(m_cbTypefaceSize);
	label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	gl->addWidget(label, 1, 0);

	m_btnTypefaceSizeAdd = new KPushButton(classOptions);
	m_btnTypefaceSizeAdd->setIcon(KIcon("list-add"));
	m_btnTypefaceSizeAdd->setWhatsThis(i18n("Add an entry to this combo box"));
	gl->addWidget(m_btnTypefaceSizeAdd, 1, 2);
	connect(m_btnTypefaceSizeAdd, SIGNAL(clicked()), this, SLOT(slotTypefaceSizeAdd()));

	m_btnTypefaceSizeDelete = new KPushButton(classOptions);
	m_btnTypefaceSizeDelete->setIcon(KIcon("list-remove"));
	m_btnTypefaceSizeDelete->setWhatsThis(i18n("Remove current entry from this combo box"));
	gl->addWidget(m_btnTypefaceSizeDelete, 1, 3);
	connect(m_btnTypefaceSizeDelete, SIGNAL(clicked()), this, SLOT(slotTypefaceSizeDelete()));

	// Papersize
	m_cbPaperSize = new KileWidget::CategoryComboBox(classOptions);
	m_cbPaperSize->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
	m_cbPaperSize->setDuplicatesEnabled(false);
	gl->addWidget(m_cbPaperSize, 2, 1);

	m_lbPaperSize = new QLabel(i18n("Paper si&ze:"), classOptions);
	m_lbPaperSize->setBuddy(m_cbPaperSize);
	m_lbPaperSize->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	gl->addWidget(m_lbPaperSize, 2, 0);

	m_btnPaperSizeAdd = new KPushButton(classOptions);
	m_btnPaperSizeAdd->setIcon(KIcon("list-add"));
	m_btnPaperSizeAdd->setWhatsThis(i18n("Add an entry to this combo box"));
	gl->addWidget(m_btnPaperSizeAdd, 2, 2);
	connect(m_btnPaperSizeAdd, SIGNAL(clicked()), this, SLOT(slotPaperSizeAdd()));

	m_btnPaperSizeDelete = new KPushButton(classOptions);
	m_btnPaperSizeDelete->setIcon(KIcon("list-remove"));
	m_btnPaperSizeDelete->setWhatsThis(i18n("Remove current entry from this combo box"));
	gl->addWidget(m_btnPaperSizeDelete, 2, 3);
	connect(m_btnPaperSizeDelete, SIGNAL(clicked()), this, SLOT(slotPaperSizeDelete()));

	// Encoding
	m_cbEncoding = new KileWidget::CategoryComboBox(classOptions);
	m_cbEncoding->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
	m_cbEncoding->setDuplicatesEnabled(false);
	gl->addWidget(m_cbEncoding, 3, 1);

	label = new QLabel(i18n("E&ncoding:"), classOptions);
	label->setBuddy(m_cbEncoding);
	label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	gl->addWidget(label, 3, 0);

	// Class Options
	m_lvClassOptions = new QTreeWidget(classOptions);
	m_lvClassOptions->setHeaderLabels(QStringList() << i18n("Option")
	                                  << i18n("Description"));
	m_lvClassOptions->setAllColumnsShowFocus(true);
	m_lvClassOptions->setRootIsDecorated(false);
	gl->addWidget(m_lvClassOptions, 4, 1, 1, 3);
	connect(m_lvClassOptions, SIGNAL(itemSelectionChanged()),
	        this, SLOT(slotEnableButtons()));
	connect(m_lvClassOptions, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
	        this, SLOT(slotOptionDoubleClicked(QTreeWidgetItem*, int)));

	label = new QLabel(i18n("Cl&ass options:"), classOptions);
	label->setBuddy(m_lvClassOptions);
	label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
	label->setAlignment(Qt::AlignTop);
	gl->addWidget(label, 4, 0);

	// button
	QWidget *frame = new QWidget(classOptions);
	QHBoxLayout *hl = new QHBoxLayout();
	hl->setMargin(0);
	hl->setSpacing(spacingHint());
	frame->setLayout(hl);
	gl->addWidget(frame, 5, 1, 1, 3, Qt::AlignCenter);

	m_btnClassOptionsAdd = new KPushButton(i18n("&Add..."), frame);
	m_btnClassOptionsAdd->setIcon(KIcon("list-add"));
	m_btnClassOptionsAdd->setWhatsThis(i18n("Add a new class option"));
	hl->addWidget(m_btnClassOptionsAdd);
	connect(m_btnClassOptionsAdd, SIGNAL(clicked()), this, SLOT(slotClassOptionAdd()));

	m_btnClassOptionsEdit = new KPushButton(i18n("Ed&it..."), frame);
	m_btnClassOptionsEdit->setIcon(KIcon("document-properties"));
	m_btnClassOptionsEdit->setWhatsThis(i18n("Edit the current class option"));
	hl->addWidget(m_btnClassOptionsEdit);
	connect(m_btnClassOptionsEdit, SIGNAL(clicked()), this, SLOT(slotClassOptionEdit()));

	m_btnClassOptionsDelete = new KPushButton(i18n("De&lete"), frame);
	m_btnClassOptionsDelete->setIcon(KIcon("list-remove"));
	m_btnClassOptionsDelete->setWhatsThis(i18n("Remove the current class option"));
	hl->addWidget(m_btnClassOptionsDelete);
	connect(m_btnClassOptionsDelete, SIGNAL(clicked()), this, SLOT(slotClassOptionDelete()));

	return classOptions;
}

QWidget *QuickDocument::setupPackages(QTabWidget *tab)
{
	KILE_DEBUG() << "\tsetupPackages";

	QWidget *packages = new QWidget(tab);
	QVBoxLayout *vl = new QVBoxLayout();
	vl->setMargin(marginHint());
	vl->setSpacing(spacingHint());
	packages->setLayout(vl);

	QLabel *label = new QLabel(i18n("LaTe&X packages:"), packages);
	vl->addWidget(label);
	m_lvPackages = new QTreeWidget(packages);
	vl->addWidget(m_lvPackages);
	m_lvPackages->setRootIsDecorated(true);
	m_lvPackages->setHeaderLabels(QStringList() << i18n("Package") << i18n("Value") << i18n("Description"));
	m_lvPackages->setAllColumnsShowFocus(true);
	m_lvPackages->setItemDelegateForColumn(1, new EditableItemDelegate());
	label->setBuddy(m_lvPackages);
	connect(m_lvPackages, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
	        this, SLOT(slotCheckParent(QTreeWidgetItem*)));
	connect(m_lvPackages, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
	        this, SLOT(slotCheckParent(QTreeWidgetItem*)));
	connect(m_lvPackages, SIGNAL(itemSelectionChanged()),
	        this, SLOT(slotEnableButtons()));
	connect(m_lvPackages, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
	        this, SLOT(slotPackageDoubleClicked(QTreeWidgetItem*)));

	QWidget *frame = new QWidget(packages);
	vl->addWidget(frame);
	QHBoxLayout *hl = new QHBoxLayout();
	hl->setMargin(0);
	hl->setSpacing(spacingHint());
	frame->setLayout(hl);
	hl->addStretch();

	m_btnPackagesAdd = new KPushButton(i18n("&Add Package..."), frame);
	m_btnPackagesAdd->setIcon(KIcon("list-add"));
	m_btnPackagesAdd->setWhatsThis(i18n("Add a new package"));
	connect(m_btnPackagesAdd, SIGNAL(clicked()), this, SLOT(slotPackageAdd()));
	hl->addWidget(m_btnPackagesAdd);
	m_btnPackagesAddOption = new KPushButton(i18n("Add Op&tion..."), frame);
	m_btnPackagesAddOption->setIcon(KIcon("list-add"));
	m_btnPackagesAddOption->setWhatsThis(i18n("Add a new package option"));
	connect(m_btnPackagesAddOption, SIGNAL(clicked()), this, SLOT(slotPackageAddOption()));
	hl->addWidget(m_btnPackagesAddOption);
	m_btnPackagesEdit = new KPushButton(i18n("Ed&it..."), frame);
	m_btnPackagesEdit->setIcon(KIcon("document-properties"));
	m_btnPackagesEdit->setWhatsThis(i18n("Edit the current package option"));
	connect(m_btnPackagesEdit, SIGNAL(clicked()), this, SLOT(slotPackageEdit()));
	hl->addWidget(m_btnPackagesEdit);
	m_btnPackagesDelete = new KPushButton(i18n("De&lete"), frame);
	m_btnPackagesDelete->setIcon(KIcon("list-remove"));
	m_btnPackagesDelete->setWhatsThis(i18n("Remove the current package option"));
	connect(m_btnPackagesDelete, SIGNAL(clicked()), this, SLOT(slotPackageDelete()));
	hl->addWidget(m_btnPackagesDelete);
	m_btnPackagesReset = new KPushButton(i18n("&Reset to Defaults"), frame);
	m_btnPackagesReset->setIcon(KIcon("document-revert"));
	m_btnPackagesReset->setWhatsThis(i18n("Reset to the default list of packages"));
	connect(m_btnPackagesReset, SIGNAL(clicked()), this, SLOT(slotPackageReset()));
	hl->addWidget(m_btnPackagesReset);
	hl->addStretch();

	return packages;
}

QWidget *QuickDocument::setupProperties(QTabWidget *tab)
{
	KILE_DEBUG() << "\tsetupProperties";
	QLabel *label;

	QWidget *personalInfoPage = new QWidget(tab);
	QVBoxLayout *vl = new QVBoxLayout();
	vl->setMargin(0);
	vl->setSpacing(spacingHint());
	personalInfoPage->setLayout(vl);

	QWidget *personalInfo = new QWidget(personalInfoPage);
	QGridLayout *gl = new QGridLayout();
	gl->setMargin(marginHint());
	gl->setSpacing(spacingHint());
	personalInfo->setLayout(gl);

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
	m_leDate->setText(KGlobal::locale()->formatDate(QDate::currentDate(), KLocale::ShortDate));

	vl->addWidget(personalInfo);
	vl->addStretch();

	return personalInfoPage;
}

//////////////////// read configuration ////////////////////

void QuickDocument::readConfig()
{
	KILE_DEBUG() << "==QuickDocument::readConfig()============";

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
	KILE_DEBUG() << "==QuickDocument::writeConfig()============";

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
	KILE_DEBUG() << "\tread config: document class";

	// read standard options
	m_userClasslist = KileConfig::userClasses();
	m_currentClass = KileConfig::documentClass();
	m_currentEncoding = KileConfig::encoding();

	// init standard classes
	QString stdFontsize = "10pt,11pt,12pt";
	QString stdPapersize = "a4paper,a5paper,b5paper,executivepaper,legalpaper,letterpaper";
	QString beamerThemes = "bars;boxes;classic;lined;plain;sidebar;sidebar (dark);sidebar (tab);"
	                       "sidebar (dark,tab);shadow;split;tree;tree (bar)";

	initStandardClass("article", stdFontsize, stdPapersize,
	                  "10pt,letterpaper,oneside,onecolumn,final",
	                  KileConfig::optionsArticle());
	initStandardClass("book", stdFontsize, stdPapersize,
	                  "10pt,letterpaper,twoside,onecolumn,final,openright",
	                  KileConfig::optionsBook());
	initStandardClass("letter", stdFontsize, stdPapersize,
	                  "10pt,letterpaper,oneside,onecolumn,final",
	                  KileConfig::optionsLetter());
	initStandardClass("report", stdFontsize, stdPapersize,
	                  "10pt,letterpaper,oneside,onecolumn,final,openany",
	                  KileConfig::optionsReport());
	initStandardClass("scrartcl", stdFontsize, stdPapersize,
	                  "11pt,a4paper,abstractoff,bigheadings,final,headnosepline,"
	                  "footnosepline,listsindent,onelinecaption,notitlepage,onecolumn,"
	                  "oneside,openany,parindent,tablecaptionbelow,tocindent",
	                  KileConfig::optionsScrartcl());
	initStandardClass("scrbook", stdFontsize, stdPapersize,
	                  "11pt,a4paper,bigheadings,final,headnosepline,footnosepline,"
	                  "listsindent,nochapterprefix,onelinecaption,onecolumn,"
	                  "openright,parindent,tablecaptionbelow,titlepage,tocindent,twoside",
	                  KileConfig::optionsScrbook());
	initStandardClass("scrreprt", stdFontsize, stdPapersize,
	                  "11pt,a4paper,abstractoff,bigheadings,final,headnosepline,"
	                  "footnosepline,listsindent,nochapterprefix,onelinecaption,onecolumn,"
	                  "oneside,openany,parindent,tablecaptionbelow,titlepage,tocindent",
	                  KileConfig::optionsScrreprt());
	initStandardClass("prosper", QString(), QString(),
	                  "final,slideBW,total,nocolorBG,ps,noaccumulate,ps2pdf",
	                  KileConfig::optionsProsper());
	initStandardClass("beamer", "8pt,9pt,10pt,11pt,12pt,14pt,17pt,20pt", beamerThemes,
	                  "11pt,blue,notes=show,sans,slidescentered",
	                  KileConfig::optionsBeamer());

	// init all user classes
	for (int i = 0; i < m_userClasslist.count(); ++i) {
		KILE_DEBUG() << "\tinit user class: " << m_userClasslist[i];
		QStringList list;
		// read dour default entries for this user class
		KConfigGroup configGroup = m_config->group(QString("QuickDocument/") + m_userClasslist[i]);
		list.append(configGroup.readEntry("fontsizesList"));
		list.append(configGroup.readEntry("pagesizesList"));
		list.append(configGroup.readEntry("defaultOptions"));
		list.append(configGroup.readEntry("selectedOptions"));
		// now read all user defined options
		QStringList options = (configGroup.readEntry("options")).split(',');
		for (int j = 0; j < options.count(); ++j) {
			list.append(options[j] + " => " + configGroup.readEntry(options[j]));
		}

		// save all information of this class into the documentClass-dictionary
		m_dictDocumentClasses[ m_userClasslist[i] ] = list;
	}

	// set classes combobox (standard and user defined classes)
	fillDocumentClassCombobox();

	// set encoding combobox
	fillCombobox(m_cbEncoding,
	             "ansinew,applemac,ascii,cp1252,cp1250,cp1251,cp1257,cp437,cp437de,cp850,cp858,"
	             "cp852,cp865,decmulti,koi8-r,latin1,latin2,latin3,latin4,latin5,latin9,latin10,next,utf8,utf8x,utfcyr",
	             m_currentEncoding);
}

void QuickDocument::fillDocumentClassCombobox()
{
	QString stdClasses = "article,book,letter,report,-,scrartcl,scrbook,scrreprt,-";
	QString stdUserClasses = "beamer,prosper";

	// set classes combobox (standard and user defined classes)
	QStringList classlist = stdUserClasses.split(',');
	for (int i = 0; i < m_userClasslist.count(); ++i) {
		classlist.append(m_userClasslist[i]);
	}
	classlist.sort();
	fillCombobox(m_cbDocumentClass, stdClasses + ',' + classlist.join(","), m_currentClass);
}

void QuickDocument::writeDocumentClassConfig()
{
	KILE_DEBUG() << "\twrite config: document class";

	// first delete all marked document classes
	for (int i = 0; i < m_deleteDocumentClasses.count(); ++i) {
		KILE_DEBUG() << "\tdelete class: " << m_deleteDocumentClasses[i];
		m_config->deleteGroup(QString("QuickDocument/") + m_deleteDocumentClasses[i]);
	}

	// write document classes and encoding
	QStringList userclasses;
	for (int i = 0; i < m_cbDocumentClass->count(); ++i) {
		if (!m_cbDocumentClass->itemText(i).isEmpty() && !isStandardClass(m_cbDocumentClass->itemText(i))) {
			userclasses.append(m_cbDocumentClass->itemText(i));
		}
	}
	KileConfig::setUserClasses(userclasses);
	KileConfig::setDocumentClass(m_cbDocumentClass->currentText());
	KileConfig::setEncoding(m_cbEncoding->currentText());

	// write checked options of standard classes
	KILE_DEBUG() << "\twrite standard classes";
	KileConfig::setOptionsArticle(m_dictDocumentClasses["article"][qd_SelectedOptions]);
	KileConfig::setOptionsBook(m_dictDocumentClasses["book"][qd_SelectedOptions]);
	KileConfig::setOptionsLetter(m_dictDocumentClasses["letter"][qd_SelectedOptions]);
	KileConfig::setOptionsReport(m_dictDocumentClasses["report"][qd_SelectedOptions]);
	KileConfig::setOptionsScrartcl(m_dictDocumentClasses["scrartcl"][qd_SelectedOptions]);
	KileConfig::setOptionsScrbook(m_dictDocumentClasses["scrbook"][qd_SelectedOptions]);
	KileConfig::setOptionsScrreprt(m_dictDocumentClasses["scrreprt"][qd_SelectedOptions]);
	KileConfig::setOptionsProsper(m_dictDocumentClasses["prosper"][qd_SelectedOptions]);
	KileConfig::setOptionsBeamer(m_dictDocumentClasses["beamer"][qd_SelectedOptions]);

	// write config of user packages
	QRegExp reg("(\\S+)\\s+=>\\s+(.*)");
	for (int i = 0; i < userclasses.count(); ++i) {
		// get the stringlist with all information
		KILE_DEBUG() << "\twrite user class: " << userclasses[i];
		QStringList list = m_dictDocumentClasses[userclasses[i]];

		// write the config group and the default entries
		KConfigGroup configGroup = m_config->group(QString("QuickDocument/") + userclasses[i]);
		configGroup.writeEntry("fontsizesList", list[qd_Fontsizes]);
		configGroup.writeEntry("pagesizesList", list[qd_Papersizes]);
		configGroup.writeEntry("defaultOptions", list[qd_DefaultOptions]);
		configGroup.writeEntry("selectedOptions", list[qd_SelectedOptions]);

		// write user defined options
		QString options;
		for (int j = qd_OptionsStart; j < list.count(); ++j) {
			int pos = reg.indexIn(list[j]);
			if (pos != -1) {
				configGroup.writeEntry(reg.cap(1), reg.cap(2));
				if (!options.isEmpty()) {
					options += ',';
				}
				options += reg.cap(1);
			}
		}
		configGroup.writeEntry("options", options);
	}
}

void QuickDocument::initDocumentClass()
{
	KILE_DEBUG() << "==QuickDocument::initDocumentClass()============";
	KILE_DEBUG() << "\tset class: " << m_currentClass;

	// get the stringlist of this class with all information
	QStringList classlist = m_dictDocumentClasses[m_currentClass];

	// First of all, we have to set the defaultOptions-dictionary and the
	// selectedOptions-dictionary for this class, before inserting options
	// into the listview. The function setClassOptions() will look
	// into both dictionaries to do some extra work.
	setDefaultClassOptions(classlist[qd_DefaultOptions]);
	setSelectedClassOptions(classlist[qd_SelectedOptions]);

	// set comboboxes for fontsizes and papersizes
	fillCombobox(m_cbTypefaceSize, classlist[qd_Fontsizes], m_currentFontsize);
	fillCombobox(m_cbPaperSize, classlist[qd_Papersizes], m_currentPapersize);

	// now we are ready to set the class options
	if (isStandardClass(m_currentClass)) {
		QStringList optionlist;
		initStandardOptions(m_currentClass, optionlist);
		setClassOptions(optionlist, 0);
	} else {
		setClassOptions(classlist, qd_OptionsStart);
	}

	// there is no papersize with class beamer, but a theme
	if (m_currentClass == "beamer")
		m_lbPaperSize->setText(i18n("&Theme:"));
	else
		m_lbPaperSize->setText(i18n("Paper si&ze:"));

	// enable/disable buttons to add or delete entries
	slotEnableButtons();
}

void QuickDocument::initStandardClass(const QString &classname,
                                      const QString &fontsize, const QString &papersize,
                                      const QString &defaultoptions, const QString &selectedoptions)
{
	KILE_DEBUG() << "\tinit standard class: " << classname;

	// remember that this is a standard class
	m_dictStandardClasses[ classname ]  =  true;

	// save all entries
	QStringList list;
	list << fontsize << papersize << defaultoptions << selectedoptions;

	// save in documentClass-dictionary
	m_dictDocumentClasses[ classname ] = list;
}

// build all option for the current standard class
void QuickDocument::initStandardOptions(const QString &classname, QStringList &optionlist)
{
	// build the bitcode for all options of this class
	int options;
	if (classname == "article")
		options = qd_Base + qd_Article;
	else
		if (classname == "book")
			options = qd_Base + qd_Article + qd_BookReport;
		else
			if (classname == "letter")
				options = qd_Base;
			else
				if (classname == "report")
					options = qd_Base + qd_Article + qd_BookReport;
				else
					if (classname == "scrartcl")
						options = qd_Base + qd_Article + qd_KomaArticle + qd_KomaAbstract;
					else
						if (classname == "scrbook")
							options = qd_Base + qd_Article + qd_BookReport + qd_KomaArticle + qd_KomaBookReport;
						else
							if (classname == "scrreprt")
								options = qd_Base + qd_Article + qd_BookReport + qd_KomaArticle + qd_KomaAbstract + qd_KomaBookReport;
							else
								if (classname == "prosper")
									options = qd_Prosper;
								else
									if (classname == "beamer")
										options = qd_Beamer;
									else
										return;

	// insert all options into the list
	if (options & qd_Base) {
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

	if (options & qd_Article) {
		optionlist
		<< QString("titlepage => ") + i18n("Puts title and abstract on an extra page")
		<< QString("notitlepage => ") + i18n("Puts title and abstract on the same page as the text")
		<< QString("onecolumn => ") + i18n("Puts the text in one column")
		<< QString("twocolumn => ") + i18n("Puts the text in two columns")
		<< QString("openbib => ") + i18n("Formats the bibliography in open style")
		;
	}

	if (options & qd_BookReport) {
		optionlist
		<< QString("openany => ") + i18n("Chapters may start on top of every page")
		<< QString("openright => ") + i18n("Chapters may only start on top of right pages")
		;
	}

	if (options & qd_KomaArticle) {
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

	if (options & qd_KomaBookReport) {
		optionlist
		<< QString("chapterprefix => ") + i18n("Use a separate line for the chapter number")
		<< QString("nochapterprefix => ") + i18n("Use the same line for the chapter number and title")
		<< QString("appendixprefix => ") + i18n("Use a separate line for the appendix name")
		<< QString("noappendixprefix  => ") + i18n("No separate line for the appendix name")
		;
	}

	if (options & qd_KomaAbstract) {
		optionlist
		<< QString("abstracton => ") + i18n("Include the abstract's title")
		<< QString("abstractoff => ") + i18n("Exclude the abstract's title")
		;
	}

	if (options & qd_Prosper) {
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

	if (options & qd_Beamer) {
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
	QStringList list = defaultoptions.split(',');
	m_currentDefaultOptions.clear();
	for (int i = 0; i < list.count(); ++i) {
		if (!list[i].isEmpty()) {
			m_currentDefaultOptions[list[i]] = true;
		}
	}
}

// insert all checked options of the current class into the selectedOptions-dictionary
void QuickDocument::setSelectedClassOptions(const QString &selectedoptions)
{
	KILE_DEBUG() << "\tset options: " << selectedoptions;

	QStringList list = selectedoptions.split(',');
	uint nlist = list.count();

	m_currentFontsize  = (nlist >= 1) ? list[0] : "";
	m_currentPapersize = (nlist >= 2) ? list[1] : "";

	m_currentSelectedOptions.clear();
	for (uint i = 0; i < nlist; ++i) {
		if (! list[i].isEmpty())
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
	for (int i = start; i < list.count(); ++i) {
		int pos = reg.indexIn(list[i]);
		if (pos != -1) {
			QTreeWidgetItem *twi = new QTreeWidgetItem(m_lvClassOptions, QStringList(reg.cap(1)));
			twi->setFlags(twi->flags() | Qt::ItemIsUserCheckable);

			// see if it is a default option
			if (isDefaultClassOption(reg.cap(1))) {
				twi->setText(1, reg.cap(2) + " [default]");
			}
			else {
				twi->setText(1, reg.cap(2));
			}

			// check it if this option is set by th user
			twi->setCheckState(0, isSelectedClassOption(reg.cap(1)) ? Qt::Checked : Qt::Unchecked);
		}
	}
}

// get all options of the current class as a comma separated list
//  - first entry: always the current fontsize
//  - second entry: always the current papersize
//  - followed by all other checked options
QString QuickDocument::getClassOptions()
{
	QString fontsize = stripDefault(m_cbTypefaceSize->currentText());
	QString papersize = stripDefault(m_cbPaperSize->currentText());

	QString options =  fontsize + ',' + papersize;

	QTreeWidgetItemIterator it(m_lvClassOptions);
	while (*it) {
		if ((*it)->checkState(0) == Qt::Checked) {
			options += ',' + (*it)->text(0);
		}
		++it;
	}

	return options;
}

// Some changes were made in the listview: add, edit oder delete entries.
// This means that the defaultOptions-dictionary, the selectedOptions-dictionary
// and the list of all options may be                                                                     . So the documentClass-dictionary,
// the defaultOptions-dictionary and the selectedOptions-dictionary must be updated.
void QuickDocument::updateClassOptions()
{
	KILE_DEBUG() << "==QuickDocument::updateClassOptions()============";
	KILE_DEBUG() << "\tclass: " << m_currentClass;

	QString defaultoptions;
	QStringList newlist;
	QStringList oldlist = m_dictDocumentClasses[m_currentClass];

	// read the first four static entries
	newlist << oldlist[qd_Fontsizes];
	newlist << oldlist[qd_Papersizes];
	newlist << QString();        // dummy entry: will be changed
	newlist << getClassOptions();

	// read all options
	QTreeWidgetItemIterator it(m_lvClassOptions);
	while (*it) {
		QString description = (*it)->text(1);
		if (description.right(10) == " [default]") {
			description = stripDefault(description);
			if (!defaultoptions.isEmpty()) {
				defaultoptions += ',';
			}
			defaultoptions += (*it)->text(0);
		}
		newlist += (*it)->text(0) + " => " + description;
		++it;
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
void QuickDocument::fillCombobox(KileWidget::CategoryComboBox *combo, const QString &cslist, const QString &seltext)
{
	bool documentclasscombo = (combo == m_cbDocumentClass);

	QString sep = (m_currentClass == "beamer" && combo == m_cbPaperSize) ? ";" : ",";
	QStringList list = cslist.split(sep, QString::SkipEmptyParts);
	if (!documentclasscombo) {
		list.sort();
	}

	combo->clear();
	for (int i = 0; i < list.count(); ++i) {
		if (!documentclasscombo && isDefaultClassOption(list[i])) {
			combo->addItem(QString(list[i]) + " [default]");
		}
		else
			if (list[i] != "-") {
				combo->addItem(list[i]);
			}
			else {
				combo->addCategoryItem("");
			}

		// should this entry be selected?
		if (!seltext.isEmpty() && list[i] == seltext) {
			combo->setCurrentIndex(i);
		}
	}
}

// Add some entries from a comma separated list to a sorted combobox.
// The new entries must match a regular expression or will be denied.
bool QuickDocument::addComboboxEntries(KileWidget::CategoryComboBox *combo, const QString &title, const QString &entry)
{
	// read current comboxbox entries
	QStringList combolist;
	for (int i = 0; i < combo->count(); ++i) {
		combolist += combo->itemText(i);
	}

	// add new entries (one or a comma separated list)
	QStringList list = entry.split(',');
	for (int i = 0; i < list.count(); ++i) {
		QString s = list[i].trimmed();
		// entries must match a regular expression
		if (combolist.indexOf(s) != -1) {
			KMessageBox::error(this, i18n("%1 '%2' already exists.", title, s));
		}
		else {
			combolist += s;
			KILE_DEBUG() << "\tinsert new " << title << ": " << s;
		}
	}

	// insert list, if there are more entries than before
	if (combolist.count() > combo->count()) {
		fillCombobox(combo, combolist.join(","), list[0]);
		return true;
	}
	else {
		return false;
	}
}

QString QuickDocument::getComboxboxList(KComboBox *combo)
{
	QStringList list;
	for (int i = 0; i < combo->count(); ++i) {
		list += combo->itemText(i);
	}

	return (list.count() > 0) ? list.join(",") : QString();
}

// strip an optional default-tag from the string
QString QuickDocument::stripDefault(const QString &s)
{
	return (s.right(10) == " [default]") ? s.left(s.length() - 10) : s;
}

////////////////////////////// packages tab //////////////////////////////

void QuickDocument::readPackagesConfig()
{
	KILE_DEBUG() << "\tread config: packages";

	if (! readPackagesListview())
		initPackages();
}

// init default values for packages tab
void QuickDocument::initPackages()
{
	KILE_DEBUG() << "read config: init standard packages";
	QTreeWidgetItem *cli;
	QTreeWidgetItem *clichild;

	m_lvPackages->clear();
	cli = insertTreeWidget(m_lvPackages, "amsmath", i18n("Special math environments and commands (AMS)"));
	cli = insertTreeWidget(m_lvPackages, "amsfonts", i18n("Collection of fonts and symbols for math mode (AMS)"));
	cli = insertTreeWidget(m_lvPackages, "amssymb", i18n("Defines symbol names for all math symbols in MSAM and MSBM (AMS)"));
	cli = insertTreeWidget(m_lvPackages, "amsthm", i18n("Improved theorem setup (AMS)"));
	cli = insertTreeWidget(m_lvPackages, "caption", i18n("Extends caption capabilities for figures and tables"));

	cli = insertTreeWidget(m_lvPackages, "hyperref", i18n("Hypertext marks in LaTeX"));
	cli->setExpanded(true);
	clichild = insertTreeWidget(cli, "dvips", i18n("Use dvips as hyperref driver"));
	clichild->setCheckState(0, Qt::Checked);
	clichild = insertTreeWidget(cli, "pdftex", i18n("Use pdftex as hyperref driver"));
	clichild = insertEditableTreeWidget(cli, "bookmarks", i18n("Make bookmarks"), "true", "true");
	clichild = insertEditableTreeWidget(cli, "bookmarksnumbered", i18n("Put section numbers in bookmarks"), "false", "false");
	clichild = insertEditableTreeWidget(cli, "bookmarksopen", i18n("Open up bookmark tree"), QString(), QString());
	clichild = insertEditableTreeWidget(cli, "pdfauthor", i18n("Text for PDF Author field"), QString(), QString());
	clichild = insertEditableTreeWidget(cli, "pdfcreator", i18n("Text for PDF Creator field"), i18n("LaTeX with hyperref package"), i18n("LaTeX with hyperref package"));
	clichild = insertEditableTreeWidget(cli, "pdffitwindow", i18n("Resize document window to fit document size"), "false", "false");
	clichild = insertEditableTreeWidget(cli, "pdfkeywords", i18n("Text for PDF Keywords field"), QString(), QString());
	clichild = insertEditableTreeWidget(cli, "pdfproducer", i18n("Text for PDF Producer field"), QString(), QString());
	clichild = insertEditableTreeWidget(cli, "pdfstartview", i18n("Starting view of PDF document"), "/Fit", "/Fit");
	clichild = insertEditableTreeWidget(cli, "pdfsubject", i18n("Text for PDF Subject field"), QString(), QString());
	clichild = insertEditableTreeWidget(cli, "pdftitle", i18n("Text for PDF Title field"), QString(), QString());

	cli = insertTreeWidget(m_lvPackages, "mathpazo", i18n("Use Palatino font as roman font (both text and math mode)"));
	cli = insertTreeWidget(m_lvPackages, "mathptmx", i18n("Use Times font as roman font (both text and math mode)"));
	cli = insertTreeWidget(m_lvPackages, "makeidx", i18n("Enable index generation"));
	cli = insertTreeWidget(m_lvPackages, "multicol", i18n("Enables multicolumn environments"));
	cli = insertTreeWidget(m_lvPackages, "pst-all", i18n("Load all pstricks packages"));
	cli = insertTreeWidget(m_lvPackages, "rotating", i18n("Rotates text"));
	cli = insertTreeWidget(m_lvPackages, "subfigure", i18n("Enables subfigures inside figures"));
	cli = insertTreeWidget(m_lvPackages, "upgreek", i18n("Typesetting capital Greek letters"));
	cli = insertTreeWidget(m_lvPackages, "xcolor", i18n("Extending LaTeX's color facilities"));

	cli = insertTreeWidget(m_lvPackages, "babel", i18n("Adds language specific support"));
	cli->setExpanded(true);
	cli->setCheckState(0, Qt::Checked);
	clichild = insertTreeWidget(cli, "acadian", "");
	clichild = insertTreeWidget(cli, "afrikaans", "");
	clichild = insertTreeWidget(cli, "american", "");
	clichild = insertTreeWidget(cli, "australian", "");
	clichild = insertTreeWidget(cli, "austrian", "");
	clichild = insertTreeWidget(cli, "bahasa", "");
	clichild = insertTreeWidget(cli, "basque", "");
	clichild = insertTreeWidget(cli, "brazil", "");
	clichild = insertTreeWidget(cli, "brazilian", "");
	clichild = insertTreeWidget(cli, "breton", "");
	clichild = insertTreeWidget(cli, "british", "");
	clichild = insertTreeWidget(cli, "bulgarian", "");
	clichild = insertTreeWidget(cli, "canadian", "");
	clichild = insertTreeWidget(cli, "canadien", "");
	clichild = insertTreeWidget(cli, "catalan", "");
	clichild = insertTreeWidget(cli, "croatian", "");
	clichild = insertTreeWidget(cli, "czech", "");
	clichild = insertTreeWidget(cli, "danish", "");
	clichild = insertTreeWidget(cli, "dutch", "");
	clichild = insertTreeWidget(cli, "english", "");
	clichild = insertTreeWidget(cli, "esperanto", "");
	clichild = insertTreeWidget(cli, "estonian", "");
	clichild = insertTreeWidget(cli, "finnish", "");
	clichild = insertTreeWidget(cli, "francais", "");
	clichild = insertTreeWidget(cli, "frenchb", "");
	clichild = insertTreeWidget(cli, "french", "");
	clichild = insertTreeWidget(cli, "galician", "");
	clichild = insertTreeWidget(cli, "german", "");
	clichild = insertTreeWidget(cli, "germanb", "");
	clichild = insertTreeWidget(cli, "greek", "");
	clichild = insertTreeWidget(cli, "polutonikogreek", "");
	clichild = insertTreeWidget(cli, "hebrew", "");
	clichild = insertTreeWidget(cli, "hungarian", "");
	clichild = insertTreeWidget(cli, "icelandic", "");
	clichild = insertTreeWidget(cli, "interlingua", "");
	clichild = insertTreeWidget(cli, "irish", "");
	clichild = insertTreeWidget(cli, "italian", "");
	clichild = insertTreeWidget(cli, "latin", "");
	clichild = insertTreeWidget(cli, "lowersorbian", "");
	clichild = insertTreeWidget(cli, "magyar", "");
	clichild = insertTreeWidget(cli, "naustrian", "");
	clichild = insertTreeWidget(cli, "newzealand", "");
	clichild = insertTreeWidget(cli, "ngerman", "");
	clichild = insertTreeWidget(cli, "norsk", "");
	clichild = insertTreeWidget(cli, "samin", "");
	clichild = insertTreeWidget(cli, "nynorsk", "");
	clichild = insertTreeWidget(cli, "polish", "");
	clichild = insertTreeWidget(cli, "portuges", "");
	clichild = insertTreeWidget(cli, "portuguese", "");
	clichild = insertTreeWidget(cli, "romanian", "");
	clichild = insertTreeWidget(cli, "russian", "");
	clichild = insertTreeWidget(cli, "scottish", "");
	clichild = insertTreeWidget(cli, "serbian", "");
	clichild = insertTreeWidget(cli, "slovak", "");
	clichild = insertTreeWidget(cli, "slovene", "");
	clichild = insertTreeWidget(cli, "spanish", "");
	clichild = insertTreeWidget(cli, "swedish", "");
	clichild = insertTreeWidget(cli, "turkish", "");
	clichild = insertTreeWidget(cli, "ukrainian", "");
	clichild = insertTreeWidget(cli, "uppersorbian", "");
	clichild = insertTreeWidget(cli, "welsh", "");
	clichild = insertTreeWidget(cli, "UKenglish", "");
	clichild = insertTreeWidget(cli, "USenglish", "");

	cli = insertTreeWidget(m_lvPackages, "fontenc", i18n("Use a font encoding scheme"));
	cli->setExpanded(true);
	cli->setCheckState(0, Qt::Checked);
	clichild = insertTreeWidget(cli, "HE8", "");
	clichild = insertTreeWidget(cli, "IL2", "");
	clichild = insertTreeWidget(cli, "LCH", "");
	clichild = insertTreeWidget(cli, "LCY", "");
	clichild = insertTreeWidget(cli, "LGR", "");
	clichild = insertTreeWidget(cli, "LHE", "");
	clichild = insertTreeWidget(cli, "LIT", "");
	clichild = insertTreeWidget(cli, "LO1", "");
	clichild = insertTreeWidget(cli, "LY1", "");
	clichild = insertTreeWidget(cli, "MTT", "");
	clichild = insertTreeWidget(cli, "OML", "");
	clichild = insertTreeWidget(cli, "OMS", "");
	clichild = insertTreeWidget(cli, "OT1", "");
	clichild = insertTreeWidget(cli, "OT2", "");
	clichild = insertTreeWidget(cli, "OT4", "");
	clichild = insertTreeWidget(cli, "PD1", "");
	clichild = insertTreeWidget(cli, "PU", "");
	clichild = insertTreeWidget(cli, "QX", "");
	clichild = insertTreeWidget(cli, "T1", "");
	clichild = insertTreeWidget(cli, "T2A", "");
	clichild = insertTreeWidget(cli, "T2B", "");
	clichild = insertTreeWidget(cli, "T2C", "");
	clichild = insertTreeWidget(cli, "T5", "");
	clichild = insertTreeWidget(cli, "TS1", "");
	clichild = insertTreeWidget(cli, "UT1", "");
	clichild = insertTreeWidget(cli, "X2", "");

	cli = insertTreeWidget(m_lvPackages, "graphicx", i18n("Support for including graphics"));
	cli->setExpanded(true);
	cli->setCheckState(0, Qt::Checked);
	clichild = insertTreeWidget(cli, "dvips", i18n("Specialize on graphic inclusion for dvips"));
	clichild = insertTreeWidget(cli, "pdftex", i18n("Specialize on graphic inclusion for pdftex"));
	clichild = insertTreeWidget(cli, "draft", i18n("Show only frames of graphics"));
}

// Try to read values from the config file:
//  - main entry:  selected,open,empty,empty,description
//  - child entry: selected,editable,defaultvalue,value,description

bool QuickDocument::readPackagesListview()
{
	KILE_DEBUG() << "\tread config: packages from config file";

	QStringList elements = KileConfig::packagesList();

	// clear packages dictionaries and listview
	m_dictPackagesEditable.clear();
	m_dictPackagesDefaultvalues.clear();
	m_lvPackages->clear();

	if (elements.empty())
		return false;

	// regular expression to split the string from the config file
	QRegExp reg("([^,]*),([^,]*),([^,]*),([^,]*),(.*)");

	KConfigGroup configGroup = m_config->group("QuickDocument/Packages");
	for (QStringList::Iterator it = elements.begin(); it != elements.end(); ++it) {
		QTreeWidgetItem *item;

		// look, if this is a main or a child entry
		KILE_DEBUG() << "\tread config entry: " << *it;
		int pos = (*it).indexOf('!');
		if (pos == -1) {                      // main entry
			item = new QTreeWidgetItem(m_lvPackages, QStringList(*it));
			item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
			item->setCheckState(0, Qt::Unchecked);
			if (reg.exactMatch(configGroup.readEntry(*it))) {
				if (reg.cap(1) == "1")          // selected state (entry 1)
					item->setCheckState(0, Qt::Checked);
				if (reg.cap(2) == "1")          // open state (entry 2)
					item->setExpanded(true);
				item->setText(2, reg.cap(5));    // description (entry 5)
			} else {
				KILE_DEBUG() << "\twrong config entry for package " << item->text(0);
			}
		} else {                              // child entry
			QList<QTreeWidgetItem*> items = m_lvPackages->findItems((*it).left(pos), Qt::MatchExactly);
			if (items.count() > 0) {
				item = items[0];
				QTreeWidgetItem *clichild;
				if (reg.exactMatch(configGroup.readEntry(*it))) {
					if (reg.cap(2) == "1") {                                       // editable state
						clichild = insertEditableTreeWidget(item, (*it).mid(pos + 1), reg.cap(5), reg.cap(4), reg.cap(3));
					} else {
						clichild = new QTreeWidgetItem(item, QStringList((*it).mid(pos + 1)));
						clichild->setFlags(clichild->flags() | Qt::ItemIsUserCheckable);
						clichild->setCheckState(0, Qt::Unchecked);
						clichild->setText(2, reg.cap(5));                           // description
					}
					if (reg.cap(1) == "1")                                         // selected state
						clichild->setCheckState(0, Qt::Checked);
				} else {
					KILE_DEBUG() << "\twrong config entry for package option " << item->text(0);
				}
			} else {
				KILE_DEBUG() << "\tlistview entry for package " << (*it).left(pos) << " not found";
			}
		}
	}

	return true;
}

void QuickDocument::writePackagesConfig()
{
	KILE_DEBUG() << "\twrite config: packages";

	QStringList packagesList;

	KConfigGroup configGroup = m_config->group("QuickDocument/Packages");
	for (int i = 0; i < m_lvPackages->topLevelItemCount(); ++i) {
		QTreeWidgetItem *currentItem = m_lvPackages->topLevelItem(i);
		KILE_DEBUG() << "\twrite config: " << currentItem->text(0);
		// add to packages list
		packagesList += currentItem->text(0);

		// determine config entry
		QString packageentry;

		// look for selected entries
		if (currentItem->checkState(0) == Qt::Checked)
			packageentry = "1,";
		else
			packageentry = "0,";

		// look if this listitem is opened
		if (currentItem->isExpanded())
			packageentry += "1,";
		else
			packageentry += "0,";

		// two dummy entries and finally the description
		packageentry += ",," + currentItem->text(2);

		// write listview entry
		configGroup.writeEntry(currentItem->text(0), packageentry);

		// look for children
		for (int j = 0; j < currentItem->childCount(); ++j) {
			QTreeWidgetItem *curchild = currentItem->child(j);
			// add child to packages list
			QString option = currentItem->text(0) + '!' + curchild->text(0);
			packagesList += option;
			KILE_DEBUG() << "\twrite config: " << option;

			// determine config entry
			QString optionentry;

			// look for selected options
			if (curchild->checkState(0) == Qt::Checked)
				optionentry = "1,";
			else
				optionentry = "0,";

			// look, if this child is editable
			if (m_dictPackagesEditable.contains(option)) {
				optionentry += "1,";
				if (m_dictPackagesDefaultvalues.contains(option))
					optionentry += m_dictPackagesDefaultvalues[option] + ',';
				else
					optionentry += ',';
			} else
				optionentry += "0,,";

			// add a value and a description
			optionentry += getPackagesValue(curchild->text(1))
			               + ',' + stripPackageDefault(option, curchild->text(2));

			// write listview entry
			configGroup.writeEntry(option, optionentry);
		}
	}

	// write the list of all packages
	KileConfig::setPackagesList(packagesList);
}

QTreeWidgetItem* QuickDocument::insertTreeWidget(QTreeWidget *treeWidget,
    const QString &entry,
    const QString &description)
{
	QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget, QStringList() << entry << "" << description);
	item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
	item->setCheckState(0, Qt::Unchecked);

	return item;
}

QTreeWidgetItem* QuickDocument::insertTreeWidget(QTreeWidgetItem *parent,
    const QString &entry,
    const QString &description)
{
	QTreeWidgetItem *item = new QTreeWidgetItem(parent, QStringList() << entry << "" << description);
	item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
	item->setCheckState(0, Qt::Unchecked);

	return item;
}

QTreeWidgetItem* QuickDocument::insertEditableTreeWidget(QTreeWidgetItem *parent,
    const QString &entry,
    const QString &description,
    const QString &value,
    const QString &defaultvalue)
{
	QTreeWidgetItem *item = new QTreeWidgetItem(parent, QStringList() << entry << "" << description);
	item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
	item->setCheckState(0, Qt::Unchecked);

	QString option = parent->text(0) + '!' + entry;
	m_dictPackagesEditable[option] = true;
	if (!defaultvalue.isEmpty())
		m_dictPackagesDefaultvalues[option] = defaultvalue;
	setPackagesValue(item, option, value);
	if (!description.isEmpty())
		item->setText(2, addPackageDefault(option, description));

	return item;
}

void QuickDocument::setPackagesValue(QTreeWidgetItem *item, const QString &option, const QString &val)
{
	QString defaultvalue = (m_dictPackagesDefaultvalues.contains(option))
	                       ? m_dictPackagesDefaultvalues[option] : QString();
	QString value = (! val.isEmpty()) ? val : QString();

	if (value == defaultvalue)
		item->setText(1, i18n("<default>"));
	else
		if (value.isEmpty())
			item->setText(1, i18n("<empty>"));
		else
			item->setText(1, value);
}

QString QuickDocument::getPackagesValue(const QString &value)
{
	return (value == i18n("<default>") || value == i18n("<empty>")) ? QString() : value;
}

bool QuickDocument::isTreeWidgetEntry(QTreeWidget *treeWidget, const QString &entry)
{
	return treeWidget->findItems(entry, Qt::MatchExactly).count() != 0;
}

bool QuickDocument::isTreeWidgetChild(QTreeWidget *treeWidget, const QString &entry, const QString &option)
{
	for (int i = 0; i < treeWidget->topLevelItemCount(); ++i) {
		QTreeWidgetItem *currentItem = treeWidget->topLevelItem(i);
		if (currentItem->text(0) == entry) {
			for (int j = 0; j < currentItem->childCount(); ++j) {
				QTreeWidgetItem *currentChild = currentItem->child(j);
				if (currentChild->text(0) == option) {
					return true;
				}
			}
			return false;
		}
	}
	return false;
}

QString QuickDocument::addPackageDefault(const QString &option, const QString &description)
{
	return (m_dictPackagesDefaultvalues.contains(option))
	       ? description + " [" + m_dictPackagesDefaultvalues[option] + ']'
	       : description + " [ ]";
}

QString QuickDocument::stripPackageDefault(const QString &option, const QString &description)
{
	QRegExp reg("(.*) \\[([^\\[]*)\\]");

	if (description.right(4) == " [ ]")
		return description.left(description.length() - 4);

	if (! reg.exactMatch(description))
		return description;

	return (reg.cap(2).isEmpty() ||
	        (m_dictPackagesDefaultvalues.contains(option) && m_dictPackagesDefaultvalues[option] == reg.cap(2))
	       ) ? reg.cap(1) : description;
}

////////////////////////////// hyperref tab //////////////////////////////

void QuickDocument::initHyperref()
{
	KILE_DEBUG() << "\tread config: init hyperref";

	QString driver =  "dvipdf,dvipdfm,dvips,dvipsone,"
	                  "dviwindo,hypertex,latex2html,pdftex,"
	                  "ps2pdf,tex4ht,textures,vtex";
	QStringList list = driver.split(',');

	m_dictHyperrefDriver.clear();
	for (int i = 0; i < list.count(); ++i) {
		m_dictHyperrefDriver[list[i]] = true;
	}
}

bool QuickDocument::isHyperrefDriver(const QString &name)
{
	return m_dictHyperrefDriver.contains(name);
}

////////////////////////////// check for existing exntries //////////////////////////////

bool QuickDocument::isDocumentClass(const QString &name)
{
	for (int i = 0; i < m_cbDocumentClass->count(); ++i) {
		if (m_cbDocumentClass->itemText(i) == name)
			return true;
	}
	return false;
}

bool QuickDocument::isDocumentClassOption(const QString &option)
{
	return isTreeWidgetEntry(m_lvClassOptions, option);
}

bool QuickDocument::isPackage(const QString &package)
{
	return isTreeWidgetEntry(m_lvPackages, package);
}

bool QuickDocument::isPackageOption(const QString &package, const QString &option)
{
	return isTreeWidgetChild(m_lvPackages, package, option);
}


////////////////////////////// print document template //////////////////////////////

void QuickDocument::printTemplate()
{
	KILE_DEBUG() << "==QuickDocument::printTemplate()============";

	// get current document class
	QString documentclass = m_cbDocumentClass->currentText();
	KILE_DEBUG() << "\tdocument class: " << documentclass;

	// build template
	m_td.tagBegin = "\\documentclass";

	// build options
	QString options;
	if (documentclass != "beamer") {
		if (!m_cbPaperSize->currentText().isEmpty())
			options += stripDefault(m_cbPaperSize->currentText()) + ',';
	}

	if (!m_cbTypefaceSize->currentText().isEmpty())
		options += stripDefault(m_cbTypefaceSize->currentText()) + ',';

	QTreeWidgetItemIterator it(m_lvClassOptions);
	while (*it) {
		if ((*it)->checkState(0) == Qt::Checked) {
			options += (*it)->text(0) + ',';
		}
		++it;
	}

	if (! options.isEmpty())
		m_td.tagBegin += '[' + options.left(options.length() - 1) + ']';
	m_td.tagBegin += '{' + documentclass + "}\n\n";


	QString enc = m_cbEncoding->currentText();
	if (!enc.isEmpty())
	{
		if (enc.indexOf("utf") != -1)
			m_td.tagBegin += "\\usepackage{ucs}\n";
		m_td.tagBegin += "\\usepackage[" + enc + "]{inputenc}\n";
	}
	if (documentclass != "beamer") {
		printPackages();
		printHyperref();
	} else {
		printBeamerTheme();
		printPackages();
	}

	if (!m_leAuthor->text().isEmpty())
		m_td.tagBegin += "\\author{" + m_leAuthor->text() + "}\n";
	if (!m_leTitle->text().isEmpty())
		m_td.tagBegin += "\\title{" + m_leTitle->text() + "}\n";
	if (!m_leDate->text().isEmpty())
		m_td.tagBegin += "\\date{" + m_leDate->text() + "}\n";
	m_td.tagBegin += '\n';

	m_td.tagBegin += "\\begin{document}\n%E%C";

	m_td.tagEnd = "\n\\end{document}\n";

	KILE_DEBUG() << "m_td.tagBegin " << m_td.tagBegin;
	KILE_DEBUG() << "m_td.tagEnd " << m_td.tagEnd;
}

void QuickDocument::printPackages()
{
	KILE_DEBUG() << "\tpackages";

	m_currentHyperref = false;
	m_hyperrefdriver.clear();
	m_hyperrefsetup.clear();

	for (int i = 0; i < m_lvPackages->topLevelItemCount(); ++i) {
		QTreeWidgetItem *cur = m_lvPackages->topLevelItem(i);

		if (cur->text(0) == "hyperref") {            // manage hyperref package
			m_currentHyperref = cur->checkState(0) == Qt::Checked;
			for (int j = 0; j < cur->childCount(); ++j) {
				QTreeWidgetItem *curchild = cur->child(j);
				if (curchild->checkState(0) == Qt::Checked) {              // manage hyperref option
					if (isHyperrefDriver(curchild->text(0))) {     // either hyperref driver
						if (! m_hyperrefdriver.isEmpty())
							m_hyperrefdriver += ',';
						m_hyperrefdriver += curchild->text(0);
					} else {
						QString value = curchild->text(1);          // or another option
						if (value != i18n("<default>")) {
							if (! m_hyperrefsetup.isEmpty())
								m_hyperrefsetup += ',';
							m_hyperrefsetup += "%\n   " + curchild->text(0) + '=' + getPackagesValue(curchild->text(1));
						}
					}
				}
			}
		} else
			if (cur->checkState(0) == Qt::Checked) {                   // manage other package options
				QString packageOptions;
				for (int j = 0; j < cur->childCount(); ++j) {
					QTreeWidgetItem *curchild = cur->child(j);
					if (curchild->checkState(0) == Qt::Checked) {
						QString optiontext;
						if (m_dictPackagesEditable.contains(cur->text(0) + '!' + curchild->text(0))) {
							QString value = curchild->text(1);
							if (value != i18n("<default>"))
								optiontext = curchild->text(0) + '=' + getPackagesValue(curchild->text(1));
						} else
							optiontext = curchild->text(0);

						if (! optiontext.isEmpty()) {
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
	if (! m_currentHyperref)
		return;

	KILE_DEBUG() << "\thyperref";

	// output hyperref package
	m_td.tagBegin += "\\usepackage";
	if (! m_hyperrefdriver.isEmpty())
		m_td.tagBegin += '[' + m_hyperrefdriver + ']';
	m_td.tagBegin += "{hyperref}\n";

	// output hyperref options
	if (! m_hyperrefsetup.isEmpty()) {
		m_td.tagBegin += "\\hypersetup{" + m_hyperrefsetup + "%\n}\n";
	}

	m_td.tagBegin += '\n';


}

void QuickDocument::printBeamerTheme()
{
	KILE_DEBUG() << "\tbeamer theme";

	QString theme = m_cbPaperSize->currentText();
	QRegExp reg("(\\w+)\\s+\\((.*)\\)$");

	if (reg.indexIn(theme) >= 0) {
		QStringList optionlist = reg.cap(2).split(',');
		m_td.tagBegin += "\\usepackage[" + optionlist.join(",") + "]{beamertheme" + reg.cap(1) + "}\n\n";
	}
	else {
		m_td.tagBegin += "\\usepackage{beamertheme" + theme + "}\n\n";
	}
}

////////////////////////////// Slots //////////////////////////////

void QuickDocument::slotButtonClicked(int button)
{
	KILE_DEBUG() << "called";

	if (button == KDialog::Ok){

		// get current class options
		m_currentClass = m_cbDocumentClass->currentText();
		KILE_DEBUG() << "current class: " << m_currentClass;

		// save the checked options
		m_dictDocumentClasses[m_currentClass][qd_SelectedOptions] = getClassOptions();
		KILE_DEBUG() << "save options: " << m_dictDocumentClasses[m_currentClass][qd_SelectedOptions];

		// build template
		printTemplate();

		// update config file
		writeConfig();

		accept();
	}
	else{
                 KDialog::slotButtonClicked(button);
	}
}

////////////////////////////// slots: document class
void QuickDocument::slotDocumentClassAdd()
{
	KILE_DEBUG() << "==QuickDocument::slotDocumentClassAdd()============";
	QStringList list;
	list << i18n("Document Class")
	<< "label,edit,label,combobox,checkbox,checkbox"
	<< i18n("Please enter the new document &class:")
	<< QString()                                     // 3
	<< i18n("&Set all options from this standard class (optional):")
	<< ",article,book,letter,report,scrartcl,scrbook,scrreprt"    // 5
	<< i18n("Use standard &fontsizes")                   // 6
	<< i18n("Use standard &papersizes")                  // 7
	;

	if (inputDialog(list, qd_CheckNotEmpty | qd_CheckDocumentClass)) {
		QString classname = list[3];

		QStringList classlist;
		if (list[5].isEmpty()) {            // no base class
			QString useFontsizes = (list[6] == "true")
			                       ? "10pt,11pt,12pt" : "";
			QString usePapersizes = (list[7] == "true")
			                        ? "a4paper,a5paper,b5paper,executivepaper,legalpaper,letterpaper" : "";
			KILE_DEBUG() << "\tadd document class: " << classname
			<< " fontsize=" << list[6] << " papersize=" << list[7];

			// set default entries for the documentClass-dictionary
			classlist <<  useFontsizes << usePapersizes << "" << "";
		}
		else {                              // based on a standard class
			// first get the first four parameters
			classlist = m_dictDocumentClasses[list[5]];
			// then add all baseclass options
			QStringList optionlist;
			initStandardOptions(list[5], optionlist);
			for (int i = 0; i < optionlist.count(); ++i) {
				classlist.append(optionlist[i]);
			}
		}

		// insert the stringlist for this new document class
		m_dictDocumentClasses[classname] = classlist;

		fillDocumentClassCombobox();

		// add the new document class into the userClasslist and the documentClass-combobox
		m_userClasslist.append(classname);

		// activate the new document class
		m_cbDocumentClass->addItem(classname);
		m_cbDocumentClass->setCurrentIndex(m_cbDocumentClass->count() - 1);
		slotDocumentClassChanged(m_cbDocumentClass->count() - 1);
	}
}

void QuickDocument::slotDocumentClassDelete()
{
	// get the name of the current class
	QString documentclass = m_cbDocumentClass->currentText();

	KILE_DEBUG() << "==QuickDocument::slotDocumentClassDelete()============";
	if (KMessageBox::warningContinueCancel(this, i18n("Do you want to remove \"%1\" from the document class list?", documentclass),
	                                             i18n("Remove Document Class")) == KMessageBox::Continue) {
		KILE_DEBUG() << "\tlazy delete class: " << documentclass;

		// remove this document class from the documentClass-dictionary
		m_dictDocumentClasses.remove(documentclass);

		// mark this document class for deleting from config file (only with OK-Button)
		if (m_deleteDocumentClasses.indexOf(documentclass) == -1) {
			m_deleteDocumentClasses.append(documentclass);
		}

		// remove it from the list of userclasses
		m_userClasslist.removeAll(documentclass);

		// and finally remove it from the combobox
		int i = m_cbDocumentClass->currentIndex();
		m_cbDocumentClass->removeItem(i);

		// init a new document class
		m_currentClass = m_cbDocumentClass->currentText();
		KILE_DEBUG() << "\tchange class:  --> " << m_currentClass;
		initDocumentClass();
	}
}

void QuickDocument::slotDocumentClassChanged(int index)
{
	KILE_DEBUG() << "==QuickDocument::slotDocumentClassChanged()============";
	if (m_cbDocumentClass->itemText(index).isEmpty()) {
		KILE_DEBUG() << "\tempty";
		return;
	}

	// get old and new document class
	QString oldclass = m_currentClass;
	m_currentClass = m_cbDocumentClass->itemText(index);
	KILE_DEBUG() << "\tchange class: " << oldclass << " --> " << m_currentClass;

	// save the checked options
	m_dictDocumentClasses[oldclass][qd_SelectedOptions] = getClassOptions();
	KILE_DEBUG() << "\tsave options: " << m_dictDocumentClasses[oldclass][qd_SelectedOptions];

	// init the new document class
	initDocumentClass();
}

void QuickDocument::slotTypefaceSizeAdd()
{
	KILE_DEBUG() << "==QuickDocument::slotTypefaceSizeAdd()============";
	QStringList list;
	list << i18n("Add Fontsize")
	<< "label,edit"
	<< i18n("Please enter the &fontsizes (comma-separated list):")
	<< QString()             // 3
	;

	if (inputDialog(list, qd_CheckNotEmpty | qd_CheckFontsize)) {
		KILE_DEBUG() << "\tadd fontsize: " << list[3];
		addComboboxEntries(m_cbTypefaceSize, "fontsize", list[3]);

		// save the new list of fontsizes
		m_dictDocumentClasses[m_currentClass][qd_Fontsizes] = getComboxboxList(m_cbTypefaceSize);

		// enable/disable buttons to add or delete entries
		slotEnableButtons();
	}
}

void QuickDocument::slotTypefaceSizeDelete()
{
	if (KMessageBox::warningContinueCancel(this, i18n("Do you want to remove \"%1\" from the fontsize list?", m_cbTypefaceSize->currentText()), i18n("Remove Fontsize")) == KMessageBox::Continue)
	{
		int i = m_cbTypefaceSize->currentIndex();
		m_cbTypefaceSize->removeItem(i);

		// save the new list of fontsizes
		m_dictDocumentClasses[m_currentClass][qd_Fontsizes] = getComboxboxList(m_cbTypefaceSize);

		// enable/disable buttons to add or delete entries
		slotEnableButtons();
	}
}

void QuickDocument::slotPaperSizeAdd()
{
	KILE_DEBUG() << "==QuickDocument::slotPaperSizeAdd()============";
	QStringList list;
	list << i18n("Add Papersize")
	<< "label,edit"
	<< i18n("Please enter the &papersizes (comma-separated list):")
	<< QString()                 // 3
	;

	if (inputDialog(list, qd_CheckNotEmpty | qd_CheckPapersize)) {
		KILE_DEBUG() << "\tadd papersize: " << list[3];
		addComboboxEntries(m_cbPaperSize, "papersize", list[3]);

		// save the new list of papersizes
		m_dictDocumentClasses[m_currentClass][qd_Papersizes] = getComboxboxList(m_cbPaperSize);

		// enable/disable buttons to add or delete entries
		slotEnableButtons();
	}
}

void QuickDocument::slotPaperSizeDelete()
{
	if (KMessageBox::warningContinueCancel(this, i18n("Do you want to remove \"%1\" from the papersize list?", m_cbPaperSize->currentText()), i18n("Remove Papersize")) == KMessageBox::Continue)
	{
		int i = m_cbPaperSize->currentIndex();
		m_cbPaperSize->removeItem(i);

		// save the new list of papersizes
		m_dictDocumentClasses[m_currentClass][qd_Papersizes] = getComboxboxList(m_cbPaperSize);

		// enable/disable buttons to add or delete entries
		slotEnableButtons();
	}
}

////////////////////////////// slots: document class button //////////////////////////////

void QuickDocument::slotClassOptionAdd()
{
	KILE_DEBUG() << "==QuickDocument::slotClassOptionAdd()============";
	QStringList list;
	list << i18n("Add Option")
	<< "label,edit,label,edit,checkbox"
	<< i18n("Name of &option:")
	<< QString()                  // 3
	<< i18n("&Description:")
	<< QString()                  // 5
	<< i18n("&Select this option")    // 6
	;

	if (inputDialog(list, qd_CheckNotEmpty | qd_CheckClassOption)) {
		// get results
		QString option = list[3];
		QString description = list[5];
		bool check = (list[6] == "true");

		// add class option
		KILE_DEBUG() << "\tadd option: " << option << " (" << description << ") checked=" << list[6];
		QTreeWidgetItem *twi = new QTreeWidgetItem(m_lvClassOptions, QStringList() << option << description);
		twi->setFlags(twi->flags() | Qt::ItemIsUserCheckable);
		twi->setCheckState(0, check ? Qt::Checked : Qt::Unchecked);

		// update dictionary
		updateClassOptions();
	}
}

void QuickDocument::slotClassOptionEdit()
{
	if (m_lvClassOptions->selectedItems().count() == 0)
		return;

	QTreeWidgetItem *cur = m_lvClassOptions->selectedItems()[0];

	KILE_DEBUG() << "==QuickDocument::slotClassOptionEdit()============";
	QStringList list;
	list << i18n("Edit Option")
	<< "label,edit-r,label,edit"
	<< i18n("Name of &option:")
	<< cur->text(0)
	<< i18n("&Description:")
	<< stripDefault(cur->text(1))           // 5
	;

	//if ( inputDialog(list,qd_CheckNotEmpty | qd_CheckClassOption) ) {
	if (inputDialog(list)) {
		// get results
		//QString option = list[3];
		QString description = list[5];

		// set changed class option
		KILE_DEBUG() << "\tedit option: " << cur->text(0) << " (" << description << ")";
		//cur->setText(0, option);
		cur->setText(1, description);

		// update dictionary
		updateClassOptions();
	}
}

void QuickDocument::slotClassOptionDelete()
{
	KILE_DEBUG() << "==QuickDocument::slotClassOptionDelete()============";
	if (m_lvClassOptions->selectedItems().count() > 0 && (KMessageBox::warningContinueCancel(this, i18n("Do you want to delete this class option?"), i18n("Delete")) == KMessageBox::Continue)) {
		QTreeWidgetItem *cur = m_lvClassOptions->selectedItems()[0];

		KILE_DEBUG() << "\tdelete option: " << cur->text(0) << " (" << cur->text(1) << ")";
		m_lvClassOptions->takeTopLevelItem(m_lvClassOptions->indexOfTopLevelItem(cur));

		// update dictionary
		updateClassOptions();
	}
}

void QuickDocument::slotOptionDoubleClicked(QTreeWidgetItem *item, int column)
{
	Q_UNUSED(column);

	item->setCheckState(0, item->checkState(0) == Qt::Checked ? Qt::Unchecked : Qt::Checked);
}

////////////////////////////// slots: packages //////////////////////////////

void QuickDocument::slotPackageAdd()
{
	KILE_DEBUG() << "==QuickDocument::slotPackageAdd()============";
	QStringList list;
	list << i18n("Add Package")
	<< "label,edit,label,edit,checkbox"
	<< i18n("&Package:")
	<< QString()                        // 3
	<< i18n("&Description:")
	<< QString()                        // 5
	<< i18n("&Select this package")         // 6
	;

	if (inputDialog(list, qd_CheckNotEmpty | qd_CheckPackage)) {
		KILE_DEBUG() << "\tadd package: " << list[3] << " (" << list[5] << ") checked=" << list[6];
		QTreeWidgetItem *cli = new QTreeWidgetItem(m_lvPackages, QStringList() << list[3] << "" << list[5]);
		cli->setFlags(cli->flags() | Qt::ItemIsUserCheckable);
		cli->setCheckState(0, list[6] == "true" ? Qt::Checked : Qt::Unchecked);
	}
}

void QuickDocument::slotPackageAddOption()
{
	if (m_lvPackages->selectedItems().count() == 0)
		return;

	QTreeWidgetItem *cur = m_lvPackages->selectedItems()[0];

	KILE_DEBUG() << "==QuickDocument::packageAddOption()============";
	QStringList list;
	list << i18n("Add Option")
	<< "label,edit,checkbox,label,edit,label,edit,label,edit,checkbox"
	<< i18n("&Option:") + " (" + i18n("package:") + ' ' + cur->text(0) + ')'
	<< QString()                   // 3
	<< i18n("&Editable")               // 4
	<< i18n("De&fault value:")
	<< QString()                   // 6
	<< i18n("&Value:")
	<< QString()                   // 8
	<< i18n("&Description:")
	<< QString()                   // 10
	<< i18n("&Select this option")     // 11
	;

	if (!cur->parent() && inputDialog(list, qd_CheckNotEmpty | qd_CheckPackageOption)) {
		KILE_DEBUG() << "\tadd option: " << list[3] << " (" << list[10] << ") checked=" << list[11];

		QTreeWidgetItem *cli;
		if (list[4] == "true") {
			cli = insertEditableTreeWidget(cur, list[3], list[10], list[8], list[6]);
		} else {
			cli = new QTreeWidgetItem(cur, QStringList() << list[3] << "" << list[10]);
			cli->setFlags(cli->flags() | Qt::ItemIsUserCheckable);
			cli->setCheckState(0, Qt::Unchecked);
		}
		if (list[11] == "true")
			cli->setCheckState(0, Qt::Checked);
		cur->setExpanded(true);
	}

}

void QuickDocument::slotPackageEdit()
{
	if (m_lvPackages->selectedItems().count() == 0)
		return;

	QTreeWidgetItem *cur = m_lvPackages->selectedItems()[0];

	KILE_DEBUG() << "==QuickDocument::slotPackageEdit()============";
	bool editableOption;
	QString caption, labelText, optionname;

	if (cur->parent()) {
//  checkmode = qd_CheckPackageOption;
		caption = i18n("Edit Option");
		labelText = i18n("Op&tion:")  + " (" + i18n("package:") + ' ' + cur->parent()->text(0) + ')';
		optionname = cur->parent()->text(0) + '!' + cur->text(0);
		editableOption = m_dictPackagesEditable.contains(optionname);
	} else {
//  checkmode = qd_CheckPackage;
		caption = i18n("Edit Package");
		labelText = i18n("&Package:");
		optionname.clear();
		editableOption = false;
	}

	// create one of three different dialogs; edit package, edit editable option, edit option
	QStringList list;
	list << caption;
	if (editableOption) {
		QString defaultvalue = (m_dictPackagesDefaultvalues.contains(optionname))
		                       ? m_dictPackagesDefaultvalues[optionname]
		                       : QString();
		QString value = (cur->text(1) == i18n("<default>"))
		                ? defaultvalue : getPackagesValue(cur->text(1));

		list << "label,edit-r,label,edit-r,label,edit,label,edit"
		<< labelText
		<< cur->text(0)                           // 3
		<< i18n("De&fault value:")
		<< defaultvalue                           // 5
		<< i18n("&Value:")
		<< value                                  // 7
		<< i18n("&Description:")
		<< stripPackageDefault(optionname, cur->text(2))     // 9
		;
	} else {
		list << "label,edit-r,label,edit"
		<< labelText
		<< cur->text(0)                           // 3
		<< i18n("&Description:")
		<< cur->text(2)                           // 5
		;
	}

	if (inputDialog(list)) {
		if (editableOption) {
			KILE_DEBUG() << "\tedit package: "
			<< list[3]
			<< " (" << list[7] << ") "
			<< " (" << list[9] << ")";
			cur->setText(0, list[3]);
			setPackagesValue(cur, optionname, list[7]);
			cur->setText(2, addPackageDefault(optionname, list[9]));
		} else {
			KILE_DEBUG() << "\tedit package: " << list[3] << " (" << list[5] << ")";
			cur->setText(0, list[3]);
			cur->setText(2, list[5]);
		}
	}
}

void QuickDocument::slotPackageDelete()
{
	if (m_lvPackages->selectedItems().count() == 0)
		return;


	QTreeWidgetItem *cur = m_lvPackages->selectedItems()[0];

	bool packageoption;
	QString message, optionname;
	if (cur->parent()) {
		packageoption = true;
		message = i18n("Do you want to delete this package option?");
		optionname = cur->parent()->text(0) + '!' + cur->text(0);
	} else {
		packageoption = false;
		message = i18n("Do you want to delete this package?");
		optionname = cur->text(0);
	}

	if (KMessageBox::warningContinueCancel(this, message, i18n("Delete")) == KMessageBox::Continue) {
		while (cur->childCount() > 0) {
			cur->takeChild(0);
		}
		m_lvPackages->takeTopLevelItem(m_lvPackages->indexOfTopLevelItem(cur));

		// also delete entries for editable package option
		if (packageoption && m_dictPackagesEditable.contains(optionname)) {
			m_dictPackagesEditable.remove(optionname);
			if (m_dictPackagesDefaultvalues.contains(optionname))
				m_dictPackagesDefaultvalues.remove(optionname);
		}
	}
}

void QuickDocument::slotPackageReset()
{
	if (KMessageBox::warningContinueCancel(this, i18n("Do you want to reset this package list?"), i18n("Reset Package List")) == KMessageBox::Continue)
	{
		KILE_DEBUG() << "\treset packages";

		initPackages();
		slotEnableButtons();
	}
}

void QuickDocument::slotCheckParent(QTreeWidgetItem *item)
{
	if (item && item->checkState(0) == Qt::Checked && item->parent()) {
		item->parent()->setCheckState(0, Qt::Checked);
	}
}

void QuickDocument::slotPackageDoubleClicked(QTreeWidgetItem *item)
{
	if (item && item->parent()) {
		QString option = item->parent()->text(0) + '!' + item->text(0);
		if (m_dictPackagesEditable.contains(option))
			slotPackageEdit();
	}
}

////////////////////////////// button states //////////////////////////////

void QuickDocument::slotEnableButtons()
{
	bool enable;

	enable = (! isStandardClass(m_currentClass));

	// add/delete button
	m_btnDocumentClassDelete->setEnabled(enable);
	m_btnTypefaceSizeAdd->setEnabled(enable);
	m_btnTypefaceSizeDelete->setEnabled(enable && m_cbTypefaceSize->count() > 0);
	m_btnPaperSizeAdd->setEnabled(enable);
	m_btnPaperSizeDelete->setEnabled(enable && m_cbPaperSize->count() > 0);

	// class options
	m_btnClassOptionsAdd->setEnabled(enable);
	enable = (enable && (m_lvClassOptions->selectedItems().count() != 0));
	m_btnClassOptionsEdit->setEnabled(enable);
	m_btnClassOptionsDelete->setEnabled(enable);

	// packeges
	if (m_lvPackages->selectedItems().count() > 0 && m_lvPackages->selectedItems()[0]->text(0) != "hyperref") {
		m_btnPackagesEdit->setEnabled(true);
		m_btnPackagesDelete->setEnabled(true);
		if (m_lvPackages->selectedItems()[0]->parent())
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
	QuickDocumentInputDialog *dialog = new QuickDocumentInputDialog(list, check, this, "inputDialog");

	bool result = false;
	if (dialog->exec()) {
		dialog->getResults(list);
		result = true;
	}

	delete dialog;
	return result;

}

QuickDocumentInputDialog::QuickDocumentInputDialog(const QStringList &list, int check,
    QuickDocument *parent,
    const char *name)
		: KDialog(parent), m_parent(parent), m_check(check)
{
	setObjectName(name);
	setCaption(list[0]);
	setModal(true);
	setButtons(Ok | Cancel);
	setDefaultButton(Ok);
	showButtonSeparator(true);

	QWidget *page = new QWidget(this);
	setMainWidget(page);
	QVBoxLayout *vl = new QVBoxLayout();
	vl->setMargin(0);
	vl->setSpacing(spacingHint());
	page->setLayout(vl);

	int firstlinedit = -1;
	m_description = list[1].split(',');
	for (int i = 0; i < m_description.count(); ++i) {
		// create the object
		if (m_description[i] == "label") {
			m_objectlist.append(new QLabel(list[i+2], page));
		}
		else
			if (m_description[i] == "checkbox") {
				m_objectlist.append(new QCheckBox(list[i+2], page));
			}
			else
				if (m_description[i] == "combobox") {
					KComboBox *combobox = new KComboBox(page);
					combobox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
					combobox->setDuplicatesEnabled(false);
					combobox->addItems(list[i+2].split(',', QString::KeepEmptyParts));
					if (i > 0 && m_description[i-1] == "label") {
						((QLabel *)m_objectlist[i-1])->setBuddy(combobox);
					}
					m_objectlist.append(combobox);
				}
				else {
					m_objectlist.append(new KLineEdit(list[i+2], page));
					if (m_description[i] == "edit-r") {
						((KLineEdit *)m_objectlist[i])->setReadOnly(true);
					}
					else
						if (firstlinedit == -1) {
							firstlinedit = i;
						}
					if (i > 0 && m_description[i-1] == "label") {
						((QLabel *)m_objectlist[i-1])->setBuddy(m_objectlist[i]);
					}
				}

		// insert the new object into the layout
		vl->addWidget(m_objectlist[i]);
	}

	if (firstlinedit != -1)
		m_objectlist[firstlinedit]->setFocus();
	vl->addStretch(1);
	page->setMinimumWidth(350);
}

QuickDocumentInputDialog::~QuickDocumentInputDialog()
{}

void QuickDocumentInputDialog::getResults(QStringList &list)
{
	for (int i = 0; i < m_description.count(); ++i) {
		if (m_description[i] == "label") {
			list[i+2] = ((QLabel *)m_objectlist[i])->text();
		}
		else
			if (m_description[i] == "checkbox") {
				list[i+2] = (((QCheckBox *)m_objectlist[i])->isChecked()) ? "true" : "false";
			}
			else
				if (m_description[i] == "combobox") {
					list[i+2] = ((KComboBox *)m_objectlist[i])->currentText();
				}
				else  {
					list[i+2] = ((KLineEdit *)m_objectlist[i])->text().simplified();
				}
	}
}

// get the package name from string 'Option: (package: name)'
QString QuickDocumentInputDialog::getPackageName(const QString &text)
{
	QRegExp reg(i18n("package:") + " ([^\\)]+)");
	return (reg.indexIn(text) >= 0) ? reg.cap(1) : QString();
}

bool QuickDocumentInputDialog::checkListEntries(const QString &title, const QString &textlist,
    const QString &pattern)
{
	// split entries (one or a comma separated list)
	QStringList list = textlist.split(',');

	for (int i = 0; i < list.count(); ++i) {
		QString s = list[i].trimmed();
		// entries must match a regular expression
		QRegExp reg(pattern);
		if (!reg.exactMatch(s)) {
			KMessageBox::error(this, i18n("%1 '%2' is not allowed.", title, s));
			return false;
		}
	}
	return true;
}

// check the main result of the input dialog
void QuickDocumentInputDialog::slotButtonClicked(int button)
{
	if(button == KDialog::Ok){

		if (m_check) {
			// get the label and main input string from the first label/linedit
			QString inputlabel = ((QLabel *)m_objectlist[0])->text();
			QString input = ((KLineEdit *)m_objectlist[1])->text().simplified();

			// should we check for an empty string
			if ((m_check & qd_CheckNotEmpty) && input.isEmpty()) {
				KMessageBox::error(this, i18n("An empty string is not allowed."));
				return;
			}

			// should we check for an existing document class
			if (m_check & qd_CheckDocumentClass) {
				if (m_parent->isDocumentClass(input)) {
					KMessageBox::error(this, i18n("This document class already exists."));
					return;
				}

				QRegExp reg("\\w+");
				if (!reg.exactMatch(input)) {
					KMessageBox::error(this, i18n("This name is not allowed for a document class."));
					return;
				}
			}

			// should we check for an existing document class option
			if ((m_check & qd_CheckClassOption) && m_parent->isDocumentClassOption(input)) {
				KMessageBox::error(this, i18n("This document class option already exists."));
				return;
			}

			// should we check for an existing package
			if ((m_check & qd_CheckPackage) && m_parent->isPackage(input)) {
				KMessageBox::error(this, i18n("This package already exists."));
				return;
			}

			// should we check for an existing package option
			if (m_check & qd_CheckPackageOption) {
				QString package = getPackageName(inputlabel);
				if (package.isEmpty()) {
					KMessageBox::error(this, i18n("Could not identify the package name."));
					return;
				}
				if (m_parent->isPackageOption(package, input)) {
					KMessageBox::error(this, i18n("This package option already exists."));
					return;
				}
			}

			// should we check for a (list of) fontsizes
			if ((m_check & qd_CheckFontsize) && !checkListEntries("Fontsize", input, "\\d+pt")) {
				return;
			}

			// should we check for a (list of) papersizes
			if ((m_check & qd_CheckPapersize) && !checkListEntries("Papersize", input, "\\w+")) {
				return;
			}
		}
		accept();
	}
	else{
		KDialog::slotButtonClicked(button);
	}
}

} // namespace

#include "quickdocumentdialog.moc"
