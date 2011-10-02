/******************************************************************************
  Copyright (C) 2009-2011 by Holger Danielsson (holger.danielsson@versanet.de)
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "pdfdialog.h"

#ifdef OKULARPARSER_AVAILABLE
#include <okular/core/document.h>
#endif

#include <QLabel>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLayout>
#include <QProcess>
#include <QStringList>
#include <QValidator>

#include <QFile>
#include <QTextStream>
#include <QRegExp>

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
#include <KInputDialog>

#include "kdatetime.h"
#include "kileconfig.h"
#include "kiledebug.h"


namespace KileDialog 
{

PdfDialog::PdfDialog(QWidget *parent,
                     const QString &texfilename,const QString &startdir,
                     const QString &latexextensions,
                     KileTool::Manager *manager,
                     KileWidget::LogWidget *log, KileWidget::OutputView *output) :
	KDialog(parent),
	m_startdir(startdir),
	m_manager(manager),
	m_log(log),
	m_output(output),
	m_proc(NULL)
{
	setCaption(i18n("PDF Wizard"));
	setModal(true);
	setButtons(Help | Close | User1);
	setDefaultButton(User1);
	showButtonSeparator(true);

	// determine if a pdffile already exists
	QString pdffilename;
	if(!texfilename.isEmpty()) {
		// working with a pdf document, so we try to determine the LaTeX source file
		QStringList extlist = latexextensions.split(' ');
		for (QStringList::Iterator it = extlist.begin(); it != extlist.end(); ++it) {
			if (texfilename.indexOf((*it), -(*it).length()) >= 0) {
				pdffilename = texfilename.left(texfilename.length() - (*it).length()) + ".pdf";
				if (!QFileInfo(pdffilename).exists())
					pdffilename.clear();
				break;
			}
		}
	}

	// prepare dialog
	QWidget *page = new QWidget(this);
	setMainWidget(page);
	m_PdfDialog.setupUi(page);
	page->setMinimumWidth(500);
	m_PdfDialog.m_pbPrinting->setIcon(KIcon("printer"));
	m_PdfDialog.m_pbAll->setIcon(KIcon("list-add"));

	// setup filenames
	m_PdfDialog.m_edInfile->setFilter(i18n("*.pdf|PDF Files"));
	m_PdfDialog.m_edInfile->lineEdit()->setText(pdffilename);
	m_PdfDialog.m_edOutfile->setFilter(i18n("*.pdf|PDF Files"));
	m_PdfDialog.m_edOutfile->setMode(KFile::File | KFile::LocalOnly );
	m_PdfDialog.m_edOutfile->lineEdit()->setText( getOutfileName(pdffilename) );

	// set an user button to execute the task and icon for help button
	setButtonText(User1, i18n("Re&arrange"));
	setButtonIcon(User1, KIcon("system-run"));
	m_PdfDialog.m_lbParameterIcon->setPixmap(KIconLoader::global()->loadIcon("help-about", KIconLoader::NoGroup, KIconLoader::SizeSmallMedium));

	// init important variables
	m_okular = true;
	m_numpages = 0;
	m_encrypted = false;
	m_pdftk = false;
	m_pdfpages = false;
	m_scriptrunning = false;

	// set data for properties: key/widget
	m_pdfInfoKeys << "title" << "subject" << "author" << "creator" << "producer" << "keywords";

	m_pdfInfoWidget["title"] = m_PdfDialog.m_leTitle;
	m_pdfInfoWidget["subject"] = m_PdfDialog.m_leSubject;
	m_pdfInfoWidget["keywords"] = m_PdfDialog.m_leKeywords;
	m_pdfInfoWidget["author"] = m_PdfDialog.m_leAuthor;
	m_pdfInfoWidget["creator"] = m_PdfDialog.m_leCreator;
	m_pdfInfoWidget["producer"] = m_PdfDialog.m_leProducer;

	m_pdfInfoPdftk["title"] = "Title";
	m_pdfInfoPdftk["subject"] = "Subject";
	m_pdfInfoPdftk["keywords"] = "Keywords";
	m_pdfInfoPdftk["author"] = "Author";
	m_pdfInfoPdftk["creator"] = "Creator";
	m_pdfInfoPdftk["producer"] = "Producer";

	// set data for  permissions: key/widget
	m_pdfPermissionKeys    << Okular::AllowModify << Okular::AllowCopy << Okular::AllowPrint
	                       << Okular::AllowNotes << Okular::AllowFillForms;

	m_pdfPermissionWidgets << m_PdfDialog.m_cbModify << m_PdfDialog.m_cbCopy << m_PdfDialog.m_cbPrinting 
	                       << m_PdfDialog.m_cbAnnotations << m_PdfDialog.m_cbFormFeeds;

	m_pdfPermissionPdftk   << "ModifyContents" << "CopyContents" << "Printing"
	                       << "ModifyAnnotations" << "FillIn";

	// default permissions
	m_pdfPermissionState << false << false  << false  << false  << false;
 
	// check for okular pdf parser
#ifndef OKULARPARSER_AVAILABLE
	m_okular = false;
	KILE_DEBUG() << "working without okular pdf parser";
	m_PdfDialog.tabWidget->removeTab(2);
	m_PdfDialog.tabWidget->removeTab(1);
	m_PdfDialog.m_lbParameterInfo->setTextFormat(Qt::RichText);
#else
	KILE_DEBUG() << "working with okular pdf parser";
#endif

	// init Dialog
	m_PdfDialog.m_cbOverwrite->setChecked(true);
	updateDialog();

	// create tempdir
	m_tempdir = new KTempDir(KStandardDirs::locateLocal("tmp", "pdfwizard/pdf-"));
	KILE_DEBUG() << "tempdir: " << m_tempdir->name() ;

	connect(this, SIGNAL(output(const QString &)), m_output, SLOT(receive(const QString &)));
	connect(m_PdfDialog.m_edInfile->lineEdit(), SIGNAL(textChanged(const QString &)), this, SLOT(slotInputfileChanged(const QString &)));
	
#ifdef OKULARPARSER_AVAILABLE
	connect(m_PdfDialog.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(slotTabwidgetChanged(int)));
	connect(m_PdfDialog.m_pbPrinting, SIGNAL(clicked()), this, SLOT(slotPrintingClicked()));
	connect(m_PdfDialog.m_pbAll, SIGNAL(clicked()), this, SLOT(slotAllClicked()));
#endif

	// find available utilities for this dialog
	executeScript("kpsewhich pdfpages.sty", QString::null, PDF_SCRIPTMODE_TOOLS);
}

PdfDialog::~PdfDialog()
{
	delete m_tempdir;
	delete m_proc;
}

void PdfDialog::initUtilities()
{
	// find pdfpages.sty?  
	m_pdfpages = m_outputtext.contains("pdfpages.sty");

	// additionally look for pdftk 
	m_pdftk = !KStandardDirs::findExe("pdftk").isEmpty();

//m_pdfpages = false;            // <----------- only for testing  HACK
//m_pdftk = false;               // <----------- only for testing  HACK

	KILE_DEBUG() << "Looking for pdf tools: pdftk=" << m_pdftk << " pdfpages.sty=" << m_pdfpages;

#ifndef OKULARPARSER_AVAILABLE
	m_imagemagick = KileConfig::imagemagick();

	// we can't use okular pdf parser and need to find another method to determine the number of pdf pages
	// Kile will use three options before giving up
	if ( m_pdftk )
		m_numpagesMode = PDF_SCRIPTMODE_NUMPAGES_PDFTK;
	else if ( m_imagemagick )
		m_numpagesMode = PDF_SCRIPTMODE_NUMPAGES_IMAGEMAGICK;
	else
		m_numpagesMode = PDF_SCRIPTMODE_NUMPAGES_GHOSTSCRIPT;
#endif
	
	// no pdftk, so properties and permissions are readonly
	if ( ! m_pdftk ) {
		// set readonly properties
		for (QStringList::const_iterator it = m_pdfInfoKeys.constBegin(); it != m_pdfInfoKeys.constEnd(); ++it) {
			m_pdfInfoWidget[*it]->setReadOnly(true);
		}
#ifdef OKULARPARSER_AVAILABLE
		//readonly checkboxes
		for (int i=0; i<m_pdfPermissionKeys.size(); ++i) {
			connect(m_pdfPermissionWidgets.at(i), SIGNAL(clicked(bool)), this, SLOT(slotPermissionClicked(bool)));
		}
#endif
	}

	// if we found at least one utility, we can enable some connections
	if ( m_pdftk || m_pdfpages) {
		connect(m_PdfDialog.m_edOutfile->lineEdit(), SIGNAL(textChanged(const QString &)), this, SLOT(slotOutputfileChanged(const QString &)));
		connect(m_PdfDialog.m_cbOverwrite, SIGNAL(stateChanged(int)), this, SLOT(slotOverwriteChanged(int)));
		connect(m_PdfDialog.m_cbTask, SIGNAL(activated(int)), this, SLOT(slotTaskChanged(int)));
	}

	// setup dialog
	slotInputfileChanged(m_PdfDialog.m_edInfile->lineEdit()->text());
}

// read properties and permissions from the PDF document
void PdfDialog::pdfparser(const QString &filename)
{
#ifdef OKULARPARSER_AVAILABLE

	KUrl url;
	url.setPath(filename);
	KMimeType::Ptr pMime = KMimeType::findByUrl(url);

	Okular::Document *okular = new Okular::Document(this);
	bool open = okular->openDocument(filename,url,pMime); //);
	KILE_DEBUG() << "Parse pdf document: " << filename << " ---> " << open;
	if ( !open )
		return;

	const Okular::DocumentInfo *docinfo = okular->documentInfo ();
	if ( docinfo )  {

		m_PdfDialog.m_lbFormat->setText( docinfo->get("format") );
		m_encrypted = ( docinfo->get("encryption") == i18n("Encrypted") ) ? true : false;
		m_PdfDialog.m_lbEncryption->setText( (m_encrypted) ? i18n("yes") : i18n("no") );

		for (QStringList::const_iterator it = m_pdfInfoKeys.constBegin(); it != m_pdfInfoKeys.constEnd(); ++it) {
			QString value = docinfo->get(*it);
			m_pdfInfo[*it] = value;
			m_pdfInfoWidget[*it]->setText(value);
		}

		setDateTimeInfo(docinfo->get("creationDate"),m_PdfDialog.m_lbCreationDate);
		setDateTimeInfo(docinfo->get("modificationDate"),m_PdfDialog.m_lbModDate);

		for (int i=0; i<m_pdfPermissionKeys.size(); ++i) {
			bool value = okular->isAllowed( (Okular::Permission)m_pdfPermissionKeys.at(i) );
			m_pdfPermissionWidgets.at(i)->setChecked(value);

			if ( !m_pdftk ) {
				m_pdfPermissionState[i] = value;
			}
		}
	}
	else
		KILE_DEBUG() << "   No document info for pdf file available.";

	setNumberOfPages( okular->pages() );
	okular->closeDocument();
	delete okular;
	
#else
	/* Okular pdf parser ist not available:
	 * - we use a brute force method to determine, if this file is encrypted
	 * - then we try to determine the number of pages with 
	 *   - pdftk (always first choice, if installed)
	 *   - imagemagick (second choice)
	 *   - gs (third and last choice)
	 * - if the pdf file is encrypted, pdftk will ask for a password
	 */
	
	// look if the pdf file is encrypted (brute force)
	m_encrypted = readEncryption(filename);
	KILE_DEBUG() << "PDF encryption: " << m_encrypted;
	
	// determine the number of pages of the pdf file 
	determineNumberOfPages(filename,m_encrypted);
	KILE_DEBUG() << "PDF number of pages: " << m_numpages;
