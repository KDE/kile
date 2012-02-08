/*************************************************************************************
  Copyright (C) 2004 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                2012 by Michel Ludwig (michel.ludwig@kdemail.net)
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

#include <QTimer>

#include <KAboutData>
#include <KConfig>
#include <KGlobal>
#include <KIO/CopyJob>
#include <KJob>
#include <KLocale>
#include <KParts/Factory>
#include <KProcess>
#include <KStandardDirs>
#include <KTempDir>

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
: m_testGroup(testGroup), m_name(name), m_isCritical(isCritical), m_isSilent(false), m_status(NotRun)
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
/*
QString ConfigTest::prettyName(const QString &test)
{
	if ( s_prettyName.contains(test) )
		return s_prettyName[test];
	else
		return test;
}*/

// QString ConfigTest::criticalMessage(const QString &test)
// {
// 	if ( s_msgCritical.contains(test) )
// 		return s_msgCritical[test];
// 	else
// 		return i18n("Critical failure");
// }

Tester::Tester(KileInfo *kileInfo, QObject *parent)
: QObject(parent),
  m_ki(kileInfo),
  m_tempDir(NULL),
  m_testsDone(0)
{
	m_tempDir = new KTempDir();

	setupTests();
	m_nextTestIterator = m_testList.begin();
#if 0
	// add some special messages, when programs are not installed,
	// which are not needed, but probably useful for the work with kile
FIXME: not finished yet:
	ConfigTest::addFailureMessage("dvipng", i18n("You cannot use the png preview for mathgroups in the bottom bar."));
	ConfigTest::addFailureMessage("convert", i18n("You cannot use the png previews with conversions 'dvi->ps->png' and 'pdf->png'."));
#ifdef Q_WS_WIN
	ConfigTest::addFailureMessage("acrord32", i18n("You cannot open pdf documents with Adobe Reader because acroread could not be found in your path.  <br>If Adobe Reader is your default pdf viewer, try setting ViewPDF to System Default.  Alternatively, you could use Okular."));
#else
	ConfigTest::addFailureMessage("acroread", i18n("You cannot open pdf documents with Adobe Reader, but you could use Okular."));
#endif
#endif
}

Tester::~Tester()
{
	if (m_tempDir) m_tempDir->unlink();
	delete m_tempDir;
	qDeleteAll(m_testList);
}

void Tester::runTests()
{
	const QString& destinationDirectory = m_tempDir->name();
	const QString& testDirectory = KStandardDirs::locate("appdata", "test/");
	KIO::CopyJob *copyJob = KIO::copyAs(KUrl::fromPath(testDirectory), KUrl::fromPath(destinationDirectory), KIO::HideProgressInfo | KIO::Overwrite);
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

QStringList Tester::testedTools()
{
	return m_results.keys();
}

QList<ConfigTest*> Tester::resultForTool(const QString & tool)
{
	return m_results[tool];
}

// 'isCritical' is set to true iff one tool that failed was critical
int Tester::statusForTool(const QString &testGroup, bool *isCritical)
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
	KILE_DEBUG();
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
	KILE_DEBUG();
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
	KileDocument::TextInfo *textInfo = m_ki->docManager()->fileOpen(KUrl::fromPath(m_filePath));
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

	connect(tool, SIGNAL(done(KileTool::Base*,int,bool)), this, SLOT(handleToolExit(KileTool::Base*,int)), Qt::UniqueConnection);
	m_ki->toolManager()->run(tool);
}

void TestToolInKileTest::reportSuccess()
{
	m_status = Success;
	m_resultText = i18n("Passed");
	emit(testComplete(this));
}

void TestToolInKileTest::reportFailure()
{
	m_status = Failure;
	m_resultText = i18n("Failed");
	emit(testComplete(this));
}

void TestToolInKileTest::handleToolExit(KileTool::Base *tool, int status)
{
	Q_UNUSED(tool);

	m_ki->docManager()->fileClose(m_documentUrl);

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
	QString version;
	if(!factory) {
		m_status = Failure;
		m_resultText = i18n("Okular could not be found");
		return;
	}
	else {
		version = factory->componentData().aboutData()->version();
	}

	m_isViewerModeSupported = false;
	if(compareVersionStrings(version, "0.14.80") >= 0) {
		m_status = Success;
		m_isViewerModeSupported = true;
		m_resultText = i18n("%1 - Forward Search and Embedded Viewer Mode are supported",version);
	}
	else if(compareVersionStrings(version, "0.8.6") >= 0) {
		m_status = Failure;
		m_resultText = i18n("%1 - Forward Search is supported, but not Embedded Viewer mode",version);
	}
	else {
		m_status = Failure;
		m_resultText = i18n("%1 - The installed version is too old for Forward Search and Embedded Viewer mode; "
		                    "you must use at least version 0.8.6 for Forward Search, and 0.14.80 for Embedded Viewer Mode", version);
	}

	emit(testComplete(this));
}

