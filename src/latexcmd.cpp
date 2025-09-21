/*****************************************************************************************
  Copyright (C) 2005 by Holger Danielsson (holger.danielsson@t-online.de)
                2010-2022 by Michel Ludwig (michel.ludwig@kdemail.net)
 ******************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "latexcmd.h"

#include <KConfigGroup>
// #include <KLocalizedString>

#include "kiledebug.h"

namespace KileDocument
{

// BEGIN LatexCommands

LatexCommands::LatexCommands(KConfig *config, KileInfo *info) : m_config(config), m_ki(info)
{
    m_envGroupName = QStringLiteral("Latex Environments");
    m_cmdGroupName = QStringLiteral("Latex Commands");

    LatexCommands::resetCommands();
}

void LatexCommands::resetCommands()
{
    // description of the fields for environments
    //  0: standard entry (+,-)
    //  1: environmenty type (a,m,l,t,v)
    //  2: including starred version (*)
    //  3: eol character (\\\\)
    //  4: need mathmode ($) or displaymathmode ($$)
    //  5: standard tabulator (tabulator string, f.e. &=& or &=)
    //  6: optional parameter
    //  7: parameter group(s)

    QStringList envlist;
    envlist
    // list environments
            << QStringLiteral("itemize,+,l,*,,,,,")
            << QStringLiteral("enumerate,+,l,*,,,,,")
            << QStringLiteral("description,+,l,*,,,,,")
            << QStringLiteral("Bitemize,+,l,,,,,,")
            << QStringLiteral("Benumerate,+,l,,,,,,")
            << QStringLiteral("Bdescription,+,l,,,,,,")
            << QStringLiteral("labeling,+,l,,,,,[ ],{ }")
            // tabular environments
            << QStringLiteral("tabular,+,t,*,\\\\,,&,[tcb],")
            << QStringLiteral("tabularx,+,t,,\\\\,,&,,{w}")
            << QStringLiteral("tabbing,+,t,,\\\\,,\\>,,")
            << QStringLiteral("longtable,+,t,,\\\\,,&,[tcb],")
            << QStringLiteral("ltxtable,+,t,,\\\\,,&,[tcb],{w}")
            << QStringLiteral("supertabular,+,t,*,\\\\,,&,,")
            << QStringLiteral("mpsupertabular,+,t,*,\\\\,,&,,")
            << QStringLiteral("xtabular,+,t,*,\\\\,,&,,")
            << QStringLiteral("mpxtabular,+,t,*,\\\\,,&,,")
            // math environments
            << QStringLiteral("displaymath,+,m,,,,,,")
            << QStringLiteral("equation,+,m,*,,,,,")
            << QStringLiteral("eqnarray,+,m,*,\\\\,,&=&,,")
            << QStringLiteral("array,+,m,,\\\\,$,&,[tcb],")
            << QStringLiteral("matrix,+,m,,\\\\,$,&,,")
            << QStringLiteral("pmatrix,+,m,,\\\\,$,&,,")
            << QStringLiteral("bmatrix,+,m,,\\\\,$,&,,")
            << QStringLiteral("Bmatrix,+,m,,\\\\,$,&,,")
            << QStringLiteral("vmatrix,+,m,,\\\\,$,&,,")
            << QStringLiteral("Vmatrix,+,m,,\\\\,$,&,,")
            // QStringLiteral(amsmath environments
            << QStringLiteral("multline,+,a,*,\\\\,,,,")
            << QStringLiteral("gather,+,a,*,\\\\,,,,")
            << QStringLiteral("split,+,a,,\\\\,$$,,,")          // needs surrounding environment
            << QStringLiteral("align,+,a,*,\\\\,,&=,,")
            << QStringLiteral("flalign,+,a,*,\\\\,,&=,,")
            << QStringLiteral("alignat,+,a,*,\\\\,,&=,,{n}")
            << QStringLiteral("aligned,+,a,,\\\\,$,&=,[tcb],")
            << QStringLiteral("gathered,+,a,,\\\\,$,,[tcb],")
            << QStringLiteral("alignedat,+,a,,\\\\,$,&=,[tcb],{n}")
            //<< "xalignat,+,a,*,\\\\,,&=,,{n}"   // obsolet
            //<< "xxalignat,+,a,*,\\\\,,&=,,{n}"  // obsolet
            << QStringLiteral("cases,+,a,,\\\\,$,&,,")
            // verbatim environments
            << QStringLiteral("verbatim,+,v,*,,,,,")
            << QStringLiteral("boxedverbatim,+,v,,,,,,")
            << QStringLiteral("Bverbatim,+,v,,,,,[ ],")
            << QStringLiteral("Lverbatim,+,v,,,,,[ ],")
            << QStringLiteral("lstlisting,+,v,,,,,[ ],")
            ;

    // description of the fields for commands
    //  0: standard entry (+,-)
    //  1: command type (L,R,C,I)
    //  2: including starred version (*)
    //  3: optional parameter
    //  4: parameter

    QStringList cmdlist;
    cmdlist
    // Labels
            << QStringLiteral("\\label,+,L,,,{ }")
            // References
            << QStringLiteral("\\ref,+,R,,,{ }")
            << QStringLiteral("\\pageref,+,R,,,{ }")
            << QStringLiteral("\\vref,+,R,,,{ }")
            << QStringLiteral("\\vpageref,+,R,,[ ],{ }")
            << QStringLiteral("\\fref,+,R,,,{ }")
            << QStringLiteral("\\Fref,+,R,,,{ }")
            << QStringLiteral("\\eqref,+,R,,,{ }")
            << QStringLiteral("\\autoref,+,R,,,{ }")
            // Bibliographies
            << QStringLiteral("\\bibliography,+,B,,,{ }")
            << QStringLiteral("\\addbibresource,+,B,*,[ ],{ }")
            << QStringLiteral("\\addglobalbib,+,B,*,[ ],{ }")
            << QStringLiteral("\\addsectionbib,+,B,*,[ ],{ }")
            // Citations
            << QStringLiteral("\\cite,+,C,,,{ }")
            // Includes
            << QStringLiteral("\\include,+,I,,,{ }")
            << QStringLiteral("\\input,+,I,,,{ }")
            << QStringLiteral("\\Input,+,I,,,{ }")
            ;

    // first clear the dictionary
    m_latexCommands.clear();

    // insert environments
    addUserCommands(m_envGroupName,envlist);
    insert(envlist);

    // insert commands
    addUserCommands(m_cmdGroupName,cmdlist);
    insert(cmdlist);
}


// add user-defined environments/commands

//FIXME: the code for reading and writing these configuration entries should be regrouped
//       within a single class (currently, the code for writing the values can be found
//       in 'latexcommanddialog.cpp').
void LatexCommands::addUserCommands(const QString &name, QStringList &list)
{
    KILE_DEBUG_MAIN << name;
    if(!m_config->hasGroup(name)) {
        KILE_DEBUG_MAIN << "\tGroup does not exist.";
        return;
    }

    KConfigGroup group = m_config->group(name);
    int nrOfDefinedCommands = group.readEntry("Number of Commands", 0);

    for(int i = 0; i < nrOfDefinedCommands; ++i) {
        const QString commandKey = QStringLiteral("Command") + QString::number(i);
        const QString parametersKey = QStringLiteral("Parameters") + QString::number(i);

        if(!group.hasKey(commandKey) || !group.hasKey(parametersKey)) {
            KILE_DEBUG_MAIN << "\tEntry" << i << "is invalid!";
        }
        const QString command = group.readEntry(commandKey);
        const QString parameters = group.readEntry(parametersKey);
        list << command + QStringLiteral(",-,") + parameters;
        KILE_DEBUG_MAIN << "\tAdding: " <<  command << " --> " << parameters;
    }
}

// insert all entries into the dictionary

void LatexCommands::insert(const QStringList &list)
{
    // now insert new entries, if they have the right number of attributes
    QStringList::ConstIterator it;
    for(it = list.begin(); it!=list.end(); ++it) {
        int pos = (*it).indexOf(QLatin1Char(','));
        if(pos >= 0)  {
            QString key = (*it).left(pos);
            QString value = (*it).right( (*it).length()-pos-1 );
            QStringList valuelist = value.split(QLatin1Char(','), Qt::KeepEmptyParts);
            int attributes = (key.at(0) == QLatin1Char('\\')) ? MaxCmdAttr : MaxEnvAttr;
            if(valuelist.count() == attributes) {
                m_latexCommands[key] = value;
            }
            else {
                KILE_DEBUG_MAIN << "\tLatexCommands error: wrong number of attributes (" << key << " ---> " << value << ")";
            }
        }
        else {
            KILE_DEBUG_MAIN << "\tLatexCommands error: no separator found (" << (*it) << ")" ;
        }
    }
}

//////////////////// get value from dictionary  ////////////////////

// Get value of a key. A star at the end is stripped.

QString LatexCommands::getValue(const QString &name)
{
    QString key = (name.indexOf(QLatin1Char('*'), -1) >= 0) ? name.left(name.length()-1) : name;
    return (m_latexCommands.contains(key)) ? m_latexCommands[key] : QString();
}

//////////////////// internal functions  ////////////////////

// get parameter at index

QString LatexCommands::getAttrAt(const QString &name, int index)
{
    if(index < 0) {
        return QString();
    }
    int attributes = (name.at(0) == QLatin1Char('\\')) ? MaxCmdAttr : MaxEnvAttr;
    QStringList list = getValue(name).split(QLatin1Char(','), Qt::KeepEmptyParts);
    return (index < attributes && list.count() == attributes) ? list[index] : QString();
}

// check for a standard environment

bool LatexCommands::isUserDefined(const QString &name)
{
    return (getValue(name).at(0) == QLatin1Char('-'));
}

// check for a special environment type

bool LatexCommands::isType(const QString &name, QChar ch)
{
    if(name.indexOf(QLatin1Char('*'), -1) >= 0) {
        QString envname = name.left(name.length() - 1);
        QString value = getValue(envname);
        return (value.length() >= 3 && value.at(2) == ch && isStarredEnv(envname));
    }
    else {
        QString value = getValue(name);
        return (value.length() >= 3 && value.at(2) == ch);
    }
}

//////////////////// attributes and characters ////////////////////

// convert attribute to character

QChar LatexCommands::getAttrChar(CmdAttribute attr)
{
    QChar ch;
    switch(attr) {
    case CmdAttrAmsmath:
        ch = QLatin1Char('a');
        break;
    case CmdAttrMath:
        ch = QLatin1Char('m');
        break;
    case CmdAttrList:
        ch = QLatin1Char('l');
        break;
    case CmdAttrVerbatim:
        ch = QLatin1Char('v');
        break;
    case CmdAttrTabular:
        ch = QLatin1Char('t');
        break;
    case CmdAttrLabel:
        ch = QLatin1Char('L');
        break;
    case CmdAttrReference:
        ch = QLatin1Char('R');
        break;
    case CmdAttrCitations:
        ch = QLatin1Char('C');
        break;
    case CmdAttrIncludes:
        ch = QLatin1Char('I');
        break;
    case CmdAttrBibliographies:
        ch = QLatin1Char('B');
        break;
    default:
        KILE_DEBUG_MAIN << "\tLatexCommands error: unknown type of env/cmd: code " << attr;
        return QLatin1Char('?');
    }

    return ch;
}

// convert character to attribute
CmdAttribute LatexCommands::getCharAttr(QChar ch)
{
    CmdAttribute attr;
    switch(ch.unicode()) {
    case 'a':
        attr = CmdAttrAmsmath;
        break;
    case 'm':
        attr = CmdAttrMath;
        break;
    case 'l':
        attr = CmdAttrList;
        break;
    case 'v':
        attr = CmdAttrVerbatim;
        break;
    case 't':
        attr = CmdAttrTabular;
        break;
    case 'L':
        attr = CmdAttrLabel;
        break;
    case 'R':
        attr = CmdAttrReference;
        break;
    case 'C':
        attr = CmdAttrCitations;
        break;
    case 'I':
        attr = CmdAttrIncludes;
        break;
    case 'B':
        attr = CmdAttrBibliographies;
        break;
    default:
        KILE_DEBUG_MAIN << "\tLatexCommands error: unknown type of env/cmd: " << static_cast<char>(ch.unicode());
        return CmdAttrNone;
    }

    return attr;
}

//////////////////// public attributes  ////////////////////

// check for environment types

bool LatexCommands::isMathEnv(const QString &name)
{
    QString value = getValue(name);
    if(value.length() < 3) {
        return false;
    }

    QChar ch = value.at(2);
    return (ch == QLatin1Char('m') || ch == QLatin1Char('a'));
}

// check for some special attributes

bool LatexCommands::isStarredEnv(const QString &name)
{
    return (getAttrAt(name, 2) == QStringLiteral("*"));
}

bool LatexCommands::isCrEnv(const QString &name)
{
    return (getAttrAt(name, 3) == QStringLiteral("\\\\"));
}

bool LatexCommands::isMathModeEnv(const QString &name)
{
    return (getAttrAt(name, 4) == QStringLiteral("$"));
}

bool LatexCommands::isDisplaymathModeEnv(const QString &name)
{
    return (getAttrAt(name, 4) == QStringLiteral("$$"));
}

bool LatexCommands::needsMathMode(const QString &name)
{
    return (isMathModeEnv(name) || isDisplaymathModeEnv(name));
}

QString LatexCommands::getTabulator(const QString &name)
{
    QString tab = getAttrAt(name, 5);
    return (tab.indexOf(QLatin1Char('&')) >= 0) ? tab : QString();
}

//////////////////// environments and commands ////////////////////

// get a list of environments and commands. The search can be restricted
// to given attributes and userdefined environments and commands

void LatexCommands::commandList(QStringList &list, uint attr, bool userdefined)
{
    list.clear();

    QMapIterator<QString,QString> it(m_latexCommands);
    while(it.hasNext()) {
        it.next();
        // first check, if we need really need all environments and commands
        // or if a restriction to some attributes is given
        if(attr != (uint)CmdAttrNone) {
            if(!(attr & (uint)getCharAttr( it.value().at(2)))) {
                continue;
            }
        }

        // second check, if we need only user-defined environments or commands
        if(!userdefined) {
            list.append(it.key());
        }
        else if(it.value().at(0) == QLatin1Char('-')) {
            list.append(it.key());
        }
    }
}

// get all attributes for a given environment and command

bool LatexCommands::commandAttributes(const QString &name, LatexCmdAttributes &attr)
{
    int attributes = (name.at(0) == QLatin1Char('\\')) ? MaxCmdAttr : MaxEnvAttr;

    // split attribute list
    QStringList list = getValue(name).split(QLatin1Char(','), Qt::KeepEmptyParts);

    // check number of attributes
    if(list.count() != attributes) {
        return false;
    }

    // check for a standard environment/command
    attr.standard = (list[0] == QStringLiteral("+"));

    // most important: type of environment or command
    attr.type = getCharAttr(list[1].at(0));
    if(attr.type == CmdAttrNone) {
        return false;
    }

    // all environments/commands have starred attribute
    attr.starred = (list[2] == QStringLiteral("*"));

    // next attributes differ for environments and commands
    if(attributes == MaxEnvAttr) {
        attr.cr = (list[3] == QStringLiteral("\\\\"));
        attr.mathmode = (list[4] == QStringLiteral("$"));
        attr.displaymathmode = (list[4] == QStringLiteral("$$"));
        attr.tabulator = list[5];
        attr.option = list[6];
        attr.parameter = list[7];
    }
    else {
        attr.cr = false;
        attr.mathmode = false;
        attr.displaymathmode = false;
        attr.tabulator.clear();
        attr.option = list[3];
        attr.parameter = list[4];
    }

    return true;
}

//////////////////// determine config string ////////////////////

QString LatexCommands::configString(const LatexCmdAttributes &attr,bool env)
{
    // most important: type of environment or command
    QChar ch = getAttrChar(attr.type);
    if(ch == QLatin1Char('?')) {
        return QString();
    }
    QString s = ch + QLatin1Char(',');

    // all environments/commands have starred attribute
    if(attr.starred) {
        s += QStringLiteral("*,");
    }
    else {
        s += QLatin1Char(',');
    }

    // next attributes are only valid for environments
    if(env) {
        if(attr.cr) {
            s += QStringLiteral("\\\\,");
        }
        else {
            s += QLatin1Char(',');
        }
        if(attr.mathmode) {
            s += QStringLiteral("$,");
        }
        else if(attr.displaymathmode) {
            s += QStringLiteral("$$");
        }
        else {
            s += QLatin1Char(',');
        }
        s += attr.tabulator + QLatin1Char(',');
    }

    // option and parameter are for both types again
    s += attr.option + QLatin1Char(',');
    s += attr.parameter;

    return s;    // s.left(s.length()-1);
}

// END LatexCommands

}
