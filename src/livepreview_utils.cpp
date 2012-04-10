/********************************************************************************
  Copyright (C) 2012 by Michel Ludwig (michel.ludwig@kdemail.net)
 ********************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "livepreview_utils.h"

#include "kileconfig.h"

namespace KileTool {

LivePreviewUserStatusHandler::LivePreviewUserStatusHandler()
: m_userSpecifiedLivePreviewStatus(false),
  m_livePreviewEnabled(true),
  m_livePreviewToolName(LIVEPREVIEW_DEFAULT_TOOL_NAME),
  m_livePreviewToolConfigName(LIVEPREVIEW_DEFAULT_TOOL_CONFIG_NAME)
{
}

bool LivePreviewUserStatusHandler::userSpecifiedLivePreviewStatus() const
{
	return m_userSpecifiedLivePreviewStatus;
}

bool LivePreviewUserStatusHandler::isLivePreviewEnabled() const
{
	return (m_userSpecifiedLivePreviewStatus ? m_livePreviewEnabled : KileConfig::previewEnabledForFreshlyOpenedDocuments());
}

void LivePreviewUserStatusHandler::setLivePreviewEnabled(bool b)
{
	m_userSpecifiedLivePreviewStatus = true;
	m_livePreviewEnabled = b;
}

QString LivePreviewUserStatusHandler::livePreviewToolName() const
{
	return m_livePreviewToolName;
}

QString LivePreviewUserStatusHandler::livePreviewToolConfigName() const
{
	return m_livePreviewToolConfigName;
}

bool LivePreviewUserStatusHandler::setLivePreviewTool(const QString& toolName, const QString& configName)
{
	if(toolName == m_livePreviewToolName && configName == m_livePreviewToolConfigName) {
		return false;
	}
	m_livePreviewToolName = toolName;
	m_livePreviewToolConfigName = configName;
	return true;
}

}