/***************************************************************************
    date                 : Aug 04 2005
    version              : 0.22
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

#include <qstring.h>

#include <kdialogbase.h>

/**
  *@author dani 
  */

class QLabel;
class QCheckBox;

class KileInfo;
class KLineEdit;
class KProcess;
class KShellProcess;

namespace KileDialog
{

class IncludeGraphics : public KDialogBase
{
	Q_OBJECT

public:
	IncludeGraphics(QWidget *parent,const QString &startdir,KileInfo *ki);
	~IncludeGraphics();

	QString getTemplate();
	QString getFilename();

private slots:
	void chooseFile();
	void updateFigure();

	void slotProcessOutput(KProcess* proc,char* buffer,int buflen);
	void slotProcessExited(KProcess* proc);

	void slotOk();

private:
	void readConfig();
	void writeConfig();
	bool checkParameter();
	QString getOptions();
	QString getInfo();
	bool getPictureSize(int &wpx, int &hpx, QString &wcm, QString &hcm);
	void setInfo();

	QLabel *infolabel;
	KLineEdit *edit_file, *edit_label, *edit_caption, *edit_width, *edit_height, *edit_angle, *edit_bb;
	QCheckBox *cb_center, *cb_pdftex, *cb_figure, *cb_graphicspath;
	QLabel *lb_label, *lb_caption;

	QString m_startdir;  
	QString m_output;

	// current picture
	float m_resolution;

	// default
	bool m_imagemagick;
	bool m_boundingbox;
	float m_defaultresolution;

	void execute(const QString &command);
	
	KileInfo *m_ki;
	KShellProcess* m_proc;
};

}

#endif
