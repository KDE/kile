/***************************************************************************
                          kileprojectdlgs.h -  description
                             -------------------
    begin                : Sun Aug 3 2003
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

#ifndef KILEPROJECTDLGS_H
#define KILEPROJECTDLGS_H

#include <kdialogbase.h>

class NewFileWidget;
class QCheckBox;
class QLabel;
class KLineEdit;

class KileNewProjectDlg : public KDialogBase
{
	Q_OBJECT

public:
	KileNewProjectDlg(QWidget* parent = 0, const char* name = 0);
	~KileNewProjectDlg();

	QString name() {return m_name->text();}
	QString location() { return m_location->text(); }
	TemplateItem* getSelection()const { return static_cast<TemplateItem*>(m_nfw->currentItem());}
	QString file() { return m_file->text();}
	bool createNewFile() { return m_cb->isChecked(); }

public slots:
	void clickedCreateNewFileCb();
	void browseLocation();

	void slotOk();

private:
	KLineEdit	*m_name, *m_location, *m_file;
	NewFileWidget *m_nfw;
	QCheckBox	*m_cb;
	QLabel *m_lb;
};

#endif
