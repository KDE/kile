/*******************************************************************************************
  Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
            (C) 2007 by Holger Danielsson (holger.danielsson@versanet.de)
            (C) 2013 by Michel Ludwig (michel.ludwig@kdemail.net)
********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// 2007-02-15 (dani)
//  - cosmetic changes
//  - use of groupboxes to prepare further extensions

// 2007-03-12 (dani)
//  - use KileDocument::Extensions
//  - allowed extensions are always defined as list, f.e.: .tex .ltx .latex

#include "dialogs/projectdialogs.h"

#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRegExp>
#include <QList>
#include <QValidator>
#include <QVBoxLayout>

#include <KApplication>
#include <KComboBox>
#include <KFileDialog>
#include <KGlobal>
#include <KIconLoader>
#include <KLocalizedString>
#include <KMessageBox>
#include <KUrlCompletion>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>

#include "kiledebug.h"
#include "kileproject.h"
#include "kiletoolmanager.h"
#include "documentinfo.h"
#include "kileconfig.h"
#include "kileextensions.h"
#include "templates.h"

KileProjectDlgBase::KileProjectDlgBase(const QString &caption, KileDocument::Extensions *extensions, QWidget *parent, const char * name)
		: QDialog(parent),
		m_extmanager(extensions), m_project(0)
{
	QWidget *page = new QWidget(this);
	QVBoxLayout *mainLayout = new QVBoxLayout;
	setLayout(mainLayout);
	mainLayout->addWidget(page);

	setWindowTitle(caption);
	setModal(true);
	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
	QWidget *mainWidget = new QWidget(this);
	setLayout(mainLayout);
	mainLayout->addWidget(mainWidget);
	QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
	okButton->setDefault(true);
	okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	//PORTING SCRIPT: WARNING mainLayout->addWidget(buttonBox) must be last item in layout. Please move it.
	mainLayout->addWidget(buttonBox);
	okButton->setDefault(true);
	setObjectName(name);

	// properties groupbox
	m_pgroup = new QGroupBox(i18n("Project"), page);
	mainLayout->addWidget(m_pgroup);
	m_pgrid = new QGridLayout(m_pgroup);
//TODO PORT QT5 	m_pgrid->setMargin(QDialog::marginHint());
//TODO PORT QT5 	m_pgrid->setSpacing(QDialog::spacingHint());
	m_pgrid->setAlignment(Qt::AlignTop);
	m_pgroup->setLayout(m_pgrid);

	const QString whatsthisName = i18n("Insert a short descriptive name of your project here.");
	const QString whatsthisExt = i18n("Insert a list (separated by spaces) of file extensions which should be treated also as files of the corresponding type in this project.");
	const QString whatsthisDefGraphicExt = i18n("Default graphic extension to open when none specified by file name.");

	m_title = new QLineEdit(m_pgroup);
	m_title->setWhatsThis(whatsthisName);
	m_plabel = new QLabel(i18n("Project &title:"), m_pgroup);
	m_plabel->setBuddy(m_title);
	m_plabel->setWhatsThis(whatsthisName);

	// extensions groupbox
	m_egroup = new QGroupBox(i18n("Extensions"), page);
	mainLayout->addWidget(m_egroup);
	m_egrid = new QGridLayout();
//TODO PORT QT5 	m_egrid->setMargin(QDialog::marginHint());
//TODO PORT QT5 	m_egrid->setSpacing(QDialog::spacingHint());
	m_egrid->setAlignment(Qt::AlignTop);
	m_egroup->setLayout(m_egrid);

	m_sel_defGraphicExt = new KComboBox(false, m_egroup);
	m_sel_defGraphicExt->addItem("eps", "eps");
	m_sel_defGraphicExt->addItem("pdf", "pdf");
	m_sel_defGraphicExt->addItem("png", "png");
	m_sel_defGraphicExt->addItem("jpg", "jpg");
	m_sel_defGraphicExt->addItem("tif", "tif");
	m_sel_defGraphicExt->addItem(i18n("(use global settings)"),"");
	m_lbDefGraphicExt = new QLabel(i18n("Default Graphics Extension:"), m_egroup);

	m_extensions = new QLineEdit(m_egroup);
	QRegExp reg("[\\. a-zA-Z0-9]+");
	QRegExpValidator *extValidator = new QRegExpValidator(reg, m_egroup);
	m_extensions->setValidator(extValidator);

	m_sel_extensions = new KComboBox(false, m_egroup);
	m_sel_extensions->addItem(i18n("Source Files"));
	m_sel_extensions->addItem(i18n("Package Files"));
	m_sel_extensions->addItem(i18n("Image Files"));
	m_lbPredefinedExtensions = new QLabel(i18n("Predefined:"), m_egroup);
	m_lbStandardExtensions = new QLabel(QString(), m_egroup);


	m_sel_defGraphicExt->setWhatsThis(whatsthisDefGraphicExt);
	m_sel_extensions->setWhatsThis(whatsthisExt);
	m_extensions->setWhatsThis(whatsthisExt);

	fillProjectDefaults();

	connect(m_sel_extensions, SIGNAL(highlighted(int)),
					this, SLOT(slotExtensionsHighlighted(int)));

	connect(m_extensions, SIGNAL(textChanged(const QString&)),
					this, SLOT(slotExtensionsTextChanged(const QString&)));
}

KileProjectDlgBase::~KileProjectDlgBase()
{
}

void KileProjectDlgBase::slotExtensionsHighlighted(int index)
{
	disconnect(m_extensions, SIGNAL(textChanged(const QString&)),
						 this, SLOT(slotExtensionsTextChanged(const QString&)));
	m_extensions->setText(m_val_extensions[index]);
	connect(m_extensions, SIGNAL(textChanged(const QString&)),
					this, SLOT(slotExtensionsTextChanged(const QString&)));

	m_lbStandardExtensions->setText(m_val_standardExtensions[index]);
}

void KileProjectDlgBase::slotExtensionsTextChanged(const QString &text)
{
	m_val_extensions[m_sel_extensions->currentIndex()] = text;
}

bool KileProjectDlgBase::acceptUserExtensions()
{
	QRegExp reg("\\.\\w+");

	for (int i = KileProjectItem::Source; i < KileProjectItem::Other; ++i) {
		m_val_extensions[i-1] = m_val_extensions[i-1].trimmed();
		if (! m_val_extensions[i-1].isEmpty()) {
			// some tiny extension checks
			QStringList::ConstIterator it;
			QStringList list = m_val_extensions[i-1].split(' ');
			for (it = list.constBegin(); it != list.constEnd(); ++it) {
				if (! reg.exactMatch(*it)) {
					KMessageBox::error(this, i18n("Error in extension") + " '" + (*it) + "':\n" + i18n("All user-defined extensions should look like '.xyz'"), i18n("Invalid extension"));
					return false;
				}
			}
		}
	}

	return true;
}

void KileProjectDlgBase::setExtensions(KileProjectItem::Type type, const QString & ext)
{
	if (m_sel_extensions->currentIndex() == type - 1) {
		m_extensions->setText(ext);
	}
	else {
		m_val_extensions[type-1] = ext;
	}
}

void KileProjectDlgBase::setProject(KileProject *project, bool override)
{
	m_project = project;

	if ((!override) || (project == 0))
		return;

	for (int i = KileProjectItem::Source; i < KileProjectItem::Other; ++i)
	{
		m_val_extensions[i - 1] = project->extensions((KileProjectItem::Type) i);
	}

	m_title->setText(m_project->name());
	m_extensions->setText(m_val_extensions[0]);
	m_lbStandardExtensions->setText(m_val_standardExtensions[0]);

	m_sel_defGraphicExt->setCurrentIndex(m_sel_defGraphicExt->findData(project->defaultGraphicExt()));
}

KileProject* KileProjectDlgBase::project()
{
	return m_project;
}

void KileProjectDlgBase::fillProjectDefaults()
{
	m_val_extensions[0].clear();
	m_val_extensions[1].clear();
	m_val_extensions[2].clear();
	//m_val_extensions[3] = OTHER_EXTENSIONS;

	m_val_standardExtensions[0] = m_extmanager->latexDocuments();
	m_val_standardExtensions[1] = m_extmanager->latexPackages();
	m_val_standardExtensions[2] = m_extmanager->images();

	m_extensions->setText(m_val_extensions[0]);
	m_lbStandardExtensions->setText(m_val_standardExtensions[0]);

	m_sel_defGraphicExt->setCurrentIndex(0);
}


/*
 * KileNewProjectDlg
 */
