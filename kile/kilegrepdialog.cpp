/* This file is part of the kile project
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>
   Copyright (C) 2003 Jan-Marek Glogowski <glogow@stud.fbi.fh-darmstadt.de>
   Copyright (C) 2005 Holger Danielsson <holgerdanielsson@t-online.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.

   Original from kdebase / kate

   changes: 2005-11-27 (dani)
    - add a search for all files of a Kile project
      (done with one grep command for each file)
    - dialog is now based on KDialogBase
    - an item of the resultbox ist opened when it's highlightened 
      (no double click is needed anymore)
    - dialog is deleted after work to minimize resources
    - added additional search modes for environments, labels etc.
    - fixed some bugs (f.e. two slashes at the end of directory 
      names, jumping to the wrong line, wrong pattern lists)
    - add some editable template modes to search for LaTeX commands
    - add some predined modes to search for environments, graphics,
      labels, and references, either all of them or some special ones

    (in other words: most parts have changed to work perfectly with Kile ...)
*/

#include "kilegrepdialog.h"

#include <qobject.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qevent.h>
#include <qregexp.h>
#include <qwhatsthis.h>
#include <qcursor.h>

#include <kcombobox.h>
#include <kapplication.h>
#include <kaccelmanager.h>
#include <kbuttonbox.h>
#include <kfiledialog.h>
#include <kprocess.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kurlrequester.h>
#include <kurlcompletion.h>
#include <klineedit.h>
#include <klistbox.h>
#include <kdebug.h>

#include "kileconfig.h"
#include "kileproject.h"
#include "kiledocmanager.h"

