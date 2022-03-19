/*************************************************************************************
    begin                : Thu Jul 17 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                               2007-2018 by Michel Ludwig (michel.ludwig@kdemail.net)
 *************************************************************************************/

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

#include "kileinfo.h"

#include <qwidget.h>
#include <QFileInfo>
#include <QObject>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <KLocalizedString>
#include <KMessageBox>

#include "parser/parsermanager.h"
#include "widgets/structurewidget.h"
#include "configurationmanager.h"
#include "editorcommands.h"
#include "kiledocmanager.h"
#include "kileviewmanager.h"
#include "documentinfo.h"
#include "kileproject.h"
#include "scriptmanager.h"
#include "abbreviationmanager.h"
#include "editorkeysequencemanager.h"
#include "templates.h"
#include "utilities.h"
#include "usermenu/usermenu.h"

#include <QStringList>
#include <QString>

/*
 * Class KileInfo.
 */

KileInfo::KileInfo(KParts::MainWindow *parent)
    : m_mainWindow(parent),
      m_configurationManager(Q_NULLPTR),
      m_docManager(Q_NULLPTR),
      m_viewManager(Q_NULLPTR),
      m_manager(Q_NULLPTR),
      m_templateManager(Q_NULLPTR),
      m_jScriptManager(Q_NULLPTR),
      m_editorKeySequenceManager(Q_NULLPTR),
      m_toolFactory(Q_NULLPTR),
      m_texKonsole(Q_NULLPTR),
      m_outputWidget(Q_NULLPTR),
      m_scriptsManagementWidget(Q_NULLPTR),
      m_bottomBar(Q_NULLPTR),
      m_previewWidget(Q_NULLPTR),
      m_previewScrollArea(Q_NULLPTR),
      m_codeCompletionManager(Q_NULLPTR),
      m_abbreviationManager(Q_NULLPTR),
      m_parserManager(Q_NULLPTR),
      m_errorHandler(Q_NULLPTR),
      m_editorCommands(Q_NULLPTR),
      m_help(Q_NULLPTR),
      m_edit(Q_NULLPTR),
      m_latexCommands(Q_NULLPTR),
      m_extensions(Q_NULLPTR),
      m_quickPreview(Q_NULLPTR),
      m_userMenu(Q_NULLPTR),
      m_livePreviewManager(Q_NULLPTR),
      m_kwStructure(Q_NULLPTR),
      m_fileBrowserWidget(Q_NULLPTR)
{
    m_configurationManager = new KileConfiguration::Manager(this, parent, "KileConfiguration::Manager");
    m_docManager = new KileDocument::Manager(this, parent, "KileDocument::Manager");
    m_templateManager = new KileTemplate::Manager(this, parent, "KileTemplate::Manager");
    m_editorKeySequenceManager = new KileEditorKeySequence::Manager(this, parent, "KileEditorKeySequence::Manager");
    m_abbreviationManager = new KileAbbreviation::Manager(this, parent);
    m_parserManager = new KileParser::Manager(this, parent);
    m_editorCommands = new EditorCommands(this);
}

KileInfo::~KileInfo()
{
    // this has to be deleted before the editor component is destroyed
    delete m_editorCommands;
}

KTextEditor::Document * KileInfo::activeTextDocument() const
{
    KTextEditor::View *view = viewManager()->currentTextView();
    if (view) return view->document();
    else return Q_NULLPTR;
}

QString KileInfo::getName(KTextEditor::Document *doc, bool shrt) const
{
    KILE_DEBUG_MAIN << "===KileInfo::getName(KTextEditor::Document *doc, bool " << shrt << ")===" << Qt::endl;
    QString title;

    if (!doc) {
        doc = activeTextDocument();
    }
    if (doc) {
        QUrl url = doc->url();
        KILE_DEBUG_MAIN << "url " << url << Qt::endl;
        if(url.isLocalFile()) {
            title = shrt ? doc->url().fileName() : doc->url().toLocalFile();
        }
        else {
            title = url.toDisplayString();
        }
    }

    return title;
}

LaTeXOutputHandler* KileInfo::findCurrentLaTeXOutputHandler() const
{
    LaTeXOutputHandler *h = Q_NULLPTR;

    getCompileName(false, &h);
    return h;
}

QString KileInfo::getCompileName(bool shrt /* = false */, LaTeXOutputHandler** h /* = Q_NULLPTR */) const
{
    KileProject *project = docManager()->activeProject();

    if (m_singlemode) {
        if (project) {
            if(h) {
                *h = project;
            }
            return getCompileNameForProject(project, shrt);
        }
        else {
            KTextEditor::Document *doc = activeTextDocument();
            if(h) {
                *h = dynamic_cast<KileDocument::LaTeXInfo*>(m_docManager->textInfoFor(doc));
            }
            return getName(doc, shrt);
        }
    }
    else {
        QFileInfo fi(m_masterDocumentFileName);
        if(h) {
            *h = dynamic_cast<KileDocument::LaTeXInfo*>(m_docManager->textInfoFor(m_masterDocumentFileName));
        }
        if(shrt) {
            return fi.fileName();
        }
        else {
            return m_masterDocumentFileName;
        }
    }
}

