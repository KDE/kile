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
    m_envGroupName = "Latex Environments";
    m_cmdGroupName = "Latex Commands";

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
            << "itemize,+,l,*,,,,,"
            << "enumerate,+,l,*,,,,,"
            << "description,+,l,*,,,,,"
            << "Bitemize,+,l,,,,,,"
            << "Benumerate,+,l,,,,,,"
            << "Bdescription,+,l,,,,,,"
            << "labeling,+,l,,,,,[ ],{ }"
            // tabular environments
            << "tabular,+,t,*,\\\\,,&,[tcb],"
            << "tabularx,+,t,,\\\\,,&,,{w}"
            << "tabbing,+,t,,\\\\,,\\>,,"
            << "longtable,+,t,,\\\\,,&,[tcb],"
            << "ltxtable,+,t,,\\\\,,&,[tcb],{w}"
            << "supertabular,+,t,*,\\\\,,&,,"
            << "mpsupertabular,+,t,*,\\\\,,&,,"
            << "xtabular,+,t,*,\\\\,,&,,"
            << "mpxtabular,+,t,*,\\\\,,&,,"
            // math environments
            << "displaymath,+,m,,,,,,"
            << "equation,+,m,*,,,,,"
            << "eqnarray,+,m,*,\\\\,,&=&,,"
            << "array,+,m,,\\\\,$,&,[tcb],"
            << "matrix,+,m,,\\\\,$,&,,"
            << "pmatrix,+,m,,\\\\,$,&,,"
            << "bmatrix,+,m,,\\\\,$,&,,"
            << "Bmatrix,+,m,,\\\\,$,&,,"
            << "vmatrix,+,m,,\\\\,$,&,,"
            << "Vmatrix,+,m,,\\\\,$,&,,"
            // amsmath environments
            << "multline,+,a,*,\\\\,,,,"
            << "gather,+,a,*,\\\\,,,,"
            << "split,+,a,,\\\\,$$,,,"          // needs surrounding environment
            << "align,+,a,*,\\\\,,&=,,"
            << "flalign,+,a,*,\\\\,,&=,,"
            << "alignat,+,a,*,\\\\,,&=,,{n}"
            << "aligned,+,a,,\\\\,$,&=,[tcb],"
            << "gathered,+,a,,\\\\,$,,[tcb],"
            << "alignedat,+,a,,\\\\,$,&=,[tcb],{n}"
            //<< "xalignat,+,a,*,\\\\,,&=,,{n}"   // obsolet
            //<< "xxalignat,+,a,*,\\\\,,&=,,{n}"  // obsolet
            << "cases,+,a,,\\\\,$,&,,"
            // verbatim environments
            << "verbatim,+,v,*,,,,,"
            << "boxedverbatim,+,v,,,,,,"
            << "Bverbatim,+,v,,,,,[ ],"
            << "Lverbatim,+,v,,,,,[ ],"
            << "lstlisting,+,v,,,,,[ ],"
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
            << "\\label,+,L,,,{ }"
            // References
            << "\\ref,+,R,,,{ }"
            << "\\pageref,+,R,,,{ }"
            << "\\vref,+,R,,,{ }"
            << "\\vpageref,+,R,,[ ],{ }"
            << "\\fref,+,R,,,{ }"
            << "\\Fref,+,R,,,{ }"
            << "\\eqref,+,R,,,{ }"
            << "\\autoref,+,R,,,{ }"
            // Bibliographies
            << "\\bibliography,+,B,,,{ }"
            << "\\addbibresource,+,B,*,[ ],{ }"
            << "\\addglobalbib,+,B,*,[ ],{ }"
            << "\\addsectionbib,+,B,*,[ ],{ }"
            // Citations
            << "\\cite,+,C,,,{ }"
            // Includes
            << "\\include,+,I,,,{ }"
            << "\\input,+,I,,,{ }"
            << "\\Input,+,I,,,{ }"
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
        const QString commandKey = "Command" + QString::number(i);
        const QString parametersKey = "Parameters" + QString::number(i);

        if(!group.hasKey(commandKey) || !group.hasKey(parametersKey)) {
            KILE_DEBUG_MAIN << "\tEntry" << i << "is invalid!";
        }
        const QString command = group.readEntry(commandKey);
        const QString parameters = group.readEntry(parametersKey);
        list << command + ",-," + parameters;
        KILE_DEBUG_MAIN << "\tAdding: " <<  command + " --> " + parameters;
    }
}

// insert all entries into the dictionary

