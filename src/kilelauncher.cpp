/****************************************************************************************
    Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                  2008-2013 by Michel Ludwig (michel.ludwig@kdemail.net)
 ****************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kilelauncher.h"

#include "config.h"

#include "docpart.h"
#ifdef HAVE_VIEWERINTERFACE_H
  #include "livepreview.h"
#endif
#include "kileconfig.h"
#include "kileinfo.h"
#include "kiletool.h"
#include "kiletoolmanager.h"
#include "kiletool_enums.h"
#include "kileviewmanager.h"

#include <QStackedWidget>
#include <QFileInfo>

#include "kiledebug.h"
#include <KRun>
#include <KProcess>
#include <KLocale>
#include <KShell>
#include <KStandardDirs>
#include <KParts/ComponentFactory>
#include <KParts/Part>
#include <KParts/Factory>
#include <KParts/PartManager>

static QVariantList toVariantList(const QStringList& list)
{
	QVariantList toReturn;
	for(QStringList::const_iterator i = list.begin(); i != list.end(); ++i) {
		toReturn.push_back(QVariant(*i));
	}
	return toReturn;
}

namespace KileTool {

	Launcher::Launcher() :
		m_tool(NULL)
	{
	}

	Launcher::~ Launcher()
	{
		KILE_DEBUG() << "DELETING launcher";
	}

	ProcessLauncher::ProcessLauncher() :
		m_changeTo(true)
	{
		KILE_DEBUG() << "==KileTool::ProcessLauncher::ProcessLauncher()==============";

		m_proc = new KProcess(this);

		m_proc->setOutputChannelMode(KProcess::MergedChannels);
		m_proc->setReadChannel(QProcess::StandardOutput);

		connect(m_proc, SIGNAL(readyReadStandardOutput()), this, SLOT(slotProcessOutput()));
		connect(m_proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotProcessExited(int, QProcess::ExitStatus)));
		connect(m_proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(slotProcessError(QProcess::ProcessError)));
	}

	ProcessLauncher::~ProcessLauncher()
	{
		KILE_DEBUG() << "DELETING ProcessLauncher";

		if(m_proc) {
			// we don't want it to emit any signals as we are being deleted
			m_proc->disconnect();
			kill(false);
			delete m_proc;
		}
	}

	void ProcessLauncher::setWorkingDirectory(const QString &wd)
	{
		m_wd = wd;
	}

	void ProcessLauncher::changeToWorkingDirectory(bool change)
	{
		m_changeTo = change;
	}

	void ProcessLauncher::setCommand(const QString& cmd)
	{
		m_cmd = cmd;
	}

	void ProcessLauncher::setOptions(const QString& opt)
	{
		m_options = opt;
	}

	bool ProcessLauncher::launch()
	{
		if(tool() == NULL){
		  kWarning() << "tool() is NULL which is a BUG";
		  return false;
		}
		if(m_proc == NULL){
		  kWarning() << "m_proc is NULL which is a BUG";
		  return false;
		}

		QString msg;
		QString out = "*****\n*****     " + tool()->name() + i18n(" output: \n");

		if(m_cmd.isEmpty()) {
			m_cmd = tool()->readEntry("command");
			KILE_DEBUG() << "readEntry('command'): " << m_cmd;
		}

		if(m_options.isEmpty()) {
			m_options = tool()->readEntry("options");
			KILE_DEBUG() << "readEntry('option'):" << m_options;
		}

		if(m_changeTo && (!m_wd.isEmpty())) {
			m_proc->setWorkingDirectory(m_wd);
			KILE_DEBUG() << "changed to " << m_wd;
			out += QString("*****     cd \"") + m_wd + QString("\"\n");
		}

		QString str;
		tool()->translate(m_cmd);
		tool()->translate(m_options, true); // quote the substituted strings using 'KShell::quoteArg'
		                                    // (see bug 314109)
		KILE_DEBUG() << "after translate: m_cmd=" << m_cmd << ", m_options=" << m_options;

		if(m_cmd.isEmpty()) {
			return false;
		}

		KShell::Errors err;
		QStringList arguments = KShell::splitArgs(m_options, KShell::AbortOnMeta | KShell::TildeExpand, &err);
		if(err == KShell::BadQuoting || err == KShell::FoundMeta) {
			return false;
		}

		// we cannot use 'KProcess::setShellCommand' here as that method uses 'KStandardDirs::findExe'
		// which doesn't respect the path preferences given by the user, i.e. 'KStandardDirs::findExe' is happy
		// to return the first executable it finds (for example, in '/usr/bin' although the user maybe didn't
		// want to use that directory)
		// BUG: 204397
		m_proc->setProgram(m_cmd, arguments);

		KILE_DEBUG() << "sent " << m_cmd << ' ' << arguments;

		out += QString("*****     ") + m_cmd + ' ' + arguments.join(" ") + '\n';

		QString src = tool()->source(false);
		QString trgt = tool()->target();
		if(src == trgt) {
			msg = src;
		}
		else {
			msg = src + " => " + trgt;
		}

		msg += " (" + m_cmd + ')';

		emit(message(Info, msg));

		QString teXInputPaths = tool()->teXInputPaths();
		QString bibInputPaths = tool()->bibInputPaths();
		QString bstInputPaths = tool()->bstInputPaths();

		// QuickView tools need a special TEXINPUTS environment variable
		if(tool()->isQuickie()) {
			teXInputPaths = KileConfig::previewTeXPaths();
			bibInputPaths = KileConfig::previewBibInputPaths();
		}

		KILE_DEBUG() << "$PATH=" << tool()->manager()->info()->expandEnvironmentVars("$PATH");
		KILE_DEBUG() << "$TEXINPUTS=" << tool()->manager()->info()->expandEnvironmentVars(teXInputPaths + ":$TEXINPUTS");
		KILE_DEBUG() << "$BIBINPUTS=" << tool()->manager()->info()->expandEnvironmentVars(bibInputPaths + ":$BIBINPUTS");
		KILE_DEBUG() << "$BSTINPUTS=" << tool()->manager()->info()->expandEnvironmentVars(bstInputPaths + ":$BSTINPUTS");
		KILE_DEBUG() << "Tool name is "<< tool()->name();

		m_proc->setEnv("PATH", tool()->manager()->info()->expandEnvironmentVars("$PATH"));

		if(!teXInputPaths.isEmpty()) {
			m_proc->setEnv("TEXINPUTS", tool()->manager()->info()->expandEnvironmentVars(teXInputPaths + ":$TEXINPUTS"));
		}
		if(!bibInputPaths.isEmpty()) {
			m_proc->setEnv("BIBINPUTS", tool()->manager()->info()->expandEnvironmentVars(bibInputPaths + ":$BIBINPUTS"));
		}
		if(!bstInputPaths.isEmpty()) {
			m_proc->setEnv("BSTINPUTS", tool()->manager()->info()->expandEnvironmentVars(bstInputPaths + ":$BSTINPUTS"));
		}

		out += "*****\n";
		emit(output(out));

		if(tool()->manager()->shouldBlock()) {
			KILE_DEBUG() << "About to execute: " << m_proc->program();
			m_proc->execute();
		}
		else {
			KILE_DEBUG() << "About to start: " << m_proc->program();
			m_proc->start();
		}
		return true;
	}

	void ProcessLauncher::kill(bool emitSignals)
	{
		KILE_DEBUG() << "==KileTool::ProcessLauncher::kill()==============";
		if(m_proc && m_proc->state() == QProcess::Running) {
			KILE_DEBUG() << "\tkilling";
			m_proc->kill();
			m_proc->waitForFinished(-1);
		}
		else {
			KILE_DEBUG() << "\tno process or process not running";
			if(emitSignals) {
				emit(message(Error, i18n("terminated")));
				emit(done(AbnormalExit));
			}
		}
	}

	// FIXME: this should be done in the 'launch()' method itself
	bool ProcessLauncher::selfCheck()
	{
		emit(message(Error, i18n("Launching failed, diagnostics:")));

		KShell::Errors err;
		QStringList arguments = KShell::splitArgs(m_options, KShell::AbortOnMeta | KShell::TildeExpand, &err);
		if(err == KShell::BadQuoting) {
			emit(message(Error, i18n("An error occurred while parsing the options given to the tool.")));
			return false;
		}
		else if(err == KShell::FoundMeta) {
			emit(message(Error, i18n("Shell meta characters that cannot be handled are present in the options given to the tool.")));
			return false;
		}


		QString exe = KRun::binaryName(tool()->readEntry("command"), false);
		QString path = KGlobal::dirs()->findExe(exe, QString(), KStandardDirs::IgnoreExecBit);

		if(path.isEmpty()) {
			emit(message(Error, i18n("There is no executable named \"%1\" in your path.", exe)));
			return false;
		}
		else {
			QFileInfo fi(path);
			if(!fi.isExecutable()) {
				emit(message(Error, i18n("You do not have permission to run %1.", path)));
				return false;
			}
		}

		emit(message(Info, i18n("Diagnostics could not find any obvious problems.")));
		return true;
	}

	void ProcessLauncher::slotProcessOutput()
	{
		QByteArray buf = m_proc->readAllStandardOutput();
		emit output(QString::fromLocal8Bit(buf, buf.size()));
	}

	void ProcessLauncher::slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
	{
		KILE_DEBUG() << "==KileTool::ProcessLauncher::slotProcessExited=============";
		KILE_DEBUG() << "\t" << tool()->name();

		if(m_proc) {
			if(exitStatus == QProcess::NormalExit) {
				KILE_DEBUG() << "\tnormal exit";
				int type = Info;
				if(exitCode != 0) {
					type = Error;
					emit(message(type, i18n("finished with exit code %1", exitCode)));
				}

				if (type == Info) {
					emit(done(Success));
				}
				else {
					emit(done(Failed));
				}
			}
			else {
				KILE_DEBUG() << "\tabnormal exit";
				emit(message(Error, i18n("finished abruptly")));
				emit(done(AbnormalExit));
			}
		}
		else {
			kWarning() << "\tNO PROCESS, emitting done";
			emit(done(Success));
		}
	}

	void ProcessLauncher::slotProcessError(QProcess::ProcessError error)
	{
		KILE_DEBUG() << "error =" << error << "tool = " << tool()->name();
		QString errorString;
		switch(error) {
			case QProcess::FailedToStart:
				errorString = i18n("failed to start");
				break;
			case QProcess::Crashed:
				errorString = i18n("crashed");
				break;
			default:
				errorString = i18n("failed (error code %i)", error);
				break;
		}
		emit(message(Error, errorString));
		emit(done(AbnormalExit));
	}

	KonsoleLauncher::KonsoleLauncher() : ProcessLauncher()
	{
	}

	bool KonsoleLauncher::launch()
	{
		QString cmd = tool()->readEntry("command");
		QString noclose = (tool()->readEntry("close") == "no") ? "--noclose" : "";
		setCommand("konsole");
		setOptions(noclose + " -e " + cmd + ' ' + tool()->readEntry("options"));
		if(KGlobal::dirs()->findExe(KRun::binaryName(cmd, false)).isEmpty()) {
			return false;
		}

		return ProcessLauncher::launch();
	}

	PartLauncher::PartLauncher() :
		m_part(NULL),
		m_state("Viewer")
	{
	}

	PartLauncher::~PartLauncher()
	{
		// the created part will be deleted in 'Kile::resetPart'
		KILE_DEBUG () << "DELETING PartLauncher";
	}

	void PartLauncher::setLibrary(const QString& lib)
	{
		m_libName = lib;
	}

	void PartLauncher::setClass(const QString& clas)
	{
		m_className = clas;
	}

	void PartLauncher::setOptions(const QString& options)
	{
		m_options = options;
	}

	bool PartLauncher::selfCheck()
	{
		return true;  //no additional self-checks, all of them are done in launch()
	}

	bool PartLauncher::launch()
	{
		m_libName = tool()->readEntry("libName");
		m_className = tool()->readEntry("className");
		m_options = tool()->readEntry("libOptions");
		m_state = tool()->readEntry("state");
#ifdef HAVE_VIEWERINTERFACE_H
		// check if should use the document viewer
		if(tool()->readEntry("useDocumentViewer") == "yes") {
			// and whether it's available
			if(!tool()->manager()->viewManager()->viewerPart()) {
				emit(message(Error, i18n("The document viewer is not available")));
				return false;
			}
			if(tool()->manager()->livePreviewManager()->isLivePreviewActive()) {
				emit(message(Error, i18n("Please disable the live preview before launching this tool")));
				return false;
			}
			const QString fileName = tool()->paramDict()["%dir_target"] + '/' + tool()->paramDict()["%target"];
			tool()->manager()->viewManager()->openInDocumentViewer(KUrl(fileName));
			if(tool()->paramDict().contains("%sourceFileName")
			    && tool()->paramDict().contains("%sourceLine")) {
				const QString sourceFileName = tool()->paramDict()["%sourceFileName"];
				const QString lineString = tool()->paramDict()["%sourceLine"];
				tool()->manager()->viewManager()->showSourceLocationInDocumentViewer(sourceFileName, lineString.toInt(), 0);
			}
			emit(done(Success));

			return true;
		}
#endif
		QString msg, out = "*****\n*****     " + tool()->name() + i18n(" output: \n");

		QString name, shrt;

		// FIXME: this should be made user configurable
		// allow support for embedding ForwardPDF (with Okular) as part (bug 245483)
		if(tool()->paramDict().contains("%absolute_target")) {
			shrt = "%absolute_target";
			tool()->translate(shrt);
			name = shrt;
		}
		else {
			shrt = "%target";
			tool()->translate(shrt);
			QString dir  = "%dir_target";
			tool()->translate(dir);

			name = shrt;
			if(!QDir::isRelativePath(dir)) {
				name = dir + '/' + shrt;
			}
		}

		KPluginLoader pluginLoader(m_libName);
		KPluginFactory *factory = pluginLoader.factory();
		if (!factory) {
			emit(message(Error, i18n("Could not find the %1 library.", m_libName)));
			return false;
		}

		QStackedWidget *stack = tool()->manager()->widgetStack();
		KParts::PartManager *pm = tool()->manager()->partManager();

		m_part = factory->create<KParts::ReadOnlyPart>(stack, toVariantList(KShell::splitArgs(m_options, KShell::AbortOnMeta)));

		if(!m_part) {
			emit(message(Error, i18n("Could not create component %1 from the library %2.", m_className, m_libName)));
			emit(done(Failed));
			return false;
		}
		else {
			QString cmd = QString(m_libName) + "->" + QString(m_className) + ' ' + m_options + ' ' + name;
			out += "*****     " + cmd + '\n';

			msg = shrt+ " (" + tool()->readEntry("libName") + ')';
			emit(message(Info,msg));
		}

		out += "*****\n";
		emit(output(out));

		tool()->manager()->wantGUIState(m_state);

		stack->insertWidget(1, m_part->widget());
		stack->setCurrentIndex(1);

		m_part->openUrl(KUrl(name));
		pm->addPart(m_part, true);
		pm->setActivePart(m_part);

		emit(done(Success));

		return true;
	}

	void PartLauncher::kill(bool emitSignals)
	{
		Q_UNUSED(emitSignals);
	}

	KParts::ReadOnlyPart* PartLauncher::part()
	{
		return m_part;
	}

	DocPartLauncher::DocPartLauncher() : PartLauncher()
	{
	}

	bool DocPartLauncher::launch()
	{
		m_state=tool()->readEntry("state");

		QString shrt = "%target";
		tool()->translate(shrt);
		QString name="%dir_target/%target";
		tool()->translate(name);

		QString out = "*****\n*****     " + tool()->name() + i18n(" output: \n") + "*****     KHTML " + name + "\n*****\n";
		QString msg =  shrt+ " (KHTML)";
		emit(message(Info, msg));
		emit(output(out));

		QStackedWidget *stack = tool()->manager()->widgetStack();
		KParts::PartManager *pm = tool()->manager()->partManager();

		DocumentationViewer *htmlpart = new DocumentationViewer(stack);
		htmlpart->setObjectName("help");
		m_part = static_cast<KParts::ReadOnlyPart*>(htmlpart);

		connect(htmlpart, SIGNAL(updateStatus(bool, bool)), tool(), SIGNAL(updateStatus(bool, bool)));

		tool()->manager()->wantGUIState(m_state);

		htmlpart->openUrl(KUrl(name));
		htmlpart->addToHistory(name);
		stack->insertWidget(1, htmlpart->widget());
		stack->setCurrentIndex(1);

		pm->addPart(htmlpart, true);
		pm->setActivePart( htmlpart);

		emit(done(Success));

		return true;
	}

}

#include "kilelauncher.moc"
