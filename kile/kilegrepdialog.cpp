/* This file is part of the kile project
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>
   Copyright (C) 2003 Jan-Marek Glogowski <glogow@stud.fbi.fh-darmstadt.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Original from kdebase / kate
*/

#include "kilegrepdialog.h"

#include <qobject.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qevent.h>
#include <qlistbox.h>
#include <qregexp.h>
#include <qwhatsthis.h>
#include <qcursor.h>

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
#include <kcombobox.h>
#include <klineedit.h>

#include <kdebug.h>

static void clear_dups(QStringList &strings)
{
	QStringList result;
	while (strings.count() > 0) {
		result.append(strings[0]);
		strings.remove(strings[0]);
	}
	strings = result;
}

KileGrepDialog::KileGrepDialog(const QString &dirname, QWidget *parent, const char *name)
	: KDialog( parent, name ), childproc(0)
{
	setCaption(i18n("Find in Files"));
	config = KGlobal::config();
	config->setGroup("KileGrepDialog");

	lastSearchItems = config->readListEntry("LastSearchItems");
	clear_dups(lastSearchItems);
	lastSearchPaths = config->readListEntry("LastSearchPaths");
	clear_dups(lastSearchPaths);

	QGridLayout *input_layout = new QGridLayout(this, 7, 3, 4, 4);
	input_layout->setColStretch(0, 0);
	input_layout->setColStretch(1, 20);

	QLabel *pattern_label = new QLabel(i18n("Pattern:"), this);
	pattern_label->setFixedSize(pattern_label->sizeHint());
	input_layout->addWidget(pattern_label, 0, 0, AlignRight | AlignVCenter);

	pattern_combo = new QComboBox(true, this);
	pattern_combo->insertStringList(lastSearchItems);
	pattern_combo->setEditText(QString::null);
	pattern_combo->setInsertionPolicy(QComboBox::NoInsertion);
	pattern_label->setBuddy(pattern_combo);
	pattern_combo->setFocus();
	pattern_combo->setMinimumSize(pattern_combo->sizeHint());
	input_layout->addWidget(pattern_combo, 0, 1);

	QLabel *template_label = new QLabel(i18n("Template:"), this);
	template_label->setFixedSize(template_label->sizeHint());
	input_layout->addWidget(template_label, 1, 0, AlignRight | AlignVCenter);

	QBoxLayout *template_layout = new QHBoxLayout(4);
	input_layout->addLayout(template_layout, 1, 1);

	template_combo = new QComboBox(false, this);
	template_combo->insertItem(i18n("normal"));
	template_list.append("%s");
	template_combo->adjustSize();
	template_combo->setFixedSize(template_combo->size());
	template_layout->addWidget(template_combo);

	template_edit = new QLineEdit(this);
	template_label->setBuddy(template_edit);
	template_edit->setText("%s");
	template_edit->setMinimumSize(template_edit->sizeHint());
	template_layout->addWidget(template_edit);

	QLabel *files_label = new QLabel(i18n("Filter:"), this);
	files_label->setFixedSize(files_label->sizeHint());
	input_layout->addWidget(files_label, 2, 0, AlignRight | AlignVCenter);

	filter_combo = new QComboBox(true, this);
	files_label->setBuddy(filter_combo->focusProxy());
	filter_combo->setMinimumSize(filter_combo->sizeHint());
	input_layout->addWidget(filter_combo, 2, 1);

	QLabel *dir_label = new QLabel(i18n("Directory:"), this);
	dir_label->setFixedSize(dir_label->sizeHint());
	input_layout->addWidget(dir_label, 3, 0, AlignRight | AlignVCenter);

	QBoxLayout *dir_layout = new QHBoxLayout(3);
	input_layout->addLayout(dir_layout, 3, 1);

	dir_combo = new KURLRequester( new KComboBox(true, this), this, "dir combo" );
	dir_combo->completionObject()->setMode(KURLCompletion::DirCompletion);
	dir_combo->comboBox()->insertStringList(lastSearchPaths);
	dir_combo->setMode( KFile::Directory|KFile::LocalOnly );
	if (dirname.right(0) == "/")
		{ dir_combo->setURL(dirname); }
	else { dir_combo->setURL(dirname + "/"); }
	dir_layout->addWidget(dir_combo);
	dir_label->setBuddy(dir_combo);

	recursive_box = new QCheckBox(i18n("Scan directories recursive"), this);
	recursive_box->setMinimumWidth(recursive_box->sizeHint().width());
	recursive_box->setChecked(true);
	input_layout->addMultiCellWidget(recursive_box, 4, 4, 1, 2);

	KButtonBox *actionbox = new KButtonBox(this, Qt::Horizontal);
	input_layout->addMultiCellWidget(actionbox, 5, 5, 0, 2);
	actionbox->addStretch();
	search_button = actionbox->addButton(i18n("Search"));
	search_button->setDefault(true);
	clear_button = actionbox->addButton(i18n("Clear"));
	cancel_button = actionbox->addButton(i18n("Cancel"));
	actionbox->addStretch();
	actionbox->layout();

	resultbox = new QListBox(this);
	input_layout->addMultiCellWidget(resultbox, 6, 6, 0, 2);
	resultbox->setMinimumHeight(70);

	// Produces error messages like
	// QListBox::property( "text" ) failed:
	// 	property invalid or does not exist
	// Anyone an idea?
	KAcceleratorManager::manage( this );

	QWhatsThis::add(pattern_combo,
		i18n("Enter the regular expression you want to search for here.<br>"
		 "Possible meta characters are:<br>"
		 "<b>.</b> - Matches any character<br>"
		 "<b>^</b> - Matches the beginning of a line<br>"
		 "<b>$</b> - Matches the end of a line<br>"
		 "<b>\\\\\\&lt;</b> - Matches the beginning of a word<br>"
		 "<b>\\\\\\&gt;</b> - Matches the end of a word<br>"
		 "<br>"
		 "The following repetition operators exist:<br>"
		 "<b>?</b> - The preceding item is matched at most once<br>"
		 "<b>*</b> - The preceding item is matched zero or more times<br>"
		 "<b>+</b> - The preceding item is matched one or more times<br>"
		 "<b>{<i>n</i>}</b> - The preceding item is matched exactly <i>n</i> times<br>"
		 "<b>{<i>n</i>,}</b> - The preceding item is matched <i>n</i> or more times<br>"
		 "<b>{,<i>n</i>}</b> - The preceding item is matched at most <i>n</i> times<br>"
		 "<b>{<i>n</i>,<i>m</i>}</b> - The preceding item is matched at least <i>n</i>,<br>"
		 "but at most <i>m</i> times.<br>"
		 "<br>"
		 "Furthermore, backreferences to bracketed subexpressions are<br>"
		 "available via the notation \\\\<i>n</i>."
		 ));
	QWhatsThis::add(filter_combo,
		i18n("Enter the file name pattern of the files to search here.\n"
		 "You may give several patterns separated by commas"));
	QWhatsThis::add(template_edit,
		i18n("You can choose a template for the pattern from the combo box\n"
		 "and edit it here. The string %s in the template is replaced\n"
		 "by the pattern input field, resulting in the regular expression\n"
		 "to search for."));
	QWhatsThis::add(dir_combo,
		i18n("Enter the directory which contains the files you want to search in."));
	QWhatsThis::add(recursive_box,
		i18n("Check this box to search in all subdirectories."));
	QWhatsThis::add(resultbox,
		i18n("The results of the grep run are listed here. Select a\n"
		 "filename/line number combination and press Enter or doubleclick\n"
		 "on the item to show the respective line in the editor."));

	// event filter, do something relevant for RETURN
	pattern_combo->installEventFilter( this );
	template_edit->installEventFilter( this );
	pattern_combo->installEventFilter( this );
	filter_combo->installEventFilter( this );
	dir_combo->comboBox()->installEventFilter( this );

	connect( template_combo, SIGNAL(activated(int)),
		SLOT(templateActivated(int)) );
	connect( resultbox, SIGNAL(selected(const QString&)),
		SLOT(itemSelected(const QString&)) );
	connect( search_button, SIGNAL(clicked()),
		SLOT(slotSearch()) );
	connect( clear_button, SIGNAL(clicked()),
		SLOT(slotClear()) );
	connect( cancel_button, SIGNAL(clicked()),
		SLOT(slotCancel()) );
	connect( pattern_combo->lineEdit(), SIGNAL(textChanged ( const QString & )),
		SLOT( patternTextChanged( const QString & )));

	patternTextChanged( pattern_combo->lineEdit()->text());
}


