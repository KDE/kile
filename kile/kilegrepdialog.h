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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Original from kdebase / kate
*/

#ifndef __KILE_GREP_DIALOG_H_
#define __KILE_GREP_DIALOG_H_

#include <kdialog.h>
#include <qstringlist.h>

class QLineEdit;
class QComboBox;
class QCheckBox;
class QListBox;
class QPushButton;
class QLabel;
class KProcess;
class KConfig;
class KURLRequester;
class QEvent;

class KileGrepDialog : public KDialog
{
	Q_OBJECT

public:
	KileGrepDialog(const QString &dirname, QWidget *parent = 0, const char *name = 0);
	~KileGrepDialog();

	void setDirName(const QString &dir);

	void setFilter(const QString &filter);
	void appendFilter(const QString &name, const QString &filter);

	void appendTemplate(const QString &name, const QString &regexp);
	void clearTemplates();

public slots:
	void slotSearchFor(const QString &pattern);

signals:
	void itemSelected(const QString &abs_filename, int line);

protected:
	bool eventFilter( QObject *, QEvent * );

private:
	void processOutput();
	void finish();

	QLineEdit *template_edit;
	QComboBox *filter_combo, *pattern_combo, *template_combo;
	KURLRequester *dir_combo;
	QCheckBox *recursive_box;
	QListBox *resultbox;
	QPushButton *search_button, *clear_button, *cancel_button;
	KProcess *childproc;
	QString buf;
	QString errbuf;
	KConfig* config;
	QStringList lastSearchItems;
	QStringList lastSearchPaths;
	QStringList filter_list;
	QStringList template_list;

private slots:
	void templateActivated(int index);
	void childExited();
	void receivedOutput(KProcess *proc, char *buffer, int buflen);
	void receivedErrOutput(KProcess *proc, char *buffer, int buflen);
	void itemSelected(const QString&);
	void slotSearch();
	void slotClear();
	void slotCancel();
	void patternTextChanged( const QString &);
};

#endif // __KILE_GREP_DIALOG_H_
