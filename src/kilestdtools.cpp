/**************************************************************************************
    begin                : Thu Nov 27 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                           (C) 2011-2017 by Michel Ludwig (michel.ludwig@kdemail.net)
 **************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kilestdtools.h"

#include <QFileInfo>
#include <QRegExp>

#include <QAction>
#include <KActionCollection>
#include <KConfig>
#include <KLocalizedString>

#include <KProcess>
#include <QStandardPaths>

#include "dialogs/listselector.h"
#include "kileconfig.h"
#include "kiletool.h"
#include "kiletoolmanager.h"
#include "kiletool_enums.h"
#include "kileinfo.h"
#include "kiledocmanager.h"
#include "documentinfo.h"
#include "outputinfo.h"
#include "parser/parsermanager.h"
#include "utilities.h"

#define SHORTCUTS_GROUP_NAME "Shortcuts"

namespace KileTool
{
Factory::Factory(Manager *mngr, KConfig *config, KActionCollection *actionCollection)
    : m_manager(mngr), m_config(config), m_actionCollection(actionCollection)
{
    m_standardToolConfigurationFileName = QStandardPaths::locate(QStandardPaths::DataLocation, "kilestdtools.rc");
}

Factory::~Factory()
{
}

Base* Factory::create(const QString& toolName, const QString& config, bool prepare /* = true */)
{
    KILE_DEBUG_MAIN << toolName << config << prepare;
    KileTool::Base *tool = Q_NULLPTR;
    //perhaps we can find the tool in the config file
    if (m_config->hasGroup(groupFor(toolName, m_config))) {
        KConfigGroup configGroup = m_config->group(groupFor(toolName, m_config));
        QString toolClass = configGroup.readEntry("class", QString());

        if(toolClass == "LaTeX") {
            tool = new LaTeX(toolName, m_manager, prepare);
        }
        else if(toolClass == "LaTeXpreview") {
            tool = new PreviewLaTeX(toolName, m_manager, prepare);
        }
        else if(toolClass == "LaTeXLivePreview") {
            tool = new LivePreviewLaTeX(toolName, m_manager, prepare);
        }
        else if(toolClass == "ForwardDVI") {
            tool = new ForwardDVI(toolName, m_manager, prepare);
        }
        else if(toolClass == "ViewHTML") {
            tool = new ViewHTML(toolName, m_manager, prepare);
        }
        else if(toolClass == "ViewBib") {
            tool = new ViewBib(toolName, m_manager, prepare);
        }
        else if(toolClass == "Base") {
            tool = new Base(toolName, m_manager, prepare);
        }
        else if(toolClass == "Compile") {
            tool = new Compile(toolName, m_manager, prepare);
        }
        else if (BibliographyCompile::ToolClass == toolClass) {
            tool = new BibliographyCompile(toolName, m_manager, prepare);
        }
        else if(toolClass == "Convert") {
            tool = new Convert(toolName, m_manager, prepare);
        }
        else if(toolClass == "Archive") {
            tool = new Archive(toolName, m_manager, prepare);
        }
        else if(toolClass == "View") {
            tool = new View(toolName, m_manager, prepare);
        }
        else if(toolClass == "Sequence") {
            tool = new Sequence(toolName, m_manager, prepare);
        }
    }
    if(!tool) {
        return Q_NULLPTR;
    }

    if(!m_manager->configure(tool, config)) {
        delete tool;
        return Q_NULLPTR;
    }
    tool->setToolConfig(config);

    // this has to be done after the configuration step only!
    if(dynamic_cast<KileTool::Sequence*>(tool)) {
        dynamic_cast<KileTool::Sequence*>(tool)->setupSequenceTools();
    }

    return tool;
}

void Factory::resetToolConfigurations()
{
    KConfig stdToolConfig(m_standardToolConfigurationFileName, KConfig::NoGlobals);

    m_config->deleteGroup(QLatin1String("Tools"));
    m_config->deleteGroup(QLatin1String("ToolsGUI"));

    // we delete all the groups whose names start with "Tool/";
    for(QString groupName : m_config->groupList()) {
        if(groupName.startsWith(QLatin1String("Tool/"))) {
            m_config->deleteGroup(groupName);
        }
    }

    // now we copy all the "Tool/" groups, the "Tools", and "ToolsGUI" groups over
    for(QString groupName : stdToolConfig.groupList()) {
        if(groupName != SHORTCUTS_GROUP_NAME) {
            KConfigGroup configGroup = stdToolConfig.group(groupName);
            m_config->deleteGroup(groupName);
            KConfigGroup newGroup = m_config->group(groupName);
            configGroup.copyTo(&newGroup, KConfigGroup::Persistent);
        }
    }
}