KileNewProjectDlg::KileNewProjectDlg(KileTemplate::Manager *templateManager, KileDocument::Extensions *extensions, QWidget* parent, const char* name)
		: KileProjectDlgBase(i18n("Create New Project"), extensions, parent, name), m_templateManager(templateManager)
{
	QWidget *page = new QWidget(this);
//TODO KF5
// 	mainLayout->addWidget(page);

	// Layout
	QVBoxLayout *vbox = new QVBoxLayout();
	vbox->setMargin(0);
//TODO PORT QT5 	vbox->setSpacing(QDialog::spacingHint());
	page->setLayout(vbox);

	// first groupbox
	m_pgrid->addWidget(m_plabel, 0, 0);
	m_pgrid->addWidget(m_title, 0, 1);

	m_folder = new KUrlRequester(m_pgroup);
	m_folder->setMode(KFile::Directory | KFile::LocalOnly);

	const QString whatsthisPath = i18n("Insert the path to your project here.");

	QLabel *lb1 = new QLabel(i18n("Project &folder:"), m_pgroup);
	lb1->setWhatsThis(whatsthisPath);
	m_folder->setWhatsThis(whatsthisPath);
	lb1->setBuddy(m_folder);

	m_pgrid->addWidget(lb1, 1, 0);
	m_pgrid->addWidget(m_folder, 1, 1);

	// second groupbox
	QGroupBox *group2 = new QGroupBox(i18n("File"), page);
//TODO KF5
// 	mainLayout->addWidget(group2);
	QGridLayout *grid2 = new QGridLayout();
//TODO PORT QT5 	grid2->setMargin(QDialog::marginHint());
//TODO PORT QT5 	grid2->setSpacing(QDialog::spacingHint());
	group2->setLayout(grid2);
	m_cb = new QCheckBox(i18n("Create a new file and add it to this project"), group2);
	m_cb->setChecked(true);
	m_lb  = new QLabel(i18n("File&name (relative to where the project file is):"), group2);
	m_file = new QLineEdit(group2);
	m_lb->setBuddy(m_file);
	m_templateIconView = new TemplateIconView(group2);
	m_templateIconView->setTemplateManager(m_templateManager);
	m_templateManager->scanForTemplates();
	m_templateIconView->fillWithTemplates(KileDocument::LaTeX);
	m_cb->setWhatsThis(i18n("If you want Kile to create a new file and add it to the project, then check this option and select a template from the list that will appear below."));

	grid2->addWidget(m_cb, 0, 0, 1, 2);
	grid2->addWidget(m_lb, 1, 0);
	grid2->addWidget(m_file, 1, 1);
	grid2->addWidget(m_templateIconView, 2, 0, 1, 2);
	grid2->setColumnStretch(1, 1);
	connect(m_cb, SIGNAL(clicked()), this, SLOT(clickedCreateNewFileCb()));

	// third groupbox
	m_egrid->addWidget(m_lbDefGraphicExt, 5, 0);
	m_egrid->addWidget(m_sel_defGraphicExt, 5, 1);
	m_egrid->addWidget(m_sel_extensions, 6, 0);
	m_egrid->addWidget(m_extensions, 6, 1, 1, 3);
	m_egrid->addWidget(m_lbPredefinedExtensions, 7, 0);
	m_egrid->addWidget(m_lbStandardExtensions, 7, 1, 1, 3);

	// add to layout
	vbox->addWidget(m_pgroup);
	vbox->addWidget(group2);
	vbox->addWidget(m_egroup);
	vbox->addStretch();

	fillProjectDefaults();
}