#endif
}

void PdfDialog::setNumberOfPages(int numpages)
{
	m_numpages = numpages;
	if (m_numpages > 0) {
		// show all, if the number of pages is known
		m_PdfDialog.tabWidget->widget(0)->setEnabled(true);
		
		QString pages;
		if ( m_encrypted )
			m_PdfDialog.m_lbPages->setText(pages.setNum(m_numpages)+"   "+i18n("(encrypted)"));
		else
			m_PdfDialog.m_lbPages->setText(pages.setNum(m_numpages));
	}
	else {
		// hide all, if the number of pages can't be determined
		m_PdfDialog.tabWidget->widget(0)->setEnabled(false);
		m_PdfDialog.m_lbPages->setText(i18n("Error: unknown number of pages"));
	}
}

#ifndef OKULARPARSER_AVAILABLE
void PdfDialog::determineNumberOfPages(const QString &filename, bool askForPassword)
{
	// determine the number of pages of the pdf file (delegate this task)
	QString command = QString::null;
	QString passwordparam = QString::null;
	int scriptmode = m_numpagesMode;

	if ( scriptmode==PDF_SCRIPTMODE_NUMPAGES_PDFTK && askForPassword ) {
		bool ok;
		QString password = KInputDialog::getText( i18n("PDFTK-Password"),
		                                          i18n("This PDF file is encrypted and 'pdftk' cannot open it.\n"
		                                               "Please enter the password for this PDF file\n or leave it blank to try another method: "),
		                                         QString::null, &ok, this ).trimmed();
		if ( ! password.isEmpty() ) {
			passwordparam = " input_pw " + password;
		}
		else {
			scriptmode = ( m_imagemagick ) ? PDF_SCRIPTMODE_NUMPAGES_IMAGEMAGICK : PDF_SCRIPTMODE_NUMPAGES_GHOSTSCRIPT;
		}
	}
		
	// now take the original or changed mode
	if ( scriptmode == PDF_SCRIPTMODE_NUMPAGES_PDFTK )
		command = "pdftk \"" + filename + "\"" + passwordparam + " dump_data | grep NumberOfPages";
	else if ( scriptmode == PDF_SCRIPTMODE_NUMPAGES_IMAGEMAGICK )
		command = "identify -format \"%n\" \"" + filename + "\"";
	else
		command = "gs -q -c \"(" + filename + ") (r) file runpdfbegin pdfpagecount = quit\"";
	
	// run Process
	KILE_DEBUG() << "execute for NumberOfPages: " << command;
	executeScript(command, m_tempdir->name(), scriptmode);
}