static void transferKeyStringPairsStartingWith(KConfigGroup& src, KConfigGroup& target, const QString& startsWith)
{
    const QStringList keyList = src.keyList();
    for(QString key : keyList) {
        if(key.startsWith(startsWith)) {
            QString value = src.readEntry(key, QString());
            target.writeEntry(key, value);
        }
    }
}

void Factory::installStandardLivePreviewTools()
{
    KConfig stdToolConfig(m_standardToolConfigurationFileName, KConfig::NoGlobals);

    const QStringList groupList = stdToolConfig.groupList();
    for(QString groupName : groupList) {
        if(groupName.startsWith(QStringLiteral("Tool/LivePreview"))) {
            KConfigGroup configGroup = stdToolConfig.group(groupName);
            m_config->deleteGroup(groupName);
            KConfigGroup newGroup = m_config->group(groupName);
            configGroup.copyTo(&newGroup, KConfigGroup::Persistent);
        }
    }

    {   // transfer the standard settings inside the "Tools" group
        const QString groupName(QStringLiteral("Tools"));
        KConfigGroup stdConfigGroup = stdToolConfig.group(groupName);
        KConfigGroup newGroup = m_config->group(groupName);
        transferKeyStringPairsStartingWith(stdConfigGroup, newGroup, QStringLiteral("LivePreview"));
    }

    {   // transfer the standard settings inside the "ToolsGUI" group
        const QString groupName(QStringLiteral("ToolsGUI"));
        KConfigGroup stdConfigGroup = stdToolConfig.group(groupName);
        KConfigGroup newGroup = m_config->group(groupName);
        transferKeyStringPairsStartingWith(stdConfigGroup, newGroup, QStringLiteral("LivePreview"));
    }
}

/////////////// LaTeX ////////////////

LaTeX::LaTeX(const QString& tool, Manager *mngr, bool prepare)
    : Compile(tool, mngr, prepare), m_latexOutputHandler(Q_NULLPTR)
{
}

LaTeX::~LaTeX()
{
}

void LaTeX::setupAsChildTool(KileTool::Base *child)
{
    KileTool::LaTeX *latexChild = dynamic_cast<KileTool::LaTeX*>(child);
    if(latexChild) {
        latexChild->setLaTeXOutputHandler(latexOutputHandler());
    }
}

LaTeXOutputHandler* LaTeX::latexOutputHandler()
{
    return m_latexOutputHandler;
}

void LaTeX::setLaTeXOutputHandler(LaTeXOutputHandler *h)
{
    m_latexOutputHandler = h;
}

bool LaTeX::determineSource()
{
    QString src = source();

    // check whether the source has been set already
    if(!src.isEmpty()) {
        return true;
    }

    //the basedir is determined from the current compile target
    //determined by getCompileName()
    LaTeXOutputHandler *h = Q_NULLPTR;
    src = m_ki->getCompileName(false, &h);

    setSource(src);
    setLaTeXOutputHandler(h);

    return true;
}

int LaTeX::m_reRun = 0;

// FIXME don't hardcode bbl and ind suffix here.
bool LaTeX::updateBibs(bool checkOnlyBibDependencies)
{
    KileDocument::TextInfo *docinfo = manager()->info()->docManager()->textInfoFor(source());
    if(docinfo) {
        QFileInfo fileinfo(docinfo->url().toLocalFile());
        QStringList dependencies;

        if (checkOnlyBibDependencies) {
            dependencies = manager()->info()->allBibliographies(docinfo);
        }
        else {
            dependencies = manager()->info()->allDependencies(docinfo);
            dependencies.append(fileinfo.fileName());
        }
        if (!dependencies.empty()) {
            return needsUpdate(targetDir() + '/' + S() + ".bbl",
                               KileUtilities::lastModifiedFile(dependencies, fileinfo.absolutePath()));
        }
    }

    return false;
}

