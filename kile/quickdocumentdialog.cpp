//
// C++ Implementation: quickdocheader
//
// Description:
//
//
// Author: Thomas Fischer <t-fisch@users.sourceforge.net>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <qtabwidget.h>
#include <qlayout.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qwhatsthis.h>

#include <kcombobox.h>
#include <klocale.h>
#include <kconfig.h>
#include <klineedit.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <kmessagebox.h>

#include "quickdocumentdialog.h"

namespace KileDialog
{

QuickDocument::QuickDocument(KConfig *config, QWidget *parent, const char *name, const QString &caption) : Wizard(config, parent,name,caption)
{
	setupGUI();
	init();
	readConfig();
	slotEnableButtons();
}


QuickDocument::~QuickDocument()
{}

/*!
    \fn QuickDocument::setupGUI
 */
void QuickDocument::setupGUI()
{
	QLabel *label;
	KPushButton *button;
	QGridLayout *gl;
	QWidget *frame, *container;
	QHBoxLayout *hl;

	QTabWidget *tabWidget = new QTabWidget( this );
	setMainWidget(tabWidget);

	QWidget *classOptions = new QWidget( tabWidget );
	tabWidget->addTab(classOptions, i18n("Cla&ss Options"));
	gl = new QGridLayout(classOptions, 9, 2, marginHint(), spacingHint());

	container = new QWidget(classOptions);
	gl->addWidget(container, 0, 1);
	hl = new QHBoxLayout(container, 0, spacingHint());
	m_cbDocumentClass = new KComboBox(container);
	m_cbDocumentClass->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
	m_cbDocumentClass->setEditable(true);
	m_cbDocumentClass->setDuplicatesEnabled(false);
	hl->addWidget(m_cbDocumentClass);
	button = new KPushButton(SmallIcon("edit_add"), "", container);
	QWhatsThis::add(button, i18n("Add current text to this list"));
	connect(button, SIGNAL(clicked()), this, SLOT(slotDocumentClassAdd()));
	hl->addWidget(button);

	button = new KPushButton(SmallIcon("eraser"), "", container);
	QWhatsThis::add(button, i18n("Remove current element from this list"));
	connect(button, SIGNAL(clicked()), this, SLOT(slotDocumentClassDelete()));
	hl->addWidget(button);

	button = new KPushButton(SmallIcon("reload"), "", container);
	QWhatsThis::add(button, i18n("Reset this list to the default values."));
	connect(button, SIGNAL(clicked()), this, SLOT(slotDocumentClassReset()));
	hl->addWidget(button);

	label = new QLabel(i18n("Doc&ument class:"), classOptions);
	gl->addWidget(label, 0, 0);
	label->setBuddy(m_cbDocumentClass);
	label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

	m_cbTypefaceSize = new KComboBox(classOptions);
	gl->addWidget(m_cbTypefaceSize, 2, 1);
	label = new QLabel(i18n("&Typeface size:"), classOptions);
	gl->addWidget(label, 2, 0);
	label->setBuddy(m_cbTypefaceSize);
	label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

	container = new QWidget(classOptions);
	gl->addWidget(container, 3, 1);
	hl = new QHBoxLayout(container, 0, spacingHint());
	m_cbPaperSize = new KComboBox(container);
	m_cbPaperSize->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
	m_cbPaperSize->setEditable(true);
	m_cbPaperSize->setDuplicatesEnabled(false);
	hl->addWidget(m_cbPaperSize);
	button = new KPushButton(SmallIcon("edit_add"), "", container);
	QWhatsThis::add(button, i18n("Add current text to this list"));
	connect(button, SIGNAL(clicked()), this, SLOT(slotPaperSizeAdd()));
	hl->addWidget(button);
	button = new KPushButton(SmallIcon("eraser"), "", container);
	QWhatsThis::add(button, i18n("Remove current element from this list"));
	connect(button, SIGNAL(clicked()), this, SLOT(slotPaperSizeDelete()));
	hl->addWidget(button);
	button = new KPushButton(SmallIcon("reload"), "", container);
	QWhatsThis::add(button, i18n("Reset this list to the default values."));
	connect(button, SIGNAL(clicked()), this, SLOT(slotPaperSizeReset()));
	hl->addWidget(button);
	label = new QLabel(i18n("Paper si&ze:"), classOptions);
	gl->addWidget(label, 3, 0);
	label->setBuddy(m_cbPaperSize);
	label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

	container = new QWidget(classOptions);
	gl->addWidget(container, 4, 1);
	hl = new QHBoxLayout(container, 0, spacingHint());
	m_cbEncoding = new KComboBox(container);
	m_cbEncoding->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
	m_cbEncoding->setEditable(true);
	m_cbEncoding->setDuplicatesEnabled(false);
	hl->addWidget(m_cbEncoding);
	button = new KPushButton(SmallIcon("edit_add"), "", container);
	QWhatsThis::add(button, i18n("Add current text to this list"));
	connect(button, SIGNAL(clicked()), this, SLOT(slotEncodingAdd()));
	hl->addWidget(button);
	button = new KPushButton(SmallIcon("eraser"), "", container);
	QWhatsThis::add(button, i18n("Remove current element from this list"));
	connect(button, SIGNAL(clicked()), this, SLOT(slotEncodingDelete()));
	hl->addWidget(button);
	button = new KPushButton(SmallIcon("reload"), "", container);
	QWhatsThis::add(button, i18n("Reset this list to the default values."));
	connect(button, SIGNAL(clicked()), this, SLOT(slotEncodingReset()));
	hl->addWidget(button);
	label = new QLabel(i18n("E&ncoding:"), classOptions);
	gl->addWidget(label, 4, 0);
	label->setBuddy(m_cbEncoding);
	label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

	m_lvClassOptions = new QListView(classOptions);
	gl->addWidget(m_lvClassOptions, 7, 1);
	m_lvClassOptions->addColumn(i18n("Option"));
	m_lvClassOptions->addColumn(i18n("Description"));
	connect(m_lvClassOptions, SIGNAL(selectionChanged()), this, SLOT(slotEnableButtons()));
	label = new QLabel(i18n("C&lass options:"), classOptions);
	gl->addWidget(label, 7, 0);
	label->setBuddy(m_lvClassOptions);
	label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
	label->setAlignment(Qt::AlignTop);

	frame = new QWidget(classOptions);
	gl->addWidget(frame, 8, 1);
	hl = new QHBoxLayout(frame, 0, spacingHint());
	hl->addStretch(1);

	button = new KPushButton(SmallIcon("edit_add"), i18n("&Add"), frame);
	QWhatsThis::add(button, i18n("Add a new class option"));
	connect(button, SIGNAL(clicked()), this, SLOT(slotClassOptionAdd()));
	hl->addWidget(button);

	m_btnClassOptionsEdit = new KPushButton(SmallIcon("edit"), i18n("Ed&it"), frame);
	QWhatsThis::add(m_btnClassOptionsEdit, i18n("Edit the current class option"));
	connect(m_btnClassOptionsEdit, SIGNAL(clicked()), this, SLOT(slotClassOptionEdit()));
	hl->addWidget(m_btnClassOptionsEdit);

	m_btnClassOptionsDelete = new KPushButton(SmallIcon("eraser"), i18n("D&elete"), frame);
	QWhatsThis::add(m_btnClassOptionsDelete, i18n("Remove the current class option"));
	connect(m_btnClassOptionsDelete, SIGNAL(clicked()), this, SLOT(slotClassOptionDelete()));
	hl->addWidget(m_btnClassOptionsDelete);

	button = new KPushButton(SmallIcon("reload"), i18n("&Reset to defaults"), frame);
	QWhatsThis::add(button, i18n("Reset this list to the default values."));
	connect(button, SIGNAL(clicked()), this, SLOT(slotClassOptionReset()));
	hl->addWidget(button);

	QWidget *packages = new QWidget( tabWidget );
	tabWidget->addTab(packages, i18n("&Packages"));
	QVBoxLayout *vl = new QVBoxLayout(packages, marginHint(), spacingHint());

	label = new QLabel(i18n("Co&mmon packages:"), packages);
	vl->addWidget(label);
	m_lvPackagesCommon = new QListView(packages);
	vl->addWidget(m_lvPackagesCommon);
	m_lvPackagesCommon->addColumn(i18n("Package"));
	m_lvPackagesCommon->addColumn(i18n("Description"));
	label->setBuddy(m_lvPackagesCommon);
	connect(m_lvPackagesCommon, SIGNAL(clicked(QListViewItem *)), this, SLOT(slotCheckParent(QListViewItem *)));
	connect(m_lvPackagesCommon, SIGNAL(spacePressed(QListViewItem *)), this, SLOT(slotCheckParent(QListViewItem *)));
	connect(m_lvPackagesCommon, SIGNAL(selectionChanged()), this, SLOT(slotEnableButtons()));

	frame = new QWidget(packages);
	vl->addWidget(frame);
	hl = new QHBoxLayout(frame, 0, spacingHint());
	hl->addStretch(1);
	button = new KPushButton(SmallIcon("edit_add"), "&Add Package", frame);
	connect(button, SIGNAL(clicked()), this, SLOT(slotCommonPackageAdd()));
	hl->addWidget(button);
	m_btnPackagesCommonAddOption = new KPushButton(SmallIcon("edit_add"), i18n("Add Op&tion"), frame);
	connect(m_btnPackagesCommonAddOption, SIGNAL(clicked()), this, SLOT(slotCommonPackageAddOption()));
	hl->addWidget(m_btnPackagesCommonAddOption);
	m_btnPackagesCommonEdit = new KPushButton(SmallIcon("edit"), "Ed&it", frame);
	connect(m_btnPackagesCommonEdit, SIGNAL(clicked()), this, SLOT(slotCommonPackageEdit()));
	hl->addWidget(m_btnPackagesCommonEdit);
	m_btnPackagesCommonDelete = new KPushButton(SmallIcon("eraser"), i18n("De&lete"), frame);
	connect(m_btnPackagesCommonDelete, SIGNAL(clicked()), this, SLOT(slotCommonPackageDelete()));
	hl->addWidget(m_btnPackagesCommonDelete);
	button = new KPushButton(SmallIcon("reload"), i18n("&Reset to defaults"), frame);
	connect(button, SIGNAL(clicked()), this, SLOT(slotCommonPackageReset()));
	hl->addWidget(button);

	vl->addSpacing(spacingHint());

	label = new QLabel(i18n("&Exotic packages:"), packages);
	vl->addWidget(label);
	m_lvPackagesExotic = new QListView(packages);
	vl->addWidget(m_lvPackagesExotic);
	m_lvPackagesExotic->addColumn(i18n("Package"));
	m_lvPackagesExotic->addColumn(i18n("Description"));
	label->setBuddy(m_lvPackagesExotic);
	connect(m_lvPackagesExotic, SIGNAL(clicked(QListViewItem *)), this, SLOT(slotCheckParent(QListViewItem *)));
	connect(m_lvPackagesExotic, SIGNAL(spacePressed(QListViewItem *)), this, SLOT(slotCheckParent(QListViewItem *)));
	connect(m_lvPackagesExotic, SIGNAL(selectionChanged()), this, SLOT(slotEnableButtons()));

	frame = new QWidget(packages);
	vl->addWidget(frame);
	hl = new QHBoxLayout(frame, 0, spacingHint());
	hl->addStretch(1);
	button = new KPushButton(SmallIcon("edit_add"), i18n("Add Package"), frame);
	connect(button, SIGNAL(clicked()), this, SLOT(slotExoticPackageAdd()));
	hl->addWidget(button);
	m_btnPackagesExoticAddOption = new KPushButton(SmallIcon("edit_add"), i18n("Add Option"), frame);
	connect(m_btnPackagesExoticAddOption, SIGNAL(clicked()), this, SLOT(slotExoticPackageAddOption()));
	hl->addWidget(m_btnPackagesExoticAddOption);
	m_btnPackagesExoticEdit = new KPushButton(SmallIcon("edit"), i18n("Edit"), frame);
	connect(m_btnPackagesExoticEdit, SIGNAL(clicked()), this, SLOT(slotExoticPackageEdit()));
	hl->addWidget(m_btnPackagesExoticEdit);
	m_btnPackagesExoticDelete = new KPushButton(SmallIcon("eraser"), i18n("Delete"), frame);
	connect(m_btnPackagesExoticDelete, SIGNAL(clicked()), this, SLOT(slotExoticPackageDelete()));
	hl->addWidget(m_btnPackagesExoticDelete);
	button = new KPushButton(SmallIcon("reload"), i18n("Reset to defaults"), frame);
	connect(button, SIGNAL(clicked()), this, SLOT(slotExoticPackageReset()));
	hl->addWidget(button);

	QWidget *personalInfo = new QWidget( tabWidget );
	tabWidget->addTab(personalInfo, i18n("&Document Properties"));
	gl = new QGridLayout(personalInfo, 4, 2, marginHint(), spacingHint());
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
}



/*!
    \fn QuickDocument::init()
 */
void QuickDocument::init()
{
	m_cbTypefaceSize->clear();
	m_cbTypefaceSize->insertItem( "10pt" );
	m_cbTypefaceSize->insertItem( "11pt" );
	m_cbTypefaceSize->insertItem( "12pt" );

	m_leDate->setText( KGlobal::locale()->formatDate(QDate::currentDate(), true) );
}

/*!
    \fn QuickDocument::initClassOption()
 */
void QuickDocument::initDocumentClass()
{
	m_cbDocumentClass->clear();
	m_cbDocumentClass->insertItem( "article" );
	m_cbDocumentClass->insertItem( "book" );
	m_cbDocumentClass->insertItem( "letter" );
	m_cbDocumentClass->insertItem( "report");
	m_cbDocumentClass->insertItem( "scrartcl" );
	m_cbDocumentClass->insertItem( "scrbook" );
	m_cbDocumentClass->insertItem( "scrlettr" );
	m_cbDocumentClass->insertItem( "scrreprt" );
}

/*!
    \fn QuickDocument::initClassOption()
 */
void QuickDocument::initPaperSize()
{
	m_cbPaperSize->clear();
	m_cbPaperSize->insertItem( "a4paper" );
	m_cbPaperSize->insertItem( "a5paper" );
	m_cbPaperSize->insertItem( "b5paper" );
	m_cbPaperSize->insertItem( "executivepaper" );
	m_cbPaperSize->insertItem( "legalpaper" );
	m_cbPaperSize->insertItem( "letterpaper" );
}

/*!
    \fn QuickDocument::initClassOption()
 */
void QuickDocument::initEncoding()
{
	m_cbEncoding->clear();
	m_cbEncoding->insertItem( "ansinew" );
	m_cbEncoding->insertItem( "applemac" );
	m_cbEncoding->insertItem( "ascii" );
	m_cbEncoding->insertItem( "cp1252" );
	m_cbEncoding->insertItem( "cp1250" );
	m_cbEncoding->insertItem( "cp1251" );
	m_cbEncoding->insertItem( "cp437" );
	m_cbEncoding->insertItem( "cp437de" );
	m_cbEncoding->insertItem( "cp850" );
	m_cbEncoding->insertItem( "cp852" );
	m_cbEncoding->insertItem( "cp865" );
	m_cbEncoding->insertItem( "decmulti" );
	m_cbEncoding->insertItem( "koi8-r" );
	m_cbEncoding->insertItem( "latin1" );
	m_cbEncoding->insertItem( "latin2" );
	m_cbEncoding->insertItem( "latin3" );
	m_cbEncoding->insertItem( "latin5" );
	m_cbEncoding->insertItem( "next" );
}

/*!
    \fn QuickDocument::initClassOption()
 */
void QuickDocument::initClassOption()
{
	QCheckListItem *cli;

	m_lvClassOptions->clear();
	cli = new QCheckListItem(m_lvClassOptions, "bibtotoc", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Adds bibliography to table of content (only KOMA classes)"));
	cli = new QCheckListItem(m_lvClassOptions, "draft", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Marks \"overfull hboxes\" on the output with black boxes"));
	cli = new QCheckListItem(m_lvClassOptions, "fleqn", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Aligns formulas on the left side"));
	cli = new QCheckListItem(m_lvClassOptions, "idxtotoc", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Adds index to table of content (only KOMA classes)"));
	cli = new QCheckListItem(m_lvClassOptions, "landscape", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Sets the document's orientation to landscape"));
	cli = new QCheckListItem(m_lvClassOptions, "leqno", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Puts formula numbers on the left side"));
	cli = new QCheckListItem(m_lvClassOptions, "liststotoc", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Adds list of tables and figures to table of content (only KOMA classes)"));
	cli = new QCheckListItem(m_lvClassOptions, "notitlepage", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Puts title and abstract on the same page as the text (default for \"article\")"));
	cli = new QCheckListItem(m_lvClassOptions, "oneside", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Margins are set for single side output (default for most document classes)"));
	cli = new QCheckListItem(m_lvClassOptions, "openany", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Chapters may start on top of every page (default for \"report\")"));
	cli = new QCheckListItem(m_lvClassOptions, "openright", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Chapters may only start on top of right pages (default for \"book\")"));
	cli = new QCheckListItem(m_lvClassOptions, "titlepage", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Puts title and abstract on an extra page (default for most document classes)"));
	cli = new QCheckListItem(m_lvClassOptions, "twocolumn", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Puts the text in two columns"));
	cli = new QCheckListItem(m_lvClassOptions, "twoside", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Left and right pages differ in page margins (default for \"book\")"));
	cli = new QCheckListItem(m_lvClassOptions, "openbib", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Formats the bibliography in open style"));
}

/*!
    \fn KileDialog::QuickDocument::initPackageCommon()
 */
void QuickDocument::initPackageCommon()
{
	QCheckListItem *cli;
	QCheckListItem *clichild;

	m_lvPackagesCommon->clear();
	cli = new QCheckListItem(m_lvPackagesCommon, "amsmath", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Special math environments and commands, provided by the AMS"));
	cli = new QCheckListItem(m_lvPackagesCommon, "amsfonts", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Collection of fonts and symbols for math mode, provided by the AMS"));
	cli = new QCheckListItem(m_lvPackagesCommon, "amssymb", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Collection of fonts and symbols for math mode, provided by the AMS"));
	cli = new QCheckListItem(m_lvPackagesCommon, "amsthm", QCheckListItem::CheckBox);
	cli = new QCheckListItem(m_lvPackagesCommon, "babel", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Adds language specific support"));
	cli->setOpen(true);
	cli->setExpandable(true);
	clichild = new QCheckListItem(cli, "american", QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "dutch", QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "german", QCheckListItem::CheckBox);
	clichild = new QCheckListItem(cli, "french", QCheckListItem::CheckBox);

	cli = new QCheckListItem(m_lvPackagesCommon, "srcltx", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Enable Inverse and Forward search"));

	cli = new QCheckListItem(m_lvPackagesCommon, "fontenc", QCheckListItem::CheckBox);
	cli->setOpen(true);
	cli->setOn(true);
	clichild = new QCheckListItem(cli, "T1", QCheckListItem::CheckBox);
	clichild->setOn(true);
	cli = new QCheckListItem(m_lvPackagesCommon, "graphicx", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Support for including graphics"));
	cli->setOpen(true);
	clichild = new QCheckListItem(cli, "pdftex", QCheckListItem::CheckBox);
	clichild->setText(1, i18n("Specialize on graphic inclusion for pdftex"));
	clichild = new QCheckListItem(cli, "dvips", QCheckListItem::CheckBox);
	clichild->setText(1, i18n("Specialize on graphic inclusion for dvips"));
	cli = new QCheckListItem(m_lvPackagesCommon, "helvetic", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Use Helvetica font as sans font"));
	cli->setOpen(true);
	clichild = new QCheckListItem(cli, "scaled", QCheckListItem::CheckBox);
	cli = new QCheckListItem(m_lvPackagesCommon, "mathpazo", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Use Palatino font as roman font (both text and math mode)"));
	cli = new QCheckListItem(m_lvPackagesCommon, "mathptmx", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Use Times font as roman font (both text and math mode)"));
	cli = new QCheckListItem(m_lvPackagesCommon, "makeidx", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Enable index generation"));
}

/*!
    \fn KileDialog::QuickDocument::initPackageExotic()
 */
void QuickDocument::initPackageExotic()
{
	QCheckListItem *cli;

	m_lvPackagesExotic->clear();
	cli = new QCheckListItem(m_lvPackagesExotic, "multicol", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Enables multicolumn environments"));
	cli = new QCheckListItem(m_lvPackagesExotic, "rotating", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Rotates text"));
	cli = new QCheckListItem(m_lvPackagesExotic, "subfigure", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Enables subfigures inside figures"));
	cli = new QCheckListItem(m_lvPackagesExotic, "wrapfig", QCheckListItem::CheckBox);
	cli->setText(1, i18n("Lets text float around figures"));
	cli = new QCheckListItem(m_lvPackagesExotic, "xspace", QCheckListItem::CheckBox);
}

/*!
    \fn QuickDocument::writeConfig()
 */
void QuickDocument::writeConfig()
{
	QStringList list;

	m_config->setGroup( "Quick" );
	m_config->writeEntry("Class", m_cbDocumentClass->currentText());
	m_config->writeEntry("Typeface", m_cbTypefaceSize->currentText());
	m_config->writeEntry("Papersize", m_cbPaperSize->currentText());

	list.clear();
	for (int i = 0; i<m_cbDocumentClass->count(); i++)
		list+=m_cbDocumentClass->text(i);
	list.sort();
	m_config->writeEntry("Document Classes", list);

	list.clear();
	for (int i = 0; i<m_cbPaperSize->count(); i++)
		list+=m_cbPaperSize->text(i);
	list.sort();
	m_config->writeEntry("Papersizes", list);

	list.clear();
	for (int i = 0; i<m_cbEncoding->count(); i++)
		list+=m_cbEncoding->text(i);
	list.sort();
	m_config->writeEntry("Encodings", list);

	writeListView("Class Options", m_lvClassOptions, false);
	writeListView("Common Packages", m_lvPackagesCommon);
	writeListView("Exotic Packages", m_lvPackagesExotic);

	m_config->setGroup( "User" );
	m_config->writeEntry("Author", m_leAuthor->text());
	QString documentClassOptions;
	for (QListViewItem *cur=m_lvClassOptions->firstChild(); cur; cur=cur->nextSibling()) {
		QCheckListItem *cli=dynamic_cast<QCheckListItem*>(cur);
		if (cli && cli->isOn()) {
			if (!documentClassOptions.isEmpty())
				documentClassOptions+=',';
			documentClassOptions+=cur->text(0);
		}
	}
	m_config->writeEntry("DocumentClassOptions", documentClassOptions);
	m_config->writeEntry("Template Encoding", m_cbEncoding->currentText());
}

/*!
    \fn QuickDocument::writeListView()
 */
void QuickDocument::writeListView(QString key, QListView *listView, bool saveSelected)
{
	QStringList elements, elementsSelected;
	QString keySelected=key+" Selected";

	for (QListViewItem *cur=listView->firstChild(); cur; cur=cur->nextSibling()) {
		elements+=cur->text(0);
		m_config->writeEntry(key+" Description "+cur->text(0), cur->text(1));

		QCheckListItem *cli=dynamic_cast<QCheckListItem*>(cur);
		if (cli && cli->isOn())
			elementsSelected+=cur->text(0);

		for (QListViewItem *curchild=cur->firstChild(); curchild; curchild=curchild->nextSibling()) {
			elements+=cur->text(0)+'!'+curchild->text(0);
			m_config->writeEntry(key+" Description "+cur->text(0)+'!'+curchild->text(0), curchild->text(1));
			QCheckListItem *clichild=dynamic_cast<QCheckListItem*>(curchild);
			if (clichild && clichild->isOn())
				elementsSelected+=cur->text(0)+'!'+curchild->text(0);
		}
	}

	m_config->writeEntry(key, elements);
	if (saveSelected)
		m_config->writeEntry(keySelected, elementsSelected);
}

/*!
    \fn QuickDocument::readConfig()
 */
void QuickDocument::readConfig()
{
	m_config->setGroup( "Quick" );

	QStringList docClasses=m_config->readListEntry("Document Classes");
	if (docClasses.isEmpty())
		initDocumentClass();
	else {
		m_cbDocumentClass->clear();
		m_cbDocumentClass->insertStringList(docClasses);
	}

	QStringList paperSizes=m_config->readListEntry("Papersizes");
	if (paperSizes.isEmpty())
		initPaperSize();
	else {
		m_cbPaperSize->clear();
		m_cbPaperSize->insertStringList(paperSizes);
	}

	QStringList encodings=m_config->readListEntry("Encodings");
	if (encodings.isEmpty())
		initEncoding();
	else {
		m_cbEncoding->clear();
		m_cbEncoding->insertStringList(encodings);
	}

	m_cbDocumentClass->setCurrentText(m_config->readEntry("Class","article"));
	m_cbTypefaceSize->setCurrentText(m_config->readEntry("Typeface","10pt"));
	m_cbPaperSize->setCurrentText(m_config->readEntry("Papersize","a4paper"));

	if (!readListView("Class Options", m_lvClassOptions, false))
		initClassOption();
	if (!readListView("Common Packages", m_lvPackagesCommon))
		initPackageCommon();
	if (!readListView("Exotic Packages", m_lvPackagesExotic))
		initPackageExotic();

	m_config->setGroup( "User" );
	m_leAuthor->setText(m_config->readEntry("Author"));
	QStringList documentClassOptions=QStringList::split(',', m_config->readEntry("DocumentClassOptions"));
	for ( QStringList::Iterator it = documentClassOptions.begin(); it != documentClassOptions.end(); it++ ) {
		QCheckListItem *cli=dynamic_cast<QCheckListItem*>(m_lvClassOptions->findItem(*it, 0));
		if (cli)
			cli->setOn(true);
	}
	m_cbEncoding->setCurrentText(m_config->readEntry("Template Encoding","latin1"));
}

/*!
    \fn QuickDocument::writeListView()
 */
bool QuickDocument::readListView(QString key, QListView *listView, bool readSelected)
{
	QString keySelected=key+" Selected";
	QStringList elements=m_config->readListEntry(key);
	QStringList elementsSelected=m_config->readListEntry(keySelected);

	if (elements.empty())
		return false;

	listView->clear();

	for ( QStringList::Iterator it = elements.begin(); it != elements.end(); it++ ) {
		int pos = (*it).find('!');
		QCheckListItem *cli, *clichild;

		if (pos==-1) {
			cli = new QCheckListItem(listView, *it, QCheckListItem::CheckBox);
			cli->setText(1, m_config->readEntry(key+" Description "+cli->text(0)));
		} else {
			cli=dynamic_cast<QCheckListItem*>(listView->findItem((*it).left(pos), 0));
			cli->setOpen(true);
			if (!cli)
				cli = new QCheckListItem(listView, (*it).left(pos), QCheckListItem::CheckBox);
			clichild = new QCheckListItem(cli, (*it).mid(pos+1), QCheckListItem::CheckBox);
			clichild->setText(1, m_config->readEntry(key+" Description "+cli->text(0)+'!'+clichild->text(0)));
		}
	}

	if (readSelected)
		for (QListViewItem *cur=listView->firstChild(); cur; cur=cur->nextSibling()) {
			QCheckListItem *cli=dynamic_cast<QCheckListItem*>(cur);
			if (cli && elementsSelected.contains(cur->text(0)))
				cli->setOn(true);

			for (QListViewItem *curchild=cur->firstChild(); curchild; curchild=curchild->nextSibling()) {
				QCheckListItem *clichild=dynamic_cast<QCheckListItem*>(curchild);
				if (clichild && elementsSelected.contains(cur->text(0)+'!'+curchild->text(0)))
					clichild->setOn(true);
			}
		}

	return true;
}

/*!
    \fn QuickDocument::printPackage()
 */
void QuickDocument::printPackage(QListView *listView)
{
	for (QListViewItem *cur=listView->firstChild(); cur; cur=cur->nextSibling()) {
		QCheckListItem *cli=dynamic_cast<QCheckListItem*>(cur);
		if (cli && cli->isOn()) {
			QString packageOptions;
			for (QListViewItem *curchild=cur->firstChild(); curchild; curchild=curchild->nextSibling()) {
				QCheckListItem *clichild=dynamic_cast<QCheckListItem*>(curchild);
				if (clichild && clichild->isOn()) {
					if (!packageOptions.isEmpty())
						packageOptions+=",";
					packageOptions+=curchild->text(0);
				}
			}


			m_td.tagBegin += "\\usepackage";
			if (!packageOptions.isEmpty())
				m_td.tagBegin += "[" + packageOptions + "]";
			m_td.tagBegin += "{" + cur->text(0) + "}\n";
		}
	}
}

/*!
    \fn QuickDocument::printTemplate()
 */
void QuickDocument::printTemplate()
{
	m_td.tagBegin = "\\documentclass[";
	if (!m_cbPaperSize->currentText().isEmpty())
		m_td.tagBegin += m_cbPaperSize->currentText()+",";
	m_td.tagBegin += m_cbTypefaceSize->currentText();

	for (QListViewItem *cur=m_lvClassOptions->firstChild(); cur; cur=cur->nextSibling()) {
		QCheckListItem *cli=dynamic_cast<QCheckListItem*>(cur);
		if (cli && cli->isOn())
			m_td.tagBegin += "," + cur->text(0);
	}

	m_td.tagBegin += "]{" + m_cbDocumentClass->currentText() + "}\n";

	if (!m_cbEncoding->currentText().isEmpty())
		m_td.tagBegin += "\\usepackage[" + m_cbEncoding->currentText()+"]{inputenc}\n";

	printPackage(m_lvPackagesCommon);
	printPackage(m_lvPackagesExotic);

	m_td.tagBegin += "\n";
	if (!m_leAuthor->text().isEmpty())
		m_td.tagBegin += "\\author{"+m_leAuthor->text()+"}\n";
	if (!m_leTitle->text().isEmpty())
		m_td.tagBegin += "\\title{"+m_leTitle->text()+"}\n";
	if (!m_leDate->text().isEmpty())
		m_td.tagBegin += "\\date{"+m_leDate->text()+"}\n";
	m_td.tagBegin += "\n";
	m_td.tagBegin += "\\begin{document}\n";

	m_td.tagEnd = "\n\\end{document}";
}

/*!
    \fn QuickDocument::slotOK()
 */
void QuickDocument::slotOk()
{
	printTemplate();
	writeConfig();
	accept();
}

/*!
    \fn KileDialog::QuickDocument::slotCheckParent()
 */
void QuickDocument::slotCheckParent(QListViewItem *listViewItem)
{
	QCheckListItem *cli=dynamic_cast<QCheckListItem*>(listViewItem);
	if (cli && listViewItem->parent() && cli->isOn()) {
		QCheckListItem *cliparent=dynamic_cast<QCheckListItem*>(listViewItem->parent());
		if (cliparent)
			cliparent->setOn(true);
	}
}

/*!
    \fn QuickDocument::slotClassOptionReset()
 */
void QuickDocument::slotClassOptionReset()
{
	if (KMessageBox::questionYesNo(this, i18n("Do you want to reset this option list?"), i18n("Reset Option List"))==KMessageBox::Yes)
	{
		initClassOption();
		slotEnableButtons();
	}
}

/*!
    \fn QuickDocument::slotClassOptionAdd()
 */
void QuickDocument::slotClassOptionAdd()
{
	QString className, description;

	if (inputDialogDouble(i18n("Add Option"), i18n("&Name of option:"), className, i18n("&Description:"), description) && !className.isEmpty()) {
		QCheckListItem *cli = new QCheckListItem(m_lvClassOptions, className, QCheckListItem::CheckBox);
		cli->setText(1, description);
	}
}


/*!
    \fn QuickDocument::slotClassOptionEdit()
 */
void QuickDocument::slotClassOptionEdit()
{
	QListViewItem *cur=m_lvClassOptions->selectedItem();

	if (cur) {
		QString className=cur->text(0);
		QString description=cur->text(1);

		if (inputDialogDouble(i18n("Add Option"), i18n("&Name of option:"), className, i18n("&Description:"), description) && !className.isEmpty()) {
			cur->setText(0, className);
			cur->setText(1, description);
		}
	}
}

/*!
    \fn QuickDocument::slotClassOptionDelete()
 */
void QuickDocument::slotClassOptionDelete()
{
	if (m_lvClassOptions->selectedItem() && (KMessageBox::questionYesNo(this, i18n("Do you want to delete this class option?"), i18n("Delete"))==KMessageBox::Yes))
		m_lvClassOptions->takeItem(m_lvClassOptions->selectedItem());
}


/*!
    \fn QuickDocument::slotCommonPackageReset()
 */
void QuickDocument::slotCommonPackageReset()
{
	if (KMessageBox::questionYesNo(this, i18n("Do you want to reset this package list?"), i18n("Reset Package List"))==KMessageBox::Yes)
	{
		initPackageCommon();
		slotEnableButtons();
	}
}

/*!
    \fn QuickDocument::slotCommonPackageAdd()
 */
void QuickDocument::slotCommonPackageAdd()
{
	packageAdd(m_lvPackagesCommon);
}

/*!
    \fn QuickDocument::slotCommonPackageAddOption()
 */
void QuickDocument::slotCommonPackageAddOption()
{
	packageAddOption(m_lvPackagesCommon->selectedItem());
}

/*!
    \fn QuickDocument::slotCommonPackageEdit()
 */
void QuickDocument::slotCommonPackageEdit()
{
	packageEdit(m_lvPackagesCommon->selectedItem());
}

/*!
    \fn QuickDocument::slotCommonPackageDelete()
 */
void QuickDocument::slotCommonPackageDelete()
{
	packageDelete(m_lvPackagesCommon->selectedItem());
}

/*!
    \fn QuickDocument::slotExoticPackageReset()
 */
void QuickDocument::slotExoticPackageReset()
{
	if (KMessageBox::questionYesNo(this, i18n("Do you want to reset this package list?"), i18n("Reset Package List"))==KMessageBox::Yes)
		initPackageExotic();
}

/*!
    \fn QuickDocument::slotExoticPackageAdd()
 */
void QuickDocument::slotExoticPackageAdd()
{
	packageAdd(m_lvPackagesExotic);
}

/*!
    \fn QuickDocument::slotExoticPackageAddOption()
 */
void QuickDocument::slotExoticPackageAddOption()
{
	packageAddOption(m_lvPackagesExotic->selectedItem());
}

/*!
    \fn QuickDocument::slotExoticPackageEdit()
 */
void QuickDocument::slotExoticPackageEdit()
{
	packageEdit(m_lvPackagesExotic->selectedItem());
}

/*!
    \fn QuickDocument::slotExoticPackageDelete()
 */
void QuickDocument::slotExoticPackageDelete()
{
	packageDelete(m_lvPackagesExotic->selectedItem());
}

/*!
    \fn QuickDocument::slotDocumentClassAdd()
 */
void QuickDocument::slotDocumentClassAdd()
{
	if (!m_cbDocumentClass->currentText().isEmpty())
		m_cbDocumentClass->insertItem(m_cbDocumentClass->currentText());
}

/*!
    \fn QuickDocument::slotDocumentClassDelete()
 */
void QuickDocument::slotDocumentClassDelete()
{
	if (KMessageBox::questionYesNo(this, i18n("Do you want to remove \"%1\" from the document class list?").arg(m_cbDocumentClass->currentText()), i18n("Remove Document Class"))==KMessageBox::Yes)
	{
		int i=m_cbDocumentClass->currentItem();
		m_cbDocumentClass->removeItem(i);
	}
}

/*!
    \fn QuickDocument::slotDocumentClassReset()
 */
void QuickDocument::slotDocumentClassReset()
{
	if (KMessageBox::questionYesNo(this, i18n("Do you want to reset the document class list?"), i18n("Reset Document Class List"))==KMessageBox::Yes)
	{
		initDocumentClass();
		slotEnableButtons();
	}
}

/*!
    \fn QuickDocument::slotDocumentClassAdd()
 */
void QuickDocument::slotPaperSizeAdd()
{
	if (!m_cbPaperSize->currentText().isEmpty())
		m_cbPaperSize->insertItem(m_cbPaperSize->currentText());
}

/*!
    \fn QuickDocument::slotDocumentClassDelete()
 */
void QuickDocument::slotPaperSizeDelete()
{
	if (KMessageBox::questionYesNo(this, i18n("Do you want to remove \"%1\" from the papersize list?").arg(m_cbPaperSize->currentText()), i18n("Remove Papersize"))==KMessageBox::Yes)
	{
		int i=m_cbPaperSize->currentItem();
		m_cbPaperSize->removeItem(i);
	}
}

/*!
    \fn QuickDocument::slotDocumentClassReset()
 */
void QuickDocument::slotPaperSizeReset()
{
	if (KMessageBox::questionYesNo(this, i18n("Do you want to reset the papersize list?"), i18n("Reset Papersize List"))==KMessageBox::Yes)
		initPaperSize();
}

/*!
    \fn QuickDocument::slotDocumentClassAdd()
 */
void QuickDocument::slotEncodingAdd()
{
	if (!m_cbEncoding->currentText().isEmpty())
		m_cbEncoding->insertItem(m_cbEncoding->currentText());
}

/*!
    \fn QuickDocument::slotDocumentClassDelete()
 */
void QuickDocument::slotEncodingDelete()
{
	if (KMessageBox::questionYesNo(this, i18n("Do you want to remove \"%1\" from the encoding list?").arg(m_cbEncoding->currentText()), i18n("Remove Encoding"))==KMessageBox::Yes)
	{
		int i=m_cbEncoding->currentItem();
		m_cbEncoding->removeItem(i);
	}
}

/*!
    \fn QuickDocument::slotDocumentClassReset()
 */
void QuickDocument::slotEncodingReset()
{
	if (KMessageBox::questionYesNo(this, i18n("Do you want to reset the encodings list?"), i18n("Reset Encodings List"))==KMessageBox::Yes)
		initEncoding();
}

void QuickDocument::slotEnableButtons()
{
	bool enable;

	enable = m_lvClassOptions->selectedItem()!=NULL;
	m_btnClassOptionsEdit->setEnabled(enable);
	m_btnClassOptionsDelete->setEnabled(enable);

	enable = (m_lvPackagesCommon->selectedItem()!=NULL) && (m_lvPackagesCommon->selectedItem()->parent()==NULL);
	m_btnPackagesCommonAddOption->setEnabled(enable);
	enable = m_lvPackagesCommon->selectedItem()!=NULL;
	m_btnPackagesCommonEdit->setEnabled(enable);
	m_btnPackagesCommonDelete->setEnabled(enable);

	enable = (m_lvPackagesExotic->selectedItem()!=NULL) && (m_lvPackagesExotic->selectedItem()->parent()==NULL);
	m_btnPackagesExoticAddOption->setEnabled(enable);
	enable = m_lvPackagesExotic->selectedItem()!=NULL;
	m_btnPackagesExoticEdit->setEnabled(enable);
	m_btnPackagesExoticDelete->setEnabled(enable);
}

/*!
    \fn QuickDocument::inputDialogDouble(QString& label1, QString& text1, QString& label2, QString& text2)
 */
bool QuickDocument::inputDialogDouble(QString caption, QString label1, QString& text1, QString label2, QString& text2)
{
	QLabel *label;

	KDialogBase *dialog = new KDialogBase(this, "inputDialogDouble", true, caption, KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true);
	QWidget *page = new QWidget(dialog);
	dialog->setMainWidget(page);
	QVBoxLayout *vl = new QVBoxLayout(page, 0, spacingHint());

	label = new QLabel(label1, page);
	vl->addWidget(label);
	KLineEdit *lineEdit1 = new KLineEdit(text1, page);
	vl->addWidget(lineEdit1);
	label->setBuddy(lineEdit1);

	label = new QLabel(label2, page);
	vl->addWidget(label);
	KLineEdit *lineEdit2 = new KLineEdit(text2, page);
	vl->addWidget(lineEdit2);
	label->setBuddy(lineEdit2);

	lineEdit1->setFocus();
	vl->addStretch(1);
	page->setMinimumWidth(320);

	if (dialog->exec()) {
		text1=lineEdit1->text();
		text2=lineEdit2->text();
		delete dialog;
		return true;
	}

	delete dialog;

	return false;
}

/*!
    \fn QuickDocument::packageDelete(QListViewItem *cur)
 */
void QuickDocument::packageDelete(QListViewItem *cur)
{
	if (cur) {
		QString message=cur->parent()?i18n("Do you want do delete this package option?"):i18n("Do you want to delete this package?");

		if (KMessageBox::questionYesNo(this, message, i18n("Delete"))==KMessageBox::Yes) {
			QListViewItem *childcur = cur->firstChild();
			while (childcur) {
				QListViewItem *nextchildcur=childcur->nextSibling();
				delete childcur;
				childcur = nextchildcur;
			}

			delete cur;
		}
	}
}


/*!
    \fn QuickDocument::packageEdit(QListViewItem *cur)
 */
void QuickDocument::packageEdit(QListViewItem *cur)
{
	if (cur) {
		QString package=cur->text(0);
		QString description=cur->text(1);
		QString labelText=cur->parent()?i18n("Op&tion:"):i18n("&Package:");
		QString caption=cur->parent()?i18n("Edit Option"):i18n("Edit Package");

		if (inputDialogDouble(caption, labelText, package, i18n("&Description:"), description) && !package.isEmpty()) {
			cur->setText(0, package);
			cur->setText(1, description);
		}
	}
}


/*!
    \fn QuickDocument::packageAddOption(QListViewItem *cur)
 */
void QuickDocument::packageAddOption(QListViewItem *cur)
{
	QString option, description;

	if (cur && !cur->parent() && inputDialogDouble(i18n("Add Option"), i18n("Op&tion:"), option, i18n("&Description:"), description) && !option.isEmpty()) {
		QCheckListItem *cli = new QCheckListItem(cur, option, QCheckListItem::CheckBox);
		cli->setText(1, description);
		cur->setOpen(true);
	}
}


/*!
    \fn QuickDocument::packageAdd(QListView *listView)
 */
void QuickDocument::packageAdd(QListView *listView)
{
	QString package, description;

	if (inputDialogDouble(i18n("Add Package"), i18n("&Package:"), package, i18n("&Description:"), description) && !package.isEmpty()) {
		QCheckListItem *cli = new QCheckListItem(listView, package, QCheckListItem::CheckBox);
		cli->setText(1, description);
	}
}

}

#include "quickdocumentdialog.moc"