KileNewProjectDlg::~KileNewProjectDlg()
{}

KileProject* KileNewProjectDlg::project()
{
	if (!m_project) {
		m_project = new KileProject(projectTitle(), m_projectFileWithPath, m_extmanager);

		KileProjectItem::Type type;
		for (int i = KileProjectItem::Source; i < KileProjectItem::Other; ++i) {
			type = (KileProjectItem::Type) i;
			m_project->setExtensions(type, extensions(type));
		}

		m_project->setDefaultGraphicExt(m_sel_defGraphicExt->itemData(m_sel_defGraphicExt->currentIndex()).toString());

		m_project->buildProjectTree();
	}

	return m_project;
}

void KileNewProjectDlg::clickedCreateNewFileCb()
{
	if (m_cb->isChecked()) {
		m_file->show();
		m_lb->show();
		m_templateIconView->show();
	}
	else {
		m_file->hide();
		m_lb->hide();
		m_templateIconView->hide();
	}
}

QString KileNewProjectDlg::cleanProjectFile()
{
	return projectTitle().toLower().trimmed().remove(QRegExp("\\s*")) + ".kilepr";
}

//Adapt code and connect okbutton or other to new slot. It doesn't exist in qdialog
//Adapt code and connect okbutton or other to new slot. It doesn't exist in qdialog
void KileNewProjectDlg::slotButtonClicked(int button)
{
// 	if(button == QDialog::Ok) {
// 		if (! acceptUserExtensions()) {
// 			return;
// 		}
// 
// 		if (projectTitle().trimmed().isEmpty()) {
// 			if (KMessageBox::warningYesNo(this, i18n("You have not entered a project name. If you decide to proceed, the project name will be set to \"Untitled\".\n"
// 			                                         "Do you want to create the project nevertheless?"), i18n("No Project Name Given")) == KMessageBox::Yes) {
// 				m_title->setText(i18n("Untitled"));
// 			}
// 			else {
// 				return;
// 			}
// 		}
// 	
// 		if (folder().trimmed().isEmpty()){
// 			KMessageBox::error(this, i18n("Please enter the folder where the project file should be saved to."), i18n("Empty Location"));
// 			return;
// 		}
// 
// 		const QString dirString = folder().trimmed();
// 		if(!QDir::isAbsolutePath(dirString)) {
// 			KMessageBox::error(this, i18n("Please enter an absolute (local) path to the project folder."), i18n("Invalid Location"));
// 			return;
// 		}
// 
// 		QDir dir = QDir(dirString);
// 		KILE_DEBUG_MAIN << "project location is " << dir.absolutePath() << endl;
// 		
// 		if(!dir.exists()){
// 			dir.mkpath(dir.absolutePath());
// 		}
// 		
// 		if(!dir.exists()){
// 			KMessageBox::error(this, i18n("Could not create the project folder, check your permissions."));
// 			return;
// 		}
// 
// 		QFileInfo fi(dir.absolutePath());
// 		if (!fi.isDir() || !fi.isWritable()){
// 			KMessageBox::error(this, i18n("The project folder is not writable, check your permissions."));
// 			return;
// 		}
// 		const QString projectFilePath = dir.filePath(cleanProjectFile());
// 		if(QFileInfo(projectFilePath).exists()){
//                        KMessageBox::error(this, i18n("The project file already exists, please select another name. Delete the existing project file if your intention was to overwrite it."), i18n("Project File Already Exists"));
// 			return;
// 		}
// 
// 		if (createNewFile()) {
// 			if (file().trimmed().isEmpty()){
// 				KMessageBox::error(this, i18n("Please enter a filename for the file that should be added to this project."), i18n("No File Name Given"));
// 				return;
// 			}
// 	
// 			//check for validity of name first, then check for existence (fixed by tbraun)
// 			QUrl fileURL;
// 			fileURL = fileURL.adjusted(QUrl::RemoveFilename);
// 			fileURL.setPath(fileURL.path() + file());
// 			QUrl validURL = KileDocument::Info::makeValidTeXURL(fileURL, this, m_extmanager->isTexFile(fileURL), true);
// 			if(validURL != fileURL) {
// 				m_file->setText(validURL.fileName());
// 			}
// 	
// 			if(QFileInfo(QDir(fi.path()) , file().trimmed()).exists()){
// 				if (KMessageBox::warningYesNo(this, i18n("The file \"%1\" already exists, overwrite it?", file()), i18n("File Already Exists")) == KMessageBox::No) {
// 					return;
// 				}
// 			}
// 		}
// 
// 		m_projectFileWithPath = QUrl::fromLocalFile(projectFilePath);
// 
// 		accept();
// 	}
// 	else{
// //Adapt code and connect okbutton or other to new slot. It doesn't exist in qdialog
// //Adapt code and connect okbutton or other to new slot. It doesn't exist in qdialog
// 		QDialog::slotButtonClicked(button);
// 	}
}

