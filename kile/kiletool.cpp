/***************************************************************************
                          kiletool.cpp  -  description
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

#include <qdir.h>
#include <qfileinfo.h>
#include <qregexp.h>

#include <klocale.h>
#include <kconfig.h>
#include <kurl.h>

#include "kiletool_enums.h"
#include "kiletool.h"
#include "kiletoolmanager.h"
#include "kileinfo.h"
#include "kiledocumentinfo.h"

namespace KileTool
{
	Base::Base(const QString &name, Manager * manager) :
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
		m_launcher(0L)
	{
		m_manager->initTool(this);
		
		m_flags = NeedTargetDirExec | NeedTargetDirWrite | NeedActiveDoc | NeedMasterDoc | NoUntitledDoc;

		setMsg(NeedTargetDirExec, i18n("Could not change to the folder %1."));
		setMsg(NeedTargetDirWrite, i18n("The folder %1 is not writable, therefore %2 will not be able to save its results."));
		setMsg(NeedTargetExists,  i18n("The file %1/%2 does not exist. If you're surprised, check the file permissions."));
		setMsg(NeedTargetRead, i18n("The file %1/%2 is not readable. If you're surprised, check the file permissions."));
		setMsg(NeedActiveDoc, i18n("Could not determine on which file to run %1, because there is no active document."));
		setMsg(NeedMasterDoc, i18n("Could not determine the master file for this document."));
		setMsg(NoUntitledDoc, i18n("Please save the untitled document first."));
	}

	Base::~Base()
	{
		delete m_launcher;
	}

	const QString Base::source(bool absolute /* = true */) const
	{
		if (m_source == QString::null)
			return QString::null;

		QString src = m_source;
		if (absolute)
			src = m_basedir+"/"+src;
			
		return src;
	}
	
	void Base::setMsg(long n, QString msg)
	{
		m_messages[n] = msg;
	}

	int Base::run()
	{
		kdDebug() << "==KileTool::Base::run()=================" << endl;
		
		//configure me
		if (!configure())
			return ConfigureFailed;
		
		//install a launcher
		if (!installLauncher())
			return NoLauncherInstalled;
		
		if (!determineSource())
			return NoValidSource;
			
		if (!determineTarget())
			return NoValidTarget;
			
		if (!checkTarget())
			return TargetHasWrongPermissions;
		
		if (!checkPrereqs())
			return NoValidPrereqs;
			
		if ( m_launcher == 0 )
			return NoLauncherInstalled;
		
		m_launcher->setWorkingDirectory(workingDir());
				
		//fill in the dictionary
		addDict("%options", m_options);

		//everythin ok so far
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
		//determine the basedir

		//Is there an active document? Only check if the source file is not explicitly set.
		if ( (src == QString::null) && (m_manager->info()->activeDocument() == 0) )
		{ 
			sendMessage(Error, msg(NeedActiveDoc).arg(name()));
			return false;
		}
				
		//the basedir is determined from the current compile target
		//determined by getCompileName()
		if ( src == QString::null)
		{
			src= m_ki->getCompileName();
			kdDebug() << "SOURCE " << src << endl;
		}

		if ( src == QString::null)
		{
			//couldn't find a source file, huh?
			//we know there is an active document, the only reason is could have failed is because
			//we couldn't find a LaTeX root document
			sendMessage(Error, msg(NeedMasterDoc));
			return false;
		}
		
		if ( src == i18n("Untitled") )
		{
			sendMessage(Error, msg(NoUntitledDoc));
			return false;
		}

		setSource(src);

		emit(requestSaveAll());

		return true;
	}
	
	void Base::setSource(const QString &source)
	{
		m_from = readEntry("from");

		QFileInfo info(source);
		
		if (m_from != QString::null)
		{
			QString src = source;
			if ( m_from.length() > 0) src.replace(QRegExp(info.extension()+"$"),m_from);
			info.setFile(src);
		}

		m_basedir = info.dirPath(true);
		m_source = info.fileName();
		m_S = info.baseName(true);
		
		addDict("%dir_base", m_basedir);
		addDict("%source", m_source);
		addDict("%S",m_S);
		
		kdDebug() << "==KileTool::Base::setSource()==============" << endl;
		kdDebug() << "\tusing " << source << endl;
		kdDebug() << "\tsource="<<m_source<<endl;
		kdDebug() << "\tS=" << m_S << endl;
		kdDebug() << "\tbasedir=" << m_basedir << endl;
	}
	
	bool Base::determineTarget()
	{
		QFileInfo info(source());
		
		m_to = readEntry("to");
		
		//if the target is not explicitly set, use the source filename
		if (m_target == QString::null)
		{
			if ( to().length() > 0)
				m_target = S()+"."+to();
			else
				m_target = source(false);
		}

		KURL url = KURL::fromPathOrURL(m_basedir);
		url.addPath(m_relativedir);
		m_targetdir = url.path();
		
		addDict("%dir_target", m_targetdir);
		addDict("%target", m_target);
		
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

		info.setFile(m_targetdir+"/"+m_target);

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
	
	bool Base::checkPrereqs()
	{
		return true;
	}

	bool Base::configure()
	{
		m_manager->configure(this);
		
		return true;
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

		kdDebug() << "\temitting done(Base*) " << name() << endl;
		emit(done(this, result));
	
		//we will only get here if the done() signal is not connected to the manager (who will destroy this object)
		if (result == Success)
			return true;
		else
			return false;
	}

	void Base::installLauncher(Launcher *lr)
	{
		m_launcher = lr;
		lr->setParamDict(paramDict());
		lr->setTool(this);
		
		connect(lr, SIGNAL(message(int, const QString &)), this, SLOT(sendMessage(int, const QString &)));
		connect(lr, SIGNAL(output(char *, int)), this, SLOT(filterOutput(char *, int)));
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
		else if (type == "Part" )
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
		emit(message(type, "<b>["+m_name+"]</b> " + msg));
	}

	void Base::filterOutput(char * buf, int len)
	{
		//here you have the change to filter the output and do some error extraction for example
		//this should be done by a OutputFilter class

		//idea: store the buffer until a complete line (or more) has been received then parse these lines
		//just send the buf immediately to the output widget, the results of the paring are displayed in
		//the log widget anyway.
		emit(output(buf,len));
	}

	bool Base::addDict(const QString & key, const QString & value)
	{
		bool e = (paramDict()->find(key) == 0);
		paramDict()->replace(key, &value);
		return e;
	}

	Compile::Compile(const QString &name, Manager * manager)
		: Base(name, manager)
	{
		setFlags( flags() | NeedTargetDirExec | NeedTargetDirWrite);
	}

	Compile::~Compile()
	{
	}
	
	bool Compile::determineSource()
	{
		if (!Base::determineSource())
			return false;
			
		bool isRoot = true;
		KileDocumentInfo *docinfo = manager()->info()->infoFor(source());
		if (docinfo) isRoot = manager()->info()->checkForRoot() ? docinfo->isLaTeXRoot() : true;
		
		if (!isRoot)
		{
			sendMessage(Error, i18n("The current document is not a LaTeX root document; won't run %1.").arg(name()));
			return false;
		}
		
		return true;
	}
	
	View::View(const QString &name, Manager * manager)
		: Base(name, manager)
	{
		setFlags( flags() | NeedTargetExists | NeedTargetRead);
		
		setMsg(NeedTargetExists, i18n("The file %2/%3 does not exist. Did you compile the source file?"));
	}

	View::~View()
	{
	}
	
	Convert::Convert(const QString &name, Manager * manager)
		: Base(name, manager)
	{
		setFlags( flags() | NeedTargetDirExec | NeedTargetDirWrite);
	}
	
	Convert::~ Convert()
	{
	}
	
	bool Convert::determineSource()
	{
		bool  br = Base::determineSource();
		setSource(baseDir()+"/"+S()+"."+from());
		return br;
	}

	Sequence::Sequence(const QString &name, Manager * manager, bool autoDelete /* = false */) : 
		Base(name, manager),
		m_index(0),
		m_current(0L)
	{
		m_tools.setAutoDelete(autoDelete);
	}

	void Sequence::append(Base *tool)
	{
		m_tools.append(tool);
	}
	
	int Sequence::run()
	{
		kdDebug() << "==KileTool::Sequence::run()==================" << endl;
		
		//determine source file (don't let the tools figure it out themselves, the result of determineTarget() could change during execution)
		if (!determineSource())
			return NoValidSource;
		
		kdDebug() << "\tsource " << source() << endl;
		
		if (m_tools.first())
		{
			m_launcher=m_tools.first()->launcher();
			emit(start(this));
			return runNext(m_tools.first(), Success);
		}
		else
		{
			sendMessage(Warning, i18n("Empty tool list."));
			return Failed;
		}
	}
	
	void Sequence::started(Base *tool)
	{
		m_current = tool;
	}

	bool Sequence::finish(int result)
	{
		kdDebug() << "==KileTool::Sequence::finish()====================" << endl;
		emit(done(this,result));
		return true;
	}
	
	void Sequence::stop()
	{
		kdDebug() << "==KileTool::Sequence::stop()=====================" << endl;
		//Base *current = m_tools.at(index());
		if (m_current)
		{
			kdDebug() << "\tstopping " << m_current->name() << endl;
			m_current->stop();
		}
			
		finish(Aborted);
	}
	
	int Sequence::runNext(Base *tool, int r)
	{
		kdDebug() << "==KileTool::Sequence::runNext()=================" << endl;
		kdDebug() << "\tindex " << index() << endl;

		//stop if the last tool was executed
		if ( index() > count() )
		{
			//disconnect(current(), SIGNAL(done(Base*, int)), this, SLOT(runNext(Base*, int)));
			kdDebug() << "\tthat's all folks!" << endl;
			finish(Success);
			return Success;
		}
		
		//update current and next
		Base *current = m_tools.at(index());
		next();

		//stop if there is no tool queued anymore
		if ( current == 0 )
		{
			kdDebug() << "\tcurrent == 0" << endl;
			finish(Failed);
			return Failed;
		}
		
		//or if last tool finished unsuccessfully
		if ( r != Success )
		{
			kdDebug() << "\tprocess failed" << endl;
			finish(Failed);
			return Failed;
		}
		
		
		kdDebug() << "\tabout to run: " << current->name() << endl;
		kdDebug() << "\tusing source: " << source() << endl;

		current->setSource(source());

		disconnect(current, SIGNAL(done(Base*, int)), manager(), SLOT(done(Base*, int))); //disconnect from manager
		disconnect(current, SIGNAL(start(Base* )), this, SLOT(started(Base* )));
		connect(current, SIGNAL(done(Base*, int)), this, SLOT(runNext(Base*, int))); //instead connect to the Sequence tool

		int result;
		if ( (result = current->run())  != KileTool::Running )
		{
			kdDebug() << "\t\tOOPS! ("<< result << ")" << current->name() << endl;
			QObject::disconnect(0, 0, this, SLOT(runNext(Base*, int)));
			finish(Failed);
		}

		//m_launcher=current->launcher();

		return result;
	}
}

#include "kiletool.moc"
