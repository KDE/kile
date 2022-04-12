/**************************************************************************
*   Copyright (C) 2011-2012 by Michel Ludwig (michel.ludwig@kdemail.net)  *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "livepreviewconfigwidget.h"

#include "kileconfig.h"

#include "livepreview_utils.h"

#include "kiledebug.h"
#include "kiletoolmanager.h"
#include <KToggleAction>


KileWidgetLivePreviewConfig::KileWidgetLivePreviewConfig(KConfig *config, QWidget *parent)
    : QWidget(parent),
      m_config(config)
{
    setupUi(this);
}

KileWidgetLivePreviewConfig::~KileWidgetLivePreviewConfig()
{
}

void KileWidgetLivePreviewConfig::readConfig()
{
    if(KileConfig::livePreviewCompileOnlyAfterSaving()) {
        m_compileDocumentOnSaveRadioButton->setChecked(true);
    }
    else {
        m_compileDocumentOnChangesRadioButton->setChecked(true);
    }

    QString defaultToolName = KileConfig::livePreviewDefaultTool();
    if(defaultToolName.isEmpty()) {
        defaultToolName = LIVEPREVIEW_DEFAULT_TOOL_NAME;
    }
    KileTool::ToolConfigPair defaultTool = KileTool::ToolConfigPair::fromConfigStringRepresentation(defaultToolName);

    int currentIndex = 0;
    int defaultToolNameIndex = 0;

    QList<KileTool::ToolConfigPair> toolList = KileTool::toolsWithConfigurationsBasedOnClass(m_config, "LaTeXLivePreview");
    std::sort(toolList.begin(), toolList.end());
    m_previewDefaultToolComboBox->clear();
    for(QList<KileTool::ToolConfigPair>::iterator i = toolList.begin(); i != toolList.end(); ++i) {
        KileTool::ToolConfigPair currentTool = KileTool::ToolConfigPair(QString((*i).first), QString((*i).second));

        if(currentTool == defaultTool) {
            defaultToolNameIndex = currentIndex;
        }
        m_previewDefaultToolComboBox->addItem(currentTool.userStringRepresentation().remove("LivePreview-"), QVariant::fromValue(currentTool));
        currentIndex++;
    }
    m_previewDefaultToolComboBox->setCurrentIndex(defaultToolNameIndex);
}

void KileWidgetLivePreviewConfig::writeConfig()
{
    KileConfig::setLivePreviewCompileOnlyAfterSaving(m_compileDocumentOnSaveRadioButton->isChecked());

    KileTool::ToolConfigPair defaultTool = m_previewDefaultToolComboBox->itemData(m_previewDefaultToolComboBox->currentIndex()).value<KileTool::ToolConfigPair>();
    KileConfig::setLivePreviewDefaultTool(defaultTool.configStringRepresentation());
}


