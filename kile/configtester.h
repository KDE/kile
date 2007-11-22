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
 
#ifndef CONFIGCHECKER_H
#define CONFIGCHECKER_H

#include <qobject.h>
//Added by qt3to4:
#include <Q3ValueList>

class KTempDir;
class KConfig;
class K3Process;
class KUrl;

/**
@author Jeroen Wijnhout
*/

class ConfigTest
{
	public:
		enum Status { Success = 3, Failure = 2, Critical = 1 };

		ConfigTest();
		ConfigTest(const QString &name, bool mustpass, const QString &arg, const QString &altarg = QString::null);

		int status() const;

		QString resultText() const;
		QString name() const;

	private:
		QString	m_name, m_arg, m_altArg;
		bool 		m_mustPass;

	public:
		static void addPrettyName(const QString &test, const QString &prettyName);
		static void addSuccessMessage(const QString &test, const QString &msg);
		static void addFailureMessage(const QString &test, const QString &msg);
		static void addCriticalMessage(const QString &test, const QString &msg);

		static QString prettyName(const QString &test);
		static QString successMessage(const QString &test);
		static QString failureMessage(const QString &test);
		static QString criticalMessage(const QString &test);

	private:
		static QMap<QString,QString> s_prettyName, s_msgSuccess, s_msgFailure, s_msgCritical;
};

class Tester : public QObject
{
	Q_OBJECT

public:
	Tester(QObject *parent = 0, const char *name = 0);
	~Tester();

	QStringList testedTools();
	Q3ValueList<ConfigTest> resultForTool(const QString &);
	int statusForTool(const QString &);

public slots:
	void runTests();
	void saveResults(const KUrl &);
	void stop();

signals:
	void started();
	void percentageDone(int);
	void finished(bool);

private slots:
	void determineProgress(K3Process *, char *, int);
	void processTestResults (K3Process *);
	void processTool(KConfig *, const QString &);

	void addResult(const QString &tool, const Q3ValueList<ConfigTest> &tests);

private:
	QMap<QString,Q3ValueList<ConfigTest> >	m_results;
	QString								m_resultsFile;
	KTempDir							*m_tempDir;
	K3Process								*m_process;
};

#endif
