/***************************************************************************
                          toolsoptionsdialog.h  -  description
                             -------------------
    begin                : Wed Jun 6 2001
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

#ifndef TOOLSOPTIONSDIALOG_H
#define TOOLSOPTIONSDIALOG_H

/**
  *@author Brachet Pascal
  */

#include <kdialogbase.h>
class QLabel;
class QLineEdit;

class QLabel;
class QComboBox;
class QSpinBox;

class QCheckBox;

class QFrame;
class KSpellConfig;
class KColorButton;


typedef  QColor ListColors[8];

class toolsoptionsdialog : public KDialogBase
{
    Q_OBJECT

public:
    toolsoptionsdialog( QWidget* parent = 0, const char* name = 0);
    ~toolsoptionsdialog();

    QLabel* TextLabel1, * TextLabel2,* TextLabel3;


    QLabel* TextLabel6;
    QLineEdit* LineEdit6;

    QLabel* TextLabel7;
    QLineEdit* LineEdit7;

    QLabel* TextLabel4;

    QComboBox *comboFamily, *comboDvi, *comboPs, *comboPdf, *comboColor;

    QLabel* TextLabel5;
    QSpinBox *spinSize;
    QLabel* TextLabel8;

    QCheckBox *checkLine, *checkWordWrap, *checkLatex, *checkPdflatex, *checkParen, *checkDviSearch, *checkDvi, *checkDviPdf, *checkPsPdf;

    QFrame* toolsPage;
    QFrame* editorPage;
    KSpellConfig *ksc;
    KColorButton* buttonColor;
    ListColors colors;

public slots:
    void init();

private:
    int previous_index;

private slots:
    void slotChangeColor(int index);
    void slotEnd();
};
#endif
