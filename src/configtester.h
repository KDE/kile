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

#ifndef CONFIGTESTER_H
#define CONFIGTESTER_H

#include <QObject>
#include <QLinkedList>
#include <QMap>
#include <QProcess>

#include <KUrl>

class KJob;
class KTempDir;
class KProcess;

class KileInfo;

namespace KileDocument
{
	class TextInfo;
}

namespace KileTool
{
	class Base;
}

class ConfigTest : public QObject
{
	Q_OBJECT

	public:
		enum Status { Success = 2, Failure = 1, NotRun = 0 };

		ConfigTest(const QString& testGroup, const QString &name, bool isCritical);
		virtual ~ConfigTest();

		int status() const;

		virtual void call() = 0;

		QString resultText() const;
		QString name() const;
		QString testGroup() const;

		void addDependency(ConfigTest *test);
		bool allDependenciesSucceeded() const;

		bool isCritical() const;

		bool isSilent() const;
		void setSilent(bool b);

	Q_SIGNALS:
		void testComplete(ConfigTest *test);

	private:
		QString				m_testGroup, m_name;
		bool				m_isCritical, m_isSilent;
		QLinkedList<ConfigTest*>	m_dependencyTestList;

	protected:
		Status		m_status;
		QString		m_resultText;

		void setName(const QString& name);
};

class OkularVersionTest : public ConfigTest
{
	Q_OBJECT
	public:
		OkularVersionTest(const QString& testGroup, bool isCritical);
		~OkularVersionTest();

		virtual void call();
};

class FindProgramTest : public ConfigTest
{
	Q_OBJECT
	public:
		FindProgramTest(const QString& testGroup, const QString& programName, bool isCritical);
		~FindProgramTest();

		virtual void call();

	protected:
		QString m_programName;
};

class TestToolInKileTest : public ConfigTest
{
	Q_OBJECT
	public:
		TestToolInKileTest(const QString& testGroup, KileInfo *kileInfo, const QString& toolName, const QString& filePath, bool isCritical);
		~TestToolInKileTest();

		virtual void call();

	protected:
		void reportSuccess();
		void reportFailure();

	protected Q_SLOTS:
		void handleToolExit(KileTool::Base *tool, int status);

	protected:
		KileInfo *m_ki;
		QString m_toolName;
		QString m_filePath;
		KUrl m_documentUrl;
};

class ProgramTest : public ConfigTest
{
	Q_OBJECT
	public:
		ProgramTest(const QString& testGroup, const QString& programName, const QString& workingDir,
		                                                                  const QString& arg0,
		                                                                  const QString& arg1,
		                                                                  const QString& arg2 = "",
		                                                                  bool isCritical = false);
		~ProgramTest();

		virtual void call();

	protected:
		virtual void reportSuccess();
		virtual void reportFailure();

	protected Q_SLOTS:
		virtual void handleTestProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
		virtual void handleTestProcessError(QProcess::ProcessError error);

	protected:
		KProcess *m_testProcess;
		QString m_programName;
		QString m_workingDir;
		QString m_arg0, m_arg1, m_arg2;

		virtual void processFinishedSuccessfully();
};

class LaTeXSrcSpecialsSupportTest : public ProgramTest
{
	Q_OBJECT
	public:
		LaTeXSrcSpecialsSupportTest(const QString& testGroup, const QString& workingDir,
		                                                      const QString& fileBaseName);
		~LaTeXSrcSpecialsSupportTest();

	protected:
		QString m_fileBaseName;

		virtual void reportSuccess();
		virtual void reportFailure();
		virtual void processFinishedSuccessfully();
};

class Tester : public QObject
{
	Q_OBJECT

public:
	Tester(KileInfo *kileInfo, QObject *parent = 0);
	~Tester();

	QStringList testedTools();
	QList<ConfigTest*> resultForTool(const QString &);
	int statusForTool(const QString &testGroup, bool *isCritical = NULL);

public Q_SLOTS:
	void runTests();

Q_SIGNALS:
	void started();
	void percentageDone(int);
	void finished(bool);

private Q_SLOTS:
	void addResult(const QString &tool, ConfigTest* testResult);

	void startNextTest();

	void handleFileCopyResult(KJob* job);
	void handleTestComplete(ConfigTest *test);
private:
	KileInfo *m_ki;
	QMap<QString, QList<ConfigTest*> >	m_results;
	KTempDir				*m_tempDir;
	ConfigTest				*m_currentTest;
	QLinkedList<ConfigTest*> m_testList;
	QLinkedList<ConfigTest*>::iterator m_nextTestIterator;
	int					m_testsDone;

	QString m_runningTestGroup;
	KUrl m_runningToolTestUrl;
	bool m_runningTestCritical;

	void setupTests();
};

#endif