bool LaTeX::updateIndex()
{
    KileDocument::TextInfo *docinfo = manager()->info()->docManager()->textInfoFor(source());
    if(docinfo) {
        QStringList pckgs = manager()->info()->allPackages(docinfo);
        if(pckgs.contains("makeidx") || pckgs.contains("imakeidx") || pckgs.contains("splitidx")) {
            return needsUpdate(targetDir() + '/' + S() + ".ind", manager()->info()->lastModifiedFile(docinfo));
        }
    }

    return false;
}

bool LaTeX::updateAsy()
{
    KileDocument::TextInfo *docinfo = manager()->info()->docManager()->textInfoFor(source());
    if(docinfo) {
        QStringList pckgs = manager()->info()->allPackages(docinfo);
        // As asymptote doesn't properly notify the user when it needs to be rerun, we run
        // it every time LaTeX is run (but only for m_reRun == 0 if LaTeX has to be rerun).
        if(pckgs.contains("asymptote")) {
            return true;
        }
    }
    return false;
}

bool LaTeX::finish(int r)
{
    KILE_DEBUG_MAIN << "==bool LaTeX::finish(" << r << ")=====";

    m_toolResult = r;

    if(m_toolResult == AbnormalExit || m_toolResult == Aborted) {
        return false;
    }

    // in case the compilation failed, we try to parse the log file in order to detect
    // errors reported by LaTeX
    QString log = targetDir() + '/' + S() + ".log";
    manager()->parserManager()->parseOutput(this, log, source());

    return true;
}

void LaTeX::latexOutputParserResultInstalled()
{
    KILE_DEBUG_MAIN;

    if(m_latexOutputHandler) {
        m_latexOutputHandler->storeLaTeXOutputParserResult(m_nErrors, m_nWarnings, m_nBadBoxes, m_latexOutputInfoList,
                m_logFile);
    }

    checqCriticals();

    if(readEntry("autoRun") == "yes") {
        checkAutoRun();
    }

    Compile::finish(m_toolResult);
}

void LaTeX::checqCriticals()
{
    // work around the 0 cases as the i18np call can cause some confusion when 0 is passed to it (#275700)
    QString es = (m_nErrors == 0 ? i18n("0 errors") : i18np("1 error", "%1 errors", m_nErrors));
    QString ws = (m_nWarnings == 0 ? i18n("0 warnings") : i18np("1 warning", "%1 warnings", m_nWarnings));
    QString bs = (m_nBadBoxes == 0 ? i18n("0 badboxes") : i18np("1 badbox", "%1 badboxes", m_nBadBoxes));

    sendMessage(Info, i18nc("String displayed in the log panel showing the number of errors/warnings/badboxes",
                            "%1, %2, %3", es, ws, bs));

    // jump to first error
    if(!isPartOfLivePreview() && m_nErrors > 0 && (readEntry("jumpToFirstError") == "yes")) {
        connect(this, SIGNAL(jumpToFirstError()), manager(), SIGNAL(jumpToFirstError()));
        emit(jumpToFirstError());
    }
}

void LaTeX::configureLaTeX(KileTool::Base *tool, const QString& source)
{
    tool->setSource(source, workingDir());
}

void LaTeX::configureBibTeX(KileTool::Base *tool, const QString& source)
{
    tool->setSource(source, workingDir());
}

void LaTeX::configureMakeIndex(KileTool::Base *tool, const QString& source)
{
    tool->setSource(source, workingDir());
}

void LaTeX::configureAsymptote(KileTool::Base *tool, const QString& source)
{
    tool->setSource(source, workingDir());
}

