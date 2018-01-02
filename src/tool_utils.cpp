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

#include "tool_utils.h"

#include <KLocalizedString>

namespace KileTool {

ToolConfigPair::ToolConfigPair()
    : QPair<QString, QString>()
{
}

ToolConfigPair::ToolConfigPair(const QString& toolName, const QString& configName)
    : QPair<QString, QString>(toolName, configName)
{
}

bool ToolConfigPair::operator<(const ToolConfigPair& p2) const
{
    const int firstCompare = first.localeAwareCompare(p2.first);

    if(firstCompare != 0) {
        return (firstCompare < 0);
    }

    if(second.isEmpty() || second == DEFAULT_TOOL_CONFIGURATION) {
        if(p2.second.isEmpty() || p2.second == DEFAULT_TOOL_CONFIGURATION) {
            return false;
        }
        else {
            return true;
        }
    }
    if(p2.second.isEmpty() || p2.second == DEFAULT_TOOL_CONFIGURATION) {
        if(second.isEmpty() || second == DEFAULT_TOOL_CONFIGURATION) {
            return true;
        }
        else {
            return false;
        }
    }

    return (second.localeAwareCompare(p2.second) < 0);
}

QString ToolConfigPair::userStringRepresentation(const QString& toolName, const QString& toolConfig)
{
    return (toolConfig == DEFAULT_TOOL_CONFIGURATION)
           ? toolName : i18nc("<tool name> - <configuration>", "%1 - %2", toolName, toolConfig);
}

QString ToolConfigPair::configStringRepresentation(const QString& toolName, const QString& toolConfig)
{
    QString configString = toolConfig;
    if(configString == DEFAULT_TOOL_CONFIGURATION) {
        configString.clear();
    }
    if(toolName.isEmpty() && configString.isEmpty()) {
        return "";
    }
    if(configString.isEmpty()) {
        return toolName;
    }
    return toolName + '/' + configString;
}

ToolConfigPair ToolConfigPair::fromConfigStringRepresentation(const QString& s)
{
    const int separatorIndex = s.indexOf('/');
    if(separatorIndex < 0) { // for example, is 's' is empty
        return ToolConfigPair(s, DEFAULT_TOOL_CONFIGURATION);
    }
    QString configString = s.mid(separatorIndex + 1);
    if(configString.isEmpty()) {
        configString = DEFAULT_TOOL_CONFIGURATION;
    }
    return ToolConfigPair(s.left(separatorIndex), configString);
}

}
