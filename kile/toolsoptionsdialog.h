/***************************************************************************
                          toolsoptionsdialog.h  -  description
                             -------------------
    begin                : Wed Jun 6 2001
    copyright            : (C) 2003 by Jeroen Wijnout
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

#ifndef TOOLSOPTIONSDIALOG_H
#define TOOLSOPTIONSDIALOG_H

#include <kdialogbase.h>
#include <qvalidator.h>

class QLabel;
class QLineEdit;

class QLabel;
class QComboBox;
class QSpinBox;

class QCheckBox;
class QRadioButton;

class QFrame;
class KSpellConfig;
class KColorButton;
class KIntNumInput;

//typedef  QColor ListColors[8];


class toolsoptionsdialog : public KDialogBase
{
    Q_OBJECT

public:
    toolsoptionsdialog( QWidget* parent = 0, const char* name = 0);
    ~toolsoptionsdialog();

    QLabel* TextLabel1, * TextLabel2,* TextLabel3;

    QLabel    *TextLabel6, *TextLabel7, *TextLabel9, *TextLabel10, *TextLabel11, *TextLabel12, *TextLabel13;
    QLineEdit *LineEdit6,  *LineEdit7,  *LineEdit9,  *LineEdit10,  *LineEdit11, *LineEdit12, *LineEdit13;

    QComboBox *comboDvi, *comboPs, *comboPdf;

    QCheckBox *checkAutosave;

    QRadioButton *checkLatex, *checkPdflatex,
        *checkDviSearch, *checkDvi, *checkDviPdf, *checkPsPdf;

    KIntNumInput *asIntervalInput;

    QLineEdit *templAuthor, *templDocClassOpt, *templEncoding;

    QFrame* generalPage;
    QFrame* toolsPage;
    QFrame* quickPage;
    QFrame* spellingPage;
    KSpellConfig *ksc;
};
#endif
