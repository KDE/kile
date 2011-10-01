/***************************************************************************
  Copyright (C) 2003 by Jeroen Wijnhout (jeroen.wijnhout@kdemail.net)
                2010 by Michel Ludwig (michel.ludwig@kdemail.net)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kiletool.h"

#include <QDir>
#include <QFileInfo>
#include <QMetaObject>
#include <QRegExp>
#include <QTimer>

#include <KConfig>
#include <KLocale>
#include <KUrl>

#include "kileconfig.h"
#include "kiletool_enums.h"
#include "kilestdtools.h" //for the factory
#include "kiletoolmanager.h"
#include "kiledocmanager.h"
#include "kileinfo.h"
#include "documentinfo.h"
#include "kileproject.h"

namespace KileTool
{
	Base::Base(const QString &name, Manager *manager, bool prepare /* = true */) :
		QObject(manager), // ensure that they are deleted whenever the tool manager gets deleted
		m_launcher(NULL),
		m_quickie(false),
		m_manager(manager),
		m_name(name),
		m_bPrepareToRun(prepare)
	{
		m_manager->initTool(this);
		
		m_flags = NeedTargetDirExec | NeedTargetDirWrite | NeedActiveDoc | NeedMasterDoc | NoUntitledDoc | NeedSourceExists | NeedSourceRead;

		setMsg(NeedTargetDirExec, ki18n("Could not change to the folder %1."));
		setMsg(NeedTargetDirWrite, ki18n("The folder %1 is not writable, therefore %2 will not be able to save its results."));
		setMsg(NeedTargetExists,  ki18n("The file %1/%2 does not exist. If this is unexpected, check the file permissions."));
		setMsg(NeedTargetRead, ki18n("The file %1/%2 is not readable. If this is unexpected, check the file permissions."));
		setMsg(NeedActiveDoc, ki18n("Could not determine on which file to run %1, because there is no active document."));
		setMsg(NeedMasterDoc, ki18n("Could not determine the master file for this document."));
		setMsg(NoUntitledDoc, ki18n("Please save the untitled document first."));
		setMsg(NeedSourceExists, ki18n("The file %1 does not exist."));
		setMsg(NeedSourceRead, ki18n("The file %1 is not readable."));

		m_bPrepared = false;
	}

	Base::~Base()
	{
		KILE_DEBUG() << "DELETING TOOL: " << name() << this;
		delete m_launcher;
	}

	QString Base::source(bool absolute /* = true */) const
	{
		if (m_source.isEmpty()) {
			return QString();
		}

		QString src = m_source;
		if (absolute) {
			src = m_basedir + '/' + src;
		}

		return src;
	}
	
	void Base::setMsg(long n, const KLocalizedString& msg)
	{
		m_messages[n] = msg;
	}

	void Base::translate(QString &str)
	{
		QHashIterator<QString,QString> it(paramDict());
		while(it.hasNext()) {
			it.next();
			str.replace(it.key(), it.value());
		}
		//Windows doesn't like single quotes on command line '*.tex'
		#ifdef Q_WS_WIN
			str.replace('\'', '\"');
		#endif 
	}

	void Base::prepareToRun(const QString &cfg)
	{
		KILE_DEBUG() << "==Base::prepareToRun()=======";
		
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
		KILE_DEBUG() << "==KileTool::Base::run()=================";
	
		if(m_nPreparationResult != 0) {
			return m_nPreparationResult;
		}

		if(!checkSource()) {
			return NoValidSource;
		}

		if(!checkTarget()) {
			return TargetHasWrongPermissions;
		}

		if (!checkPrereqs()) {
			return NoValidPrereqs;
		}

		//everything ok so far
		emit(requestSaveAll(false, true));
		emit(start(this));
		
		if (!m_launcher || !m_launcher->launch()) {
			KILE_DEBUG() << "\tlaunching failed";
			if(!m_launcher) {
				return CouldNotLaunch;
			}
			if(!m_launcher->selfCheck()) {
				return SelfCheckFailed;
			}
			else {
				return CouldNotLaunch;
			}
		}

		KILE_DEBUG() << "\trunning...";

		return Running;

	}

	bool Base::determineSource()
	{
		QString src = source();

		//the basedir is determined from the current compile target
		//determined by getCompileName()
		if(src.isEmpty()) {
			src = m_ki->getCompileName();
		}
		setSource(src);

		return true;
	}
	
	bool Base::checkSource()
	{
		//FIXME deal with tools that do not need a source or target (yes they exist)
		//Is there an active document? Only check if the source file is not explicitly set.
		if((m_source.isEmpty()) && (m_manager->info()->activeTextDocument() == NULL)) {
			sendMessage(Error, msg(NeedActiveDoc).subs(name()).toString());
			return false;
		}

		if(m_source.isEmpty() && m_manager->info()->activeTextDocument() != NULL) {
			if(m_manager->info()->activeTextDocument()->url().isEmpty()
			   && (flags() & NoUntitledDoc)) {
				sendMessage(Error, msg(NoUntitledDoc).toString());
				emit(requestSaveAll());
				return false;
			}
			else {
				//couldn't find a source file, huh?
				//we know there is an active document, the only reason is could have failed is because
				//we couldn't find a LaTeX root document
				sendMessage(Error, msg(NeedMasterDoc).toString());
				return false;
			}
		}

		QFileInfo fi(source());
		if((flags() & NeedSourceExists) && !fi.exists()) {
			sendMessage(Error, msg(NeedSourceExists).subs(fi.absoluteFilePath()).toString());
			return false;
		}

		if((flags() & NeedSourceRead) && !fi.isReadable()) {
			sendMessage(Error, msg(NeedSourceRead).subs(fi.absoluteFilePath()).toString());
			return false;
		}

		return true;
	}

	void Base::setSource(const QString &source)
	{
		m_from = readEntry("from");

		QFileInfo info(source);
		
		if(!m_from.isEmpty()) {
			QString src = source;
			if((m_from.length() > 0) && (info.suffix().length() > 0)) {
				src.replace(QRegExp(info.suffix() + '$'), m_from);
			}
 			info.setFile(src);
		}

		m_basedir = info.absolutePath();
		m_source = info.fileName();
		m_S = info.completeBaseName();
		
		addDict("%dir_base", m_basedir);
		addDict("%source", m_source);
		addDict("%S",m_S);
		
		KILE_DEBUG() << "===KileTool::Base::setSource()==============";
		KILE_DEBUG() << "using " << source;
		KILE_DEBUG() << "source="<<m_source;
		KILE_DEBUG() << "S=" << m_S;
		KILE_DEBUG() << "basedir=" << m_basedir;
	}
	
	bool Base::determineTarget()
	{
		QFileInfo info(source());

		m_to = readEntry("to");
		
		//if the target is not set previously, use the source filename
		if(m_target.isEmpty()) {
			//test for explicit override
			if (!readEntry("target").isEmpty()) {
				KILE_DEBUG() << "USING target SETTING";
				m_target = readEntry("target");
			}
			else if ( to().length() > 0) {
				m_target = S() + '.' + to();
			}
			else {
				m_target = source(false);
			}
		}

		if(m_relativedir.isEmpty() && (!readEntry("relDir").isEmpty())) {
			m_relativedir = readEntry("relDir");
		}

		KUrl url = KUrl::fromPathOrUrl(m_basedir);
		url.addPath(m_relativedir);
		url.cleanPath();
		m_targetdir = url.toLocalFile();
		
		setTarget(m_target);
		setTargetDir(m_targetdir);		
		
		KILE_DEBUG() << "==KileTool::Base::determineTarget()=========";
		KILE_DEBUG() << "\tm_targetdir=" << m_targetdir;
		KILE_DEBUG() << "\tm_target=" << m_target;
		
		return true;
	}

	bool Base::checkTarget()
	{
		//check if the target directory is accessible
		QFileInfo info(m_targetdir);
		
		if((flags() & NeedTargetDirExec ) && (!info.isExecutable())) {
			sendMessage(Error, msg(NeedTargetDirExec).subs(m_targetdir).toString());
			return false;
		}

		if((flags() & NeedTargetDirWrite) && (!info.isWritable())) {
			sendMessage(Error, msg(NeedTargetDirWrite).subs(m_targetdir).subs(m_name).toString());
			return false;
		}

		info.setFile(m_targetdir + '/' + m_target);

		if((flags() & NeedTargetExists) && (!info.exists())) {
			sendMessage(Error, msg(NeedTargetExists).subs(m_targetdir).subs(m_target).toString());
			return false;
		}

		if((flags() & NeedTargetRead) && (!info.isReadable())) {
			sendMessage(Error, msg(NeedTargetRead).subs(m_targetdir).subs(m_target).toString());
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
		setTargetDir(fi.absolutePath());
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
		KILE_DEBUG() << "==KileTool::Base::finish()==============";
		if (sender())
		{
			KILE_DEBUG() << "\tcalled by " << sender()->objectName() << " " << sender()->metaObject()->className();
		}
		
		if ( result == Aborted )
			sendMessage(Error, "Aborted");
		
		if ( result == Success )
			sendMessage(Info,"Done!");

		KILE_DEBUG() << "\temitting done(Base*, int) " << name();
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
		KILE_DEBUG() << "installing launcher of type " << type;
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
		bool e = !(paramDict().contains(key));
		paramDict()[key] = value;
		return e;
	}

	bool Base::needsUpdate(const QString &target, const QString &source)
	{
		KILE_DEBUG() << "==Base::needsUpdate(" << target << "," << source;
		QFileInfo targetinfo(target);
		QFileInfo sourceinfo(source);
		QDateTime currDateTime = QDateTime::currentDateTime();

		if(!(sourceinfo.exists() && sourceinfo.isReadable())) {
			KILE_DEBUG() << "\treturning false: source does not exist";
			return false;
		}

		if(!targetinfo.exists()) {
			KILE_DEBUG() << "\treturning true: target does not exist";
			return true;
		}

		KILE_DEBUG() << "\ttarget: " << targetinfo.lastModified().toString();
		KILE_DEBUG() << "\tsource: " << sourceinfo.lastModified().toString();
		
		if(targetinfo.lastModified() > currDateTime) {
			KILE_DEBUG() << "targetinfo.lastModifiedTime() is in the future";
			return false;
		}
		else if(sourceinfo.lastModified() > currDateTime) {
			KILE_DEBUG() << "sourceinfo.lastModifiedTime() is in the future";
			return false;
		}
		
		KILE_DEBUG() << "\treturning " << (targetinfo.lastModified() < sourceinfo.lastModified());
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
			return  manager()->queryContinue(i18n("The document %1 is not a LaTeX root document; continue anyway?", source()), i18n("Continue?"));
		}

		return true;
	}
	
	View::View(const QString &name, Manager * manager, bool prepare /*= true*/)
		: Base(name, manager, prepare)
	{
		setFlags(NeedTargetDirExec | NeedTargetExists | NeedTargetRead);
		
		KILE_DEBUG() << "View: flag " << (flags() & NeedTargetExists);
		setMsg(NeedTargetExists, ki18n("The file %1/%2 does not exist; did you compile the source file?"));
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
		if(!m_project) {
			sendMessage(Error,i18n("The current document is not associated to a project. Please activate a document that is associated to the project you want to archive, then choose Archive again."));
			return false;
		}
		else if(m_fileList.isEmpty()) {
			sendMessage(Error, i18n("No files have been chosen for archiving."));
			return false;
		}
		else {
			return true;
		}
	}

	void Archive::setSource(const QString &source)
	{	
		KUrl url = KUrl::fromPathOrUrl(source);
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

		manager()->info()->docManager()->projectSave(m_project);
		Base::setSource(m_project->url().toLocalFile());
		m_fileList = m_project->archiveFileList();
		
		addDict("%AFL", m_fileList);
		
		KILE_DEBUG() << "===KileTool::Archive::setSource("<< source << ")==============";
		KILE_DEBUG() << "m_fileList="<<m_fileList<<endl;
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
		KILE_DEBUG() << "==KileTool::Sequence::run()==================";

		configure();
		determineSource();
		if (!checkSource()) {
			return NoValidSource;
		}

		QStringList tools = readEntry("sequence").split(',');
		QString tl, cfg;
		Base *tool;
		for(int i=0; i < tools.count(); ++i) {
			tools[i] = tools[i].trimmed();
			extract(tools[i], tl, cfg);

			tool = manager()->factory()->create(tl, false); //create tool with delayed preparation
			if (tool) {
				KILE_DEBUG() << "===tool created with name " << tool->name();
				if(!(manager()->info()->watchFile() && tool->isViewer())) {
					KILE_DEBUG() << "\tqueueing " << tl << "(" << cfg << ") with " << source();
					tool->setSource(source());
					manager()->run(tool, cfg);
				}
			}
			else {
				sendMessage(Error, i18n("Unknown tool %1.", tools[i]));
				emit(done(this, Failed));
				return ConfigureFailed;
			}
		}

		emit(done(this, Silent));

		return Success;
	}
}

#include "kiletool.moc"
