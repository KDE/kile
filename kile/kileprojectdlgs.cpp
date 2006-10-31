/***************************************************************************
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
#include <qptrlist.h>

#include <klocale.h>
#include <klineedit.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <kurlcompletion.h>
#include <kfiledialog.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kapplication.h>

#include "newfilewizard.h"
#include "kileproject.h"
#include "kileprojectdlgs.h"
#include "kiletoolmanager.h"
#include "kiledocumentinfo.h"
#include "kileconfig.h"

const QString whatsthisName = i18n("Insert a short descriptive name of your project here.");
const QString whatsthisPath = i18n("Insert the path to your project file here. If this file does not yet exists, it will be created. The filename should have the extension: .kilepr. You can also use the browse button to insert a filename.");
const QString whatsthisExt = i18n("Insert a list (separated with spaces) of the extensions of the files in your project that are not TeX source files. These files will be put in a separate place in the Project View. You can also use a regular expression to detect which files are non-source files.");
const QString whatsthisMaster = i18n("Select the default master document. Leave empty for auto detection.");

const QString tool_default = i18n("(use global setting)");

KileProjectDlgBase::KileProjectDlgBase(const QString &caption, QWidget *parent, const char * name)
	: KDialogBase( KDialogBase::Plain, caption, (Ok | Cancel), Ok, parent, name, true, true),
	m_project(0)
{
	m_title = new KLineEdit(plainPage(), "le_projectname");
	QWhatsThis::add(m_title, whatsthisName);

	m_extensions = new KLineEdit(plainPage(), "le_ext");
	m_sel_extensions = new KComboBox(false, plainPage(), "le_sel_ext");
	m_sel_extensions->insertItem(i18n("Extensions for Source Files"));
	m_sel_extensions->insertItem(i18n("Extensions for Package Files"));
	m_sel_extensions->insertItem(i18n("Extensions for Image Files"));
	m_isregexp = new QCheckBox(i18n("Use extension list as a regular expression"), plainPage());
	QWhatsThis::add(m_sel_extensions, whatsthisExt);
	QWhatsThis::add(m_extensions, whatsthisExt);
	QWhatsThis::add(m_isregexp, whatsthisExt);

	connect(m_sel_extensions, SIGNAL(highlighted(int)),
		this, SLOT(slotExtensionsHighlighted(int)));

	connect(m_extensions, SIGNAL(textChanged(const QString&)),
		this, SLOT(slotExtensionsTextChanged(const QString&)));

	connect(m_isregexp, SIGNAL(toggled(bool)),
		this, SLOT(slotRegExpToggled(bool)));
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

	disconnect(m_isregexp, SIGNAL(toggled(bool)),
		this, SLOT(slotRegExpToggled(bool)));
	m_isregexp->setChecked(m_val_isregexp[index]);
	connect(m_isregexp, SIGNAL(toggled(bool)),
		this, SLOT(slotRegExpToggled(bool)));
}

void KileProjectDlgBase::slotExtensionsTextChanged(const QString &text)
{
	m_val_extensions[m_sel_extensions->currentItem()] = text;
}

void KileProjectDlgBase::slotRegExpToggled(bool on)
{
	m_val_isregexp[m_sel_extensions->currentItem()] = on;
}

void KileProjectDlgBase::setExtensions(KileProjectItem::Type type, const QString & ext)
{
	if (m_sel_extensions->currentItem() == type-1)
		m_extensions->setText(ext);
	else
		m_val_extensions[type-1] = ext;
}

void KileProjectDlgBase::setExtIsRegExp(KileProjectItem::Type type, bool is)
{
	if (m_sel_extensions->currentItem() == type-1)
		m_isregexp->setChecked(is);
	else
		m_val_isregexp[type-1] = is;
}

void KileProjectDlgBase::setProject(KileProject *project, bool override)
{
	m_project = project;

	if ((!override) || (project == 0))
		return;

	for (int i = KileProjectItem::Source; i < KileProjectItem::Other; ++i) {
		m_val_extensions[i - 1] =
			project->extensions((KileProjectItem::Type) i);
		m_val_isregexp[i - 1] =
			project->extIsRegExp((KileProjectItem::Type) i);
	}

	m_title->setText(m_project->name());
	m_extensions->setText(m_val_extensions[0]);
	m_isregexp->setChecked(m_val_isregexp[0]);
}

KileProject* KileProjectDlgBase::project()
{
	return m_project;
}

void KileProjectDlgBase::fillProjectDefaults()
{
	m_val_isregexp[0] = false;
	m_val_isregexp[1] = false;
	m_val_isregexp[2] = false;
	//m_val_isregexp[3] = false;

	m_val_extensions[0] = SOURCE_EXTENSIONS;
	m_val_extensions[1] = PACKAGE_EXTENSIONS;
	m_val_extensions[2] = IMAGE_EXTENSIONS;
	//m_val_extensions[3] = OTHER_EXTENSIONS;

	m_extensions->setText(m_val_extensions[0]);
	m_isregexp->setChecked(m_val_isregexp[0]);
}


/*
 * KileNewProjectDlg
 */
