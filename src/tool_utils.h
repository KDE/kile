/***************************************************************************
  Copyright (C) 2012 by Michel Ludwig (michel.ludwig@kdemail.net)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TOOL_UTILS_H
#define TOOL_UTILS_H

#include <QMetaType>
#include <QPair>
#include <QString>

#define DEFAULT_TOOL_CONFIGURATION "Default"

namespace KileTool
{

class ToolConfigPair : public QPair<QString, QString>
{
public:
    ToolConfigPair();
    ToolConfigPair(const QString& toolName, const QString& configName);

    inline bool isValid() const
    {
        return !first.isEmpty();
    }

    /** If the first components are equal, we compare the second one but
     * an empty config name or the default tool config name should precede all others.
     **/
    bool operator<(const ToolConfigPair& p2) const;

    static QString userStringRepresentation(const QString& toolName, const QString& toolConfig);
    inline QString userStringRepresentation() const
    {
        return userStringRepresentation(first, second);
    }

    static QString configStringRepresentation(const QString& toolName, const QString& toolConfig);
    QString configStringRepresentation() const
    {
        return configStringRepresentation(first, second);
    }

    static ToolConfigPair fromConfigStringRepresentation(const QString& s);
};

}

Q_DECLARE_METATYPE(KileTool::ToolConfigPair)

#endif