bool OkularVersionTest::isViewerModeSupported() const
{
	return m_isViewerModeSupported;
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
	const QString execPath = KStandardDirs::findExe(m_programName);
	if(execPath.isEmpty()) {
		m_status = Failure;
		if(isCritical()) {
			m_resultText = i18n("Could not find the binary for this essential tool");
		}
		else {
			m_resultText = i18n("No executable '%1' found").arg(m_programName);
		}
	}
	else {
		m_status = Success;
		m_resultText = i18nc("executable => path", "Found (%1 => %2)", m_programName, execPath);
	}
	emit(testComplete(this));
}

ProgramTest::ProgramTest(const QString& testGroup, const QString& programName, const QString& workingDir,
                                                                               const QString& arg0,
                                                                               const QString& arg1,
                                                                               const QString& arg2,
                                                                               bool isCritical)
: ConfigTest(testGroup, i18n("Simple Test"), isCritical),
  m_testProcess(NULL),
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
	m_testProcess = NULL;

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
	m_testProcess = NULL;
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
	m_testList
	<< new FindProgramTest("TeX", "tex", true)
	<< new ProgramTest("TeX", "tex", m_tempDir->name(), "--interaction=nonstopmode",  "test_plain.tex", "", true)
	<< new TestToolInKileTest("TeX", m_ki, "TeX", m_tempDir->name() + '/' + "test_plain.tex", true);
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
	m_testList
	<< new FindProgramTest("PDFTeX", "pdftex", false)
	<< new ProgramTest("PDFTeX", "pdftex", m_tempDir->name(), "--interaction=nonstopmode",  "test_plain.tex", "", false)
	<< new TestToolInKileTest("PDFTeX", m_ki, "PDFTeX", m_tempDir->name() + '/' + "test_plain.tex", false);
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
	m_testList
	<< new FindProgramTest("LaTeX", "latex", true);
	ProgramTest *latexProgramTest = new ProgramTest("LaTeX", "latex", m_tempDir->name(), "--interaction=nonstopmode",  "test.tex", "", true);
	m_testList
	<< latexProgramTest
	<< new TestToolInKileTest("LaTeX", m_ki, "LaTeX", m_tempDir->name() + '/' + "test.tex", true)
	<< new LaTeXSrcSpecialsSupportTest("LaTeX", m_tempDir->name(), "test");
/*
echo "starting test: PDFLaTeX"
setTool PDFLaTeX
setKey mustpass ""
setKey executable pdflatex
setKey where `which pdflatex`
performTest basic "pdflatex $testFile"
performKileTest kile "run PDFLaTeX"
*/
	m_testList
	<< new FindProgramTest("PDFLaTeX", "pdflatex", false)
	<< new ProgramTest("PDFLaTeX", "pdflatex", m_tempDir->name(), "--interaction=nonstopmode",  "test.tex", "", false)
	<< new TestToolInKileTest("PDFLaTeX", m_ki, "PDFLaTeX", m_tempDir->name() + '/' + "test.tex", false);
	m_pdfLaTeXSyncTeXSupportTest = new SyncTeXSupportTest("PDFLaTeX", "pdflatex", m_tempDir->name(), "test");
	m_testList << m_pdfLaTeXSyncTeXSupportTest;
/*
echo "starting test: DVItoPS"
setTool DVItoPS
setKey mustpass ""
setKey executable dvips
setKey where `which dvips`
if [ -r $testFileBase.dvi ]; then performKileTest kile "run DVItoPS"; fi
*/
	m_testList << new FindProgramTest("DVItoPS", "dvips", false);
	TestToolInKileTest *dvipsKileTest = new TestToolInKileTest("DVItoPS", m_ki, "DVItoPS", m_tempDir->name() + '/' + "test.tex", false);
	dvipsKileTest->addDependency(latexProgramTest);
	m_testList << dvipsKileTest;