KileNewProjectDlg::KileNewProjectDlg(QWidget* parent, const char* name)
        : KileProjectDlgBase( i18n("Create New Project"), parent, name),
		m_filename(QString::null)
{
	QGridLayout *layout = new QGridLayout(plainPage(), 4,8, 10);
	layout->setColStretch(2,1);
	layout->setColStretch(3,1);

	QLabel *lb = new QLabel(i18n("Project &title:"), plainPage());
	lb->setBuddy(m_title);
	QWhatsThis::add(lb, whatsthisName);
	layout->addWidget(lb, 0,0);
	layout->addWidget(m_title, 0,1);

	connect(m_title, SIGNAL(textChanged(const QString&)), this, SLOT(makeProjectPath()));

	m_location = new KLineEdit(plainPage(), "le_projectlocation");
	m_location->setMinimumWidth(200);

	lb = new QLabel(i18n("Project &file:"), plainPage());
	QWhatsThis::add(lb, whatsthisPath);
	QWhatsThis::add(m_location, whatsthisPath);
	lb->setBuddy(m_location);
	KPushButton *pb = new KPushButton(i18n("Select Folder..."), plainPage());
	connect(pb, SIGNAL(clicked()), this, SLOT(browseLocation()));
	layout->addWidget(lb, 1,0);
	layout->addMultiCellWidget(m_location, 1,1, 1,2);
	layout->addWidget(pb, 1,3);

	m_cb = new QCheckBox(i18n("Create a new file and add it to this project"),plainPage());
	m_cb->setChecked(true);
	m_lb  = new QLabel(i18n("File&name (relative to where the project file is):"), plainPage());
	m_file = new KLineEdit(plainPage());
	m_lb->setBuddy(m_file);
	m_nfw = new NewFileWidget(plainPage());
	QWhatsThis::add(m_cb, i18n("If you want Kile to create a new file and add it to the project, then check this option and select a template from the list that will appear below."));
	layout->addMultiCellWidget(m_cb, 2,2, 0,3);
	layout->addMultiCellWidget(m_lb, 3,3, 0,1);
	layout->addMultiCellWidget(m_file, 3,3, 2,3);
	layout->addMultiCellWidget(m_nfw, 4,4, 0,3);
	connect(m_cb, SIGNAL(clicked()), this, SLOT(clickedCreateNewFileCb()));

	layout->addWidget(m_sel_extensions, 6,0);
	layout->addMultiCellWidget(m_extensions, 6,6, 1,3);
	layout->addMultiCellWidget(m_isregexp, 7,7, 1,3);

	fillProjectDefaults();
}

KileNewProjectDlg::~KileNewProjectDlg()
{}

KileProject* KileNewProjectDlg::project()
{
	if (!m_project) {
		m_project = new KileProject(projectTitle(), KURL(location()));

		KileProjectItem::Type type;
		for (int i = KileProjectItem::Source; i < KileProjectItem::Other; ++i) {
			type = (KileProjectItem::Type) i;
			m_project->setExtIsRegExp(type, extIsRegExp(type));
			m_project->setExtensions(type, extensions(type));
		}

		m_project->buildProjectTree();
	}

	return m_project;
}

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

QString KileNewProjectDlg::bare()
{
	return projectTitle().lower().stripWhiteSpace().replace(QRegExp("\\s*"),"")+".kilepr";
}

void KileNewProjectDlg::browseLocation()
{
	QString dir = m_dir;
	if (!QFileInfo(m_dir).exists())
		dir = QString::null;

	dir = KFileDialog::getExistingDirectory(dir, this);

	if (! dir.isNull())
	{
		m_dir = dir;
		if (m_dir.right(1) != "/") m_dir = m_dir + '/';
		m_location->setText(m_dir+bare());
	}
}

void KileNewProjectDlg::makeProjectPath()
{
	m_filename=bare();
	kdDebug() << "BEFORE " << QFileInfo(location()).absFilePath() << " " << QFileInfo(location()).dirPath() << endl;
	m_dir = QFileInfo(location()).dirPath();
	if (m_dir.right(1) != "/") m_dir = m_dir + '/';

	kdDebug() << "LOCATION " << location() << " AND " << m_dir << endl;
	m_location->setText(m_dir+m_filename);
}

