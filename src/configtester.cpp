/***************************************************************************
    begin                : Fri Jun 4 2004
    copyright            : (C) 2004 by Jeroen Wijnout
    email                : Jeroen.Wijnhout@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "configtester.h"

#include <kio/job.h>
#include <klocale.h>
#include <kprocess.h>
#include <kshell.h>
#include <kstandarddirs.h>
#include <ktempdir.h>
#include <kconfig.h>
#include <kglobal.h>
#include "kiledebug.h"

#include "kiletoolmanager.h"
#include "kileinfo.h"
#include "kileconfig.h"
#include "kiletool.h"

ConfigTest::ConfigTest() :
	m_name(QString()),
	m_arg(QString()),
	m_altArg(QString()),
	m_mustPass(false)
{
}

ConfigTest::ConfigTest(const QString &name, bool mustpass, const QString &arg, const QString &altarg /*= QString()*/) :
	m_name(name),
	m_arg(arg),
	m_altArg(altarg),
	m_mustPass(mustpass)
{
}

int ConfigTest::status() const
{
	bool passed = false;
	if ( m_name == "binary" ){
		passed = m_arg.endsWith(m_altArg);
	#ifdef Q_WS_WIN
		passed = passed || m_arg.endsWith(m_altArg + ".exe");
	#endif
	}
	else if ( m_name == "version" )
		passed = true;
	else
		passed = (m_arg == "0");

	if ( passed )
		return Success;
	else if ( m_mustPass )
		return Critical;
	else
		return Failure;
}

QString ConfigTest::name() const
{
	return prettyName(m_name);
}

QString ConfigTest::resultText() const
{
	QString str = successMessage(m_name);
	if ( status() == Failure )
		str = failureMessage(m_name);
	else if ( status() == Critical )
		str = criticalMessage(m_name);

	if ( m_name == "binary" )
	{
		str += " (" + m_altArg + " => " + m_arg + ')';
		if ( status()==Failure && s_msgFailure.contains(m_altArg) )
			str += QString("<br>(%1)").arg( s_msgFailure[m_altArg] );
		return str;
	}
	else if ( m_name == "version" )
		return m_arg;
	else
		return str;
}

QMap<QString,QString> ConfigTest::s_prettyName;
QMap<QString,QString> ConfigTest::s_msgSuccess;
QMap<QString,QString> ConfigTest::s_msgFailure;
QMap<QString,QString> ConfigTest::s_msgCritical;

void ConfigTest::addPrettyName(const QString &test, const QString &prettyName) { s_prettyName [test] = prettyName;}
void ConfigTest::addSuccessMessage(const QString &test, const QString &msg) { s_msgSuccess [test]  = msg; }
void ConfigTest::addFailureMessage(const QString &test, const QString &msg) { s_msgFailure [test] = msg; }
void ConfigTest::addCriticalMessage(const QString &test, const QString &msg) { s_msgCritical [test] = msg; }

QString ConfigTest::prettyName(const QString &test)
{
	if ( s_prettyName.contains(test) )
		return s_prettyName[test];
	else
		return test;
}

QString ConfigTest::successMessage(const QString &test)
{
	if ( s_msgSuccess.contains(test) )
		return s_msgSuccess[test];
	else
		return i18n("Passed");
}

QString ConfigTest::failureMessage(const QString &test)
{
	if ( s_msgFailure.contains(test) )
		return s_msgFailure[test];
	else
		return i18n("Failed");
}

QString ConfigTest::criticalMessage(const QString &test)
{
	if ( s_msgCritical.contains(test) )
		return s_msgCritical[test];
	else
		return i18n("Critical failure");
}

Tester::Tester(QObject *parent) : QObject(parent), m_process(0L)
{
	ConfigTest::addPrettyName("binary", i18n("Binary"));
	ConfigTest::addCriticalMessage("binary", i18n("Could not find the binary for this essential tool."));

	ConfigTest::addPrettyName("basic", i18n("Simple Test"));
	ConfigTest::addCriticalMessage("basic", i18n("This essential tool does not work at all, check your installation."));

	ConfigTest::addPrettyName("version", i18n("Version"));

	ConfigTest::addPrettyName("kile", i18n("Running in Kile"));
	QString str = i18n("Kile is not configured correctly. Go to Settings->Configure Kile->Tools and either fix the problem or change to the default settings.");
	ConfigTest::addCriticalMessage("kile", str);
	ConfigTest::addFailureMessage("kile", str);

	ConfigTest::addPrettyName("src", i18n("Source Specials Switch"));
	ConfigTest::addSuccessMessage("src", i18n("Supported, use the 'Modern' configuration for (La)TeX and PDF(La)TeX to auto-enable inverse and forward search capabilities."));
	ConfigTest::addFailureMessage("src", i18n("Not supported, use the srcltx package to enable the inverse and forward search capabilities."));

	// add some special messages, when programs are not installed,
	// which are not needed, but probably useful for the work with kile
	ConfigTest::addFailureMessage("dvipng", i18n("You cannot use the png preview for mathgroups in the bottom bar."));
	ConfigTest::addFailureMessage("convert", i18n("You cannot use the png previews with conversions 'dvi->ps->png' and 'pdf->png'."));
#ifdef Q_WS_WIN
	ConfigTest::addFailureMessage("acrord32", i18n("You cannot open pdf documents with Adobe Reader because acroread could not be found in your path.  <br>If Adobe Reader is your default pdf viewer, try setting ViewPDF to System Default.  Alternatively, you could use Okular."));
#else
	ConfigTest::addFailureMessage("acroread", i18n("You cannot open pdf documents with Adobe Reader, but you could use Okular."));
#endif

	ConfigTest::addPrettyName("okular", i18n("ForwardDVI"));
	ConfigTest::addSuccessMessage("okular", i18n("Supported."));
	ConfigTest::addFailureMessage("okular", i18n("The Okular version is too old for ForwardDVI, you must use at least version 0.8.6"));

}


