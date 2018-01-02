/*************************************************************************************
    begin                : Wed Jun 6 2001
    copyright            : (C) 2003 by Jeroen Wijnout (Jeroen.Wijnhout@kdemail.net)
                               2007-2016 by Michel Ludwig (michel.ludwig@kdemail.net)
 *************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONFIGURATIONDIALOG_H
#define CONFIGURATIONDIALOG_H

#include <KConfigDialog>

#include <KConfigDialogManager>

#include "widgets/codecompletionconfigwidget.h"     // code completion (dani)
#include "widgets/previewconfigwidget.h"      // QuickPreview (dani)
#include "widgets/environmentconfigwidget.h"          // environments (dani)
#include "widgets/graphicsconfigwidget.h"     // graphics (dani)
#include "widgets/structureviewconfigwidget.h"    // structure view (dani)
#include "widgets/symbolviewconfigwidget.h"

class KConfig;

namespace KileWidget {
class ToolConfig;
}
class KileWidgetAppearanceConfig;
class KileWidgetHelpConfig;
class KileWidgetLatexConfig;
class KileWidgetLivePreviewConfig;
class KileWidgetGeneralConfig;
class KileWidgetEnvironmentConfig;
class KileWidgetGraphicsConfig;
class KileWidgetStructureViewConfig;
class KileWidgetScriptingConfig;
class KileWidgetUsermenuConfig;

namespace KileTool {
class Manager;
}

namespace KTextEditor {
class ConfigPage;
}

namespace KileDialog
{
class Config : public KPageDialog
{
    Q_OBJECT

public:
    Config( KConfig *config, KileInfo *ki, QWidget* parent = 0);
    ~Config();

    virtual void show();

    //Q_SIGNALS:
    //	void widgetModified();

private Q_SLOTS:
    void slotAcceptChanges();

private:
    // dialog manager
    KConfigDialogManager *m_manager;

    KConfig *m_config;
    KConfigGroup m_configDialogSize;
    KileInfo *m_ki;

    QList<KPageWidgetItem*> m_pageWidgetItemList;

    bool m_editorSettingsChanged;

    KileWidget::ToolConfig	*toolPage;

    // CodeCompletion (dani)
    CodeCompletionConfigWidget *completePage;
    KileWidgetPreviewConfig *previewPage;

    KileWidgetHelpConfig *helpPage;
    KileWidgetLatexConfig *latexPage;
    KileWidgetGeneralConfig *generalPage;
    KileWidgetEnvironmentConfig *envPage;
    KileWidgetGraphicsConfig *graphicsPage;
    KileWidgetStructureViewConfig *structurePage;
    KileWidgetSymbolViewConfig *symbolViewPage;
    KileWidgetScriptingConfig *scriptingPage;
    KileWidgetUsermenuConfig *usermenuPage;
    KileWidgetLivePreviewConfig *livePreviewPage;
    KileWidgetAppearanceConfig *appearancePage;

    // setup configuration
    KPageWidgetItem* addConfigFolder(const QString &section,const QString &icon);

    KPageWidgetItem* addConfigPage(KPageWidgetItem* parent, QWidget *page,
                                   const QString &itemName, const QString &pixmapName,
                                   const QString &header = QString());

    KPageWidgetItem* addConfigPage(KPageWidgetItem* parent, QWidget *page,
                                   const QString &itemName, const QIcon& icon,
                                   const QString &header = QString());

    void setupGeneralOptions(KPageWidgetItem* parent);
    void setupTools(KPageWidgetItem* parent);
    void setupLatex(KPageWidgetItem* parent);
    void setupCodeCompletion(KPageWidgetItem* parent);
    void setupQuickPreview(KPageWidgetItem* parent);
    void setupHelp(KPageWidgetItem* parent);
    void setupEditor(KPageWidgetItem* parent);
    void setupEnvironment(KPageWidgetItem* parent);
    void setupGraphics(KPageWidgetItem* parent);
    void setupStructure(KPageWidgetItem* parent);
    void setupSymbolView(KPageWidgetItem* parent);
    void setupScripting(KPageWidgetItem* parent);
    void setupUsermenu(KPageWidgetItem* parent);
    void setupLivePreview(KPageWidgetItem* parent);
    void setupAppearance(KPageWidgetItem* parent);

    // write configuration
    void writeGeneralOptionsConfig();

    // editor pages
    QMap<KPageWidgetItem*, KTextEditor::ConfigPage*> m_editorPages;
};
}
#endif
