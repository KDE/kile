/***************************************************************************
                          kiletoolmanager.h  -  description
                             -------------------
    begin                : Tue Nov 25 2003
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
 ****************************************************************************/

#ifndef KILETOOLMANAGER_H
#define KILETOOLMANAGER_H
#include <qobject.h>

class KileInfo;
class KConfig;
class KTextEdit;
class KAction;
namespace KParts { class PartManager; }
class QWidgetStack;

namespace KileTool
{
	class Base;
	class Factory;
	
 	class Manager : public QObject
	{
		Q_OBJECT
		
	public:
		Manager(KileInfo *ki, KConfig *config, KTextEdit *log, KTextEdit *output, KParts::PartManager *, QWidgetStack *, KAction *);
		~Manager();

	public:
		void initTool(Base*);
		bool configure(Base*);

		void wantGUIState(const QString &);
		
		KParts::PartManager * partManager() { return m_pm; }
		QWidgetStack * widgetStack() { return m_stack; }
		
		KileInfo * info() { return m_ki; }
		KConfig * config() { return m_config; }
		
		void setFactory(Factory* fac) { m_factory = fac; }
		Factory* factory() { return m_factory; }

	public slots:
		void recvMessage(int, const QString &);
		void recvOutput(char *, int);
		
		void started(Base*);
		void done(Base *, int);
		
		void run(const QString &);
		void run(Base *);

	signals:
		void requestGUIState(const QString &);
		void requestSaveAll();

		void stop();

	private:
		KileInfo		*m_ki;
		KConfig		*m_config;
		KTextEdit		*m_log, *m_output;
		KParts::PartManager	*m_pm;
		QWidgetStack 			*m_stack;
		KAction				*m_stop;
		Factory				*m_factory;
	};
}

#endif
