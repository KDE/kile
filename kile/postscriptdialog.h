/***************************************************************************
    date                 : Mar 12 2007
    version              : 0.20
    copyright            : (C) 2005-2007 by Holger Danielsson
    email                : holger.danielsson@versanet.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef POSTSCRIPTDIALOG_H
#define POSTSCRIPTDIALOG_H

#include <kdialogbase.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <qstring.h>
//Added by qt3to4:
#include <QLabel>
#include <k3process.h>

#include "kilelogwidget.h"
#include "kileoutputwidget.h"

#define PS_A5_EMPTY       0
#define PS_A5_DUPLICATE   1
#define PS_2xA5           2
#define PS_2xA5L          3
#define PS_4xA5           4
#define PS_A4_EMPTY       5
#define PS_A4_DUPLICATE   6
#define PS_2xA4           7
#define PS_2xA4L          8
#define PS_EVEN           9
#define PS_ODD            10
#define PS_EVEN_REV       11
#define PS_ODD_REV        12
#define PS_REVERSE        13
#define PS_COPY_SORTED    14
#define PS_COPY_UNSORTED  15
#define PS_PSTOPS_FREE    16
#define PS_PSSELECT_FREE  17

class K3ShellProcess;

namespace KileDialog
{

class PostscriptDialog : public KDialogBase   
{
	Q_OBJECT

public:
	PostscriptDialog(QWidget *parent, 
	              const QString &texfilename,const QString &startdir,
                 const QString &latexextensions,
	              KileWidget::LogMsg *log, KileWidget::Output *output);
	~PostscriptDialog();

signals:
	void output(const QString &);

private slots:
	void chooseInfile();
	void chooseOutfile();
	void comboboxChanged(int index);
	void slotUser1();
	void slotProcessOutput(K3Process*,char* buf,int len);
	void slotProcessExited (K3Process *proc);
	
private:
	bool checkParameter();
	QString buildTempfile();
	QString duplicateParameter(const QString &param);
	void showError(const QString &text);
	void execute();
	
	QLineEdit *m_edInfile, *m_edOutfile, *m_edParameter;
	QComboBox *m_cbTask;
	QCheckBox *m_cbView;
	QSpinBox *m_spCopies;
	QLabel *m_lbParameter;
	
	QString m_startdir;
	KileWidget::LogMsg *m_log;
	KileWidget::Output *m_output;
	
	QString m_tempfile;
	QString m_program;
	QString m_param;

	K3ShellProcess* m_proc;

};

}


#endif