void PdfDialog::readNumberOfPages(int scriptmode, const QString &output)
{
	int numpages = 0;

	bool ok;
	if ( scriptmode == PDF_SCRIPTMODE_NUMPAGES_PDFTK ) {
			KILE_DEBUG() << "pdftk output for NumberOfPages: " << output;
			if ( output.contains("OWNER PASSWORD REQUIRED") ) {
				QString filename = m_PdfDialog.m_edInfile->lineEdit()->text().trimmed();
				determineNumberOfPages(m_PdfDialog.m_edInfile->lineEdit()->text().trimmed(),true);
				return;
			} else {
				QRegExp re("\\d+");
					if ( re.indexIn(output) >= 0) {
						numpages = re.cap(0).toInt(&ok);
					}
			}

	} else {
		QString s = output;
		numpages = s.remove("\n").toInt(&ok);
	}
	
	setNumberOfPages(numpages);
}

bool PdfDialog::readEncryption(const QString &filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;     

	KILE_DEBUG() << "search for encryption ";
	QRegExp re("/Encrypt(\\W|\\s|$)");
	QTextStream in(&file);
	QString line = in.readLine();
	while ( !line.isNull() ) {
		if ( re.indexIn(line) >= 0 ) {
			KILE_DEBUG() << "pdf file is encrypted !!!";
			return  true;
		}
		line = in.readLine();
	}
	return false;
}
#endif

void PdfDialog::setDateTimeInfo(const QString &value, QLabel *label)
{
	KDateTime date = KDateTime::fromString(value,"%:A %d %:B %Y %H:%M:%S");
	QString info = ( value.length() >= 10 ) ? KGlobal::locale()->formatDateTime(date,KLocale::LongDate,KLocale::Seconds)
	                                       : QString::null;

	label->setText(info);
}

void PdfDialog::clearDocumentInfo()
{
	m_numpages = 0;
	m_encrypted = false;
	m_PdfDialog.m_lbPassword->setEnabled(false);
	m_PdfDialog.m_edPassword->setEnabled(false);
	m_PdfDialog.m_edPassword->setText(QString::null);

	for (QStringList::const_iterator it = m_pdfInfoKeys.constBegin(); it != m_pdfInfoKeys.constEnd(); ++it) {
		m_pdfInfoWidget[*it]->setText(QString::null);
	}

	m_PdfDialog.m_lbCreationDate->setText(QString::null);
	m_PdfDialog.m_lbModDate->setText(QString::null);

	for (int i=0; i<m_pdfPermissionKeys.size(); ++i) {
		m_pdfPermissionWidgets.at(i)->setChecked(false);
	}

	m_PdfDialog.m_lbPages->setText(QString::null);
	m_PdfDialog.m_lbFormat->setText(QString::null);
	m_PdfDialog.m_lbEncryption->setText(QString::null);
}

void PdfDialog::updateOwnerPassword(bool infile_exists)
{
	int tabindex = m_PdfDialog.tabWidget->currentIndex();
	bool state = ( infile_exists && (m_encrypted || (!m_encrypted && tabindex==2)) ) ? m_pdftk : false;
	m_PdfDialog.m_lbPassword->setEnabled(state);
	m_PdfDialog.m_edPassword->setEnabled(state);
}

// update dialog widgets
void PdfDialog::updateDialog()
{
	QString infile = m_PdfDialog.m_edInfile->lineEdit()->text().trimmed();
	bool infile_exists = QFile(infile).exists();

	updateOwnerPassword(infile_exists);
	updateTasks();
	updateToolsInfo(); 

	bool pstate = ( m_encrypted ) ? infile_exists && m_pdftk : infile_exists && (m_pdftk || m_pdfpages);
	m_PdfDialog.m_gbParameter->setEnabled(pstate);

	m_PdfDialog.m_gbProperties->setEnabled(infile_exists);
	m_PdfDialog.m_gbPermissions->setEnabled(infile_exists);
	m_PdfDialog.m_lbPrinting->setEnabled(infile_exists);
	m_PdfDialog.m_pbPrinting->setEnabled(infile_exists);
	m_PdfDialog.m_lbAll->setEnabled(infile_exists);
	m_PdfDialog.m_pbAll->setEnabled(infile_exists);

	// and exec button
	QString outfile = m_PdfDialog.m_edOutfile->lineEdit()->text().trimmed();
	bool destination = m_PdfDialog.m_cbOverwrite->isChecked() || m_PdfDialog.m_cbView->isChecked();

	bool state = ( infile_exists && (destination || (!destination && !outfile.isEmpty())) );
	if ( m_PdfDialog.tabWidget->currentIndex() == 0 ) {
		state = state && (m_pdfpages || m_pdftk);
	}
	else {
		state = state && m_pdftk;
	}
	enableButton(User1,state && !m_scriptrunning);
}