Tester::~Tester()
{
	if (m_tempDir) m_tempDir->unlink();
	delete m_tempDir;
	delete m_process;
}

void Tester::saveResults(const KUrl & dest)
{
	KIO::file_copy(KUrl(m_resultsFile), dest, -1, KIO::Overwrite | KIO::HideProgressInfo);
}

void Tester::runTests()
{
#ifdef Q_WS_WIN
	QString srcdir = KGlobal::dirs()->findResourceDir("appdata","test/runTests.bat") + "test";
#else
	QString srcdir = KGlobal::dirs()->findResourceDir("appdata","test/runTests.sh") + "test";
#endif
	QString command;
	KILE_DEBUG() << "Tester::runTests: srcdir = " << srcdir << endl;
	m_tempDir = new KTempDir();
	QString destdir = m_tempDir->name();
	KILE_DEBUG() << "Tester::runTests: destdir = " << destdir << endl;
	m_resultsFile = destdir + "results.rc";

	m_process = new KProcess();

	if (! KileConfig::teXPaths().isEmpty())
	{
		m_process->setEnv("TEXINPUTS", KileInfo::expandEnvironmentVars( KileConfig::teXPaths() + ":$TEXINPUTS"));
	}
#ifdef Q_WS_WIN
	//Most things don't care, but the command-line copy tool won't work with '/'
	//Also: calling quoteArg on something ending with '\' behaves oddly
	destdir = KShell::quoteArg(destdir);
	srcdir = KShell::quoteArg(srcdir + "\\*");
	srcdir.replace('/','\\');
	destdir.replace('/','\\');

	//Copy files to working directory
	QStringList copyArgs;
	copyArgs << "/c" << "copy" << srcdir << destdir;
	int res = KProcess::execute("cmd", copyArgs);

	//Execute the test script
	command = "runTests.bat " + KShell::quoteArg(m_resultsFile) + ' ' +  destdir;
	m_process->setWorkingDirectory(destdir);
#else
	command = "cd " + KShell::quoteArg(destdir) + " && ";
	command += "cp " + KShell::quoteArg(srcdir) +"/* " + KShell::quoteArg(destdir) + " && ";
	command += "bash runTests.sh " + KShell::quoteArg(m_resultsFile) + " " +  KShell::quoteArg(destdir);
#endif //def Q_WS_WIN

	m_process->setShellCommand(command);

	connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(determineProgress()));
	connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(processTestResults(int,QProcess::ExitStatus)));
	m_process->setOutputChannelMode(KProcess::MergedChannels);
	m_process->setReadChannel(QProcess::StandardOutput);
	m_process->start();
}

void Tester::stop()
{
	if (m_process)
		m_process->kill();
}

void Tester::determineProgress()
{
	static QString s = QString();

	s += QString::fromLocal8Bit(m_process->readAllStandardOutput());

	if ( s.endsWith("\n") )
	{
		bool ok = false;
		int number = s.toInt(&ok);
		if (ok)
			emit(percentageDone(number));
		s = QString();
	}
}

void Tester::processTestResults(int /* exitCode */, QProcess::ExitStatus exitStatus)
{
	if(exitStatus == QProcess::NormalExit) {
		emit(percentageDone(100));

		KConfig config(m_resultsFile, KConfig::SimpleConfig);
		QStringList groups = config.groupList();
		QStringList::Iterator itend = groups.end();
		for(QStringList::Iterator it = groups.begin(); it != itend; ++it) {
			processTool(&config, *it);
		}

		emit(finished(true));
	}
	else {
		emit(percentageDone(0));
		emit(finished(false));
	}
}

void Tester::processTool(KConfig *config, const QString &tool)
{
	KConfigGroup group = config->group(tool);

	QStringList criticaltests = (group.readEntry("mustpass", "")).split(',');

	//Did we find the executable?
	QList<ConfigTest> tests;
	tests << ConfigTest("binary", criticaltests.contains("where"), group.readEntry("where"), group.readEntry("executable"));
	if (group.hasKey("version") )
		tests << ConfigTest("version", criticaltests.contains("version"), group.readEntry("version"));
	if (group.hasKey("basic") )
		tests << ConfigTest("basic", criticaltests.contains("basic"), group.readEntry("basic"));
	if (group.hasKey("src") )
		tests << ConfigTest("src", criticaltests.contains("src"), group.readEntry("src"));
	if (group.hasKey("kile") )
		tests << ConfigTest("kile", criticaltests.contains("kile"), group.readEntry("kile"));
	if (group.hasKey("okular") )
		tests << ConfigTest("okular", criticaltests.contains("okular"), group.readEntry("okular"));

	addResult(tool, tests);
}

void Tester::addResult(const QString &tool, const QList<ConfigTest> &tests)
{
	m_results [tool] = tests;
}

QStringList Tester::testedTools()
{
	return m_results.keys();
}

QList<ConfigTest> Tester::resultForTool(const QString & tool)
{
	return m_results[tool];
}

int Tester::statusForTool(const QString & tool)
{
	QList<ConfigTest> tests = m_results[tool];
	int status = ConfigTest::Success;
	for ( int i = 0; i < tests.count(); ++i)
	{
		if ( (tests[i].status() == ConfigTest::Failure) && (status == ConfigTest::Success))
			status = ConfigTest::Failure;
		if (tests[i].status() == ConfigTest::Critical)
			status = ConfigTest::Critical;
	}
	return status;
}

#include "configtester.moc"
