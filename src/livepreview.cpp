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

#include "livepreview.h"

#include "config.h"

#include <QCryptographicHash>
#include <QDir>
#include <QHBoxLayout>
#include <QMap>
#include <QFile>
#include <QFileInfo>
#include <QTextCodec>
#include <QTextStream>
#include <QTimer>

#include <KActionCollection>
#include <KLocale>
#include <KStandardDirs>
#include <KTempDir>
#include <KTextEditor/CodeCompletionInterface>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KToolBar>

#ifdef HAVE_VIEWERINTERFACE_H
#include <okular/interfaces/viewerinterface.h>
#endif

#include "kiledebug.h"
#include "kiletool_enums.h"
#include "kiledocmanager.h"
#include "kileviewmanager.h"
#include "widgets/logwidget.h"


namespace KileTool
{

class LivePreviewManager::PreviewInformation {
public:
	PreviewInformation(KileDocument::TextInfo *textInfo)
	: m_textInfo(textInfo),
	  m_previewEnabled(KileConfig::previewEnabledForFreshlyOpenedDocuments())
	{
		m_tempDir = new KTempDir(KStandardDirs::locateLocal("tmp", "kile-livepreview"));
	}

	~PreviewInformation() {
		delete m_tempDir;
	}

	QString getTempDir() const {
		return m_tempDir->name();
	}

	void clearPreviewPathMappings() {
		pathToPreviewPathHash.clear();
		previewPathToPathHash.clear();
	}

	void setPreviewEnabled(bool b) {
		m_previewEnabled = b;
	}