// update tools information
void PdfDialog::updateToolsInfo()
{
	QString info; 
	QString newline = ( m_okular ) ? "\n" : "<br>";
	QString password = i18n("A password is necessary to set or change the current settings.");

	int tabindex = m_PdfDialog.tabWidget->currentIndex();
	if (tabindex == 2 ) {
		info = ( m_pdftk ) ? i18n("The permissions of this document can be changed with 'pdftk'.") + newline + password
		                   : i18n("'pdftk' is not available, so no permission can be changed.");
	}
	else if ( tabindex == 1 ) {
		if ( ! m_pdftk ) {
			info = i18n("'pdftk' is not available, so no property can be changed.");
		}
		else {
			info = i18n("The properties of this document can be changed with 'pdftk'.");
			if ( m_encrypted )
				info += newline + password;
		}
	}
	else { // if ( tabindex == 0 )
		if ( m_encrypted ) {
			info = ( m_pdftk ) ? i18n("This input file is encrypted, so only 'pdftk' works.") + newline
			                       + i18n("A password is necessary to rearrange pages.") 
			                   : i18n("This input file is encrypted, but 'pdftk' is not installed.");
		}
		else {
			if ( m_pdftk ) { // not encrypted and pdftk
				info = ( m_pdfpages ) ? i18n("This wizard will use 'pdftk' and the LaTeX package 'pdfpages'.")
				                      : i18n("This wizard will only use 'pdftk' ('pdfpages.sty' is not installed).");
			}
			else {           // not encrypted and not pdftk
				info = ( m_pdfpages ) ? i18n("This wizard will only use the LaTeX package 'pdfpages' ('pdftk' was not found).")
				                      : i18n("This wizard can't work, because no tool was found (see help section).");
			}
		}
	}
	
	QString okularinfo = (m_okular ) ? QString::null : newline + i18n("<i>(Compiled without Okular PDF parser. Not all tasks are available.)</i>");
	info += okularinfo;

	// set info text
	m_PdfDialog.m_lbParameterInfo->setText(info);
}

// it is important to calculate the task index from the combobox index,
// as not all tasks are available, when an utility was not found 
void PdfDialog::updateTasks()
{
	// according to QT 4.4 docu the index of QComboBox might change if adding or removing items
	// but because we populate the QComboBox before we start the dialog, we can use the index here
	int lastindex = m_PdfDialog.m_cbTask->currentIndex();
	QString lasttext = m_PdfDialog.m_cbTask->currentText();

	m_PdfDialog.m_cbTask->clear();
	if (m_pdfpages && !m_encrypted) {                                                // task index
		m_PdfDialog.m_cbTask->addItem(i18n("1 Page + Empty Page --> 2up"));           // 0   PDF_PAGE_EMPTY
		m_PdfDialog.m_cbTask->addItem(i18n("1 Page + Duplicate --> 2up"));            // 1   PDF_PAGE_DUPLICATE
		m_PdfDialog.m_cbTask->addItem(i18n("2 Pages --> 2up"));                       // 2   PDF_2UP
		m_PdfDialog.m_cbTask->addItem(i18n("2 Pages (landscape) --> 2up"));           // 3   PDF_2UP_LANDSCAPE
		m_PdfDialog.m_cbTask->addItem(i18n("4 Pages --> 4up"));                       // 4   PDF_4UP
		m_PdfDialog.m_cbTask->addItem(i18n("4 Pages (landscape) --> 4up"));           // 5   PDF_4UP_LANDSCAPE
	}

	if ( (m_pdfpages && !m_encrypted) || m_pdftk ){
		m_PdfDialog.m_cbTask->addItem(i18n("Select Even Pages"));                     // 6   PDF_EVEN
		m_PdfDialog.m_cbTask->addItem(i18n("Select Odd Pages"));                      // 7   PDF_ODD
		m_PdfDialog.m_cbTask->addItem(i18n("Select Even Pages (reverse order)"));     // 8   PDF_EVEN_REV
		m_PdfDialog.m_cbTask->addItem(i18n("Select Odd Pages (reverse order)"));      // 9   PDF_ODD_REV
		m_PdfDialog.m_cbTask->addItem(i18n("Reverse All Pages"));                     // 10  PDF_REVERSE
		m_PdfDialog.m_cbTask->addItem(i18n("Select Pages"));                          // 11  PDF_SELECT
	}

	if (m_pdfpages && !m_encrypted) {  
		m_PdfDialog.m_cbTask->addItem(i18n("pdfpages: Choose Parameter"));            // 12  PDF_PDFPAGES_FREE 
	}
	if (m_pdftk) {
		m_PdfDialog.m_cbTask->addItem(i18n("pdftk: Choose Parameter"));               // 13  PDF_PDFTK_FREE 
		m_PdfDialog.m_cbTask->addItem(i18n("Apply a background watermark"));          // 14  PDF_PDFTK_BACKGROUND
		m_PdfDialog.m_cbTask->addItem(i18n("Apply a foreground stamp"));              // 15  PDF_PDFTK_STAMP 
	}

	// choose one common task (need to calculate the combobox index)
	int index = m_PdfDialog.m_cbTask->findText(lasttext);
	if ( lastindex==-1 || index==-1 ) {
		index = ( (m_pdftk && !m_pdfpages) || m_encrypted ) ? 5 : PDF_SELECT;
	}
	
	m_PdfDialog.m_cbTask->setCurrentIndex(index);
	slotTaskChanged(index);

	setFocusProxy(m_PdfDialog.m_edInfile);
	m_PdfDialog.m_edInfile->setFocus();
}

QString PdfDialog::getOutfileName(const QString &infile)
{
	return infile.left(infile.length()-4) + "-out" + ".pdf";
}


// calculate task index from comboxbox index (available at this moment: 0..13)
//
// taskindex                 -----------------comboxindex-----------------       
//                           -----not encrypted-----   -----encrypted-----        
//                           both   pdfpages   pdftk          pdftk
// 0..5  (only pdfpages)      0..5     0..5      --             --
// 6-11  (pdfpages+pdftk)     6-11     6..11    0..5           0..5
// 12    (only pdfpages)       12       12       --             --
// 13-15 (only pdftk)        13-15      --      6-8            6-8

int PdfDialog::taskIndex(int index)
{
	if ( (m_pdftk  && !m_pdfpages) || m_encrypted ) {
		if ( index <= 5) {
			return index + 6;
		}
		if ( index == 6 ) {
			return 13;
		}
	}

	return index;
}

