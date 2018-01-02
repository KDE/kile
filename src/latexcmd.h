/***************************************************************************
                         latexcmd.h
                         ----------
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

// kommandos mit weiteren Parametern

#ifndef LATEXCMD_H
#define LATEXCMD_H

#include <QObject>
#include <QStringList>
#include <QMap>

#include <kconfig.h>

class KileInfo;

namespace KileDocument
{

const int MaxEnvAttr = 8;
const int MaxCmdAttr = 5;

enum CmdAttribute {
    CmdAttrNone = 0,                                                                                  // unknown
    CmdAttrAmsmath = 1, CmdAttrMath = 2, CmdAttrList = 4, CmdAttrTabular = 8, CmdAttrVerbatim = 16,   // environments
    CmdAttrLabel = 32, CmdAttrReference = 64, CmdAttrCitations = 128, CmdAttrIncludes = 256, // commands
    CmdAttrBibliographies = 512 //commands - continue
};

class LatexCmdAttributes
{
public:
    bool standard;
    CmdAttribute type;
    bool starred;
    bool cr;
    bool mathmode;
    bool displaymathmode;
    QString tabulator;
    QString option;
    QString parameter;
};

class LatexCommands : public QObject
{
    Q_OBJECT

public:
    LatexCommands(KConfig *config, KileInfo *info);
    ~LatexCommands() {};

    QString envGroupName() {
        return m_envGroupName;
    }
    QString cmdGroupName() {
        return m_cmdGroupName;
    }
    QString configString(LatexCmdAttributes &attr,bool env);

    bool isMathEnv(const QString &name);
    bool isListEnv(const QString &name) {
        return isType(name,'l');
    }
    bool isTabularEnv(const QString &name) {
        return isType(name,'t');
    }
    bool isVerbatimEnv(const QString &name) {
        return isType(name,'v');
    }

    bool isLabelCmd(const QString &name) {
        return isType(name,'L');
    }
    bool isReferenceCmd(const QString &name) {
        return isType(name,'R');
    }
    bool isCitationCmd(const QString &name) {
        return isType(name,'C');
    }
    bool isInputCmd(const QString &name) {
        return isType(name,'I');
    }

    bool isStarredEnv(const QString &name);
    bool isCrEnv(const QString &name);
    bool isMathModeEnv(const QString &name);
    bool isDisplaymathModeEnv(const QString &name);
    bool needsMathMode(const QString &name);
    QString getTabulator(const QString &name);

    void commandList(QStringList &list, uint attr, bool userdefined);
    bool commandAttributes(const QString &name, LatexCmdAttributes &attr);

    void resetCommands();

private:

    KConfig *m_config;
    KileInfo	*m_ki;

    QString m_envGroupName, m_cmdGroupName;
    QMap<QString,QString> m_latexCommands;

    void addUserCommands(const QString &name, QStringList &list);
    void insert(const QStringList &list);

    QString getValue(const QString &name);


    bool isUserDefined(const QString &name);
    bool isType(const QString &name, QChar ch);
    QString getAttrAt(const QString &name, int index);
    QChar getAttrChar(CmdAttribute attr);
    CmdAttribute getCharAttr(QChar ch);

};


}

#endif
