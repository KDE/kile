/***************************************************************************
 *   Copyright (C) 2003 by Roland Schulz                                   *
 *   mail@r2s2.de                                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef COMPILE_H
#define COMPILE_H

#include <qobject.h>
#include <qstringlist.h>
#include <qstring.h>
#include <qfileinfo.h>
#include <kprocess.h>
#include <kaction.h>
#include <kxmlguiclient.h>
#include <kate/mainwindow.h>
#include "commandprocess.h"
#include "messagewidget.h"

/**
@author Roland Schulz
*/
class Compile : public QObject, public KXMLGUIClient
{
Q_OBJECT
public:
    Compile(Kate::MainWindow *win, QObject *parent = 0, const char *name = 0);

    ~Compile();

private slots:
	void Latex();
   void ClickedOnOutput(int parag, int index);
   void slotProcessOutput(KProcess* proc,char* buffer,int buflen);
   void slotProcessExited(KProcess* proc);
   void slotDisableStop();

signals:
   void stopProcess();

private:
   CommandProcess* execCommand(const QStringList &command, const QFileInfo &file, bool enablestop);
   QString prepareForCompile(const QString & command);
   void addView();

public:
	MessageWidget *OutputWidget, *LogWidget;
   Kate::MainWindow *m_win;

private:
	QString latex_command;
   bool logpresent, singlemode;
   KShellProcess *currentProcess;
   KAction *StopAction;
   QString MasterName;

};

#endif
