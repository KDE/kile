/*******************************************************************************************
    begin                : Sat Apr 26 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (wijnhout@science.uva.nl)
                               2005 by Holger Danielsson (holger.danielsson@t-online.de)
                               2007-2024 by Michel Ludwig (michel.ludwig@kdemail.net)
 *******************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "templates.h"

#include <QDir>
#include <QFileInfo>
#include <QStringList>

#include <KLocalizedString>
#include <KMessageBox>
#include <KProcess>
#include <KShell>
#include <KIO/FileCopyJob>
#include <KIO/SimpleJob>
#include <KJobWidgets>
#include <QTemporaryFile>

#include "kileinfo.h"
#include "kiledebug.h"
#include "utilities.h"

// 2005-08-04: dani
//  - added script support to search existing class files
//    (classes: Koma, Beamer, Prosper, HA-prosper)
//  - sort items ('Empty Document' will always be the first entry)

// 2006-30-04: tbraun
//  - drag and drop makes no sense here
//  - use the Select mode

namespace KileTemplate {

////////////////////// Info //////////////////////

Info::Info() : type(KileDocument::Undefined)
{
}

bool Info::operator==(const Info &ti) const
{
    return name==ti.name;
}

////////////////////// Manager //////////////////////

Manager::Manager(KileInfo* kileInfo, QObject* parent, const char* name) : QObject(parent), m_kileInfo(kileInfo)
{
    setObjectName(name);
}

Manager::~Manager() {
}

bool Manager::copyAppData(const QUrl &src, const QString& subdir, const QString& fileName)
{
    //let saveLocation find and create the appropriate place to
    //store the templates (usually $HOME/.kde/share/apps/kile/templates)
    QString dir = KileUtilities::writableLocation(QStandardPaths::AppDataLocation) + QLatin1Char('/') + subdir;

    QUrl targetURL = QUrl::fromUserInput(dir);
    targetURL = targetURL.adjusted(QUrl::StripTrailingSlash);
    targetURL.setPath(targetURL.path() + QLatin1Char('/') + fileName);

    //if a directory is found
    if (!dir.isNull()) {
        // create dir if not existing, needed for copyjob
        QDir testDir(dir);
        if (!testDir.exists()) {
            testDir.mkpath(dir);
        }
        // copy file
        if(src == targetURL) { // copying a file over itself
            return true;
        }
        KIO::FileCopyJob* copyJob = KIO::file_copy(src, targetURL, -1, KIO::Overwrite);
        KJobWidgets::setWindow(copyJob, m_kileInfo->mainWindow());
        return copyJob->exec();
    }
    else {
        KMessageBox::error(nullptr, i18n("Could not find a folder to save %1 to.\nCheck whether you have a folder named \".kde\" with write permissions in your home folder.", fileName));
        return false;
    }
}

bool Manager::removeAppData(const QString &file) {
    QFileInfo fileInfo(file);
    if(fileInfo.exists()) {
        KIO::SimpleJob* deleteJob = KIO::file_delete(QUrl::fromUserInput(file));
        KJobWidgets::setWindow(deleteJob, m_kileInfo->mainWindow());
        return deleteJob->exec();
    }
    return true;
}

bool Manager::searchForTemplate(const QString& name, KileDocument::Type& type) const {
    for (KileTemplate::TemplateListConstIterator i = m_TemplateList.constBegin(); i != m_TemplateList.constEnd(); ++i)
    {
        KileTemplate::Info info = *i;
        if(info.name == name && info.type == type) {
            return true;
        }
    }
    return false;
}

bool Manager::add(const QUrl &templateSourceURL, const QString &name, const QUrl &icon) {
    KileDocument::Extensions *extensions = m_kileInfo->extensions();
    KileDocument::Type type = extensions->determineDocumentType(templateSourceURL);
    return add(templateSourceURL, type, name, icon);
}

bool Manager::add(const QUrl &templateSourceURL, KileDocument::Type type, const QString &name, const QUrl &icon) {
    KileDocument::Extensions *extensions = m_kileInfo->extensions();
    QString extension = extensions->defaultExtensionForDocumentType(type);

    return copyAppData(templateSourceURL, QStringLiteral("templates"), QStringLiteral("template_") + name + extension)
      && copyAppData(icon, QStringLiteral("pics"), QStringLiteral("type_") + name + extension + QStringLiteral(".kileicon"));
}

bool Manager::remove(Info ti) {
    return removeAppData(ti.path) && removeAppData(ti.icon);
}

void Manager::scanForTemplates() {
    KILE_DEBUG_MAIN << "===scanForTemplates()===================";
    QStringList dirs = KileUtilities::locateAll(QStandardPaths::AppDataLocation, QStringLiteral("templates"), QStandardPaths::LocateDirectory);
    QDir templates;
    KileTemplate::Info ti;
    const KileDocument::Extensions *extensions = m_kileInfo->extensions();

    m_TemplateList.clear();
    for(QStringList::iterator i = dirs.begin(); i != dirs.end(); ++i) {
        templates = QDir(*i, QStringLiteral("template_*"));
        for (uint j = 0; j < templates.count(); ++j) {
            ti.path = templates.path() + QLatin1Char('/') + templates[j];
            QFileInfo fileInfo(ti.path);
            ti.name = fileInfo.completeBaseName().mid(9); //remove "template_", do it this way to avoid problems with user input!
            ti.type = extensions->determineDocumentType(QUrl::fromUserInput(ti.path));
            ti.icon = KileUtilities::locate(QStandardPaths::AppDataLocation, QStringLiteral("pics/type_") + ti.name + extensions->defaultExtensionForDocumentType(ti.type) + QStringLiteral(".kileicon"));
            if (m_TemplateList.contains(ti)) {
                KILE_DEBUG_MAIN << "\tignoring: " << ti.path;
            }
            else {
                m_TemplateList.append(ti);
                KILE_DEBUG_MAIN << "\tadding: " << ti.name << " " << ti.path;
            }
        }
    }
}

QString Manager::defaultEmptyTemplateCaption()
{
    return i18n("Empty File");
}

QString Manager::defaultEmptyLaTeXTemplateCaption()
{
    return i18n("Empty LaTeX File");
}

QString Manager::defaultEmptyBibTeXTemplateCaption()
{
    return i18n("Empty BibTeX File");
}

TemplateList Manager::getAllTemplates() const {
    return m_TemplateList;
}

TemplateList Manager::getTemplates(KileDocument::Type type) const {
    if(type == KileDocument::Undefined) {
        return getAllTemplates();
    }

    TemplateList toReturn;
    for (KileTemplate::TemplateListConstIterator i = m_TemplateList.constBegin(); i != m_TemplateList.constEnd(); ++i) {
        KileTemplate::Info info = *i;
        if(info.type == type) {
            toReturn.push_back(info);
        }
    }
    return toReturn;
}

}
////////////////////// TemplateItem //////////////////////

// new compare function to make the "Empty (...) Document" items appear at the beginning

TemplateItem::TemplateItem(QListWidget * parent, const KileTemplate::Info& info)
    : QListWidgetItem(QPixmap(info.icon), info.name, parent)
{
    m_info = info;
}

bool TemplateItem::operator<(const QListWidgetItem &other) const
{
    if(text() == KileTemplate::Manager::defaultEmptyTemplateCaption()) {
        return true;
    }
    else if(other.text() == KileTemplate::Manager::defaultEmptyTemplateCaption()) {
        return false;
    }
    else {
        return QListWidgetItem::operator<(other);
    }
}

////////////////////// TemplateIconView //////////////////////

TemplateIconView::TemplateIconView(QWidget *parent)
    : QListWidget(parent), m_templateManager(nullptr), m_proc(nullptr) {
    setViewMode(QListView::IconMode);
    setMovement(QListView::Static);
    setResizeMode(QListView::Adjust);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setFlow(QListView::TopToBottom);
    setMinimumHeight(100);
    setIconSize(QSize(48, 48));
}

TemplateIconView::~TemplateIconView() {
}

void TemplateIconView::setTemplateManager(KileTemplate::Manager *templateManager) {
    m_templateManager = templateManager;
}

void TemplateIconView::fillWithTemplates(KileDocument::Type type) {
    if(!m_templateManager) {
        return;
    }

    clear();

    if(type == KileDocument::LaTeX) {
        searchLaTeXClassFiles();
    }
    else {
        addTemplateIcons(type);
    }
}

void TemplateIconView::searchLaTeXClassFiles()
{
    if(!m_templateManager) {
        return;
    }

    m_output.clear();

    QString command = QStringLiteral("kpsewhich -format=tex scrartcl.cls beamer.cls prosper.cls HA-prosper.sty");

    delete m_proc;

    m_proc = new KProcess(this);
    (*m_proc) << KShell::splitArgs(command);

    m_proc->setOutputChannelMode(KProcess::MergedChannels);
    m_proc->setReadChannel(QProcess::StandardOutput);

    connect(m_proc, SIGNAL(readyReadStandardOutput()), this, SLOT(slotProcessOutput()));
    connect(m_proc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotProcessExited(int,QProcess::ExitStatus)));
    connect(m_proc, &KProcess::errorOccurred, this, &TemplateIconView::slotProcessError);
    KILE_DEBUG_MAIN << "=== NewFileWidget::searchClassFiles() ====================";
    KILE_DEBUG_MAIN << "\texecute: " << command;
    m_proc->start();
}

void TemplateIconView::slotProcessOutput()
{
    QByteArray buf = m_proc->readAllStandardOutput();
    m_output += QString::fromLocal8Bit(buf.data(), buf.size());
}

void TemplateIconView::slotProcessError()
{
    addTemplateIcons(KileDocument::LaTeX);
    Q_EMIT classFileSearchFinished();
}

void TemplateIconView::slotProcessExited(int /*exitCode*/, QProcess::ExitStatus exitStatus)
{
    if(exitStatus != QProcess::NormalExit) {
        m_output.clear();
    }

    addTemplateIcons(KileDocument::LaTeX);
    Q_EMIT classFileSearchFinished();
}