QString KileInfo::getCompileNameForProject(KileProject *project, bool shrt) const
{
    if (!project->masterDocument().isEmpty()) {
        QUrl master = QUrl::fromLocalFile(project->masterDocument());
        if(shrt) {
            return master.fileName();
        }
        else {
            return master.toLocalFile();
        }
    }
    else {
        KileProjectItem *item = project->rootItem(docManager()->activeProjectItem());
        if (item) {
            QUrl url = item->url();
            if(shrt) {
                return url.fileName();
            }
            else {
                return url.toLocalFile();
            }
        }
        else {
            return QString();
        }
    }
}

QString KileInfo::getFullFromPrettyName(const OutputInfo& info, const QString& name) const
{
    if(name.isEmpty()) {
        return name;
    }

    QString file = name;

    if(file.left(2) == "./") {
        file = QFileInfo(info.mainSourceFile()).absolutePath() + '/' + file.mid(2);
    }

    if(QDir::isRelativePath(file)) {
        file = QFileInfo(info.mainSourceFile()).absolutePath() + '/' + file;
    }

    QFileInfo fi(file);
    if(file.isEmpty() || fi.isDir() || (! fi.exists()) || (! fi.isReadable())) {
        // - call from logwidget or error handling, which
        //   tries to determine the LaTeX source file
        bool found = false;
        QStringList extlist = (m_extensions->latexDocuments()).split(' ');
        for(QStringList::Iterator it=extlist.begin(); it!=extlist.end(); ++it) {
            QString name = file + (*it);
            if(QFileInfo(name).exists()) {
                file = name;
                fi.setFile(name);
                found = true;
                break;
            }
        }
        if(!found) {
            file.clear();
        }
    }

    if(!fi.isReadable()) {
        return QString();
    }

    return file;
}

QList<QUrl> KileInfo::getParentsFor(KileDocument::Info *info)
{
    QList<KileProjectItem*> items = docManager()->itemsFor(info);
    QList<QUrl> list;
    for(QList<KileProjectItem*>::iterator it = items.begin(); it != items.end(); ++it) {
        if((*it)->parent()) {
            list.append((*it)->parent()->url());
        }
    }
    return list;
}

QStringList KileInfo::retrieveList(QStringList (KileDocument::Info::*getit)() const, KileDocument::TextInfo *docinfo)
{
    if(!docinfo) {
        docinfo = docManager()->getInfo();
    }
    KileProjectItem *item = docManager()->itemFor(docinfo, docManager()->activeProject());

    KILE_DEBUG_MAIN << "Kile::retrieveList()";
    if (item) {
        KileProject *project = item->project();
        KileProjectItem *root = project->rootItem(item);
        if (root) {
            KILE_DEBUG_MAIN << "\tusing root item " << root->url().fileName();

            QList<KileProjectItem*> children;
            children.append(root);
            root->allChildren(&children);

            QStringList toReturn;
            for(QList<KileProjectItem*>::iterator it = children.begin(); it != children.end(); ++it) {
                const KileProjectItem *item = *it;
                KileDocument::TextInfo *textInfo = item->getInfo();
                KILE_DEBUG_MAIN << "\t" << item->url();

                if(textInfo) {
                    toReturn << (textInfo->*getit)();
                }
            }
            return toReturn;
        }
        else {
            return QStringList();
        }
    }
    else if (docinfo) {
        return (docinfo->*getit)();
    }
    else {
        return QStringList();
    }
}

QStringList KileInfo::allLabels(KileDocument::TextInfo *info)
{
    KILE_DEBUG_MAIN << "Kile::allLabels()" << Qt::endl;
    return retrieveList(&KileDocument::Info::labels, info);
}

QStringList KileInfo::allBibItems(KileDocument::TextInfo *info)
{
    KILE_DEBUG_MAIN << "Kile::allBibItems()" << Qt::endl;
    return retrieveList(&KileDocument::Info::bibItems, info);
}

QStringList KileInfo::allBibliographies(KileDocument::TextInfo *info)
{
    KILE_DEBUG_MAIN << "Kile::bibliographies()" << Qt::endl;
    return retrieveList(&KileDocument::Info::bibliographies, info);
}

QStringList KileInfo::allDependencies(KileDocument::TextInfo *info)
{
    KILE_DEBUG_MAIN << "Kile::dependencies()" << Qt::endl;
    return retrieveList(&KileDocument::Info::dependencies, info);
}

