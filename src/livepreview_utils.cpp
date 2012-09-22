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
  m_livePreviewTool(LIVEPREVIEW_DEFAULT_TOOL_NAME, DEFAULT_TOOL_CONFIGURATION)
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

ToolConfigPair LivePreviewUserStatusHandler::livePreviewTool() const
{
	return m_livePreviewTool;
}

bool LivePreviewUserStatusHandler::setLivePreviewTool(const ToolConfigPair& p)
{
	if(m_livePreviewTool == p) {
		return false;
	}
	m_livePreviewTool = p;
	return true;
}

}