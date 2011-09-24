/***************************************************************************
    begin                : May 12 2009
    copyright            : (C) 2009 dani
  ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef PDFDIALOG_H
#define PDFDIALOG_H

#include <KDialog>
#include <KTempDir>

#include <QProcess>
#include <QMap>
#include <QVector>
#include <QCheckBox>
#include <QLineEdit>

#include "widgets/logwidget.h"
#include "widgets/outputview.h"

#include "kiletool_enums.h"
#include "kiletoolmanager.h"

#include "ui_pdfdialog_base.h"


class KProcess;

namespace KileDialog
{

class PdfDialog : public KDialog
{
		Q_OBJECT

	public:
		PdfDialog(QWidget *parent,
		          const QString &texfilename, const QString &startdir,
		          const QString &latexextensions,
		          KileTool::Manager *manager,
		          KileWidget::LogWidget *log, KileWidget::OutputView *output);
		~PdfDialog();

	Q_SIGNALS:
		void output(const QString &);

	private Q_SLOTS:
		void slotInputfileChanged(const QString &text);
		void slotOutputfileChanged(const QString &text);
		void slotTaskChanged(int index);
		void slotOverwriteChanged(int state);
		void slotButtonClicked(int button);
		void slotTabwidgetChanged(int index);
		void slotPrintingClicked();
		void slotAllClicked();
		void slotPermissionClicked(bool);

		void slotProcessOutput();
		void slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus);

	private:
		enum PDF_Action { PDF_PAGE_EMPTY=0,     PDF_PAGE_DUPLICATE=1, PDF_2UP=2,               PDF_2UP_LANDSCAPE=3,
		                  PDF_4UP=4,            PDF_4UP_LANDSCAPE=5,  PDF_EVEN=6,              PDF_ODD=7,
		                  PDF_EVEN_REV=8,       PDF_ODD_REV=9,        PDF_REVERSE=10,          PDF_SELECT=11,
		                  PDF_PDFPAGES_FREE=12,  PDF_PDFTK_FREE=13,   PDF_PDFTK_BACKGROUND=14, PDF_PDFTK_STAMP=15 };

		enum PDF_ScriptMode { PDF_SCRIPTMODE_TOOLS=0,      PDF_SCRIPTMODE_ACTION=1,
		                      PDF_SCRIPTMODE_PROPERTIES=2, PDF_SCRIPTMODE_PERMISSIONS=3 };

		QString m_inputfile;
		QString m_outputfile;

		void executeAction();
		void executeProperties();
		void executePermissions();

		bool checkParameter();
		bool checkProperties();
		bool checkPermissions();
		bool checkInputFile();
		bool checkPassword();

		QString buildActionCommand();
		QString buildLatexFile(const QString &param);
		QString buildPageRange(int type);
		QString buildPageList(bool even);
		QString buildReversPageList(bool even);
		QString getOutfileName(const QString &infile);
		void showError(const QString &text);

		void executeScript(const QString &command, const QString &dir, int scriptmode);
		void showLogs(const QString &title, const QString &inputfile, const QString &param);

		void initUtilities();
		void finishPdfAction(bool state);
		void runViewer();
		void updateDialog();
		void updateOwnerPassword(bool infile_exists);
		void updateToolsInfo();
		void updateTasks();
		int taskIndex(int index);
		void clearDocumentInfo();
		void setPermissions(bool print,bool other);

		void pdfparser(const QString &filename);
		void setDateTimeInfo(const QString &value, QLabel *label);

		QString m_startdir;
		KileTool::Manager *m_manager;
		KileWidget::LogWidget *m_log;
		KileWidget::OutputView *m_output;

		int m_scriptmode;
		QString m_outputtext;
		bool m_execLatex;
		QString m_param;
		bool m_scriptrunning;

		KTempDir *m_tempdir;
		QStringList m_move_filelist;
		
		bool m_pdftk;
		bool m_pdfpages;

		int  m_numpages;
		bool  m_encrypted;

		QStringList m_pdfInfoKeys;
		QMap<QString,QString> m_pdfInfo;
		QMap<QString,QLineEdit *> m_pdfInfoWidget;
		QMap<QString,QString> m_pdfInfoPdftk;

		QVector<int> m_pdfPermissionKeys;
		QVector<QCheckBox *> m_pdfPermissionWidgets;
		QVector<QString> m_pdfPermissionPdftk;
		QVector<bool> m_pdfPermissionState;

		KProcess* m_proc;

		Ui::PdfDialog m_PdfDialog;
};

}

#endif