KileGrepDialog::~KileGrepDialog()
{
	delete childproc;
}

void KileGrepDialog::patternTextChanged( const QString & _text)
{
	search_button->setEnabled( !_text.isEmpty() );
}

void KileGrepDialog::templateActivated(int index)
{
	template_edit->setText(template_list[index]);
}

void KileGrepDialog::itemSelected(const QString& item)
{
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
			emit itemSelected(lastSearchPaths[0] + filename,linenumber.toInt()-1);
		}
	}
}

void KileGrepDialog::processOutput()
{
	int pos;
	while ( (pos = buf.find('\n')) != -1)
	{
		QString item = buf.left(pos);
		if (!item.isEmpty()) {
			resultbox->insertItem
				(item.mid(dir_combo->url().length()));
		}
		buf = buf.right(buf.length()-pos-1);
	}
	kapp->processEvents();
}

void KileGrepDialog::slotSearch()
{
	slotClear ();

	if (pattern_combo->currentText().isEmpty())
		return;

//	 search_button->setEnabled(false);
	if ( childproc && childproc->isRunning() )
	{
		childproc->kill();
		return;
	}

	kdDebug() << "New Search" << endl;

	QString files;
	QString files_temp;

	if (filter_combo->currentItem() >= 0)
		files_temp = filter_list[filter_combo->currentItem()];
	else
		files_temp = filter_combo->currentText();

	kdDebug() << "\t" << files_temp << endl;

	if (files_temp.right(1) != ",")
	files_temp = files_temp + ",";

	QStringList tokens = QStringList::split ( ",", files_temp, FALSE );
	QStringList::Iterator it = tokens.begin();
	if (it != tokens.end())
	files = " '"+(*it++)+"'" ;

	for ( ; it != tokens.end(); it++ )
	files = files + " -o -name " + "'"+(*it)+ "'";

	QString pattern = template_edit->text();
	pattern.replace("%s", pattern_combo->currentText());
	pattern.replace("'", "'\\''");

	QString shell_command;
	shell_command += "find ";
	shell_command += KProcess::quote(dir_combo->url());
	shell_command += " \\( -name ";
	shell_command += files;
	shell_command += " \\)";
	if (!recursive_box->isChecked())
		shell_command += " -maxdepth 1";
	shell_command += " -exec";
	shell_command += " grep";
	shell_command += " -n";
	shell_command += " -I";
	shell_command += " -H";
	shell_command += (QString(" -e ") + KProcess::quote(pattern));
	shell_command += " {} \\;";

	kdDebug() << "\t" << shell_command << endl;

	childproc = new KProcess();
	childproc->setUseShell(true);
	*childproc << shell_command;

	connect( childproc, SIGNAL(processExited(KProcess *)),
		SLOT(childExited()) );
	connect( childproc, SIGNAL(receivedStdout(KProcess *, char *, int)),
		SLOT(receivedOutput(KProcess *, char *, int)) );
	connect( childproc, SIGNAL(receivedStderr(KProcess *, char *, int)),
		SLOT(receivedErrOutput(KProcess *, char *, int)) );

	// actually it should be checked whether the process was started successfully
	resultbox->setCursor( QCursor(Qt::WaitCursor) );
	clear_button->setEnabled( false );
	search_button->setText( i18n("Cancel") );
	childproc->start(KProcess::NotifyOnExit, KProcess::AllOutput);
}

