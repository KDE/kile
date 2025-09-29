/****************************************************************************************
    Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                  2008-2024 by Michel Ludwig (michel.ludwig@kdemail.net)
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
#include <KIO/DesktopExecParser>
#include <KProcess>
#include <KLocalizedString>
#include <KShell>

#include <KParts/Part>
#include <KParts/PartManager>

namespace KileTool {

Launcher::Launcher() :
    m_tool(nullptr)
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

    connect(m_proc, &KProcess::readyReadStandardOutput, this, &ProcessLauncher::slotProcessOutput);
    connect(m_proc, &KProcess::finished, this, &ProcessLauncher::slotProcessExited);
    connect(m_proc, &KProcess::errorOccurred, this, &ProcessLauncher::slotProcessError);
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
    if(tool() == nullptr) {
        qWarning() << "tool() is nullptr which is a BUG";
        return false;
    }
    if(m_proc == nullptr) {
        qWarning() << "m_proc is nullptr which is a BUG";
        return false;
    }

    QString msg;
    QString out = QStringLiteral("*****\n*****     ") + tool()->name() + i18n(" output: \n");

    if(m_cmd.isEmpty()) {
        m_cmd = tool()->readEntry(QStringLiteral("command"));
        KILE_DEBUG_MAIN << "readEntry('command'): " << m_cmd;
    }

    if(m_options.isEmpty()) {
        m_options = tool()->readEntry(QStringLiteral("options"));
        KILE_DEBUG_MAIN << "readEntry('option'):" << m_options;
    }

    if(m_changeTo && (!m_wd.isEmpty())) {
        m_proc->setWorkingDirectory(m_wd);
        KILE_DEBUG_MAIN << "changed to " << m_wd;
        out += QStringLiteral("*****     cd \"") + m_wd + QStringLiteral("\"\n");
    }

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

    out += QStringLiteral("*****     ") + m_cmd + QLatin1Char(' ') + arguments.join(QStringLiteral(" ")) + QLatin1Char('\n');

    QString src = tool()->source(false);
    QString trgt = tool()->target();
    if(src == trgt) {
        msg = src;
    }
    else {
        msg = src + QStringLiteral(" => ") + trgt;
    }

    msg += QStringLiteral(" (") + m_cmd + QLatin1Char(')');

    Q_EMIT(message(Info, msg));

    QString teXInputPaths = tool()->teXInputPaths();
    QString bibInputPaths = tool()->bibInputPaths();
    QString bstInputPaths = tool()->bstInputPaths();

    // QuickView tools need a special TEXINPUTS environment variable
    if(tool()->isQuickie()) {
        teXInputPaths = KileConfig::previewTeXPaths();
        bibInputPaths = KileConfig::previewBibInputPaths();
    }

    KILE_DEBUG_MAIN << "$PATH=" << tool()->manager()->info()->expandEnvironmentVars(QStringLiteral("$PATH"));
    KILE_DEBUG_MAIN << "$TEXINPUTS=" << tool()->manager()->info()->expandEnvironmentVars(teXInputPaths + LIST_SEPARATOR + QStringLiteral("$TEXINPUTS"));
    KILE_DEBUG_MAIN << "$BIBINPUTS=" << tool()->manager()->info()->expandEnvironmentVars(bibInputPaths + LIST_SEPARATOR + QStringLiteral("$BIBINPUTS"));
    KILE_DEBUG_MAIN << "$BSTINPUTS=" << tool()->manager()->info()->expandEnvironmentVars(bstInputPaths + LIST_SEPARATOR + QStringLiteral("$BSTINPUTS"));
    KILE_DEBUG_MAIN << "Tool name is "<< tool()->name();

    m_proc->setEnv(QStringLiteral("PATH"), tool()->manager()->info()->expandEnvironmentVars(QStringLiteral("$PATH")));

    if(!teXInputPaths.isEmpty()) {
        m_proc->setEnv(QStringLiteral("TEXINPUTS"), tool()->manager()->info()->expandEnvironmentVars(teXInputPaths + LIST_SEPARATOR + QStringLiteral("$TEXINPUTS")));
    }
    if(!bibInputPaths.isEmpty()) {
        m_proc->setEnv(QStringLiteral("BIBINPUTS"), tool()->manager()->info()->expandEnvironmentVars(bibInputPaths + LIST_SEPARATOR + QStringLiteral("$BIBINPUTS")));
    }
    if(!bstInputPaths.isEmpty()) {
        m_proc->setEnv(QStringLiteral("BSTINPUTS"), tool()->manager()->info()->expandEnvironmentVars(bstInputPaths + LIST_SEPARATOR + QStringLiteral("$BSTINPUTS")));
    }

    out += QStringLiteral("*****\n");
    Q_EMIT(output(out));

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
            Q_EMIT(message(Error, i18n("terminated")));
            Q_EMIT(done(AbnormalExit));
        }
    }
}

// FIXME: this should be done in the 'launch()' method itself
bool ProcessLauncher::selfCheck()
{
    Q_EMIT(message(Error, i18n("Launching failed, diagnostics:")));

    KShell::Errors err;
    KShell::splitArgs(m_options, KShell::AbortOnMeta | KShell::TildeExpand, &err);
    if(err == KShell::BadQuoting) {
        Q_EMIT(message(Error, i18n("An error occurred while parsing the options given to the tool.")));
        return false;
    }
    else if(err == KShell::FoundMeta) {
        Q_EMIT(message(Error, i18n("Shell meta characters that cannot be handled are present in the options given to the tool.")));
        return false;
    }


    QString exe = KIO::DesktopExecParser::executablePath(tool()->readEntry(QStringLiteral("command")));
    QString path = QStandardPaths::findExecutable(exe);

    if(path.isEmpty()) {
        Q_EMIT(message(Error, i18n("There is no executable named \"%1\" in your path.", exe)));
        return false;
    }
    else {
        QFileInfo fi(path);
        if(!fi.isExecutable()) {
            Q_EMIT(message(Error, i18n("You do not have permission to run %1.", path)));
            return false;
        }
    }

    Q_EMIT(message(Info, i18n("Diagnostics could not find any obvious problems.")));
    return true;
}

void ProcessLauncher::slotProcessOutput()
{
    QByteArray buf = m_proc->readAllStandardOutput();
    Q_EMIT output(QString::fromLocal8Bit(buf.constData(), buf.size()));
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
                Q_EMIT(message(type, i18n("finished with exit code %1", exitCode)));
            }

            if (type == Info) {
                Q_EMIT(done(Success));
            }
            else {
                Q_EMIT(done(Failed));
            }
        }
        else {
            KILE_DEBUG_MAIN << "\tabnormal exit";
            Q_EMIT(message(Error, i18n("finished abruptly")));
            Q_EMIT(done(AbnormalExit));
        }
    }
    else {
        qWarning() << "\tNO PROCESS, emitting done";
        Q_EMIT(done(Success));
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
    Q_EMIT(message(Error, errorString));
    Q_EMIT(done(AbnormalExit));
}

KonsoleLauncher::KonsoleLauncher() : ProcessLauncher()
{
}

bool KonsoleLauncher::launch()
{
    QString cmd = tool()->readEntry(QStringLiteral("command"));
    QString noclose = (tool()->readEntry(QStringLiteral("close")) == QStringLiteral("no")) ? QStringLiteral("--noclose") : QString();
    setCommand(QStringLiteral("konsole"));
    setOptions(noclose + QStringLiteral(" -e ") + cmd + QLatin1Char(' ') + tool()->readEntry(QStringLiteral("options")));
    if(QStandardPaths::findExecutable(KIO::DesktopExecParser::executablePath(cmd)).isEmpty()) {
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
        Q_EMIT(message(Error, i18n("The document viewer is not available")));
        return false;
    }
    if(tool()->manager()->livePreviewManager() && tool()->manager()->livePreviewManager()->isLivePreviewActive()) {
        Q_EMIT(message(Error, i18n("Please disable the live preview before launching this tool")));
        return false;
    }
    const QString fileName = tool()->paramDict()[QStringLiteral("%dir_target")] + QLatin1Char('/') + tool()->paramDict()[QStringLiteral("%target")];
    tool()->manager()->viewManager()->openInDocumentViewer(QUrl::fromLocalFile(fileName));
    if (tool()->paramDict().contains(QStringLiteral("%sourceFileName"))
            && tool()->paramDict().contains(QStringLiteral("%sourceLine"))) {
        const QString sourceFileName = tool()->paramDict()[QStringLiteral("%sourceFileName")];
        const QString lineString = tool()->paramDict()[QStringLiteral("%sourceLine")];
        tool()->manager()->viewManager()->showSourceLocationInDocumentViewer(sourceFileName, lineString.toInt(), 0);
    }
    Q_EMIT(done(Success));

    return true;
}

void DocumentViewerLauncher::kill(bool emitSignals)
{
    Q_UNUSED(emitSignals);
}

}