KileGrepDialog::KileGrepDialog(QWidget *parent, KileInfo *ki, KileGrep::Mode mode, const char *name)
	: KDialogBase (parent, name, false, QString::null, 0, Ok, false ), 
	  m_ki(ki), m_mode(mode), childproc(0), m_grepJobs(0)
{
	QWidget *page = new QWidget(this);
	setMainWidget(page);
	setWFlags( Qt::WStyle_StaysOnTop );

	// build dialog
	QVBoxLayout *vbox = new QVBoxLayout( page,5,KDialog::spacingHint() );

	// project groupbox
	QGroupBox *projectgroup = new QGroupBox( i18n("Project"),page); 
	projectgroup->setColumnLayout(0, Qt::Vertical );
	projectgroup->layout()->setSpacing( 6 );
	projectgroup->layout()->setMargin( 11 );
	QGridLayout *projectgrouplayout = new QGridLayout( projectgroup->layout() );
	projectgrouplayout->setAlignment( Qt::AlignTop );

	QLabel *project_label = new QLabel(i18n("Name:"), projectgroup);
	int labelwidth = project_label->sizeHint().width();

	QLabel *projectdir_label = new QLabel(i18n("Directory:"), projectgroup);
	if ( projectdir_label->sizeHint().width() > labelwidth )
		labelwidth = projectdir_label->sizeHint().width();

	projectname_label = new QLabel(projectgroup);
	projectdirname_label = new QLabel(projectgroup);

	projectgrouplayout->addWidget(project_label, 0,0, AlignLeft | AlignVCenter);
	projectgrouplayout->addWidget(projectname_label, 0,1, AlignLeft | AlignVCenter);
	projectgrouplayout->addWidget(projectdir_label, 1,0, AlignLeft | AlignVCenter);
	projectgrouplayout->addWidget(projectdirname_label, 1,1, AlignLeft | AlignVCenter);
	projectgrouplayout->setColStretch(1,1);

	// search groupbox
	QGroupBox *searchgroup = new QGroupBox( i18n("Search"),page); 
	searchgroup->setColumnLayout(0, Qt::Vertical );
	searchgroup->layout()->setSpacing( 6 );
	searchgroup->layout()->setMargin( 11 );
	QGridLayout *searchgrouplayout = new QGridLayout( searchgroup->layout() );
	searchgrouplayout->setAlignment( Qt::AlignTop );

	QLabel *pattern_label = new QLabel(i18n("Pattern:"), searchgroup);
	if ( pattern_label->sizeHint().width() > labelwidth )
		labelwidth = pattern_label->sizeHint().width();

	pattern_combo = new KComboBox(true, searchgroup);
	pattern_combo->setInsertionPolicy(KComboBox::NoInsertion);
	pattern_combo->setFocus();
	pattern_combo->setMinimumSize(pattern_combo->sizeHint());
	pattern_label->setBuddy(pattern_combo);

	QLabel *template_label = new QLabel(i18n("Template:"), searchgroup);
	if ( template_label->sizeHint().width() > labelwidth )
		labelwidth = template_label->sizeHint().width();

	QStringList templatemode_list;
	templatemode_list << i18n("Normal") 
	                  << i18n("Command") 
	                  << i18n("Command[]") 
	                  << i18n("Environment") 
	                  << i18n("Image")
	                  << i18n("Label") 
	                  << i18n("Reference")
	                  << i18n("File")
	                  ;

	QBoxLayout *template_layout = new QHBoxLayout(4);
	template_combo = new KComboBox(false, searchgroup);
	template_combo->insertStringList(templatemode_list);
	template_combo->adjustSize();
	template_combo->setFixedSize(template_combo->size());
	template_layout->addWidget(template_combo);
	m_lastTemplateIndex = 0;

	template_edit = new KLineEdit(searchgroup);
	template_edit->setText("%s");
	template_edit->setMinimumSize(template_edit->sizeHint());
	template_label->setBuddy(template_edit);
	template_layout->addWidget(template_edit);

	searchgrouplayout->addWidget(pattern_label, 0,0, AlignLeft | AlignVCenter);
	searchgrouplayout->addWidget(pattern_combo, 0,1);
	searchgrouplayout->addWidget(template_label, 1,0, AlignLeft | AlignVCenter);
	searchgrouplayout->addLayout(template_layout, 1,1);

	// filter groupbox
	QGroupBox *filtergroup = new QGroupBox( i18n("Directory Options"),page); 
	filtergroup->setColumnLayout(0, Qt::Vertical );
	filtergroup->layout()->setSpacing( 6 );
	filtergroup->layout()->setMargin( 11 );
	QGridLayout *filtergrouplayout = new QGridLayout( filtergroup->layout() );
	filtergrouplayout->setAlignment( Qt::AlignTop );

	QLabel *files_label = new QLabel(i18n("Filter:"), filtergroup);
	if ( files_label->sizeHint().width() > labelwidth )
		labelwidth = files_label->sizeHint().width();

	filter_combo = new KComboBox(true, filtergroup);
	filter_combo->setMinimumSize(filter_combo->sizeHint());
	files_label->setBuddy(filter_combo->focusProxy());

	QLabel *dir_label = new QLabel(i18n("Directory:"), filtergroup);
	if ( dir_label->sizeHint().width() > labelwidth )
		labelwidth = dir_label->sizeHint().width();

	QBoxLayout *dir_layout = new QHBoxLayout(3);
	dir_combo = new KURLRequester( new KComboBox(true, filtergroup), filtergroup, "dir combo" );
	dir_combo->completionObject()->setMode(KURLCompletion::DirCompletion);
	dir_combo->setMode(KFile::Directory|KFile::LocalOnly|KFile::ExistingOnly);
	dir_label->setBuddy(dir_combo);
	dir_layout->addWidget(dir_combo);

	recursive_box = new QCheckBox(i18n("Scan directories recursive"), filtergroup);
	recursive_box->setMinimumWidth(recursive_box->sizeHint().width());

	filtergrouplayout->addWidget(files_label, 2,0, AlignLeft | AlignVCenter);
	filtergrouplayout->addWidget(filter_combo, 2, 1);
	filtergrouplayout->addWidget(dir_label, 3,0, AlignLeft | AlignVCenter);
	filtergrouplayout->addLayout(dir_layout, 3,1);
	filtergrouplayout->addMultiCellWidget(recursive_box, 4,4, 1,2);
	filtergrouplayout->setColStretch(1,1);

	// result box
	resultbox = new KListBox(page);
	resultbox->setMinimumHeight(150);

	// button box
	KButtonBox *actionbox = new KButtonBox(page, Qt::Horizontal);
	search_button = actionbox->addButton(i18n("&Search"));
	search_button->setDefault(true);
	search_button->setEnabled(false);
	clear_button = actionbox->addButton(i18n("&Clear"));
	clear_button->setEnabled(false);
	actionbox->addStretch();
#if KDE_VERSION >= KDE_MAKE_VERSION(3,3,0)
	close_button = actionbox->addButton(KStdGuiItem::close());
#else
        close_button = actionbox->addButton(i18n("Cl&ose"));
#endif
	actionbox->layout();

	// adjust labels
	project_label->setFixedWidth(labelwidth);
	projectdir_label->setFixedWidth(labelwidth);
	pattern_label->setFixedWidth(labelwidth);
	template_label->setFixedWidth(labelwidth);
	files_label->setFixedWidth(labelwidth);
	dir_label->setFixedWidth(labelwidth);

	if ( m_mode == KileGrep::Project )
	{
		filtergroup->hide();
		vbox->addWidget(projectgroup); 
		vbox->addWidget(searchgroup);
	}
	else
	{
		projectgroup->hide();
		vbox->addWidget(searchgroup);
		vbox->addWidget(filtergroup); 
	}
	vbox->addWidget(resultbox);
	vbox->addWidget(actionbox);

	// Produces error messages like
	// QListBox::property( "text" ) failed:
	// 	property invalid or does not exist
	// Anyone an idea?
	KAcceleratorManager::manage( this );

	QWhatsThis::add(pattern_combo,
		i18n("Enter the regular expression you want to search for here.<br>"
		 "Possible meta characters are:<br>"
		 "<ul>"
		 "<li>&nbsp;<b>.</b> - Matches any character</li>"
		 "<li>&nbsp;<b>^</b> - Matches the beginning of a line</li>"
		 "<li>&nbsp;<b>$</b> - Matches the end of a line</li>"
		 "<li>&nbsp;<b>\\\\\\&lt;</b> - Matches the beginning of a word</li>"
		 "<li>&nbsp;<b>\\\\\\&gt;</b> - Matches the end of a word</li>"
		 "</ul>"
		 "The following repetition operators exist:"
		 "<ul>"
		 "<li>&nbsp;<b>?</b> - The preceding item is matched at most once</li>"
		 "<li>&nbsp;<b>*</b> - The preceding item is matched zero or more times</li>"
		 "<li>&nbsp;<b>+</b> - The preceding item is matched one or more times</li>"
		 "<li>&nbsp;<b>{<i>n</i>}</b> - The preceding item is matched exactly <i>n</i> times</li>"
		 "<li>&nbsp;<b>{<i>n</i>,}</b> - The preceding item is matched <i>n</i> or more times</li>"
		 "<li>&nbsp;<b>{,<i>n</i>}</b> - The preceding item is matched at most <i>n</i> times</li>"
		 "<li>&nbsp;<b>{<i>n</i>,<i>m</i>}</b> - The preceding item is matched at least <i>n</i>, "
		 "but at most <i>m</i> times.</li>"
		 "</ul>"
		 "Furthermore, backreferences to bracketed subexpressions are "
		 "available via the notation \\\\<i>n</i>."
		 ));
	QWhatsThis::add(filter_combo,
		i18n("Enter the file name pattern of the files to search here. "
		 "You may give several patterns separated by commas."));
	QWhatsThis::add(template_combo,
		i18n("Choose one search mode. For the first modes, the search pattern is "
		 "build from the editable template, where '%s' is replaced by the given pattern.<br><br>"
		 "There are additional fixed predefined modes for environments, graphics, labels, references "
		 "and input files. If the pattern is empty, Kile will search for all commands of this mode. "
		 "If a pattern is given, it will be inserted as a parameter. F.e., in environment mode with "
		 "pattern 'center', Kile will search for '\\begin{center}' and in graphics mode with "
		 "pattern '.*\\.png', Kile will search for all png files."));
	QWhatsThis::add(template_edit,
		i18n("For the first three modes you can choose a template for the pattern from the combo box "
		 "and edit it here. The string %s in the template is replaced "
		 "by the pattern input field, resulting in the regular expression "
		 "to search for. In all other modes this template is ignored."));
	QWhatsThis::add(dir_combo,
		i18n("Enter the directory which contains the files you want to search in."));
	QWhatsThis::add(recursive_box,
		i18n("Check this box to search in all subdirectories."));
	QWhatsThis::add(resultbox,
		i18n("The results of the grep run are listed here. Select a "
		 "filename/line number combination with a mouse click on the item "
		 "or with the cursor to show the respective line in the editor."));

	// read config and setup dialog for both modes
	readConfig();
	if ( m_mode == KileGrep::Directory )
	{
		setCaption(i18n("Find in Files"));
		setupDirectory();
	}
	else
	{
		setCaption(i18n("Find in Project"));
		setupProject();
	}
 
	pattern_combo->setEditText(QString::null);
	template_edit->setText(template_list[0]);
	slotPatternTextChanged(QString::null);

	connect( pattern_combo->lineEdit(), SIGNAL(textChanged ( const QString & )),
		SLOT( slotPatternTextChanged( const QString & )));
	connect( template_combo, SIGNAL(activated(int)),
		SLOT(slotTemplateActivated(int)) );
	connect( resultbox, SIGNAL(highlighted(const QString&)),
		SLOT(slotItemSelected(const QString&)) );

	connect( search_button, SIGNAL(clicked()), SLOT(slotSearch()) );
	connect( clear_button,  SIGNAL(clicked()), SLOT(slotClear()) );
	connect( close_button,  SIGNAL(clicked()), SIGNAL(closeClicked()) );

	connect( this, SIGNAL(closeClicked()), SLOT(slotClose()) );
	connect( this, SIGNAL(finished()), SLOT(slotFinished()) );

	resize(450,sizeHint().height());
	kdDebug() << "==KileGrepDialog (create dialog)=============================" << endl;
}