void LatexCommands::insert(const QStringList &list)
{
    // now insert new entries, if they have the right number of attributes
    QStringList::ConstIterator it;
    for(it = list.begin(); it!=list.end(); ++it) {
        int pos = (*it).indexOf(',');
        if(pos >= 0)  {
            QString key = (*it).left(pos);
            QString value = (*it).right( (*it).length()-pos-1 );
            QStringList valuelist = value.split(',', Qt::KeepEmptyParts);
            int attributes = ( key.at(0)=='\\' ) ? MaxCmdAttr : MaxEnvAttr;
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
    QString key = (name.indexOf('*',-1) >= 0) ? name.left(name.length()-1) : name;
    return (m_latexCommands.contains(key)) ? m_latexCommands[key] : QString();
}

//////////////////// internal functions  ////////////////////

// get parameter at index

QString LatexCommands::getAttrAt(const QString &name, int index)
{
    if(index < 0) {
        return QString();
    }
    int attributes = (name.at(0) == '\\') ? MaxCmdAttr : MaxEnvAttr;
    QStringList list = getValue(name).split(',', Qt::KeepEmptyParts);
    return (index < attributes && list.count() == attributes) ? list[index] : QString();
}

// check for a standard environment

bool LatexCommands::isUserDefined(const QString &name)
{
    return ( getValue(name).at(0) == '-' );
}

// check for a special environment type

bool LatexCommands::isType(const QString &name, QChar ch)
{
    if(name.indexOf('*', -1) >= 0) {
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
        ch = 'a';
        break;
    case CmdAttrMath:
        ch = 'm';
        break;
    case CmdAttrList:
        ch = 'l';
        break;
    case CmdAttrVerbatim:
        ch = 'v';
        break;
    case CmdAttrTabular:
        ch = 't';
        break;
    case CmdAttrLabel:
        ch = 'L';
        break;
    case CmdAttrReference:
        ch = 'R';
        break;
    case CmdAttrCitations:
        ch = 'C';
        break;
    case CmdAttrIncludes:
        ch = 'I';
        break;
    case CmdAttrBibliographies:
        ch = 'B';
        break;
    default:
        KILE_DEBUG_MAIN << "\tLatexCommands error: unknown type of env/cmd: code " << attr;
        return '?';
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
    return (ch == 'm' || ch == 'a');
}

// check for some special attributes

bool LatexCommands::isStarredEnv(const QString &name)
{
    return (getAttrAt(name, 2) == "*");
}

bool LatexCommands::isCrEnv(const QString &name)
{
    return (getAttrAt(name, 3) == "\\\\");
}

bool LatexCommands::isMathModeEnv(const QString &name)
{
    return (getAttrAt(name, 4) == "$");
}

bool LatexCommands::isDisplaymathModeEnv(const QString &name)
{
    return (getAttrAt(name, 4) == "$$");
}

bool LatexCommands::needsMathMode(const QString &name)
{
    return (isMathModeEnv(name) || isDisplaymathModeEnv(name));
}

QString LatexCommands::getTabulator(const QString &name)
{
    QString tab = getAttrAt(name, 5);
    return (tab.indexOf('&') >= 0) ? tab : QString();
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
        else if(it.value().at(0) == '-') {
            list.append(it.key());
        }
    }
}

// get all attributes for a given environment and command

bool LatexCommands::commandAttributes(const QString &name, LatexCmdAttributes &attr)
{
    int attributes = (name.at(0) == '\\') ? MaxCmdAttr : MaxEnvAttr;

    // split attribute list
    QStringList list = getValue(name).split(',', Qt::KeepEmptyParts);

    // check number of attributes
    if(list.count() != attributes) {
        return false;
    }

    // check for a standard environment/command
    attr.standard = (list[0] == "+");

    // most important: type of environment or command
    attr.type = getCharAttr(list[1].at(0));
    if(attr.type == CmdAttrNone) {
        return false;
    }

    // all environments/commands have starred attribute
    attr.starred = (list[2] == "*");

    // next attributes differ for environments and commands
    if(attributes == MaxEnvAttr) {
        attr.cr = (list[3] == "\\\\");
        attr.mathmode = (list[4] == "$");
        attr.displaymathmode = (list[4] == "$$");
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

QString LatexCommands::configString(LatexCmdAttributes &attr,bool env)
{
    // most important: type of environment or command
    QChar ch = getAttrChar(attr.type);
    if(ch == '?') {
        return QString();
    }
    QString s = ch + QString(',');

    // all environments/commands have starred attribute
    if(attr.starred) {
        s += "*,";
    }
    else {
        s += ',';
    }

    // next attributes are only valid for environments
    if(env) {
        if(attr.cr) {
            s += "\\\\,";
        }
        else {
            s += ',';
        }
        if(attr.mathmode) {
            s += "$,";
        }
        else if(attr.displaymathmode) {
            s += "$$";
        }
        else {
            s += ',';
        }
        s += attr.tabulator + ',';
    }

    // option and parameter are for both types again
    s += attr.option + ',';
    s += attr.parameter;

    return s;    // s.left(s.length()-1);
}

// END LatexCommands

}