void KileNewProjectDlg::slotOk()
{
	//replace ~ and environment variables in the paths
	KURLCompletion uc;
	uc.setReplaceEnv(true);
	uc.setReplaceHome(true);
	m_location->setText(uc.replacedPath(location()));
	m_file->setText(uc.replacedPath(file()));

	if ( projectTitle().stripWhiteSpace().isEmpty())
	{
		if (KMessageBox::warningYesNo(this, i18n("You did not enter a project name, if you continue the project name will be set to: Untitled."), i18n("No Name")) == KMessageBox::Yes)
			m_title->setText(i18n("Untitled"));
		else
			return;
	}

	if ( location().stripWhiteSpace().isEmpty() )
	{
		KMessageBox::error(this, i18n("Please enter the location where the project file should be save to. Also make sure it ends with .kilepr ."), i18n("Empty Location"));
		return;
	}

	QFileInfo fi(location().stripWhiteSpace());
	QFileInfo dr(fi.dirPath());
	QDir dir = dr.dir();

	if ( location().stripWhiteSpace().right(7) != ".kilepr")
	{
		KMessageBox::error(this, i18n("The extension of the project filename is not .kilepr , please correct the extension"), i18n("Wrong Filename Extension"));
		return;
	}
	else
	{

		if (dir.isRelative())
		{
			KMessageBox::error(this, i18n("The path for the project file is not an absolute path, absolute paths always begin with an /"),i18n("Relative Path"));
			return;
		}

		kdDebug() << "==KileNewProjectDlg::slotOk()==============" << endl;
		kdDebug() << "\t" << location() << " " << fi.dirPath() << endl;
		if (! dr.exists())
		{
			bool suc = true;
			QStringList dirs = QStringList::split("/", fi.dirPath());
			QString path;

			for (uint i=0; i < dirs.count(); ++i)
			{
				path += '/' + dirs[i];
				dir.setPath(path);
				kdDebug() << "\tchecking : " << dir.absPath() << endl;
				if ( ! dir.exists() )
				{
					dir.mkdir(dir.absPath());
					suc = dir.exists();
					kdDebug() << "\t\tcreated : " << dir.absPath() << " suc = " << suc << endl;
				}

				if (!suc)
				{
					KMessageBox::error(this, i18n("Could not create the project folder, check your permissions."));
					return;
				}
			}
		}

		if (! dr.isWritable())
		{
			KMessageBox::error(this, i18n("The project folder is not writable, check your permissions."));
			return;
		}
	}

	if ( createNewFile() )
	{
		if ( file().stripWhiteSpace().isEmpty())
		{
			KMessageBox::error(this, i18n("Please enter a filename for the file that should be added to this project."),
				i18n("No File Name Given"));
			return;
		}

		//check for validity of name first, then check for existence (fixed by tbraun)
		KURL fileURL; fileURL.setFileName(file());
		KURL validURL = KileDocument::Info::makeValidTeXURL(fileURL);
		if ( validURL != fileURL )
			m_file->setText(validURL.fileName());

		if ( QFileInfo( QDir(fi.dirPath()) , file().stripWhiteSpace()).exists() )
		{
			if (KMessageBox::warningYesNo(this, i18n("The file \"%1\" already exists, overwrite it?").arg(file()),
				i18n("File Already Exists")) == KMessageBox::No)
				return;
		}
	}

	if (QFileInfo(location()).exists())
	{
		KMessageBox::error(this, i18n("The project file already exists, please select another name. Delete the existing project file if your intention was to overwrite it."),i18n("Project File Already Exists"));
		return;
	}

	accept();
}

void KileNewProjectDlg::fillProjectDefaults()
{
	m_dir = KileConfig::defaultProjectLocation();
	if( !m_dir.endsWith("/") )
		m_dir += '/';
	kdDebug() << "M_DIR " << m_dir << endl;
	m_location->setText(m_dir);
	m_cb->setChecked(true);

	KileProjectDlgBase::fillProjectDefaults();
}

TemplateItem* KileNewProjectDlg::getSelection() const
{
	return static_cast<TemplateItem*>(m_nfw->currentItem());
}

/*
 * KileProjectOptionsDlg
 */
