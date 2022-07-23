/***************************************************************************
  Copyright (C) 2003 by Jeroen Wijnhout (jeroen.wijnhout@kdemail.net)
                2010-2022 by Michel Ludwig (michel.ludwig@kdemail.net)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kiletool.h"

#include <QDir>
#include <QFileInfo>
#include <QMetaObject>
#include <QRegExp>
#include <QTimer>

#include <KConfig>
#include <KLocalizedString>
#include <KShell>
#include <QUrl>

#include "kileconfig.h"
#include "kiletool_enums.h"
#include "kilestdtools.h" //for the factory
#include "kiletoolmanager.h"
#include "kiledocmanager.h"
#include "kileinfo.h"
#include "documentinfo.h"
#include "kileproject.h"

namespace KileTool
{
Base::Base(const QString &name, Manager *manager, bool prepare /* = true */) :
    QObject(manager), // ensure that they are deleted whenever the tool manager gets deleted
    m_launcher(Q_NULLPTR),
    m_quickie(false),
    m_isPartOfLivePreview(false),
    m_manager(manager),
    m_name(name),
    m_bPrepareToRun(prepare),
    m_texInputs(KileConfig::teXPaths()),
    m_bibInputs(KileConfig::bibInputPaths()),
    m_bstInputs(KileConfig::bstInputPaths()),
    m_childToolSpawned(false),
    m_toolResult(-1)
{
    // WARNING: 'NeedSaveAll' is currently needed to indicate to the tool manager that parsing has to be finished
    //          before it launches this tool! This is important to ensure that the right master document is used
    //          in the case of projects, for example.
    m_flags = NeedTargetDirExec | NeedTargetDirWrite | NeedActiveDoc | NeedMasterDoc | NoUntitledDoc | NeedSourceExists | NeedSourceRead | NeedSaveAll;

    setMsg(NeedTargetDirExec, ki18n("Could not change to the folder %1."));
    setMsg(NeedTargetDirWrite, ki18n("The folder %1 is not writable, therefore %2 will not be able to save its results."));
    setMsg(NeedTargetExists,  ki18n("The file %1/%2 does not exist. If this is unexpected, check the file permissions."));
    setMsg(NeedTargetRead, ki18n("The file %1/%2 is not readable. If this is unexpected, check the file permissions."));
    setMsg(NeedActiveDoc, ki18n("Could not determine on which file to run %1, because there is no active document."));
    setMsg(NeedMasterDoc, ki18n("Could not determine the master file for this document."));
    setMsg(NoUntitledDoc, ki18n("Please save the untitled document first."));
    setMsg(NeedSourceExists, ki18n("The file %1 does not exist."));
    setMsg(NeedSourceRead, ki18n("The file %1 is not readable."));

    m_bPrepared = false;
}

Base::~Base()
{
    KILE_DEBUG_MAIN << "DELETING TOOL: " << name() << this;
    emit(aboutToBeDestroyed(this));
    delete m_launcher;
}

QString Base::source(bool absolute /* = true */) const
{
    if (m_source.isEmpty()) {
        return QString();
    }

    QString src = m_source;
    if (absolute) {
        src = m_basedir + '/' + src;
    }

    return src;
}

void Base::setMsg(long n, const KLocalizedString& msg)
{
    m_messages[n] = msg;
}

void Base::translate(QString &str, bool quoteForShell)
{
    QHashIterator<QString,QString> it(paramDict());
    while(it.hasNext()) {
        it.next();
        QString value;
        // the file names in %AFL are quoted already
        if(quoteForShell && it.key() != "%AFL") {
            value = KShell::quoteArg(it.value());
        }
        else {
            value = it.value();
        }
        str.replace(it.key(), value);
    }
}

void Base::removeFlag(uint flag)
{
    m_flags &= ~flag;
}


bool Base::requestSaveAll()
{
    return (flags() & NeedSaveAll);
}

void Base::setEntry(const QString& key, const QString& value)
{
    m_entryMap[key] = value;
}

