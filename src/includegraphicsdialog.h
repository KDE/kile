/***************************************************************************
    date                 : Nov 02 2005
    version              : 0.23
    copyright            : (C) 2004-2005 by Holger Danielsson, 2004 Jeroen Wijnhout
    email                : holger.danielsson@t-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef INCLUDEGRAPHICSDIALOG_H
#define INCLUDEGRAPHICSDIALOG_H

#include <KDialog>

#include "ui_includegraphicsdialog_base.h"

/**
  *@author dani
  */
class KileInfo;
class K3Process;
class K3ShellProcess;

namespace KileDialog
{

class IncludeGraphics : public KDialog
{
		Q_OBJECT

	public:
		IncludeGraphics(QWidget *parent, const QString &startdir, KileInfo *ki);
		~IncludeGraphics();

		QString getTemplate();
		QString getFilename();

	private Q_SLOTS:
		void slotChooseFilter();
		void slotUrlSelected(const KUrl& url);
		void slotTextChanged(const QString& string);
		void slotProcessOutput(K3Process* proc, char* buffer, int buflen);
		void slotProcessExited(K3Process* proc);

		void slotOk();

	private:
		void readConfig();
		void writeConfig();
		bool checkParameter();
		QString getOptions();
		QString getInfo();
		bool getPictureSize(int &wpx, int &hpx, QString &dpi, QString &wcm, QString &hcm);
		void setInfo();

		Ui::IncludeGraphicsWidget m_widget;

		QString m_startdir;
		QString m_output;

		// current picture
		int m_width, m_height;
		float m_resolution;

		// default
		bool m_imagemagick;
		bool m_boundingbox;
		float m_defaultresolution;

		void execute(const QString &command);

		KileInfo *m_ki;
		K3ShellProcess* m_proc;
};

}

#endif
