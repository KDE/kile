/***************************************************************************************
    begin                : sam jui 13 09:50:06 CEST 2002
    copyright            : (C) 2003 by Jeroen Wijnhout (wijnhout@science.uva.nl)
                               2007-2019 by Michel Ludwig (michel.ludwig@kdemail.net)
                           (C) 2009 Thomas Braun (thomas.braun@virtuell-zuhause.de)

 ***************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KILE_H
#define KILE_H

#include <QList>
#include <QSplitter>
#include <QStackedWidget>
#include <QString>
#include <QStringList>
#include <QToolBox>
#include <QWidget>

#include <KTextEditor/View>
#include <KTextEditor/Document>
#include <ktogglefullscreenaction.h>
#include <KXmlGuiWindow>

#include "widgets/filebrowserwidget.h"
#include "kileinfo.h"
#include "kileactions.h"
#include "kileversion.h"
#include "widgets/symbolview.h"
#include "widgets/commandview.h"

#include "outputinfo.h"

#include "codecompletion.h"        // code completion (dani)
#include "editorextension.h"              // advanced editor (dani)
#include "kilehelp.h"              // kile help (dani)
#include "quickpreview.h"
#include "widgets/abbreviationview.h"

class QFileInfo;
class QTimer;

class KToolBar;
class QAction;
class KActionMenu;
class KRecentFilesAction;
class KToggleFullScreenAction;
class KToggleToolBarAction;
class KMultiTabBar;

namespace KParts {
class MainWindow;
}

class KileLyxServer;
class KileProject;
class KileProjectItem;
class TemplateItem;
class KileAutoSaveJob;

namespace KileAction {
class TagData;
}
namespace KileDocument {
class Info;
class TextInfo;
class Extensions;
}
namespace KileTool {
class Manager;
class Factory;
}
namespace KileWidget {
class LogWidget;
class Output;
class Konsole;
class StructureWidget;
class SideBar;
class BottomBar;
class StatusBar;
}

//TODO remove once we stop supporting pre 1.7 user tools
struct userItem
{
    QString name, tag;
};

/**
 * The Kile main class. It acts as information manager and DBUS interface.
 **/
class Kile : public KParts::MainWindow, public KileInfo
{
    Q_OBJECT

public:
    explicit Kile(bool allowRestore = true, QWidget *parent = Q_NULLPTR);
    ~Kile();

    int lineNumber() override;
    KileWidget::StatusBar * statusBar();

public Q_SLOTS:
    void setCursor(const QUrl &, int, int);

    void runArchiveTool();
    void runArchiveTool(const QUrl&);

    void updateModeStatus();
    void newCaption();

    void rebuildBibliographyMenu();

    void openDocument(const QUrl &url);
    void openProject(const QUrl &url);

    // D-Bus Interface
    void openDocument(const QString & url);
    void closeDocument();
    void setActive();
    /**
     * @param line : Jump to give line in current editor (can be called via DBUS interface).
     **/
    void setLine(const QString &line) override;
    void openProject(const QString& proj);
    void runTool(const QString& tool);
    void runToolWithConfig(const QString &tool, const QString &config);
    void insertText(const QString &text);
    void insertTag(const KileAction::TagData& td);

Q_SIGNALS:
    void masterDocumentChanged();

protected:
    virtual bool queryClose() override;

private:
    QMap<QString,bool>              m_dictMenuAction,
                                    m_dictMenuFile,
                                    m_dictMenuProject;

    KToolBar                       *m_toolsToolBar;
    KActionMenu                    *m_userHelpActionMenu;
    KSelectAction                  *m_bibTagSettings;
    ToolbarSelectAction            *m_compilerActions,
                                   *m_viewActions,
                                   *m_convertActions,
                                   *m_quickActions;

    QList<KileAction::TagData>      m_listUserTags;
    QList<userItem>                 m_listUserTools;
    QList<QAction*>                 m_listQuickActions,
                                    m_listCompilerActions,
                                    m_listConverterActions,
                                    m_listViewerActions,
                                    m_listOtherActions;

    KActionMenu                    *m_bibTagActionMenu;
    KToggleAction                  *ModeAction,
                                   *WatchFileAction;
    KToggleAction                  *m_actionMessageView;
    KToggleAction                  *m_actionShowMenuBar;
    KRecentFilesAction             *m_actRecentFiles;
    KToggleFullScreenAction        *m_pFullScreen;