void Base::prepareToRun()
{
    KILE_DEBUG_MAIN << "==Base::prepareToRun()=======";

    //install a launcher
    if (!installLauncher())
    {
        m_nPreparationResult = NoLauncherInstalled;
        m_bPrepared = false;
        return;
    }

    if (!determineSource())
    {
        m_nPreparationResult = NoValidSource;
        m_bPrepared = false;
        return;
    }

    if (!determineTarget())
    {
        m_nPreparationResult = NoValidTarget;
        m_bPrepared = false;
        return;
    }

    if ( m_launcher == 0 )
    {
        m_nPreparationResult = NoLauncherInstalled;
        m_bPrepared = false;
        return;
    }

    if(!workingDir().isEmpty()) {
        m_launcher->setWorkingDirectory(workingDir());
    }
    else {
        m_launcher->setWorkingDirectory(baseDir());
    }

    //fill in the dictionary
    addDict("%options", m_options);

    m_resolution = KileConfig::dvipngResolution() ;
    addDict("%res",m_resolution);

    m_bPrepared = true;
    m_nPreparationResult = Running;
}

int Base::run()
{
    KILE_DEBUG_MAIN << "==KileTool::Base::run()=================";

    if(m_nPreparationResult != 0) {
        emit(failedToRun(this, m_nPreparationResult));
        return m_nPreparationResult;
    }

    if(!checkSource()) {
        emit(failedToRun(this, NoValidSource));
        return NoValidSource;
    }

    if(!checkTarget()) {
        emit(failedToRun(this, TargetHasWrongPermissions));
        return TargetHasWrongPermissions;
    }

    if (!checkPrereqs()) {
        emit(failedToRun(this, NoValidPrereqs));
        return NoValidPrereqs;
    }

    emit(start(this));

    if (!m_launcher || !m_launcher->launch()) {
        KILE_DEBUG_MAIN << "\tlaunching failed";
        if(!m_launcher) {
            emit(failedToRun(this, CouldNotLaunch));
            return CouldNotLaunch;
        }
        if(!m_launcher->selfCheck()) {
            emit(failedToRun(this, SelfCheckFailed));
            return SelfCheckFailed;
        }
        else {
            emit(failedToRun(this, CouldNotLaunch));
            return CouldNotLaunch;
        }
    }

    KILE_DEBUG_MAIN << "\trunning...";

    return Running;

}

bool Base::determineSource()
{
    QString src = source();

    // check whether the source has been set already
    if(!src.isEmpty()) {
        return true;
    }

    //the basedir is determined from the current compile target
    //determined by getCompileName()
    if(src.isEmpty()) {
        src = m_ki->getCompileName();
    }
    setSource(src);

    return true;
}

bool Base::checkSource()
{
    //FIXME deal with tools that do not need a source or target (yes they exist)
    //Is there an active document? Only check if the source file is not explicitly set.
    if((m_source.isEmpty()) && (m_manager->info()->activeTextDocument() == Q_NULLPTR)) {
        sendMessage(Error, msg(NeedActiveDoc).subs(name()).toString());
        return false;
    }

    if(m_source.isEmpty() && m_manager->info()->activeTextDocument() != Q_NULLPTR) {
        if(m_manager->info()->activeTextDocument()->url().isEmpty()
                && (flags() & NoUntitledDoc)) {
            sendMessage(Error, msg(NoUntitledDoc).toString());
            return false;
        }
        else {
            //couldn't find a source file, huh?
            //we know there is an active document, the only reason is could have failed is because
            //we couldn't find a LaTeX root document
            sendMessage(Error, msg(NeedMasterDoc).toString());
            return false;
        }
    }

    QFileInfo fi(source());
    if((flags() & NeedSourceExists) && !fi.exists()) {
        sendMessage(Error, msg(NeedSourceExists).subs(fi.absoluteFilePath()).toString());
        return false;
    }

    if((flags() & NeedSourceRead) && !fi.isReadable()) {
        sendMessage(Error, msg(NeedSourceRead).subs(fi.absoluteFilePath()).toString());
        return false;
    }

    return true;
}

