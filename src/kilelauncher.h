/***************************************************************************************
    begin                : mon 3-11 20:40:00 CEST 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                               2008 by Michel Ludwig (michel.ludwig@kdemail.net)
 ***************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KILE_LAUNCHER
#define KILE_LAUNCHER

#include <QObject>
#include <QProcess>

class KProcess;

class KileInfo;

namespace KParts {
class ReadOnlyPart;
class PartManager;
}

namespace KileTool
{
class Base;

/**
 * This class represents a way to launch a tool. This could be a commandline tool
 * running in a Konsole, running as a separate process, it could even be responsible
 * for starting a KPart.
 *
 * @author Jeroen Wijnhout
 **/
class Launcher : public QObject
{
    Q_OBJECT

public:
    Launcher();
    ~Launcher();

public Q_SLOTS:
    virtual bool launch() = 0;
    virtual void kill(bool emitSignals = true) = 0;
    virtual bool selfCheck() = 0;

public:
    virtual void setWorkingDirectory(const QString &) {}

    void setTool(Base *tool) {
        m_tool = tool;
    }
    Base* tool() {
        return m_tool;
    }

Q_SIGNALS:
    void message(int, const QString&);
    void output(const QString&);

    void exitedWith(int);
    void abnormalExit();

    void done(int);

private:
    //QDict<QString>	*m_pdictParams;
    Base			*m_tool;
};

class ProcessLauncher : public Launcher
{
    Q_OBJECT

public:
    ProcessLauncher();
    ~ProcessLauncher();

public:
    void setWorkingDirectory(const QString &wd);
    void changeToWorkingDirectory(bool change);
    void setCommand(const QString& cmd);
    void setOptions(const QString& opt);

public Q_SLOTS:
    bool launch();
    void kill(bool emitSignals = true);
    bool selfCheck();

private Q_SLOTS:
    void slotProcessOutput();
    void slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus);
    void slotProcessError(QProcess::ProcessError error);

private:
    QString 	m_wd, m_cmd;
    QString		m_options;
    KProcess	*m_proc;
    bool		m_changeTo;
};

class KonsoleLauncher : public ProcessLauncher
{
    Q_OBJECT

public:
    KonsoleLauncher();

public Q_SLOTS:
    bool launch();
};

class DocumentViewerLauncher : public Launcher
{
    Q_OBJECT

public:
    DocumentViewerLauncher();
    ~DocumentViewerLauncher();

public Q_SLOTS:
    bool launch();
    void kill(bool emitSignals = true);
    bool selfCheck();

};
}

#endif
