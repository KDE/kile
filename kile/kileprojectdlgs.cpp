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
#include "kileprojectdlgs.h"

KileNewProjectDlg::KileNewProjectDlg(QWidget* parent,  const char* name)
        : KDialogBase( KDialogBase::Plain, i18n("Create a new project"), Ok|Cancel,Ok, parent, name, true, true )
{
	QGridLayout *layout = new QGridLayout(plainPage(),4,4, 10);

	m_name = new KLineEdit(plainPage(), "le_projectname");
	QLabel *lb = new QLabel(i18n("Project &name"), plainPage());
	lb->setBuddy(m_name);
	QWhatsThis::add(lb, i18n("Insert a short descriptive name of your project here."));
	QWhatsThis::add(m_name, i18n("Insert a short descriptive name of your project here."));
	layout->addWidget(lb, 0,0);
	layout->addWidget(m_name, 0,1);

	m_location = new KLineEdit(plainPage(), "le_projectlocation");
	lb = new QLabel(i18n("Project &file"), plainPage());
	QWhatsThis::add(lb, i18n("Insert the path to your project file here. If this file does not yet exists, it will be created. The filename should have the extension: .kilepr. You can also use the browse button to insert a filename."));
	QWhatsThis::add(m_location, i18n("Insert the path to your project file here. If this file does not yet exists, it will be created. The filename should have the extension: .kilepr. You can also use the browse button to insert a filename."));
	lb->setBuddy(m_location);
	KPushButton *pb = new KPushButton(i18n("Browse..."), plainPage());
	connect(pb, SIGNAL(clicked()), this, SLOT(browseLocation()));
	layout->addWidget(lb, 1,0);
	layout->addMultiCellWidget(m_location, 1,1, 1,2);
	layout->addWidget(pb,1,3);

	m_cb = new QCheckBox(i18n("Create a new file and add it to this project."),plainPage());
	m_cb->setChecked(true);
	lb  = new QLabel(i18n("File&name (relative to the project file)"), plainPage());
	m_file = new KLineEdit(plainPage());
	lb->setBuddy(m_file);
	m_nfw = new NewFileWidget(plainPage());
	QWhatsThis::add(m_cb, i18n("If you want Kile to create a new file and add it to the project, then check this option and select a template from the list that will appear below."));
	layout->addMultiCellWidget(m_cb, 2,2,0,3);
	layout->addWidget(lb, 3,0);
	layout->addMultiCellWidget(m_file, 3,3, 1,3);
	layout->addMultiCellWidget(m_nfw, 4,4,0,3);
	connect(m_cb, SIGNAL(clicked()), this, SLOT(clickedCreateNewFileCb()));
}

KileNewProjectDlg::~KileNewProjectDlg()
{}

void KileNewProjectDlg::clickedCreateNewFileCb()
{
	if (m_cb->isChecked())
	{
		m_file->show();
		m_nfw->show();
	}
	else
	{
		m_file->hide();
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
		if (KMessageBox::warningYesNo(this, i18n("You did not enter a project name, if you continue the project name will be set to untitled."), i18n("No name")) == KMessageBox::Yes)
			m_name->setText("untitled");
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

#include "kileprojectdlgs.moc"