// if 'Biblatex' is not used in the document, 'hint' will be empty
ToolConfigPair LaTeX::determineBibliographyBackend(const QString& hint)
{
    if(m_latexOutputHandler) {
        ToolConfigPair userBibTool = m_latexOutputHandler->bibliographyBackendToolUserOverride();

        if(userBibTool.isValid()) {
            // now we still check whether such a tool really exists
            if (manager()->containsBibliographyTool(userBibTool)) {
                return userBibTool;
            }
            else {
                KILE_DEBUG_MAIN << "Cannot find the following bibtool set by the user:" << userBibTool;
                KILE_DEBUG_MAIN << "trying to auto-detect it now!";
                sendMessage(Warning, i18n("Manually selected bibliography tool does not exist: trying to "
                                          "auto-detect it now."));
            }
        }
    }

    // we will now try to detect the bib tool by using the given command hint
    ToolConfigPair bibTool = manager()->findFirstBibliographyToolForCommand(hint);

    if(m_latexOutputHandler) {
        // if we managed to detect a backend, store (or update) it for future runs
        if(bibTool.isValid()) {
            m_latexOutputHandler->setBibliographyBackendToolAutoDetected(bibTool);
        }
        else {
            // perhaps we have it stored from a previous run?
            bibTool = m_latexOutputHandler->bibliographyBackendToolAutoDetected();
            // perhaps the bib tools have changed from the previous run?
            if (!manager()->containsBibliographyTool(bibTool)) {
                bibTool = ToolConfigPair();
            }
        }
    }

    // this tool must always be available
    const ToolConfigPair defaultBibTool = ToolConfigPair(QString("BibTeX"), DEFAULT_TOOL_CONFIGURATION);

    // if no tool has been detected, the default is BibTeX
    return bibTool.isValid() ? bibTool : defaultBibTool;
}