void PdfDialog::setPermissions(bool print, bool other)
{
	for (int i = 0; i<m_pdfPermissionKeys.size(); ++i) {
		QCheckBox *box = m_pdfPermissionWidgets.at(i);
		bool state = ( box == m_PdfDialog.m_cbPrinting ) ? print : other;
		box->setChecked(state);
	}
}

//-------------------- slots --------------------

void PdfDialog::slotTabwidgetChanged(int index)
{
	setButtonText(User1, (index == 0) ? i18n("Re&arrange") : i18n("&Update") );
	updateDialog();
}

void PdfDialog::slotPrintingClicked()
{
	if ( m_pdftk ) {
		setPermissions(true, false);
	}
}

void PdfDialog::slotAllClicked()
{
	if ( m_pdftk ) {
		setPermissions(true, true);
	}
}

void PdfDialog::slotPermissionClicked(bool)
{
	for (int i=0; i<m_pdfPermissionKeys.size(); ++i) {
		QCheckBox *box = m_pdfPermissionWidgets.at(i);
		if ( box->isChecked() != m_pdfPermissionState[i] ) {
			box->setChecked( m_pdfPermissionState[i] );
		}
	}
}

void PdfDialog::slotInputfileChanged(const QString &text)
{
	clearDocumentInfo();
	if ( QFile(text).exists() ) { 
		m_PdfDialog.m_edOutfile->lineEdit()->setText( getOutfileName(text) );
		pdfparser(text);
	}

	updateDialog();
}

void PdfDialog::slotOverwriteChanged(int state)
{
	bool checked = (state!=Qt::Checked);
	m_PdfDialog.m_lbOutfile->setEnabled(checked);
	m_PdfDialog.m_edOutfile->setEnabled(checked);

	updateDialog();
}

void PdfDialog::slotOutputfileChanged(const QString &)
{
	updateDialog();
}

void PdfDialog::slotTaskChanged(int index)
{
	int taskindex = taskIndex(index);
	bool state = (taskindex == PDF_SELECT || taskindex == PDF_PDFPAGES_FREE || taskindex == PDF_PDFTK_FREE );
	if ( state ) {
		QString s,labeltext;
		if (taskindex==PDF_SELECT) {
			labeltext = i18n("Pages:");
			s = i18n("Comma separated page list: 1,4-7,9");
			QRegExp re("((\\d+(-\\d+)?),)*\\d+(-\\d+)?");
			m_PdfDialog.m_edParameter->setValidator(new QRegExpValidator(re, m_PdfDialog.m_edParameter));
		}
		else if (taskindex==PDF_PDFTK_FREE) {
			labeltext = i18n("Parameter:");
			s = i18n("All options for 'pdftk'");
			m_PdfDialog.m_edParameter->setValidator(0);
		}
		else { //if (taskindex==PDF_PDFPAGES_FREE) {
			labeltext = i18n("Parameter:");
			s = i18n("All options for 'pdfpages'");
			m_PdfDialog.m_edParameter->setValidator(0);
		}
		m_PdfDialog.m_lbParamInfo->setText(" (" + s + ")");

		m_PdfDialog.m_lbParameter->setText(labeltext);
		m_PdfDialog.m_lbParameter->show();
		m_PdfDialog.m_edParameter->show();
		m_PdfDialog.m_lbParamInfo->show();
	}
	else {
		m_PdfDialog.m_lbParameter->hide();
		m_PdfDialog.m_edParameter->hide();
		m_PdfDialog.m_lbParamInfo->hide();
	}
	
	state = ( taskindex == PDF_PDFTK_BACKGROUND || taskindex == PDF_PDFTK_STAMP );
	if ( state ) {
		m_PdfDialog.m_lbStamp->show();
		m_PdfDialog.m_edStamp->show();
	}
	else {
		m_PdfDialog.m_lbStamp->hide();
		m_PdfDialog.m_edStamp->hide();
	}
	
	if ( taskindex == PDF_PDFTK_BACKGROUND )
		m_PdfDialog.m_edStamp->setWhatsThis(i18n("Applies a PDF watermark to the background of a single input PDF. "
		                                         "Pdftk uses only the first page from the background PDF and applies it to every page of the input PDF. "
		                                         "This page is scaled and rotated as needed to fit the input page.") );
	else if ( taskindex == PDF_PDFTK_STAMP )
		m_PdfDialog.m_edStamp->setWhatsThis( i18n("Applies a foreground stamp on top of the input PDF document's pages. "
		                                          "Pdftk uses only the first page from the stamp PDF and applies it to every page of the input PDF. "
		                                          "This page is scaled and rotated as needed to fit the input page. "
		                                          "This works best if the stamp PDF page has a transparent background.") );


}

// execute commands
void PdfDialog::slotButtonClicked(int button)
{
	int tabindex = m_PdfDialog.tabWidget->currentIndex();

	if ( button == User1 ) {
		switch (tabindex) {
			case 0: if (checkParameter()) 
				        executeAction();
				     break;
			case 1: if (checkProperties())
				        executeProperties();
				     break;
			case 2: if (checkPermissions()) 
				        executePermissions();
				     break;
		}
	}
	else if (button == Help) {
		QString message = i18n("<center>PDF-Wizard</center><br>"
		"This wizard uses 'pdftk' and the LaTeX package 'pdfpages' to"
		"<ul>"
		"<li>rearrange pages of an existing PDF document</li>"
		"<li>read and update documentinfo of a PDF document (only pdftk)</li>"
		"<li>read, set or change some permissions of a PDF document (only pdftk)."
		"A password is necessary to set or change this document settings. "
		"Additionally PDF encryption is done to lock the file's content behind this password.</li>"
		"</ul>"
		"<p>The package 'pdfpages' will only work with non encrypted documents. "
		"'pdftk' can handle both kind of documents, but a password is needed for encrypted files. "
		"If one of 'pdftk' or 'pdfpages' is not available, the possible rearrangements are reduced.</p>"
		"<p><i>Warning:</i> Encryption and a password does not provide any real PDF security. The content "
		"is encrypted, but the key is known. You should see it more as a polite but firm request "
		"to respect the author's wishes.</p>");

#ifndef OKULARPARSER_AVAILABLE
	message += i18n("<p><i>Information: </i>This version of Kile wasn't compiled with Okular PDF parser."
	                "Setting, changing and removing of properties and permissions is not possible.</p>");
#endif
	
		KMessageBox::information(this,message,i18n("PDF Tools"));
	}
	else {
		KDialog::slotButtonClicked(button);
	}
}