    /* GUI */
    //widgets
    KileWidget::SideBar            *m_sideBar;
    KileWidget::AbbreviationView   *m_kileAbbrevView;
    QStackedWidget                 *m_topWidgetStack;
    QSplitter                      *m_horizontalSplitter,
                                   *m_verticalSplitter;
    QToolBox                       *m_toolBox;
    KileWidget::CommandViewToolBox *m_commandViewToolBox;
    KileWidget::SymbolView         *m_symbolViewMFUS,
                                   *m_symbolViewRelation,
                                   *m_symbolViewArrows,
                                   *m_symbolViewMiscMath,
                                   *m_symbolViewMiscText,
                                   *m_symbolViewOperators,
                                   *m_symbolViewUser,
                                   *m_symbolViewDelimiters,
                                   *m_symbolViewGreek,
                                   *m_symbolViewSpecial,
                                   *m_symbolViewCyrillic;
    KileWidget::CommandView        *m_commandView;
    KToolBar                       *m_latexOutputErrorToolBar;
    QMenu                          *m_buildMenuTopLevel,
                                   *m_buildMenuCompile,
                                   *m_buildMenuConvert,
                                   *m_buildMenuViewer,
                                   *m_buildMenuOther,
                                   *m_buildMenuQuickPreview;

    /* config */
    KSharedConfigPtr               m_config;
    QStringList                    m_recentFilesList,
                                   m_listDocsOpenOnStart,
                                   m_listEncodingsOfDocsOpenOnStart,
                                   m_listProjectsOpenOnStart;

    KRecentFilesAction             *m_actRecentProjects;

    KileLyxServer                  *m_lyxserver;

    /* actions */
    void initSelectActions();
    void setupSideBar();
    void setupProjectView();
    void setupStructureView();
    void setupScriptsManagementView();
    void setupCommandViewToolbox();
    void setupSymbolViews();
    void enableSymbolViewMFUS();
    void disableSymbolViewMFUS();
    void setupAbbreviationView();
    void setupBottomBar();
    void setupGraphicTools();
    void setupPreviewTools();
    void setupActions();
    QAction* createToolAction(const QString& toolName);
    void createToolActions();
    void setupTools();
    void updateUserDefinedMenus();
    void cleanUpActionList(QList<QAction*> &list, const QStringList &tools);
    void restoreLastSelectedAction();
    void saveLastSelectedAction();

    void transformOldUserTags();

    void initMenu();
    void setMenuItems(QStringList &list, QMap<QString,bool> &dict);
    void updateMenu();
    bool updateMenuActivationStatus(QMenu *menu);
    bool updateMenuActivationStatus(QMenu *menu, const QSet<QMenu*>& visited);
    void updateLatexenuActivationStatus(QMenu *menu, bool state);
    void updateUserMenuStatus(bool state);

    template<class ContextType, class Func>
    inline QAction* createAction(const QString &text, const char* actionName, const ContextType* context, Func function)
    {
        return createAction<ContextType, Func>(text, actionName, QString(), QKeySequence(), context, function);
    }

    template<class ContextType, class Func>
    inline QAction* createAction(const QString &text, const char* actionName, const QString& iconName, const ContextType* context, Func function)
    {
        return createAction<ContextType, Func>(text, QLatin1String(actionName), iconName, QKeySequence(), context, function);
    }

    template<class ContextType, class Func>
    QAction* createAction(const QString &text, const QString& actionName, const QString& iconName, const ContextType* context, Func function)
    {
        return createAction<ContextType, Func>(text, actionName, iconName, QKeySequence(), context, function);
    }

    template<class ContextType, class Func>
    inline QAction* createAction(const QString &text, const char* actionName, const QKeySequence& shortcut, const ContextType* context, Func function)
    {
        return createAction<ContextType, Func>(text, actionName, QString(), shortcut, context, function);
    }

    template<class ContextType, class Func>
    inline QAction* createAction(const QString &text, const char* actionName, const QString& iconName, const QKeySequence& shortcut,
                                 const ContextType* context, Func function)
    {
        return createAction<ContextType, Func>(text, QLatin1String(actionName), iconName, shortcut, context, function);
    }

