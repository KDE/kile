/*************************************************************************************
  Copyright (C) 2004 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                2012-2018 by Michel Ludwig (michel.ludwig@kdemail.net)
 *************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "configtester.h"

#include <okular/interfaces/viewerinterface.h>

#include <QTimer>

#include <KAboutData>
#include <KConfig>
#include <KIO/CopyJob>
#include <KJob>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KProcess>

#include <QTemporaryDir>
#include <QStandardPaths>

#include "documentinfo.h"
#include "kiledebug.h"
#include "kiledocmanager.h"
#include "kiletoolmanager.h"
#include "kileinfo.h"
#include "kileconfig.h"
#include "kiletool.h"
#include "kiletool_enums.h"
#include "kileversion.h"

ConfigTest::ConfigTest(const QString& testGroup, const QString &name, bool isCritical)
    : m_testGroup(testGroup)
    , m_name(name)
    , m_isCritical(isCritical)
    , m_isSilent(false)
    , m_status(NotRun)
{
}

ConfigTest::~ConfigTest()
{
}

int ConfigTest::status() const
{
    return m_status;
}

QString ConfigTest::name() const
{
    return m_name;
}

QString ConfigTest::testGroup() const
{
    return m_testGroup;
}

QString ConfigTest::resultText() const
{
    return m_resultText;
}

void ConfigTest::addDependency(ConfigTest *test)
{
    m_dependencyTestList.push_back(test);
}

bool ConfigTest::allDependenciesSucceeded() const
{
    Q_FOREACH(ConfigTest *test, m_dependencyTestList) {
        if(test->status() != Success) {
            return false;
        }
    }
    return true;
}

bool ConfigTest::isCritical() const
{
    return m_isCritical;
}

bool ConfigTest::isSilent() const
{
    return m_isSilent;
}

void ConfigTest::setSilent(bool b)
{
    m_isSilent = b;
}

void ConfigTest::setName(const QString& name)
{
    m_name = name;
}

Tester::Tester(KileInfo *kileInfo, QObject *parent)
    : QObject(parent),
      m_ki(kileInfo),
      m_tempDir(Q_NULLPTR),
      m_testsDone(0)
{
    m_tempDir = new QTemporaryDir();

    setupTests();
    m_nextTestIterator = m_testList.begin();
}

Tester::~Tester()
{
    if (m_tempDir) m_tempDir->remove();
    delete m_tempDir;
    qDeleteAll(m_testList);
}

void Tester::runTests()
{
    const QString& destinationDirectory = m_tempDir->path();
    const QString& testDirectory =
#ifdef Q_OS_WIN
        QStandardPaths::locate(QStandardPaths::AppDataLocation, "kile/test", QStandardPaths::LocateDirectory);
#else
        QStandardPaths::locate(QStandardPaths::AppDataLocation, "test", QStandardPaths::LocateDirectory);
#endif
    KIO::CopyJob *copyJob = KIO::copyAs(QUrl::fromLocalFile(testDirectory), QUrl::fromLocalFile(destinationDirectory), KIO::HideProgressInfo | KIO::Overwrite);
    connect(copyJob, SIGNAL(result(KJob*)), this, SLOT(handleFileCopyResult(KJob*)));
    emit(percentageDone(0));
}

void Tester::handleFileCopyResult(KJob* job)
{
    if(job->error()) {
        emit(finished(false));
    }
    else {
        startNextTest();
    }
}

void Tester::addResult(const QString &tool, ConfigTest* testResult)
{
    m_results[tool].push_back(testResult);
}

QStringList Tester::testGroups()
{
    return m_results.keys();
}

QList<ConfigTest*> Tester::resultForGroup(const QString & tool)
{
    return m_results[tool];
}

// 'isCritical' is set to true iff one tool that failed was critical
int Tester::statusForGroup(const QString &testGroup, bool *isCritical)
{
    if(isCritical) {
        *isCritical = false;
    }
    QList<ConfigTest*> tests = m_results[testGroup];
    int status = ConfigTest::Success;
    for(int i = 0; i < tests.count(); ++i) {
        if(tests[i]->status() == ConfigTest::Failure) {
            if(isCritical && tests[i]->isCritical()) {
                *isCritical = true;
            }
            status = ConfigTest::Failure;
        }
    }
    return status;
}

void Tester::startNextTest()
{
    KILE_DEBUG_MAIN;
    if(m_nextTestIterator != m_testList.end()) {
        m_currentTest = *m_nextTestIterator;
        ++m_nextTestIterator;
        if(!m_currentTest->allDependenciesSucceeded()) {
            QTimer::singleShot(0, this, SLOT(startNextTest()));
            return;
        }
        // we want events to be handled inbetween tests -> QueuedConnection
        connect(m_currentTest, SIGNAL(testComplete(ConfigTest*)), this, SLOT(handleTestComplete(ConfigTest*)), Qt::QueuedConnection);
        m_currentTest->call();
    }
    else {
        emit(percentageDone(100));
        emit(finished(true));
    }
}

void Tester::handleTestComplete(ConfigTest *test)
{
    KILE_DEBUG_MAIN;
    if(!test->isSilent()) {
        addResult(test->testGroup(), test);
    }
    ++m_testsDone;
    emit(percentageDone((m_testsDone / (float) m_testList.size()) * 100.0));
    startNextTest();
}


TestToolInKileTest::TestToolInKileTest(const QString& testGroup, KileInfo *kileInfo, const QString& toolName,
                                       const QString& filePath,
                                       bool isCritical)
    : ConfigTest(testGroup, i18n("Running in Kile"), isCritical),
      m_ki(kileInfo),
      m_toolName(toolName),
      m_filePath(filePath)
{
}

TestToolInKileTest::~TestToolInKileTest()
{
}

void TestToolInKileTest::call()
{
    KileDocument::TextInfo *textInfo = m_ki->docManager()->fileOpen(QUrl::fromLocalFile(m_filePath));
    if(!textInfo) {
        reportFailure();
        return;
    }
    m_documentUrl = textInfo->url();

    KileTool::Base *tool = m_ki->toolManager()->createTool(m_toolName, QString(), false);
    if(!tool) {
        m_ki->docManager()->fileClose(m_documentUrl);
        m_status = Failure;
        m_resultText = i18n("Tool not found.\n"
                            "Kile is not configured correctly. Go to Settings->Configure Kile->Tools "
                            "and either fix the problem or change to the default settings."
                           );
        emit(testComplete(this));
        return;
    }
    // We don't want the tool to spawn subtools (especially, for LaTeX-style tools).
    // If we did, we might come into the situation that a subtool is launched before the
    // parsing is complete, which could trigger a "root document not found" error message.
    tool->setEntry("autoRun", "no");
    connect(tool, SIGNAL(done(KileTool::Base*,int,bool)), this, SLOT(handleToolExit(KileTool::Base*,int,bool)), Qt::UniqueConnection);
    connect(tool, SIGNAL(failedToRun(KileTool::Base*, int)), this, SLOT(reportFailure()));
    m_ki->toolManager()->run(tool);
}

void TestToolInKileTest::reportSuccess()
{
    m_ki->docManager()->fileClose(m_documentUrl);
    m_documentUrl.clear();

    m_status = Success;
    m_resultText = i18n("Passed");
    emit(testComplete(this));
}

void TestToolInKileTest::reportFailure()
{
    m_ki->docManager()->fileClose(m_documentUrl);
    m_documentUrl.clear();

    m_status = Failure;
    m_resultText = i18n("Failed");
    emit(testComplete(this));
}


void TestToolInKileTest::handleToolExit(KileTool::Base *tool, int status, bool childToolSpawned)
{
    Q_UNUSED(tool);
    Q_UNUSED(childToolSpawned);

    if(status == KileTool::Success) {
        reportSuccess();
    }
    else {
        reportFailure();
    }
}

OkularVersionTest::OkularVersionTest(const QString& testGroup, bool isCritical)
    : ConfigTest(testGroup, i18n("Version"), isCritical)
{
}

OkularVersionTest::~OkularVersionTest()
{
}

void OkularVersionTest::call()
{
    KPluginLoader pluginLoader(OKULAR_LIBRARY_NAME);
    KPluginFactory *factory = pluginLoader.factory();

    if (!factory) {
        m_status = Failure;
    }
    else {
        KParts::ReadOnlyPart *part = factory->create<KParts::ReadOnlyPart>();
        Okular::ViewerInterface *viewerInterface = dynamic_cast<Okular::ViewerInterface*>(part);

        if(!viewerInterface) {
            // OkularPart doesn't provide the ViewerInterface
            m_status = Failure;
        }
        else {
            m_status = Success;
            // it seems that the version of OkularPart cannot be detected, so we don't try it
            m_resultText = i18n("Embedding of Okular is supported");
        }
        delete part;
    }

    delete factory;

    emit(testComplete(this));
}


FindProgramTest::FindProgramTest(const QString& testGroup, const QString& programName, bool isCritical)
    : ConfigTest(testGroup, i18n("Binary"), isCritical),
      m_programName(programName)
{
}

FindProgramTest::~FindProgramTest()
{
}

void FindProgramTest::call()
{
    QString execPath = QStandardPaths::findExecutable(m_programName);
#ifdef Q_OS_WIN
    if(execPath.isEmpty()) {
        execPath = QStandardPaths::findExecutable(m_programName, QStringList(QCoreApplication::applicationDirPath()));
    }
#endif
    bool thisIsWindowsConvertExe = false;
#ifdef Q_OS_WIN
    QFileInfo execPathInfo(execPath);
    thisIsWindowsConvertExe = (m_programName == "convert") && (execPathInfo.dir().dirName() == "system32");
#endif
    if(execPath.isEmpty() || thisIsWindowsConvertExe) {
        m_status = Failure;
        if(!m_additionalFailureMessage.isEmpty()) {
            if(isCritical()) {
                m_resultText = i18nc("additional failure message given as argument",
                                     "Could not find the binary for this essential tool. %1", m_additionalFailureMessage);
            }
            else {
                m_resultText = i18nc("additional failure message given as argument",
                                     "No executable '%1' found. %2", m_programName, m_additionalFailureMessage);
            }
        }
        else {
            if(isCritical()) {
                m_resultText = i18n("Could not find the binary for this essential tool");
            }
            else {
                m_resultText = i18n("No executable '%1' found", m_programName);
            }
        }
    }
    else {
        m_status = Success;
        m_resultText = i18nc("executable => path", "Found (%1 => %2)", m_programName, execPath);
    }
    emit(testComplete(this));
}

void FindProgramTest::setAdditionalFailureMessage(const QString& s)
{
    m_additionalFailureMessage = s;
}

ProgramTest::ProgramTest(const QString& testGroup, const QString& programName, const QString& workingDir,
                         const QString& arg0,
                         const QString& arg1,
                         const QString& arg2,
                         bool isCritical)
    : ConfigTest(testGroup, i18n("Simple Test"), isCritical),
      m_testProcess(Q_NULLPTR),
      m_programName(programName),
      m_workingDir(workingDir),
      m_arg0(arg0),
      m_arg1(arg1),
      m_arg2(arg2)
{
}

ProgramTest::~ProgramTest()
{
    delete m_testProcess;
}

void ProgramTest::call()
{
    m_testProcess = new KProcess();
    m_testProcess->setWorkingDirectory(m_workingDir);
    QStringList argList;
    if(!m_arg0.isEmpty()) {
        argList << m_arg0;
    }
    if(!m_arg1.isEmpty()) {
        argList << m_arg1;
    }
    if(!m_arg2.isEmpty()) {
        argList << m_arg2;
    }
    m_testProcess->setProgram(m_programName, argList);
    if (!KileConfig::teXPaths().isEmpty()) {
        m_testProcess->setEnv("TEXINPUTS", KileInfo::expandEnvironmentVars(KileConfig::teXPaths() + ":$TEXINPUTS"));
    }
    connect(m_testProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(handleTestProcessFinished(int,QProcess::ExitStatus)));
    connect(m_testProcess, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(handleTestProcessError(QProcess::ProcessError)));
    m_testProcess->start();
}

void ProgramTest::handleTestProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_testProcess->deleteLater();
    m_testProcess = Q_NULLPTR;

    if(exitStatus == QProcess::NormalExit && exitCode == 0) {
        processFinishedSuccessfully();
    }
    else {
        reportFailure();
    }
}

void ProgramTest::processFinishedSuccessfully()
{
    reportSuccess();
}

void ProgramTest::handleTestProcessError(QProcess::ProcessError error)
{
    Q_UNUSED(error);

    m_testProcess->deleteLater();
    m_testProcess = Q_NULLPTR;
    reportFailure();
}

void ProgramTest::reportSuccess()
{
    m_resultText = i18n("Passed");
    m_status = Success;
    emit(testComplete(this));
}

void ProgramTest::reportFailure()
{
    if(isCritical()) {
        m_resultText = i18n("This essential tool does not work; please check your installation.");
    }
    else {
        m_resultText = i18n("Failed");
    }
    m_status = Failure;
    emit(testComplete(this));
}


LaTeXSrcSpecialsSupportTest::LaTeXSrcSpecialsSupportTest(const QString& testGroup, const QString& workingDir,
        const QString& fileBaseName)
    : ProgramTest(testGroup, "latex", workingDir, "-src-specials", "--interaction=nonstopmode", fileBaseName + ".tex", false),
      m_fileBaseName(fileBaseName)
{
    setName(i18n("Source Specials Switch"));
}

LaTeXSrcSpecialsSupportTest::~LaTeXSrcSpecialsSupportTest()
{
}

void LaTeXSrcSpecialsSupportTest::processFinishedSuccessfully()
{
    // before we can report success, we still have to perform the
    // following check:
    // src-specials are supported if the created file contains source
    // information (LaTeX doesn't report unknown command line flags as
    // errors). Hence, we now check whether the created file contains
    // the string 'src:'.
    QFile file(m_workingDir + '/' + m_fileBaseName + ".dvi");
    if (!file.open(QIODevice::ReadOnly)) {
        reportFailure();
        return;
    }
    // we read everything as it's a small file
    QByteArray array = file.readAll();
    file.close();
    if(!array.contains("src:")) {
        reportFailure();
        return;
    }
    reportSuccess();
}

void LaTeXSrcSpecialsSupportTest::reportSuccess()
{
    m_resultText = i18n("Supported, use the 'Modern' configuration for (La)TeX to auto-enable inverse and forward search capabilities.");
    m_status = Success;
    emit(testComplete(this));
}

void LaTeXSrcSpecialsSupportTest::reportFailure()
{
    m_resultText = i18n("Not supported, use the srcltx package to enable the inverse and forward search capabilities.");
    m_status = Failure;
    emit(testComplete(this));
}


SyncTeXSupportTest::SyncTeXSupportTest(const QString& testGroup, const QString& toolName, const QString& workingDir,
                                       const QString& fileBaseName)
    : ProgramTest(testGroup, toolName, workingDir, "-synctex=1", "--interaction=nonstopmode", fileBaseName + ".tex", false),
      m_fileBaseName(fileBaseName)
{
    setName(i18n("SyncTeX Support"));
}

SyncTeXSupportTest::~SyncTeXSupportTest()
{
}

void SyncTeXSupportTest::reportSuccess()
{
    m_resultText = i18n("Supported, use the 'Modern' configuration for PDFLaTeX and XeLaTeX to auto-enable inverse and forward search capabilities.");
    m_status = Success;
    emit(testComplete(this));
}

void SyncTeXSupportTest::reportFailure()
{
    m_resultText = i18n("Not supported");
    m_status = Failure;
    emit(testComplete(this));
}

void SyncTeXSupportTest::processFinishedSuccessfully()
{
    // before we can report success, we still have to check
    // whether a .synctex.gz file has been generated
    QFile file(m_workingDir + '/' + m_fileBaseName + ".synctex.gz");
    if (!file.exists()) {
        reportFailure();
        return;
    }
    reportSuccess();
}

void Tester::installConsecutivelyDependentTests(ConfigTest *t1, ConfigTest *t2, ConfigTest *t3, ConfigTest *t4)
{
    if(!t1) {
        return;
    }
    m_testList << t1;
    if(!t2) {
        return;
    }
    t2->addDependency(t1);
    m_testList << t2;
    if(!t3) {
        return;
    }
    t3->addDependency(t2);
    m_testList << t3;
    if(!t4) {
        return;
    }
    t4->addDependency(t3);
    m_testList << t4;
}

void Tester::setupTests()
{

    /*
    testFile=test_plain.tex
    echo "opening $basedir/$testFile"
    $openDoc"$basedir/$testFile"

    echo "starting test: TeX"
    setTool TeX
    tool="tex --interaction=nonstopmode"
    setKey mustpass "where,basic,kile"
    setKey executable tex
    setKey where `which tex`
    setKey version `getTeXVersion tex`
    performTest basic "$tool test_plain.tex"
    performKileTest kile "run TeX"
    */
    installConsecutivelyDependentTests(
        new FindProgramTest("TeX", "tex", true),
        new ProgramTest("TeX", "tex", m_tempDir->path(), "--interaction=nonstopmode",  "test_plain.tex", "", true),
        new TestToolInKileTest("TeX", m_ki, "TeX", m_tempDir->path() + '/' + "test_plain.tex", true));
    /*
    echo "starting test: PDFTeX"
    setTool PDFTeX
    tool="pdftex --interaction=nonstopmode"
    setKey mustpass ""
    setKey executable pdftex
    setKey where `which pdftex`
    performTest basic "$tool test_plain.tex"
    performKileTest kile "run PDFTeX"
    $closeDoc
    */
    installConsecutivelyDependentTests(
        new FindProgramTest("PDFTeX", "pdftex", false),
        new ProgramTest("PDFTeX", "pdftex", m_tempDir->path(), "--interaction=nonstopmode",  "test_plain.tex", "", false),
        new TestToolInKileTest("PDFTeX", m_ki, "PDFTeX", m_tempDir->path() + '/' + "test_plain.tex", false));
    /*
    testFileBase="test"
    testFile=$testFileBase.tex
    echo "opening $basedir/$testFile"
    $openDoc"$basedir/$testFile"
    echo "starting test: LaTeX"
    setTool LaTeX
    tool="latex --interaction=nonstopmode"
    setKey mustpass "where,basic,kile"
    setKey executable latex
    setKey where `which latex`
    performTest basic "$tool $testFile"
    performKileTest kile "run LaTeX"
    performTest src "$tool -src $testFile"
    */
    ProgramTest *latexProgramTest = new ProgramTest("LaTeX", "latex", m_tempDir->path(), "--interaction=nonstopmode",  "test.tex", "", true);
    m_laTeXSrcSpecialsSupportTest = new LaTeXSrcSpecialsSupportTest("LaTeX", m_tempDir->path(), "test");
    installConsecutivelyDependentTests(
        new FindProgramTest("LaTeX", "latex", true),
        latexProgramTest,
        new TestToolInKileTest("LaTeX", m_ki, "LaTeX", m_tempDir->path() + '/' + "test.tex", true),
        m_laTeXSrcSpecialsSupportTest);
    /*
    echo "starting test: PDFLaTeX"
    setTool PDFLaTeX
    setKey mustpass ""
    setKey executable pdflatex
    setKey where `which pdflatex`
    performTest basic "pdflatex $testFile"
    performKileTest kile "run PDFLaTeX"
    */
    m_pdfLaTeXSyncTeXSupportTest = new SyncTeXSupportTest("PDFLaTeX", "pdflatex", m_tempDir->path(), "test");
    installConsecutivelyDependentTests(
        new FindProgramTest("PDFLaTeX", "pdflatex", false),
        new ProgramTest("PDFLaTeX", "pdflatex", m_tempDir->path(), "--interaction=nonstopmode",  "test.tex", "", false),
        new TestToolInKileTest("PDFLaTeX", m_ki, "PDFLaTeX", m_tempDir->path() + '/' + "test.tex", false),
        m_pdfLaTeXSyncTeXSupportTest);
    /*
    echo "starting test: DVItoPS"
    setTool DVItoPS
    setKey mustpass ""
    setKey executable dvips
    setKey where `which dvips`
    if [ -r $testFileBase.dvi ]; then performKileTest kile "run DVItoPS"; fi
    */
    TestToolInKileTest *dvipsKileTest = new TestToolInKileTest("DVItoPS", m_ki, "DVItoPS", m_tempDir->path() + '/' + "test.tex", false);
    dvipsKileTest->addDependency(latexProgramTest);
    installConsecutivelyDependentTests(
        new FindProgramTest("DVItoPS", "dvips", false),
        dvipsKileTest);
    /*
    echo "starting test: DVItoPDF"
    setTool DVItoPDF
    setKey mustpass ""
    setKey executable dvipdfmx
    setKey where `which dvipdfmx`
    if [ -r $testFileBase.dvi ]; then performKileTest kile "run DVItoPDF"; fi
    */
    TestToolInKileTest *dvipdfmxKileTest = new TestToolInKileTest("DVItoPDF", m_ki, "DVItoPDF", m_tempDir->path() + '/' + "test.tex", false);
    dvipdfmxKileTest->addDependency(latexProgramTest);
    installConsecutivelyDependentTests(
        new FindProgramTest("DVItoPDF", "dvipdfmx", false),
        dvipdfmxKileTest);
    /*
    echo "starting test: PStoPDF"
    setTool PStoPDF
    setKey mustpass ""
    setKey executable ps2pdf
    setKey where `which ps2pdf`
    if [ -r $testFileBase.ps ]; then performKileTest kile "run PStoPDF"; fi
    $closeDoc
    */
    TestToolInKileTest *ps2pdfKileTest = new TestToolInKileTest("PStoPDF", m_ki, "PStoPDF", m_tempDir->path() + '/' + "test.tex", false);
    ps2pdfKileTest->addDependency(dvipsKileTest);
    installConsecutivelyDependentTests(
        new FindProgramTest("PStoPDF", "ps2pdf", false),
        ps2pdfKileTest);
    /*
    echo "starting test: BibTeX"
    setTool BibTeX
    setKey mustpass ""
    setKey executable bibtex
    setKey where `which bibtex`
    if [ -r "test.dvi" ] #LaTeX is working
    then
    	testFileBase=test_bib
    	testFile=$testFileBase.tex
    	$openDoc"$basedir/$testFile"
    	latex --interaction=nonstopmode $testFile
    	performTest basic "bibtex $testFileBase"
    	performKileTest kile "run BibTeX"
    	$closeDoc
    fi
    */
    TestToolInKileTest *latexForBibTeX = new TestToolInKileTest("BibTeX", m_ki, "LaTeX", m_tempDir->path() + '/' + "test_bib.tex", false);
    latexForBibTeX->addDependency(latexProgramTest);
    latexForBibTeX->setSilent(true);
    ProgramTest *bibtexProgramTest = new ProgramTest("BibTeX", "bibtex", m_tempDir->path(), "test_bib",  "", "", false);
    bibtexProgramTest->addDependency(latexForBibTeX);
    TestToolInKileTest *bibtexKileTest = new TestToolInKileTest("BibTeX", m_ki, "BibTeX", m_tempDir->path() + '/' + "test_bib.tex", false);
    bibtexKileTest->addDependency(latexProgramTest);
    installConsecutivelyDependentTests(
        new FindProgramTest("BibTeX", "bibtex", false),
        latexForBibTeX,
        bibtexProgramTest,
        bibtexKileTest);
    /*
    echo "starting test: MakeIndex"
    setTool MakeIndex
    setKey mustpass ""
    setKey executable makeindex
    setKey where `which makeindex`

    if [ -r "test.dvi" ] #LaTeX is working
    then
    	testFileBase=test_index
    	testFile=$testFileBase.tex
    	$openDoc"$basedir/$testFile"
    	latex --interaction=nonstopmode $testFile
    	performTest basic "makeindex $testFileBase"
    	performKileTest kile "run MakeIndex"
    	$closeDoc
    fi
    */
    TestToolInKileTest *latexForMakeIndex = new TestToolInKileTest("MakeIndex", m_ki, "LaTeX", m_tempDir->path() + '/' + "test_index.tex", false);
    latexForMakeIndex->addDependency(latexProgramTest);
    latexForMakeIndex->setSilent(true);
    ProgramTest *makeIndexProgramTest = new ProgramTest("MakeIndex", "makeindex", m_tempDir->path(), "test_index",  "", "", false);
    makeIndexProgramTest->addDependency(latexProgramTest);
    TestToolInKileTest *makeindexKileTest = new TestToolInKileTest("MakeIndex", m_ki, "MakeIndex", m_tempDir->path() + '/' + "test_index.tex", false);
    makeindexKileTest->addDependency(latexProgramTest);
    installConsecutivelyDependentTests(
        new FindProgramTest("MakeIndex", "makeindex", false),
        latexForMakeIndex,
        makeIndexProgramTest,
        makeindexKileTest);
    /*
    echo "starting test: Okular"
    setTool Okular
    setKey mustpass "where"
    setKey executable okular
    setKey version `getOkularVersion okular`
    performTest okular "isTheOkularVersionRecentEnough"
    setKey where `which okular`
    */
    m_okularVersionTest = new OkularVersionTest("Okular", false);
    installConsecutivelyDependentTests(
        new FindProgramTest("Okular", "okular", false),
        m_okularVersionTest);
    /*
    echo "starting test: Acroread"
    setTool Acroread
    setKey mustpass ""
    setKey executable acroread
    setKey where `which acroread`
    */

    /*
    echo "starting test: DVItoPNG"
    setTool DVItoPNG
    setKey mustpass ""
    setKey executable dvipng
    setKey where `which dvipng`
    */
    FindProgramTest *dvipngProgramTest = new FindProgramTest("DVItoPNG", "dvipng", false);
    dvipngProgramTest->setAdditionalFailureMessage(i18n("PNG previews cannot be used for mathgroups in the bottom preview pane"));
    m_testList << dvipngProgramTest;
    /*
    echo "starting test: Convert"
    setTool Convert
    setKey mustpass ""
    setKey executable convert
    setKey where `which convert`
    */
    FindProgramTest *convertProgramTest = new FindProgramTest("Convert", "convert", false);
    convertProgramTest->setAdditionalFailureMessage(i18n("PNG previews cannot be used with conversions 'dvi->ps->png' and 'pdf->png' in the bottom preview pane"));
    m_testList << convertProgramTest;
}

bool Tester::isSyncTeXSupportedForPDFLaTeX()
{
    return (m_pdfLaTeXSyncTeXSupportTest->status() == ConfigTest::Success);
}

bool Tester::isViewerModeSupportedInOkular()
{
    return (m_okularVersionTest->status() == ConfigTest::Success);
}

bool Tester::areSrcSpecialsSupportedForLaTeX()
{
    return (m_laTeXSrcSpecialsSupportTest->status() == ConfigTest::Success);
}