void KileGrepDialog::slotSearchFor(const QString &pattern)
{
	slotClear();
	pattern_combo->setEditText(pattern);
	slotSearch();
}

void KileGrepDialog::finish()
{
	search_button->setEnabled( !pattern_combo->lineEdit()->text().isEmpty() );

	buf += '\n';
	processOutput();
	delete childproc;
	childproc = 0;

	config->setGroup("KileGrepDialog");

	int dup_idx = lastSearchItems.findIndex(pattern_combo->currentText());
	if (dup_idx != 0) {
		if ((dup_idx != -1) || (lastSearchItems.count() == 10)) {
			if (dup_idx == -1)
				dup_idx = lastSearchItems.count() - 1;
			lastSearchItems.remove(pattern_combo->currentText());
			pattern_combo->removeItem(dup_idx);
		}
		lastSearchItems.prepend(pattern_combo->currentText());
		pattern_combo->insertItem(pattern_combo->currentText(), 0);
		config->writeEntry("LastSearchItems", lastSearchItems);
	}

	dup_idx = lastSearchPaths.findIndex(dir_combo->url());
	if (dup_idx != 0) {
		if ((dup_idx != -1) || (lastSearchPaths.count() == 10)) {
			if (dup_idx == -1)
				dup_idx = lastSearchPaths.count() - 1;
			lastSearchPaths.remove(dir_combo->url());
			dir_combo->comboBox()->removeItem(dup_idx);
		}
		lastSearchPaths.prepend(dir_combo->url());
		dir_combo->comboBox()->insertItem(dir_combo->url(), 0);
		config->writeEntry("LastSearchPaths", lastSearchPaths);
	}
}

