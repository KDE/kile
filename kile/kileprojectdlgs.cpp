/***************************************************************************
                          kileprojectdlgs.cpp -  description
                             -------------------
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

#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>
#include <qfileinfo.h>

#include <klocale.h>
#include <klineedit.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <kfiledialog.h>

#include "newfilewizard.h"
#include "kileproject.h"
#include "kileprojectdlgs.h"

QString whatsthisName = i18n("Insert a short descriptive name of your project here.");
QString whatsthisPath = i18n("Insert the path to your project file here. If this file does not yet exists, it will be created. The filename should have the extension: .kilepr. You can also use the browse button to insert a filename.");
QString whatsthisArchive = i18n("Enter the command to create an archive of all the project files here. %S will be replaced with the project name, %F with a list of all the project files (items are separated by a space). This command will be executed from the base directory of the project (i.e. the directory where the .kilepr file resides).");
QString whatsthisExt = i18n("Insert a list (separated with spaces) of the extensions of the files in your project that are not TeX source files. These files will be put in a separate place in the Project View. You can also use a regular expression to detect which files are non-source files.");

/*
 * KileNewProjectDlg
 */
KileNewProjectDlg::KileNewProjectDlg(QWidget* parent,  const char* name)
        : KDialogBase( KDialogBase::Plain, i18n("Create a new project"), Ok|Cancel,Ok, parent, name, true, true )
{
	QGridLayout *layout = new QGridLayout(plainPage(),4,8, 10);
	layout->setColStretch(2,1);
	layout->setColStretch(3,1);

	m_name = new KLineEdit(plainPage(), "le_projectname");
	QLabel *lb = new QLabel(i18n("Project &title"), plainPage());
	lb->setBuddy(m_name);
	QWhatsThis::add(lb, whatsthisName);
	QWhatsThis::add(m_name, whatsthisName);
	layout->addWidget(lb, 0,0);
	layout->addWidget(m_name, 0,1);

	m_location = new KLineEdit(plainPage(), "le_projectlocation");
	lb = new QLabel(i18n("Project &file"), plainPage());
	QWhatsThis::add(lb, whatsthisPath);
	QWhatsThis::add(m_location, whatsthisPath);
	lb->setBuddy(m_location);
	KPushButton *pb = new KPushButton(i18n("Browse..."), plainPage());
	connect(pb, SIGNAL(clicked()), this, SLOT(browseLocation()));
	layout->addWidget(lb, 1,0);
	layout->addMultiCellWidget(m_location, 1,1, 1,2);
	layout->addWidget(pb,1,3);

	m_cb = new QCheckBox(i18n("Create a new file and add it to this project."),plainPage());
	m_cb->setChecked(true);
	m_lb  = new QLabel(i18n("File&name (relative to where the project file is)"), plainPage());
	m_file = new KLineEdit(plainPage());
	m_lb->setBuddy(m_file);
	m_nfw = new NewFileWidget(plainPage());
	QWhatsThis::add(m_cb, i18n("If you want Kile to create a new file and add it to the project, then check this option and select a template from the list that will appear below."));
	layout->addMultiCellWidget(m_cb, 2,2,0,3);
	layout->addMultiCellWidget(m_lb, 3,3,0,1);
	layout->addMultiCellWidget(m_file, 3,3, 2,3);
	layout->addMultiCellWidget(m_nfw, 4,4,0,3);
	connect(m_cb, SIGNAL(clicked()), this, SLOT(clickedCreateNewFileCb()));

	m_archive = new KLineEdit(plainPage(), "le_archive");
	m_archive->setText("tar zcvf '%S.tar.gz' %F");
	lb = new QLabel(i18n("&Archive command"), plainPage());
	lb->setBuddy(m_archive);
	QWhatsThis::add(m_archive, whatsthisArchive);
	QWhatsThis::add(lb, whatsthisArchive);
	layout->addWidget(lb, 5,0);
	layout->addMultiCellWidget(m_archive, 5,5,1,3);

	m_extensions = new KLineEdit(plainPage(), "le_ext");
	m_extensions->setText(DEFAULT_EXTENSIONS);
	lb = new QLabel(i18n("&Extensions for non-source files"), plainPage());
	lb->setBuddy(m_extensions);
	m_isregexp = new QCheckBox(i18n("use extension list as a regular expression"), plainPage());
	m_isregexp->setChecked(false);
	QWhatsThis::add(m_extensions, whatsthisExt);
	QWhatsThis::add(m_isregexp, whatsthisExt);
	QWhatsThis::add(lb,whatsthisExt);
	layout->addWidget(lb, 6,0);
	layout->addMultiCellWidget(m_extensions, 6,6,1,3);
	layout->addMultiCellWidget(m_isregexp, 7,7,1,3);
}