void Base::runChildNext(Base *tool, bool block /*= false*/)
{
    m_childToolSpawned = true;
    if(isPartOfLivePreview()) {
        tool->setPartOfLivePreview();
    }
    manager()->runChildNext(this, tool, block);
}

void Base::setSource(const QString &source, const QString& workingDir)
{
    QFileInfo info(source);

    if(!from().isEmpty()) {
        QString src = source;
        if(info.suffix().length() > 0) {
            src.replace(QRegExp(info.suffix() + '$'), from());
        }
        info.setFile(src);
    }

    if(!workingDir.isEmpty()) {
        setWorkingDir(workingDir);
    }

    m_basedir = info.absolutePath();
    m_source = info.fileName();
    m_S = info.completeBaseName();

    addDict("%dir_base", m_basedir);
    addDict("%source", m_source);
    addDict("%S",m_S);

    KILE_DEBUG_MAIN << "===KileTool::Base::setSource()==============";
    KILE_DEBUG_MAIN << "using " << source;
    KILE_DEBUG_MAIN << "source="<<m_source;
    KILE_DEBUG_MAIN << "S=" << m_S;
    KILE_DEBUG_MAIN << "basedir=" << m_basedir;
    KILE_DEBUG_MAIN << "workingDir=" << m_workingDir;
}

void Base::setTeXInputPaths(const QString& s)
{
    m_texInputs = s;
}

QString Base::teXInputPaths() const
{
    return m_texInputs;
}

void Base::setBibInputPaths(const QString& s)
{
    m_bibInputs = s;
}

QString Base::bibInputPaths() const
{
    return m_bibInputs;
}

void Base::setBstInputPaths(const QString& s)
{
    m_bstInputs = s;
}

QString Base::bstInputPaths() const
{
    return m_bstInputs;
}

void Base::copyPaths(Base* tool)
{
    setTeXInputPaths(tool->teXInputPaths());
    setBibInputPaths(tool->bibInputPaths());
    setBstInputPaths(tool->bstInputPaths());
}

bool Base::determineTarget()
{
    QFileInfo info(source());

    //if the target is not set previously, use the source filename
    if(m_target.isEmpty()) {
        //test for explicit override
        if (!readEntry("target").isEmpty()) {
            KILE_DEBUG_MAIN << "USING target SETTING";
            m_target = readEntry("target");
        }
        else if ( to().length() > 0) {
            m_target = S() + '.' + to();
        }
        else {
            m_target = source(false);
        }
    }

    if(m_relativedir.isEmpty() && (!readEntry("relDir").isEmpty())) {
        m_relativedir = readEntry("relDir");
    }

    QUrl url;
    if(!m_targetdir.isEmpty()) {
        url = QUrl::fromLocalFile(m_targetdir);
    }
    else if(!m_workingDir.isEmpty()) {
        url = QUrl::fromLocalFile(m_workingDir);
    }
    else {
        url = QUrl::fromLocalFile(m_basedir);
    }
    url = url.adjusted(QUrl::StripTrailingSlash);
    url.setPath(QDir::cleanPath(url.path() + '/' + m_relativedir));
    m_targetdir = url.toLocalFile();

    setTarget(m_target);
    setTargetDir(m_targetdir);

    KILE_DEBUG_MAIN << "==KileTool::Base::determineTarget()=========";
    KILE_DEBUG_MAIN << "\tm_targetdir=" << m_targetdir;
    KILE_DEBUG_MAIN << "\tm_target=" << m_target;

    return true;
}

bool Base::checkTarget()
{
    //check if the target directory is accessible
    QFileInfo info(m_targetdir);

    if((flags() & NeedTargetDirExec ) && (!info.isExecutable())) {
        sendMessage(Error, msg(NeedTargetDirExec).subs(m_targetdir).toString());
        return false;
    }

    if((flags() & NeedTargetDirWrite) && (!info.isWritable())) {
        sendMessage(Error, msg(NeedTargetDirWrite).subs(m_targetdir).subs(m_name).toString());
        return false;
    }

    info.setFile(m_targetdir + '/' + m_target);

    if((flags() & NeedTargetExists) && (!info.exists())) {
        sendMessage(Error, msg(NeedTargetExists).subs(m_targetdir).subs(m_target).toString());
        return false;
    }

    if((flags() & NeedTargetRead) && (!info.isReadable())) {
        sendMessage(Error, msg(NeedTargetRead).subs(m_targetdir).subs(m_target).toString());
        return false;
    }

    return true;
}

