/***************************************************************************
                          letterdialog.cpp  -  description
                             -------------------
    begin                : Tue Oct 30 2001
    copyright            : (C) 2001 by Brachet Pascal
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "letterdialog.h"

#include <qcheckbox.h>
#include <qlayout.h>
#include <qlabel.h>

#include <klocale.h>
#include <klineedit.h>
#include <kpushbutton.h>

namespace KileDialog
{
	QuickLetter::QuickLetter(KConfig *config, QWidget *parent, const char *name, const QString &caption) : 
		QuickDocument(config, parent, name, caption)
	{
		m_layout->remove(m_cbDocClass); m_cbDocClass->hide();
		m_layout->remove(m_lbDocClass); m_lbDocClass->hide();
		m_layout->remove(m_leTitle); m_leTitle->hide();
		m_layout->remove(m_lbTitle); m_lbTitle->hide();
		m_layout->remove(m_leAuthor); m_leAuthor->hide();
		m_layout->remove(m_lbAuthor); m_lbAuthor->hide();
		m_layout->remove(m_ckIdx); m_ckIdx->hide();
		m_layout->remove(m_bxOptions); m_bxOptions->hide();
		m_layout->remove(m_lbOptions); m_lbOptions->hide();
		m_layout->remove(userClassBtn); userClassBtn->hide();
		m_layout->remove(userOptionsBtn); userOptionsBtn->hide();
		this->resize(300,150);
	}
	
	QuickLetter::~QuickLetter()
	{}

	void QuickLetter::slotOk()
	{
		m_td.dy = 7;
		m_td.tagBegin = "\\documentclass[" + m_cbPaperSize->currentText()+",";
		m_td.tagBegin += m_cbFontSize->currentText()+"]{letter}\n";

		if ( m_cbEncoding->currentText() != "NONE")
		{
			m_td.tagBegin += "\\usepackage["+ m_cbEncoding->currentText() + "]{inputenc}\n";
			m_td.dy++;
		}

		if ( m_ckAMS->isChecked())
		{
			m_td.tagBegin +=  "\\usepackage{amsmath}\n\\usepackage{amsfonts}\n\\usepackage{amssymb}\n";
			m_td.dy += 3;
		}

		m_td.tagBegin += "\\address{your name and address} \n\\signature{your signature} \n";
		m_td.tagBegin += "\\begin{document} \n\\begin{letter}{name and address of the recipient} \n";
		m_td.tagBegin += "\\opening{saying hello} \n \nwrite your letter here \n \n";
		m_td.tagBegin += "\\closing{saying goodbye} \n%\\cc{Cclist} \n";
		m_td.tagBegin += "%\\ps{adding a postscript} \n%\\encl{list of enclosed material} \n";
		m_td.tagEnd = "\n\\end{letter}\n\\end{document}";

		accept();
	}
}

#include "letterdialog.moc"
