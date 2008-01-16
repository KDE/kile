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

#include <KDialog>

#include <QProcess>

#include "widgets/logwidget.h"
#include "widgets/outputview.h"

#include "ui_postscriptdialog_base.h"

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

class KProcess;

namespace KileDialog
{

class PostscriptDialog : public KDialog
{
		Q_OBJECT

	public:
		PostscriptDialog(QWidget *parent,
		                 const QString &texfilename, const QString &startdir,
		                 const QString &latexextensions,
		                 KileWidget::LogWidget *log, KileWidget::OutputView *output);
		~PostscriptDialog();

	Q_SIGNALS:
		void output(const QString &);

	private Q_SLOTS:
		void comboboxChanged(int index);
		void slotButtonClicked(int button);
		void slotProcessOutput();
		void slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus);

	private:
		bool checkParameter();
		QString buildTempfile();
		QString duplicateParameter(const QString &param);
		void showError(const QString &text);
		void execute();

		QString m_startdir;
		KileWidget::LogWidget *m_log;
		KileWidget::OutputView *m_output;

		QString m_tempfile;
		QString m_program;
		QString m_param;

		KProcess* m_proc;

		Ui::PostscriptDialog m_PostscriptDialog;
};

}

#endif