void PdfDialog::executeAction()
{
	QString command = buildActionCommand();

	m_log->clear();
	QFileInfo from(m_inputfile);
	QFileInfo to(m_outputfile);

	// output for log window
	QString program = (m_execLatex) ? i18n("LaTeX with 'pdfpages' package") : i18n("pdftk");
	QString msg = i18n("Rearranging PDF file: ") + from.fileName();
	if (!to.fileName().isEmpty())
		msg += " ---> " + to.fileName();
	m_log->printMessage(KileTool::Info, msg, program);

	// some output logs 
	m_output->clear();
	QString s = QString("*****\n")
	              + i18n("***** tool:        ") + program + '\n'
	              + i18n("***** input file:  ") + from.fileName()+ '\n'
	              + i18n("***** output file: ") + to.fileName()+ '\n'
	              + i18n("***** param:       ") + m_param + '\n'
	              + i18n("***** viewer:      ") + ((m_PdfDialog.m_cbView->isChecked()) ? i18n("yes") : i18n("no")) + '\n'
	              + "*****\n";
	emit( output(s) );
	
	// run Process
	executeScript(command, m_tempdir->name(), PDF_SCRIPTMODE_ACTION);
}

void PdfDialog::executeProperties()
{
	// create temporary file
	KTemporaryFile infotemp;
	infotemp.setSuffix(".txt");
	infotemp.setAutoRemove(false);

	if(!infotemp.open()) {
		KILE_DEBUG() << "Could not create tempfile for key/value pairs in QString PdfDialog::executeProperties()" ;
		return;
	}
	QString infofile = infotemp.fileName();

	// create a text file with key/value pairs for pdftk
	QTextStream infostream(&infotemp);
	for (QStringList::const_iterator it = m_pdfInfoKeys.constBegin(); it != m_pdfInfoKeys.constEnd(); ++it) {
		infostream << "InfoKey: " << m_pdfInfoPdftk[*it];
		infostream << "InfoValue: " << m_pdfInfoWidget[*it]->text().trimmed();
	}
	infotemp.close();

	// build command
	QString inputfile = m_PdfDialog.m_edInfile->lineEdit()->text().trimmed();
	QString password =  m_PdfDialog.m_edPassword->text().trimmed();
	QString pdffile = m_tempdir->name() + QFileInfo(m_inputfile).baseName() + "-props.pdf";

	QString param = "\"" + inputfile + "\"";
	if ( m_encrypted )
		param += " input_pw " + password;
	param += " update_info " + infofile + " output \"" + pdffile+ "\"";
	QString command = "pdftk " + param;

	// move destination file
	m_move_filelist.clear();
	m_move_filelist << pdffile << inputfile;
	
	// execute script
	showLogs("Updating properties", inputfile, param);
	executeScript(command, QString::null, PDF_SCRIPTMODE_PROPERTIES);

}

void PdfDialog::executePermissions()
{
	// read permissions
	QString permissions;
	for (int i=0; i<m_pdfPermissionKeys.size(); ++i) {
		if ( m_pdfPermissionWidgets.at(i)->isChecked() )
			permissions += m_pdfPermissionPdftk.at(i) + " ";
	}

	// build command
	QString inputfile = m_PdfDialog.m_edInfile->lineEdit()->text().trimmed();
	QString password =  m_PdfDialog.m_edPassword->text().trimmed();
	QString pdffile = m_tempdir->name() + QFileInfo(m_inputfile).baseName() + "-perms.pdf";

	QString param = "\"" + inputfile + "\"";
	if ( m_encrypted )
		param += " input_pw " + password;
	param += " output \"" + pdffile + "\" encrypt_128bit";
	if ( !permissions.isEmpty() )
		param += " allow " + permissions;
	if ( !m_encrypted )
		param += " owner_pw " + password;
	QString command = "pdftk " + param;

	// move destination file
	m_move_filelist.clear();
	m_move_filelist << pdffile << inputfile;

	// execute script
	showLogs("Updating permissions", inputfile, param);
	executeScript(command, QString::null, PDF_SCRIPTMODE_PERMISSIONS);

}

void PdfDialog::showLogs(const QString &title, const QString &inputfile, const QString &param)
{
	// some info for log widget 
	m_log->clear();
	m_log->printMessage(KileTool::Info,title,"pdftk" );

	// some info for output widget 
	QFileInfo input(inputfile);
	m_output->clear();
	QString s = QString("*****\n")
	              + i18n("***** tool:        ") + "pdftk" + '\n'
	              + i18n("***** input file:  ") + input.fileName()+ '\n'
	              + i18n("***** param:       ") + param + '\n'
	              + "*****\n";
	emit( output(s) );
}

void PdfDialog::executeScript(const QString &command, const QString &dir, int scriptmode)
{
	// delete old KProcess
	if (m_proc)
		delete m_proc;

	m_scriptmode = scriptmode;
	m_outputtext = "";

	m_proc = new KProcess();
	if (!dir.isEmpty())
		m_proc->setWorkingDirectory(dir);
	m_proc->setShellCommand(command);
	m_proc->setOutputChannelMode(KProcess::MergedChannels);
	m_proc->setReadChannel(QProcess::StandardOutput);

	connect(m_proc, SIGNAL(readyReadStandardOutput()),
	        this, SLOT(slotProcessOutput()));
	connect(m_proc, SIGNAL(readyReadStandardError()),
	        this, SLOT(slotProcessOutput()));
	connect(m_proc, SIGNAL(finished(int, QProcess::ExitStatus)),
	        this, SLOT(slotProcessExited(int, QProcess::ExitStatus)));

	KILE_DEBUG() << "=== PdfDialog::runPdfutils() ====================";
	KILE_DEBUG() << "execute '" << command << "'";
	m_scriptrunning = true;
	enableButton(User1,false);
	enableButton(Close,false);
	m_proc->start();
}

void PdfDialog::slotProcessOutput()
{
	m_outputtext += m_proc->readAll();
}