QStringList KileInfo::allNewCommands(KileDocument::TextInfo *info)
{
    KILE_DEBUG_MAIN << "Kile::newCommands()" << Qt::endl;
    return retrieveList(&KileDocument::Info::newCommands, info);
}

QStringList KileInfo::allAsyFigures(KileDocument::TextInfo *info)
{
    KILE_DEBUG_MAIN << "Kile::asyFigures()" << Qt::endl;
    return retrieveList(&KileDocument::Info::asyFigures, info);
}

QStringList KileInfo::allPackages(KileDocument::TextInfo *info)
{
    KILE_DEBUG_MAIN << "Kile::allPackages()" << Qt::endl;
    return retrieveList(&KileDocument::Info::packages, info);
}

QString KileInfo::lastModifiedFile(KileDocument::TextInfo* info)
{
    if(!info) {
        info = docManager()->getInfo();
    }
    QStringList list = allDependencies(info);
    QFileInfo fileinfo(info->url().toLocalFile());
    list.append(fileinfo.fileName());

    return KileUtilities::lastModifiedFile(list, fileinfo.absolutePath());
}

QString KileInfo::documentTypeToString(KileDocument::Type type)
{
    switch(type) {
    case KileDocument::Undefined:
        return i18n("Undefined");
    case KileDocument::Text:
        return i18n("Text");
    case KileDocument::LaTeX:
        return i18n("LaTeX");
    case KileDocument::BibTeX:
        return i18n("BibTeX");
    case KileDocument::Script:
        return i18n("Script");
    }
    return QString();
}

bool KileInfo::similarOrEqualURL(const QUrl &validurl, const QUrl &testurl)
{
    if ( testurl.isEmpty() || testurl.path().isEmpty() ) return false;


    bool absolute = QDir::isAbsolutePath(testurl.toLocalFile());
    return (
               (validurl == testurl) ||
               (!absolute && validurl.path().endsWith(testurl.path()))
           );
}

bool KileInfo::isOpen(const QUrl &url)
{
    KILE_DEBUG_MAIN << "==bool KileInfo::isOpen(const QUrl &url)=============" << Qt::endl;

    for (int i = 0; i < viewManager()->textViewCount(); ++i) {
        KTextEditor::View *view = viewManager()->textView(i);
        if (view->document() && similarOrEqualURL(view->document()->url(), url)) {
            return true;
        }
    }

    return false;
}

bool KileInfo::projectIsOpen(const QUrl &url)
{
    KileProject *project = docManager()->projectFor(url);

    return project != 0 ;
}


QString KileInfo::getSelection() const
{
    KTextEditor::View *view = viewManager()->currentTextView();

    if (view && view->selection()) {
        return view->selectionText();
    }

    return QString();
}

void KileInfo::clearSelection() const
{
    KTextEditor::View *view = viewManager()->currentTextView();

    if(view && view->selection()) {
        view->removeSelectionText();
    }
}

QString KileInfo::expandEnvironmentVars(const QString &str)
{
    static QRegExp reEnvVars("\\$(\\w+)");
    QString result = str;
    int index = -1;
    while ( (index = str.indexOf(reEnvVars, index + 1)) != -1 )
        result.replace(reEnvVars.cap(0),qgetenv(reEnvVars.cap(1).toLocal8Bit()));

    return result;
}

QString KileInfo::checkOtherPaths(const QString &path,const QString &file, int type)
{
    KILE_DEBUG_MAIN << "QString KileInfo::checkOtherPaths(const QString &path,const QString &file, int type)" << Qt::endl;
    QStringList inputpaths;
    QString configpaths;
    QFileInfo info;

    switch(type)
    {
    case bibinputs:
        configpaths = KileConfig::bibInputPaths() + LIST_SEPARATOR + "$BIBINPUTS";
        break;
    case texinputs:
        configpaths = KileConfig::teXPaths() + LIST_SEPARATOR + "$TEXINPUTS";
        break;
    case bstinputs:
        configpaths = KileConfig::bstInputPaths() + LIST_SEPARATOR + "$BSTINPUTS";
        break;
    default:
        KILE_DEBUG_MAIN << "Unknown type in checkOtherPaths" << Qt::endl;
        return QString();
        break;
    }

    inputpaths = expandEnvironmentVars(configpaths).split(LIST_SEPARATOR);
    inputpaths.prepend(path);

    // the first match is supposed to be the correct one
    foreach(const QString &string, inputpaths) {
        KILE_DEBUG_MAIN << "path is " << string << "and file is " << file << Qt::endl;
        info.setFile(string + '/' + file);
        if(info.exists()) {
            KILE_DEBUG_MAIN << "filepath after correction is: " << info.path() << Qt::endl;
            return info.absoluteFilePath();
        }
    }
    return QString();
}

