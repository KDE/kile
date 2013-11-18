/************************************************************************************************
                         texdocdialog.cpp
                         ----------------
    date                 : Feb 15 2007
    version              : 0.14
    copyright            : (C) 2005-2007 by Holger Danielsson (holger.danielsson@versanet.de)
 ************************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dialogs/texdocumentationdialog.h"

#include <QBoxLayout>
#include <QDesktopWidget>
#include <QEvent>
#include <QFile>
#include <QFileInfo>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QRegExp>
#include <QTextStream>
#include <QTreeWidget>

#include <KApplication>
#include <KIconLoader>
#include <KLineEdit>
#include <KLocale>
#include <KMessageBox>
#include <KMimeType>
#include <KMimeTypeTrader>
#include <KProcess>
#include <KPushButton>
#include <KRun>
#include <KService>
#include <KTemporaryFile>
#include <KUrl>

#include "kileconstants.h"
#include "kiledebug.h"

namespace KileDialog
{

TexDocDialog::TexDocDialog(QWidget *parent)
		: KDialog(parent), m_tempfile(NULL), m_proc(NULL)
{
	setCaption(i18n("Documentation Browser"));
	setModal(true);
	setButtons(Close | Default);
	setDefaultButton(NoDefault);
	showButtonSeparator(true);

	QWidget *page = new QWidget(this);
	setMainWidget(page);

	QBoxLayout *vbox = new QBoxLayout(QBoxLayout::TopToBottom, page);

	// listview
	m_texdocs = new QTreeWidget(page);
	m_texdocs->setRootIsDecorated(true);
	m_texdocs->setHeaderLabel(i18n("Table of Contents"));

	// groupbox
	QGroupBox *groupbox = new QGroupBox(i18n("Search"), page);
	QHBoxLayout *groupboxLayout = new QHBoxLayout();
	groupboxLayout->setAlignment(Qt::AlignTop);
	groupboxLayout->setMargin(KDialog::marginHint());
	groupboxLayout->setSpacing(KDialog::spacingHint());
	groupbox->setLayout(groupboxLayout);

	m_leKeywords = new KLineEdit("", groupbox);
	m_leKeywords->setClickMessage("Keyword");
	m_leKeywords->setClearButtonShown(true);
	m_pbSearch = new KPushButton(i18n("&Search"), groupbox);

	groupboxLayout->addWidget(m_leKeywords);
	groupboxLayout->addWidget(m_pbSearch);

	m_texdocs->setWhatsThis(i18n("A list of available documents, which are listed in 'texdoctk.dat', that come with TexLive/teTeX. Double clicking with the mouse or pressing the space key will open a viewer to show this file."));
	m_leKeywords->setWhatsThis(i18n("You can choose a keyword to show only document files that are related to this keyword."));
	m_pbSearch->setWhatsThis(i18n("Start the search for the chosen keyword."));
	button(Default)->setWhatsThis(i18n("Reset TOC to show all available files."));

	setButtonText(Default, i18n("Reset &TOC"));
	m_pbSearch->setEnabled(false);
	enableButton(Default, false);

	// catch some Return/Enter events
	m_texdocs->installEventFilter(this);
	m_leKeywords->installEventFilter(this);

	connect(m_texdocs, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
	        this, SLOT(slotListViewDoubleClicked(QTreeWidgetItem*)));
	connect(m_pbSearch, SIGNAL(clicked()), this, SLOT(slotSearchClicked()));
	connect(m_leKeywords, SIGNAL(textChanged(const QString &)), this, SLOT(slotTextChanged(const QString &)));

	m_texmfPath.clear();
	m_texmfdocPath.clear();
	m_texdoctkPath.clear();

	connect(this, SIGNAL(processFinished()), this, SLOT(slotInitToc()));
	executeScript(
	  "kpsewhich --progname=texdoctk --format='other text files' texdoctk.dat && "
	  "kpsewhich --expand-path='$TEXMF/doc' && "
	  "kpsewhich --expand-path='$TEXMF'"
	);
	
	vbox->addWidget(m_texdocs);
	vbox->addWidget(groupbox);

	resize(sizeHint() + m_texdocs->sizeHint());
}

TexDocDialog::~TexDocDialog()
{
	delete m_proc;
	delete m_tempfile;
}

////////////////////// TOC //////////////////////

void TexDocDialog::readToc()
{
	// open to read
	QFile fin(m_texdoctkPath);
	if(!fin.exists() || !fin.open(QIODevice::ReadOnly)) {
		KMessageBox::error(this, i18n("Could not read 'texdoctk.dat'."));
		return;
	}

	// use a textstream to read all data
	QString textline;
	QTextStream data(&fin);
	while(!data.atEnd()) {
		textline = data.readLine();
		if(!(textline.isEmpty() || textline[0] == '#')) {
			// save the whole entry
			m_tocList.append(textline);

			// list entries 0,1,basename(2),3 are needed for keyword search
			// (key,title,filepath,keywords)
			QStringList list = textline.split(';', QString::KeepEmptyParts);

			// get basename of help file
			QString basename;
			if(list.count() > 2) {
				QFileInfo fi(list[2]);
				basename = fi.baseName().toLower();
			}
			else
				if(list.count() < 2) {
					continue;
				}
			QString entry = list[0] + ';' + list[1];
			if(!basename.isEmpty()) {
				entry += ';' + basename;
			}
			if(list.count() > 3) {
				entry += ';' + list[3];
			}
			m_tocSearchList.append(entry);
		}
	}
}

void TexDocDialog::showToc(const QString &caption, const QStringList &doclist, bool toc)
{
	QString section, textline;
	QStringList keylist;
	QTreeWidgetItem *itemsection = NULL;

	setUpdatesEnabled(false);
	m_texdocs->setHeaderLabel(caption);

	for (int i = 0; i < doclist.count(); ++i) {
		if(doclist[i][0] == '@') {
			section = doclist[i];
			itemsection = new QTreeWidgetItem(m_texdocs, QStringList(section.remove(0, 1)));
		}
		else {
			keylist = doclist[i].split(';', QString::KeepEmptyParts);
			if(keylist.size() < 4) {
				continue;
			}
			if(itemsection) {
				QTreeWidgetItem *item = new QTreeWidgetItem(itemsection, QStringList() << keylist[1] << keylist[0]);
				item->setIcon(0, SmallIcon(getIconName(keylist[2])));

				// save filename in dictionary
				m_dictDocuments[keylist[0]] = keylist[2];

				// search for special keywords
				QRegExp reg("^\\s*(-\\d-)");
				if(keylist[3].indexOf(reg, 0) == 0) {
					m_dictStyleCodes[keylist[0]] = reg.cap(1);
				}
			}
		}
	}
	setUpdatesEnabled(true);

	if(toc) {
		m_pbSearch->setEnabled(false);
	}
	enableButton(Default, !toc);
	m_texdocs->setFocus();
	
	if( m_texdocs->topLevelItemCount() == 1 )
		m_texdocs->expandAll();
}

bool TexDocDialog::eventFilter(QObject *o, QEvent *e)
{
	// catch KeyPress events
	if(e->type() == QEvent::KeyPress) {
		QKeyEvent *kev = (QKeyEvent*) e;

		// ListView:
		//  - space:  enable start of viewer
		//  - return: ignore
		if(o == m_texdocs) {
			if(kev->key() == Qt::Key_Space) {
				slotListViewDoubleClicked(m_texdocs->currentItem());
				return true;
			}
			if(kev->key() == Qt::Key_Return || kev->key() == Qt::Key_Enter) {
				return true;
			}
		}

		// LineEdit
		//  - return: start search, if button is enabled
		if(o == m_leKeywords) {
			if(kev->key() == Qt::Key_Return || kev->key() == Qt::Key_Enter) {
				callSearch();
				return true;
			}
		}
	}

	return false;
}

////////////////////// prepare document file //////////////////////

QString TexDocDialog::searchFile(const QString &docfilename, const QString &listofpaths, const QString &subdir)
{
	QStringList pathlist  = listofpaths.split(PATH_SEPARATOR);
	QStringList extlist   = QString(",.gz,.bz2").split(',', QString::KeepEmptyParts);

	QString filename;
	for (QStringList::Iterator itp = pathlist.begin(); itp != pathlist.end(); ++itp) {
		for (QStringList::Iterator ite = extlist.begin(); ite != extlist.end(); ++ite) {
			filename = (subdir.isEmpty()) ? (*itp) + '/' + docfilename + (*ite)
			           : (*itp) + '/' + subdir + '/' + docfilename + (*ite);
			KILE_DEBUG() << "search file: "  << filename << endl;
			if(QFile::exists(filename)) {
				return filename;
			}
		}
	}

	return QString();
}

void TexDocDialog::decompressFile(const QString &docfile, const QString &command)
{
	QString ext = QFileInfo(docfile).suffix().toLower();
	if(!(ext == "dvi" || ext == "pdf" || ext == "ps" || ext == "html")) {
		ext = "txt";
	}

	if(m_tempfile) {
		delete m_tempfile;
	}
	m_tempfile = new KTemporaryFile();
	m_tempfile->setSuffix('.' + ext);
	m_tempfile->setAutoRemove(true);

	if(!m_tempfile->open()) {
		KMessageBox::error(this, i18n("Could not create a temporary file."));
		m_filename.clear();
		return;
	}
	m_filename = m_tempfile->fileName(); // the unique file name of the temporary file should be kept
	m_tempfile->close(); 

	KILE_DEBUG() << "\tdecompress file: "  << command + " > " + m_filename << endl;
	connect(this, SIGNAL(processFinished()), this, SLOT(slotShowFile()));
	executeScript(command + " > " + m_filename);
}

void TexDocDialog::showStyleFile(const QString &filename, const QString &stylecode)
{
	KILE_DEBUG() << "\tshow style file: " << filename << endl;
	if(! QFile::exists(filename)) {
		return;
	}

	// open to read
	QFile fin(filename);
	if(!fin.exists() || !fin.open(QIODevice::ReadOnly)) {
		KMessageBox::error(this, i18n("Could not read the style file."));
		return;
	}

	if(m_tempfile) {
		delete m_tempfile;
	}
	m_tempfile = new KTemporaryFile();
	m_tempfile->setAutoRemove(true);
	m_tempfile->setSuffix(".txt");

	// use a textstream to write to the temporary file
	if(!m_tempfile->open()) {
		KMessageBox::error(this, i18n("Could not create a temporary file."));
		return ;
	}
	QTextStream stream(m_tempfile);

	// use another textstream to read from the style file
	QTextStream sty(&fin);

	// there are four mode to read from the style file
	QString textline;
	if(stylecode == "-3-") {
		// mode 3: read everything up to the first empty line
		while(!sty.atEnd()) {
			textline = sty.readLine().trimmed();
			if(textline.isEmpty()) {
				break;
			}
			stream << textline << "\n";
		}
	}
	else {
		if(stylecode == "-2-") {
			// mode 2: read everything up to a line starting with at least 4 '%' characters
			for (int i = 0; i < 9; ++i) {
				stream << sty.readLine() << "\n";
			}
			while(!sty.atEnd()) {
				textline = sty.readLine();
				if(textline.indexOf("%%%%") == 0)
					break;
				stream << textline << "\n";
			}
		}
		else {
			if(stylecode == "-1-") {
				// mode 1: read all lines at the end behind \endinput
				while(!sty.atEnd()) {
					textline = sty.readLine().trimmed();
					if(textline.indexOf("\\endinput") == 0)
						break;
				}
				while(!sty.atEnd()) {
					stream << sty.readLine() << "\n";
				}
			}
			else {
				// mode 0: read everything except empty lines and comments
				while(!sty.atEnd()) {
					textline = sty.readLine();
					if(!textline.isEmpty() && textline[0] != '%') {
						stream << textline << "\n";
					}
				}
			}
		}
	}
	// start the viewer
	showFile(m_tempfile->fileName());
}

void TexDocDialog::showFile(const QString &filename)
{
	KILE_DEBUG() << "\tshow file: " << filename << endl;
	if(QFile::exists(filename)) {
		KUrl url;
		url.setPath(filename);

		KService::List offers = KMimeTypeTrader::self()->query(getMimeType(filename), "Application");
		if(offers.isEmpty()) {
			KMessageBox::error(this, i18n("No KDE service found for this file."));
			return;
		}
		KUrl::List lst;
		lst.append(url);
		KRun::run(*(offers.first()), lst, this, true);
	}
}


////////////////////// Slots //////////////////////

void TexDocDialog::slotListViewDoubleClicked(QTreeWidgetItem *item)
{
	if(! item->parent()) {
		return;
	}

	QString package = item->text(1);
	KILE_DEBUG() << "\tselect child: "  << item->text(0) << endl
	<< "\tis package: " << package << endl;
	if(! m_dictDocuments.contains(package)) {
		return;
	}

	QString texdocfile = m_dictDocuments[package];
	KILE_DEBUG() << "\tis texdocfile: " << texdocfile << endl;

	// search for the file in the documentation directories
	QString filename = searchFile(texdocfile, m_texmfdocPath);
	if(filename.isEmpty()) {
		// not found: search it elsewhere
		filename = searchFile(texdocfile, m_texmfPath, "tex");
		if(filename.isEmpty()) {
			KMessageBox::error(this, i18n("Could not find '%1'", filename));
			return;
		}
	}
	KILE_DEBUG() << "\tfound file: " << filename << endl;

	QString ext = QFileInfo(filename).suffix().toLower();
	m_filename.clear();
	if(ext == "gz") {
		decompressFile(m_dictDocuments[package], "gzip -cd " + filename);
	}
	else {
		if(ext == "bz2") {
			decompressFile(m_dictDocuments[package], "bzip2 -cd " + filename);
		}
		else {
			if(ext == "sty" &&  m_dictStyleCodes.contains(package)) {
				showStyleFile(filename, m_dictStyleCodes[package]);
			}
			else {
				showFile(filename);
			}
		}
	}
}

void TexDocDialog::slotTextChanged(const QString &text)
{
	m_pbSearch->setEnabled(! text.trimmed().isEmpty());
}

void TexDocDialog::slotSearchClicked()
{
	QString keyword = m_leKeywords->text().trimmed();
	if(keyword.isEmpty()) {
		KMessageBox::error(this, i18n("No keyword given."));
		return;
	}

	QString section;
	bool writesection = true;
	QStringList searchlist;

	for(int i = 0; i < m_tocList.count(); i++) {
		if(m_tocList[i][0] == '@') {
			section = m_tocList[i];
			writesection = true;
		}
		else
			if(i < m_tocSearchList.count() && m_tocSearchList[i].indexOf(keyword, 0, Qt::CaseInsensitive) > -1) {
				if(writesection)
					searchlist.append(section);
				searchlist.append(m_tocList[i]);
				writesection = false;
			}
	}

	if(searchlist.count() > 0) {
		m_texdocs->clear();
		showToc(i18n("Search results for keyword '%1'", keyword), searchlist, false);
	}
	else {
		KMessageBox::error(this, i18n("No documents found for keyword '%1'.", keyword));
	}
}

void TexDocDialog::slotButtonClicked(int button)
{
	if(button == Default) {
		m_leKeywords->setText(QString());
		m_texdocs->clear();
		showToc(i18n("Table of Contents"), m_tocList, true);
	}
	KDialog::slotButtonClicked(button);
}

void TexDocDialog::callSearch()
{
	if(m_pbSearch->isEnabled()) {
		slotSearchClicked();
	}
}

////////////////////// execute shell script //////////////////////

void TexDocDialog::executeScript(const QString &command)
{
	if(m_proc) {
		delete m_proc;
	}

	m_proc = new KProcess();
	m_proc->setShellCommand(command);
	m_proc->setOutputChannelMode(KProcess::MergedChannels);
	m_proc->setReadChannel(QProcess::StandardOutput);
	m_output.clear();

	connect(m_proc, SIGNAL(readyReadStandardOutput()),
	        this,   SLOT(slotProcessOutput()));
	connect(m_proc, SIGNAL(readyReadStandardError()),
	        this,   SLOT(slotProcessOutput()));
	connect(m_proc, SIGNAL(finished(int, QProcess::ExitStatus)),
	        this,   SLOT(slotProcessExited(int, QProcess::ExitStatus)));

	KILE_DEBUG() << "=== TexDocDialog::runShellSkript() ====================" << endl;
	KILE_DEBUG() << "   execute: " << command << endl;
	m_proc->start();
}

void TexDocDialog::slotProcessOutput()
{
	m_output += m_proc->readAll();
}


void TexDocDialog::slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
	Q_UNUSED(exitCode);

	if(exitStatus == QProcess::NormalExit) {
		//showFile(m_filename);
		emit(processFinished());
	}
	else {
		KMessageBox::error(this, i18n("<center>") + i18n("Could not determine the search paths of TexLive/teTeX or file 'texdoctk.dat'.<br> Hence, this dialog is unable to provide any useful information.") + i18n("</center>"), i18n("TexDoc Dialog"));
	}
}

////////////////////// process slots, when finished //////////////////////

void TexDocDialog::slotInitToc()
{
	disconnect(this, SIGNAL(processFinished()), this, SLOT(slotInitToc()));

	QStringList results = m_output.split('\n', QString::KeepEmptyParts);
	if(results.count() < 3) {
		KMessageBox::error(this, i18n("Could not determine the search paths of TexLive or file 'texdoctk.dat'.<br> Hence, this dialog is unable to provide any useful information."));
		return;
	}

	m_texdoctkPath = results[0];
	m_texmfdocPath = results[1];
	m_texmfPath = results[2];

	KILE_DEBUG() << "\ttexdoctk path: " << m_texdoctkPath << endl;
	KILE_DEBUG() << "\ttexmfdoc path: " << m_texmfdocPath << endl;
	KILE_DEBUG() << "\ttexmf path: " << m_texmfPath << endl;

	if(m_texdoctkPath.indexOf('\n', -1) > -1) {
		m_texdoctkPath.truncate(m_texdoctkPath.length() - 1);
	}

	// read data and initialize listview
	readToc();
	slotButtonClicked(Default);
}

void TexDocDialog::slotShowFile()
{
	disconnect(this, SIGNAL(processFinished()), this, SLOT(slotShowFile()));
	showFile(m_filename);
}

////////////////////// Icon/Mime //////////////////////

QString TexDocDialog::getMimeType(const QString &filename)
{
	QFileInfo fi(filename);
	QString basename = fi.baseName().toLower();
	QString ext = fi.suffix().toLower();

	QString mimetype;
	if(ext == "txt" || ext == "faq" || ext == "sty" || basename == "readme" || basename == "00readme") {
		mimetype = "text/plain";
	}
	else {
		KUrl mimeurl;
		mimeurl.setPath(filename);
		KMimeType::Ptr pMime = KMimeType::findByUrl(mimeurl);
		mimetype = pMime->name();
	}

	KILE_DEBUG() << "\tmime = "  << mimetype << " " << endl;
	return mimetype;
}

QString TexDocDialog::getIconName(const QString &filename)
{
	QFileInfo fi(filename);
	QString basename = fi.baseName().toLower();
	QString ext = fi.suffix().toLower();

	QString icon;
	if(ext == "application-x-bzdvi" ) { // FIXME exchange as soon as a real dvi icon is available
		icon = ext;
	}
	else if( ext == "htm" || ext == "html" ){
		icon = "text-html";
	}
	else if(ext == "pdf" ){
		icon = "application-pdf";
	}
	else if( ext == "txt"){
		ext = "text-plain";
	}
	else {
		if(ext == "ps") {
			icon = "application-postscript";
		}
		else {
			if(ext == "sty") {
				icon = "text-x-tex";
			}
			else {
				if(ext == "faq" || basename == "readme" || basename == "00readme") {
					icon = "text-x-readme";
				}
				else {
					icon = "text-plain";
				}
			}
		}
	}
	return icon;
}

}
#include "texdocumentationdialog.moc"