void KileNewProjectDlg::fillProjectDefaults()
{
	m_folder->lineEdit()->setText(QDir::cleanPath(KileConfig::defaultProjectLocation()));
	m_cb->setChecked(true);
	KileProjectDlgBase::fillProjectDefaults();
}

TemplateItem* KileNewProjectDlg::getSelection() const
{
	return static_cast<TemplateItem*>(m_templateIconView->currentItem());
}

/*
 * KileProjectOptionsDlg
 */
KileProjectOptionsDlg::KileProjectOptionsDlg(KileProject *project, KileDocument::Extensions *extensions, QWidget *parent, const char * name) :
		KileProjectDlgBase(i18n("Project Options"), extensions, parent, name),
		m_toolDefaultString(i18n("(use global setting)"))
{
	QWidget *page = new QWidget(this);
//TODO KF5
// 	mainLayout->addWidget(page);

	// Layout
	QVBoxLayout *vbox = new QVBoxLayout();
	vbox->setMargin(0);
//TODO PORT QT5 	vbox->setSpacing(QDialog::spacingHint());
	page->setLayout(vbox);

	m_pgrid->addWidget(m_plabel, 0, 0);
	m_pgrid->addWidget(m_title, 0, 1);

	// second groupbox
	m_egrid->addWidget(m_lbDefGraphicExt, 5, 0);
	m_egrid->addWidget(m_sel_defGraphicExt, 5, 1);
	m_egrid->addWidget(m_sel_extensions, 6, 0);
	m_egrid->addWidget(m_extensions, 6, 1, 1, 3);
	m_egrid->addWidget(m_lbPredefinedExtensions, 7, 0);
	m_egrid->addWidget(m_lbStandardExtensions, 7, 1, 1, 3);

	// third groupbox
	QGroupBox *group3 = new QGroupBox(i18n("Properties"), page);
//TODO KF5
// 	mainLayout->addWidget(group3);
	QGridLayout *grid3 = new QGridLayout();
//TODO PORT QT5 	grid3->setMargin(QDialog::marginHint());
//TODO PORT QT5 	grid3->setSpacing(QDialog::spacingHint());
	grid3->setAlignment(Qt::AlignTop);
	group3->setLayout(grid3);

	const QString whatsthisMaster = i18n("Select the default master document. Leave empty for auto detection.");

	m_master = new KComboBox(false, group3);
	m_master->setObjectName("master");
	//m_master->setDisabled(true);
	QLabel *lb1 = new QLabel(i18n("&Master document:"), group3);
	lb1->setBuddy(m_master);
	lb1->setMinimumWidth(m_sel_extensions->sizeHint().width());
	m_master->setWhatsThis(whatsthisMaster);
	lb1->setWhatsThis(whatsthisMaster);

	m_master->addItem(i18n("(auto-detect)"));
	QList<KileProjectItem*> rootItemList = project->rootItems();
	int index = 0;
	for (QList<KileProjectItem*>::iterator it = rootItemList.begin(); it != rootItemList.end(); ++it) {
		if ((*it)->type() == KileProjectItem::Source) {
			m_master->addItem((*it)->url().fileName());
			++index;
			if ((*it)->url().path() == project->masterDocument()) {
				m_master->setCurrentIndex(index);
			}
		}
	}

	if (project->masterDocument().isEmpty()) {
		m_master->setCurrentIndex(0);
	}

	QLabel *lb2 = new QLabel(i18n("&QuickBuild configuration:"), group3);
	m_cbQuick = new KComboBox(group3);
	lb2->setBuddy(m_cbQuick);
	m_cbQuick->addItem(m_toolDefaultString);
	m_cbQuick->addItems(KileTool::configNames("QuickBuild", KSharedConfig::openConfig().data()));
	QString itemToSelect = project->quickBuildConfig().length() > 0 ? project->quickBuildConfig() : m_toolDefaultString;
	int selectIndex = m_cbQuick->findText(itemToSelect);
	if(selectIndex >= 0) {
		m_cbQuick->setCurrentIndex(selectIndex);
	}
	else {
		m_cbQuick->addItem(itemToSelect);
	}


	//don't put this after the call to toggleMakeIndex
	setProject(project, true);

	m_ckMakeIndex = new QCheckBox(i18n("&MakeIndex options"), group3);
	connect(m_ckMakeIndex, SIGNAL(toggled(bool)), this, SLOT(toggleMakeIndex(bool)));
	m_leMakeIndex = new QLineEdit(group3);
	m_ckMakeIndex->setChecked(project->useMakeIndexOptions());
	toggleMakeIndex(m_ckMakeIndex->isChecked());

	grid3->addWidget(lb1, 0, 0);
	grid3->addWidget(m_master, 0, 1);
	grid3->addWidget(lb2, 1, 0);
	grid3->addWidget(m_cbQuick, 1, 1);
	grid3->addWidget(m_ckMakeIndex, 2, 0);
	grid3->addWidget(m_leMakeIndex, 2, 1, 1, 2);
	grid3->setColumnStretch(2, 1);

	// add to layout
	vbox->addWidget(m_pgroup);
	vbox->addWidget(m_egroup);
	vbox->addWidget(group3);
	vbox->addStretch();
}