void Base::setTarget(const QString &target)
{
    m_target = target;
    addDict("%target", m_target);
}

void Base::setTargetDir(const QString &target)
{
    m_targetdir = target;
    addDict("%dir_target", m_targetdir);
}

void Base::setTargetPath(const QString &target)
{
    QFileInfo fi(target);
    setTarget(fi.fileName());
    setTargetDir(fi.absolutePath());
}

bool Base::checkPrereqs()
{
    return true;
}

void Base::stop()
{
    if (m_launcher) {
        m_launcher->kill();
    }

    emit(done(this, Aborted, m_childToolSpawned));
}

bool Base::finish(int result)
{
    KILE_DEBUG_MAIN << "==KileTool::Base::finish()==============";
    if (sender())
    {
        KILE_DEBUG_MAIN << "\tcalled by " << sender()->objectName() << " " << sender()->metaObject()->className();
    }

    if ( result == Aborted )
        sendMessage(Error, "Aborted");

    if ( result == Success )
        sendMessage(Info,"Done!");

    KILE_DEBUG_MAIN << "\temitting done(KileTool::Base*, int) " << name();
    emit(done(this, result, m_childToolSpawned));

    //we will only get here if the done() signal is not connected to the manager (who will destroy this object)
    if (result == Success) {
        return true;
    }
    else {
        return false;
    }
}

void Base::installLaTeXOutputParserResult(int nErrors, int nWarnings, int nBadBoxes, const LatexOutputInfoArray& outputList,
        const QString& logFile)
{
    m_nErrors = nErrors;
    m_nWarnings = nWarnings;
    m_nBadBoxes = nBadBoxes;
    m_latexOutputInfoList = outputList;
    m_logFile = logFile;

    latexOutputParserResultInstalled();
}

void Base::latexOutputParserResultInstalled()
{
    finish(Success);
}

void Base::installLauncher(Launcher *lr)
{
    if(m_launcher != lr)
        delete m_launcher;

    m_launcher = lr;
    //lr->setParamDict(paramDict());
    lr->setTool(this);

    connect(lr, SIGNAL(message(int,QString)), this, SLOT(sendMessage(int,QString)));
    connect(lr, SIGNAL(output(QString)), this, SLOT(filterOutput(QString)));
    connect(lr, SIGNAL(done(int)), this, SLOT(finish(int)));
}

bool Base::installLauncher()
{
    if (m_launcher) {
        return true;
    }

    QString type = readEntry("type");
    KILE_DEBUG_MAIN << "installing launcher of type " << type;
    Launcher *lr = Q_NULLPTR;

    if ( type == "Process" ) {
        lr = new ProcessLauncher();
    }
    else if ( type == "Konsole" ) {
        lr = new KonsoleLauncher();
    }
    else if ( type == "DocumentViewer" ) {
        lr = new DocumentViewerLauncher();
    }

    if (lr) {
        installLauncher(lr);
        return true;
    }
    else {
        m_launcher = Q_NULLPTR;
        return false;
    }
}

void Base::setupAsChildTool(KileTool::Base *child)
{
    Q_UNUSED(child);
}

void Base::sendMessage(int type, const QString &msg)
{
    emit(message(type, msg, name()));
}

void Base::filterOutput(const QString & str)
{
    //here you have the change to filter the output and do some error extraction for example
    //this should be done by a OutputFilter class

    //idea: store the buffer until a complete line (or more) has been received then parse these lines
    //just send the buf immediately to the output widget, the results of the parsing are displayed in
    //the log widget anyway.
    emit(output(str));
}

