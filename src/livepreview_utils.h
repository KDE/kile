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

#ifndef LIVEPREVIEW_UTILS_H
#define LIVEPREVIEW_UTILS_H

#include <QString>

#include "tool_utils.h"

#define LIVEPREVIEW_DEFAULT_TOOL_NAME        "LivePreview-PDFLaTeX"

namespace KileTool {

class LivePreviewUserStatusHandler
{
public:
    LivePreviewUserStatusHandler();

    bool userSpecifiedLivePreviewStatus() const;
    bool isLivePreviewEnabled() const;
    void setLivePreviewEnabled(bool b);

    ToolConfigPair livePreviewTool() const;
    // returns 'true' iff the live preview type has changed
    bool setLivePreviewTool(const ToolConfigPair& p);

private:
    bool m_userSpecifiedLivePreviewStatus;
    bool m_livePreviewEnabled;
    ToolConfigPair m_livePreviewTool;
};

}

#endif
