/***************************************************************************
                          configchecker.cpp  -  description
                             -------------------
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

#include <kio/netaccess.h> 
#include <klocale.h>
#include <kprocess.h>
#include <kstddirs.h>
#include <ktempdir.h>
#include <ksimpleconfig.h>
#include <kglobal.h>
#include <kdebug.h>

#include "configtester.h"

ConfigTest::ConfigTest() :
	m_name(QString::null),
	m_arg(QString::null),
	m_altArg(QString::null),
	m_mustPass(false)
{
}

ConfigTest::ConfigTest(const QString &name, bool mustpass, const QString &arg, const QString &altarg /*= QString::null*/) :
	m_name(name),
	m_arg(arg),
	m_altArg(altarg),
	m_mustPass(mustpass)
{
}

int ConfigTest::status() const
{
	bool passed = false;
	if ( m_name == "binary" )
		passed = m_arg.endsWith(m_altArg);
	else if ( m_name == "version" )
		passed = true;
	else
		passed = (m_arg == "0");

	if ( passed ) return Success;
	else if ( m_mustPass ) return Critical;
	else return Failure;
}

QString ConfigTest::name() const
{
	return prettyName(m_name);
}

QString ConfigTest::resultText() const
{
	QString str = successMessage(m_name);
	if ( status() == Failure ) str = failureMessage(m_name);
	else if ( status() == Critical ) str = criticalMessage(m_name);

	if ( m_name == "binary" )
		return str + " (" + m_altArg + " => " + m_arg + ")";
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
	if ( s_prettyName.contains(test) ) return s_prettyName[test];
	else return test;
}

QString ConfigTest::successMessage(const QString &test)
{
	if ( s_msgSuccess.contains(test) ) return s_msgSuccess[test];
	else return i18n("Passed");
}

QString ConfigTest::failureMessage(const QString &test)
{
	if ( s_msgFailure.contains(test) ) return s_msgFailure[test];
	else return i18n("Failed");
}

QString ConfigTest::criticalMessage(const QString &test)
{
	if ( s_msgCritical.contains(test) ) return s_msgCritical[test];
	else return i18n("Critical failure");
}

Tester::Tester(QObject *parent, const char *name) : QObject(parent, name), m_process(0L)
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

	ConfigTest::addPrettyName("srcpkg", i18n("Source Specials Package"));
	ConfigTest::addSuccessMessage("srcpkg", i18n("Package is functional, include '\\usepackage[active]{srcltx}' in your preamble to enable the inverse and forward search capabilities."));
	ConfigTest::addFailureMessage("srcpkg", i18n("The srcltx.sty package is not in your TeX input-path. Download this file from http://ctan.org and save it in $HOME/.TeX."));
}


Tester::~Tester()
{
	if (m_tempDir) m_tempDir->unlink();
	delete m_tempDir;
	delete m_process;
}

void Tester::saveResults(const KURL & dest)
{
	KIO::NetAccess::file_copy(KURL::fromPathOrURL(m_resultsFile), dest, -1, true);
}

void Tester::runTests()
{
	QString srcdir = KGlobal::dirs()->findResourceDir("appdata","test/runTests.sh") + "test";
	kdDebug() << "Tester::runTests: srcdir = " << srcdir << endl;
	m_tempDir = new KTempDir();
	QString destdir = m_tempDir->name();
	kdDebug() << "Tester::runTests: destdir = " << destdir << endl;
	m_resultsFile = destdir + "results.rc";

	QString shellname = KGlobal::dirs()->findExe("sh");
	kdDebug() << "Tester::runTests: shellname = " << shellname << endl;
	m_process = new KShellProcess(shellname.local8Bit());
	*m_process << "cd " + KShellProcess::quote(destdir) + " && ";
	*m_process << "cp " + KShellProcess::quote(srcdir) +"/* " + KShellProcess::quote(destdir) + " && ";
	*m_process << "source runTests.sh " + KShellProcess::quote(m_resultsFile) + " " +  KShellProcess::quote(destdir);
	connect(m_process, SIGNAL(receivedStdout(KProcess *, char *, int)), this, SLOT(determineProgress(KProcess *, char *, int)));
	connect(m_process, SIGNAL(processExited(KProcess *)), this, SLOT(processTestResults(KProcess *)));
	if (m_process->start(KProcess::NotifyOnExit, KProcess::AllOutput)) emit(started());
}

void Tester::stop()
{
	if (m_process) m_process->kill();
}

void Tester::determineProgress(KProcess */*proc*/, char *buf, int len)
{
	static QString s = QString::null;

	s += QString::fromLocal8Bit(buf, len);
	if ( s.endsWith("\n") )
	{
		bool ok = false;
		int number = s.toInt(&ok);
		if (ok) emit(percentageDone(number));
		s = QString::null;
	}
}

void Tester::processTestResults (KProcess *proc)
{
	if (proc->normalExit())
	{
		emit(percentageDone(100));

		KSimpleConfig config(m_resultsFile, true);
		QStringList groups = config.groupList();
		QStringList::Iterator itend = groups.end();
		for ( QStringList::Iterator it = groups.begin(); it != itend; ++it )
			processTool(&config, *it);
			
		emit(finished(true));
	}
	else
	{
		emit(percentageDone(0));
		emit(finished(false));
	}
}

void Tester::processTool(KConfig *config, const QString &tool)
{
	config->setGroup(tool);

	QStringList criticaltests = QStringList::split(",", config->readEntry("mustpass", ""));

	//Did we find the executable?
	QValueList<ConfigTest> tests;
	tests << ConfigTest("binary", criticaltests.contains("where"), config->readEntry("where"), config->readEntry("executable"));
	if (config->hasKey("version") ) tests << ConfigTest("version", criticaltests.contains("version"), config->readEntry("version"));
	if (config->hasKey("basic") ) tests << ConfigTest("basic", criticaltests.contains("basic"), config->readEntry("basic"));
	if (config->hasKey("src") ) tests << ConfigTest("src", criticaltests.contains("src"), config->readEntry("src"));
	if (config->hasKey("srcpkg") ) tests << ConfigTest("srcpkg", criticaltests.contains("srcpkg"), config->readEntry("srcpkg"));
	if (config->hasKey("kile") ) tests << ConfigTest("kile", criticaltests.contains("kile"), config->readEntry("kile"));

	addResult(tool, tests);
}

void Tester::addResult(const QString &tool, const QValueList<ConfigTest> &tests)
{
	m_results [tool] = tests;
}

QStringList Tester::testedTools()
{
	return m_results.keys();
}

QValueList<ConfigTest> Tester::resultForTool(const QString & tool)
{
	return m_results[tool];
}

int Tester::statusForTool(const QString & tool)
{
	QValueList<ConfigTest> tests = m_results[tool];
	int status = ConfigTest::Success;
	for ( uint i = 0; i < tests.count(); i++)
	{
		if ( (tests[i].status() == ConfigTest::Failure) && (status == ConfigTest::Success)) status = ConfigTest::Failure;
		if (tests[i].status() == ConfigTest::Critical) status = ConfigTest::Critical;
	}
	return status;
}

#include "configtester.moc"
