/***************************************************************************
                          kiletool.h  -  description
                             -------------------
    begin                : mon 3-11 20:40:00 CEST 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
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

#ifndef KILETOOL_H
#define KILETOOL_H

#include "kilelauncher.h"

#include <qobject.h>
#include <qmap.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qdict.h>
#include <qstringlist.h>

class KConfig;
class KileInfo;
class KShellProcess;

namespace KileTool
{
	typedef QMap<QString,QString> Config;

	class Manager;
	
	/**
 	* A class that defines a general tool (latex, dvips etc.) to be launched from
	* within Kile.
	*
	* @author Jeroen Wijnhout
	**/

	class Base : public QObject
	{
		Q_OBJECT

	public:
		Base(const QString &name, Manager *manager);
		~Base();

		/**
		 * Sets the KileInfo object, this is already taken care of by the Manager.
		 **/
		void setInfo(KileInfo *ki) { m_ki = ki; }
		
		/**
		 * Sets the KConfig object, this is already taken care of by the Manager.
		 **/
		void setConfig(KConfig *config) { m_config = config; }
		
		/**
		 * @returns the Manager object for this tool.
		 **/
		Manager* manager() const { return m_manager; }
		
		/**
		 * @returns a short descriptive name for this tool.
		 **/
		const QString& name() const { return m_name; }
		
		/**
		 * Allows you to set the source file explicitly (absolute path).
		 **/
		void setSource(const QString & source);
		
		/**
		 * @returns the source file that is used to run the tool on.
		 **/
		const QString source(bool absolute = true) const;
		
		const QString S() const { return m_S; }
		const QString baseDir() const { return m_basedir; }
		const QString relativeDir() const { return m_relativedir; }
		const QString targetDir() const { return m_targetdir; }
		const QString from() const { return m_from; }
		const QString to() const { return m_to; }
		const QString target() const { return m_target; }
		const QString options() const { return m_options; }

		void setOptions(const QString &opt) { m_options = opt; }

		virtual bool isViewer() { return false; }

		/**
		 * Allows you to set the target file explicitly (filename only).
		 **/
		void setTarget(const QString & target) { m_target = target; }

		/**
		 * Sets the target directory relative to the source directory.
		 **/
		void setRelativeBaseDir(const QString & dir) { m_relativedir = dir; }

		/**
		 * Installs a launcher object that will be responsible for actually starting the tool. The
		 * tool can be a command-line tool or a kpart, the KileTool class doesn't need to know
		 * about the specifics of the launcher.
		 **/
		void installLauncher(Launcher *lr );
		
		/**
		 * Installs a launcher as indicated by the tool type. This creates a launcher object.
		 **/
		bool installLauncher();

		/**
		 * @returns a pointer to the launcher object, returns 0 if no launcher is installed.
		 **/
		Launcher *launcher() { return m_launcher; }

		/**
		 * @returns the working dir for this tool.
		 **/
		const QString &workingDir() const { return m_basedir; }

		/**
		 * @returns the dictionary that translates the following keys
		 * %dir_base : the directory of the root file
		 * %dir_target : same as %dir_base, except when the relativeDir has been set explicitly, then %dir_target= %dir_base/relativedir
		 * %source : the source file (no path)
		 * %S : the source filename without an extension (no path)
		 **/
		QDict<QString>* paramDict() { return &m_dictParams; }

		bool addDict(const QString & key, const QString & value);

		void translate(QString &str);

		void setFlags(uint flags) { m_flags = flags; }
		uint flags() { return m_flags; }

		void setMsg(long n, QString msg);
		QString msg(long n) const { return m_messages[n]; }

	protected:
		bool needsUpdate(const QString &target, const QString &source);

	public slots:
		void sendMessage(int, const QString &);
		virtual void filterOutput(const QString &);

	signals:
		void message(int, const QString &, const QString &);
		void output(const QString &);

		void start(Base*);
		void done(Base*, int);

		void requestSaveAll();

	public:
		void setEntryMap(Config map) { m_entryMap = map; }
		const QString readEntry(const QString & key) { return m_entryMap[key]; }

	public:
		void prepareToRun();

	protected:
		/**
		 * Checks if the prerequisites are in order.
		 * @returns true if everything is ok, false otherwise.
		 **/
		virtual bool checkPrereqs();

		/**
		 * Determines on which file to run the tool.
		 **/
		virtual bool determineSource();
		
		/**
		 * Determines the target of the tool (i.e. a DVI for latex, PS for dvips) and
		 * checks if the target file can be written to the specified location.
		 **/
		virtual bool determineTarget();

		/**
		 * Check if the target dir and file have the correct permissions (according to the flags set).
		 **/
		virtual bool checkTarget();
		
		/**
		 * Configures the tool object.
		 **/
		 virtual bool configure();

	public slots:
		/**
		 * Starts the tool. First it performs basic checks (checkPrereqs()),
		 * if all is well it launches the tool (launch()). After the process has
		 * exited it calls finish().
		 * @return the exit code of the tool (if available)
		 **/
		virtual int run();

		/**
		 * Terminates the running process.
		 **/
		virtual void stop();

		/**
		 * Clean up after the process/lib has finished.
		 **/
		virtual bool finish(int);

	private:
		Manager			*m_manager;
		KileInfo			*m_ki;
		KConfig			*m_config;

		QString			m_name, m_from, m_to;
		QString			m_target, m_basedir, m_relativedir, m_targetdir, m_source, m_S, m_options;

		QString			m_message;

		bool				m_buildPrereqs;

	protected:
		Launcher			*m_launcher;

	private:
		QDict<QString>		m_dictParams;
		Config				m_entryMap;

		uint		    	m_flags;
		int					m_nPreparationResult;
		bool				m_bPrepared;

		//messages
		QMap<long,QString>	m_messages;
	};

	/**
	 * A class that represents a compile tool (such as latex, pdflatex).
	 **/
	class Compile : public Base
	{
	public:
		Compile(const QString &name, Manager * manager);
		~Compile();
		
	protected:
		bool determineSource();
	};

	/**
	 * A class that represents a view tool (such as KDVI, gv, etc.).
	 **/
	class View : public Base
	{
	public:
		View(const QString &name, Manager * manager);
		~View();

		bool isViewer() { return true; }
	};

	/**
	 * A class that represents a conversion tool (such as dvips).
	 **/
	class Convert : public Base
	{
	public:
		Convert(const QString &name, Manager * manager);
		~Convert();
		
		bool determineSource();
	};

	/**
	 * A class that represents a tool based on the (GNU) make program.
	 **/
	//class Make : public Base
	//{
	//};
	
	class Sequence : public Base
	{
		Q_OBJECT
		
	public:
		Sequence(const QString &name, Manager * manager);

	public slots:
		int run();
	};
}

#endif