void LaTeX::checkAutoRun()
{
    KILE_DEBUG_MAIN << "check for autorun, m_reRun is " << m_reRun;
    if(m_reRun >= 2) {
        KILE_DEBUG_MAIN << "Already rerun twice, doing nothing.";
        m_reRun = 0;
        return;
    }
    if(m_nErrors > 0) {
        KILE_DEBUG_MAIN << "Errors found, not running again.";
        m_reRun = 0;
        return;
    }
    bool reRunWarningFound = false;
    QString bibToolInLaTexOutput;
    bool haveUndefinedCitations = false;
    // check for "rerun" LaTeX and other tools warnings
    if(m_nWarnings > 0) {
        int sz = m_latexOutputInfoList.size();
        // the messages we are looking for are the last ones (most likely the very last one), so go from end to beginning
        for(int i = sz-1; i >= 0; --i) {
            if (m_latexOutputInfoList[i].type() == LatexOutputInfo::itmWarning
                    && m_latexOutputInfoList[i].message().contains("Rerun", Qt::CaseInsensitive)) {
                // the message could be a message from Biblatex like this:
                // Package biblatex Warning: The following entry could not be found
                // (biblatex)                in the database:
                // (biblatex)                <entry_name>
                // (biblatex)                Please verify the spelling and rerun
                // (biblatex)                LaTeX afterwards.
                //
                // our strategy: if the warning message contains "(biblatex)", Biblatex only
                // suggests to check the source files first, but not to recompile yet
                if (!m_latexOutputInfoList[i].message().contains("(biblatex)", Qt::CaseInsensitive)) {
                    reRunWarningFound = true;
                    break;
                }
            }
        }
        // Now look for messages from Biblatex like the following:
        // Please (re)run Biber on the file:
        // or
        // Please (re)run Bibtex on the file:
        QRegExp biblatexBackendMessage = QRegExp(".*Please \\(re\\)run ([A-Za-z]+) on the file", Qt::CaseInsensitive);
        for(int i = sz-1; i >= 0; --i) { // same here, start from the end
            if (m_latexOutputInfoList[i].type() == LatexOutputInfo::itmWarning
                    && biblatexBackendMessage.indexIn(m_latexOutputInfoList[i].message()) != -1) {
                bibToolInLaTexOutput = biblatexBackendMessage.cap(1);
                KILE_DEBUG_MAIN << "Captured Bib tool: " << bibToolInLaTexOutput;
                break;
            }
        }
        // If we did not get a message from Biblatex about bibtool (re)run, then
        // we look for messages like "LaTeX Warning: Citation `A' on page 234 undefined on input line 12345."
        // In that case we probably need to (re)run the bibtool.
        if (bibToolInLaTexOutput.isEmpty()) {
            QRegExp citationUndefinedMessage = QRegExp("Citation `(.+)' on page (\\d+) undefined on input line (\\d+)",
                                               Qt::CaseInsensitive);
            for(int i = 0; i < sz; ++i) {
                if (m_latexOutputInfoList[i].type() == LatexOutputInfo::itmWarning
                        && citationUndefinedMessage.indexIn(m_latexOutputInfoList[i].message()) != -1) {
                    haveUndefinedCitations = true;
                    KILE_DEBUG_MAIN << "Detected undefined citations";
                    break;
                }
            }
        }
    }

    bool asy = (m_reRun == 0) && updateAsy();
    // We run bibtool in the following cases:
    // 1. Biblatex said that we have to (in this case bibToolInLaTexOutput is not empty), OR
    // 2. There are no undefined citations and at least one of the .bib files has a younger modification
    //    date than the .bbl file, OR
    // 3. We have undefined citations and at least one of the source files (including .bib and .tex) is
    //    younger than .bbl.
    //    (If the .bbl file is younger than all of them, the next rerun will not change anything)
    bool bibs = !bibToolInLaTexOutput.isEmpty() || updateBibs(!haveUndefinedCitations);
    bool index = updateIndex();
    KILE_DEBUG_MAIN << "asy:" << asy << "bibs:" << bibs << "index:" << index << "reRunWarningFound:" << reRunWarningFound;
    // Currently, we don't properly detect yet whether asymptote has to be run.
    // So, if asymtote figures are present, we run it each time after the first LaTeX run.
    bool reRun = (asy || bibs || index || reRunWarningFound);
    KILE_DEBUG_MAIN << "reRun:" << reRun;

    if(reRun) {
        KILE_DEBUG_MAIN << "rerunning LaTeX, m_reRun is now " << m_reRun;
        Base *tool = manager()->createTool(name(), toolConfig());
        if(tool) {
            configureLaTeX(tool, source());
            // e.g. for LivePreview, it is necessary that the paths are copied to child processes
            tool->copyPaths(this);
            runChildNext(tool);
            m_reRun++;
        }
    }
    else {
        m_reRun = 0;
    }

    if(bibs) {
        KILE_DEBUG_MAIN << "need to run the bibliography tool " << bibToolInLaTexOutput;
        ToolConfigPair bibTool = determineBibliographyBackend(bibToolInLaTexOutput);
        Base *tool = manager()->createTool(bibTool.first, bibTool.second);
        if(tool) {
            configureBibTeX(tool, targetDir() + '/' + S() + '.' + tool->from());
            // e.g. for LivePreview, it is necessary that the paths are copied to child processes
            tool->copyPaths(this);
            runChildNext(tool);
        }
    }

    if(index) {
        KILE_DEBUG_MAIN << "need to run MakeIndex";
        Base *tool = manager()->createTool("MakeIndex", QString());
        KILE_DEBUG_MAIN << targetDir() << S() << tool->from();
        if(tool) {
            configureMakeIndex(tool, targetDir() + '/' + S() + '.' + tool->from());
            // e.g. for LivePreview, it is necessary that the paths are copied to child processes
            tool->copyPaths(this);
            runChildNext(tool);
        }
    }

    if(asy) {
        KILE_DEBUG_MAIN << "need to run asymptote";
        int sz = manager()->info()->allAsyFigures().size();
        for(int i = sz -1; i >= 0; --i) {
            Base *tool = manager()->createTool("Asymptote", QString());

            if(tool) {
                configureAsymptote(tool, targetDir() + '/' + S() + "-" + QString::number(i + 1) + '.' + tool->from());
                // e.g. for LivePreview, it is necessary that the paths are copied to child processes
                tool->copyPaths(this);
                runChildNext(tool);
            }
        }
    }
}


/////////////// PreviewLaTeX (dani) ////////////////

PreviewLaTeX::PreviewLaTeX(const QString& tool, Manager *mngr, bool prepare) : LaTeX(tool, mngr, prepare)
{
}

// PreviewLatex makes three steps:
// - filterLogfile()  : parse logfile and read info into InfoLists
// - updateInfoLists(): change entries of temporary file into normal tex file
// - checqCriticals()    : count errors and warnings and emit signals
bool PreviewLaTeX::finish(int r)
{
    KILE_DEBUG_MAIN << r;

    m_toolResult = r;

    if(r != Success) {
        return Compile::finish(r);
    }

    QString log = targetDir() + '/' + S() + ".log";
    manager()->parserManager()->parseOutput(this, log, source(), m_filename, m_selrow, m_docrow);

    return true;
}

