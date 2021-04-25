/******************************************************************************
  Copyright (C) 2009-2011 by Holger Danielsson (holger.danielsson@versanet.de)
            (C) 2019 by Michel Ludwig (michel.ludwig@kdemail.net)
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

#include <QCheckBox>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QFile>
#include <QGridLayout>
#include <QGroupBox>
#include <QInputDialog>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QLocale>
#include <QProcess>
#include <QPushButton>
#include <QRegExp>
#include <QStandardPaths>
#include <QStringList>
#include <QTemporaryFile>
#include <QTextStream>
#include <QValidator>
#include <QVBoxLayout>

#include <KComboBox>
#include <KConfigGroup>
#include <KIconLoader>
#include <KLocalizedString>
#include <KMessageBox>
#include <KProcess>
#include <KUrlRequester>

#include "errorhandler.h"
#include "kileconfig.h"
#include "kiledebug.h"


namespace KileDialog
{

PdfDialog::PdfDialog(QWidget *parent,
                     const QString &texfilename,const QString &startdir,
                     const QString &latexextensions,
                     KileTool::Manager *manager,
                     KileErrorHandler *errorHandler, KileWidget::OutputView *output)
    : QDialog(parent)
    , m_startdir(startdir)
    , m_manager(manager)
    , m_errorHandler(errorHandler)
    , m_output(output)
    , m_tempdir(Q_NULLPTR)
    , m_proc(Q_NULLPTR)
    , m_rearrangeButton(new QPushButton)
    , m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Help|QDialogButtonBox::Close))
{
    setWindowTitle(i18n("PDF Wizard"));
    setModal(true);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    m_rearrangeButton->setDefault(true);

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
    mainLayout->addWidget(page);
    m_PdfDialog.setupUi(page);
    page->setMinimumWidth(500);
    m_PdfDialog.m_pbPrinting->setIcon(QIcon::fromTheme("printer"));
    m_PdfDialog.m_pbAll->setIcon(QIcon::fromTheme("list-add"));
    m_PdfDialog.m_pbBackgroundColor->setColor(QColor(255, 255, 224));

    // insert KileWidget::CategoryComboBox
    m_cbTask = new KileWidget::CategoryComboBox(m_PdfDialog.m_gbParameter);
    QGridLayout *paramLayout = (QGridLayout *)m_PdfDialog.m_gbParameter->layout();
    paramLayout->addWidget(m_cbTask, 4, 1);

    // setup filenames
    m_PdfDialog.m_edInfile->setFilter(i18n("*.pdf|PDF Files"));
    m_PdfDialog.m_edInfile->lineEdit()->setText(pdffilename);
    m_PdfDialog.m_edOutfile->setFilter(i18n("*.pdf|PDF Files"));
    m_PdfDialog.m_edOutfile->setMode(KFile::File | KFile::LocalOnly );
    m_PdfDialog.m_edOutfile->lineEdit()->setText( getOutfileName(pdffilename) );

    //max password length for pdf files
    m_PdfDialog.m_edPassword->setMaxLength(32);

    // set an user button to execute the task and icon for help button
    m_rearrangeButton->setText(i18n("Re&arrange"));
    m_rearrangeButton->setIcon(QIcon::fromTheme("system-run"));
    m_PdfDialog.m_lbParameterIcon->setPixmap(QIcon::fromTheme("help-about").pixmap(KIconLoader::SizeSmallMedium));

    // init important variables
    m_numpages = 0;
    m_encrypted = false;
    m_pdftk = false;
    m_pdfpages = false;
    m_scriptrunning = false;
    m_pagesize = QSize(0,0);

    // setup tasks
    m_tasklist << i18n("1 Page + Empty Page --> 2up")           // 0   PDF_PAGE_EMPTY
               << i18n("1 Page + Duplicate --> 2up")            // 1   PDF_PAGE_DUPLICATE
               << i18n("2 Pages --> 2up")                       // 2   PDF_2UP
               << i18n("2 Pages (landscape) --> 2up")           // 3   PDF_2UP_LANDSCAPE
               << i18n("4 Pages --> 4up")                       // 4   PDF_4UP
               << i18n("4 Pages (landscape) --> 4up")           // 5   PDF_4UP_LANDSCAPE
               << i18n("Select Even Pages")                     // 6   PDF_EVEN
               << i18n("Select Odd Pages")                      // 7   PDF_ODD
               << i18n("Select Even Pages (reverse order)")     // 8   PDF_EVEN_REV
               << i18n("Select Odd Pages (reverse order)")      // 9   PDF_ODD_REV
               << i18n("Reverse All Pages")                     // 10  PDF_REVERSE
               << i18n("Decrypt")                               // 11  PDF_DECRYPT
               << i18n("Select Pages")                          // 12  PDF_SELECT
               << i18n("Delete Pages")                          // 13  PDF_DELETE
               << i18n("Apply a background watermark")          // 14  PDF_PDFTK_BACKGROUND
               << i18n("Apply a background color")              // 15  PDF_PDFTK_BGCOLOR
               << i18n("Apply a foreground stamp")              // 16  PDF_PDFTK_STAMP
               << i18n("pdftk: Choose Parameter")               // 17  PDF_PDFTK_FREE
               << i18n("pdfpages: Choose Parameter")            // 18  PDF_PDFPAGES_FREE
               ;

    // set data for properties: key/widget
    m_pdfInfoKeys << "Title" << "Subject" << "Author" << "Creator" << "Producer" << "Keywords";

    m_pdfInfoWidget["Title"] = m_PdfDialog.m_leTitle;
    m_pdfInfoWidget["Subject"] = m_PdfDialog.m_leSubject;
    m_pdfInfoWidget["Keywords"] = m_PdfDialog.m_leKeywords;
    m_pdfInfoWidget["Author"] = m_PdfDialog.m_leAuthor;
    m_pdfInfoWidget["Creator"] = m_PdfDialog.m_leCreator;
    m_pdfInfoWidget["Producer"] = m_PdfDialog.m_leProducer;

    // set data for  permissions: key/widget
    m_pdfPermissionKeys    << AllowModify << AllowCopy << AllowPrint
                           << AllowNotes  << AllowFillForms;

    m_pdfPermissionWidgets << m_PdfDialog.m_cbModify << m_PdfDialog.m_cbCopy << m_PdfDialog.m_cbPrinting
                           << m_PdfDialog.m_cbAnnotations << m_PdfDialog.m_cbFormFeeds;

    m_pdfPermissionPdftk   << "ModifyContents" << "CopyContents" << "Printing"
                           << "ModifyAnnotations" << "FillIn";

    // default permissions
    m_pdfPermissionState << false << false  << false  << false  << false;

    // check for libpoppler pdf library
#if LIBPOPPLER_AVAILABLE
    m_poppler = true;
    KILE_DEBUG_MAIN << "working with libpoppler pdf library";
#else
    m_poppler = false;
    KILE_DEBUG_MAIN << "working without libpoppler pdf library";
    m_PdfDialog.tabWidget->removeTab(2);
    m_PdfDialog.tabWidget->removeTab(1);
#endif

    // init Dialog
    m_PdfDialog.m_lbParameterInfo->setTextFormat(Qt::RichText);
    m_PdfDialog.m_cbOverwrite->setChecked(false);
    updateDialog();

    connect(this, &PdfDialog::output, m_output, &KileWidget::OutputView::receive);
    connect(m_PdfDialog.m_edInfile->lineEdit(), &QLineEdit::textChanged, this, &PdfDialog::slotInputfileChanged);

#if LIBPOPPLER_AVAILABLE
    connect(m_PdfDialog.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(slotTabwidgetChanged(int)));
    connect(m_PdfDialog.m_pbPrinting, SIGNAL(clicked()), this, SLOT(slotPrintingClicked()));
    connect(m_PdfDialog.m_pbAll, SIGNAL(clicked()), this, SLOT(slotAllClicked()));
#endif

    m_buttonBox->addButton(m_rearrangeButton, QDialogButtonBox::ActionRole);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_buttonBox, &QDialogButtonBox::helpRequested, this, &PdfDialog::slotShowHelp);
    connect(m_rearrangeButton, &QPushButton::clicked, this, &PdfDialog::slotExecute);
    mainLayout->addWidget(m_buttonBox);

    // find available utilities for this dialog
    executeScript("kpsewhich pdfpages.sty", QString(), PDF_SCRIPTMODE_TOOLS);
}

PdfDialog::~PdfDialog()
{
    if (m_cbTask->currentIndex() != -1) {
        KileConfig::setPdfWizardLastTask(m_cbTask->currentIndex());
    }
    delete m_tempdir;
    delete m_proc;
}

void PdfDialog::initUtilities()
{
    // find pdfpages.sty?
    m_pdfpages = m_outputtext.contains("pdfpages.sty");

    // additionally look for pdftk
    m_pdftk = !QStandardPaths::findExecutable("pdftk").isEmpty();

//m_pdfpages = false;            // <----------- only for testing  HACK
//m_pdftk = false;               // <----------- only for testing  HACK

    KILE_DEBUG_MAIN << "Looking for pdf tools: pdftk=" << m_pdftk << " pdfpages.sty=" << m_pdfpages;

#if !LIBPOPPLER_AVAILABLE
    m_imagemagick = KileConfig::imagemagick();

    // we can't use libpoppler pdf library and need to find another method to determine the number of pdf pages
    // Kile will use three options before giving up
    if ( m_pdftk )
        m_numpagesMode = PDF_SCRIPTMODE_NUMPAGES_PDFTK;
    else if ( m_imagemagick )
        m_numpagesMode = PDF_SCRIPTMODE_NUMPAGES_IMAGEMAGICK;
    else
        m_numpagesMode = PDF_SCRIPTMODE_NUMPAGES_GHOSTSCRIPT;
#endif

    // no pdftk, so properties and permissions are readonly
    if ( !m_pdftk ) {
        // set readonly properties
        for (QStringList::const_iterator it = m_pdfInfoKeys.constBegin(); it != m_pdfInfoKeys.constEnd(); ++it) {
            m_pdfInfoWidget[*it]->setReadOnly(true);
        }
#if LIBPOPPLER_AVAILABLE
        // connect permission widgets
        for (int i=0; i<m_pdfPermissionKeys.size(); ++i) {
            connect(m_pdfPermissionWidgets.at(i), SIGNAL(clicked(bool)), this, SLOT(slotPermissionClicked(bool)));
        }
#endif
    }

    // if we found at least one utility, we can enable some connections
    if ( m_pdftk || m_pdfpages) {
        connect(m_PdfDialog.m_edOutfile->lineEdit(), SIGNAL(textChanged(QString)), this, SLOT(slotOutputfileChanged(QString)));
        connect(m_PdfDialog.m_cbOverwrite, SIGNAL(stateChanged(int)), this, SLOT(slotOverwriteChanged(int)));
        connect(m_cbTask, SIGNAL(activated(int)), this, SLOT(slotTaskChanged(int)));
    }

    // setup dialog
    slotInputfileChanged(m_PdfDialog.m_edInfile->lineEdit()->text());
}

// read properties and permissions from the PDF document
void PdfDialog::pdfParser(const QString &filename)
{
#if LIBPOPPLER_AVAILABLE
    Poppler::Document *doc = Poppler::Document::load(filename);
    if ( !doc || doc->isLocked() ) {
        KILE_DEBUG_MAIN << "Error: could not open pdf document '" << filename << "'";
        return;
    }
    KILE_DEBUG_MAIN << "Parse pdf document: " << filename;

    // read encryption
    m_encrypted = doc->isEncrypted();
    m_PdfDialog.m_lbEncryption->setText( (m_encrypted) ? i18n("yes") : i18n("no") );

    // read properties
    for (QStringList::const_iterator it = m_pdfInfoKeys.constBegin(); it != m_pdfInfoKeys.constEnd(); ++it) {
        QString value = doc->info(*it);
        m_pdfInfo[*it] = value;
        m_pdfInfoWidget[*it]->setText(value);
    }

    // read creation date and modification date
    m_PdfDialog.m_lbCreationDate->setText(QLocale().toString(doc->date("CreationDate")));
    m_PdfDialog.m_lbModDate->setText(QLocale().toString(doc->date("ModDate")));

    // read PDF version
    int major,minor;
    doc->getPdfVersion(&major,&minor);
    m_PdfDialog.m_lbFormat->setText( QString("PDF version %1.%2").arg(major).arg(minor) );

    // read permissions
    for (int i=0; i<m_pdfPermissionKeys.size(); ++i) {
        bool value = isAllowed( doc, (PDF_Permission)m_pdfPermissionKeys.at(i) );
        m_pdfPermissionWidgets.at(i)->setChecked(value);

        if ( !m_pdftk ) {
            m_pdfPermissionState[i] = value;
        }
    }

    // determine and set number of pages
    setNumberOfPages( doc->numPages() );

    // look if all pages have the same size
    m_pagesize = allPagesSize(doc);

    delete doc;
#else
    /* libpoppler pdf library is not available:
     * - we use a brute force method to determine, if this file is encrypted
     * - then we try to determine the number of pages with
     *   - pdftk (always first choice, if installed)
     *   - imagemagick (second choice)
     *   - gs (third and last choice)
     * - if the pdf file is encrypted, pdftk will ask for a password
     */

    // look if the pdf file is encrypted (brute force)
    m_encrypted = readEncryption(filename);
    KILE_DEBUG_MAIN << "PDF encryption: " << m_encrypted;

    // determine the number of pages of the pdf file
    determineNumberOfPages(filename,m_encrypted);
    KILE_DEBUG_MAIN << "PDF number of pages: " << m_numpages;

    // clear pagesize
    m_pagesize = QSize(0,0);
