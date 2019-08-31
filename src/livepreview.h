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

#ifndef LIVEPREVIEW_H
#define LIVEPREVIEW_H

#include "documentinfo.h"
#include "kileinfo.h"
#include "kileproject.h"
#include "kiletool.h"
#include "editorextension.h"
#include "widgets/previewwidget.h"

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>

#include <KLed>
#include <KToggleAction>
#include <QTemporaryDir>

namespace KileDocument {
class TextInfo;
}

namespace KileTool
{

class LivePreviewManager : public QObject
{
    Q_OBJECT

public:
    // has to be instatiated after the view manager only!
    LivePreviewManager(KileInfo *ki, KActionCollection *ac);
    ~LivePreviewManager();

    // live preview won't be run in 'boot up' mode, which is enabled by default
    void disableBootUpMode();

    void readConfig(KConfig *config);
    void writeConfig();
    static void readLivePreviewStatusSettings(KConfigGroup &configGroup, LivePreviewUserStatusHandler *handler);
    static void writeLivePreviewStatusSettings(KConfigGroup &configGroup, LivePreviewUserStatusHandler *handler);

    void compilePreview(KileDocument::LaTeXInfo *info, KTextEditor::View *view);
    void showPreviewCompileIfNecessary(KileDocument::LaTeXInfo *info, KTextEditor::View *view);

    bool isLivePreviewActive() const;
    bool isLivePreviewPossible() const;

    bool isLivePreviewEnabledForCurrentDocument();
    void setLivePreviewEnabledForCurrentDocument(bool b);

    void buildLivePreviewMenu(KConfig *config);

    QString getPreviewFile() const;
    inline QUrl getPreviewFileURL() const {
        return QUrl::fromLocalFile(getPreviewFile());
    }

Q_SIGNALS:
    void livePreviewSuccessful();
    void livePreviewRunning();
    void livePreviewStopped(); // disabled or stopped

public Q_SLOTS:
    void handleTextChanged(KTextEditor::Document *doc);
    void handleDocumentSavedOrUploaded(KTextEditor::Document *doc, bool savedAs);

    void handleMasterDocumentChanged();

    void recompileLivePreview();
    void refreshLivePreview();

    void showCursorPositionInDocumentViewer();

private Q_SLOTS:
    void handleDocumentModificationTimerTimeout();

    // TextInfo* object due to the signal 'aboutToBeDestroyed(KileDocument::TextInfo*)'
    void removeLaTeXInfo(KileDocument::TextInfo *info);
    void removeProject(KileProject *project);

    void toolDestroyed();
    void toolDone(KileTool::Base *base, int i, bool childToolSpawned);
    void childToolDone(KileTool::Base *base, int i, bool childToolSpawned);

    void handleTextViewActivated(KTextEditor::View *view, bool clearPreview = true, bool forceCompilation = false);
    void handleTextViewClosed(KTextEditor::View *view, bool wasActiveView);

    void handleDocumentOpened(KileDocument::TextInfo *info);

    void handleProjectOpened(KileProject *project);
    void handleProjectItemAdded(KileProject *project, KileProjectItem *item);
    void handleProjectItemRemoved(KileProject *project, KileProjectItem *item);

    void handleDocumentSavedAs(KTextEditor::View*, KileDocument::TextInfo*);

    void handleSpawnedChildTool(KileTool::Base *parent, KileTool::Base *child);

    void previewForCurrentDocumentActionTriggered(bool b);

    void livePreviewToolActionTriggered();

private:
    class PreviewInformation;

    KileInfo *m_ki;
    bool m_bootUpMode;
    QPointer<KLed> m_previewStatusLed;
    KToggleAction *m_previewForCurrentDocumentAction;
    QAction *m_recompileLivePreviewAction;
    QTimer *m_ledBlinkingTimer, *m_documentChangedTimer;

    QHash<QString, QString> m_runningPathToPreviewPathHash;
    QHash<QString, QString> m_runningPreviewPathToPathHash;
    QString m_runningPreviewFile;
    KileDocument::LaTeXInfo *m_runningLaTeXInfo;
    KTextEditor::View *m_runningTextView;
    KileProject *m_runningProject;
    PreviewInformation *m_runningPreviewInformation;
    QHash<KileDocument::TextInfo*, QByteArray> m_runningTextHash;

    PreviewInformation *m_shownPreviewInformation;

    QHash<KileDocument::LaTeXInfo*, PreviewInformation*> m_latexInfoToPreviewInformationHash;
    QHash<KileProject*, PreviewInformation*> m_projectToPreviewInformationHash;
    PreviewInformation *m_masterDocumentPreviewInformation;

    // all the members required to handle tool actions for live preview
    QHash<ToolConfigPair, QAction *> m_livePreviewToolToActionHash;
    QHash<QAction *, ToolConfigPair> m_actionToLivePreviewToolHash;
    QActionGroup *m_livePreviewToolActionGroup;
    QLinkedList<QAction *> m_livePreviewToolActionList;

    PreviewInformation* findPreviewInformation(KileDocument::TextInfo *textInfo, KileProject* *locatedProject = Q_NULLPTR,
            LivePreviewUserStatusHandler* *userStatusHandler = Q_NULLPTR,
            LaTeXOutputHandler* *latexOutputHandler = Q_NULLPTR);
    bool isCurrentDocumentOrProject(KTextEditor::Document *doc);

    void updatePreviewInformationAfterCompilationFinished();

    void displayErrorMessage(const QString &text, bool clearFirst = false);

    void createActions(KActionCollection *ac);
    void populateViewerControlToolBar();
    void synchronizeViewWithCursor(KileDocument::TextInfo *info, KTextEditor::View *view,
                                   const KTextEditor::Cursor& newPosition,
                                   bool calledFromCursorPositionChange = false);

    void stopAndClearPreview();

    void showPreviewDisabled();
    void showPreviewRunning();
    void showPreviewFailed();
    void showPreviewSuccessful();
    void showPreviewOutOfDate();

    void stopLivePreview();
    void clearLivePreview();

    void deleteAllLivePreviewInformation();

    void handleProjectItemAdditionOrRemoval(KileProject *project, KileProjectItem *item);

    void fillTextHashForMasterDocument(QHash<KileDocument::TextInfo*, QByteArray> &textHash);

    void disablePreview();

    void clearRunningLivePreviewInformation();

    void updateLivePreviewToolActions(LivePreviewUserStatusHandler *statusHandler);
    void setLivePreviewToolActionsEnabled(bool b);

    bool ensureDocumentIsOpenInViewer(PreviewInformation *previewInformation, bool *hadToOpen = Q_NULLPTR);
    void reloadDocumentInViewer();

};

}

#endif