void PreviewLaTeX::setPreviewInfo(const QString &filename, int selrow,int docrow)
{
    m_filename = filename;
    m_selrow = selrow;
    m_docrow = docrow;
}

/////////////// LivePreviewLaTeX ////////////////

LivePreviewLaTeX::LivePreviewLaTeX(const QString& tool, Manager *mngr, bool prepare)
    : LaTeX(tool, mngr, prepare)
{
}

void LivePreviewLaTeX::configureLaTeX(KileTool::Base *tool, const QString& source)
{
    LaTeX::configureLaTeX(tool, source);
    tool->setTargetDir(targetDir());
}

void LivePreviewLaTeX::configureBibTeX(KileTool::Base *tool, const QString& source)
{
    tool->setSource(source, targetDir());
}

void LivePreviewLaTeX::configureMakeIndex(KileTool::Base *tool, const QString& source)
{
    tool->setSource(source, targetDir());
}

void LivePreviewLaTeX::configureAsymptote(KileTool::Base *tool, const QString& source)
{
    tool->setSource(source, targetDir());
}
// PreviewLatex makes three steps:
// - filterLogfile()  : parse logfile and read info into InfoLists
// - updateInfoLists(): change entries of temporary file into normal tex file
// - checqCriticals()    : count errors and warnings and emit signals
// 	bool LivePreviewLaTeX::finish(int r)
// 	{
// 		KILE_DEBUG_MAIN << "==bool PreviewLaTeX::finish(" << r << ")=====";
//
// 		int nErrors = 0, nWarnings = 0;
// 		if(filterLogfile()) {
// 			manager()->info()->outputFilter()->updateInfoLists(m_filename,m_selrow,m_docrow);
// 			checqCriticals(nErrors,nWarnings);
// 		}
//
// 		return Compile::finish(r);
// 	}
//
// 	void LivePreviewLaTeX::setPreviewInfo(const QString &filename, int selrow,int docrow)
// 	{
// 		m_filename = filename;
// 		m_selrow = selrow;
// 		m_docrow = docrow;
// 	}


ForwardDVI::ForwardDVI(const QString& tool, Manager *mngr, bool prepare) : View(tool, mngr, prepare)
{
}

bool ForwardDVI::checkPrereqs ()
{
    KProcess okularVersionTester;
    okularVersionTester.setOutputChannelMode(KProcess::MergedChannels);
    okularVersionTester.setProgram("okular", QStringList("--version"));
    okularVersionTester.start();

    if (okularVersionTester.waitForFinished()) {
        QString output = okularVersionTester.readAll();
        QRegExp regExp = QRegExp("Okular: (\\d+).(\\d+).(\\d+)");

        if(output.contains(regExp)) {
            int majorVersion = regExp.cap(1).toInt();
            int minorVersion = regExp.cap(2).toInt();
            int veryMinorVersion = regExp.cap(3).toInt();

            //  see http://mail.kde.org/pipermail/okular-devel/2009-May/003741.html
            // 	the required okular version is > 0.8.5
            if(  majorVersion > 0  ||
                    ( majorVersion == 0 && minorVersion > 8 ) ||
                    ( majorVersion == 0 && minorVersion == 8 && veryMinorVersion > 5 ) ) {
                ; // everything okay
            }
            else {
                sendMessage(Error,i18n("The version %1.%2.%3 of okular is too old for ForwardDVI. Please update okular to version 0.8.6 or higher",majorVersion,minorVersion,veryMinorVersion));
            }
        }
    }
    // don't return false here because we don't know for sure if okular is used
    return true;
}

bool ForwardDVI::determineTarget()
{
    if (!View::determineTarget()) {
        return false;
    }

    int para = manager()->info()->lineNumber();
    KTextEditor::Document *doc = manager()->info()->activeTextDocument();

    if (!doc) {
        return false;
    }

    QString filepath = doc->url().toLocalFile();
    QString texfile = QDir(baseDir()).relativeFilePath(filepath);
    QString relativeTarget = "file:" + targetDir() + '/' + target() + "#src:" + QString::number(para + 1) + ' ' + texfile; // space added, for files starting with numbers
    QString absoluteTarget = "file:" + targetDir() + '/' + target() + "#src:" + QString::number(para + 1) + filepath;

    if(readEntry("type") == "DocumentViewer") {
        addDict("%dir_target", targetDir());
        addDict("%target", target());
        addDict("%sourceFileName", filepath);
        addDict("%sourceLine", QString::number(para + 1));
    }
    else {
        addDict("%dir_target", QString());
        addDict("%target", relativeTarget);
    }

    addDict("%absolute_target", absoluteTarget);
    KILE_DEBUG_MAIN << "==KileTool::ForwardDVI::determineTarget()=============\n";
    KILE_DEBUG_MAIN << "\tusing  (absolute)" << absoluteTarget;
    KILE_DEBUG_MAIN << "\tusing  (relative)" << relativeTarget;

    return true;
}

