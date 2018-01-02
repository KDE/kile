/* This file is part of the kile project
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>
   Copyright (C) 2003 Jan-Marek Glogowski <glogow@stud.fbi.fh-darmstadt.de>
   Copyright (C) 2008-2010 Michel Ludwig <michel.ludwig@kdemail.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Original from kdebase / kate
*/

#ifndef FINDFILESDIALOG_H
#define FINDFILESDIALOG_H

#include <QDialog>

#include <QStringList>

#include "kileinfo.h"
#include "latexcmd.h"

class QCheckBox;
class QEvent;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;

class KProcess;
class KComboBox;
class KUrlRequester;

#define KILEGREP_MAX 12

namespace KileGrep
{
enum Mode { Project = 0, Directory  };
enum List { SearchItems = 0, SearchPaths, SearchTemplates };
enum TemplateMode { tmNormal = 0, tmCommand, tmCommandWithOption, tmEnv, tmGraphics, tmLabel, tmRefs, tmFiles };
}

namespace KileDialog {

class FindFilesDialog : public QDialog
{
    Q_OBJECT

public:
    FindFilesDialog(QWidget *parent, KileInfo *ki, KileGrep::Mode mode, const char *name = 0);
    ~FindFilesDialog();

    void appendFilter(const QString &name, const QString &filter);

    void appendTemplate(const QString &name, const QString &regexp);
    void clearTemplates();

public Q_SLOTS:
    void slotSearchFor(const QString &pattern);

Q_SIGNALS:
    void itemSelected(const QString &abs_filename, int line);

private:
    KileInfo *m_ki;
    KileGrep::Mode m_mode;
    KProcess *m_proc;
    int m_grepJobs;

    void readConfig();
    void writeConfig();

    QStringList getListItems(KComboBox *combo);
    int findListItem(KComboBox *combo, const QString &s);
    void updateListItems(KComboBox *combo);

    void processOutput(bool forceAll = false);
    void finish();

    void startGrep();
    bool shouldRestart() {
        return (m_grepJobs > 0);
    }
    void clearGrepJobs() {
        m_grepJobs = 0;
    }
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
    QLineEdit *template_edit;
    KComboBox *filter_combo, *pattern_combo, *template_combo;
    KUrlRequester *dir_combo;
    QCheckBox *recursive_box;
    QListWidget *resultbox;
    QPushButton *search_button, *clear_button, *close_button;
    QString m_buf;
    QString m_errbuf;
    QStringList m_filterList;
    QStringList m_TemplateList;
    int m_lastTemplateIndex;

private Q_SLOTS:
    void processExited(int exitCode, QProcess::ExitStatus exitStatus);
    void processStandardOutputReady();
    void processErrorOutputReady();
    void slotItemSelected(const QString&);
    void slotSearch();
    void slotClear();
    void slotClose();
    void slotPatternTextChanged(const QString &);
    void slotTemplateActivated(int index);
};

}

#endif