KileGrepDialog::~KileGrepDialog()
{
	kdDebug() << "==KileGrepDialog (delete dialog)=============================" << endl;
	writeConfig();
	delete childproc;
}

///////////////////// config /////////////////////

void KileGrepDialog::readConfig()
{
	pattern_combo->insertStringList( readList(KileGrep::SearchItems) );

	QString labels = getCommandList(KileDocument::CmdAttrLabel);
	QString references = getCommandList(KileDocument::CmdAttrReference);
	template_list = readList(KileGrep::SearchTemplates) ;
	if ( template_list.count() != 3 )
	{
		template_list.clear();
		template_list << "%s" << "\\\\%s\\{" << "\\\\%s(\\[[^]]*\\])?\\{";
	}
	template_list << "\\\\begin\\{"                             // to be closed with "%s\\}"
	              << "\\\\includegraphics(\\[[^]]*\\])?\\{"
	              << "\\\\(label" + labels + ")\\{"
	              << "\\\\(ref|pageref|vref|vpageref|fref|Fref|eqref" + references + ")(\\[[^]]*\\])?\\{"
	              << "\\\\(input|include)\\{"
	              ;
	
	if ( m_mode == KileGrep::Directory )
	{
		dir_combo->comboBox()->insertStringList( readList(KileGrep::SearchPaths) );
		recursive_box->setChecked( KileConfig::grepRecursive() );
	}
}

