/****************************************************************************************
  Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
            (C) 2011-2013 by Michel Ludwig (michel.ludwig@kdemail.net)
 ****************************************************************************************/

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

#include <QHash>
#include <QLinkedList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>

#include <KLocalizedString>

#include "kilelauncher.h"
#include "outputinfo.h"

class KConfig;
class KileInfo;
class KileProject;

namespace KileTool
{
	typedef QMap<QString, QString> Config;

	class Factory;
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
		friend class KileTool::Factory;

	// only the factory can create tools
	protected:
		Base(const QString &name, Manager *manager, bool prepare = true);
	public:
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
		 * Allows you to set the source file and working directory explicitly (absolute path).
		 **/
		virtual void setSource(const QString& source, const QString& workingDir = "");

		/**
		 * @returns the source file that is used to run the tool on.
		 **/
		QString source(bool absolute = true) const;

		QString S() const { return m_S; }
		QString baseDir() const { return m_basedir; }
		QString relativeDir() const { return m_relativedir; }
		QString targetDir() const { return m_targetdir; }
		inline QString from() const { return readEntry("from"); }
		inline QString to() const { return readEntry("to"); }
		QString target() const { return m_target; }
		QString options() const { return m_options; }

		QString toolConfig() const { return m_toolConfig; }

		void setOptions(const QString& opt) { m_options = opt; }

		virtual bool isViewer() { return false; }

		void setQuickie() { m_quickie = true; }
		bool isQuickie() { return m_quickie; }

		/**
		 * Returns true iff all documents must be saved before the tool can be launched
		 **/
		virtual bool requestSaveAll();

		void setPartOfLivePreview() { m_isPartOfLivePreview = true; }
		bool isPartOfLivePreview() const { return m_isPartOfLivePreview; }

		void setTeXInputPaths(const QString& s);
		QString teXInputPaths() const;
		void setBibInputPaths(const QString& s);
		QString bibInputPaths() const;
		void setBstInputPaths(const QString& s);
		QString bstInputPaths() const;
		void copyPaths(Base* tool);

		/**
		 * Allows you to set the target file explicitly (filename only).
		 **/
		virtual void setTarget(const QString & target);
		virtual void setTargetDir(const QString & target);
		virtual void setTargetPath(const QString & target);

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
		const QString &workingDir() const { return m_workingDir; }

		void setWorkingDir(const QString& s) { m_workingDir = s; }

		/**
		 * @returns the dictionary that translates the following keys
		Example docu:
		Consider a file which is called myBestBook.tex which resides in /home/thomas/latex and you compile it with pdflatex to myBestBook.pdf.

		The variables have the following meanings:
		%source ->  filename with suffix but without path <-> myBestBook.tex
		%S ->  filename without suffix but without path <-> myBestBook
		%dir_base  -> path of the source file without filename  <-> /home/thomas/latex
		%dir_target -> path of the target file without filename, same as %dir_base if no relative path has been set <-> /home/thomas/latex
		%target -> target filename without path <-> without filename

		And these are special variables:
		%res <-> resolution of the quickpreview action set in configure kile->tools->preview

		%AFL <-> List of all files in a project marked for archiving. You can set the archive flag in the "Files and projects" sidebar using the context menu.

		%absolute_target -> Used in conjunction with Okular to inform it about the cursor position for
		                    the ForwardDVI/PDF feature

		%sourceFileName <-> Source file name (for synchronizing the cursor location with the viewer)
		%sourceLine <-> Line in the source file on which the cursor is located (for synchronizing the cursor location with the viewer)
		*/
		QHash<QString,QString>& paramDict() { return m_dictParams; }

		bool addDict(const QString& key, const QString& value);

		void translate(QString &str, bool quoteForShell = false);

		void setFlags(uint flags) { m_flags = flags; }
		uint flags() { return m_flags; }
		void removeFlag(uint flag);

		void setMsg(long n, const KLocalizedString& msg);
		KLocalizedString msg(long n) const { return m_messages[n]; }

		virtual void setupAsChildTool(KileTool::Base *child);

	public Q_SLOTS:
		void sendMessage(int, const QString &);
		virtual void filterOutput(const QString &);

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