void PdfDialog::slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
	if ( exitStatus != QProcess::NormalExit) {
		if (m_scriptmode != PDF_SCRIPTMODE_TOOLS)
			showError(i18n("An error occurred while executing the task."));
	}
	else {
		bool state = ( exitCode == 0 );
		if ( m_scriptmode == PDF_SCRIPTMODE_TOOLS ) 
			initUtilities();
#ifndef OKULARPARSER_AVAILABLE
		else if ( m_scriptmode==PDF_SCRIPTMODE_NUMPAGES_PDFTK 
			      || m_scriptmode==PDF_SCRIPTMODE_NUMPAGES_IMAGEMAGICK 
			      || m_scriptmode==PDF_SCRIPTMODE_NUMPAGES_GHOSTSCRIPT ) {
				readNumberOfPages(m_scriptmode,m_outputtext);
		}
#endif
		else
			finishPdfAction(state);
	} 
   
	m_scriptrunning = false;
	enableButton(Close,true);
	updateDialog();
}

void PdfDialog::finishPdfAction(bool state)
{
	// output window
	emit( output(m_outputtext) );

	// log window
	QString program = (m_scriptmode==PDF_SCRIPTMODE_ACTION && m_execLatex) ? "LaTeX with 'pdfpages' package" : "pdftk";

	if ( state ) {
			m_log->printMessage(KileTool::Info, "finished", program);

			// should we move the temporary pdf file
			if ( ! m_move_filelist.isEmpty() ) {
				QFile::remove( m_move_filelist[1] );
				QFile::rename( m_move_filelist[0], m_move_filelist[1] );
				KILE_DEBUG() << "move file: " << m_move_filelist[0] << " --->  " << m_move_filelist[1];
			}
			
			// run viewer
			if ( m_PdfDialog.m_cbView->isChecked() )
				runViewer();
			
			// perhaps file properties changed in overwrite mode
			if ( m_PdfDialog.m_cbOverwrite->isChecked() )
				slotInputfileChanged( m_PdfDialog.m_edInfile->lineEdit()->text().trimmed() );
	}
	else {
		QString msg;
		if (m_outputtext.indexOf("OWNER PASSWORD") >= 0 ) {
			msg = i18n("Finished with an error (wrong password)");
		}
		else {
			msg = i18n("Finished with an error");
		}
		m_log->printMessage(KileTool::Error, msg, program);
	}
}

void PdfDialog::runViewer()
{
	m_log->printMessage(KileTool::Info, "Running viewer", "ViewPDF");
	
	// call ViewPDF
	QString cfg = KileTool::configName("ViewPDF", m_manager->config());
	KileTool::View *tool = new KileTool::View("ViewPDF", m_manager, false);
	tool->setFlags(0);
	tool->setSource(m_outputfile);
	m_manager->run(tool,cfg);
	
}

QString PdfDialog::buildActionCommand()
{
	// build action: parameter
	m_execLatex = true;           // default
	m_inputfile = m_PdfDialog.m_edInfile->lineEdit()->text().trimmed();
	m_outputfile = m_PdfDialog.m_edOutfile->lineEdit()->text().trimmed();
	
	switch (taskIndex(m_PdfDialog.m_cbTask->currentIndex())) {
		case PDF_PAGE_EMPTY:     
			m_param = "nup=1x2,landscape,pages=" + buildPageRange(PDF_PAGE_EMPTY);                     
		break;

		case PDF_PAGE_DUPLICATE: 
			m_param = "nup=1x2,landscape,pages=" + buildPageRange(PDF_PAGE_DUPLICATE);                 
		break;

		case PDF_2UP:            
			m_param = "nup=1x2,landscape,pages=1-";                   
		break;

		case PDF_2UP_LANDSCAPE:  
			m_param = "nup=1x2,pages=1-";                         
		break;

		case PDF_4UP:            
			m_param = "nup=2x2,pages=1-";                       
		break;

		case PDF_4UP_LANDSCAPE:  
			m_param = "nup=2x2,landscape,pages=1-";                    
		break;

		case PDF_EVEN:           
			if ( m_pdftk ) {
				m_param = "cat 1-endeven";
				m_execLatex = false;                        
			}                         
			else {
				m_param = buildPageList(true);                         
			}                         
		break;

		case PDF_ODD:            
			if ( m_pdftk ) {
				m_param = "cat 1-endodd";
				m_execLatex = false;
			}
			else {
				m_param = buildPageList(false);
			}
		break;

		case PDF_EVEN_REV:       
			if ( m_pdftk ) {
				m_param = "cat end-1even";
				m_execLatex = false;
			}
			else {
				m_param = buildReversPageList(true);
			}
		break;

		case PDF_ODD_REV:        
			if ( m_pdftk ) {
				m_param = "cat end-1odd";
				m_execLatex = false;
			}
			else {
				m_param = buildReversPageList(false);
			}
		break;

		case PDF_REVERSE:        
			if ( m_pdftk ) {
				m_param = "cat end-1";
				m_execLatex = false;
			}
			else {
				m_param = "last-1";
			}
		break;

		case PDF_SELECT:         
			m_param = m_PdfDialog.m_edParameter->text().trimmed();
			if ( m_pdftk ) {
				m_param = "cat " + m_param.replace(","," ");
				m_execLatex = false;
			}
			else {
				m_param = "pages={" + m_param + "}";
			}
		break;

		case PDF_PDFPAGES_FREE:  
			m_param = m_PdfDialog.m_edParameter->text().trimmed();                         
		break;

		case PDF_PDFTK_FREE:     
			m_param = m_PdfDialog.m_edParameter->text().trimmed();
			m_execLatex = false;                        
		break;
		
		case PDF_PDFTK_BACKGROUND:     
			m_param = "background " + m_PdfDialog.m_edStamp->text().trimmed();
			m_execLatex = false;                        
		break;
		
		case PDF_PDFTK_STAMP:     
			m_param = "stamp " + m_PdfDialog.m_edStamp->text().trimmed();
			m_execLatex = false;                        
		break;
	}

	// build action: command
	QString command,latexfile,pdffile;
	if ( m_execLatex ) {
		latexfile = buildLatexFile(m_param);
		pdffile = latexfile + ".pdf";
		command = "pdflatex " + latexfile + ".tex";
	}
	else {
		pdffile = m_tempdir->name() + QFileInfo(m_inputfile).baseName() + "-temp.pdf";
		command = "pdftk \"" + m_inputfile + "\"";
		if ( m_encrypted ) {
			QString password =  m_PdfDialog.m_edPassword->text().trimmed();
			command += " input_pw " + password;
		}
		command += " " + m_param + " output \"" + pdffile+ "\"";
	}

	// additional actions
	bool viewer = m_PdfDialog.m_cbView->isChecked();
 
	bool equalfiles = (m_PdfDialog.m_cbOverwrite->isChecked() || m_inputfile==m_outputfile);
	if (equalfiles)
		 m_outputfile = m_inputfile;

	// move destination file
	m_move_filelist.clear();
	if ( equalfiles ) {
		m_move_filelist << pdffile << m_inputfile;
	}
	else if ( !m_outputfile.isEmpty() ) {
		m_move_filelist << pdffile << m_outputfile;
	}

	// viewer
	if ( viewer && m_outputfile.isEmpty() )
		m_outputfile = pdffile;

	return command;
}