void KileGrepDialog::writeConfig()
{
	KileConfig::setLastSearchItems( getListItems(pattern_combo) );

	QStringList list;
	list << template_list[0] << template_list[1] << template_list[2];
	KileConfig::setLastSearchTemplates( list );

	if ( m_mode == KileGrep::Directory )
	{
		KileConfig::setLastSearchPaths( getListItems(dir_combo->comboBox()) );
		KileConfig::setGrepRecursive( recursive_box->isChecked() );
	}
}

///////////////////// setup search modes /////////////////////

void KileGrepDialog::setupDirectory()
{
	setDirName( QDir::home().absPath() );

	QString filter(SOURCE_EXTENSIONS);
	filter.append(" ");
	filter.append(PACKAGE_EXTENSIONS);
	filter.replace(".", "*.");
	filter.replace(" ", ",");
	filter.append("|");
	filter.append(i18n("TeX Files"));
	filter.append("\n*|");
	filter.append(i18n("All Files"));
	setFilter(filter);
}

void KileGrepDialog::setupProject()
{
	KileProject *project = m_ki->docManager()->activeProject();
	if ( project )
	{
		m_projectOpened = true;
		m_projectdir = project->baseURL().path();
		projectname_label->setText( project->name() );
		projectdirname_label->setText( m_projectdir );

		m_projectfiles.clear();
		m_projectfiles = m_ki->docManager()->getProjectFiles();
	}
	else
	{
		m_projectOpened = false;
		projectname_label->setText( i18n("no project opened") );
		projectdirname_label->setText( QString::null );
	}
}

