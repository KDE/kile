/***************************************************************************
    date                 : Mar 12 2007
    version              : 0.20
    copyright            : (C) 2005-2007 by Holger Danielsson
    email                : holger.danielsson@versanet.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// 2007-03-12 dani
//  - use KileDocument::Extensions

#include "postscriptdialog.h"

#include <QLabel>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLayout>
#include <QProcess>
#include <QSpinBox>
#include <QStringList>

#include <KComboBox>
#include <KFileDialog>
#include <KIconLoader>
#include <KLineEdit>
#include <KLocale>
#include <KMessageBox>
#include <KProcess>
#include <KStandardDirs>
#include <KTemporaryFile>
#include <KUrlRequester>

#include "kiledebug.h"
#include "kiletool_enums.h"

namespace KileDialog
{

PostscriptDialog::PostscriptDialog(QWidget *parent,
                                   const QString &texfilename,const QString &startdir,
                                   const QString &latexextensions,
                                   KileWidget::LogMsg *log,KileWidget::Output *output) :
	KDialog(parent),
	m_startdir(startdir),
	m_log(log),
	m_output(output),
	m_proc(0)
{	
	setCaption(i18n("Rearrange Postscript File"));
	setModal(true);
	setButtons(Close | User1);
	setDefaultButton(User1);
	showButtonSeparator(true);

	// determine if a psfile already exists
	QString psfilename;
	if ( !texfilename.isEmpty() ) {
		// working with a postscript document, so we try to determine the LaTeX source file
		QStringList extlist = latexextensions.split(" ");
		for (QStringList::Iterator it = extlist.begin(); it != extlist.end(); ++it) {
			if (texfilename.indexOf((*it), -(*it).length()) >= 0) {
				psfilename = texfilename.left(texfilename.length() - (*it).length()) + ".ps";
				if (!QFileInfo(psfilename).exists())
					psfilename = QString::null;
				break;
			}
		}
	}

	// prepare dialog
	QWidget *page = new QWidget(this);
	setMainWidget(page);

	// Layout
	QVBoxLayout *vbox = new QVBoxLayout();
	vbox->setMargin(0);
	vbox->setSpacing(KDialog::spacingHint());
	page->setLayout(vbox);

	// groupbox with file selection
	QGroupBox* group = new QGroupBox(i18n("Parameter"), page);
	QGridLayout *grid = new QGridLayout;
	grid->setMargin(KDialog::marginHint());
	grid->setSpacing(KDialog::spacingHint());
	group->setLayout(grid);

	// line 0: QLabel
	bool pstops = !KStandardDirs::findExe("pstops").isNull();
	bool psselect = !KStandardDirs::findExe("psselect").isNull();

	QString title = i18n("Conversion of ps files is made by 'pstops' and 'psselect'.\nBe sure to call 'dvips' with option '-t a4' and\nhyperref package (if needed) with option 'a4paper'.");

	if (!pstops || !psselect) {
		QString msg = QString::null;
		if (!pstops) { 
			msg = "'pstops'";
			if (!psselect)
				msg += " and ";
		}
		if (!psselect) {
			msg += "'psselect'";
		}
		title += "\n(Error: " + msg + " not found.)";
	}

	QLabel *label1 = new QLabel(title, group);
	label1->setAlignment(Qt::AlignHCenter);
	grid->addWidget(label1, 0, 0, 1, 2, Qt::AlignCenter);

	// empty line
	grid->setRowSpacing(1, 10);
	
	// line 1: QLabel
	QLabel *label2 = new QLabel(i18n("Input file:"), group);
	grid->addWidget(label2, 2, 0);

	// line 1: QLineEdit
	m_edInfile = new KUrlRequester(group);
	m_edInfile->setFilter(i18n("*.ps|PS Files\n*.ps.gz|Zipped PS Files"));
	m_edInfile->lineEdit()->setText(psfilename);
	grid->addWidget(m_edInfile, 2, 1);

	// line 2: QLabel
	QLabel *label3 = new QLabel(i18n("Output file:"), group);
	grid->addWidget(label3, 3, 0);

	// line 2: QLineEdit
	m_edOutfile = new KUrlRequester(group);
	m_edOutfile->setFilter(i18n("*.ps|PS Files\n*.ps.gz|Zipped PS Files"));
	grid->addWidget(m_edOutfile, 3, 1);

	// line 3: task selection
	QLabel *label4 = new QLabel(i18n("Task:"), group);
	grid->addWidget(label4, 4, 0);

	// line 3: predefined tasks
	m_cbTask = new KComboBox(false, group);
	if (pstops) {
		m_cbTask->insertItem(i18n("1 DIN A5 Page + Empty Page --> DIN A4"));      // 0   PS_A5_EMPTY
		m_cbTask->insertItem(i18n("1 DIN A5 Page + Duplicate --> DIN A4"));       // 1   PS_A5_DUPLICATE
		m_cbTask->insertItem(i18n("2 DIN A5 Pages --> DIN A4"));                  // 2   PS_2xA5
		m_cbTask->insertItem(i18n("2 DIN A5L Pages --> DIN A4"));                 // 3   PS_2xA5L
		m_cbTask->insertItem(i18n("4 DIN A5 Pages --> DIN A4"));                  // 4   PS_4xA5
		m_cbTask->insertItem(i18n("1 DIN A4 Page + Empty Page --> DIN A4"));      // 5   PS_A4_EMPTY
		m_cbTask->insertItem(i18n("1 DIN A4 Page + Duplicate --> DIN A4"));       // 6   PS_A4_DUPLICATE
		m_cbTask->insertItem(i18n("2 DIN A4 Pages --> DIN A4"));                  // 7   PS_2xA4
		m_cbTask->insertItem(i18n("2 DIN A4L Pages --> DIN A4"));                 // 8   PS_2xA4L
	}
	if (psselect) {
		m_cbTask->insertItem(i18n("Select Even Pages"));                          // 9   PS_EVEN
		m_cbTask->insertItem(i18n("Select Odd Pages"));                           // 10  PS_ODD
		m_cbTask->insertItem(i18n("Select Even Pages (reverse order)"));          // 11  PS_EVEN_REV
		m_cbTask->insertItem(i18n("Select Odd Pages (reverse order)"));           // 12  PS_ODD_REV
		m_cbTask->insertItem(i18n("Reverse All Pages"));                          // 13  PS_REVERSE
		m_cbTask->insertItem(i18n("Copy All Pages (sorted)"));                    // 14  PS_COPY_SORTED
	}
	if (pstops) {
		m_cbTask->insertItem(i18n("Copy All Pages (unsorted)"));                  // 15  PS_COPY_UNSORTED
		m_cbTask->insertItem(i18n("pstops: Choose Parameter"));                   // 16  PS_PSTOPS_FREE 
	}
	if (psselect) {
		m_cbTask->insertItem(i18n("psselect: Choose Parameter"));                 // 17  PS_PSSELECT_FREE 
	}
	grid->addWidget(m_cbTask, 4, 1);

	// line 4: QLabel (parameter or copies)
	m_lbParameter = new QLabel(i18n("Parameter:"), group);
	grid->addWidget(m_lbParameter, 5, 0);

	// line 4: QLineEdit or QSpinBox
	m_edParameter = new KLineEdit("", group);
	m_edParameter->setMinimumWidth(300);
	grid->addWidget(m_edParameter, 5, 1);
	m_spCopies = new QSpinBox(group);
	m_spCopies->setValue(1);
	m_spCopies->setRange(1, 99);
	grid->addWidget(m_spCopies, 5, 1);

	// choose one common task
	m_cbTask->setCurrentIndex(PS_2xA4);
	comboboxChanged(PS_2xA4);

	// line 5: QLabel
	QLabel *label6 = new QLabel(i18n("Viewer:"), group);
	grid->addWidget(label6, 6, 0);
	
	// line 5: QCheckBox
	m_cbView = new QCheckBox(i18n("Show ps file with 'okular'"), group);
	m_cbView->setChecked(true);
	grid->addWidget(m_cbView, 6, 1);

	// build Layout
	vbox->addWidget(group);
	vbox->addStretch();

	// set an user button to execute the task
	setButtonText(Close, i18n("Done"));
	setButtonText(User1, i18n("Execute"));
	setButtonIcon(User1, KIcon("gear"));
	if (!pstops && !psselect)
		enableButton(User1, false);

	m_cbTask->setWhatsThis(i18n("Choose one of the 18 operations to convert a postscript file. The last four operations need specific parameters."));
	m_edInfile->setWhatsThis(i18n("Input file, which should be converted."));
	m_edOutfile->setWhatsThis(i18n("The name of the output file. This entry may also be empty, if you only want to view the result without saving it. In this case the viewer checkbox must be checked."));
	m_edParameter->setWhatsThis(i18n("'Select pages' and 'Free Parameter' need some specific parameter, which you can enter here"));
	m_spCopies->setWhatsThis(i18n("When you want to copy pages, you must enter the number of copies"));
	m_cbView->setWhatsThis(i18n("View the result of the conversion process. okular is always taken as an external viewer."));

	// some connections
	connect(m_cbTask,SIGNAL(activated(int)),this, SLOT(comboboxChanged(int)));
	connect(this, SIGNAL(output(const QString &)), m_output, SLOT(receive(const QString &)));

	setFocusProxy(m_edInfile);
}

PostscriptDialog::~PostscriptDialog()
{
	if (m_proc)
		delete m_proc;
}

void PostscriptDialog::slotButtonClicked(int button)
{
	if (button == User1) {
		if (checkParameter()) {
			execute();
		}
	}
	KDialog::slotButtonClicked(button);
}

void PostscriptDialog::execute()
{
	m_tempfile = buildTempfile();
	if ( m_tempfile != QString::null ) {
		m_log->clear();
		QFileInfo from(m_edInfile->lineEdit()->text());
		QFileInfo to(m_edOutfile->lineEdit()->text());

		// output for log window
		QString msg = i18n("rearrange ps file: ") + from.fileName();
		if (!to.fileName().isEmpty())
			msg += " ---> " + to.fileName();
		m_log->printMsg(KileTool::Info, msg, m_program);

		// some output logs 
		m_output->clear();
		QString s = QString("*****\n")
		              + i18n("***** tool:        ") + m_program + ' ' + m_param + '\n'
		              + i18n("***** input file:  ") + from.fileName()+ '\n'
		              + i18n("***** output file: ") + to.fileName()+ '\n'
		              + i18n("***** viewer:      ") + ((m_cbView->isChecked()) ? i18n("yes") : i18n("no")) + '\n'
		              + "*****\n";
		emit( output(s) );
 
		// delete old KProcess
		if (m_proc)
			delete m_proc;

		m_proc = new KProcess();
		m_proc->setShellCommand("sh " + m_tempfile);
		m_proc->setOutputChannelMode(KProcess::MergedChannels);
		m_proc->setReadChannel(QProcess::StandardOutput);

		connect(m_proc, SIGNAL(readyReadStandardOutput()),
		        this, SLOT(slotProcessOutput()));
		connect(m_proc, SIGNAL(readyReadStandardError()),
		        this, SLOT(slotProcessOutput()));
		connect(m_proc, SIGNAL(finished(int, QProcess::ExitStatus)),
		        this, SLOT(slotProcessExited(int, QProcess::ExitStatus)));

		KILE_DEBUG() << "=== PostscriptDialog::runPsutils() ====================";
		KILE_DEBUG() << "   execute '" << m_tempfile << "'";
		m_proc->start();
	}
	
}

void PostscriptDialog::slotProcessOutput()
{
	emit(output(m_proc->readAllStandardOutput()));
	emit(output(m_proc->readAllStandardError()));
}


void PostscriptDialog::slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
	if (exitStatus != QProcess::NormalExit)
		showError(i18n("An error occurred, while rearranging the file."));

	QFile::remove(m_tempfile);
}

#ifdef __GNUC__
#warning FIXME: redesign the method buildTempfile(). It won't work correctly like it is now!
#endif
QString PostscriptDialog::buildTempfile()
{
	// build command
	m_program = "pstops";          // default
	m_param = "";

	switch (m_cbTask->currentItem()) {
		case PS_A5_EMPTY:      m_param = "1:0L(29.7cm,0cm)";
		                       break;
		case PS_A5_DUPLICATE:  m_param = "1:0L(29.7cm,0cm)+0L(29.7cm,14.85cm)";
		                       break;
		case PS_2xA5:          m_param = "2:0L(29.7cm,0cm)+1L(29.7cm,14.85cm)";
		                       break;
		case PS_2xA5L:         break;
		case PS_4xA5:          m_param = "4:0@0.7(0cm,8.7cm)"
		                                 "+1@0.7(10.5cm,8.7cm)"
		                                 "+2@0.7(0cm,-6.15cm)"
		                                 "+3@0.7(10.5cm,-6.15cm)";
		                       break;
		case PS_A4_EMPTY:      m_param = "1:0L@0.7(21cm,0cm)";
		                       break;
		case PS_A4_DUPLICATE:  m_param = "1:0L@0.7(21cm,0cm)+0L@0.7(21cm,14.85cm)";
		                       break;
		case PS_2xA4:          m_param = "2:0L@0.7(21cm,0cm)+1L@0.7(21cm,14.85cm)";
		                       break;
		case PS_2xA4L:         m_param = "2:0R@0.7(0cm,29.7cm)+1R@0.7(0cm,14.85cm)";
		                       break;
		case PS_EVEN:          m_program = "psselect";
		                       m_param = "-e";
		                       break;
		case PS_ODD:           m_program = "psselect";
		                       m_param = "-o";
		                       break;
		case PS_EVEN_REV:      m_program = "psselect";
		                       m_param = "-e -r";
		                       break;
		case PS_ODD_REV:       m_program = "psselect";
		                       m_param = "-o -r";
		                       break;
		case PS_REVERSE:       m_program = "psselect";
		                       m_param = "-r";
		                       break;
		case PS_COPY_SORTED:   m_program = "psselect";
		                       m_param = "-p" + duplicateParameter("1-");
		                       break;
		case PS_COPY_UNSORTED: m_param = "1:" + duplicateParameter("0");
		                       break;
		case PS_PSTOPS_FREE:   m_param = m_edParameter->text();
		                       break;
		case PS_PSSELECT_FREE: m_program = "psselect";
		                       m_param = m_edParameter->text();
		                       break;
	}

	// create a temporary file
	KTemporaryFile temp;
	temp.setSuffix(".sh");
	temp.setAutoRemove(false);
	if(!temp.open()) {
#ifdef __GNUC__
#warning FIXME: add error handling
#endif
	}
	QString tempname = temp.name();
	
	QTextStream stream(&temp);
	stream << "#! /bin/sh" << endl;

	// accept only ".ps" or ".ps.gz" as an input file
	QFileInfo fi(m_edInfile->lineEdit()->text());
	bool zipped_psfile = (fi.completeSuffix() == "ps.gz") ? true : false;

	// there are four possible cases
	//         outfile view
	//     1)    +      +        pstops/psselect + okular
	//     2)    +      -        pstops/psselect
	//     3)    -      +        pstops/psselect | okular (nur Shell)
	//     4)    -      -        error (already detected by checkParameter())

	// some files, which are used
	QString command    = m_program + " \"" + m_param + "\"";
	QString inputfile  = "\"" + m_edInfile->lineEdit()->text() + "\"";
	QString outputfile = "\"" + m_edOutfile->lineEdit()->text() + "\"";
	bool viewer = m_cbView->isChecked();
	
	bool equalfiles = false;
	if (inputfile == outputfile) {
		outputfile = tempname + ".tmp";
		equalfiles = true;
	}
	
	if (!zipped_psfile) {                                       // unzipped ps files
		if (m_edOutfile->lineEdit()->text().isEmpty()) {                      // pstops/psselect | okular
			stream << command << " " << inputfile << " | okular -" << endl;  
			viewer = false;
		} else {                                                    // pstops/psselect
			stream << command << " " << inputfile << " " << outputfile << endl;
		}
	} else {                                                      // zipped ps files
		if ( m_edOutfile->lineEdit()->text().isEmpty() ) {                     // pstops/psselect | okular
			stream << "gunzip -c " << inputfile
			       << " | " << command
			       << " | okular -"
			       << endl;
			viewer = false;
		} else {
			stream << "gunzip -c " << inputfile                    // pstops/psselect
			       << " | " << command
			       << " > " << outputfile
			       << endl;
		}
	}

	// check, if we should stop
	if ( equalfiles || viewer ) {
		stream << "if [ $? != 0 ]; then" << endl;
		stream << "   exit 1" << endl;
		stream << "fi" << endl;
	}

	// replace the original file
	if ( equalfiles ) {
		stream << "rm " << inputfile << endl;
		stream << "mv " << outputfile << " " << inputfile << endl;
	}

	// viewer
	if ( viewer ) {                                                // viewer: okular
		stream << "okular" << " " 
		       << ((equalfiles) ? inputfile : outputfile) << endl;
	}

	// everything is prepared to do the job
	temp.close();

	return(tempname);
}

QString PostscriptDialog::duplicateParameter(const QString &param)
{
	QString s = QString::null;
	for (int i = 0; i < m_spCopies->value(); ++i) {
		if (i == 0)
			s += param;
		else
			s += ',' + param;
	}

	return s;
}


bool PostscriptDialog::checkParameter()
{
	QString infile = m_edInfile->lineEdit()->text();
	if (infile.isEmpty()) {
		showError(i18n("No input file is given."));
		return false;
	}

	QFileInfo fi(infile);
	QString suffix = fi.completeSuffix();
	if (suffix != "ps" && suffix != "ps.gz") {
		showError(i18n("Unknown file format: only '.ps' and '.ps.gz' are accepted for input files."));
		return false;
	}

	if (!fi.exists()) {
		showError(i18n("This input file does not exist."));
		return false;
	}

	// check parameter
	int index = m_cbTask->currentItem();
	if (m_edParameter->text().isEmpty()) {
		if (index == PS_PSSELECT_FREE) {
			showError( i18n("psselect needs some parameters in this mode.") );
			return false;
		} else if (index == PS_PSTOPS_FREE) {
			showError( i18n("pstops needs some parameters in this mode.") );
			return false;
		}
	}

	QString outfile = m_edOutfile->lineEdit()->text();
	if (outfile.isEmpty() && !m_cbView->isChecked()) {
		showError(i18n("You need to define an output file or select the viewer."));
		return false;
	}

	if (! outfile.isEmpty()) {
		QFileInfo fo(outfile);
		if (fo.completeSuffix() != "ps") {
			showError(i18n("Unknown file format: only '.ps' is accepted as output file."));
			return false;
		}
	
		if (infile != outfile && fo.exists()) {
			QString s = i18n("A file named \"%1\" already exists. Are you sure you want to overwrite it?", fo.fileName());
			if (KMessageBox::questionYesNo(this,
			                               "<center>" + s + "</center>",
			                               "Postscript tools") == KMessageBox::No) {
				return false;
			}
		}
	}
	
	return true;
}

void PostscriptDialog::comboboxChanged(int index)
{
	KILE_DEBUG() << index << endl;
	if (index == PS_COPY_SORTED || index == PS_COPY_UNSORTED) {
		m_lbParameter->setEnabled(true);
		m_lbParameter->setText(i18n("Copies:"));
		m_edParameter->hide();
		m_spCopies->show();
		m_spCopies->setEnabled(true);
	} else {
		if (index == PS_PSSELECT_FREE || index == PS_PSTOPS_FREE) {
			m_lbParameter->setEnabled(true);
			m_lbParameter->setText(i18n("Parameter:"));
			m_spCopies->hide();
			m_edParameter->show();
			m_edParameter->setEnabled(true);
		} else {
			m_lbParameter->setEnabled(false);
			m_edParameter->setEnabled(false);
			m_spCopies->setEnabled(false);
		}
	}
}

void PostscriptDialog::showError(const QString &text)
{
	KMessageBox::error(this, i18n("<center>") + text + i18n("</center>"), i18n("Postscript Tools"));
}

}

#include "postscriptdialog.moc"