KileProjectOptionsDlg::KileProjectOptionsDlg(KileProject *project, QWidget *parent, const char * name) :
 	KileProjectDlgBase(i18n("Project Options"), parent, name )
{
	QGridLayout *layout = new QGridLayout(plainPage(), 5,4, 10);

	QLabel *lb = new QLabel(i18n("Project &title:"), plainPage());
	lb->setBuddy(m_title);
	QWhatsThis::add(lb, whatsthisName);
	layout->addWidget(lb, 0,0);
	layout->addMultiCellWidget(m_title, 0,0, 1,3);

	layout->addWidget(m_sel_extensions, 2,0);
	layout->addMultiCellWidget(m_extensions, 2,2, 1,3);
	layout->addMultiCellWidget(m_isregexp, 3,3, 1,3);

	m_master = new KComboBox(false, plainPage(), "master");
	//m_master->setDisabled(true);
	lb = new QLabel(i18n("&Master document:"), plainPage());
	lb->setBuddy(m_master);
	QWhatsThis::add(m_master, whatsthisMaster);
	QWhatsThis::add(lb,whatsthisMaster);
	layout->addWidget(lb, 4,0);
	layout->addMultiCellWidget(m_master, 4,4, 1,3);

	m_master->insertItem(i18n("(auto-detect)"));
	QPtrListIterator<KileProjectItem> rit(*(project->rootItems()));
	int index = 0;
	while (rit.current())
	{
		if ((*rit)->type() == KileProjectItem::Source)
		{
			m_master->insertItem((*rit)->url().fileName());
			++index;
			if ( (*rit)->url().path() == project->masterDocument() )
				m_master->setCurrentItem(index);
		}
		++rit;
	}

	if (project->masterDocument().isNull())
		m_master->setCurrentItem(0);

	lb = new QLabel(i18n("&QuickBuild configuration:"), plainPage()); layout->addWidget(lb, 5, 0);
	m_cbQuick = new KComboBox(plainPage()); layout->addWidget(m_cbQuick, 5, 1);
	lb->setBuddy(m_cbQuick);
	m_cbQuick->insertItem(tool_default);
	m_cbQuick->insertStringList(KileTool::configNames("QuickBuild", kapp->config()));
	m_cbQuick->setCurrentText(project->quickBuildConfig().length() > 0 ? project->quickBuildConfig() : tool_default );

	//don't put this after the call to toggleMakeIndex
	setProject(project, true);

	m_ckMakeIndex = new QCheckBox(i18n("&MakeIndex options"), plainPage()); layout->addWidget(m_ckMakeIndex, 6, 0);
	connect(m_ckMakeIndex, SIGNAL(toggled(bool)), this, SLOT(toggleMakeIndex(bool)));
	m_leMakeIndex = new KLineEdit(plainPage()); layout->addMultiCellWidget(m_leMakeIndex, 6, 6, 1, 3);
	m_ckMakeIndex->setChecked(project->useMakeIndexOptions());
	toggleMakeIndex(m_ckMakeIndex->isChecked());
}

KileProjectOptionsDlg::~KileProjectOptionsDlg()
{
}

void KileProjectOptionsDlg::toggleMakeIndex(bool on)
{
	kdDebug() << "TOGGLED!" << endl;
	m_leMakeIndex->setEnabled(on);
	m_project->setUseMakeIndexOptions(on);
	m_project->writeUseMakeIndexOptions();
	m_project->readMakeIndexOptions();
	m_leMakeIndex->setText(m_project->makeIndexOptions());
}

void KileProjectOptionsDlg::slotOk()
{
	this->m_project->setName(m_title->text());

	QPtrListIterator<KileProjectItem> rit(*(m_project->rootItems()));
	while (rit.current())
	{
		if ((*rit)->url().fileName() == m_master->currentText() )
			m_project->setMasterDocument((*rit)->url().path());
		++rit;
	}
	if (m_master->currentItem() == 0) m_project->setMasterDocument(QString::null);

	m_val_extensions[m_sel_extensions->currentItem()]
		= m_extensions->text();
	m_val_isregexp[m_sel_extensions->currentItem()]
		= m_isregexp->isChecked();

	for (int i = KileProjectItem::Source; i < KileProjectItem::Other; ++i) {
		m_project->setExtensions
			((KileProjectItem::Type) i, m_val_extensions[i-1]);
		m_project->setExtIsRegExp
			((KileProjectItem::Type) i, m_val_isregexp[i-1]);
	}

	if ( m_cbQuick->currentText() == tool_default )
		m_project->setQuickBuildConfig("");
	else
		m_project->setQuickBuildConfig(m_cbQuick->currentText());

	m_project->setUseMakeIndexOptions(m_ckMakeIndex->isChecked());
	if ( m_project->useMakeIndexOptions() )
		m_project->setMakeIndexOptions(m_leMakeIndex->text());

	m_project->save();

	accept();
}


#include "kileprojectdlgs.moc"