///////////////////// read entries /////////////////////

QStringList KileGrepDialog::readList(KileGrep::List listtype)
{
	QStringList strings,result;

	bool stripSlash = false;
	switch ( listtype )
	{
		case KileGrep::SearchItems: 
			strings = KileConfig::lastSearchItems(); 
			break;
		case KileGrep::SearchPaths: 
			strings = KileConfig::lastSearchPaths(); 
			stripSlash = true;
			break;
		case KileGrep::SearchTemplates: 
			strings = KileConfig::lastSearchTemplates(); 
			break;
	}

	while (strings.count() > 0) 
	{
		if ( stripSlash && strings[0].right(1)=="/" )
			strings[0].truncate( strings[0].length()-1 );
		if ( ! strings[0].isEmpty() )
			result.append(strings[0]);
		strings.remove(strings[0]);
	}
	return result;
}

///////////////////// item selected /////////////////////

void KileGrepDialog::slotItemSelected(const QString& item)
{
	kdDebug() << "\tgrep: start item selected" << endl;
	int pos;
	QString filename, linenumber;

	QString str = item;
	if ( (pos = str.find(':')) != -1)
	{
		filename = str.left(pos);
		str = str.right(str.length()-1-pos);
		if ( (pos = str.find(':')) != -1)
		{
			linenumber = str.left(pos);
			if ( m_mode == KileGrep::Project )
				emit itemSelected(m_projectdir + "/" + filename,linenumber.toInt());
			else
				//emit itemSelected(lastSearchPaths[0] + "/" + filename,linenumber.toInt());
				emit itemSelected(dir_combo->comboBox()->text(0) + "/" + filename,linenumber.toInt());
		}
	}
}

///////////////////// grep /////////////////////

void KileGrepDialog::startGrep()
{
	childproc = new KProcess();
	childproc->setUseShell(true);

	if ( m_mode == KileGrep::Project )
	{
		QString command = buildProjectCommand() + " " + KProcess::quote(m_projectfiles[m_grepJobs-1]);
		kdDebug() << "\tgrep (project): " <<  command << endl;
		(*childproc) << QStringList::split(' ',command);
	}
	else
	{
		QString command = buildFilesCommand();
		kdDebug() << "\tgrep (files): " << command << endl;
		(*childproc) << QStringList::split(' ', command);
	}
	m_grepJobs--;

	connect( childproc, SIGNAL(processExited(KProcess *)),
		SLOT(childExited()) );
	connect( childproc, SIGNAL(receivedStdout(KProcess *, char *, int)),
		SLOT(receivedOutput(KProcess *, char *, int)) );
	connect( childproc, SIGNAL(receivedStderr(KProcess *, char *, int)),
		SLOT(receivedErrOutput(KProcess *, char *, int)) );

	childproc->start(KProcess::NotifyOnExit, KProcess::AllOutput);
}

void KileGrepDialog::processOutput()
{
	int pos;
	while ( (pos = buf.find('\n')) != -1)
	{
		QString item = buf.left(pos);
		if ( ! item.isEmpty() )
		{
			if ( m_mode == KileGrep::Project )
			{
				if ( item.find(m_projectdir+"/") == 0 )
					resultbox->insertItem( item.mid(m_projectdir.length()+1) );
				else
					resultbox->insertItem(item);
			}
			else
			{
				resultbox->insertItem( item.mid(dir_combo->url().length()+1) );
			}
		}
		buf = buf.right(buf.length()-pos-1);
	}
	kapp->processEvents();
}

void KileGrepDialog::receivedOutput(KProcess */*proc*/, char *buffer, int buflen)
{
	buf += QString::fromLocal8Bit(buffer, buflen);
	processOutput();
}

void KileGrepDialog::receivedErrOutput(KProcess */*proc*/, char *buffer, int buflen)
{
	errbuf += QString::fromLocal8Bit( buffer, buflen );
}

void KileGrepDialog::childExited()
{
//	 int status = childproc->exitStatus();

	if ( ! errbuf.isEmpty() )
	{
		KMessageBox::information( parentWidget(),
			i18n("<strong>Error:</strong><p>") + errbuf,
			i18n("Grep Tool Error") );
		errbuf.truncate(0);
	}
	else
		finish();
}