bool Base::addDict(const QString & key, const QString & value)
{
    bool e = !(paramDict().contains(key));
    paramDict()[key] = value;
    return e;
}

bool Base::needsUpdate(const QString &target, const QString &source)
{
    KILE_DEBUG_MAIN << "==Base::needsUpdate(" << target << "," << source;
    QFileInfo targetinfo(target);
    QFileInfo sourceinfo(source);
    QDateTime currDateTime = QDateTime::currentDateTime();

    if(!(sourceinfo.exists() && sourceinfo.isReadable())) {
        KILE_DEBUG_MAIN << "\treturning false: source does not exist";
        return false;
    }

    if(!targetinfo.exists()) {
        KILE_DEBUG_MAIN << "\treturning true: target does not exist";
        return true;
    }

    KILE_DEBUG_MAIN << "\ttarget: " << targetinfo.lastModified().toString();
    KILE_DEBUG_MAIN << "\tsource: " << sourceinfo.lastModified().toString();

    if(targetinfo.lastModified() > currDateTime) {
        KILE_DEBUG_MAIN << "targetinfo.lastModifiedTime() is in the future";
        return false;
    }
    else if(sourceinfo.lastModified() > currDateTime) {
        KILE_DEBUG_MAIN << "sourceinfo.lastModifiedTime() is in the future";
        return false;
    }

    KILE_DEBUG_MAIN << "\treturning " << (targetinfo.lastModified() < sourceinfo.lastModified());
    return targetinfo.lastModified() < sourceinfo.lastModified();
}

Compile::Compile(const QString &name, Manager * manager, bool prepare /*= true*/)
    : Base(name, manager, prepare)
{
    setFlags( flags() | NeedTargetDirExec | NeedTargetDirWrite);
}

Compile::~Compile()
{}

bool Compile::checkSource()
{
    if ( !Base::checkSource() ) return false;

    bool isRoot = true;
    KileDocument::TextInfo *docinfo = manager()->info()->docManager()->textInfoFor(source());
    if (docinfo) {
        isRoot = (readEntry("checkForRoot") == "yes") ? docinfo->isLaTeXRoot() : true;
    }

    if (!isRoot)
    {
        return  manager()->queryContinue(i18n("The document %1 is not a LaTeX root document; continue anyway?", source()), i18n("Continue?"));
    }

    return true;
}

View::View(const QString &name, Manager * manager, bool prepare /*= true*/)
    : Base(name, manager, prepare)
{
    setFlags(NeedTargetDirExec | NeedTargetExists | NeedTargetRead);

    KILE_DEBUG_MAIN << "View: flag " << (flags() & NeedTargetExists);
    setMsg(NeedTargetExists, ki18n("The file %1/%2 does not exist; did you compile the source file?"));
}

View::~View()
{
}


Archive::Archive(const QString &name, Manager * manager, bool prepare /* = true*/)
    : Base(name, manager,prepare)
{
    setFlags( NeedTargetDirExec | NeedTargetDirWrite );
}

Archive::~Archive()
{}

bool Archive::checkPrereqs()
{
    if(!m_project) {
        sendMessage(Error,i18n("The current document is not associated to a project. Please activate a document that is associated to the project you want to archive, then choose Archive again."));
        return false;
    }
    else if(m_fileList.isEmpty()) {
        sendMessage(Error, i18n("No files have been chosen for archiving."));
        return false;
    }
    else {
        return true;
    }
}

void Archive::setSource(const QString &source, const QString& workingDir)
{
    Q_UNUSED(workingDir);
    QUrl url = QUrl::fromLocalFile(source);
    m_project = manager()->info()->docManager()->projectFor(url);
    if ( !m_project )
        m_project = manager()->info()->docManager()->activeProject();
    if ( !m_project )
        m_project = manager()->info()->docManager()->selectProject(i18n("Archive Project"));
    if ( !m_project )
    {
        Base::setSource(source);
        return;
    }

    manager()->info()->docManager()->projectSave(m_project);
    Base::setSource(m_project->url().toLocalFile());
    m_fileList = m_project->archiveFileList();

    addDict("%AFL", m_fileList);

    KILE_DEBUG_MAIN << "===KileTool::Archive::setSource("<< source << ")==============";
    KILE_DEBUG_MAIN << "m_fileList="<<m_fileList<< Qt::endl;
}

