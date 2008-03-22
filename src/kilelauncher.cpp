/****************************************************************************************
    begin                : mon 3-11 20:40:00 CEST 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                               2008 by Michel Ludwig (michel.ludwig@kdemail.net)
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

#include "kileinfo.h"
#include "kiletool.h"
#include "kiletoolmanager.h"
#include "kiletool_enums.h"
#include "docpart.h"
#include "kileconfig.h"

#include <QStackedWidget>
#include <QFileInfo>

#include "kiledebug.h"
#include <KRun>
#include <KProcess>
#include <KLocale>
#include <KShell>
#include <KStandardDirs>
#include <KLibLoader>
#include <KParts/Part>
#include <KParts/Factory>
#include <KParts/PartManager>

namespace KileTool {

	Launcher::Launcher() :
		m_tool(NULL)
	{
	}
	
	Launcher::~ Launcher()
	{
		KILE_DEBUG() << "DELETING launcher";
	}

	ProcessLauncher::ProcessLauncher(const QString& shellCommand) :
		m_texinputs(KileConfig::teXPaths()),
		m_bibinputs(KileConfig::bibInputPaths()),
 		m_bstinputs(KileConfig::bstInputPaths()),
		m_changeTo(true)
	{
		KILE_DEBUG() << "==KileTool::ProcessLauncher::ProcessLauncher()==============";

		m_proc = new KProcess(this);
		if(m_proc) {
			KILE_DEBUG() << "\tKProcess created";
		}
		else {
			KILE_DEBUG() << "\tNo KProcess created";
		}

		m_proc->setOutputChannelMode(KProcess::MergedChannels);
		m_proc->setReadChannel(QProcess::StandardOutput);
		if(!shellCommand.isEmpty()) {
			m_proc->setShellCommand(shellCommand);
		}

		connect(m_proc, SIGNAL(readyReadStandardOutput()), this, SLOT(slotProcessOutput()));
		connect(m_proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotProcessExited(int, QProcess::ExitStatus)));
	}

	ProcessLauncher::~ProcessLauncher()
	{
		KILE_DEBUG() << "DELETING ProcessLauncher";
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
		KILE_DEBUG() << "KileTool::ProcessLauncher::launch()=================";
		KILE_DEBUG() << "\tbelongs to tool " << tool()->name();

		QString msg, out = "*****\n*****     " + tool()->name() + i18n(" output: \n");

		if(m_cmd.isEmpty()) {
			m_cmd = tool()->readEntry("command");
		}

		if(m_options.isEmpty()) {
#ifdef __GNUC__
#warning Better turn this into a QStringList and check whether KShell::quote has been used!
#endif
			m_options = tool()->readEntry("options");
		}

		if(m_changeTo && (!m_wd.isEmpty())) {
			m_proc->setWorkingDirectory(m_wd);
			out += QString("*****     cd '") + m_wd + QString("'\n");
		}

		QString str;
		tool()->translate(m_cmd);
		tool()->translate(m_options);
		if(m_cmd.isEmpty()) {
			return false;
		}
		*m_proc << m_cmd << KShell::splitArgs(m_options, KShell::AbortOnMeta);

		if (m_proc) {
			out += QString("*****     ") + m_cmd+ ' ' + m_options + '\n';

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

			// QuickView tools need a special TEXINPUTS environment variable
			if(tool()->isQuickie()) {
				m_texinputs = KileConfig::previewTeXPaths();
			}

			KILE_DEBUG() << "$PATH=" << tool()->manager()->info()->expandEnvironmentVars("$PATH");
			KILE_DEBUG() << "$TEXINPUTS=" << tool()->manager()->info()->expandEnvironmentVars(m_texinputs + ":$TEXINPUTS");
			KILE_DEBUG() << "$BIBINPUTS=" << tool()->manager()->info()->expandEnvironmentVars(m_bibinputs + ":$BIBINPUTS");
			KILE_DEBUG() << "$BSTINPUTS=" << tool()->manager()->info()->expandEnvironmentVars(m_bstinputs + ":$BSTINPUTS");
			KILE_DEBUG() << "Tool name is "<< tool()->name();

			m_proc->setEnv("PATH", tool()->manager()->info()->expandEnvironmentVars("$PATH"));

			if(!m_texinputs.isEmpty()) {
				m_proc->setEnv("TEXINPUTS", tool()->manager()->info()->expandEnvironmentVars(m_texinputs + ":$TEXINPUTS"));
			}
			if(!m_bibinputs.isEmpty()) {
				m_proc->setEnv("BIBINPUTS", tool()->manager()->info()->expandEnvironmentVars(m_bibinputs + ":$BIBINPUTS"));
			}
			if(!m_bstinputs.isEmpty()) {
				m_proc->setEnv("BSTINPUTS", tool()->manager()->info()->expandEnvironmentVars(m_bstinputs + ":$BSTINPUTS"));
			}

			out += "*****\n";
			emit(output(out));

			if(tool()->manager()->shouldBlock()) {
				m_proc->execute();
			}
			else {
				m_proc->start();
			}
		}
		else {
			return false;
		}
		return true;
	}

	void ProcessLauncher::kill()
	{
		KILE_DEBUG() << "==KileTool::ProcessLauncher::kill()==============";
		if(m_proc && m_proc->state() == QProcess::Running) {
			KILE_DEBUG() << "\tkilling";
			m_proc->kill();
		}
		else {
			KILE_DEBUG() << "\tno process or process not running";
		}
	}

	bool ProcessLauncher::selfCheck()
	{
		emit(message(Error, i18n("Launching failed, diagnostics:")));

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
				emit(message(Error,i18n("finished abruptly")));
				emit(done(AbnormalExit));
			}
		}
		else {
			kWarning() << "\tNO PROCESS, emitting done";
			emit(done(Success));
		}
	}

	KonsoleLauncher::KonsoleLauncher(const QString& shellCommand) : ProcessLauncher(shellCommand)
	{
	}

	bool KonsoleLauncher::launch()
	{
		QString cmd = tool()->readEntry("command");
		QString noclose = (tool()->readEntry("close") == "no") ? "--noclose" : "";
		setCommand("konsole");
		setOptions(noclose + " -T \"" + cmd + " (Kile)\" -e " + cmd + ' ' + tool()->readEntry("options"));

		if(KGlobal::dirs()->findExe(KRun::binaryName(cmd, false)).isEmpty()) {
			return false;
		}

		return ProcessLauncher::launch();
	}

	PartLauncher::PartLauncher() :
		m_state("Viewer")
	{
	}

	PartLauncher::~PartLauncher()
	{
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
		m_options=tool()->readEntry("libOptions");
		m_state=tool()->readEntry("state");

		QString msg, out = "*****\n*****     " + tool()->name() + i18n(" output: \n");
		
		QString shrt = "%target";
		tool()->translate(shrt);
		QString dir  = "%dir_target"; tool()->translate(dir);

		QString name = shrt;
		if(dir[0] == '/') {
			name = dir + '/' + shrt;
		}

		KLibFactory *factory = KLibLoader::self()->factory(m_libName);
		if (!factory) {
			emit(message(Error, i18n("Could not find the %1 library.", m_libName)));
			return false;
		}

		QStackedWidget *stack = tool()->manager()->widgetStack();
		KParts::PartManager *pm = tool()->manager()->partManager();

#ifdef __GNUC__
#warning Port KPluginFactory::create correctly!
#endif
//FIXME: port for KDE4
// 		m_part = (KParts::ReadOnlyPart *)factory->create(stack, m_libName, m_className, m_options);
m_part = NULL;
		if (!m_part) {
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

	void PartLauncher::kill()
	{
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