void KileGrepDialog::finish()
{
	buf += '\n';
	processOutput();
	if ( childproc )
	{
		delete childproc;
		childproc = 0;
	}

	if ( shouldRestart() )
	{
		startGrep();
	}
	else
	{
		updateLists();

		resultbox->unsetCursor();
		clear_button->setEnabled( resultbox->count() > 0 );
		search_button->setText( i18n("&Search") );
		slotPatternTextChanged( pattern_combo->lineEdit()->text() );
	}
}

void KileGrepDialog::updateLists()
{
	updateListItems(pattern_combo);
	if ( m_mode==KileGrep::Directory )
	{
		updateListItems(dir_combo->comboBox());
	}
}

///////////////////// build commands /////////////////////

QString KileGrepDialog::getPattern()
{
	QString pattern;
	int template_mode = template_combo->currentItem();
	if ( template_mode < KileGrep::tmEnv )
	{
		pattern = template_edit->text();
		if ( pattern.isEmpty() )
			pattern = pattern_combo->currentText();
		else
			pattern.replace("%s", pattern_combo->currentText());
	}
	else
	{
		pattern = template_list[template_mode];
		if ( ! pattern_combo->currentText().isEmpty() )
			pattern += pattern_combo->currentText()  + "\\}";
	}

	return pattern;
}

QString KileGrepDialog::getShellPattern()
{
	QString pattern = getPattern();
	pattern.replace("'", "'\\''");
	return QString("'") + pattern + "'";
	//return KProcess::quote(pattern);
}


QString KileGrepDialog::buildFilesCommand()
{
	QString files, files_temp;

	if (filter_combo->currentItem() >= 0)
		files_temp = filter_list[filter_combo->currentItem()];
	else
		files_temp = filter_combo->currentText();

	if (files_temp.right(1) != ",")
		files_temp = files_temp + ",";

	QStringList tokens = QStringList::split ( ",", files_temp, false );
	QStringList::Iterator it = tokens.begin();
	if (it != tokens.end())
	{
		files = " '" + (*it) + "'" ;
		++it;
	}

	for ( ; it != tokens.end(); ++it )
		files = files + " -o -name " + "'" + (*it) + "'";

	QString shell_command;
	shell_command += "find ";
	shell_command += KProcess::quote(dir_combo->url());
	shell_command += " \\( -name ";
	shell_command += files;
	shell_command += " \\)";
	if (!recursive_box->isChecked())
		shell_command += " -maxdepth 1";
	shell_command += " -exec grep -n -E -I -H -e " + getShellPattern() + " {} \\;";

	return shell_command;
}

QString KileGrepDialog::buildProjectCommand()
{
	return "grep -n -E -I -H -e " + getShellPattern();
}

///////////////////// Search /////////////////////

void KileGrepDialog::slotSearch()
{
	kdDebug() << "\tgrep: start slot search" << endl;
	slotClear ();

	if ( template_combo->currentItem()<KileGrep::tmEnv && pattern_combo->currentText().isEmpty() )
		return;

	if ( childproc && childproc->isRunning() )
	{
		childproc->kill();
		return;
	}

	kdDebug() << "\tgrep: start new search" << endl;
	QRegExp re( getPattern() );
	if ( ! re.isValid() )
	{
		KMessageBox::error( 0, i18n("Invalid regular expression: %1").arg(re.errorString()), i18n("Grep Tool Error") );
		return;
	}

	resultbox->setCursor( QCursor(Qt::WaitCursor) );
	search_button->setText( i18n("&Cancel") );
	if ( template_combo->currentItem() < KileGrep::tmEnv)
		template_list[m_lastTemplateIndex] = template_edit->text();

	// start grep command
	m_grepJobs = ( m_mode == KileGrep::Project ) ? m_projectfiles.count() : 1;
	startGrep();
}

void KileGrepDialog::slotSearchFor(const QString &pattern)
{
	slotClear();
	pattern_combo->setEditText(pattern);
	slotSearch();
}

void KileGrepDialog::slotClear()
{
	//kdDebug() << "\tgrep: slot clear" << endl;
	clearGrepJobs();
	resultbox->clear();
	finish();
}