Convert::Convert(const QString &name, Manager * manager, bool prepare /*= true*/)
    : Base(name, manager,prepare)
{
    setFlags( flags() | NeedTargetDirExec | NeedTargetDirWrite );
}

Convert::~Convert()
{
}

bool Convert::determineSource()
{
    bool  br = Base::determineSource();
    setSource(baseDir() + '/' + S() + '.' + from());
    return br;
}

Sequence::Sequence(const QString &name, Manager *manager, bool prepare /*= true*/)
    : Base(name, manager, prepare), m_latexOutputHandler(Q_NULLPTR)
{
}

Sequence::~Sequence() {
    qDeleteAll(m_tools);
}

LaTeXOutputHandler* Sequence::latexOutputHandler()
{
    return m_latexOutputHandler;
}

void Sequence::setLaTeXOutputHandler(LaTeXOutputHandler *h)
{
    m_latexOutputHandler = h;
}

bool Sequence::determineSource()
{
    QString src = source();

    // check whether the source has been set already
    if(!src.isEmpty()) {
        return true;
    }

    // the basedir is determined from the current compile target,
    // determined by getCompileName()
    LaTeXOutputHandler *h = Q_NULLPTR;
    src = m_ki->getCompileName(false, &h);

    setSource(src);
    setLaTeXOutputHandler(h);

    return true;
}

bool Sequence::requestSaveAll()
{
    // if one of the tools in the sequence requests save-all, then we also
    // request it
    for(Base *tool : m_tools) {
        if(tool->requestSaveAll()) {
            return true;
        }
    }

    return false;
}

void Sequence::setupSequenceTools()
{
    QStringList toolNameList = readEntry("sequence").split(',');
    QString tl, cfg;
    Base *tool;
    for(QStringList::iterator i = toolNameList.begin(); i != toolNameList.end(); ++i) {
        QString fullToolSpec = (*i).trimmed();
        extract(fullToolSpec, tl, cfg);

        tool = manager()->createTool(tl, cfg, false); // create tool with delayed preparation
        if (tool) {
            KILE_DEBUG_MAIN << "===tool created with name " << tool->name();
            if(!(manager()->info()->watchFile() && tool->isViewer())) { // FIXME: why this?
                KILE_DEBUG_MAIN << "\tqueueing " << tl << "(" << cfg << ") with " << source();
                m_tools.push_back(tool);
            }
            else {
                delete tool;
            }
        }
        else {
            m_unknownToolSpec = fullToolSpec;
            qDeleteAll(m_tools);
            m_tools.clear();
            return;
        }
    }
}

int Sequence::run()
{
    KILE_DEBUG_MAIN << "==KileTool::Sequence::run()==================";

    determineSource();
    if (!checkSource()) {
        // tools in 'm_tools' will be deleted in the destructor
        return NoValidSource;
    }

    if(!m_unknownToolSpec.isEmpty()) {
        // 'm_tools' is empty
        sendMessage(Error, i18n("Unknown tool %1.", m_unknownToolSpec));
        emit(done(this, Failed, m_childToolSpawned));
        return ConfigureFailed;
    }

    for(Base *tool : m_tools) {
        tool->setSource(source());

        // if we are running a 'LaTeX' tool here, we still have to set the
        // appropriate LaTeXOutputHandler
        {
            LaTeX *latex = dynamic_cast<LaTeX*>(tool);
            if(latex && m_latexOutputHandler) {
                latex->setLaTeXOutputHandler(m_latexOutputHandler);
            }
        }

        manager()->run(tool);
    }

    m_tools.clear(); // the tools will be deleted by the tool manager from now on
    emit(done(this, Silent, m_childToolSpawned));

    return Success;
}
}