#endif


}

#if LIBPOPPLER_AVAILABLE
bool PdfDialog::isAllowed(Poppler::Document *doc, PDF_Permission permission) const
{
    bool b = true;
    switch ( permission )
    {
    case AllowModify:
        b = doc->okToChange();
        break;
    case AllowCopy:
        b = doc->okToCopy();
        break;
    case AllowPrint:
        b = doc->okToPrint();
        break;
    case AllowNotes:
        b = doc->okToAddNotes();
        break;
    case AllowFillForms:
        b = doc->okToFillForm();
        break;
    default:
        ;
    }
    return b;
}

QSize PdfDialog::allPagesSize(Poppler::Document *doc)
{
    QSize commonsize = QSize(0,0);

    // Access all pages of the PDF file (m_numpages is known)
    for ( int i=0; i<m_numpages; ++i ) {
        Poppler::Page *pdfpage = doc->page(i);
        if ( pdfpage == 0 ) {
            KILE_DEBUG_MAIN << "Cannot parse all pages of the PDF file";
            delete pdfpage;
            return QSize(0,0);
        }

        if ( i == 0 ) {
            commonsize = pdfpage->pageSize();
        } else if ( commonsize != pdfpage->pageSize() ) {
            delete pdfpage;
            return QSize(0,0);
        }
        // documentation says: after the usage, the page must be deleted
        delete pdfpage;
    }

    return commonsize;
}
#endif