void KileGrepDialog::slotClose()
{
	//kdDebug() << "\tgrep: slot close" << endl;
	clearGrepJobs();
	finish();
	delayedDestruct();
}

void KileGrepDialog::slotFinished()
{
	//kdDebug() << "\tgrep: slot finished" << endl;
	finish();
	delayedDestruct();
}

///////////////////// templates /////////////////////

void KileGrepDialog::slotPatternTextChanged(const QString &)
{
	updateWidgets();
}

void KileGrepDialog::slotTemplateActivated(int index)
{
	if ( index < KileGrep::tmEnv)
	{
		template_list[m_lastTemplateIndex] = template_edit->text();
		template_edit->setText( template_list[index] );
	}
	else
	{
		template_edit->setText(QString::null);
	}
	m_lastTemplateIndex = index;

	updateWidgets();
}

void KileGrepDialog::updateWidgets()
{
	bool search_state = (m_mode==KileGrep::Directory) || (m_mode==KileGrep::Project && m_projectOpened);

	if ( template_combo->currentItem()  < KileGrep::tmEnv )
	{
		template_edit->setEnabled(true);
		search_button->setEnabled( search_state && !pattern_combo->currentText().isEmpty() );
	}
	else
	{
		template_edit->setEnabled(false);
		search_button->setEnabled( search_state );
	}
}

///////////////////// directory /////////////////////

void KileGrepDialog::setDirName(const QString &dir)
{
	dir_combo->setURL(dir);
	if (dir_combo->comboBox()->text(0) != dir)
		slotClear();
}

///////////////////// filter /////////////////////

void KileGrepDialog::setFilter(const QString &filter)
{
	filter_list.clear();
	filter_combo->clear();
	if ( !filter.isEmpty() )
	{
		QStringList filter_lst = QStringList::split("\n", filter);
		for (QStringList::Iterator it = filter_lst.begin();
			it != filter_lst.end(); ++it)
		{
			QStringList filter_split = QStringList::split("|", *it);
			filter_list.append(filter_split[0]);
			filter_combo->insertItem(filter_split[1]);
		}
	}
}

void KileGrepDialog::appendFilter(const QString &name, const QString &filter)
{
	filter_combo->insertItem(name);
	filter_list.append(filter);
}

///////////////////// template /////////////////////

void KileGrepDialog::appendTemplate(const QString &name, const QString &regexp)
{
	template_combo->insertItem(name);
	template_list.append(regexp);
}

void KileGrepDialog::clearTemplates()
{
	template_combo->clear();
	template_list.clear();
}

///////////////////// KComboBox /////////////////////

QStringList KileGrepDialog::getListItems(KComboBox *combo)
{
	QStringList list;
	for ( int i=0; i<combo->count() && i<KILEGREP_MAX; ++i )
		list.append( combo->text(i) );
	return list;
}

int KileGrepDialog::findListItem(KComboBox *combo, const QString &s)
{
	for ( int i=0; i<combo->count(); ++i )
	{
		if ( combo->text(i) == s )
			return i;
	}
	return -1;
}

void KileGrepDialog::updateListItems(KComboBox *combo)
{
	QString s = combo->currentText();
	if ( s.isEmpty() )
		return;

	int index = findListItem(combo,s);
	if ( index > 0 )                               // combo already contains s
	{
		combo->removeItem(index);                   // remove this item
		combo->insertItem(s,0);                     // insert this item as first item
	}
	else if ( index == -1 )                        // combo doesn't contain s
	{
		if ( combo->count() >= KILEGREP_MAX )
			combo->removeItem( combo->count()-1 );   // remove last item
		combo->insertItem(s,0);                     // insert this item as first item
	}
}

///////////////////// template /////////////////////

QString KileGrepDialog::getCommandList(KileDocument::CmdAttribute attrtype)
{
	QStringList cmdlist;
	QStringList::ConstIterator it;

	// get info about user defined references
	KileDocument::LatexCommands *cmd = m_ki->latexCommands();
	cmd->commandList(cmdlist,attrtype,true);

	// build list of references
	QString commands = QString::null;
	for ( it=cmdlist.begin(); it != cmdlist.end(); ++it ) 
	{
		commands += "|" + (*it).mid(1);
	}
	return commands;
}


#include "kilegrepdialog.moc"