		void installLaTeXOutputParserResult(int nErrors, int nWarnings, int nBadBoxes, const LatexOutputInfoArray& outputList,
		                                                                               const QString& logFile);

	Q_SIGNALS:
		void message(int, const QString &, const QString &);
		void output(const QString &);

		void start(KileTool::Base*);
		void done(KileTool::Base*, int, bool childToolSpawned);
		void failedToRun(KileTool::Base*, int);

		void aboutToBeDestroyed(KileTool::Base*);

	public:
		void setEntryMap(Config map) { m_entryMap = map; }
		void setEntry(const QString& key, const QString& value);
		const QString readEntry(const QString& key) const { return m_entryMap[key]; }

		virtual void prepareToRun();
		bool isPrepared() { return m_bPrepared; }
		bool needsToBePrepared() { return m_bPrepareToRun; }

	protected:
		Launcher	*m_launcher;
		bool		m_quickie;
		bool		m_isPartOfLivePreview;

		bool needsUpdate(const QString &target, const QString &source);

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

		virtual bool checkSource();

		void runChildNext(Base *tool, bool block = false);

		void setToolConfig(const QString& config) { m_toolConfig = config; }

		virtual void latexOutputParserResultInstalled();

	protected:
		Manager			*m_manager;
		KileInfo		*m_ki;
		KConfig			*m_config;

	private:
		QString			m_name;
		QString			m_target, m_basedir, m_relativedir, m_targetdir, m_source, m_S, m_workingDir;
		QString			m_options;
		QString			m_resolution;

		QString			m_message;

		bool			m_buildPrereqs;

		QHash<QString,QString> m_dictParams;
		Config			m_entryMap;

		uint		    	m_flags;
		int			m_nPreparationResult;
		bool			m_bPrepared;
		bool			m_bPrepareToRun;
		QString			m_toolConfig;

		QString m_texInputs, m_bibInputs, m_bstInputs;

		//messages
		QMap<long, KLocalizedString>	m_messages;

	protected:
		int			m_nErrors, m_nWarnings, m_nBadBoxes;
		LatexOutputInfoArray	m_latexOutputInfoList;
		QString			m_logFile;
		bool			m_childToolSpawned;
		int			m_toolResult;
	};

	/**
	 * A class that represents a compile tool (such as latex, pdflatex).
	 **/
	class Compile : public Base
	{
		friend class KileTool::Factory;
	protected:
		Compile(const QString &name, Manager * manager, bool prepare = true);
	public:
		~Compile();

	protected:
		bool checkSource();
	};

	/**
	 * A class that represents a view tool (such as KDVI, gv, etc.).
	 **/
	class View : public Base
	{
		friend class KileTool::Factory;
	protected:
		View(const QString &name, Manager * manager, bool prepare = true);
	public:
		~View();

		bool isViewer() { return true; }
	};

	/**
	 * A class that represents a conversion tool (such as dvips).
	 **/
	class Convert : public Base
	{
		friend class KileTool::Factory;
	protected:
		Convert(const QString &name, Manager * manager, bool prepare = true);
	public:
		~Convert();

		bool determineSource();
	};

	/**
	 * A class that represents a tool like tar, from multiple files to one file
	 **/
	class Archive: public Base
	{
		Q_OBJECT
		friend class KileTool::Factory;

	protected:
		Archive(const QString &name, Manager * manager, bool prepare = true);
	public:
		~Archive();
		bool checkPrereqs();
		void setSource(const QString & source, const QString& workingDir = "");
	private:
		KileProject *m_project;
		QString m_fileList;
	};

	class Sequence : public Base
	{
		Q_OBJECT
		friend class KileTool::Factory;

		bool requestSaveAll();

		void setupSequenceTools();

		LaTeXOutputHandler* latexOutputHandler();
		void setLaTeXOutputHandler(LaTeXOutputHandler *h);

	public Q_SLOTS:
		int run();

	protected:
		Sequence(const QString &name, Manager *manager, bool prepare = true);
		~Sequence();

		// will also determine the current LaTeXOutputHandler
		bool determineSource();

		QLinkedList<Base*> m_tools;
		QString m_unknownToolSpec;
		LaTeXOutputHandler *m_latexOutputHandler;
	};
}

#endif