void PdfDialog::setNumberOfPages(int numpages)
{
    m_numpages = numpages;
    if (m_numpages > 0) {
        // show all, if the number of pages is known
        m_PdfDialog.tabWidget->widget(0)->setEnabled(true);

        QString pages;
        if ( m_encrypted )
            m_PdfDialog.m_lbPages->setText(i18nc("%1 is the number of pages", "%1 (encrypted)", QString::number(m_numpages)));
        else
            m_PdfDialog.m_lbPages->setText(pages.setNum(m_numpages));
    }
    else {
        // hide all, if the number of pages can't be determined
        m_PdfDialog.tabWidget->widget(0)->setEnabled(false);
        m_PdfDialog.m_lbPages->setText(i18n("Error: unknown number of pages"));
    }
}

#if !LIBPOPPLER_AVAILABLE
void PdfDialog::determineNumberOfPages(const QString &filename, bool askForPassword)
{
    // determine the number of pages of the pdf file (delegate this task)
    QString command;
    QString passwordparam;
    int scriptmode = m_numpagesMode;

    if ( scriptmode==PDF_SCRIPTMODE_NUMPAGES_PDFTK && askForPassword ) {
        QString password = QInputDialog::getText(this, i18n("PDFTK-Password"),
                           i18n("This PDF file is encrypted and 'pdftk' cannot open it.\n"
                                "Please enter the password for this PDF file\n or leave it blank to try another method: "),
                           QLineEdit::Normal, QString()).trimmed();
        if(!password.isEmpty()) {
            passwordparam = " input_pw " + password;
        }
        else {
            scriptmode = ( m_imagemagick ) ? PDF_SCRIPTMODE_NUMPAGES_IMAGEMAGICK : PDF_SCRIPTMODE_NUMPAGES_GHOSTSCRIPT;
        }
    }

    // now take the original or changed mode
    if ( scriptmode == PDF_SCRIPTMODE_NUMPAGES_PDFTK ) {
        command = "pdftk \"" + filename + "\"" + passwordparam + " dump_data | grep NumberOfPages";
    }
    else if ( scriptmode == PDF_SCRIPTMODE_NUMPAGES_IMAGEMAGICK ) {
        command = "identify -format \"%n\" \"" + filename + "\"";
    }
    else {
        command = "gs -q -c \"(" + filename + ") (r) file runpdfbegin pdfpagecount = quit\"";
    }

    // run Process
    KILE_DEBUG_MAIN << "execute for NumberOfPages: " << command;
    executeScript(command, m_tempdir->path(), scriptmode);
}

void PdfDialog::readNumberOfPages(int scriptmode, const QString &output)
{
    int numpages = 0;

    bool ok;
    if ( scriptmode == PDF_SCRIPTMODE_NUMPAGES_PDFTK ) {
        KILE_DEBUG_MAIN << "pdftk output for NumberOfPages: " << output;
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

    }
    else {
        QString s = output;
        numpages = s.remove('\n').toInt(&ok);
    }

    setNumberOfPages(numpages);
}

bool PdfDialog::readEncryption(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    KILE_DEBUG_MAIN << "search for encryption ";
    QRegExp re("/Encrypt(\\W|\\s|$)");
    QTextStream in(&file);
    QString line = in.readLine();
    while ( !line.isNull() ) {
        if ( re.indexIn(line) >= 0 ) {
            KILE_DEBUG_MAIN << "pdf file is encrypted !!!";
            return  true;
        }
        line = in.readLine();
    }
    return false;
}
#endif

void PdfDialog::clearDocumentInfo()
{
    m_numpages = 0;
    m_encrypted = false;
    m_PdfDialog.m_lbPassword->setEnabled(false);
    m_PdfDialog.m_edPassword->setEnabled(false);
    m_PdfDialog.m_edPassword->clear();

    for (QStringList::const_iterator it = m_pdfInfoKeys.constBegin(); it != m_pdfInfoKeys.constEnd(); ++it) {
        m_pdfInfoWidget[*it]->clear();
    }

    m_PdfDialog.m_lbCreationDate->clear();
    m_PdfDialog.m_lbModDate->clear();

    for (int i=0; i<m_pdfPermissionKeys.size(); ++i) {
        m_pdfPermissionWidgets.at(i)->setChecked(false);
    }

    m_PdfDialog.m_lbPages->clear();
    m_PdfDialog.m_lbFormat->clear();
    m_PdfDialog.m_lbEncryption->clear();
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
    m_rearrangeButton->setEnabled(state&&!m_scriptrunning);
}

