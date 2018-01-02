/***************************************************************************
                         latexcmddialog.h
                         --------------
    date                 : Jul 25 2005
    version              : 0.20
    copyright            : (C) 2005 by Holger Danielsson
    email                : holger.danielsson@t-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef LATEXCOMMANDDIALOG_H
#define LATEXCOMMANDDIALOG_H

#include <QDialog>

#include <QMap>

#include "latexcmd.h"
#include "ui_latexcommanddialog_base.h"

class KComboBox;
class KConfig;
class QCheckBox;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QTabWidget;
class QTreeWidget;
class QTreeWidgetItem;

namespace KileDialog
{

class NewLatexCommand : public QDialog
{
    Q_OBJECT

public:
    NewLatexCommand(QWidget *parent, const QString &caption,
                    const QString &groupname, QTreeWidgetItem *lvitem,
                    KileDocument::CmdAttribute cmdtype, QMap<QString, bool> *dict);
    ~NewLatexCommand() {}
    void getParameter(QString &name, KileDocument::LatexCmdAttributes &attr);

private:
    QLineEdit *m_edName;
    QCheckBox *m_chStarred, *m_chEndofline, *m_chMath;
    KComboBox *m_coTab, *m_coOption, *m_coParameter;

    bool m_addmode, m_envmode;
    bool m_useMathOrTab, m_useOption, m_useParameter;
    KileDocument::CmdAttribute m_cmdType;
    QMap<QString, bool> *m_dict;

private Q_SLOTS:
    virtual void slotAccepted();
};


class LatexCommandsDialog : public QDialog
{
    Q_OBJECT

public:
    LatexCommandsDialog(KConfig *config, KileDocument::LatexCommands *commands, QWidget *parent = 0);
    ~LatexCommandsDialog() {}

    //enum EnvParameter { envName,envStarred,envEOL,envMath,envTab,envOption };

private:
    enum LVmode { lvEnvMode = 1, lvCmdMode = 2 };

    KConfig *m_config;
    KileDocument::LatexCommands *m_commands;
    QMap<QString, bool> m_dictCommands;
    bool m_commandChanged;

    //QTreeWidget *m_lvEnvironments, *m_lvCommands;
    QTreeWidgetItem *m_lviList, *m_lviTabular, *m_lviMath, *m_lviAmsmath, *m_lviVerbatim;
    QTreeWidgetItem *m_lviLabels, *m_lviReferences, *m_lviBibliographies, *m_lviCitations;
    QTreeWidgetItem *m_lviInputs;
    /*QTabWidget *m_tab;
    QPushButton *m_btnAdd, *m_btnDelete, *m_btnEdit;
    QCheckBox *m_cbUserDefined;*/
    QDialogButtonBox *m_buttonBox;
    Ui::LatexCommandWidget m_widget;

    void resetListviews();
    LVmode getListviewMode();
    KileDocument::CmdAttribute getCommandMode(QTreeWidgetItem *item);
    bool isParentItem(QTreeWidgetItem *item);

    void setEntry(QTreeWidgetItem *parent, const QString &name, KileDocument::LatexCmdAttributes &attr);
    void getEntry(QTreeWidgetItem *item, KileDocument::LatexCmdAttributes &attr);

    bool isUserDefined(const QString &name);
    bool hasUserDefined(QTreeWidget *listview);

    void resetEnvironments();
    void resetCommands();
    void getListviewStates(bool states[]);
    void setListviewStates(bool states[]);

    void readConfig();
    void writeConfig(QTreeWidget *listview, const QString &groupname, bool env);

private Q_SLOTS:
    void slotEnableButtons();
    void slotAddClicked();
    void slotDeleteClicked();
    void slotEditClicked();
    void slotUserDefinedClicked();
    void slotAccepted();
    void slotSetDefaults();
};

}

#endif
