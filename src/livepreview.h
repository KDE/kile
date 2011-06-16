/********************************************************************************
  Copyright (C) 2011 by Michel Ludwig (michel.ludwig@kdemail.net)
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
#include "kiletool.h"
#include "editorextension.h"
#include "widgets/previewwidget.h"

#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>

#include <KLed>
#include <KToggleAction>
#include <KTempDir>

namespace KileDocument { class LaTeXInfo; }

namespace KileTool
{

class LivePreviewManager : public QObject
{
	Q_OBJECT

public:
	// has to be instatiated after the view manager only!
	LivePreviewManager(KileInfo *ki, KActionCollection *ac, QWidget *livePreviewPartParent);
	~LivePreviewManager();

	void readConfig(KConfig *config);
	void writeConfig();

	bool run(const QString &text,const QString &textfilename,int startrow);
	bool isRunning();

	void cursorPositionUpdated(KileDocument::LaTeXInfo *info, KTextEditor::View *view, const KTextEditor::Cursor& newPosition);
	void compilePreview(KileDocument::LaTeXInfo *info, KTextEditor::View *view);
	void showPreviewCompileIfNecessary(KileDocument::LaTeXInfo *info, KTextEditor::View *view);

	KParts::ReadOnlyPart* livePreviewPart() const { return m_livePreviewPart; }

	bool isLivePreviewPossible() const;
  /**
   * run (text, textfilename, startrow) works with the 
   * default configuration for QuickPreview. This method
   * supports a forth parameter to choose the configuration as
   * comma - separated string as you can see them in run (text, textfilename, startrow)
   *
   * It is also possible not to specify a viewer, so the viewer is not
   * executed.
   *
   * @param text         Text to preview
   * @param textfilename Filename of the document
   * @param startrow     Position of preview text in the document
   * @param spreviewlist user defined configuration, e.g. "PreviewLaTeX,DVItoPS,,,ps" (with no preview)
   * @return             true if method succeeds, else false
   */
// 	bool run (const QString &text, const QString &textfilename, int startrow, const QString &spreviewlist);
  
	QWidget* getControlToolBar();

public Q_SLOTS:
	void handleCursorPositionChanged(KTextEditor::View *view, const KTextEditor::Cursor &pos);
	void handleTextChanged(KTextEditor::Document *doc);

private Q_SLOTS:
	void handleDocumentModificationTimerTimeout();

	void toolDestroyed();
	void toolDone(KileTool::Base *base, int i, bool childToolSpawned);
	void childToolDone(KileTool::Base *base, int i, bool childToolSpawned);

	void handleActivatedSourceReference(const QString& absFileName, int line, int col);
	void handleTextViewActivated(KTextEditor::View *view);
	void handleTextInfoDestroyed(QObject *obj);

	void handleSpawnedChildTool(KileTool::Base *parent, KileTool::Base *child);

	void synchronizeViewWithCursorActionToggled(bool b);
	void previewForCurrentDocumentActionToggled(bool b);

	void handleLivePreviewPartDestroyed();

private:
	class PreviewInformation;

	KileInfo *m_ki;
	KToolBar *m_controlToolBar;
	KLed *m_previewStatusLed;
	KToggleAction *m_synchronizeViewWithCursorAction, *m_previewForCurrentDocumentAction;
	QTimer *m_ledBlinkingTimer, *m_documentChangedTimer;

	KParts::ReadOnlyPart *m_livePreviewPart;

	QHash<QString, QString> m_runningPathToPreviewPathHash;
	QHash<QString, QString> m_runningPreviewPathToPathHash;
	QString m_runningPreviewFile;
	KileDocument::LaTeXInfo *m_runningLaTeXInfo;
	KTextEditor::View *m_runningTextView;
	PreviewInformation *m_runningPreviewInformation;
	QList<QPair<QString, QByteArray> > m_runningTextHashList;

	PreviewInformation *m_shownPreviewInformation;

	QHash<KileDocument::TextInfo*,PreviewInformation*> m_textInfoToPreviewInformationHash;
// 	QHash<KileDocument::TextInfo*,PreviewInformation*> m_projectToPreviewInformationHash;

	void createLivePreviewPart(QWidget *parent);

	void displayErrorMessage(const QString &text);

	void createActions(KActionCollection *ac);
	void createControlToolBar();
	void synchronizeViewWithCursor(KileDocument::LaTeXInfo *info, KTextEditor::View *view, const KTextEditor::Cursor& newPosition);

	void stopAndClearPreview();

	void showPreviewDisabled();
	void showPreviewRunning();
	void showPreviewFailed();
	void showPreviewSuccessful();

	void stopLivePreview();
};

}

#endif