// update tools information
void PdfDialog::updateToolsInfo()
{
    QString info;
    QString newline = "<br>";
    QString password = i18n("A password is necessary to set or change the current settings.");

    int tabindex = m_PdfDialog.tabWidget->currentIndex();
    if (tabindex == 2 ) {
        info = ( m_pdftk ) ? i18n("The permissions of this document can be changed with <i>pdftk</i>.") + newline + password
               : i18n("<i>pdftk</i> is not available, so no permission can be changed.");
    }
    else if ( tabindex == 1 ) {
        if ( ! m_pdftk ) {
            info = i18n("<i>pdftk</i> is not available, so no property can be changed.");
        }
        else {
            info = i18n("The properties of this document can be changed with <i>pdftk</i>.");
            if ( m_encrypted ) {
                info += newline + password;
            }
        }
    }
    else { // if ( tabindex == 0 )
        if ( m_encrypted ) {
            info = ( m_pdftk ) ? i18n("This input file is encrypted, so only <i>pdftk</i> works.") + newline
                   + i18n("A password is necessary to rearrange pages.")
                   : i18n("This input file is encrypted, but <i>pdftk</i> is not installed.");
        }
        else {
            if ( m_pdftk ) { // not encrypted and pdftk
                info = ( m_pdfpages ) ? i18n("This wizard will use <i>pdftk</i> and the LaTeX package <i>pdfpages</i>.")
                       : i18n("This wizard will only use <i>pdftk</i> (<i>pdfpages.sty</i> is not installed).");
            }
            else {           // not encrypted and not pdftk
                info = ( m_pdfpages ) ? i18n("This wizard will only use the LaTeX package <i>pdfpages</i> (<i>pdftk</i> was not found).")
                       : i18n("This wizard can't work, because no tool was found (see help section).");
            }
        }
    }

    QString popplerinfo = (m_poppler ) ? QString() : newline + i18n("<i>(Compiled without libpoppler pdf library. Not all tasks are available.)</i>");
    info += popplerinfo;

    // set info text
    m_PdfDialog.m_lbParameterInfo->setText(info);
}

// it is important to calculate the task index from the combobox index,
// as not all tasks are available, when an utility was not found
void PdfDialog::updateTasks()
{
    // according to QT 4.4 docu the index of QComboBox might change if adding or removing items
    // but because we populate the QComboBox before we start the dialog, we can use the index here
    int lastindex = m_cbTask->currentIndex();
    QString lasttext = m_cbTask->currentText();

    int group = 0;
    m_cbTask->clear();
    if (m_pdfpages && !m_encrypted) {                               // task index
        m_cbTask->addItem( m_tasklist[PDF_PAGE_EMPTY] );             // 0   PDF_PAGE_EMPTY
        m_cbTask->addItem( m_tasklist[PDF_PAGE_DUPLICATE] );         // 1   PDF_PAGE_DUPLICATE
        m_cbTask->addItem( m_tasklist[PDF_2UP] );                    // 2   PDF_2UP
        m_cbTask->addItem( m_tasklist[PDF_2UP_LANDSCAPE] );          // 3   PDF_2UP_LANDSCAPE
        m_cbTask->addItem( m_tasklist[PDF_4UP] );                    // 4   PDF_4UP
        m_cbTask->addItem( m_tasklist[PDF_4UP_LANDSCAPE] );          // 5   PDF_4UP_LANDSCAPE
        group = 1;
    }

    if ( (m_pdfpages && !m_encrypted) || m_pdftk ) {
        if ( group > 0 ) {
            m_cbTask->addCategoryItem("");
        }
        m_cbTask->addItem( m_tasklist[PDF_EVEN] );                   // 6   PDF_EVEN
        m_cbTask->addItem( m_tasklist[PDF_ODD] );                    // 7   PDF_ODD
        m_cbTask->addItem( m_tasklist[PDF_EVEN_REV] );               // 8   PDF_EVEN_REV
        m_cbTask->addItem( m_tasklist[PDF_ODD_REV] );                // 9   PDF_ODD_REV
        m_cbTask->addItem( m_tasklist[PDF_REVERSE] );                // 10  PDF_REVERSE
        if (m_encrypted) {
            m_cbTask->addItem( m_tasklist[PDF_DECRYPT] );             // 11  PDF_DECRYPT
        }
        m_cbTask->addCategoryItem("");
        m_cbTask->addItem( m_tasklist[PDF_SELECT] );                 // 12  PDF_SELECT
        m_cbTask->addItem( m_tasklist[PDF_DELETE] );                 // 13  PDF_DELETE
        group = 2;
    }

    if (m_pdftk) {
        m_cbTask->addCategoryItem("");
        m_cbTask->addItem( m_tasklist[PDF_PDFTK_BACKGROUND] );       // 14  PDF_PDFTK_BACKGROUND
        if ( ! m_pagesize.isNull() ) {
            m_cbTask->addItem( m_tasklist[PDF_PDFTK_BGCOLOR] );       // 15  PDF_PDFTK_BGCOLOR
        }
        m_cbTask->addItem( m_tasklist[PDF_PDFTK_STAMP] );            // 16  PDF_PDFTK_STAMP
        m_cbTask->addCategoryItem("");
        m_cbTask->addItem( m_tasklist[PDF_PDFTK_FREE] );             // 17  PDF_PDFTK_FREE
        group = 3;
    }

    if (m_pdfpages && !m_encrypted) {
        if ( group < 3 ) {
            m_cbTask->addCategoryItem("");
        }
        m_cbTask->addItem( m_tasklist[PDF_PDFPAGES_FREE] );          // 17  PDF_PDFPAGES_FREE
    }

    // choose one common task (need to calculate the combobox index)
    int index = m_cbTask->findText(lasttext);
    if ( lastindex==-1 || index==-1 ) {
        int lastTask = KileConfig::pdfWizardLastTask();
        int task = ( lastTask < m_cbTask->count() ) ? lastTask : PDF_SELECT;
        index = m_cbTask->findText(m_tasklist[task]);
        if ( index == -1 ) {
            index = 0;
        }
    }

    m_cbTask->setCurrentIndex(index);
    slotTaskChanged(index);

    setFocusProxy(m_PdfDialog.m_edInfile);
    m_PdfDialog.m_edInfile->setFocus();
}

QString PdfDialog::getOutfileName(const QString &infile)
{
    return ( infile.isEmpty() ) ? QString() : infile.left(infile.length()-4) + "-out" + ".pdf";
}

// calculate task index from comboxbox index
int PdfDialog::taskIndex()
{
    return m_tasklist.indexOf( m_cbTask->currentText() );
}

void PdfDialog::setPermissions(bool print, bool other)
{
    for (int i = 0; i<m_pdfPermissionKeys.size(); ++i) {
        QCheckBox *box = m_pdfPermissionWidgets.at(i);
        bool state = ( box == m_PdfDialog.m_cbPrinting ) ? print : other;
        box->setChecked(state);
    }
}

// read permissions
QString PdfDialog::readPermissions()
{
    QString permissions;
    for (int i = 0; i < m_pdfPermissionKeys.size(); ++i) {
        if ( m_pdfPermissionWidgets.at(i)->isChecked() ) {
            permissions += m_pdfPermissionPdftk.at(i) + ' ';
        }
    }
    return permissions;
}

//-------------------- slots --------------------