// create a temporary file to run latex with package pdfpages.sty
QString PdfDialog::buildLatexFile(const QString &param)
{
	KTemporaryFile temp;
	temp.setPrefix(m_tempdir->name());
	temp.setSuffix(".tex");
	temp.setAutoRemove(false);

	if(!temp.open()) {
		KILE_DEBUG() << "Could not create tempfile in PdfDialog::buildLatexFile()" ;
		return QString();
	}
	QString tempname = temp.fileName();
	
	QTextStream stream(&temp);
	stream << "\\documentclass[a4paper,12pt]{article}";
	stream << "\\usepackage[final]{pdfpages}";
	stream << "\\begin{document}";
	stream << "\\includepdf[" << param << "]{" << m_inputfile << "}";
	stream << "\\end{document}";

	// everything is prepared to do the job
	temp.close();
	return(tempname.left(tempname.length()-4));
}

QString PdfDialog::buildPageRange(int type)
{
	QString s;
	for (int i = 1; i <= m_numpages; ++i) {
		if (type == PDF_PAGE_EMPTY)
			s += QString("%1,{},").arg(i);
		else
			s += QString("%1,%2,").arg(i).arg(i);
	}

	return "{" + s.left(s.length()-1) + "}";    
}

QString PdfDialog::buildPageList(bool even)
{
	QString s,number;

	int start = ( even ) ? 2 : 1;
	for (int i=start; i<=m_numpages; i+=2 )
		s += number.setNum(i) + ",";

	if ( !s.isEmpty() )
		s.truncate(s.length()-1);
	return "{" + s + "}";    
}

QString PdfDialog::buildReversPageList(bool even)
{
	QString s,number;

	int last = m_numpages;
	if ( even ) {
		if ( (last & 1) == 1 )
			last--;
	}
	else {
		if ( (last & 1) == 0 )
			last--;		
	}

	for (int i=last; i>=1; i-=2 )
		s += number.setNum(i) + ",";

	if ( !s.isEmpty() )
		s.truncate(s.length()-1);
	return "{" + s + "}";    
}

bool PdfDialog::checkParameter()
{
	if ( !checkInputFile() )
		return false;

	if ( m_encrypted ) {
		if ( !checkPassword() )
			return false;
	}

	// check parameter
	int taskindex = taskIndex(m_PdfDialog.m_cbTask->currentIndex());
	if ( taskindex>=PDF_SELECT && taskindex<=PDF_PDFTK_FREE && m_PdfDialog.m_edParameter->text().trimmed().isEmpty() ) {
		showError( i18n("The utility needs some parameters in this mode.") );
		return false;
	}

	// check background/stamp parameter
	if ( taskindex==PDF_PDFTK_BACKGROUND || taskindex==PDF_PDFTK_STAMP ) {
		QString stampfile = m_PdfDialog.m_edStamp->text().trimmed();
		
		if ( stampfile.isEmpty() ) {
			showError(i18n("You need to define an PDF file as image in this mode."));
			return false;
		}
	
		QFileInfo fs(stampfile);
		if (fs.completeSuffix() != "pdf") {
			showError(i18n("Unknown file format: only '.pdf' is accepted as image file in this mode."));
			return false;
		}
	}
	
	// overwrite mode: no output file is needed
	if ( m_PdfDialog.m_cbOverwrite->isChecked() )
		return true;  

	// create a different output file
	QString outfile = m_PdfDialog.m_edOutfile->lineEdit()->text().trimmed();
	if ( outfile.isEmpty() ) {
		showError(i18n("You need to define an output file."));
		return false;
	}

	// outfile file must have extension pdf 
	QFileInfo fo(outfile);
	if (fo.completeSuffix() != "pdf") {
		showError(i18n("Unknown file format: only '.pdf' is accepted as output file."));
		return false;
	}
	
	// check, if this output file already exists
	if ( fo.exists() ) {
		QString s = i18n("A file named \"%1\" already exists. Are you sure you want to overwrite it?", fo.fileName());
		if (KMessageBox::questionYesNo(this,
		                               "<center>" + s + "</center>",
		                               i18n("PDF Tools")) == KMessageBox::No) {
			return false;
		}
	}

	return true;
}

bool PdfDialog::checkProperties()
{
	if ( !checkInputFile() ) {
		return false;
	}

	return ( m_encrypted ) ? checkPassword() : true;
}

bool PdfDialog::checkPermissions()
{
	if ( !checkInputFile() ) {
		return false;
	}

	return checkPassword();
}

bool PdfDialog::checkInputFile()
{
	QString infile = m_PdfDialog.m_edInfile->lineEdit()->text().trimmed();
	if (infile.isEmpty()) {
		showError(i18n("No input file is given."));
		return false;
	}

	QFileInfo fi(infile);
	QString suffix = fi.completeSuffix();
	if (suffix != "pdf") {
		showError(i18n("Unknown file format: only '.pdf' are accepted for input files."));
		return false;
	}

	if (!fi.exists()) {
		showError(i18n("This input file does not exist."));
		return false;
	}

	return true;
}

bool PdfDialog::checkPassword()
{
	// check password
	QString password = m_PdfDialog.m_edPassword->text().trimmed();
	if (password.isEmpty()) {
		showError(i18n("No password is given."));
		return false;
	}
	
	if (password.length() < 6) {
		showError(i18n("The password should be at least 6 characters long."));
		return false;
	}

	return true;
}

void PdfDialog::showError(const QString &text)
{
	KMessageBox::error(this, i18n("<center>") + text + i18n("</center>"), i18n("PDF Tools"));
}

}

#include "pdfdialog.moc"