/*
echo "starting test: DVItoPDF"
setTool DVItoPDF
setKey mustpass ""
setKey executable dvipdfmx
setKey where `which dvipdfmx`
if [ -r $testFileBase.dvi ]; then performKileTest kile "run DVItoPDF"; fi
*/
	m_testList << new FindProgramTest("DVItoPDF", "dvipdfmx", false);
	TestToolInKileTest *dvipdfmxKileTest = new TestToolInKileTest("DVItoPDF", m_ki, "DVItoPDF", m_tempDir->name() + '/' + "test.tex", false);
	dvipdfmxKileTest->addDependency(latexProgramTest);
	m_testList << dvipdfmxKileTest;
/*
echo "starting test: PStoPDF"
setTool PStoPDF
setKey mustpass ""
setKey executable ps2pdf
setKey where `which ps2pdf`
if [ -r $testFileBase.ps ]; then performKileTest kile "run PStoPDF"; fi
$closeDoc
*/
	m_testList << new FindProgramTest("PStoPDF", "ps2pdf", false);
	TestToolInKileTest *ps2pdfKileTest = new TestToolInKileTest("PStoPDF", m_ki, "PStoPDF", m_tempDir->name() + '/' + "test.tex", false);
	ps2pdfKileTest->addDependency(dvipsKileTest);
	m_testList << ps2pdfKileTest;
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
	m_testList << new FindProgramTest("BibTeX", "bibtex", false);
	TestToolInKileTest *latexForBibTeX = new TestToolInKileTest("BibTeX", m_ki, "LaTeX", m_tempDir->name() + '/' + "test_bib.tex", false);
	latexForBibTeX->addDependency(latexProgramTest);
	latexForBibTeX->setSilent(true);
	m_testList << latexForBibTeX;
	ProgramTest *bibtexProgramTest = new ProgramTest("BibTeX", "bibtex", m_tempDir->name(), "test_bib",  "", "", false);
	bibtexProgramTest->addDependency(latexForBibTeX);
	m_testList << bibtexProgramTest;
	TestToolInKileTest *bibtexKileTest = new TestToolInKileTest("BibTeX", m_ki, "BibTeX", m_tempDir->name() + '/' + "test_bib.tex", false);
	bibtexKileTest->addDependency(latexProgramTest);
	m_testList << bibtexKileTest;

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
	m_testList << new FindProgramTest("MakeIndex", "makeindex", false);
	TestToolInKileTest *latexForMakeIndex = new TestToolInKileTest("MakeIndex", m_ki, "LaTeX", m_tempDir->name() + '/' + "test_index.tex", false);
	latexForMakeIndex->addDependency(latexProgramTest);
	latexForMakeIndex->setSilent(true);
	m_testList << latexForMakeIndex;
	ProgramTest *makeIndexProgramTest = new ProgramTest("MakeIndex", "makeindex", m_tempDir->name(), "test_index",  "", "", false);
	makeIndexProgramTest->addDependency(latexProgramTest);
	m_testList << makeIndexProgramTest;
	TestToolInKileTest *makeindexKileTest = new TestToolInKileTest("MakeIndex", m_ki, "MakeIndex", m_tempDir->name() + '/' + "test_index.tex", false);
	makeindexKileTest->addDependency(latexProgramTest);
	m_testList << makeindexKileTest;

/*
echo "starting test: Okular"
setTool Okular
setKey mustpass "where"
setKey executable okular
setKey version `getOkularVersion okular`
performTest okular "isTheOkularVersionRecentEnough"
setKey where `which okular`
*/
	m_testList << new FindProgramTest("Okular", "okular", false);
	m_okularVersionTest = new OkularVersionTest("Okular", false);
	m_testList << m_okularVersionTest;
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
	m_testList << new FindProgramTest("DVItoPNG", "dvipng", false);
/*
echo "starting test: Convert"
setTool Convert
setKey mustpass ""
setKey executable convert
setKey where `which convert`
*/
	m_testList << new FindProgramTest("Convert", "convert", false);
}

bool Tester::isSyncTeXSupportedForPDFLaTeX()
{
	return (m_pdfLaTeXSyncTeXSupportTest->status() == ConfigTest::Success);
}

bool Tester::isViewerModeSupportedInOkular()
{
	return m_okularVersionTest->isViewerModeSupported();
}

#include "configtester.moc"