void PdfDialog::slotTabwidgetChanged(int index)
{
    m_rearrangeButton->setText(index == 0 ? i18n("Re&arrange") : i18n("&Update"));
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
    for (int i = 0; i < m_pdfPermissionKeys.size(); ++i) {
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
        pdfParser(text);
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

void PdfDialog::slotTaskChanged(int)
{
    if ( m_PdfDialog.tabWidget->currentIndex() > 0 ) {
        return;
    }

    int taskindex = taskIndex();
    if ( isParameterTask(taskindex) ) {
        QString s,labeltext;
        if ( taskindex==PDF_SELECT || taskindex==PDF_DELETE ) {
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
        m_PdfDialog.m_lbParamInfo->setText(" (" + s + ')');

        m_PdfDialog.m_lbParameter->setText(labeltext);
        m_PdfDialog.m_lbParameter->show();
        m_PdfDialog.m_edParameter->clear();
        m_PdfDialog.m_edParameter->show();
        m_PdfDialog.m_lbParamInfo->show();
    }
    else {
        m_PdfDialog.m_lbParameter->hide();
        m_PdfDialog.m_edParameter->hide();
        m_PdfDialog.m_lbParamInfo->hide();
    }

    if ( isOverlayTask(taskindex) ) {
        m_PdfDialog.m_lbStamp->show();
        m_PdfDialog.m_edStamp->show();

        if ( taskindex == PDF_PDFTK_BACKGROUND ) {
            m_PdfDialog.m_edStamp->setWhatsThis(i18n("Applies a PDF watermark to the background of a single input PDF. "
                                                "Pdftk uses only the first page from the background PDF and applies it to every page of the input PDF. "
                                                "This page is scaled and rotated as needed to fit the input page.") );
        }
        else if ( taskindex == PDF_PDFTK_STAMP ) {
            m_PdfDialog.m_edStamp->setWhatsThis( i18n("Applies a foreground stamp on top of the input PDF document's pages. "
                                                 "Pdftk uses only the first page from the stamp PDF and applies it to every page of the input PDF. "
                                                 "This page is scaled and rotated as needed to fit the input page. "
                                                 "This works best if the stamp PDF page has a transparent background.") );
        }
    }
    else {
        m_PdfDialog.m_lbStamp->hide();
        m_PdfDialog.m_edStamp->hide();
    }

    if (isBackgroundColor(taskindex)) {
        m_PdfDialog.m_lbBackgroundColor->show();
        m_PdfDialog.m_pbBackgroundColor->show();
    }
    else {
        m_PdfDialog.m_lbBackgroundColor->hide();
        m_PdfDialog.m_pbBackgroundColor->hide();
    }
    if (isOverlayTask(taskindex) || isBackgroundColor(taskindex) || isFreeTask(taskindex)) {
        m_rearrangeButton->setText(i18n("&Apply"));
    }
    else {
        m_rearrangeButton->setText(i18n("Re&arrange"));
    }
}

// execute commands
void PdfDialog::slotExecute()
{
    if(!m_tempdir) {
        // create tempdir
        m_tempdir = new QTemporaryDir(QDir::tempPath() + QLatin1String("/kile-pdfwizard"));
        m_tempdir->setAutoRemove(true);
        KILE_DEBUG_MAIN << "tempdir: " << m_tempdir->path();
    }

    if(!m_tempdir->isValid()) {
        KMessageBox::error(this, i18n("Failed to create a temporary directory.\n\nThis wizard cannot be used."));
        reject();
        return;
    }

    int tabindex = m_PdfDialog.tabWidget->currentIndex();

    switch (tabindex) {
    case 0:
        if (checkParameter()) {
            executeAction();
        }
        break;
    case 1:
        if (checkProperties()) {
            executeProperties();
        }
        break;
    case 2:
        if (checkPermissions()) {
            executePermissions();
        }
        break;
    }
}

void PdfDialog::slotShowHelp()
{
    QString message = i18n("<center>PDF-Wizard</center><br>"
                           "This wizard uses 'pdftk' and the LaTeX package 'pdfpages' to"
                           "<ul>"
                           "<li>rearrange pages of an existing PDF document</li>"
                           "<li>read and update documentinfo of a PDF document (only pdftk)</li>"
                           "<li>read, set or change some permissions of a PDF document (only pdftk). "
                           "A password is necessary to set or change this document settings. "
                           "Additionally PDF encryption is done to lock the file's content behind this password.</li>"
                           "</ul>"
                           "<p>The package 'pdfpages' will only work with non-encrypted documents. "
                           "'pdftk' can handle both kind of documents, but a password is needed for encrypted files. "
                           "If one of 'pdftk' or 'pdfpages' is not available, the possible rearrangements are reduced.</p>"
                           "<p><i>Warning:</i> Encryption and a password does not provide any real PDF security. The content "
                           "is encrypted, but the key is known. You should see it more as a polite but firm request "
                           "to respect the author's wishes.</p>");

#if !LIBPOPPLER_AVAILABLE
    message += i18n("<p><i>Information: </i>This version of Kile was compiled without libpoppler library. "
                    "Setting, changing and removing of properties and permissions is not possible.</p>");
#endif

    KMessageBox::information(this, message, i18n("PDF Tools"));
}

void PdfDialog::executeAction()
{
    QString command = buildActionCommand();
    if ( command.isEmpty() ) {
        return;
    }

    m_errorHandler->clearMessages();
    QFileInfo from(m_inputfile);
    QFileInfo to(m_outputfile);

    // output for log window
    QString program = (m_execLatex) ? i18n("LaTeX with 'pdfpages' package") : i18n("pdftk");
    QString msg = i18n("Rearranging PDF file: %1", from.fileName());
    if (!to.fileName().isEmpty())
        msg += " ---> " + to.fileName();
    m_errorHandler->printMessage(KileTool::Info, msg, program);

    // some output logs
    m_output->clear();
    QString s = QString("*****\n")
                + i18n("***** tool:        ") + program + '\n'
                + i18n("***** input file:  ") + from.fileName()+ '\n'
                + i18n("***** output file: ") + to.fileName()+ '\n'
                + i18n("***** param:       ") + m_param + '\n'
                + i18n("***** command:     ") + command + '\n'
                + i18n("***** viewer:      ") + ((m_PdfDialog.m_cbView->isChecked()) ? i18n("yes") : i18n("no")) + '\n'
                + "*****\n";
    emit( output(s) );

    // run Process
    executeScript(command, m_tempdir->path(), PDF_SCRIPTMODE_ACTION);
}

void PdfDialog::executeProperties()
{
    // create temporary file
    QTemporaryFile infotemp(m_tempdir->path() + QLatin1String("/kile-pdfdialog-XXXXXX.txt"));
    infotemp.setAutoRemove(false);

    if(!infotemp.open()) {
        KILE_DEBUG_MAIN << "Could not create tempfile for key/value pairs in QString PdfDialog::executeProperties()" ;
        return;
    }
    QString infofile = infotemp.fileName();

    // create a text file with key/value pairs for pdftk
    QTextStream infostream(&infotemp);
    for (QStringList::const_iterator it = m_pdfInfoKeys.constBegin(); it != m_pdfInfoKeys.constEnd(); ++it) {
        infostream << "InfoKey: " << (*it) << "\n";
        infostream << "InfoValue: " << m_pdfInfoWidget[*it]->text().trimmed() << "\n";
    }
    // add modification Date
    QString datetime = QDateTime::currentDateTimeUtc().toString("%Y%m%d%H%M%S%:z");
    datetime = datetime.replace(":","'");
    infostream << "InfoKey: " << "ModDate" << "\n";
    infostream << "InfoValue: " << "D:" << datetime << "'\n";
    infotemp.close();

    // build command
    QString inputfile = m_PdfDialog.m_edInfile->lineEdit()->text().trimmed();
    QString password =  m_PdfDialog.m_edPassword->text().trimmed();
    QString pdffile = m_tempdir->path() + QFileInfo(m_inputfile).baseName() + "-props.pdf";

    // read permissions
    QString permissions = readPermissions();

    // build param
    QString param = "\"" + inputfile + "\"";
    if ( m_encrypted ) {
        param += " input_pw " + password;
    }

    param += " update_info " + infofile + " output \"" + pdffile+ "\"";
    if ( m_encrypted ) {
        param += " encrypt_128bit";
        if ( !permissions.isEmpty() )
            param += " allow " + permissions;
        param += " owner_pw " + password;
    }
    QString command = "pdftk " + param;

    // move destination file
    m_move_filelist.clear();
    m_move_filelist << pdffile << inputfile;

    // execute script
    showLogs("Updating properties", inputfile, param);
    executeScript(command, QString(), PDF_SCRIPTMODE_PROPERTIES);

}

void PdfDialog::executePermissions()
{
    // read permissions
    QString permissions = readPermissions();

    // build command
    QString inputfile = m_PdfDialog.m_edInfile->lineEdit()->text().trimmed();
    QString password =  m_PdfDialog.m_edPassword->text().trimmed();
    QString pdffile = m_tempdir->path() + QFileInfo(m_inputfile).baseName() + "-perms.pdf";

    QString param = "\"" + inputfile + "\"";
    if ( m_encrypted ) {
        param += " input_pw " + password;
    }
    param += " output \"" + pdffile + "\" encrypt_128bit";
    if ( !permissions.isEmpty() ) {
        param += " allow " + permissions;
    }
    param += " owner_pw " + password;
    QString command = "pdftk " + param;

    // move destination file
    m_move_filelist.clear();
    m_move_filelist << pdffile << inputfile;

    // execute script
    showLogs("Updating permissions", inputfile, param);
    executeScript(command, QString(), PDF_SCRIPTMODE_PERMISSIONS);

}

void PdfDialog::showLogs(const QString &title, const QString &inputfile, const QString &param)
{
    // some info for log widget
    m_errorHandler->clearMessages();
    m_errorHandler->printMessage(KileTool::Info, title, "pdftk" );

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
    if(m_proc) {
        delete m_proc;
    }

    m_scriptmode = scriptmode;
    m_outputtext = "";

    m_proc = new KProcess();
    if (!dir.isEmpty()) {
        m_proc->setWorkingDirectory(dir);
    }
    m_proc->setShellCommand(command);
    m_proc->setOutputChannelMode(KProcess::MergedChannels);
    m_proc->setReadChannel(QProcess::StandardOutput);

    connect(m_proc, &QProcess::readyReadStandardOutput,
            this, &PdfDialog::slotProcessOutput);

    connect(m_proc, &QProcess::readyReadStandardError,
            this, &PdfDialog::slotProcessOutput);

    connect(m_proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &PdfDialog::slotProcessExited);

    connect(m_proc, &QProcess::errorOccurred,
            this, [this]() { slotProcessExited(-1, QProcess::CrashExit); });

    KILE_DEBUG_MAIN << "=== PdfDialog::runPdfutils() ====================";
    KILE_DEBUG_MAIN << "execute '" << command << "'";
    m_scriptrunning = true;
    m_rearrangeButton->setEnabled(false);
    m_buttonBox->button(QDialogButtonBox::Close)->setEnabled(false);
    m_proc->start();
}

void PdfDialog::slotProcessOutput()
{
    m_outputtext += m_proc->readAll();
}


void PdfDialog::slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(exitCode != 0 || exitStatus != QProcess::NormalExit) {
        if (m_scriptmode != PDF_SCRIPTMODE_TOOLS)
            showError(i18n("An error occurred while executing the task."));
    }
    else {
        bool state = ( exitCode == 0 );
        if ( m_scriptmode == PDF_SCRIPTMODE_TOOLS ) {
            initUtilities();
        }
#if !LIBPOPPLER_AVAILABLE
        else if ( m_scriptmode==PDF_SCRIPTMODE_NUMPAGES_PDFTK
                  || m_scriptmode==PDF_SCRIPTMODE_NUMPAGES_IMAGEMAGICK
                  || m_scriptmode==PDF_SCRIPTMODE_NUMPAGES_GHOSTSCRIPT ) {
            readNumberOfPages(m_scriptmode,m_outputtext);
        }
#endif
        else {
            finishPdfAction(state);
        }
    }

    m_scriptrunning = false;
    m_buttonBox->button(QDialogButtonBox::Close)->setEnabled(true);
    updateDialog();
}

void PdfDialog::finishPdfAction(bool state)
{
    // output window
    emit( output(m_outputtext) );

    // log window
    QString program = (m_scriptmode==PDF_SCRIPTMODE_ACTION && m_execLatex) ? "LaTeX with 'pdfpages' package" : "pdftk";

    if ( state ) {
        m_errorHandler->printMessage(KileTool::Info, "finished", program);

        // should we move the temporary pdf file
        if ( ! m_move_filelist.isEmpty() ) {
            QFile::remove( m_move_filelist[1] );
            QFile::rename( m_move_filelist[0], m_move_filelist[1] );
            KILE_DEBUG_MAIN << "move file: " << m_move_filelist[0] << " --->  " << m_move_filelist[1];
        }

        // run viewer
        if ( m_PdfDialog.m_cbView->isChecked() && m_scriptmode==PDF_SCRIPTMODE_ACTION ) {
            runViewer();
        }

        // file properties/permissions could be changed
        if ( (m_scriptmode==PDF_SCRIPTMODE_ACTION && m_PdfDialog.m_cbOverwrite->isChecked())
                || m_scriptmode==PDF_SCRIPTMODE_PROPERTIES || m_scriptmode==PDF_SCRIPTMODE_PERMISSIONS ) {
            slotInputfileChanged( m_PdfDialog.m_edInfile->lineEdit()->text().trimmed() );
        }
    }
    else {
        QString msg;
        if (m_outputtext.indexOf("OWNER PASSWORD") >= 0 ) {
            msg = i18n("Finished with an error (wrong password)");
        }
        else {
            msg = i18n("Finished with an error");
        }
        m_errorHandler->printMessage(KileTool::Error, msg, program);
    }
}

void PdfDialog::runViewer()
{
    m_errorHandler->printMessage(KileTool::Info, "Running viewer", "ViewPDF");

    // call ViewPDF
    QString cfg = KileTool::configName("ViewPDF", m_manager->config());
    KileTool::View *tool = dynamic_cast<KileTool::View*>(m_manager->createTool("ViewPDF", cfg, false));
    if(!tool) {
        m_errorHandler->printMessage(KileTool::Error, i18n("Could not create the ViewPDF tool"), i18n("ViewPDF"));
        return;
    }
    tool->setFlags(0);
    tool->setSource(m_outputfile);
    m_manager->run(tool);
}

QString PdfDialog::buildActionCommand()
{
    // build action: parameter
    m_execLatex = true;           // default
    m_inputfile = m_PdfDialog.m_edInfile->lineEdit()->text().trimmed();
    m_outputfile = m_PdfDialog.m_edOutfile->lineEdit()->text().trimmed();

    QColor bgcolor;
    QString bgfile;
    int taskindex = taskIndex();
    switch (taskindex) {
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

    case PDF_DECRYPT:
        m_param.clear();
        m_execLatex = false;
        break;

    case PDF_SELECT:
    case PDF_DELETE:
        m_param = ( taskindex == PDF_SELECT ) ? buildSelectPageList() : buildDeletePageList();
        if ( m_pdftk ) {
            m_param = "cat " + m_param.replace(","," ");
            m_execLatex = false;
        }
        else {
            m_param = "pages={" + m_param + "}";
        }
        break;

    case PDF_PDFTK_BACKGROUND:
        m_param = "background \"" + m_PdfDialog.m_edStamp->text().trimmed() + "\"";
        m_execLatex = false;
        break;

    case PDF_PDFTK_BGCOLOR:
        bgcolor = m_PdfDialog.m_pbBackgroundColor->color();
        bgfile = buildPdfBackgroundFile(&bgcolor);
        m_param = "background " + bgfile;
        m_execLatex = false;
        break;

    case PDF_PDFTK_STAMP:
        m_param = "stamp \"" + m_PdfDialog.m_edStamp->text().trimmed() + "\"";
        m_execLatex = false;
        break;

    case PDF_PDFTK_FREE:
        m_param = m_PdfDialog.m_edParameter->text().trimmed();
        m_execLatex = false;
        break;

    case PDF_PDFPAGES_FREE:
        m_param = m_PdfDialog.m_edParameter->text().trimmed();
        break;
    }

    // build action: command
    QString command,latexfile,pdffile;
    if ( m_execLatex ) {
        latexfile = buildLatexFile(m_param);
        pdffile = latexfile + ".pdf";
        command = "pdflatex -interaction=nonstopmode " + latexfile + ".tex";
    }
    else {
        pdffile = m_tempdir->path() + QFileInfo(m_inputfile).baseName() + "-temp.pdf";
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
    if (equalfiles) {
        m_outputfile = m_inputfile;
    }

    // move destination file
    m_move_filelist.clear();
    if ( equalfiles ) {
        m_move_filelist << pdffile << m_inputfile;
    }
    else if ( !m_outputfile.isEmpty() ) {
        m_move_filelist << pdffile << m_outputfile;
    }

    // viewer
    if ( viewer && m_outputfile.isEmpty() ) {
        m_outputfile = pdffile;
    }

    return command;
}

// create a temporary file to run latex with package pdfpages.sty
QString PdfDialog::buildLatexFile(const QString &param)
{
    QTemporaryFile temp(m_tempdir->path() + QLatin1String("/kile-pdfdialog-XXXXXX.tex"));
    temp.setAutoRemove(false);

    if(!temp.open()) {
        KILE_DEBUG_MAIN << "Could not create tempfile in PdfDialog::buildLatexFile()" ;
        return QString();
    }
    QString tempname = temp.fileName();

    QTextStream stream(&temp);
    stream << "\\documentclass[a4paper,12pt]{article}\n";
    stream << "\\usepackage[final]{pdfpages}\n";
    stream << "\\begin{document}\n";
    stream << "\\includepdf[" << param << "]{" << m_inputfile << "}\n";
    stream << "\\end{document}\n";

    // everything is prepared to do the job
    temp.close();
    return(tempname.left(tempname.length() - 4));
}

// create a temporary pdf file to set a background color
QString PdfDialog::buildPdfBackgroundFile(QColor *color)
{
    QTemporaryFile temp(m_tempdir->path() + QLatin1String("/kile-pdfdialog-XXXXXX.pdf"));
    temp.setAutoRemove(false);

    if(!temp.open()) {
        KILE_DEBUG_MAIN << "Could not create tempfile in PdfDialog::buildPdfBackgroundFile()" ;
        return QString();
    }
    QString tempname = temp.fileName();

    QTextStream stream(&temp);
    stream << "%PDF-1.4\n";
    stream << '%' << '\0' << '\0' << '\0' << '\0' << '\r';
    stream << "5 0 obj \n"
           "<<\n"
           "/Type /ExtGState\n"
           "/OPM 1\n"
           ">>\n"
           "endobj \n"
           "4 0 obj \n"
           "<<\n"
           "/R7 5 0 R\n"
           ">>\n"
           "endobj \n"
           "6 0 obj \n"
           "<<\n"
           "/Length 83\n"
           ">>\n"
           "stream\n"
           "q 0.1 0 0 0.1 0 0 cm\n"
           "/R7 gs\n";
    stream << color->redF() << " " << color->greenF() << " " << color->blueF() << " rg\n";
    stream << "0 0 " << 10*m_pagesize.width() << " " << 10*m_pagesize.height() << " re\n";
    stream << "f\n"
           "0 g\n"
           "Q\n"
           "\n"
           "endstream \n"
           "endobj \n"
           "3 0 obj \n"
           "<<\n"
           "/Parent 1 0 R\n";
    stream << "/MediaBox [0 0 " << m_pagesize.width() << " " << m_pagesize.height() << "]\n";
    stream << "/Resources \n"
           "<<\n"
           "/ExtGState 4 0 R\n"
           "/ProcSet [/PDF]\n"
           ">>\n"
           "/pdftk_PageNum 1\n"
           "/Type /Page\n"
           "/Contents 6 0 R\n"
           ">>\n"
           "endobj \n"
           "1 0 obj \n"
           "<<\n"
           "/Kids [3 0 R]\n"
           "/Count 1\n"
           "/Type /Pages\n"
           ">>\n"
           "endobj \n"
           "7 0 obj \n"
           "<<\n"
           "/Pages 1 0 R\n"
           "/Type /Catalog\n"
           ">>\n"
           "endobj \n"
           "8 0 obj \n"
           "<<\n"
           "/Creator ()\n"
           "/Producer ())\n"
           "/ModDate ()\n"
           "/CreationDate ()\n"
           ">>\n"
           "endobj xref\n"
           "0 9\n"
           "0000000000 65535 f \n"
           "0000000388 00000 n \n"
           "0000000000 65536 n \n"
           "0000000231 00000 n \n"
           "0000000062 00000 n \n"
           "0000000015 00000 n \n"
           "0000000095 00000 n \n"
           "0000000447 00000 n \n"
           "0000000498 00000 n \n"
           "trailer\n"
           "\n"
           "<<\n"
           "/Info 8 0 R\n"
           "/Root 7 0 R\n"
           "/Size 9\n"
           "/ID [<4a7c31ef3aeb884b18f59c2037a752f5><54079f85d95a11f3400fe5fc3cfc832b>]\n"
           ">>\n"
           "startxref\n"
           "721\n"
           "%%EOF\n";

    // everything is prepared to do the job
    temp.close();
    return tempname;
}

QString PdfDialog::buildPageRange(int type)
{
    QString s;
    for (int i = 1; i <= m_numpages; ++i) {
        if (type == PDF_PAGE_EMPTY) {
            s += QString("%1,{},").arg(i);
        }
        else {
            s += QString("%1,%2,").arg(i).arg(i);
        }
    }

    return "{" + s.left(s.length()-1) + "}";
}

QString PdfDialog::buildPageList(bool even)
{
    QString s, number;

    int start = ( even ) ? 2 : 1;
    for (int i=start; i<=m_numpages; i+=2 ) {
        s += number.setNum(i) + ',';
    }

    if ( !s.isEmpty() ) {
        s.truncate(s.length()-1);
    }
    return "{" + s + "}";
}

QString PdfDialog::buildReversPageList(bool even)
{
    QString s,number;

    int last = m_numpages;
    if ( even ) {
        if ( (last & 1) == 1 ) {
            last--;
        }
    }
    else {
        if ( (last & 1) == 0 ) {
            last--;
        }
    }

    for (int i=last; i>=1; i-=2 ) {
        s += number.setNum(i) + ",";
    }

    if ( !s.isEmpty() ) {
        s.truncate(s.length()-1);
    }
    return "{" + s + "}";
}

QString PdfDialog::buildSelectPageList()
{
    return m_PdfDialog.m_edParameter->text().trimmed();
}

QString PdfDialog::buildDeletePageList()
{
    // m_numpages is known
    QString param = m_PdfDialog.m_edParameter->text().trimmed();
    QRegExp re("(\\d+)-(\\d+)");

    // analyze delete list
    bool ok;
    QBitArray arr(m_numpages + 1,false);
    QStringList pagelist = param.split(',');
    foreach (const QString &s, pagelist) {
        if ( s.contains('-') && re.indexIn(s) >= 0 ) {
            int from = re.cap(1).toInt(&ok);
            int to = re.cap(2).toInt(&ok);
            for (int i=from; i<=to; ++i) {
                arr.setBit(i);
            }
        }
        else {
            arr.setBit(s.toInt(&ok));
        }
    }

    // build select list
    QString result;
    int page = 1;
    while ( page <= m_numpages ) {
        int from = searchPages(&arr,page,m_numpages,true);
        if ( from > m_numpages ) {
            break;
        }
        int to = searchPages(&arr,from+1,m_numpages,false) - 1;
        if ( !result.isEmpty() ) {
            result += ',';
        }
        if ( from < to ) {
            result += QString::number(from) + '-' + QString::number(to);
        }
        else {
            result += QString::number(from);
        }
        page = to + 1;
    }

    return result;
}

int PdfDialog::searchPages(QBitArray *arr, int page, int lastpage, bool value)
{
    while ( page <= lastpage ) {
        if ( arr->at(page) != value ) {
            return page;
        }
        page++;
    }
    return lastpage + 1;
}

bool PdfDialog::checkParameter()
{
    if ( !checkInputFile() ) {
        return false;
    }

    if ( m_encrypted ) {
        if ( !checkPassword() ) {
            return false;
        }
    }

    // check parameter
    int taskindex = taskIndex();
    if ( isParameterTask(taskindex) && m_PdfDialog.m_edParameter->text().trimmed().isEmpty() ) {
        showError( i18n("The utility needs some parameters in this mode.") );
        return false;
    }

    // check select/delete page list (m_numpages is known)
    if ( taskindex==PDF_SELECT || taskindex==PDF_DELETE ) {
        // m_numpages is known
        QString param = m_PdfDialog.m_edParameter->text().trimmed();
        QRegExp re("(\\d+)-(\\d+)");

        // analyze page list
        bool ok;
        QStringList pagelist = param.split(',');
        foreach (const QString &s, pagelist) {
            if ( s.contains('-') && re.indexIn(s)>=0 ) {
                int from = re.cap(1).toInt(&ok);
                int to = re.cap(2).toInt(&ok);
                if ( from > to ) {
                    showError(i18n("Illegal page list 'from-to': %1 is bigger than %2.",from,to));
                    return false;
                }
                if ( to > m_numpages ) {
                    showError(i18n("Illegal pagenumber: %1.",to));
                    return false;
                }
            }
            else {
                int page = s.toInt(&ok);
                if ( page > m_numpages ) {
                    showError(i18n("Illegal pagenumber: %1.",page));
                    return false;
                }
            }
        }
    }

    // check background/stamp parameter
    if ( isOverlayTask(taskindex) ) {
        QString filename = m_PdfDialog.m_edStamp->text().trimmed();

        if ( filename.isEmpty() ) {
            QString message = ( taskindex == PDF_PDFTK_STAMP )
                              ? i18n("You need to define a PDF file as foreground stamp.")
                              : i18n("You need to define a PDF file as background watermark.");
            showError(message);
            return false;
        }

        QFileInfo fs(filename);
        if (fs.completeSuffix() != "pdf") {
            showError(i18n("Unknown file format: only '.pdf' is accepted as image file in this mode."));
            return false;
        }

        if ( !QFile::exists(filename) ) {
            showError(i18n("The given file doesn't exist."));
            return false;
        }
    }

    // overwrite mode: no output file is needed
    if ( m_PdfDialog.m_cbOverwrite->isChecked() ) {
        return true;
    }

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

// check tasks
bool PdfDialog::isParameterTask(int task)
{
    return ( task==PDF_SELECT || task==PDF_DELETE || task==PDF_PDFPAGES_FREE || task==PDF_PDFTK_FREE );
}

bool PdfDialog::isOverlayTask(int task)
{
    return ( task==PDF_PDFTK_BACKGROUND || task==PDF_PDFTK_STAMP );
}

bool PdfDialog::isBackgroundColor(int task)
{
    return ( task == PDF_PDFTK_BGCOLOR ) ? true : false;
}

bool PdfDialog::isFreeTask(int task)
{
    return ( task==PDF_PDFPAGES_FREE || task==PDF_PDFTK_FREE );
}

}
