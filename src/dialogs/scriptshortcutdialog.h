/******************************************************************************
  Copyright (C) 2011-2012 by Holger Danielsson (holger.danielsson@versanet.de)
 ******************************************************************************/


#ifndef SCRIPTSHORTCUTDIALOG_H
#define SCRIPTSHORTCUTDIALOG_H

#include <QDialog>

#include "kileinfo.h"
#include "scripting/script.h"
#include "ui_scriptshortcutdialog_base.h"


namespace KileDialog
{


class ScriptShortcutDialog : public QDialog
{
    Q_OBJECT

public:
    ScriptShortcutDialog(QWidget *parent, KileInfo *ki, int type, const QString &sequence);
    ~ScriptShortcutDialog() {}

    int sequenceType();
    QString sequenceValue();

private Q_SLOTS:
    void slotUpdate();

private:
    Ui::ScriptShortcutDialog m_scriptShortcutDialog;

};

}

#endif
