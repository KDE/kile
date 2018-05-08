/********************************************************************************
  Copyright (C) 2011-2018 by Michel Ludwig (michel.ludwig@kdemail.net)
 ********************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "livepreview.h"
#include "config.h"

#include <algorithm>

#include <QCryptographicHash>
#include <QDir>
#include <QHBoxLayout>
#include <QMap>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTextCodec>
#include <QTextStream>
#include <QTimer>
#include <QTemporaryDir>

#include <KActionCollection>
#include <KIconLoader>
#include <KLocalizedString>
#include <KTextEditor/Application>
#include <KTextEditor/CodeCompletionInterface>
#include <KTextEditor/Document>
#include <KTextEditor/MainWindow>
#include <KTextEditor/View>
#include <KToolBar>
#include <KParts/MainWindow>
#include <KXMLGUIFactory>

#include <okular/interfaces/viewerinterface.h>

#include "errorhandler.h"
#include "kiledebug.h"
#include "kiletool_enums.h"
#include "kiledocmanager.h"
#include "kileviewmanager.h"

//TODO: it still has to be checked whether it is necessary to use LaTeXInfo objects

namespace KileTool
{

class LivePreviewManager::PreviewInformation {
public:
    PreviewInformation()
        : lastSynchronizationCursor(-1, -1)
    {
        initTemporaryDirectory();
    }

    ~PreviewInformation() {
        delete m_tempDir;
    }

    QString getTempDir() const {
        return m_tempDir->path();
    }

    void clearPreviewPathMappings() {
        pathToPreviewPathHash.clear();
        previewPathToPathHash.clear();
    }

    bool createSubDirectoriesForProject(KileProject *project, bool *containsInvalidRelativeItem = Q_NULLPTR) {
        if(containsInvalidRelativeItem) {
            *containsInvalidRelativeItem = false;
        }
        const QList<KileProjectItem*> items = project->items();
        const QString tempCanonicalDir = QDir(m_tempDir->path()).canonicalPath();
        if(tempCanonicalDir.isEmpty()) {
            return false;
        }
        for(KileProjectItem *item : items) {
            bool successful = true;
            const QString itemRelativeDir = QFileInfo(tempCanonicalDir + '/' + item->path()).path();
            const QString itemAbsolutePath = QDir(itemRelativeDir).absolutePath();
            if(itemAbsolutePath.isEmpty()) {
                successful = false;
            }
            else if(!itemAbsolutePath.startsWith(tempCanonicalDir)) {
                if(containsInvalidRelativeItem) {
                    *containsInvalidRelativeItem = true;
                }
                successful = false; // we don't want to create directories below 'm_tempDir->name()'
            }
            else {
                successful = QDir().mkpath(itemAbsolutePath);
            }
            if(!successful) {
                return false;
            }
        }
        return true;
    }

    void setLastSynchronizationCursor(int line, int col)
    {
        lastSynchronizationCursor.setLine(line);
        lastSynchronizationCursor.setColumn(col);
    }

private:
    QTemporaryDir *m_tempDir;

    void initTemporaryDirectory() {
        m_tempDir = new QTemporaryDir(QDir::tempPath() + QLatin1Char('/') + "kile-livepreview");
    }

public:
    QHash<QString, QString> pathToPreviewPathHash;
    QHash<QString, QString> previewPathToPathHash;
    QString previewFile;
    QHash<KileDocument::TextInfo*, QByteArray> textHash;
    KTextEditor::Cursor lastSynchronizationCursor;
};

LivePreviewManager::LivePreviewManager(KileInfo *ki, KActionCollection *ac)
    : m_ki(ki),
      m_bootUpMode(true),
      m_previewStatusLed(Q_NULLPTR),
      m_previewForCurrentDocumentAction(Q_NULLPTR),
      m_recompileLivePreviewAction(Q_NULLPTR),
      m_runningLaTeXInfo(Q_NULLPTR), m_runningTextView(Q_NULLPTR), m_runningProject(Q_NULLPTR),
      m_runningPreviewInformation(Q_NULLPTR), m_shownPreviewInformation(Q_NULLPTR), m_masterDocumentPreviewInformation(Q_NULLPTR)
{
    connect(m_ki->viewManager(), SIGNAL(textViewActivated(KTextEditor::View*)),
            this, SLOT(handleTextViewActivated(KTextEditor::View*)));
    connect(m_ki->viewManager(), SIGNAL(textViewClosed(KTextEditor::View*,bool)),
            this, SLOT(handleTextViewClosed(KTextEditor::View*,bool)));
    connect(m_ki->toolManager(), SIGNAL(childToolSpawned(KileTool::Base*,KileTool::Base*)),
            this, SLOT(handleSpawnedChildTool(KileTool::Base*, KileTool::Base*)));
    connect(m_ki->docManager(), SIGNAL(documentSavedAs(KTextEditor::View*, KileDocument::TextInfo*)),
            this, SLOT(handleDocumentSavedAs(KTextEditor::View*, KileDocument::TextInfo*)));
    connect(m_ki->docManager(), SIGNAL(documentOpened(KileDocument::TextInfo*)),
            this, SLOT(handleDocumentOpened(KileDocument::TextInfo*)));
    connect(m_ki->docManager(), SIGNAL(projectOpened(KileProject*)),
            this, SLOT(handleProjectOpened(KileProject*)));

    createActions(ac);
    populateViewerControlToolBar();

    m_ledBlinkingTimer = new QTimer(this);
    m_ledBlinkingTimer->setSingleShot(false);
    m_ledBlinkingTimer->setInterval(500);
    connect(m_ledBlinkingTimer, SIGNAL(timeout()), m_previewStatusLed, SLOT(toggle()));

    m_documentChangedTimer = new QTimer(this);
    m_documentChangedTimer->setSingleShot(true);
    connect(m_documentChangedTimer, SIGNAL(timeout()), this, SLOT(handleDocumentModificationTimerTimeout()));

    showPreviewDisabled();
}

LivePreviewManager::~LivePreviewManager()
{
    KILE_DEBUG_MAIN;

    qDeleteAll(m_livePreviewToolActionList);
    m_livePreviewToolActionList.clear();

    deleteAllLivePreviewInformation();
}

void LivePreviewManager::disableBootUpMode()
{
    m_bootUpMode = false;
    recompileLivePreview();
}

void LivePreviewManager::createActions(KActionCollection *ac)
{

    m_livePreviewToolActionGroup = new QActionGroup(ac);

    m_previewForCurrentDocumentAction = new KToggleAction(QIcon::fromTheme("document-preview"), i18n("Live Preview for Current Document or Project"), this);
    m_previewForCurrentDocumentAction->setChecked(true);
    connect(m_previewForCurrentDocumentAction, SIGNAL(triggered(bool)), this, SLOT(previewForCurrentDocumentActionTriggered(bool)));
    ac->addAction("live_preview_for_current_document", m_previewForCurrentDocumentAction);

    m_recompileLivePreviewAction = new QAction(i18n("Recompile Live Preview"), this);
    connect(m_recompileLivePreviewAction, SIGNAL(triggered()), this, SLOT(recompileLivePreview()));
    ac->addAction("live_preview_recompile", m_recompileLivePreviewAction);

    {
        QAction *action = new QAction(i18n("Save Compiled Document..."), this);
        connect(action, &QAction::triggered, m_ki->docManager(), &KileDocument::Manager::fileSaveCompiledDocument);
        ac->addAction("file_save_compiled_document", action);
        connect(this, &KileTool::LivePreviewManager::livePreviewSuccessful, action, [=]() { action->setEnabled(true); });
        connect(this, &KileTool::LivePreviewManager::livePreviewRunning, action, [=]() { action->setEnabled(false); });
        connect(this, &KileTool::LivePreviewManager::livePreviewStopped, action, [=]() { action->setEnabled(false); });
    }
}

void LivePreviewManager::previewForCurrentDocumentActionTriggered(bool b)
{
    if(m_bootUpMode || !KileConfig::livePreviewEnabled()) {
        return;
    }
    KTextEditor::View *view = m_ki->viewManager()->currentTextView();
    if(!view) {
        return;
    }
    KileDocument::LaTeXInfo *latexInfo = dynamic_cast<KileDocument::LaTeXInfo*>(m_ki->docManager()->textInfoFor(view->document()));
    if(!latexInfo) {
        return;
    }
    LivePreviewUserStatusHandler *userStatusHandler;
    findPreviewInformation(latexInfo, Q_NULLPTR, &userStatusHandler);
    Q_ASSERT(userStatusHandler);

    userStatusHandler->setLivePreviewEnabled(b);

    if(b) {
        showPreviewCompileIfNecessary(latexInfo, view);
    }
    else {
        disablePreview();
    }
}

void LivePreviewManager::livePreviewToolActionTriggered()
{
    QAction *action = dynamic_cast<QAction*>(sender());
    if(!action) {
        KILE_DEBUG_MAIN << "slot called from wrong object!!";
        return;
    }
    if(!m_actionToLivePreviewToolHash.contains(action)) {
        KILE_DEBUG_MAIN << "action not found in hash!!";
        return;
    }
    const ToolConfigPair p = m_actionToLivePreviewToolHash[action];
    KTextEditor::View *view = m_ki->viewManager()->currentTextView();
    if(!view) {
        KILE_DEBUG_MAIN << "no text view open!";
        return;
    }
    KileDocument::LaTeXInfo *latexInfo = dynamic_cast<KileDocument::LaTeXInfo*>(m_ki->docManager()->textInfoFor(view->document()));
    if(!latexInfo) {
        KILE_DEBUG_MAIN << "current view is not LaTeX-compatible!";
        return;
    }

    LivePreviewUserStatusHandler *userStatusHandler;
    findPreviewInformation(latexInfo, Q_NULLPTR, &userStatusHandler);
    if(!userStatusHandler) {
        KILE_DEBUG_MAIN << "no preview information found!";
        return;
    }
    const bool changed = userStatusHandler->setLivePreviewTool(p);
    if(changed) {
        recompileLivePreview();
    }
}

void LivePreviewManager::updateLivePreviewToolActions(LivePreviewUserStatusHandler *userStatusHandler)
{
    setLivePreviewToolActionsEnabled(true);
    const ToolConfigPair p = userStatusHandler->livePreviewTool();
    if(!m_livePreviewToolToActionHash.contains(p)) {
        return;
    }
    m_livePreviewToolToActionHash[p]->setChecked(true);
}

void LivePreviewManager::setLivePreviewToolActionsEnabled(bool b)
{
    Q_FOREACH(QAction *action, m_livePreviewToolActionList) {
        action->setEnabled(b);
    }
}

void LivePreviewManager::buildLivePreviewMenu(KConfig *config)
{
    QMenu *menu = dynamic_cast<QMenu*>(m_ki->mainWindow()->guiFactory()->container("menu_livepreview", m_ki->mainWindow()));
    if(!menu) {
        KILE_DEBUG_MAIN << "live preview menu not found!!";
        return;
    }

    qDeleteAll(m_livePreviewToolActionList);
    m_livePreviewToolActionList.clear();
    m_livePreviewToolToActionHash.clear();
    m_actionToLivePreviewToolHash.clear();

    // necessary as it will be disabled otherwise in 'kile.cpp' (as it's empty initially)
    menu->setEnabled(true);
    menu->clear();
    menu->addAction(m_previewForCurrentDocumentAction);
    menu->addSeparator();

    QList<ToolConfigPair> toolList = toolsWithConfigurationsBasedOnClass(config, "LaTeXLivePreview");
    std::sort(toolList.begin(), toolList.end());
    for(QList<ToolConfigPair>::iterator i = toolList.begin(); i != toolList.end(); ++i) {
        const QString shortToolName = QString((*i).first).remove("LivePreview-");
        QAction *action = new KToggleAction(ToolConfigPair::userStringRepresentation(shortToolName, (*i).second), this);

        m_livePreviewToolActionGroup->addAction(action);
        connect(action, SIGNAL(triggered()), this, SLOT(livePreviewToolActionTriggered()));
        m_livePreviewToolActionList.push_back(action);
        m_livePreviewToolToActionHash[*i] = action;
        m_actionToLivePreviewToolHash[action] = *i;
        menu->addAction(action);
    }
    menu->addSeparator();
    menu->addAction(m_recompileLivePreviewAction);
}

QString LivePreviewManager::getPreviewFile() const
{
    if(!m_shownPreviewInformation) {
        return QString();
    }
    return m_shownPreviewInformation->previewFile;
}

bool LivePreviewManager::isLivePreviewEnabledForCurrentDocument()
{
    return m_previewForCurrentDocumentAction->isChecked();
}

void LivePreviewManager::setLivePreviewEnabledForCurrentDocument(bool b)
{
    m_previewForCurrentDocumentAction->setChecked(b);
    previewForCurrentDocumentActionTriggered(b);
}

void LivePreviewManager::disablePreview()
{
    stopAndClearPreview();
    setLivePreviewToolActionsEnabled(false);
    m_previewForCurrentDocumentAction->setChecked(false);
    m_ki->viewManager()->setLivePreviewModeForDocumentViewer(false);
}

void LivePreviewManager::stopAndClearPreview()
{
    KILE_DEBUG_MAIN;
    stopLivePreview();
    clearLivePreview();
}

void LivePreviewManager::clearLivePreview()
{
    KILE_DEBUG_MAIN;
    showPreviewDisabled();

    KParts::ReadOnlyPart *viewerPart = m_ki->viewManager()->viewerPart();
    if(m_shownPreviewInformation && viewerPart->url() == QUrl::fromLocalFile(m_shownPreviewInformation->previewFile)) {
        viewerPart->closeUrl();
    }
    m_shownPreviewInformation = Q_NULLPTR;
    emit(livePreviewStopped());
}

void LivePreviewManager::stopLivePreview()
{
    m_documentChangedTimer->stop();
    m_ki->toolManager()->stopLivePreview();

    clearRunningLivePreviewInformation();
}

void LivePreviewManager::clearRunningLivePreviewInformation()
{
    m_runningPathToPreviewPathHash.clear();
    m_runningPreviewPathToPathHash.clear();
    m_runningPreviewFile.clear();
    m_runningLaTeXInfo = Q_NULLPTR;
    m_runningProject = Q_NULLPTR;
    m_runningTextView = Q_NULLPTR;
    m_runningPreviewInformation = Q_NULLPTR;
    m_runningTextHash.clear();
}

void LivePreviewManager::deleteAllLivePreviewInformation()
{
    // first, we have to make sure that nothing is shown anymore,
    // and that no preview is running
    stopAndClearPreview();

    disablePreview();

    // and now we can delete all the 'PreviewInformation' objects
    delete m_masterDocumentPreviewInformation;
    m_masterDocumentPreviewInformation = Q_NULLPTR;

    for(QHash<KileDocument::LaTeXInfo*, PreviewInformation*>::iterator i = m_latexInfoToPreviewInformationHash.begin();
            i != m_latexInfoToPreviewInformationHash.end(); ++i) {
        delete i.value();
    }

    for(QHash<KileProject*,PreviewInformation*>::iterator i = m_projectToPreviewInformationHash.begin();
            i != m_projectToPreviewInformationHash.end(); ++i) {
        delete i.value();
    }
    m_latexInfoToPreviewInformationHash.clear();
    m_projectToPreviewInformationHash.clear();
}

void LivePreviewManager::readConfig(KConfig *config)
{
    Q_UNUSED(config);

    buildLivePreviewMenu(config);

    m_previewForCurrentDocumentAction->setEnabled(KileConfig::livePreviewEnabled());
    m_previewStatusLed->setEnabled(KileConfig::livePreviewEnabled());

    if(m_bootUpMode || !KileConfig::livePreviewEnabled()) {
        deleteAllLivePreviewInformation();
    }
    else {
        refreshLivePreview(); // e.g. in case the live preview was disabled and no preview is
        // currently shown
    }
}

void LivePreviewManager::writeConfig()
{
}

void LivePreviewManager::readLivePreviewStatusSettings(KConfigGroup &configGroup, LivePreviewUserStatusHandler *handler)
{
    // the prefix 'kile_' is necessary as these settings might be written into a config group that is also modified
    // by KatePart
    if(configGroup.readEntry("kile_livePreviewStatusUserSpecified", false)) {
        handler->setLivePreviewEnabled(configGroup.readEntry("kile_livePreviewEnabled", true));
    }

    const QString livePreviewToolConfigString = configGroup.readEntry("kile_livePreviewTool", "");
    if(livePreviewToolConfigString.isEmpty()) {
        handler->setLivePreviewTool(ToolConfigPair(LIVEPREVIEW_DEFAULT_TOOL_NAME, DEFAULT_TOOL_CONFIGURATION));
    }
    else {
        handler->setLivePreviewTool(ToolConfigPair::fromConfigStringRepresentation(livePreviewToolConfigString));
    }
}

void LivePreviewManager::writeLivePreviewStatusSettings(KConfigGroup &configGroup, LivePreviewUserStatusHandler *handler)
{
    configGroup.writeEntry("kile_livePreviewTool", handler->livePreviewTool().configStringRepresentation());
    configGroup.writeEntry("kile_livePreviewEnabled", handler->isLivePreviewEnabled());
    configGroup.writeEntry("kile_livePreviewStatusUserSpecified", handler->userSpecifiedLivePreviewStatus());
}

void LivePreviewManager::populateViewerControlToolBar()
{
    KToolBar* viewerControlToolBar = m_ki->viewManager()->getViewerControlToolBar();
    viewerControlToolBar->addAction(m_previewForCurrentDocumentAction);

    m_previewStatusLed = new KLed(viewerControlToolBar);
    m_previewStatusLed->setShape(KLed::Circular);
    m_previewStatusLed->setLook(KLed::Flat);
    viewerControlToolBar->addWidget(m_previewStatusLed);
}

void LivePreviewManager::handleMasterDocumentChanged()
{
    if(m_bootUpMode || !KileConfig::livePreviewEnabled()) {
        return;
    }

    deleteAllLivePreviewInformation();
    refreshLivePreview();
}

void LivePreviewManager::handleTextChanged(KTextEditor::Document *doc)
{
    if(m_bootUpMode || !KileConfig::livePreviewEnabled()
                    || !isLivePreviewEnabledForCurrentDocument()) {
        return;
    }

    KILE_DEBUG_MAIN;
    if(!isCurrentDocumentOrProject(doc)) {
        return;
    }

    stopLivePreview();
    showPreviewOutOfDate();

    if(!KileConfig::livePreviewCompileOnlyAfterSaving()) {
        m_documentChangedTimer->start(KileConfig::livePreviewCompilationDelay());
    }
}

void LivePreviewManager::handleDocumentSavedOrUploaded(KTextEditor::Document *doc, bool savedAs)
{
    Q_UNUSED(savedAs);

    if(m_bootUpMode || !KileConfig::livePreviewEnabled()) {
        return;
    }

    KILE_DEBUG_MAIN;

    if(!KileConfig::livePreviewCompileOnlyAfterSaving()) {
        return;
    }

    if(!isCurrentDocumentOrProject(doc)) {
        return;
    }
    KTextEditor::View *view = m_ki->viewManager()->currentTextView();
    KileDocument::LaTeXInfo *latexInfo = dynamic_cast<KileDocument::LaTeXInfo*>(m_ki->docManager()->textInfoFor(view->document()));
    if(!latexInfo) {
        return;
    }

    LivePreviewUserStatusHandler *userStatusHandler;
    findPreviewInformation(latexInfo, Q_NULLPTR, &userStatusHandler);
    Q_ASSERT(userStatusHandler);
    if(userStatusHandler->isLivePreviewEnabled()) {
        showPreviewCompileIfNecessary(latexInfo, view);
    }
}

void LivePreviewManager::handleDocumentModificationTimerTimeout()
{
    if(m_bootUpMode || !KileConfig::livePreviewEnabled()) {
        return;
    }

    KILE_DEBUG_MAIN;

    KTextEditor::View *view = m_ki->viewManager()->currentTextView();
    KileDocument::LaTeXInfo *latexInfo = dynamic_cast<KileDocument::LaTeXInfo*>(m_ki->docManager()->textInfoFor(view->document()));
    if(!latexInfo) {
        return;
    }

    KTextEditor::CodeCompletionInterface *codeCompletionInterface = qobject_cast<KTextEditor::CodeCompletionInterface*>(view);

    // if the code completion box is currently shown, we don't trigger an update of the preview
    // as this will cause the document to be saved and the completion box to be hidden as a consequence
    if(codeCompletionInterface && codeCompletionInterface->isCompletionActive()) {
        m_documentChangedTimer->start();
        return;
    }

    LivePreviewUserStatusHandler *userStatusHandler;
    findPreviewInformation(latexInfo, Q_NULLPTR, &userStatusHandler);
    Q_ASSERT(userStatusHandler);
    if(userStatusHandler->isLivePreviewEnabled()) {
        compilePreview(latexInfo, view);
    }
}

void LivePreviewManager::showPreviewDisabled()
{
    KILE_DEBUG_MAIN;
    m_ledBlinkingTimer->stop();
    if(m_previewStatusLed) {
        m_previewStatusLed->off();
    }
}

void LivePreviewManager::showPreviewRunning()
{
    KILE_DEBUG_MAIN;
    if(m_previewStatusLed) {
        m_previewStatusLed->setColor(QColor(Qt::yellow));
        m_previewStatusLed->off();
    }
    m_ledBlinkingTimer->start();
}

void LivePreviewManager::showPreviewFailed()
{
    KILE_DEBUG_MAIN;
    m_ledBlinkingTimer->stop();
    if(m_previewStatusLed) {
        m_previewStatusLed->on();
        m_previewStatusLed->setColor(QColor(Qt::red));
    }
}

void LivePreviewManager::showPreviewSuccessful()
{
    KILE_DEBUG_MAIN;
    m_ledBlinkingTimer->stop();
    if(m_previewStatusLed) {
        m_previewStatusLed->on();
        m_previewStatusLed->setColor(QColor(Qt::green));
    }
}

void LivePreviewManager::showPreviewOutOfDate()
{
    KILE_DEBUG_MAIN;
    m_ledBlinkingTimer->stop();
    if(m_previewStatusLed) {
        m_previewStatusLed->on();
        m_previewStatusLed->setColor(QColor(Qt::yellow));
    }

}

// If a LaTeXInfo* pointer is passed as first argument, it is guaranteed that '*userStatusHandler' won't be Q_NULLPTR.
LivePreviewManager::PreviewInformation* LivePreviewManager::findPreviewInformation(KileDocument::TextInfo *textInfo,
        KileProject* *locatedProject,
        LivePreviewUserStatusHandler* *userStatusHandler,
        LaTeXOutputHandler* *latexOutputHandler)
{
    const QString masterDocumentFileName = m_ki->getMasterDocumentFileName();
    if(locatedProject) {
        *locatedProject = Q_NULLPTR;
    }
    KileDocument::LaTeXInfo *latexInfo = dynamic_cast<KileDocument::LaTeXInfo*>(textInfo);
    if(userStatusHandler) {
        *userStatusHandler = latexInfo;
    }
    if(latexOutputHandler) {
        *latexOutputHandler = latexInfo;
    }
    if(!masterDocumentFileName.isEmpty()) {
        KILE_DEBUG_MAIN << "master document defined";
        return m_masterDocumentPreviewInformation;
    }
    KileProject *project = m_ki->docManager()->projectForMember(textInfo->url());
    if(project) {
        KILE_DEBUG_MAIN << "part of a project";
        if(locatedProject) {
            *locatedProject = project;
        }
        if(userStatusHandler) {
            *userStatusHandler = project;
        }
        if(latexOutputHandler) {
            *latexOutputHandler = project;
        }
        if(m_projectToPreviewInformationHash.contains(project)) {
            KILE_DEBUG_MAIN << "project found";
            return m_projectToPreviewInformationHash[project];
        }
        else {
            KILE_DEBUG_MAIN << "project not found";
            return Q_NULLPTR;
        }
    }
    else if(latexInfo && m_latexInfoToPreviewInformationHash.contains(latexInfo)) {
        KILE_DEBUG_MAIN << "not part of a project";
        return m_latexInfoToPreviewInformationHash[latexInfo];
    }
    else {
        KILE_DEBUG_MAIN << "not found";
        return Q_NULLPTR;
    }
}

bool LivePreviewManager::isCurrentDocumentOrProject(KTextEditor::Document *doc)
{
    const KTextEditor::View *currentView = m_ki->viewManager()->currentTextView();

    if(currentView->document() != doc) {
        const KileProject *project = m_ki->docManager()->projectForMember(doc->url());
        const KileProject *currentProject = m_ki->docManager()->activeProject();
        if(!currentProject || (project != currentProject)) {
            return false;
        }
    }

    return true;
}

void LivePreviewManager::showCursorPositionInDocumentViewer()
{
    KTextEditor::View *view = m_ki->viewManager()->currentTextView();
    if(!view) {
        return;
    }
    KileDocument::LaTeXInfo *latexInfo = dynamic_cast<KileDocument::LaTeXInfo*>(m_ki->docManager()->textInfoFor(view->document()));
    if(!latexInfo) {
        return;
    }
    LivePreviewUserStatusHandler *userStatusHandler = Q_NULLPTR;
    findPreviewInformation(latexInfo, Q_NULLPTR, &userStatusHandler);
    if(!userStatusHandler->isLivePreviewEnabled()) {
        return;
    }

    synchronizeViewWithCursor(latexInfo, view, view->cursorPosition(), true); // called from a cursor position change
}

// Note: this method won't open a document again if it's open already
bool LivePreviewManager::ensureDocumentIsOpenInViewer(PreviewInformation *previewInformation, bool *hadToOpen)
{
    if(hadToOpen) {
        *hadToOpen = false;
    }
    const QFile previewFileInfo(previewInformation->previewFile);
    if(!m_ki->viewManager()->viewerPart() || !previewFileInfo.exists() || previewFileInfo.size() == 0) {
        return false;
    }
    const QUrl previewUrl(QUrl::fromLocalFile(previewInformation->previewFile));
    if(m_ki->viewManager()->viewerPart()->url().isEmpty() || m_ki->viewManager()->viewerPart()->url() != previewUrl) {
        KILE_DEBUG_MAIN << "loading again";
        if(m_ki->viewManager()->viewerPart()->openUrl(previewUrl)) {
            if(hadToOpen) {
                *hadToOpen = true;
            }
            // don't forget this
            m_shownPreviewInformation = previewInformation;
            return true;
        }
        else {
            m_shownPreviewInformation = Q_NULLPTR;
            return false;
        }
    }
    return true;
}

void LivePreviewManager::synchronizeViewWithCursor(KileDocument::TextInfo *textInfo, KTextEditor::View *view,
        const KTextEditor::Cursor& newPosition,
        bool calledFromCursorPositionChange)
{
    Q_UNUSED(view);
    KILE_DEBUG_MAIN << "new position " << newPosition;

    PreviewInformation *previewInformation = findPreviewInformation(textInfo);
    if(!previewInformation) {
        KILE_DEBUG_MAIN << "couldn't find preview information for" << textInfo;
        return;
    }

    QFileInfo updatedFileInfo(textInfo->getDoc()->url().toLocalFile());
    QString filePath;
    if(previewInformation->pathToPreviewPathHash.contains(updatedFileInfo.absoluteFilePath())) {
        KILE_DEBUG_MAIN << "found";
        filePath = previewInformation->pathToPreviewPathHash[updatedFileInfo.absoluteFilePath()];
    }
    else {
        KILE_DEBUG_MAIN << "not found";
        filePath = textInfo->getDoc()->url().toLocalFile();
    }
    KILE_DEBUG_MAIN << "filePath" << filePath;

    KILE_DEBUG_MAIN << "previewFile" << previewInformation->previewFile;

    if(!m_ki->viewManager()->viewerPart() || !QFile::exists(previewInformation->previewFile)) {
        return;
    }

    KILE_DEBUG_MAIN << "url" << m_ki->viewManager()->viewerPart()->url();

    if(!ensureDocumentIsOpenInViewer(previewInformation)) {
        clearLivePreview();
        // must happen after the call to 'clearLivePreview' only
        showPreviewFailed();
        emit(livePreviewStopped());
        return;
    }


    // to increase the performance, if 'calledFromCursorPositionChange' is true, we only synchronize when the cursor line
    // has changed from the last synchronization
    // NOTE: the performance of SyncTeX has to be improved if changes in cursor columns should be taken into account as
    //       well (bug 305254)
    if(!calledFromCursorPositionChange || (previewInformation->lastSynchronizationCursor.line() != newPosition.line())) {
        m_ki->viewManager()->showSourceLocationInDocumentViewer(filePath, newPosition.line(), newPosition.column());
        previewInformation->setLastSynchronizationCursor(newPosition.line(), newPosition.column());
    }
}

void LivePreviewManager::reloadDocumentInViewer()
{
    if(!m_ki->viewManager()->viewerPart()) {
        return;
    }

    //FIXME ideally, this method should be integrated in an interface extending Okular...
    QMetaObject::invokeMethod(m_ki->viewManager()->viewerPart(), "reload");
}


static QByteArray computeHashOfDocument(KTextEditor::Document *doc)
{
    QCryptographicHash cryptographicHash(QCryptographicHash::Sha1);
    cryptographicHash.addData(doc->text().toUtf8());
    // allows to catch situations when the URL of the document has changed,
    // e.g. after a save-as operation, which breaks the handling of source
    // references for the displayed document
    cryptographicHash.addData(doc->url().toEncoded());

    return cryptographicHash.result();
}

static void fillTextHashForProject(KileProject *project, QHash<KileDocument::TextInfo*, QByteArray> &textHash)
{
    QList<KileProjectItem*> list = project->items();
    for(QList<KileProjectItem*>::iterator it = list.begin(); it != list.end(); ++it) {
        KileProjectItem *item = *it;

        KileDocument::TextInfo *textInfo = item->getInfo();
        if(!textInfo) {
            continue;
        }
        KTextEditor::Document *document = textInfo->getDoc();
        if(!document) {
            continue;
        }
        textHash[textInfo] = computeHashOfDocument(document);
    }
}

void LivePreviewManager::fillTextHashForMasterDocument(QHash<KileDocument::TextInfo*, QByteArray> &textHash)
{
    // we compute hashes over all the opened files
    QList<KileDocument::TextInfo*> textDocumentInfos = m_ki->docManager()->textDocumentInfos();
    for(QList<KileDocument::TextInfo*>::iterator it = textDocumentInfos.begin(); it != textDocumentInfos.end(); ++it) {
        KileDocument::TextInfo *textInfo = *it;
        if(!textInfo) {
            continue;
        }
        KTextEditor::Document *document = textInfo->getDoc();
        if(!document) {
            continue;
        }
        textHash[textInfo] = computeHashOfDocument(document);
    }
}

void LivePreviewManager::showPreviewCompileIfNecessary(KileDocument::LaTeXInfo *latexInfo, KTextEditor::View *view)
{
    KILE_DEBUG_MAIN;
    // first, stop any running live preview
    stopLivePreview();

    KileProject *project = Q_NULLPTR;
    LivePreviewUserStatusHandler *userStatusHandler = Q_NULLPTR;
    PreviewInformation *previewInformation = findPreviewInformation(latexInfo, &project, &userStatusHandler);
    if(!previewInformation) {
        KILE_DEBUG_MAIN << "not found";
        compilePreview(latexInfo, view);
    }
    else {
        Q_ASSERT(userStatusHandler);
        updateLivePreviewToolActions(userStatusHandler);
        QHash<KileDocument::TextInfo*, QByteArray> newHash;
// 		QString fileName;
// 		QFileInfo fileInfo(view->document()->url().path());
// 		if(previewInformation->pathToPreviewPathHash.contains(fileInfo.absoluteFilePath())) {
// 			KILE_DEBUG_MAIN << "contains";
// 			fileName = previewInformation->pathToPreviewPathHash[fileInfo.absoluteFilePath()];
// 		}
// 		else {
// 			KILE_DEBUG_MAIN << "does not contain";
// 			fileName = fileInfo.absoluteFilePath();
// 		}
// 		KILE_DEBUG_MAIN << "fileName:" << fileName;
        bool masterDocumentSet = !m_ki->getMasterDocumentFileName().isEmpty();

        if(masterDocumentSet) {
            fillTextHashForMasterDocument(newHash);
        }
        else if(project) {
            fillTextHashForProject(project, newHash);
        }
        else {
            newHash[latexInfo] = computeHashOfDocument(view->document());
        }

        if(newHash != previewInformation->textHash || !QFile::exists(previewInformation->previewFile)) {
            KILE_DEBUG_MAIN << "hashes don't match";
            compilePreview(latexInfo, view);
        }
        else {
            KILE_DEBUG_MAIN << "hashes match";
            showPreviewSuccessful();
            synchronizeViewWithCursor(latexInfo, view, view->cursorPosition());
            emit(livePreviewSuccessful());
        }
    }
}

void LivePreviewManager::compilePreview(KileDocument::LaTeXInfo *latexInfo, KTextEditor::View *view)
{
    KILE_DEBUG_MAIN << "updating preview";
    m_ki->viewManager()->setLivePreviewModeForDocumentViewer(true);
    m_runningPathToPreviewPathHash.clear();
    m_runningPreviewPathToPathHash.clear();

    //CAUTION: as saving launches an event loop, we don't want 'compilePreview'
    //         to be called from within 'compilePreview'
    m_documentChangedTimer->blockSignals(true);
    bool saveResult = m_ki->docManager()->fileSaveAll();
    m_documentChangedTimer->blockSignals(false);
    // first, we have to save the documents
    if(!saveResult) {
        displayErrorMessage(i18n("Some documents could not be saved correctly"));
        return;
    }

    // document is new and hasn't been saved yet at all
    if(view->document()->url().isEmpty()) {
        displayErrorMessage(i18n("The document must have been saved before the live preview can be started"));
        return;
    }

    // first, stop any running live preview
    stopLivePreview();

    KileProject *project = Q_NULLPTR;
    LivePreviewUserStatusHandler *userStatusHandler;
    LaTeXOutputHandler *latexOutputHandler;
    PreviewInformation *previewInformation = findPreviewInformation(latexInfo, &project, &userStatusHandler, &latexOutputHandler);
    Q_ASSERT(userStatusHandler);
    Q_ASSERT(latexOutputHandler);
    if(!previewInformation) {
        previewInformation = new PreviewInformation();
        if(!m_ki->getMasterDocumentFileName().isEmpty()) {
            m_masterDocumentPreviewInformation = previewInformation;
        }
        else if(project) {
            bool containsInvalidRelativeItem = false;
            // in the case of a project, we might have to create a similar subdirectory
            // structure as it is present in the real project in order for LaTeX
            // to work correctly
            if(!previewInformation->createSubDirectoriesForProject(project, &containsInvalidRelativeItem)) {
                userStatusHandler->setLivePreviewEnabled(false);
                if(containsInvalidRelativeItem) {
                    displayErrorMessage(i18n("The location of one project item is not relative to the project's base directory\n"
                                             "Live preview for this project has been disabled"), true);
                }
                else {
                    displayErrorMessage(i18n("Failed to create the subdirectory structure"));
                }
                delete previewInformation;
                disablePreview();
                return;
            }
            m_projectToPreviewInformationHash[project] = previewInformation;
        }
        else {
            m_latexInfoToPreviewInformationHash[latexInfo] = previewInformation;
        }
    }

    connect(latexInfo, SIGNAL(aboutToBeDestroyed(KileDocument::TextInfo*)),
            this, SLOT(removeLaTeXInfo(KileDocument::TextInfo*)),
            Qt::UniqueConnection);

    if(project) {
        handleProjectOpened(project); // create the necessary signal-slot connections
    }

    updateLivePreviewToolActions(userStatusHandler);
    KileTool::LivePreviewLaTeX *latex = dynamic_cast<KileTool::LivePreviewLaTeX *>(m_ki->toolManager()->createTool(userStatusHandler->livePreviewTool(),
                                        false));
    if(!latex) {
        KILE_DEBUG_MAIN<< "couldn't create the live preview tool";
        return;
    }

    // important!
    latex->setPartOfLivePreview();
    connect(latex, SIGNAL(done(KileTool::Base*,int,bool)), this, SLOT(toolDone(KileTool::Base*,int,bool)));
    connect(latex, SIGNAL(destroyed()), this, SLOT(toolDestroyed()));

    QFileInfo fileInfo;
    const bool masterDocumentSet = !m_ki->getMasterDocumentFileName().isEmpty();
    if(masterDocumentSet) {
        fileInfo = QFileInfo(m_ki->getMasterDocumentFileName());
    }
    else if(project) {
        fileInfo = QFileInfo(m_ki->getCompileNameForProject(project));
    }
    else {
        fileInfo = QFileInfo(m_ki->getCompileName());
    }

    const QString inputDir = previewInformation->getTempDir() + LIST_SEPARATOR + fileInfo.absolutePath();

    // set value of texinput path (only for LivePreviewManager tools)
    QString texInputPath = KileConfig::teXPaths();
    if(!texInputPath.isEmpty()) {
        texInputPath = inputDir + LIST_SEPARATOR + texInputPath;
    }
    else {
        texInputPath = inputDir;
    }
    latex->setTeXInputPaths(texInputPath);

    QString bibInputPath = KileConfig::bibInputPaths();
    if(!bibInputPath.isEmpty()) {
        bibInputPath = inputDir + LIST_SEPARATOR + bibInputPath;
    }
    else {
        bibInputPath = inputDir;
    }
    latex->setBibInputPaths(bibInputPath);

    QString bstInputPath = KileConfig::bstInputPaths();
    if(!bstInputPath.isEmpty()) {
        bstInputPath = inputDir + LIST_SEPARATOR + bstInputPath;
    }
    else {
        bstInputPath = inputDir;
    }
    latex->setBstInputPaths(bstInputPath);

// 	m_runningPathToPreviewPathHash[fileInfo.absoluteFilePath()] = tempFile;
// 	m_runningPreviewPathToPathHash[tempFile] = fileInfo.absoluteFilePath();

    // don't emit the 'requestSaveAll' signal
// 	latex->removeFlag(EmitSaveAllSignal);

    latex->setTargetDir(previewInformation->getTempDir());
    latex->setSource(fileInfo.absoluteFilePath(), fileInfo.absolutePath());
    latex->setLaTeXOutputHandler(latexOutputHandler);

    latex->prepareToRun();
// 	latex->launcher()->setWorkingDirectory(previewInformation->getTempDir());
    KILE_DEBUG_MAIN << "dir:" << previewInformation->getTempDir();

    m_runningTextView = view;
    m_runningLaTeXInfo = latexInfo;
    m_runningProject = project;
    m_runningPreviewFile = previewInformation->getTempDir() + '/' + latex->target();
    m_runningTextHash.clear();
    if(masterDocumentSet) {
        fillTextHashForMasterDocument(m_runningTextHash);
    }
    else if(project) {
        fillTextHashForProject(project, m_runningTextHash);
    }
    else {
        m_runningTextHash[latexInfo] = computeHashOfDocument(latexInfo->getDoc());
    }
    m_runningPreviewInformation = previewInformation;
    showPreviewRunning();

    // finally, run the tool
    m_ki->toolManager()->run(latex);
    emit(livePreviewRunning());
}

bool LivePreviewManager::isLivePreviewActive() const
{
    KParts::ReadOnlyPart *viewerPart = m_ki->viewManager()->viewerPart();

    return m_runningPreviewInformation
           || (m_shownPreviewInformation
               && viewerPart
               && viewerPart->url() == QUrl::fromLocalFile(m_shownPreviewInformation->previewFile));
}

bool LivePreviewManager::isLivePreviewPossible() const
{
    return true;
}

void LivePreviewManager::handleDocumentOpened(KileDocument::TextInfo *info)
{
    if(m_bootUpMode || !KileConfig::livePreviewEnabled()) {
        return;
    }

    KTextEditor::View *view = m_ki->viewManager()->currentTextView();
    if(view && view->document() == info->getDoc()) {
        handleTextViewActivated(view);
    }
}

void LivePreviewManager::handleTextViewActivated(KTextEditor::View *view, bool clearPreview, bool forceCompilation)
{
    // when a file is currently being opened, we don't react to the view activation signal as the correct live preview
    // tools might not be loaded yet for the document that belongs to 'view'
    if(m_bootUpMode || !KileConfig::livePreviewEnabled() || m_ki->docManager()->isOpeningFile()) {
        return;
    }
    if(clearPreview) {
        stopAndClearPreview();
    }
    else {
        stopLivePreview();
    }
    KileDocument::LaTeXInfo *latexInfo = dynamic_cast<KileDocument::LaTeXInfo*>(m_ki->docManager()->textInfoFor(view->document()));
    if(!latexInfo) {
        return;
    }
    m_documentChangedTimer->stop();

    LivePreviewUserStatusHandler *userStatusHandler = Q_NULLPTR;
    findPreviewInformation(latexInfo, Q_NULLPTR, &userStatusHandler);
    Q_ASSERT(userStatusHandler);
    const bool livePreviewActive = userStatusHandler->isLivePreviewEnabled();
    updateLivePreviewToolActions(userStatusHandler);
    // update the state of the live preview control button
    m_previewForCurrentDocumentAction->setChecked(livePreviewActive);

    if(!livePreviewActive) {
        disablePreview();
    }
    else {
        if(forceCompilation) {
            compilePreview(latexInfo, view);
        }
        else {
            showPreviewCompileIfNecessary(latexInfo, view);
        }
    }
}

void LivePreviewManager::handleTextViewClosed(KTextEditor::View *view, bool wasActiveView)
{
    Q_UNUSED(view);
    Q_UNUSED(wasActiveView);

    if(m_bootUpMode || !KileConfig::livePreviewEnabled()) {
        return;
    }

    // check if there is still an open editor tab
    if(!KTextEditor::Editor::instance()->application()->activeMainWindow()->activeView()) {
        stopAndClearPreview();
    }
}

void LivePreviewManager::refreshLivePreview()
{
    KTextEditor::View *textView = m_ki->viewManager()->currentTextView();
    if(!textView) {
        KILE_DEBUG_MAIN << "no text view is shown; hence, no preview can be shown";
        return;
    }
    handleTextViewActivated(textView, false); // don't automatically clear the preview
}

void LivePreviewManager::recompileLivePreview()
{
    KTextEditor::View *textView = m_ki->viewManager()->currentTextView();
    if(!textView) {
        KILE_DEBUG_MAIN << "no text view is shown; hence, no preview can be shown";
        return;
    }
    handleTextViewActivated(textView, false, true); // don't automatically clear the preview but force compilation
}

void LivePreviewManager::removeLaTeXInfo(KileDocument::TextInfo *textInfo)
{
    KileDocument::LaTeXInfo *latexInfo = dynamic_cast<KileDocument::LaTeXInfo*>(textInfo);
    if(!latexInfo) {
        return;
    }

    if(!m_latexInfoToPreviewInformationHash.contains(latexInfo)) {
        return; // nothing to be done
    }

    PreviewInformation *previewInformation = m_latexInfoToPreviewInformationHash[latexInfo];

    if(m_runningLaTeXInfo == latexInfo) {
        stopLivePreview();
    }

    if(previewInformation == m_shownPreviewInformation) {
        clearLivePreview();
    }

    m_latexInfoToPreviewInformationHash.remove(latexInfo);
    delete previewInformation;
}

void LivePreviewManager::removeProject(KileProject *project)
{
    if(!m_projectToPreviewInformationHash.contains(project)) {
        return; // nothing to be done
    }

    PreviewInformation *previewInformation = m_projectToPreviewInformationHash[project];

    if(m_runningProject == project) {
        stopLivePreview();
    }

    if(previewInformation == m_shownPreviewInformation) {
        clearLivePreview();
    }

    m_projectToPreviewInformationHash.remove(project);
    delete previewInformation;
}

void LivePreviewManager::handleProjectOpened(KileProject *project)
{
    if(m_bootUpMode || !KileConfig::livePreviewEnabled()) {
        return;
    }

    connect(project, SIGNAL(aboutToBeDestroyed(KileProject*)),
            this, SLOT(removeProject(KileProject*)),
            Qt::UniqueConnection);
    connect(project, SIGNAL(projectItemAdded(KileProject*,KileProjectItem*)),
            this, SLOT(handleProjectItemAdded(KileProject*,KileProjectItem*)),
            Qt::UniqueConnection);
    connect(project, SIGNAL(projectItemRemoved(KileProject*,KileProjectItem*)),
            this, SLOT(handleProjectItemRemoved(KileProject*,KileProjectItem*)),
            Qt::UniqueConnection);
}

void LivePreviewManager::handleProjectItemAdditionOrRemoval(KileProject *project, KileProjectItem *item)
{
    if(m_bootUpMode || !KileConfig::livePreviewEnabled()) {
        return;
    }

    KILE_DEBUG_MAIN;
    bool previewNeedsToBeRefreshed = false;

    // we can't use TextInfo pointers here as they might not be set in 'item' yet
    KileDocument::LaTeXInfo *latexInfo = dynamic_cast<KileDocument::LaTeXInfo*>(m_ki->docManager()->textInfoFor(item->url()));
    if(latexInfo && m_latexInfoToPreviewInformationHash.contains(latexInfo)) {
        PreviewInformation *previewInformation = m_latexInfoToPreviewInformationHash[latexInfo];
        if(previewInformation == m_shownPreviewInformation) {
            previewNeedsToBeRefreshed = true;
        }
        removeLaTeXInfo(latexInfo);
    }

    if(m_projectToPreviewInformationHash.contains(project)) {
        PreviewInformation *previewInformation = m_projectToPreviewInformationHash[project];
        if(previewInformation == m_shownPreviewInformation) {
            previewNeedsToBeRefreshed = true;
        }
        removeProject(project);
    }

    // finally, check whether the currently activated text view is the 'modified' project item
    if(!previewNeedsToBeRefreshed) {
        KTextEditor::View *view = m_ki->viewManager()->currentTextView();
        // we can't use TextInfo pointers here as they might not be set in 'item' yet
        if(view && (view->document()->url() == item->url())) {
            previewNeedsToBeRefreshed = true;
        }
    }

    KILE_DEBUG_MAIN << "previewNeedsToBeRefreshed" << previewNeedsToBeRefreshed;
    if(previewNeedsToBeRefreshed) {
        // we can't do this here directly as 'item' might not be fully set up yet (e.g., if it has been added)
        QTimer::singleShot(0, this, SLOT(refreshLivePreview()));
    }
}

void LivePreviewManager::handleProjectItemAdded(KileProject *project, KileProjectItem *item)
{
    if(m_bootUpMode || !KileConfig::livePreviewEnabled()) {
        return;
    }
    KILE_DEBUG_MAIN;

    // the directory structure in the temporary directory will be updated when
    // 'compilePreview' is called; 'handleProjectItemAdditionOrRemoval' will delete
    // PreviewInformation objects
    handleProjectItemAdditionOrRemoval(project, item);
}

void LivePreviewManager::handleProjectItemRemoved(KileProject *project, KileProjectItem *item)
{
    if(m_bootUpMode || !KileConfig::livePreviewEnabled()) {
        return;
    }

    KILE_DEBUG_MAIN;
    handleProjectItemAdditionOrRemoval(project, item);
}

void LivePreviewManager::handleDocumentSavedAs(KTextEditor::View *view, KileDocument::TextInfo *info)
{
    Q_UNUSED(info);

    if(m_bootUpMode || !KileConfig::livePreviewEnabled()) {
        return;
    }

    KTextEditor::View *currentTextView = m_ki->viewManager()->currentTextView();
    if(view != currentTextView) { // might maybe happen at some point...
        // preview will be refreshed the next time that view is activated as the hashes don't
        // match anymore
        return;
    }
    refreshLivePreview();
}

void LivePreviewManager::toolDestroyed()
{
    KILE_DEBUG_MAIN << "\tLivePreviewManager: tool destroyed" << endl;
}

void LivePreviewManager::handleSpawnedChildTool(KileTool::Base *parent, KileTool::Base *child)
{
    Q_UNUSED(parent);

    if(m_bootUpMode || !KileConfig::livePreviewEnabled()) {
        return;
    }

    KILE_DEBUG_MAIN;
    // only connect the signal for tools that are part of live preview!
    if(parent->isPartOfLivePreview()) {
        connect(child, SIGNAL(done(KileTool::Base*,int,bool)), this, SLOT(childToolDone(KileTool::Base*,int,bool)));
    }
}

void LivePreviewManager::toolDone(KileTool::Base *base, int i, bool childToolSpawned)
{
    KILE_DEBUG_MAIN << "\t!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << i << endl;
    KILE_DEBUG_MAIN << "\t!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << i << endl;
    KILE_DEBUG_MAIN << "\tLivePreviewManager: tool done" << base->name() << i << childToolSpawned <<  endl;
    if(i != Success) {
        KILE_DEBUG_MAIN << "tool didn't return successfully, doing nothing";
        showPreviewFailed();
        clearRunningLivePreviewInformation();
        emit(livePreviewStopped());
    }
    // a LaTeX variant must have finished for the preview to be complete
    else if(!childToolSpawned && dynamic_cast<KileTool::LaTeX*>(base)) {
        updatePreviewInformationAfterCompilationFinished();
        clearRunningLivePreviewInformation();
    }
}

void LivePreviewManager::childToolDone(KileTool::Base *base, int i, bool childToolSpawned)
{
    KILE_DEBUG_MAIN << "\t!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << i << endl;
    KILE_DEBUG_MAIN << "\t!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << i << endl;
    KILE_DEBUG_MAIN << "\tLivePreviewManager: child tool done" << base->name() << i << childToolSpawned << endl;
    if(!m_ki->viewManager()->viewerPart()) {
        return;
    }
    if(i != Success) {
        KILE_DEBUG_MAIN << "tool didn't return successfully, doing nothing";
        showPreviewFailed();
        clearRunningLivePreviewInformation();
        emit(livePreviewStopped());
    }
    // a LaTeX variant must have finished for the preview to be complete
    else if(!childToolSpawned && dynamic_cast<KileTool::LaTeX*>(base)) {
        updatePreviewInformationAfterCompilationFinished();
        clearRunningLivePreviewInformation();
    }
}

void LivePreviewManager::updatePreviewInformationAfterCompilationFinished()
{
    if(!m_runningPreviewInformation) { // LivePreview has been stopped in the meantime
        return;
    }

    m_shownPreviewInformation = m_runningPreviewInformation;
    m_shownPreviewInformation->pathToPreviewPathHash = m_runningPathToPreviewPathHash;
    m_shownPreviewInformation->previewPathToPathHash = m_runningPreviewPathToPathHash;
    m_shownPreviewInformation->textHash = m_runningTextHash;
    m_shownPreviewInformation->previewFile = m_runningPreviewFile;

    m_runningPreviewInformation = Q_NULLPTR;

    bool hadToOpen = false;
    if(!ensureDocumentIsOpenInViewer(m_shownPreviewInformation, &hadToOpen)) {
        clearLivePreview();
        // must happen after the call to 'clearLivePreview' only
        showPreviewFailed();
        emit(livePreviewStopped());
        return;
    }

    // as 'ensureDocumentIsOpenInViewer' won't reload when the document is open
    // already, we have to do it here
    if(!hadToOpen) {
        reloadDocumentInViewer();
    }

    if(m_ki->viewManager()->isSynchronisingCursorWithDocumentViewer()) {
        synchronizeViewWithCursor(m_runningLaTeXInfo, m_runningTextView, m_runningTextView->cursorPosition());
    }

    showPreviewSuccessful();
    emit(livePreviewSuccessful());
}

void LivePreviewManager::displayErrorMessage(const QString &text, bool clearFirst)
{
    if(clearFirst) {
        m_ki->errorHandler()->clearMessages();
    }
    m_ki->errorHandler()->printMessage(KileTool::Error, text, i18n("LivePreview"));
}

}