	bool isPreviewEnabled() const {
		return m_previewEnabled;
	}

private:
	KileDocument::TextInfo *m_textInfo;
	KTempDir *m_tempDir;
	bool m_previewEnabled;

public:
	QHash<QString, QString> pathToPreviewPathHash;
	QHash<QString, QString> previewPathToPathHash;
	QString previewFile;
	QList<QPair<QString, QByteArray> > textHashList;
};

LivePreviewManager::LivePreviewManager(KileInfo *ki, KActionCollection *ac, QWidget *livePreviewPartParent)
 : m_ki(ki),
   m_controlToolBar(NULL),
   m_previewStatusLed(NULL),
   m_previewForCurrentDocumentAction(NULL),
   m_runningLaTeXInfo(NULL), m_runningTextView(NULL),
   m_runningPreviewInformation(NULL), m_shownPreviewInformation(NULL)
{
	connect(m_ki->viewManager(), SIGNAL(textViewActivated(KTextEditor::View*)),
	        this, SLOT(handleTextViewActivated(KTextEditor::View*)));
	connect(m_ki->toolManager(), SIGNAL(childToolSpawned(KileTool::Base*,KileTool::Base*)),
	        this, SLOT(handleSpawnedChildTool(KileTool::Base*, KileTool::Base*)));
	createActions(ac);
	createControlToolBar();

	m_ledBlinkingTimer = new QTimer(this);
	m_ledBlinkingTimer->setSingleShot(false);
	m_ledBlinkingTimer->setInterval(500);
	connect(m_ledBlinkingTimer, SIGNAL(timeout()), m_previewStatusLed, SLOT(toggle()));

	m_documentChangedTimer = new QTimer(this);
	m_documentChangedTimer->setSingleShot(true);
	connect(m_documentChangedTimer, SIGNAL(timeout()), this, SLOT(handleDocumentModificationTimerTimeout()));

	showPreviewDisabled();
	createLivePreviewPart(livePreviewPartParent);
}

LivePreviewManager::~LivePreviewManager()
{
	stopAndClearPreview();
	for(QHash<KileDocument::TextInfo*,PreviewInformation*>::iterator i = m_textInfoToPreviewInformationHash.begin();
	    i != m_textInfoToPreviewInformationHash.end(); ++i) {
		delete i.value();
	}
	delete m_livePreviewPart;
}

void LivePreviewManager::createActions(KActionCollection *ac)
{
	m_synchronizeViewWithCursorAction = new KToggleAction(KIcon("document-swap"), i18n("Synchronize Cursor Position with Preview Document"), this);
	// just to get synchronization back when the sync feature is activated (again)
	connect(m_synchronizeViewWithCursorAction, SIGNAL(toggled(bool)), this, SLOT(synchronizeViewWithCursorActionToggled(bool)));
	ac->addAction("synchronize_cursor_preview", m_synchronizeViewWithCursorAction);

	m_previewForCurrentDocumentAction = new KToggleAction(KIcon("document-preview"), i18n("Toggle Live Preview for Current Document"), this);
	m_previewForCurrentDocumentAction->setChecked(true);
	connect(m_previewForCurrentDocumentAction, SIGNAL(toggled(bool)), this, SLOT(previewForCurrentDocumentActionToggled(bool)));
	ac->addAction("preview_current_document", m_synchronizeViewWithCursorAction);
}

void LivePreviewManager::synchronizeViewWithCursorActionToggled(bool b)
{
	KTextEditor::View *view = m_ki->viewManager()->currentTextView();
	if(!b || !view) {
		return;
	}
	KileDocument::LaTeXInfo *latexInfo = dynamic_cast<KileDocument::LaTeXInfo*>(m_ki->docManager()->textInfoFor(view->document()));
	if(latexInfo) {
		// showPreviewCompileIfNecessary synchronizes the view with the cursor
		showPreviewCompileIfNecessary(latexInfo, view);
	}
}

void LivePreviewManager::previewForCurrentDocumentActionToggled(bool b)
{
	KTextEditor::View *view = m_ki->viewManager()->currentTextView();
	if(!view) {
		return;
	}
	KileDocument::LaTeXInfo *latexInfo = dynamic_cast<KileDocument::LaTeXInfo*>(m_ki->docManager()->textInfoFor(view->document()));
	if(!latexInfo) {
		return;
	}
	if(m_textInfoToPreviewInformationHash.contains(latexInfo)) {
		PreviewInformation *previewInformation = m_textInfoToPreviewInformationHash[latexInfo];
		previewInformation->setPreviewEnabled(b);
	}

	if(b) {
		showPreviewCompileIfNecessary(latexInfo, view);
	}
	else {
		stopAndClearPreview();
	}
}

void LivePreviewManager::stopAndClearPreview()
{
	KILE_DEBUG();
	showPreviewDisabled();
	stopLivePreview();
	if(m_livePreviewPart) {
		m_livePreviewPart->closeUrl();
		KILE_DEBUG() << "url shown: " << m_livePreviewPart->url();
	}
	m_shownPreviewInformation = NULL;
}

void LivePreviewManager::stopLivePreview()
{
	m_documentChangedTimer->stop();
	m_ki->toolManager()->stopLivePreview();

	m_runningPathToPreviewPathHash.clear();
	m_runningPreviewPathToPathHash.clear();
	m_runningPreviewFile.clear();
	m_runningLaTeXInfo = NULL;
	m_runningTextView = NULL;
	m_runningPreviewInformation = NULL;
	m_runningTextHashList.clear();
}

void LivePreviewManager::readConfig(KConfig *config)
{
	m_synchronizeViewWithCursorAction->setChecked(KileConfig::synchronizeCursorWithView());
}

void LivePreviewManager::writeConfig()
{
	KileConfig::setSynchronizeCursorWithView(m_synchronizeViewWithCursorAction->isChecked());
}

QWidget* LivePreviewManager::getControlToolBar()
{
	return m_controlToolBar;
}

void LivePreviewManager::createControlToolBar()
{
	m_controlToolBar = new KToolBar(NULL, false, false);
	m_controlToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
	m_controlToolBar->setFloatable(false);
	m_controlToolBar->setMovable(false);
	m_controlToolBar->setIconDimensions(KIconLoader::SizeSmall);

	m_controlToolBar->addAction(m_previewForCurrentDocumentAction);
	m_controlToolBar->addAction(m_synchronizeViewWithCursorAction);

	m_controlToolBar->addSeparator();

	m_previewStatusLed = new KLed(m_controlToolBar);
	m_controlToolBar->addWidget(m_previewStatusLed);
}

void LivePreviewManager::handleCursorPositionChanged(KTextEditor::View *view, const KTextEditor::Cursor &pos)
{
	KileDocument::LaTeXInfo *latexInfo = dynamic_cast<KileDocument::LaTeXInfo*>(m_ki->docManager()->textInfoFor(view->document()));
	if(!latexInfo) {
		return;
	}
	cursorPositionUpdated(latexInfo, view, pos);
}

void LivePreviewManager::handleTextChanged(KTextEditor::Document *doc)
{
	Q_UNUSED(doc);
	m_documentChangedTimer->start(500);
}

void LivePreviewManager::handleDocumentModificationTimerTimeout()
{
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

	compilePreview(latexInfo, view);
}

void LivePreviewManager::showPreviewDisabled()
{
	KILE_DEBUG();
	m_ledBlinkingTimer->stop();
	m_previewStatusLed->off();
}

void LivePreviewManager::showPreviewRunning()
{
	KILE_DEBUG();
	m_previewStatusLed->setColor(QColor(Qt::yellow));
	m_previewStatusLed->off();
	m_ledBlinkingTimer->start();
}

void LivePreviewManager::showPreviewFailed()
{
	KILE_DEBUG();
	m_ledBlinkingTimer->stop();
	m_previewStatusLed->on();
	m_previewStatusLed->setColor(QColor(Qt::red));
}

void LivePreviewManager::showPreviewSuccessful()
{
	KILE_DEBUG();
	m_ledBlinkingTimer->stop();
	m_previewStatusLed->on();
	m_previewStatusLed->setColor(QColor(Qt::green));
}

void LivePreviewManager::cursorPositionUpdated(KileDocument::LaTeXInfo *latexInfo, KTextEditor::View *view, const KTextEditor::Cursor& newPosition)
{
	KILE_DEBUG() << "new position " << newPosition;
	if(!m_synchronizeViewWithCursorAction->isChecked()) {
		return;
	}

	if(!latexInfo) {
		return;
	}

	if(!m_textInfoToPreviewInformationHash.contains(latexInfo) || !m_textInfoToPreviewInformationHash[latexInfo]->isPreviewEnabled()) {
		return;
	}

	synchronizeViewWithCursor(latexInfo, view, newPosition);
}

void LivePreviewManager::synchronizeViewWithCursor(KileDocument::LaTeXInfo *info, KTextEditor::View *view, const KTextEditor::Cursor& newPosition)
{
	KILE_DEBUG() << "new position " << newPosition;

// 	KileTool::ForwardDVI *forwardDVI = dynamic_cast<KileTool::ForwardDVI*>(m_ki->toolFactory()->create("ForwardPDF", false));
// 	if(!forwardDVI) {
// 		displayErrorMessage(i18n("Could not run '%1' for LivePreviewManager.", QString("LaTeX")));
// 		KILE_DEBUG() << "error";
// 		return;
// 	}

// 	forwardDVI->setSource(m_ki->getName(info->getDoc()));
// 	forwardDVI->prepareToRun();
	if(!m_textInfoToPreviewInformationHash.contains(info)) {
		KILE_DEBUG() << "couldn't find preview information for" << info;
		return;
	}
	PreviewInformation *previewInformation = m_textInfoToPreviewInformationHash[info];

	QFileInfo updatedFileInfo(info->getDoc()->url().toLocalFile());
	QString filePath;
	if(previewInformation->pathToPreviewPathHash.contains(updatedFileInfo.absoluteFilePath())) {
	  KILE_DEBUG() << "contains";
		filePath = previewInformation->pathToPreviewPathHash[updatedFileInfo.absoluteFilePath()];
	}
	else {
	  KILE_DEBUG() << "does not contain";
		filePath = info->getDoc()->url().toLocalFile();
	}
	KILE_DEBUG() << "filePath" << filePath;

// 	QString absoluteTarget = "file:" + info->getLivePreviewTempDir() + '/' + forwardDVI->target() + "#src:" + QString::number(newPosition.line()) + filepath;

// 	KILE_DEBUG() << forwardDVI->paramDict()["%absolute_target"];
// 	KILE_DEBUG() << "absoluteTarget" << absoluteTarget;
	if(m_livePreviewPart) {
	KILE_DEBUG() << "url" << m_livePreviewPart->url();
	}
	KILE_DEBUG() << "currentFileShown" << previewInformation->previewFile;

	KUrl previewUrl(KUrl(previewInformation->previewFile));

// 	if(QFileInfo(m_currentFileShown).exists()) {
	if(!m_livePreviewPart || !QFile::exists(previewInformation->previewFile)) {
		return;
	}

	if(m_livePreviewPart->url().isEmpty() || m_livePreviewPart->url() != previewUrl) {
// 		m_livePreviewPart->openUrl(KUrl(absoluteTarget));
		m_livePreviewPart->openUrl(previewUrl);
		// don't forget this
		m_shownPreviewInformation = previewInformation;
	}

	Okular::ViewerInterface *v = dynamic_cast<Okular::ViewerInterface*>(m_livePreviewPart);
	if(v) {
		v->showSourceLocation(filePath, newPosition.line(), newPosition.column());
	}
}

static QByteArray computeHash(const QString& string)
{
	QCryptographicHash cryptographicHash(QCryptographicHash::Sha1);
	cryptographicHash.addData(string.toUtf8());

	return cryptographicHash.result();
}

void LivePreviewManager::showPreviewCompileIfNecessary(KileDocument::LaTeXInfo *info, KTextEditor::View *view)
{
	// first, stop any running live preview
	stopLivePreview();

	if(!m_textInfoToPreviewInformationHash.contains(info)) {
		KILE_DEBUG() << "not found";
		compilePreview(info, view);
	}
	else {
		PreviewInformation *previewInformation = m_textInfoToPreviewInformationHash[info];
		QByteArray newHash = computeHash(view->document()->text());
		QList<QPair<QString, QByteArray> > newHashList;
		QString fileName;
		QFileInfo fileInfo(view->document()->url().path());
		if(previewInformation->pathToPreviewPathHash.contains(fileInfo.absoluteFilePath())) {
			KILE_DEBUG() << "contains";
			fileName = previewInformation->pathToPreviewPathHash[fileInfo.absoluteFilePath()];
		}
		else {
			KILE_DEBUG() << "does not contain";
			fileName = fileInfo.absoluteFilePath();
		}
		KILE_DEBUG() << "fileName:" << fileName;
		newHashList << QPair<QString, QByteArray>(fileName, newHash);
		if(newHashList != previewInformation->textHashList || !QFile::exists(previewInformation->previewFile)) {
			compilePreview(info, view);
		}
		else {
			showPreviewSuccessful();
			synchronizeViewWithCursor(info, view, view->cursorPosition());
		}
	}
}

void LivePreviewManager::compilePreview(KileDocument::LaTeXInfo *info, KTextEditor::View *view)
{
	KILE_DEBUG() << "updating preview";
	m_runningPathToPreviewPathHash.clear();
	m_runningPreviewPathToPathHash.clear();

	// document is new and hasn't been saved yet at all
	if(view->document()->url().isEmpty()) {
		displayErrorMessage(i18n("The document must have been saved before live preview can be launched"));
		return;
	}

	// first, stop any running live preview
	stopLivePreview();

	connect(info, SIGNAL(destroyed(QObject*)),
	        this, SLOT(handleTextInfoDestroyed(QObject*)),
	        Qt::UniqueConnection);

	PreviewInformation *previewInformation;
	if(!m_textInfoToPreviewInformationHash.contains(info)) {
		previewInformation = new PreviewInformation(info);
		m_textInfoToPreviewInformationHash[info] = previewInformation;
	}
	else {
		previewInformation = m_textInfoToPreviewInformationHash[info];
	}

	if(!previewInformation->isPreviewEnabled()) {
		return;
	}

	KileTool::Base *latex = m_ki->toolFactory()->create("LivePreviewPDFLaTeX", false);
	if(!latex) {
		KILE_DEBUG()<< "couldn't create the tool";
		return;
	}
	// important!
	latex->setPartOfLivePreview();
	connect(latex, SIGNAL(done(KileTool::Base*,int,bool)), this, SLOT(toolDone(KileTool::Base*,int,bool)));
	connect(latex, SIGNAL(destroyed()), this, SLOT(toolDestroyed()));

	KILE_DEBUG() << "getCompileName" << m_ki->getCompileName();
	QFileInfo fileInfo(m_ki->getCompileName());
	const QString fileName = fileInfo.fileName();
	KILE_DEBUG() << "fileName" << fileName;

	const QString inputDir = previewInformation->getTempDir() + ':' + fileInfo.absolutePath();

	// set value of texinput path (only for LivePreviewManager tools)
	QString texInputPath = KileConfig::teXPaths();
	if(!texInputPath.isEmpty()) {
		texInputPath = inputDir + ':' + texInputPath;
	}
	else {
		texInputPath = inputDir;
	}
	latex->setTeXInputPaths(texInputPath);

	QString bibInputPath = KileConfig::bibInputPaths();
	if(!bibInputPath.isEmpty()) {
		bibInputPath = inputDir + ':' + bibInputPath;
	}
	else {
		bibInputPath = inputDir;
	}
	latex->setBibInputPaths(bibInputPath);

	QString bstInputPath = KileConfig::bstInputPaths();
	if(!bstInputPath.isEmpty()) {
		bstInputPath = inputDir + ':' + bstInputPath;
	}
	else {
		bstInputPath = inputDir;
	}
	latex->setBstInputPaths(bstInputPath);

	// prepare tools: previewlatex
// 	latex->setPreviewInfo(textfilename, startrow, preamblelines + 1);

// 	m_runningPathToPreviewPathHash[fileInfo.absoluteFilePath()] = tempFile;
// 	m_runningPreviewPathToPathHash[tempFile] = fileInfo.absoluteFilePath();

	// don't emit the 'requestSaveAll' signal
// 	latex->removeFlag(EmitSaveAllSignal);

	latex->setSource(fileInfo.absoluteFilePath(), previewInformation->getTempDir());
// 	latex->setTargetDir(previewInformation->getTempDir());
	latex->prepareToRun();
// 	latex->launcher()->setWorkingDirectory(previewInformation->getTempDir());
	KILE_DEBUG() << "dir:" << previewInformation->getTempDir();

	m_runningTextView = view;
	m_runningLaTeXInfo = info;
	m_runningPreviewFile = previewInformation->getTempDir() + '/' + latex->target();
	m_runningTextHashList.clear();
	m_runningTextHashList << QPair<QString, QByteArray>(fileInfo.absoluteFilePath(), computeHash(info->getDoc()->text()));
	m_runningPreviewInformation = previewInformation;
	showPreviewRunning();
	if(m_ki->toolManager()->run(latex) != KileTool::Running) {
		return;
	}
}

void LivePreviewManager::createLivePreviewPart(QWidget *parent)
{
	KPluginLoader pluginLoader("okularpart");
	KPluginFactory *factory = pluginLoader.factory();
	if (!factory) {
		KILE_DEBUG() << i18n("Could not find the Okular library.");
		m_livePreviewPart = NULL;
		return;
	}
	else {
		QVariantList argList;
		argList << "ViewerWidget";
		m_livePreviewPart = factory->create<KParts::ReadOnlyPart>(parent, argList);
		Okular::ViewerInterface *viewerInterface = dynamic_cast<Okular::ViewerInterface*>(m_livePreviewPart);
		if(!viewerInterface) {
			// Okular doesn't provide the ViewerInterface
			delete m_livePreviewPart;
			m_livePreviewPart = NULL;
			return;
		}
		viewerInterface->setWatchFileModeEnabled(false);
		connect(m_livePreviewPart, SIGNAL(openSourceReference(const QString&, int, int)), this, SLOT(handleActivatedSourceReference(const QString&, int, int)));
		connect(m_livePreviewPart, SIGNAL(destroyed()), this, SLOT(handleLivePreviewPartDestroyed()));
	}
}

void LivePreviewManager::handleLivePreviewPartDestroyed()
{
	m_livePreviewPart = NULL;
}

bool LivePreviewManager::isLivePreviewPossible() const
{
#ifdef LIVEPREVIEW_POSSIBLE
	return true;
#else
	return false;
#endif
}

void LivePreviewManager::handleActivatedSourceReference(const QString& absFileName, int line, int col)
{
	KILE_DEBUG() << "absFileName:" << absFileName << "line:" << line << "column:" << col;
	QString fileName;
	if(!m_shownPreviewInformation) {
		return;
	}
	KILE_DEBUG() << m_shownPreviewInformation->previewPathToPathHash;
	if(m_shownPreviewInformation->previewPathToPathHash.contains(absFileName)) {
		KILE_DEBUG() << "contains";
		fileName = m_shownPreviewInformation->previewPathToPathHash[absFileName];
	}
	else {
		KILE_DEBUG() << "does not contain";
		fileName = absFileName;
	}
	KILE_DEBUG() << "fileName:" << fileName;
	KileDocument::TextInfo *textInfo = m_ki->docManager()->textInfoFor(fileName);
	if(!textInfo) {
		return;
	}
	KTextEditor::View *view = m_ki->viewManager()->textView(textInfo);
	if(!view) {
		return;
	}
	view->setCursorPosition(KTextEditor::Cursor(line, col));
	m_ki->viewManager()->switchToTextView(view, true);
}

void LivePreviewManager::handleTextViewActivated(KTextEditor::View *view)
{
	stopAndClearPreview();
	KileDocument::LaTeXInfo *latexInfo = dynamic_cast<KileDocument::LaTeXInfo*>(m_ki->docManager()->textInfoFor(view->document()));
	if(!latexInfo) {
		return;
	}
	m_documentChangedTimer->stop();
	bool actionIsToggled = false;
	if(m_textInfoToPreviewInformationHash.contains(latexInfo)) {
		PreviewInformation *previewInformation = m_textInfoToPreviewInformationHash[latexInfo];
		actionIsToggled = (m_previewForCurrentDocumentAction->isChecked() && !previewInformation->isPreviewEnabled())
		                  || (!m_previewForCurrentDocumentAction->isChecked() && previewInformation->isPreviewEnabled());
		// the potentially triggered action will handle everything
		m_previewForCurrentDocumentAction->setChecked(previewInformation->isPreviewEnabled());
		if(!actionIsToggled && previewInformation->isPreviewEnabled()) {
			showPreviewCompileIfNecessary(latexInfo, view);
		}
	}
	else if(KileConfig::previewEnabledForFreshlyOpenedDocuments()) {
		showPreviewCompileIfNecessary(latexInfo, view);
	}
}


void LivePreviewManager::handleTextInfoDestroyed(QObject *obj)
{
	KileDocument::LaTeXInfo *latexInfo = dynamic_cast<KileDocument::LaTeXInfo*>(obj);
	if(!latexInfo) {
		return;
	}
	if(!m_textInfoToPreviewInformationHash.contains(latexInfo)) {
		return; // nothing to be done
	}

	PreviewInformation *previewInformation = m_textInfoToPreviewInformationHash[latexInfo];

	if(m_runningLaTeXInfo == latexInfo) {
		stopLivePreview();
	}

	if(previewInformation == m_shownPreviewInformation) {
		stopAndClearPreview();
	}

	m_textInfoToPreviewInformationHash.remove(latexInfo);
	delete previewInformation;
}

void LivePreviewManager::toolDestroyed()
{
	KILE_DEBUG() << "\tLivePreviewManager: tool destroyed" << endl;
}

void LivePreviewManager::toolDone(KileTool::Base *base, int i, bool childToolSpawned)
{
	KILE_DEBUG() << "\t!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << i << endl;
	KILE_DEBUG() << "\t!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << i << endl;
	KILE_DEBUG() << "\tLivePreviewManager: tool done" << base->name() << i << childToolSpawned <<  endl;
	if(i != Success) {
		KILE_DEBUG() << "tool didn't return successfully, doing nothing";
		showPreviewFailed();
	}
	else if(!childToolSpawned) {
		m_shownPreviewInformation = m_runningPreviewInformation;
		m_shownPreviewInformation->pathToPreviewPathHash = m_runningPathToPreviewPathHash;
		m_shownPreviewInformation->previewPathToPathHash = m_runningPreviewPathToPathHash;
		m_shownPreviewInformation->textHashList = m_runningTextHashList;
		m_shownPreviewInformation->previewFile = m_runningPreviewFile;
		m_shownPreviewInformation->setPreviewEnabled(true);
		if(QFile::exists(m_shownPreviewInformation->previewFile) && m_livePreviewPart) {
			showPreviewSuccessful();
			m_livePreviewPart->openUrl(KUrl(m_shownPreviewInformation->previewFile));
			synchronizeViewWithCursor(m_runningLaTeXInfo, m_runningTextView, m_runningTextView->cursorPosition());
		}
	}
}

void LivePreviewManager::handleSpawnedChildTool(KileTool::Base *parent, KileTool::Base *child)
{
	KILE_DEBUG() << "\t!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
	KILE_DEBUG() << "\t!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
	KILE_DEBUG() << "\t!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
	KILE_DEBUG() << "\t!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
	KILE_DEBUG() << "\tLivePreviewManager: handleSpawnedChildTool" << endl;
	if(dynamic_cast<KileTool::LaTeX*>(child)) {
		connect(child, SIGNAL(done(KileTool::Base*,int,bool)), this, SLOT(childToolDone(KileTool::Base*,int,bool)));
	}
}

void LivePreviewManager::childToolDone(KileTool::Base *base, int i, bool childToolSpawned)
{
	KILE_DEBUG() << "\t!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << i << endl;
	KILE_DEBUG() << "\t!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << i << endl;
	KILE_DEBUG() << "\tLivePreviewManager: child tool done" << base->name() << i << childToolSpawned << endl;
	if(!m_livePreviewPart) {
		return;
	}
	if(i != Success) {
		KILE_DEBUG() << "tool didn't return successfully, doing nothing";
		showPreviewFailed();
	}
	else if(!childToolSpawned) {
		m_shownPreviewInformation = m_runningPreviewInformation;
		m_shownPreviewInformation->pathToPreviewPathHash = m_runningPathToPreviewPathHash;
		m_shownPreviewInformation->previewPathToPathHash = m_runningPreviewPathToPathHash;
		m_shownPreviewInformation->textHashList = m_runningTextHashList;
		m_shownPreviewInformation->previewFile = m_runningPreviewFile;
		m_shownPreviewInformation->setPreviewEnabled(true);
		if(QFile::exists(m_shownPreviewInformation->previewFile)) {
			showPreviewSuccessful();
			m_livePreviewPart->openUrl(KUrl(m_shownPreviewInformation->previewFile));
			synchronizeViewWithCursor(m_runningLaTeXInfo, m_runningTextView, m_runningTextView->cursorPosition());
		}
	}
}

void LivePreviewManager::displayErrorMessage(const QString &text)
{
	m_ki->logWidget()->printMessage(KileTool::Error, text, i18n("LivePreviewManager"));
}

}

#include "livepreview.moc"
