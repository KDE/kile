/***************************************************************************
                          kiletool.h  -  description
                             -------------------
    begin                : mon 3-11 20:40:00 CEST 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
    email                : wijnhout@science.uva.nl
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

#include <qobject.h>

#include <qstring.h>
#include <qstringlist.h>

class KConfig;
class KileInfo;
class KShellProcess;

namespace KileTool
{
	/**
 	* A class that defines a general tool (latex, dvips etc.) to be launched from
	* within Kile.
	*
	* @author Jeroen Wijnhout
	**/

	class Custom : public QObject
	{
		Q_OBJECT;

	public:
		Custom(const QString &name, int type, const QString &from, const QString &to, KileInfo *ki, KConfig *config);
		~Custom();

		/**
		 * The type of the tool:
		 * Shell - tool needs to be run in its own shell (most common)
		 * Konsole - run the tool in Kile's Konsole, useful if the process needs user interaction
		 * Run - start the tool with KRun
		 * Part - the tool is started using a KPart
		 **/
		enum { Shell, Konsole, Run, Part };
		
		/**
		 * KileTool can send several types of messages
		 * Error
		 * Warning
		 * Info
		 **/
		enum { Error, Warning, Info };
		
		/**
		 * Allows you to set the target file explicitly (filename only).
		 **/
		void setTarget(const QString & target);

		/**
		 * Sets the target directory relative to the source directory.
		 **/
		void setRelativeBaseDir(const QString & dir);

		/**
		 * Explicitly adds a prerequisite to the list (absolute path).
		 **/
		void addPrereq(const QString & prereq) { m_prereqs.append(prereq); }
		
		/**
		 * Clears the list of prerequisites.
		 **/
		void clearPrereqs() { m_prereqs.clear(); }

		/**
		 * Determines whether or not to build prerequisites if there are out
		 * of date.
		 **/
		void setBuildPrereqs(bool build) { m_buildPrereqs = build; }
		bool buildPrereqs() const { return m_buildPrereqs; }
		
		/**
		 * Add an option for this tool (for example a command line option)
		 **/
		void addOption(const QString & option) { m_options.append(option); }
		
		/**
		 * Clears the list of options.
		 **/
		void clearOptions() { m_options.clear(); }
		
		void setTool(const QString & tool) { m_tool = tool; }
		const QString& tool() const { return m_tool; }
		
	signals:
		void message(int type, const QString &);

	public:
		virtual void readConfig();
		virtual void writeConfig();
		
		/**
		 * Performs a self-check: should check if the tool can be launched, if not
		 * it should analyze why.
		 **/
		virtual bool selfCheck();

	protected:
		/**
		 * Checks if the prerequisites are in order.
		 * @returns true if everything is ok, false otherwise.
		 **/
		virtual bool checkPrereqs();

		/**
		 * Determines the target of the tool (i.e. a DVI for latex, PS for dvips) and
		 * checks if the target file can be written to the specified location.
		 **/
		virtual bool determineTarget();

		/**
		 * Launch the process/lib and connect signals for stdout and stderr to the
		 * specified slots.
		 **/
		virtual bool launch();
		
		/**
		 * Setup a KShellProcess object that is used to run the tool.
		 **/
		virtual bool setupProcess();
		
		/**
		 * Setup a KRun object that is used to run the tool.
		 **/
		virtual bool setupKRun();
		
		/**
		 * Setup the part to load.
		 **/
		virtual bool setupPart();

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
		virtual bool finish();

	private:
		KileInfo		*m_ki;
		KConfig			*m_config;

		int 			m_type;
		
		QString			m_name;
		QString			m_from;
		QString			m_to;
		QString			m_target, m_basedir, m_relativedir, m_targetdir;
		QStringList		m_prereqs;
		
		QString 		m_tool;
		QStringList		m_options;
		
		QString			m_message;
		
		bool			m_buildPrereqs;
		
		KShellProcess		*m_shellProcess;
		
	};

	/**
	 * A class that represents a compile tool (such as latex, pdflatex).
	 **/
	//class Compile : public Custom
	//{
	//};

	/**
	 * A class that represents a view tool (such as KDVI, gv, etc.).
	 **/
	//class View : public Custom
	//{
	//};

	/**
	 * A class that represents a conversion tool (such as dvips).
	 **/
	//class Convert : public Custom
	//{
	//};

	/**
	 * A class that represents a tool based on the (GNU) make program.
	 **/
	//class Make : public Custom
	//{
	//};

}

#endif
