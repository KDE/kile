/***************************************************************************
                          kilestdtools.h  -  description
                             -------------------
    begin                : Thu Nov 27 2003
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

#ifndef KILESTDTOOLS_H
#define KILESTDTOOLS_H

#include <qstring.h>

class KConfig;

namespace KileTool
{
	class View;
	class Compile;
	class Manager;

	class Factory
	{
	public:
		Factory(Manager *mngr, KConfig *config) : m_manager(mngr), m_config(config) {}
		~Factory() {}

		Base* create(const QString & tool);

		void writeStdConfig();

	private:
		Manager		*m_manager;
		KConfig		*m_config;
	};

	class LaTeX : public Compile
	{
		Q_OBJECT

	public:
		LaTeX(const QString & tool, Manager *mngr) : Compile(tool, mngr) {}

	signals:
		void jumpToFirstError();

	public slots:
		bool finish(int);

	private:
		bool updateBibs();
		bool updateIndex();

	private:
		static int m_reRun;
	};

	class ForwardDVI : public View
	{
	public:
		ForwardDVI(const QString & tool, Manager *mngr) : View(tool, mngr) {}

	protected:
		bool determineTarget();

	private:
		QString	m_urlstr;
	};

	class ViewBib : public View
	{
	public:
		ViewBib(const QString & tool, Manager *mngr) : View(tool, mngr) {}

	protected:
		bool determineSource();
	};

	class ViewHTML : public View
	{
		Q_OBJECT

	public:
		ViewHTML(const QString & tool, Manager *mngr) : View(tool, mngr) {}

	protected:
		bool determineTarget();

	signals:
		void updateStatus(bool, bool);
	};
}

#endif
