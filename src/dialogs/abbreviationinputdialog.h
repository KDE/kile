/*************************************************************************************
    begin                : Feb 24 2007
    copyright            : 2007 by Holger Danielsson (holger.danielsson@versanet.de)
 *************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ABBREVIATIONINPUTDIALOG_H
#define ABBREVIATIONINPUTDIALOG_H

#include <QLabel>
#include <QDialog>
#include <QString>
#include <QTreeWidgetItem>

#include "widgets/abbreviationview.h"

class QDialogButtonBox;
class QLineEdit;

//////////////////// add/edit dialog for abbreviations ////////////////////

namespace KileDialog {

class AbbreviationInputDialog : public QDialog
{
    Q_OBJECT

public:
    AbbreviationInputDialog(KileWidget::AbbreviationView *listview, QTreeWidgetItem *item, int mode, const char *name = Q_NULLPTR);
    ~AbbreviationInputDialog();
    void abbreviation(QString &abbrev, QString &expansion);

private Q_SLOTS:
    void onTextChanged(const QString &text);

private:
    KileWidget::AbbreviationView *m_listview;
    QDialogButtonBox *m_buttonBox;
    QTreeWidgetItem *m_abbrevItem;
    QLineEdit *m_leAbbrev;
    QLineEdit *m_leExpansion;
    int m_mode;
    QString m_abbrev, m_expansion;
};

}

#endif