void KileGrepDialog::childExited()
{
//	 int status = childproc->exitStatus();
	resultbox->unsetCursor();
	clear_button->setEnabled( true );
	search_button->setText( i18n("Search") );

	if ( ! errbuf.isEmpty() )
	{
		KMessageBox::information( parentWidget(),
			i18n("<strong>Error:</strong><p>") + errbuf,
			i18n("Grep tool error") );
		errbuf.truncate(0);
	}
	else
		finish();
}

void KileGrepDialog::receivedOutput(KProcess */*proc*/, char *buffer, int buflen)
{
	buf += QCString(buffer, buflen+1);
	processOutput();
}

void KileGrepDialog::receivedErrOutput(KProcess */*proc*/, char *buffer, int buflen)
{
	errbuf += QCString( buffer, buflen + 1 );
}

void KileGrepDialog::slotClear()
{
	finish();
	resultbox->clear();
}

void KileGrepDialog::slotCancel()
{
	finish();
	reject();
}

void KileGrepDialog::setDirName(const QString &dir)
{
	QString fixed_dir;
	if (dir.right(0) == "/")
		{ fixed_dir = dir; }
	else { fixed_dir = dir + "/"; }
	dir_combo->setURL(fixed_dir);
	if (dir_combo->comboBox()->text(0) != fixed_dir)
		slotClear();
}

void KileGrepDialog::setFilter(const QString &filter)
{
	filter_list.clear();
	filter_combo->clear();
	if ( (filter != "") )
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

bool KileGrepDialog::eventFilter( QObject *o, QEvent *e )
{
	if ( e->type() == QEvent::KeyPress && (
		((QKeyEvent*)e)->key() == Qt::Key_Return ||
		((QKeyEvent*)e)->key() == Qt::Key_Enter ) )
	{
		if ( pattern_combo->currentText().isEmpty() )
			pattern_combo->setFocus();
		else if ( template_edit->text().isEmpty() )
			template_edit->setFocus();
		else if ( filter_combo->currentText().isEmpty() )
			filter_combo->setFocus();
		else if ( dir_combo->url().isEmpty() )
			dir_combo->setFocus();
		else
			slotSearch();

		return true;
	}

	return QWidget::eventFilter( o, e );
}

#include "kilegrepdialog.moc"
