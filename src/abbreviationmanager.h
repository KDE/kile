/**************************************************************************
*   Copyright (C) 2009-2010 by Michel Ludwig (michel.ludwig@kdemail.net)  *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef ABBREVIATIONMANAGER_H
#define ABBREVIATIONMANAGER_H

#include <QMap>
#include <QObject>
#include <QPair>
#include <QString>

#include <KConfig>

class KileInfo;

namespace KileAbbreviation {

typedef QPair<QString, bool> StringBooleanPair;
typedef QMap<QString, StringBooleanPair> AbbreviationMap;

/**
 * This manager class is responsible for handling abbreviations.
 **/
class Manager : public QObject {
    Q_OBJECT

public:
    /**
     * Constructs a new manager object.
     **/
    explicit Manager(KileInfo* kileInfo, QObject *parent = Q_NULLPTR);
    virtual ~Manager();

    // the boolean value is 'true' iff the abbreviation is global
    const AbbreviationMap& getAbbreviationMap();

    void readAbbreviationFiles();
    void saveLocalAbbreviations();

    void updateLocalAbbreviation(const QString& text, const QString& replacement);
    void removeLocalAbbreviation(const QString& text);

    static inline bool isLocalAbbreviation(const StringBooleanPair& p)
    {
        return !p.second;
    }

    static inline bool isGlobalAbbreviation(const StringBooleanPair& p)
    {
        return p.second;
    }

    static inline StringBooleanPair createGlobalAbbreviationPair(const QString& s)
    {
        return StringBooleanPair(s, true);
    }

    static inline StringBooleanPair createLocalAbbreviationPair(const QString& s)
    {
        return StringBooleanPair(s, false);
    }

    /**
     * Returns the replacement strings of those strings that start with 'text'.
     **/
    QStringList getAbbreviationTextMatches(const QString& text) const;

    /**
     * Returns the replacement string for 'text'; an empty string is returned
     * is 'text' is not found
     **/
    QString getAbbreviationTextMatch(const QString& text) const;

    /**
     * Returns true iff there exists an abbreviation which starts with 'text'.
     **/
    bool abbreviationStartsWith(const QString& text) const;

    bool isAbbreviationDefined(const QString& text) const;

Q_SIGNALS:
    void abbreviationsChanged();

protected:
    KileInfo *m_kileInfo;
    bool m_abbreviationsDirty;
    QString m_localAbbreviationFile;
    AbbreviationMap m_abbreviationMap;

    void addAbbreviationListToMap(const QStringList& list, bool global);
};


}

#endif
