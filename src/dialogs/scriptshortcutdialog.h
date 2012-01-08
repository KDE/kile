/******************************************************************************
  Copyright (C) 2011-2012 by Holger Danielsson (holger.danielsson@versanet.de)
 ******************************************************************************/


#ifndef SCRIPTSHORTCUTDIALOG_H
#define SCRIPTSHORTCUTDIALOG_H

#include <KDialog>

#include "kileinfo.h"
#include "ui_scriptshortcutdialog_base.h"


namespace KileDialog
{


class ScriptShortcutDialog : public KDialog
{
	Q_OBJECT

	public:
		ScriptShortcutDialog(QWidget *parent, KileInfo *ki, const QString &shortcut);
		~ScriptShortcutDialog() {}

		QString keySequence();

	private:
		Ui::ScriptShortcutDialog m_scriptShortcutDialog;

};

}

#endif
