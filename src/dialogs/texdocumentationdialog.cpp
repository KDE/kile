/***************************************************************************
  Copyright (C) 2005-2007 by Holger Danielsson (holger.danielsson@t-online.de)
            (C) 2014-2022 by Michel Ludwig (michel.ludwig@kdemail.net)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dialogs/texdocumentationdialog.h"
#include "kileconstants.h"
#include "kiledebug.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KApplicationTrader>
#include <KProcess>
#include <KRun>
#include <KService>

#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QFileInfo>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMimeDatabase>
#include <QMimeType>
#include <QPushButton>
#include <QRegExp>
#include <QTemporaryFile>
#include <QTreeWidget>
#include <QUrl>
#include <QVBoxLayout>

namespace KileDialog
{

TexDocDialog::TexDocDialog(QWidget *parent)
    : QDialog(parent)
    , m_buttonBox(new QDialogButtonBox(QDialogButtonBox::RestoreDefaults|QDialogButtonBox::Close))
    , m_tempfile(Q_NULLPTR)
    , m_proc(Q_NULLPTR)
{
    setWindowTitle(i18n("Documentation Browser"));
    setModal(true);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    // listview
    m_texdocs = new QTreeWidget(this);
    mainLayout->addWidget(m_texdocs);
    m_texdocs->setRootIsDecorated(true);
    m_texdocs->setHeaderLabel(i18n("Table of Contents"));

    // groupbox
    QGroupBox *groupbox = new QGroupBox(i18n("Search"), this);
    mainLayout->addWidget(groupbox);
    QHBoxLayout *groupboxLayout = new QHBoxLayout();
    groupboxLayout->setAlignment(Qt::AlignTop);
    groupbox->setLayout(groupboxLayout);

    m_leKeywords = new QLineEdit(groupbox);
    m_leKeywords->setPlaceholderText("Keyword");
    m_leKeywords->setClearButtonEnabled(true);
    m_pbSearch = new QPushButton(i18n("&Search"), groupbox);

    groupboxLayout->addWidget(m_leKeywords);
    groupboxLayout->addWidget(m_pbSearch);

    m_texdocs->setWhatsThis(i18n("<p>A list of the documentation provided by the installed TeX distribution.</p>"
                                 "<p>Double clicking on an item or pressing the space key will open a viewer to show the corresponding file.</p>"
                                 "<p>Items that are grayed out are not installed.</p>"));
    m_leKeywords->setWhatsThis(i18n("You can choose a keyword to show only document files that are related to this keyword."));
    m_pbSearch->setWhatsThis(i18n("Start the search for the chosen keyword."));
    m_pbSearch->setEnabled(false);
    m_buttonBox->button(QDialogButtonBox::RestoreDefaults)->setWhatsThis(i18n("Reset list to all available documentation files."));
    m_buttonBox->button(QDialogButtonBox::RestoreDefaults)->setText(i18n("Cancel &Search"));
    m_buttonBox->button(QDialogButtonBox::RestoreDefaults)->setEnabled(false);
    connect(m_buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked,
            this, &TexDocDialog::slotResetSearch);

    // catch some Return/Enter events
    m_texdocs->installEventFilter(this);
    m_leKeywords->installEventFilter(this);

    connect(m_texdocs, &QTreeWidget::itemDoubleClicked, this, &TexDocDialog::slotListViewDoubleClicked);
    connect(m_pbSearch, &QPushButton::clicked, this, &TexDocDialog::slotSearchClicked);
    connect(m_leKeywords, &QLineEdit::textChanged, this, &TexDocDialog::slotTextChanged);

    m_texmfPath.clear();
    m_texmfdocPath.clear();
    m_texdoctkPath.clear();

    connect(this, &TexDocDialog::processFinished, this, &TexDocDialog::slotInitToc);
    executeScript(
        "kpsewhich --progname=texdoctk --format='other text files' texdoctk.dat && "
        "kpsewhich --expand-path='$TEXMF/doc' && "
        "kpsewhich --expand-path='$TEXMF'"
    );

    mainLayout->addWidget(m_texdocs);
    mainLayout->addWidget(groupbox);
    mainLayout->addWidget(m_buttonBox);

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

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
    if (!fin.exists() || !fin.open(QIODevice::ReadOnly)) {
        KMessageBox::error(this, i18n("Could not read 'texdoctk.dat'."));
        return;
    }

    // use a textstream to read all data
    QString textline;
    QTextStream data(&fin);
    while (!data.atEnd()) {
        textline = data.readLine();
        if (!(textline.isEmpty() || textline[0] == '#')) {
            // save the whole entry
            m_tocList.append(textline);

            // list entries 0,1,basename(2),3 are needed for keyword search
            // (key,title,filepath,keywords)
            QStringList list = textline.split(';', Qt::KeepEmptyParts);

            // get basename of help file
            QString basename;
            if (list.count() > 2) {
                QFileInfo fi(list[2]);
                basename = fi.baseName().toLower();
            }
            else {
                if (list.count() < 2) {
                    continue;
                }
            }
            QString entry = list[0] + ';' + list[1];
            if (!basename.isEmpty()) {
                entry += ';' + basename;
            }
            if (list.count() > 3) {
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
    QTreeWidgetItem *itemsection = Q_NULLPTR;

    setUpdatesEnabled(false);
    m_texdocs->setHeaderLabel(caption);

    for (int i = 0; i < doclist.count(); ++i) {
        if (doclist[i][0] == '@') {
            section = doclist[i];
            itemsection = new QTreeWidgetItem(m_texdocs, QStringList(section.remove(0, 1)));
        }
        else {
            keylist = doclist[i].split(';', Qt::KeepEmptyParts);
            if (keylist.size() < 4) {
                continue;
            }
            if (itemsection) {
                QTreeWidgetItem *item = new QTreeWidgetItem(itemsection, QStringList() << keylist[1] << keylist[0]);
                item->setIcon(0, QIcon::fromTheme(getIconName(keylist[2])));

                QString filename = findFile(keylist[2]);
                if(filename.isEmpty()) {
                    item->setDisabled(true);
                }

                // save filename in dictionary
                m_dictDocuments[keylist[0]] = filename;
            }
        }
    }
    setUpdatesEnabled(true);

    if (toc) {
        m_pbSearch->setEnabled(false);
    }
    m_buttonBox->button(QDialogButtonBox::RestoreDefaults)->setEnabled(!toc);
    m_texdocs->setFocus();

    if (m_texdocs->topLevelItemCount() == 1) {
        m_texdocs->expandAll();
    }
}

bool TexDocDialog::eventFilter(QObject *o, QEvent *e)
{
    // catch KeyPress events
    if (e->type() == QEvent::KeyPress) {
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
        if (o == m_leKeywords) {
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
    const QStringList pathlist  = listofpaths.split(LIST_SEPARATOR);
    const QString extensions[] = {"", QLatin1String(".gz"), QLatin1String(".bz2")};

    QString filename;
    for(const QString& itp : pathlist) {
        for(const QString& ite : extensions) {
            filename = (subdir.isEmpty()) ? itp + '/' + docfilename + ite
                       : itp + '/' + subdir + '/' + docfilename + ite;

            if(QFile::exists(filename)) {
                return filename;
            }
        }
    }

    return QString();
}

QString TexDocDialog::findFile(const QString &docfilename)
{
    QString filename = searchFile(docfilename, m_texmfdocPath);
    if(filename.isEmpty()) {
        // not found: search it elsewhere
        filename = searchFile(docfilename, m_texmfPath, "tex");
        if(filename.isEmpty()) {
            return QString();
        }
        return filename;
    }
    return filename;
}

void TexDocDialog::showFile(const QString &filename)
{
    KILE_DEBUG_MAIN << "\tshow file: " << filename << Qt::endl;
    if (QFile::exists(filename)) {
        QUrl url;
        url.setPath(filename);

        KService::List offers = KApplicationTrader::queryByMimeType(getMimeType(filename));
        if(offers.isEmpty()) {
            KMessageBox::error(this, i18n("No KDE service found for this file."));
            return;
        }
        QList<QUrl> lst;
        lst.append(url);
        KRun::runService(*(offers.first()), lst, this, true);
    }
}


////////////////////// Slots //////////////////////

void TexDocDialog::slotListViewDoubleClicked(QTreeWidgetItem *item)
{
    if (!item->parent() || item->isDisabled()) {
        return;
    }

    QString package = item->text(1);
    KILE_DEBUG_MAIN << "\tselect child: "  << item->text(0) << Qt::endl
                    << "\tis package: " << package << Qt::endl;
    if (!m_dictDocuments.contains(package)) {
        return;
    }

    QString filename = m_dictDocuments[package];
    KILE_DEBUG_MAIN << "\tgot filename:" << filename << Qt::endl;
    if(filename.isEmpty()) {
        KMessageBox::error(this, i18n("Could not find the documentation file '%1'", filename));
        return;
    }

    showFile(filename);
}

void TexDocDialog::slotTextChanged(const QString &text)
{
    m_pbSearch->setEnabled(! text.trimmed().isEmpty());
}

void TexDocDialog::slotSearchClicked()
{
    QString keyword = m_leKeywords->text().trimmed();
    if (keyword.isEmpty()) {
        KMessageBox::error(this, i18n("No keyword given."));
        return;
    }

    QString section;
    bool writesection = true;
    QStringList searchlist;

    for (int i = 0; i < m_tocList.count(); i++) {
        if (m_tocList[i][0] == '@') {
            section = m_tocList[i];
            writesection = true;
        }
        else {
            if (i < m_tocSearchList.count() && m_tocSearchList[i].indexOf(keyword, 0, Qt::CaseInsensitive) > -1) {
                if (writesection) {
                    searchlist.append(section);
                }
                searchlist.append(m_tocList[i]);
                writesection = false;
            }
        }
    }

    if (searchlist.count() > 0) {
        m_texdocs->clear();
        showToc(i18n("Search results for keyword '%1'", keyword), searchlist, false);
    }
    else {
        KMessageBox::error(this, i18n("No documents found for keyword '%1'.", keyword));
    }
}

void TexDocDialog::slotResetSearch()
{
    m_leKeywords->setText(QString());
    m_texdocs->clear();
    showToc(i18n("Table of Contents"), m_tocList, true);
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
    if (m_proc) {
        delete m_proc;
    }

    m_proc = new KProcess();
    m_proc->setShellCommand(command);
    m_proc->setOutputChannelMode(KProcess::MergedChannels);
    m_proc->setReadChannel(QProcess::StandardOutput);
    m_output.clear();

    connect(m_proc, &KProcess::readyReadStandardOutput,
            this, &TexDocDialog::slotProcessOutput);
    connect(m_proc, &KProcess::readyReadStandardError,
            this, &TexDocDialog::slotProcessOutput);
    connect(m_proc, static_cast<void (QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished),
            this, &TexDocDialog::slotProcessExited);

    KILE_DEBUG_MAIN << "=== TexDocDialog::runShellScript() ====================" << Qt::endl;
    KILE_DEBUG_MAIN << "   execute: " << command << Qt::endl;
    m_proc->start();
}

void TexDocDialog::slotProcessOutput()
{
    m_output += m_proc->readAll();
}

void TexDocDialog::slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);

    if (exitStatus == QProcess::NormalExit) {
        //showFile(m_filename);
        emit(processFinished());
    }
    else {
        KMessageBox::error(this, i18n("<center>") + i18n("Could not determine the search paths of TexLive/teTeX or file 'texdoctk.dat'.<br/>"
                                                         "Unfortunately, Kile cannot show any useful information.") + i18n("</center>"), i18n("TexDoc Dialog"));
    }
}

////////////////////// process slots, when finished //////////////////////

void TexDocDialog::slotInitToc()
{
    disconnect(this, &TexDocDialog::processFinished, this, &TexDocDialog::slotInitToc);

    QStringList results = m_output.split('\n', Qt::KeepEmptyParts);
    if (results.count() < 3) {
        KMessageBox::error(this, i18n("Could not determine the installation path of your TeX distribution or find the file 'texdoctk.dat'.<br/>"
                                      "Hence, we cannot provide you with an overview of the installed TeX documentation."));
        return;
    }

    m_texdoctkPath = results[0];
    m_texmfdocPath = results[1];
    m_texmfPath = results[2];

    KILE_DEBUG_MAIN << "\ttexdoctk path: " << m_texdoctkPath << Qt::endl;
    KILE_DEBUG_MAIN << "\ttexmfdoc path: " << m_texmfdocPath << Qt::endl;
    KILE_DEBUG_MAIN << "\ttexmf path: " << m_texmfPath << Qt::endl;

    if(m_texdoctkPath.indexOf('\n', -1) > -1) {
        m_texdoctkPath.truncate(m_texdoctkPath.length() - 1);
    }

    // read data and initialize listview
    readToc();
    slotResetSearch();
}

void TexDocDialog::slotShowFile()
{
    disconnect(this, &TexDocDialog::processFinished, this, &TexDocDialog::slotShowFile);
    showFile(m_filename);
}

////////////////////// Icon/Mime //////////////////////

QString TexDocDialog::getMimeType(const QString &filename)
{
    QFileInfo fi(filename);
    QString basename = fi.baseName().toLower();
    QString ext = fi.suffix().toLower();

    QString mimetype;
    if (ext == "txt" || ext == "faq" || ext == "sty" || basename == "readme" || basename == "00readme") {
        mimetype = "text/plain";
    }
    else {
        QUrl mimeurl;
        mimeurl.setPath(filename);
        QMimeDatabase db;
        QMimeType pMime = db.mimeTypeForUrl(mimeurl);
        mimetype = pMime.name();
    }

    KILE_DEBUG_MAIN << "\tmime = "  << mimetype << " " << Qt::endl;
    return mimetype;
}

QString TexDocDialog::getIconName(const QString &filename)
{
    QFileInfo fi(filename);
    QString basename = fi.baseName().toLower();
    QString ext = fi.suffix().toLower();

    QString icon;
    if (ext == "application-x-bzdvi" ) { // FIXME exchange as soon as a real dvi icon is available
        icon = ext;
    }
    else if( ext == "htm" || ext == "html" ) {
        icon = "text-html";
    }
    else if(ext == "pdf" ) {
        icon = "application-pdf";
    }
    else if( ext == "txt") {
        ext = "text-plain";
    }
    else if(ext == "ps") {
        icon = "application-postscript";
    }
    else if(ext == "sty") {
        icon = "text-x-tex";
    }
    else if(ext == "faq" || basename == "readme" || basename == "00readme") {
        icon = "text-x-readme";
    }
    else {
        icon = "text-plain";
    }
    return icon;
}

}