void TemplateIconView::addTemplateIcons(KileDocument::Type type)
{
    if(!m_templateManager) {
        return;
    }

    QString emptyIcon = KileUtilities::locate(QStandardPaths::AppDataLocation, QStringLiteral("pics/") + QStringLiteral(DEFAULT_EMPTY_ICON) + QStringLiteral(".png"));

    KileTemplate::Info emptyDocumentInfo;
    emptyDocumentInfo.name = KileTemplate::Manager::defaultEmptyTemplateCaption();
    emptyDocumentInfo.icon = emptyIcon;
    emptyDocumentInfo.type = type;
    TemplateItem *emp = new TemplateItem(this, emptyDocumentInfo);
    setCurrentItem(emp);

    if(type == KileDocument::LaTeX) {
        // disable non standard templates
        QMap<QString,bool> map;
        map[QStringLiteral("Scrartcl")] = false;
        map[QStringLiteral("Scrbook")]  = false;
        map[QStringLiteral("Scrreprt")] = false;
        map[QStringLiteral("Scrlttr2")] = false;
        map[QStringLiteral("Beamer")]   = false;
        map[QStringLiteral("Prosper")]  = false;
        map[QStringLiteral("HA-prosper")] = false;

        // split search results and look, which class files are present
        QStringList list = m_output.split(QLatin1Char('\n'));
        for(QStringList::Iterator it=list.begin(); it!=list.end(); ++it) {
            QString filename = QFileInfo(*it).fileName();
            if (filename == QStringLiteral("scrartcl.cls")) {
                map[QStringLiteral("Scrartcl")] = true;
                map[QStringLiteral("Scrbook")]  = true;
                map[QStringLiteral("Scrreprt")] = true;
                map[QStringLiteral("Scrlttr2")] = true;
            }
            else if (filename == QStringLiteral("beamer.cls")) {
                map[QStringLiteral("Beamer")] = true;
            }
            else if (filename == QStringLiteral("prosper.cls")) {
                map[QStringLiteral("Prosper")] = true;
            }
            else if (filename == QStringLiteral("HA-prosper.sty")) {
                map[QStringLiteral("HA-prosper")] = true;
            }
        }


        KileTemplate::TemplateList templateList = m_templateManager->getTemplates(KileDocument::LaTeX);
        // insert all standard templates, all user-defined templates
        // and those templates, which have a present class
        for (KileTemplate::TemplateListIterator i=templateList.begin(); i != templateList.end(); ++i) {
            KileTemplate::Info info = *i;
            QString classname = info.name;
            if(!map.contains(classname) || map[classname]==true) {
                new TemplateItem(this, info);
            }
        }
    }
    else {
        KileTemplate::TemplateList templateList = m_templateManager->getTemplates(type);
        for (KileTemplate::TemplateListIterator i=templateList.begin(); i != templateList.end(); ++i) {
            new TemplateItem(this, *i);
        }
    }

    // sort all items (item for 'Empty Document' will always be the first one)
    sortItems();
}

