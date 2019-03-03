/********************************************************************************
*   Copyright (C) 2007 by Holger Danielsson (holger.danielsson@versanet.de)     *
*                 2008 - 2010 by Michel Ludwig (michel.ludwig@kdemail.net)      *
*********************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "abbreviationmanager.h"

#include <KMessageBox>
#include "codecompletion.h"
#include "kileinfo.h"
#include "utilities.h"

namespace KileAbbreviation {

Manager::Manager(KileInfo* kileInfo, QObject *parent) : QObject(parent), m_kileInfo(kileInfo), m_abbreviationsDirty(false)
{
    setObjectName("KileAbbreviation::Manager");
    m_localAbbreviationFile = KileUtilities::writableLocation(QStandardPaths::AppDataLocation) + '/' + "complete/abbreviation/" + "kile-abbrevs.cwl";
    QDir testDir(m_localAbbreviationFile);
    if (!testDir.exists()) {
        testDir.mkpath(m_localAbbreviationFile);
    }
}

Manager::~Manager()
{
}

const AbbreviationMap& Manager::getAbbreviationMap()
{
    return m_abbreviationMap;
}

void Manager::updateLocalAbbreviation(const QString& text, const QString& replacement)
{
    if(text.isEmpty() || replacement.isEmpty()) {
        return;
    }
    AbbreviationMap::iterator it = m_abbreviationMap.find(text);
    if(it != m_abbreviationMap.end()) {
        StringBooleanPair pair = it.value();
        if(pair.first == replacement) {
            return;
        }
        m_abbreviationMap.erase(it);
    }
    m_abbreviationMap[text] = createLocalAbbreviationPair(replacement);
    m_abbreviationsDirty = true;
    emit(abbreviationsChanged());
}

void Manager::removeLocalAbbreviation(const QString& text)
{
    AbbreviationMap::iterator it = m_abbreviationMap.find(text);
    if(it == m_abbreviationMap.end()) {
        return;
    }
    StringBooleanPair pair = it.value();
    if(isLocalAbbreviation(pair)) {
        m_abbreviationMap.erase(it);
        m_abbreviationsDirty = true;
    }
    emit(abbreviationsChanged());
}

void Manager::readAbbreviationFiles()
{
    if(m_abbreviationsDirty) {
        saveLocalAbbreviations();
    }
    m_abbreviationMap.clear();
    QStringList list = m_kileInfo->codeCompletionManager()->readCWLFiles(KileConfig::completeAbbrev(), "abbreviation");
    addAbbreviationListToMap(list, true);

    // read local wordlist
    list = m_kileInfo->codeCompletionManager()->readCWLFile(m_localAbbreviationFile, true);
    addAbbreviationListToMap(list, false);

    emit(abbreviationsChanged());
}

void Manager::saveLocalAbbreviations()
{
    if(!m_abbreviationsDirty) {
        return;
    }

    KILE_DEBUG_MAIN;
    // create the file
    QFile abbreviationFile(m_localAbbreviationFile);
    if(!abbreviationFile.open(QIODevice::WriteOnly)) {
        KMessageBox::error(m_kileInfo->mainWindow(), i18n("Could not save the local abbreviation list.\nError code %1.", QString::number(abbreviationFile.error())),
                           i18n("Saving Problem"));
        return;
    }

    QTextStream stream(&abbreviationFile);
    stream << "# abbreviation mode: editable abbreviations\n";

    //QTextCodec *codec = QTextCodec::codecForName(m_ki->activeTextDocument()->encoding().ascii());
    // stream.setCodec(codec);

    for(AbbreviationMap::iterator i = m_abbreviationMap.begin();
            i != m_abbreviationMap.end(); ++i) {
        StringBooleanPair pair = i.value();
        if(!pair.second) {
            stream << QString(i.key()).replace('=', "\\=")
                   << '=' << pair.first << '\n';
        }
    }
    abbreviationFile.close();

    m_abbreviationsDirty = false;
}

void Manager::addAbbreviationListToMap(const QStringList& list, bool global)
{
    // a '=' symbol in the left-hand side is encoded by '\='
    for(QStringList::const_iterator i = list.begin(); i != list.end(); ++i) {
        QString entry = *i;
        int delimiter = entry.indexOf(QRegExp("[^\\\\]="));
        if(delimiter < 0) {
            continue;
        }
        QString left = entry.left(delimiter + 1); // [^\\\\]= has length 2.
        left.replace("\\=", "=");
        QString right = entry.mid(delimiter + 2); // [^\\\\]= has length 2.
        if(right.isEmpty()) {
            continue;
        }
        m_abbreviationMap[left] = StringBooleanPair(right, global);
    }
}

QStringList Manager::getAbbreviationTextMatches(const QString& text) const
{
    QStringList toReturn;
    for(AbbreviationMap::const_iterator i = m_abbreviationMap.begin();
            i != m_abbreviationMap.end(); ++i) {
        if(i.key().startsWith(text)) {
            toReturn.append(i.value().first);
        }
    }
    return toReturn;
}

QString Manager::getAbbreviationTextMatch(const QString& text) const
{
    return m_abbreviationMap[text].first;
}

bool Manager::abbreviationStartsWith(const QString& text) const
{
    for(AbbreviationMap::const_iterator i = m_abbreviationMap.begin();
            i != m_abbreviationMap.end(); ++i) {
        if(i.key().startsWith(text)) {
            return true;
        }
    }
    return false;
}

bool Manager::isAbbreviationDefined(const QString& text) const
{
    return m_abbreviationMap.find(text) != m_abbreviationMap.end();
}


}