KileNewProjectDlg::~KileNewProjectDlg()
{}

void KileNewProjectDlg::clickedCreateNewFileCb()
{
	if (m_cb->isChecked())
	{
		m_file->show();
		m_lb->show();
		m_nfw->show();
	}
	else
	{
		m_file->hide();
		m_lb->hide();
		m_nfw->hide();
	}
}

void KileNewProjectDlg::browseLocation()
{
	QString filename = KFileDialog::getOpenFileName();

	if (!filename.isNull())
		m_location->setText(filename);
}

void KileNewProjectDlg::slotOk()
{
	if ( name().stripWhiteSpace() == "")
		if (KMessageBox::warningYesNo(this, i18n("You did not enter a project name, if you continue the project name will be set to: Untitled."), i18n("No name")) == KMessageBox::Yes)
			m_name->setText(i18n("Untitled"));
		else
			return;

	if ( location().stripWhiteSpace() == "" )
	{
		KMessageBox::error(this, i18n("Please enter the location where the project file should be save to. Also make sure it ends with .kilepr ."), i18n("Empty location"));
		return;
	}

	if ( location().stripWhiteSpace().right(7) != ".kilepr")
	{
		KMessageBox::error(this, i18n("The extension of the project filename is not .kilepr , please correct the extension"), i18n("Wrong filename extension."));
		return;
	}

	if (createNewFile() && file().stripWhiteSpace() == "")
	{
		KMessageBox::error(this, i18n("Please enter a filename for the file that should be added to this project"), i18n("No file name give"));
		return;
	}

	if (QFileInfo(location()).exists())
	{
		KMessageBox::error(this, i18n("The project file already exists, please select another name. Delete the existing project file if your intention was to overwrite it."),i18n("Project file already exists."));
		return;
	}

	accept();
}

/*
 * KileProjectOptionsDlg
 */
KileProjectOptionsDlg::KileProjectOptionsDlg(KileProject *project, QWidget *parent, const char * name) :
 	KDialogBase(KDialogBase::Plain, i18n("Project Options"), Ok|Cancel,Ok, parent, name, true, true ),
	m_project(project)
{
	QGridLayout *layout = new QGridLayout(plainPage(),4,4, 10);

	m_name = new KLineEdit(plainPage(), "le_projectname");
	m_name->setText(m_project->name());
	QLabel *lb = new QLabel(i18n("Project &title"), plainPage());
	lb->setBuddy(m_name);
	QWhatsThis::add(lb, whatsthisArchive);
	QWhatsThis::add(m_name, whatsthisArchive);
	layout->addWidget(lb, 0,0);
	layout->addMultiCellWidget(m_name, 0,0,1,3);

	m_archive = new KLineEdit(plainPage(), "le_archive");
	m_archive->setText(project->archiveCommand());
	lb = new QLabel(i18n("&Archive command"), plainPage());
	lb->setBuddy(m_archive);
	QWhatsThis::add(m_archive, whatsthisArchive);
	QWhatsThis::add(lb,whatsthisArchive);
	layout->addWidget(lb, 1,0);
	layout->addMultiCellWidget(m_archive, 1,1,1,3);

	m_extensions = new KLineEdit(plainPage(), "le_ext");
	m_extensions->setText(project->extensions());
	lb = new QLabel(i18n("&Extensions for non-source files"), plainPage());
	lb->setBuddy(m_extensions);
	m_isregexp = new QCheckBox(i18n("use extension list as a regular expression"), plainPage());
	m_isregexp->setChecked(project->extIsRegExp());
	QWhatsThis::add(m_extensions, whatsthisExt);
	QWhatsThis::add(m_isregexp, whatsthisExt);
	QWhatsThis::add(lb,whatsthisExt);
	layout->addWidget(lb, 2,0);
	layout->addMultiCellWidget(m_extensions, 2,2,1,3);
	layout->addMultiCellWidget(m_isregexp, 3,3,1,3);
}

KileProjectOptionsDlg::~KileProjectOptionsDlg()
{
}

void KileProjectOptionsDlg::slotOk()
{
	m_project->setName(m_name->text());
	m_project->setArchiveCommand(m_archive->text());
	m_project->setExtensions(m_extensions->text());
	m_project->setExtIsRegExp(m_isregexp->isChecked());
	accept();
}

#include "kileprojectdlgs.moc"
