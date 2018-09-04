/****************************************************************************************
    Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                  2008-2018 by Michel Ludwig (michel.ludwig@kdemail.net)
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

#include <config.h>

#include "docpart.h"
#include "kileconfig.h"
#include "kileinfo.h"
#include "kiletool.h"
#include "kiletoolmanager.h"
#include "kiletool_enums.h"
#include "kileviewmanager.h"
#include "livepreview.h"

#include <QStackedWidget>
#include <QFileInfo>

#include "kiledebug.h"
#include <KRun>
#include <KProcess>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KShell>

#include <KParts/Part>
#include <KParts/PartManager>

namespace KileTool {

Launcher::Launcher() :
    m_tool(Q_NULLPTR)
{
}

Launcher::~ Launcher()
{
    KILE_DEBUG_MAIN << "DELETING launcher";
}

ProcessLauncher::ProcessLauncher() :
    m_changeTo(true)
{
    KILE_DEBUG_MAIN << "==KileTool::ProcessLauncher::ProcessLauncher()==============";

    m_proc = new KProcess(this);

    m_proc->setOutputChannelMode(KProcess::MergedChannels);
    m_proc->setReadChannel(QProcess::StandardOutput);

    connect(m_proc, SIGNAL(readyReadStandardOutput()), this, SLOT(slotProcessOutput()));
    connect(m_proc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotProcessExited(int,QProcess::ExitStatus)));
    connect(m_proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(slotProcessError(QProcess::ProcessError)));
}

ProcessLauncher::~ProcessLauncher()
{
    KILE_DEBUG_MAIN << "DELETING ProcessLauncher";

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
    if(tool() == Q_NULLPTR) {
        qWarning() << "tool() is Q_NULLPTR which is a BUG";
        return false;
    }
    if(m_proc == Q_NULLPTR) {
        qWarning() << "m_proc is Q_NULLPTR which is a BUG";
        return false;
    }

    QString msg;
    QString out = "*****\n*****     " + tool()->name() + i18n(" output: \n");

    if(m_cmd.isEmpty()) {
        m_cmd = tool()->readEntry("command");
        KILE_DEBUG_MAIN << "readEntry('command'): " << m_cmd;
    }

    if(m_options.isEmpty()) {
        m_options = tool()->readEntry("options");
        KILE_DEBUG_MAIN << "readEntry('option'):" << m_options;
    }

    if(m_changeTo && (!m_wd.isEmpty())) {
        m_proc->setWorkingDirectory(m_wd);
        KILE_DEBUG_MAIN << "changed to " << m_wd;
        out += QString("*****     cd \"") + m_wd + QString("\"\n");
    }

    QString str;
    tool()->translate(m_cmd);
    tool()->translate(m_options, true); // quote the substituted strings using 'KShell::quoteArg'
    // (see bug 314109)
    KILE_DEBUG_MAIN << "after translate: m_cmd=" << m_cmd << ", m_options=" << m_options;

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

    KILE_DEBUG_MAIN << "sent " << m_cmd << ' ' << arguments;

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

    KILE_DEBUG_MAIN << "$PATH=" << tool()->manager()->info()->expandEnvironmentVars("$PATH");
    KILE_DEBUG_MAIN << "$TEXINPUTS=" << tool()->manager()->info()->expandEnvironmentVars(teXInputPaths + LIST_SEPARATOR + "$TEXINPUTS");
    KILE_DEBUG_MAIN << "$BIBINPUTS=" << tool()->manager()->info()->expandEnvironmentVars(bibInputPaths + LIST_SEPARATOR + "$BIBINPUTS");
    KILE_DEBUG_MAIN << "$BSTINPUTS=" << tool()->manager()->info()->expandEnvironmentVars(bstInputPaths + LIST_SEPARATOR + "$BSTINPUTS");
    KILE_DEBUG_MAIN << "Tool name is "<< tool()->name();

    m_proc->setEnv("PATH", tool()->manager()->info()->expandEnvironmentVars("$PATH"));

    if(!teXInputPaths.isEmpty()) {
        m_proc->setEnv("TEXINPUTS", tool()->manager()->info()->expandEnvironmentVars(teXInputPaths + LIST_SEPARATOR + "$TEXINPUTS"));
    }
    if(!bibInputPaths.isEmpty()) {
        m_proc->setEnv("BIBINPUTS", tool()->manager()->info()->expandEnvironmentVars(bibInputPaths + LIST_SEPARATOR + "$BIBINPUTS"));
    }
    if(!bstInputPaths.isEmpty()) {
        m_proc->setEnv("BSTINPUTS", tool()->manager()->info()->expandEnvironmentVars(bstInputPaths + LIST_SEPARATOR + "$BSTINPUTS"));
    }

    out += "*****\n";
    emit(output(out));

    if(tool()->manager()->shouldBlock()) {
        KILE_DEBUG_MAIN << "About to execute: " << m_proc->program();
        m_proc->execute();
    }
    else {
        KILE_DEBUG_MAIN << "About to start: " << m_proc->program();
        m_proc->start();
    }
    return true;
}

void ProcessLauncher::kill(bool emitSignals)
{
    KILE_DEBUG_MAIN << "==KileTool::ProcessLauncher::kill()==============";
    if(m_proc && m_proc->state() == QProcess::Running) {
        KILE_DEBUG_MAIN << "\tkilling";
        m_proc->kill();
        m_proc->waitForFinished(-1);
    }
    else {
        KILE_DEBUG_MAIN << "\tno process or process not running";
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
    QString path = QStandardPaths::findExecutable(exe);

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
    KILE_DEBUG_MAIN << "==KileTool::ProcessLauncher::slotProcessExited=============";
    KILE_DEBUG_MAIN << "\t" << tool()->name();

    if(m_proc) {
        if(exitStatus == QProcess::NormalExit) {
            KILE_DEBUG_MAIN << "\tnormal exit";
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
            KILE_DEBUG_MAIN << "\tabnormal exit";
            emit(message(Error, i18n("finished abruptly")));
            emit(done(AbnormalExit));
        }
    }
    else {
        qWarning() << "\tNO PROCESS, emitting done";
        emit(done(Success));
    }
}

void ProcessLauncher::slotProcessError(QProcess::ProcessError error)
{
    KILE_DEBUG_MAIN << "error =" << error << "tool = " << tool()->name();
    QString errorString;
    switch(error) {
    case QProcess::FailedToStart:
        errorString = i18n("failed to start");
        break;
    case QProcess::Crashed:
        errorString = i18n("crashed");
        break;
    default:
        errorString = i18n("failed (error code %1)", error);
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
    if(QStandardPaths::findExecutable(KRun::binaryName(cmd, false)).isEmpty()) {
        return false;
    }

    return ProcessLauncher::launch();
}

DocumentViewerLauncher::DocumentViewerLauncher()
{
}

DocumentViewerLauncher::~DocumentViewerLauncher()
{
    KILE_DEBUG_MAIN << "DELETING DocumentViewerLauncher";
}

bool DocumentViewerLauncher::selfCheck()
{
    return true;  //no additional self-checks, all of them are done in launch()
}

bool DocumentViewerLauncher::launch()
{
    if(!tool()->manager()->viewManager()->viewerPart()) {
        emit(message(Error, i18n("The document viewer is not available")));
        return false;
    }
    if(tool()->manager()->livePreviewManager() && tool()->manager()->livePreviewManager()->isLivePreviewActive()) {
        emit(message(Error, i18n("Please disable the live preview before launching this tool")));
        return false;
    }
    const QString fileName = tool()->paramDict()["%dir_target"] + '/' + tool()->paramDict()["%target"];
    tool()->manager()->viewManager()->openInDocumentViewer(QUrl::fromLocalFile(fileName));
    if(tool()->paramDict().contains("%sourceFileName")
            && tool()->paramDict().contains("%sourceLine")) {
        const QString sourceFileName = tool()->paramDict()["%sourceFileName"];
        const QString lineString = tool()->paramDict()["%sourceLine"];
        tool()->manager()->viewManager()->showSourceLocationInDocumentViewer(sourceFileName, lineString.toInt(), 0);
    }
    emit(done(Success));

    return true;
}

void DocumentViewerLauncher::kill(bool emitSignals)
{
    Q_UNUSED(emitSignals);
}

}