KileProjectOptionsDlg::~KileProjectOptionsDlg()
{
}

void KileProjectOptionsDlg::toggleMakeIndex(bool on)
{
	KILE_DEBUG_MAIN << "TOGGLED!" << endl;
	m_leMakeIndex->setEnabled(on);
	m_project->setUseMakeIndexOptions(on);
	m_project->writeUseMakeIndexOptions();
	m_project->readMakeIndexOptions();
	m_leMakeIndex->setText(m_project->makeIndexOptions());
}

//Adapt code and connect okbutton or other to new slot. It doesn't exist in qdialog
//Adapt code and connect okbutton or other to new slot. It doesn't exist in qdialog
void KileProjectOptionsDlg::slotButtonClicked(int button)
{
// 	if( button == QDialog::Ok ){
// 		if(!acceptUserExtensions()) {
// 			return;
// 		}
// 		
// 		this->m_project->setName(m_title->text());
// 	
// 		QList<KileProjectItem*> rootItemList = m_project->rootItems();
// 		for (QList<KileProjectItem*>::iterator it = rootItemList.begin(); it != rootItemList.end(); ++it) {
// 			if ((*it)->url().fileName() == m_master->currentText()) {
// 				m_project->setMasterDocument((*it)->url().toLocalFile());
// 			}
// 		}
// 		if (m_master->currentIndex() == 0) {
// 			m_project->setMasterDocument(QString());
// 		}
// 	
// 		m_val_extensions[m_sel_extensions->currentIndex()] = m_extensions->text();
// 	
// 		for (int i = KileProjectItem::Source; i < KileProjectItem::Other; ++i) {
// 			m_project->setExtensions((KileProjectItem::Type) i, m_val_extensions[i-1]);
// 		}
// 	
// 		if (m_cbQuick->currentText() == m_toolDefaultString) {
// 			m_project->setQuickBuildConfig("");
// 		}
// 		else {
// 			m_project->setQuickBuildConfig(m_cbQuick->currentText());
// 		}
// 		
// 		m_project->setUseMakeIndexOptions(m_ckMakeIndex->isChecked());
// 		if (m_project->useMakeIndexOptions()) {
// 			m_project->setMakeIndexOptions(m_leMakeIndex->text());
// 		}
// 	
// 		m_project->setDefaultGraphicExt(m_sel_defGraphicExt->itemData(m_sel_defGraphicExt->currentIndex()).toString());
// 
// 		m_project->save();
// 	
// 		accept();
// 	}
// 	else{
// //Adapt code and connect okbutton or other to new slot. It doesn't exist in qdialog
// //Adapt code and connect okbutton or other to new slot. It doesn't exist in qdialog
// 		QDialog::slotButtonClicked(button);
// 	}
}


#include "projectdialogs.moc"
