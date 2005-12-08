/* This file is part of the kile project
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>
   Copyright (C) 2003 Jan-Marek Glogowski <glogow@stud.fbi.fh-darmstadt.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.

   Original from kdebase / kate
*/

#ifndef __KILE_GREP_DIALOG_H_
#define __KILE_GREP_DIALOG_H_

#include <kdialogbase.h>
#include <qstringlist.h>

#include "kileinfo.h"
#include "latexcmd.h"

class QCheckBox;
class QPushButton;
class QLabel;
class QEvent;

class KComboBox;
class KLineEdit;
class KProcess;
class KURLRequester;
class KListBox;

#define KILEGREP_MAX 12

namespace KileGrep
{
	enum Mode { Project=0, Directory  };
	enum List { SearchItems=0, SearchPaths, SearchTemplates };
	enum TemplateMode { tmNormal=0,tmCommand,tmCommandWithOption,tmEnv,tmGraphics,tmLabel,tmRefs,tmFiles };
}

class KileGrepDialog : public KDialogBase
{
	Q_OBJECT

public:
	KileGrepDialog(QWidget *parent, KileInfo *ki, KileGrep::Mode mode, const char *name = 0);
	~KileGrepDialog();

	void appendFilter(const QString &name, const QString &filter);

	void appendTemplate(const QString &name, const QString &regexp);
	void clearTemplates();

public slots:
	void slotSearchFor(const QString &pattern);

signals:
	void itemSelected(const QString &abs_filename, int line);

private:
	KileInfo *m_ki;
	KileGrep::Mode m_mode;
	KProcess *childproc;
	int m_grepJobs;

	void readConfig();
	void writeConfig();

	QStringList getListItems(KComboBox *combo);
	int findListItem(KComboBox *combo, const QString &s);
	void updateListItems(KComboBox *combo);

	void processOutput();
	void finish();

	void startGrep();
	bool shouldRestart() { return (m_grepJobs > 0); }
	void clearGrepJobs() { m_grepJobs = 0; }
	QString buildFilesCommand();
	QString buildProjectCommand();
	QString getPattern();
	QString getShellPattern();
	QString getCommandList(KileDocument::CmdAttribute attrtype);

	void setupDirectory();
	void setupProject();
	void setDirName(const QString &dir);
	void setFilter(const QString &filter);
	QStringList readList(KileGrep::List listtype);
	void updateLists();
	void updateWidgets();

	QStringList m_projectfiles;
	QString m_projectdir;
	bool m_projectOpened;
	
	QLabel *projectname_label, *projectdirname_label;
	KLineEdit *template_edit;
	KComboBox *filter_combo, *pattern_combo, *template_combo;
	KURLRequester *dir_combo;
	QCheckBox *recursive_box;
	KListBox *resultbox;
	QPushButton *search_button, *clear_button, *close_button;
	QString buf;
	QString errbuf;
	QStringList lastSearchPaths;
	QStringList filter_list;
	QStringList template_list;
	int m_lastTemplateIndex;

private slots:
	void childExited();
	void receivedOutput(KProcess *proc, char *buffer, int buflen);
	void receivedErrOutput(KProcess *proc, char *buffer, int buflen);
	void slotItemSelected(const QString&);
	void slotSearch();
	void slotClear();
	void slotClose();
	void slotFinished();
	void slotPatternTextChanged(const QString &);
	void slotTemplateActivated(int index);
};

#endif // __KILE_GREP_DIALOG_H_