ViewBib::ViewBib(const QString& tool, Manager *mngr, bool prepare) : View(tool, mngr, prepare)
{
}

bool ViewBib::determineSource()
{
    KILE_DEBUG_MAIN << "==ViewBib::determineSource()=======";
    if (!View::determineSource()) {
        return false;
    }

    QString path = source(true);
    QFileInfo info(path);

    //get the bibliographies for this source
    QStringList bibs = manager()->info()->allBibliographies(manager()->info()->docManager()->textInfoFor(path));
    KILE_DEBUG_MAIN << "\tfound " << bibs.count() << " bibs";
    if(bibs.count() > 0) {
        QString bib = bibs.front();
        if (bibs.count() > 1) {
            //show dialog
            bool bib_selected = false;
            KileListSelector *dlg = new KileListSelector(bibs, i18n("Select Bibliography"),i18n("Select a bibliography"));
            if (dlg->exec() && dlg->hasSelection()) {
                bib = dlg->selectedItems().first();
                bib_selected = true;
                KILE_DEBUG_MAIN << "Bibliography selected : " << bib;
            }
            delete dlg;

            if(!bib_selected) {
                sendMessage(Warning, i18n("No bibliography selected."));
                return false;
            }
        }
        KILE_DEBUG_MAIN << "filename before: " << info.path();
        setSource(manager()->info()->checkOtherPaths(info.path(),bib + ".bib",KileInfo::bibinputs));
    }
    else if(info.exists()) { //active doc is a bib file
        KILE_DEBUG_MAIN << "filename before: " << info.path();
        setSource(manager()->info()->checkOtherPaths(info.path(),info.fileName(),KileInfo::bibinputs));
    }
    else {
        sendMessage(Error, i18n("No bibliographies found."));
        return false;
    }
    return true;
}

ViewHTML::ViewHTML(const QString& tool, Manager *mngr, bool prepare) : View(tool, mngr, prepare)
{
}

bool ViewHTML::determineTarget()
{
    if (target().isNull()) {
        //setRelativeBaseDir(S());
        QString dir = readEntry("relDir");
        QString trg = readEntry("target");

        if(!dir.isEmpty()) {
            translate(dir);
            setRelativeBaseDir(dir);
        }

        if(!trg.isEmpty()) {
            translate(trg);
            setTarget(trg);
        }

        //auto-detect the file to view
        if(dir.isEmpty() && trg.isEmpty()) {
            QFileInfo file1 = QFileInfo(baseDir() + '/' + S() + "/index.html");
            QFileInfo file2 = QFileInfo(baseDir() + '/' + S() + ".html");

            bool read1 = file1.isReadable();
            bool read2 = file2.isReadable();

            if(!read1 && !read2) {
                sendMessage(Error, i18n("Unable to find %1 or %2; if you are trying to view some other HTML file, go to Settings->Configure Kile->Tools->ViewHTML->Advanced.", file1.absoluteFilePath(), file2.absoluteFilePath()));
                return false;
            }

            //both exist, take most recent
            if(read1 && read2) {
                read1 = file1.lastModified() > file2.lastModified();
                read2 = !read1;
            }

            if(read1) {
                dir = S();
                trg = "index.html";

                translate(dir);
                setRelativeBaseDir(dir);
                translate(trg);
                setTarget(trg);
            }
        }
    }

    return View::determineTarget();
}
}

/*
 * BibliographyCompile
 */

const QString KileTool::BibliographyCompile::ToolClass = "Bibliography";

KileTool::BibliographyCompile::BibliographyCompile(const QString& name, KileTool::Manager* manager, bool prepare)
    : Compile(name, manager, prepare)
{

}