    template<class ContextType, class Func>
    QAction* createAction(const QString &text, const QString& actionName, const QString& iconName, const QKeySequence& shortcut,
                          const ContextType* context, Func function);

    template<class ContextType, class Func>
    inline QAction* createAction(KStandardAction::StandardAction actionType, const ContextType* context, Func function)
    {
        return createAction<ContextType, Func>(actionType, QString(), context, function);
    }

    template<class ContextType, class Func>
    QAction* createAction(KStandardAction::StandardAction actionType, const QString &name, const ContextType* context, Func function);

    template<class ContextType, class Func>
    QAction* createAction(const QString &text, const QString &actionName, const QString& iconName, const QList<QKeySequence>& shortcut, const ContextType* context, Func function);

    void setMasterDocumentFileName(const QString& fileName);
    void clearMasterDocument();

private Q_SLOTS:
    void toggleMasterDocumentMode();
    void toggleWatchFile();
    void refreshStructure();

    bool resetPart();
    void enableGUI(bool);
    void slotToggleFullScreen();

    void toggleShowMenuBar(bool showMessage = true);

    void restoreFilesAndProjects(bool allowRestore);
    void readGUISettings();
    void transformOldUserSettings();
    void readRecentFileSettings();
    void saveSettings();

    void readConfig();

    void generalOptions();
    void configureKeys();
    void configureToolbars() override;
    void slotPerformCheck();

    void aboutEditorComponent();

    /**
     * Activates (sets up the GUI for the editor part) the view.
     * @param updateStruct  If true, force an update of the structure view.
     **/
    void activateView(QWidget* view, bool updateStruct = true);

    void focusLog() override;
    void focusOutput() override;
    void focusKonsole() override;
    void focusEditor() override;
    void focusPreview() override;

    void sideOrBottomBarChanged(bool visible);

    void showDocInfo(KTextEditor::View *view = Q_NULLPTR);
    void convertToASCII(KTextEditor::Document *doc = Q_NULLPTR);
    void convertToEnc(KTextEditor::Document *doc = Q_NULLPTR);

    void cleanAll(KileDocument::TextInfo *docinfo = Q_NULLPTR);
    void cleanBib();

    void findInFiles();
    void findInProjects();
    void grepItemSelected(const QString &abs_filename, int line);

    /**
    * @param td Inserts the TagData td into the current editor
    * @param pkgs list of packages needed for this command
    *
    * warns if latex packages in pkgs are not included in the document
    **/
    void insertTag(const KileAction::TagData& td, const QStringList& pkgs);

    /**
     * An overloaded member function, behaves essentially as above.
     **/

    void insertTag(const KileAction::TagData& td, const QList<Package>& pkgs);
    void insertTag(const QString& tagB, const QString& tagE, int dx, int dy);
    void insertAmsTag(const KileAction::TagData& td);
    void insertText(const QString &text, const QStringList &pkgs);
    void insertText(const QString &text, const QList<Package> &pkgs);

    void quickTabular();
    void quickArray();
    void quickTabbing();
    void quickDocument();
    void quickFloat();
    void quickMathenv();
    void quickPostscript();
    void quickPdf();
    void quickTabulardialog(bool tabularenv);

    void quickUserMenuDialog();
    void slotUpdateUserMenuStatus();

    void includeGraphics();

    // QuickPreview
    void slotQuickPreview(int type);

    void quickPreviewEnvironment() {
        slotQuickPreview(KileTool::qpEnvironment);
    }
    void quickPreviewSelection()   {
        slotQuickPreview(KileTool::qpSelection);
    }
    void quickPreviewSubdocument() {
        slotQuickPreview(KileTool::qpSubdocument);
    }
    void quickPreviewMathgroup()   {
        slotQuickPreview(KileTool::qpMathgroup);
    }

    void addRecentFile(const QUrl &url);
    void removeRecentFile(const QUrl &url);
    void addRecentProject(const QUrl &url);
    void removeRecentProject(const QUrl &url);

    void updateStatusBarCursorPosition(KTextEditor::View *view, const KTextEditor::Cursor &newPosition);
    void updateStatusBarViewMode(KTextEditor::View *view);
    void updateStatusBarInformationMessage(KTextEditor::View *view, const QString &message);
    void updateStatusBarSelection(KTextEditor::View *view);

    void handleDocumentParsingStarted();
    void handleDocumentParsingComplete();
};

#endif
