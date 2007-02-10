/***************************************************************************
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

#include <qdir.h>
#include <qfileinfo.h>
#include <qmetaobject.h>
#include <qregexp.h>
#include <qtimer.h>

#include <klocale.h>
#include <kconfig.h>
#include <kurl.h>

#include "kileconfig.h"
#include "kileuntitled.h"
#include "kiletool_enums.h"
#include "kiletool.h"
#include "kilestdtools.h" //for the factory
#include "kiletoolmanager.h"
#include "kiledocmanager.h"
#include "kileinfo.h"
#include "kiledocumentinfo.h"
#include "kileproject.h"

namespace KileTool
{
	Base::Base(const QString &name, Manager * manager, bool prepare /* = true */) :
		m_manager(manager),
		m_name(name),
		m_from(QString::null),
		m_to(QString::null),
		m_target(QString::null),
		m_basedir(QString::null),
		m_relativedir(QString::null),
		m_targetdir(QString::null),
		m_source(QString::null),
		m_S(QString::null),
		m_options(QString::null),
		m_resolution(QString::null),
		m_launcher(0L),
		m_quickie(false),
		m_bPrepareToRun(prepare)
	{
		m_manager->initTool(this);
		
		m_flags = NeedTargetDirExec | NeedTargetDirWrite | NeedActiveDoc | NeedMasterDoc | NoUntitledDoc | NeedSourceExists | NeedSourceRead;

		setMsg(NeedTargetDirExec, i18n("Could not change to the folder %1."));
		setMsg(NeedTargetDirWrite, i18n("The folder %1 is not writable, therefore %2 will not be able to save its results."));
		setMsg(NeedTargetExists,  i18n("The file %1/%2 does not exist. If you're surprised, check the file permissions."));
		setMsg(NeedTargetRead, i18n("The file %1/%2 is not readable. If you're surprised, check the file permissions."));
		setMsg(NeedActiveDoc, i18n("Could not determine on which file to run %1, because there is no active document."));
		setMsg(NeedMasterDoc, i18n("Could not determine the master file for this document."));
		setMsg(NoUntitledDoc, i18n("Please save the untitled document first."));
		setMsg(NeedSourceExists, i18n("Sorry, the file %1 does not exist."));
		setMsg(NeedSourceRead, i18n("Sorry, the file %1 is not readable."));

		m_bPrepared = false;
	}

	Base::~Base()
	{
		kdDebug() << "DELETING TOOL: " << name() << endl;
		delete m_launcher;
	}

	const QString Base::source(bool absolute /* = true */) const
	{
		if (m_source.isNull())
			return QString::null;

		QString src = m_source;
		if (absolute)
			src = m_basedir + '/' + src;
			
		return src;
	}
	
	void Base::setMsg(long n, const QString & msg)
	{
		m_messages[n] = msg;
	}

	void Base::translate(QString &str)
	{
		QDictIterator<QString> it(*paramDict());
		for( it.toFirst() ; it.current(); ++it )
		{
// 			kdDebug() << "translate " << str << " /// key=" << it.currentKey() << " value=" << *(it.current()) << endl;
			str.replace(it.currentKey(), *( it.current() ) );
		}
	}

	void Base::prepareToRun(const QString &cfg)
	{
		kdDebug() << "==Base::prepareToRun()=======" << endl;
		
		m_bPrepared = true;		
		m_nPreparationResult = Running;

		//configure me
		if (!configure(cfg))
		{
			m_nPreparationResult = ConfigureFailed;
			m_bPrepared = false;
			return;
		}
		
		//install a launcher
		if (!installLauncher())
		{
			m_nPreparationResult = NoLauncherInstalled;
			m_bPrepared = false;
			return;
		}
		
		if (!determineSource())
		{
			m_nPreparationResult = NoValidSource;
			m_bPrepared = false;
			return;
		}
			
		if (!determineTarget())
		{
			m_nPreparationResult = NoValidTarget;
			m_bPrepared = false;
			return;
		}
			
		if ( m_launcher == 0 )
		{
			m_nPreparationResult = NoLauncherInstalled;
			m_bPrepared = false;
			return;
		}

		m_launcher->setWorkingDirectory(workingDir());

		//fill in the dictionary
		addDict("%options", m_options);

		m_resolution = KileConfig::dvipngResolution() ;
		addDict("%res",m_resolution);
	}

	int Base::run()
	{
		kdDebug() << "==KileTool::Base::run()=================" << endl;
	
		if ( m_nPreparationResult != 0 )
			return m_nPreparationResult;
		
		if (!checkSource())
			return NoValidSource;
		
		if (!checkTarget())
			return TargetHasWrongPermissions;
		
		if (!checkPrereqs())
			return NoValidPrereqs;
		
		//everything ok so far
		emit(requestSaveAll(false, true));
		emit(start(this));
		
		if (!m_launcher->launch())
		{
			kdDebug() << "\tlaunching failed" << endl;
			if (!m_launcher->selfCheck())
				return SelfCheckFailed;
			else
				return CouldNotLaunch;
		}

		kdDebug() << "\trunning..." << endl;

		return Running;

	}

	bool Base::determineSource()
	{
		QString src = source();

		//the basedir is determined from the current compile target
		//determined by getCompileName()
		if (src.isNull()) src = m_ki->getCompileName();

		setSource(src);

		return true;
	}
	
	bool Base::checkSource()
	{
		//FIXME deal with tools that do not need a source or target (yes they exist)
		//Is there an active document? Only check if the source file is not explicitly set.
		if ( (m_source.isNull()) && (m_manager->info()->activeTextDocument() == 0L)  )
		{ 
			sendMessage(Error, msg(NeedActiveDoc).arg(name()));
			return false;
		}

		if ( (m_source.isNull()) && (m_manager->info()->activeTextDocument() != 0L) )
		{
			//couldn't find a source file, huh?
			//we know there is an active document, the only reason is could have failed is because
			//we couldn't find a LaTeX root document
			sendMessage(Error, msg(NeedMasterDoc));
			return false;
		}

		if ( KileUntitled::isUntitled(m_source) &&  (flags() & NoUntitledDoc) )
		{
			sendMessage(Error, msg(NoUntitledDoc));
			emit(requestSaveAll());
			return false;
		}
		
		QFileInfo fi(source());
		if ( (flags() & NeedSourceExists) && !fi.exists() )
		{
			sendMessage(Error, msg(NeedSourceExists).arg(fi.absFilePath()));
			return false;
		}
		
		if ( (flags() & NeedSourceRead) && !fi.isReadable() )
		{
			sendMessage(Error, msg(NeedSourceRead).arg(fi.absFilePath()));
			return false;
		}

		return true;		
	}

	void Base::setSource(const QString &source)
	{
		m_from = readEntry("from");

		QFileInfo info(source);
		
		if (!m_from.isNull())
		{
			QString src = source;
			if ( (m_from.length() > 0) && (info.extension(false).length() > 0) )
				src.replace(QRegExp(info.extension(false) + '$'), m_from);
 			info.setFile(src);
		}

		m_basedir = info.dirPath(true);
		m_source = info.fileName();
		m_S = info.baseName(true);
		
		addDict("%dir_base", m_basedir);
		addDict("%source", m_source);
		addDict("%S",m_S);
		
		kdDebug() << "===KileTool::Base::setSource()==============" << endl;
		kdDebug() << "using " << source << endl;
		kdDebug() << "source="<<m_source<<endl;
		kdDebug() << "S=" << m_S << endl;
		kdDebug() << "basedir=" << m_basedir << endl;
	}
	
	bool Base::determineTarget()
	{
		QFileInfo info(source());

		m_to = readEntry("to");
		
		//if the target is not set previously, use the source filename
		if (m_target.isNull())
		{
			//test for explicit override
			if ( !readEntry("target").isEmpty() )
			{
				kdDebug() << "USING target SETTING" << endl;
				m_target = readEntry("target");
			}
			else if ( to().length() > 0)
				m_target = S() + '.' + to();
			else
				m_target = source(false);
		}

		if ( m_relativedir.isNull() && (!readEntry("relDir").isEmpty()) )
		{
			m_relativedir = readEntry("relDir");
		}

		KURL url = KURL::fromPathOrURL(m_basedir);
		url.addPath(m_relativedir);
		url.cleanPath();
		m_targetdir = url.path();
		
		setTarget(m_target);
		setTargetDir(m_targetdir);		
		
		kdDebug() << "==KileTool::Base::determineTarget()=========" << endl;
		kdDebug() << "\tm_targetdir=" << m_targetdir << endl;
		kdDebug() << "\tm_target=" << m_target << endl;
		
		return true;
	}

	bool Base::checkTarget()
	{
		//check if the target directory is accessible
		QFileInfo info(m_targetdir);
		
		if ( (flags() & NeedTargetDirExec ) && (! info.isExecutable()) )
		{
			sendMessage(Error, msg(NeedTargetDirExec).arg(m_targetdir));
			return false;
		}

		if ((flags() & NeedTargetDirWrite) && (! info.isWritable()) )
		{
			sendMessage(Error, msg(NeedTargetDirWrite).arg(m_targetdir).arg(m_name));
			return false;
		}

		info.setFile(m_targetdir + '/' + m_target);

		if ( (flags() & NeedTargetExists) && ( ! info.exists() ))
		{
			sendMessage(Error, msg(NeedTargetExists).arg(m_targetdir).arg(m_target));
			return false;
		}

		if ( (flags() & NeedTargetRead) && ( ! info.isReadable() ))
		{
			sendMessage(Error, msg(NeedTargetRead).arg(m_targetdir).arg(m_target));
			return false;
		}

		return true;
	}
	
	void Base::setTarget(const QString &target)
	{
		m_target = target;
		addDict("%target", m_target);
	}
	
	void Base::setTargetDir(const QString &target)
	{
		m_targetdir = target;
		addDict("%dir_target", m_targetdir);
	}

	void Base::setTargetPath(const QString &target)
	{
		QFileInfo fi(target);
		setTarget(fi.fileName());
		setTargetDir(fi.dirPath(true));
	}
	
	bool Base::checkPrereqs()
	{
		return true;
	}

	bool Base::configure(const QString &cfg)
	{
		return m_manager->configure(this, cfg);
	}
	
	void Base::stop()
	{
		if (m_launcher)
			m_launcher->kill();

		//emit(done(this, Aborted));
	}

	bool Base::finish(int result)
	{
		kdDebug() << "==KileTool::Base::finish()==============" << endl;
		if (sender())
		{
			kdDebug() << "\tcalled by " << sender()->name() << " " << sender()->className() << endl;
		}
		
		if ( result == Aborted )
			sendMessage(Error, "Aborted");
		
		if ( result == Success )
			sendMessage(Info,"Done!");

		kdDebug() << "\temitting done(Base*, int) " << name() << endl;
		emit(done(this, result));
	
		//we will only get here if the done() signal is not connected to the manager (who will destroy this object)
		if (result == Success)
			return true;
		else
			return false;
	}

	void Base::installLauncher(Launcher *lr)
	{
		if(m_launcher != lr)
			delete m_launcher;

		m_launcher = lr;
		//lr->setParamDict(paramDict());
		lr->setTool(this);
		
		connect(lr, SIGNAL(message(int, const QString &)), this, SLOT(sendMessage(int, const QString &)));
		connect(lr, SIGNAL(output(const QString &)), this, SLOT(filterOutput(const QString &)));
		connect(lr, SIGNAL(done(int)), this, SLOT(finish(int)));
	}

	bool Base::installLauncher()
	{
		if (m_launcher)
			return true;

		QString type = readEntry("type");
		kdDebug() << "installing launcher of type " << type << endl;
		Launcher *lr = 0;

		if ( type == "Process" )
		{
			lr = new ProcessLauncher();
		}
		else if ( type == "Konsole" )
		{
			lr = new KonsoleLauncher();
		}
		else if ( type == "Part" )
		{	
			lr = new PartLauncher();
		}
		else if ( type == "DocPart" )
		{
			lr = new DocPartLauncher();
		}
		
		if (lr) 
		{
			installLauncher(lr);
			return true;
		}
		else
		{
			m_launcher = 0;
			return false;
		}
	}
	
	void Base::sendMessage(int type, const QString &msg)
	{
		emit(message(type, msg, name()));
	}

	void Base::filterOutput(const QString & str)
	{
		//here you have the change to filter the output and do some error extraction for example
		//this should be done by a OutputFilter class

		//idea: store the buffer until a complete line (or more) has been received then parse these lines
		//just send the buf immediately to the output widget, the results of the parsing are displayed in
		//the log widget anyway.
		emit(output(str));
	}

	bool Base::addDict(const QString & key, const QString & value)
	{
		bool e = (paramDict()->find(key) == 0);
		paramDict()->replace(key, &value);
		return e;
	}

	bool Base::needsUpdate(const QString &target, const QString &source)
	{
		kdDebug() << "==Base::needsUpdate(" << target << "," << source << endl;
		QFileInfo targetinfo(target);
		QFileInfo sourceinfo(source);

		if ( !(sourceinfo.exists() && sourceinfo.isReadable()) )
		{
			kdDebug() << "\treturning false: source doesn't exist" << endl;
			return false;
		}

		if ( ! targetinfo.exists() )
		{
			kdDebug() << "\treturning true: target doesn't exist" << endl;
			return true;
		}

		kdDebug() << "\ttarget: " << targetinfo.lastModified().toString() << endl;
		kdDebug() << "\tsource: " << sourceinfo.lastModified().toString() << endl;
		kdDebug() << "\treturning " << (targetinfo.lastModified() < sourceinfo.lastModified()) << endl;
		return targetinfo.lastModified() < sourceinfo.lastModified();
	}

	Compile::Compile(const QString &name, Manager * manager, bool prepare /*= true*/)
		: Base(name, manager, prepare)
	{
		setFlags( flags() | NeedTargetDirExec | NeedTargetDirWrite);
	}

	Compile::~Compile()
	{}
	
	bool Compile::checkSource()
	{
		if ( !Base::checkSource() ) return false;

		bool isRoot = true;
		KileDocument::TextInfo *docinfo = manager()->info()->docManager()->textInfoFor(source());
		if (docinfo) isRoot = (readEntry("checkForRoot") == "yes") ? docinfo->isLaTeXRoot() : true;

		if (!isRoot)
		{
			return  manager()->queryContinue(i18n("The document %1 is not a LaTeX root document; continue anyway?").arg(source()), i18n("Continue?"));
		}

		return true;
	}
	
	View::View(const QString &name, Manager * manager, bool prepare /*= true*/)
		: Base(name, manager, prepare)
	{
		setFlags( NeedTargetDirExec | NeedTargetExists | NeedTargetRead);
		
		kdDebug() << "View: flag " << (flags() & NeedTargetExists) << endl;
		setMsg(NeedTargetExists, i18n("The file %2/%3 does not exist; did you compile the source file?"));
	}

	View::~View()
	{
	}
	

	Archive::Archive(const QString &name, Manager * manager, bool prepare /* = true*/)
		: Base(name, manager,prepare)
	{
		setFlags( NeedTargetDirExec | NeedTargetDirWrite );
	}
	
	Archive::~Archive()
	{}

	bool Archive::checkPrereqs()
	{
		if(m_project == 0L)
		{	
			sendMessage(Error,i18n("The current document is not associated to a project. Please activate a document that is associated to the project you want to archive, then choose Archive again."));
			return false;
		}
		else if(m_fileList.isEmpty())	
		{
			sendMessage(Error,i18n("No files have been chossen to archive"));
			return false;
		}
		else
			return true;
	}

	void Archive::setSource(const QString &source)
	{	
		KURL url = KURL::fromPathOrURL(source);
		m_project = manager()->info()->docManager()->projectFor(url);
		if ( !m_project )
			m_project = manager()->info()->docManager()->activeProject();
		if ( !m_project )
			m_project = manager()->info()->docManager()->selectProject(i18n("Archive Project"));	
		if ( !m_project )
		{
			Base::setSource(source);
			return;
		}
		
		Base::setSource(m_project->url().path());
		m_fileList = m_project->archiveFileList();
		
		addDict("%AFL", m_fileList);
		
		kdDebug() << "===KileTool::Archive::setSource("<< source << ")==============" << endl;
		kdDebug() << "m_fileList="<<m_fileList<<endl;
	}
	
	Convert::Convert(const QString &name, Manager * manager, bool prepare /*= true*/)
		: Base(name, manager,prepare)
	{
		setFlags( flags() | NeedTargetDirExec | NeedTargetDirWrite );
	}
	
	Convert::~Convert()
	{
	}
	
	bool Convert::determineSource()
	{
		bool  br = Base::determineSource();
		setSource(baseDir() + '/' + S() + '.' + from());
		return br;
	}

	Sequence::Sequence(const QString &name, Manager * manager, bool prepare /*= true*/) : 
		Base(name, manager, prepare)
	{
	}

	int Sequence::run()
	{
		kdDebug() << "==KileTool::Sequence::run()==================" << endl;

 		configure();
		determineSource();
		if (!checkSource()) return NoValidSource;		

		QStringList tools = QStringList::split(',',readEntry("sequence"));
		QString tl, cfg;
		Base *tool;
		for (uint i=0; i < tools.count(); ++i)
		{
			tools[i] = tools[i].stripWhiteSpace();
			extract(tools[i], tl, cfg);

			tool = manager()->factory()->create(tl, false); //create tool with delayed preparation
			if (tool)
			{
				kdDebug() << "===tool created with name " << tool->name() << endl;
				if ( ! (manager()->info()->watchFile() && tool->isViewer() ) )
				{
					kdDebug() << "\tqueueing " << tl << "(" << cfg << ") with " << source() << endl;
					tool->setSource(source());
					manager()->run(tool, cfg);
				}
			}
			else
			{
				sendMessage(Error, i18n("Unknown tool %1.").arg(tools[i]));
				emit(done(this, Failed));
				return ConfigureFailed;
			}
		}

		emit(done(this,Silent));

		return Success;
	}
}

#include "kiletool.moc"
