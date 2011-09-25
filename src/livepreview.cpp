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
	PreviewInformation()
	: m_previewEnabled(KileConfig::previewEnabledForFreshlyOpenedDocuments())
	{
		initTemporaryDirectory();
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
	KTempDir *m_tempDir;
	bool m_previewEnabled;

	void initTemporaryDirectory() {
		// work around bug in the SyncTeX implementation of PDFTeX (can't rename file)
		// should be: KStandardDirs::locateLocal("tmp", "kile-livepreview")
		m_tempDir = new KTempDir(KStandardDirs::locateLocal("appdata", "livepreview/preview-"));
	}

public:
	QHash<QString, QString> pathToPreviewPathHash;
	QHash<QString, QString> previewPathToPathHash;
	QString previewFile;
	QHash<KileDocument::TextInfo*, QByteArray> textHash;
};

LivePreviewManager::LivePreviewManager(KileInfo *ki, KActionCollection *ac, QWidget *livePreviewPartParent)
 : m_ki(ki),
   m_controlToolBar(NULL),
   m_previewStatusLed(NULL),
   m_previewForCurrentDocumentAction(NULL),
   m_runningLaTeXInfo(NULL), m_runningTextView(NULL), m_runningProject(NULL),
   m_runningPreviewInformation(NULL), m_shownPreviewInformation(NULL), m_masterDocumentPreviewInformation(NULL)
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
	deleteAllLivePreviewInformation();
	delete m_livePreviewPart;
}

void LivePreviewManager::createActions(KActionCollection *ac)
{
	m_synchronizeViewWithCursorAction = new KToggleAction(KIcon("document-swap"), i18n("Synchronize Cursor Position with Preview Document"), this);
	// just to get synchronization back when the sync feature is activated (again)
	connect(m_synchronizeViewWithCursorAction, SIGNAL(toggled(bool)), this, SLOT(synchronizeViewWithCursorActionToggled(bool)));
	ac->addAction("synchronize_cursor_preview", m_synchronizeViewWithCursorAction);

	m_previewForCurrentDocumentAction = new KToggleAction(KIcon("document-preview"), i18n("Toggle Live Preview for Current Document or Project"), this);
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
	PreviewInformation *previewInformation = findPreviewInformation(latexInfo);

	if(previewInformation) {
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
	stopLivePreview();
	clearLivePreview();
}

void LivePreviewManager::clearLivePreview()
{
	showPreviewDisabled();
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
	m_runningProject = NULL;
	m_runningTextView = NULL;
	m_runningPreviewInformation = NULL;
	m_runningTextHash.clear();
}

void LivePreviewManager::deleteAllLivePreviewInformation()
{
	// first, we have to make sure that nothing is shown anymore,
	// and that no preview is running
	stopAndClearPreview();

	// and now we can delete all the 'PreviewInformation' objects
	delete m_masterDocumentPreviewInformation;
	m_masterDocumentPreviewInformation = NULL;

	for(QHash<KileDocument::TextInfo*,PreviewInformation*>::iterator i = m_textInfoToPreviewInformationHash.begin();
	    i != m_textInfoToPreviewInformationHash.end(); ++i) {
		delete i.value();
	}

	for(QHash<KileProject*,PreviewInformation*>::iterator i = m_projectToPreviewInformationHash.begin();
	    i != m_projectToPreviewInformationHash.end(); ++i) {
		delete i.value();
	}
	m_textInfoToPreviewInformationHash.clear();
	m_projectToPreviewInformationHash.clear();
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

	connect(m_controlToolBar, SIGNAL(destroyed()), this, SLOT(handleControlToolbarDestroyed()));

	m_previewStatusLed = new KLed(m_controlToolBar);
	m_controlToolBar->addWidget(m_previewStatusLed);

	connect(m_previewStatusLed, SIGNAL(destroyed()), this, SLOT(handlePreviewStatusLedDestroyed()));
}

void LivePreviewManager::handleMasterDocumentChanged()
{
	deleteAllLivePreviewInformation();
	refreshLivePreview();
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
	KILE_DEBUG();
	Q_UNUSED(doc);
	m_documentChangedTimer->start(500);
}

void LivePreviewManager::handleDocumentModificationTimerTimeout()
{
	KILE_DEBUG();
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

	PreviewInformation *previewInformation = findPreviewInformation(latexInfo);
	if(previewInformation) {
		if(previewInformation->isPreviewEnabled()) {
			compilePreview(latexInfo, view);
		}
	}
	else if(KileConfig::previewEnabledForFreshlyOpenedDocuments()) {
		compilePreview(latexInfo, view);
	}

}

void LivePreviewManager::showPreviewDisabled()
{
	KILE_DEBUG();
	m_ledBlinkingTimer->stop();
	if(m_previewStatusLed) {
		m_previewStatusLed->off();
	}
}

void LivePreviewManager::showPreviewRunning()
{
	KILE_DEBUG();
	if(m_previewStatusLed) {
		m_previewStatusLed->setColor(QColor(Qt::yellow));
		m_previewStatusLed->off();
	}
	m_ledBlinkingTimer->start();
}

void LivePreviewManager::showPreviewFailed()
{
	KILE_DEBUG();
	m_ledBlinkingTimer->stop();
	if(m_previewStatusLed) {
		m_previewStatusLed->on();
		m_previewStatusLed->setColor(QColor(Qt::red));
	}
}

void LivePreviewManager::showPreviewSuccessful()
{
	KILE_DEBUG();
	m_ledBlinkingTimer->stop();
	if(m_previewStatusLed) {
		m_previewStatusLed->on();
		m_previewStatusLed->setColor(QColor(Qt::green));
	}
}

LivePreviewManager::PreviewInformation* LivePreviewManager::findPreviewInformation(KileDocument::LaTeXInfo *latexInfo,
                                                                                   KileProject* *locatedProject)
{
	const QString masterDocumentFileName = m_ki->getMasterDocumentFileName();
	if(!masterDocumentFileName.isEmpty()) {
		KILE_DEBUG() << "master document defined";
		return m_masterDocumentPreviewInformation;
	}
	KileProject *project = m_ki->docManager()->projectForMember(latexInfo->url());
	if(project) {
		KILE_DEBUG() << "part of a project";
		if(locatedProject) {
			*locatedProject = project;
		}
		if(m_projectToPreviewInformationHash.contains(project)) {
			KILE_DEBUG() << "project found";
			return m_projectToPreviewInformationHash[project];
		}
		else {
			KILE_DEBUG() << "project not found";
			return NULL;
		}
	}
	else if(m_textInfoToPreviewInformationHash.contains(latexInfo)) {
		KILE_DEBUG() << "not part of a project";
		return m_textInfoToPreviewInformationHash[latexInfo];
	}
	else {
		KILE_DEBUG() << "not found";
		return NULL;
	}
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

	PreviewInformation *previewInformation = findPreviewInformation(latexInfo);
	if(!previewInformation || !previewInformation->isPreviewEnabled()) {
		return;
	}

	synchronizeViewWithCursor(latexInfo, view, newPosition);
}

void LivePreviewManager::synchronizeViewWithCursor(KileDocument::LaTeXInfo *info, KTextEditor::View *view, const KTextEditor::Cursor& newPosition)
{
	KILE_DEBUG() << "new position " << newPosition;

	PreviewInformation *previewInformation = findPreviewInformation(info);
	if(!previewInformation) {
		KILE_DEBUG() << "couldn't find preview information for" << info;
		return;
	}

	QFileInfo updatedFileInfo(info->getDoc()->url().toLocalFile());
	QString filePath;
	if(previewInformation->pathToPreviewPathHash.contains(updatedFileInfo.absoluteFilePath())) {
		KILE_DEBUG() << "found";
		filePath = previewInformation->pathToPreviewPathHash[updatedFileInfo.absoluteFilePath()];
	}
	else {
		KILE_DEBUG() << "not found";
		filePath = info->getDoc()->url().toLocalFile();
	}
	KILE_DEBUG() << "filePath" << filePath;


	if(m_livePreviewPart) {
		KILE_DEBUG() << "url" << m_livePreviewPart->url();
	}
	KILE_DEBUG() << "currentFileShown" << previewInformation->previewFile;

	KUrl previewUrl(KUrl(previewInformation->previewFile));

	if(!m_livePreviewPart || !QFile::exists(previewInformation->previewFile)) {
		return;
	}

	if(m_livePreviewPart->url().isEmpty() || m_livePreviewPart->url() != previewUrl) {
		KILE_DEBUG() << "loading again";
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

static QByteArray computeHashOfText(const QString& string)
{
	QCryptographicHash cryptographicHash(QCryptographicHash::Sha1);
	cryptographicHash.addData(string.toUtf8());

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
		textHash[textInfo] = computeHashOfText(document->text());
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
		textHash[textInfo] = computeHashOfText(document->text());
	}
}

void LivePreviewManager::showPreviewCompileIfNecessary(KileDocument::LaTeXInfo *latexInfo, KTextEditor::View *view)
{
	KILE_DEBUG();
	// first, stop any running live preview
	stopLivePreview();

	KileProject *project = NULL;
	PreviewInformation *previewInformation = findPreviewInformation(latexInfo, &project);
	if(!previewInformation) {
		KILE_DEBUG() << "not found";
		compilePreview(latexInfo, view);
	}
	else {
		QHash<KileDocument::TextInfo*, QByteArray> newHash;
// 		QString fileName;
// 		QFileInfo fileInfo(view->document()->url().path());
// 		if(previewInformation->pathToPreviewPathHash.contains(fileInfo.absoluteFilePath())) {
// 			KILE_DEBUG() << "contains";
// 			fileName = previewInformation->pathToPreviewPathHash[fileInfo.absoluteFilePath()];
// 		}
// 		else {
// 			KILE_DEBUG() << "does not contain";
// 			fileName = fileInfo.absoluteFilePath();
// 		}
// 		KILE_DEBUG() << "fileName:" << fileName;
		bool masterDocumentSet = !m_ki->getMasterDocumentFileName().isEmpty();

		if(masterDocumentSet) {
			fillTextHashForMasterDocument(newHash);
		}
		else if(project) {
			fillTextHashForProject(project, newHash);
		}
		else {
			newHash[latexInfo] = computeHashOfText(view->document()->text());
		}

		if(newHash != previewInformation->textHash || !QFile::exists(previewInformation->previewFile)) {
			KILE_DEBUG() << "hashes don't match";
			compilePreview(latexInfo, view);
		}
		else {
			KILE_DEBUG() << "hashes match";
			showPreviewSuccessful();
			synchronizeViewWithCursor(latexInfo, view, view->cursorPosition());
		}
	}
}

void LivePreviewManager::compilePreview(KileDocument::LaTeXInfo *info, KTextEditor::View *view)
{
	KILE_DEBUG() << "updating preview";
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

	KileProject *project = NULL;
	PreviewInformation *previewInformation = findPreviewInformation(info, &project);
	if(!previewInformation) {
		previewInformation = new PreviewInformation();
		if(!m_ki->getMasterDocumentFileName().isEmpty()) {
			m_masterDocumentPreviewInformation = previewInformation;
		}
		else if(project) {
			m_projectToPreviewInformationHash[project] = previewInformation;
		}
		else {
			m_textInfoToPreviewInformationHash[info] = previewInformation;
		}
	}

	connect(info, SIGNAL(destroyed(QObject*)),
	        this, SLOT(handleTextInfoDestroyed(QObject*)),
	        Qt::UniqueConnection);

	if(project) {
		connect(info, SIGNAL(destroyed(QObject*)),
		        this, SLOT(handleProjectDestroyed(QObject*)),
		        Qt::UniqueConnection);
		connect(project, SIGNAL(projectItemAdded(KileProject*,KileProjectItem*)),
		        this, SLOT(handleProjectItemAdded(KileProject*,KileProjectItem*)),
		        Qt::UniqueConnection);
		connect(project, SIGNAL(projectItemRemoved(KileProject*,KileProjectItem*)),
		        this, SLOT(handleProjectItemRemoved(KileProject*,KileProjectItem*)),
		        Qt::UniqueConnection);
	}

	KileTool::LivePreviewLaTeX *latex = dynamic_cast<KileTool::LivePreviewLaTeX *>(m_ki->toolManager()->createTool("LivePreviewPDFLaTeX", QString(), false));
	if(!latex) {
		KILE_DEBUG()<< "couldn't create the tool";
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

// 	m_runningPathToPreviewPathHash[fileInfo.absoluteFilePath()] = tempFile;
// 	m_runningPreviewPathToPathHash[tempFile] = fileInfo.absoluteFilePath();

	// don't emit the 'requestSaveAll' signal
// 	latex->removeFlag(EmitSaveAllSignal);

	latex->setTargetDir(previewInformation->getTempDir());
	latex->setSource(fileInfo.absoluteFilePath(), fileInfo.absolutePath());
// 	latex->setTargetDir(previewInformation->getTempDir());
	latex->prepareToRun();
// 	latex->launcher()->setWorkingDirectory(previewInformation->getTempDir());
	KILE_DEBUG() << "dir:" << previewInformation->getTempDir();

	m_runningTextView = view;
	m_runningLaTeXInfo = info;
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
		m_runningTextHash[info] = computeHashOfText(info->getDoc()->text());
	}
	m_runningPreviewInformation = previewInformation;
	showPreviewRunning();

	// finally, run the tool
	m_ki->toolManager()->run(latex);
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

void LivePreviewManager::handlePreviewStatusLedDestroyed()
{
	m_previewStatusLed = NULL;
}

void LivePreviewManager::handleControlToolbarDestroyed()
{
	m_controlToolBar = NULL;
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
		KILE_DEBUG() << "found";
		fileName = m_shownPreviewInformation->previewPathToPathHash[absFileName];
	}
	else {
		KILE_DEBUG() << "not found";
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
	PreviewInformation *previewInformation = findPreviewInformation(latexInfo);
	if(previewInformation) {
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
	else {
		clearLivePreview();
	}
}

void LivePreviewManager::refreshLivePreview()
{
	KTextEditor::View *textView = m_ki->viewManager()->currentTextView();
	if(!textView) {
		KILE_DEBUG() << "no text view is shown; hence, no preview can be shown";
		return;
	}
	handleTextViewActivated(textView);
}

void LivePreviewManager::handleTextInfoDestroyed(QObject *obj)
{
	KileDocument::LaTeXInfo *latexInfo = dynamic_cast<KileDocument::LaTeXInfo*>(obj);
	if(!latexInfo) {
		return;
	}
	removeTextInfo(latexInfo);
}

void LivePreviewManager::handleProjectDestroyed(QObject *obj)
{
	KileProject *project = dynamic_cast<KileProject*>(obj);
	if(!project) {
		return;
	}
	removeProject(project);
}

void LivePreviewManager::removeTextInfo(KileDocument::TextInfo *info)
{
	if(!m_textInfoToPreviewInformationHash.contains(info)) {
		return; // nothing to be done
	}

	PreviewInformation *previewInformation = m_textInfoToPreviewInformationHash[info];

	if(m_runningLaTeXInfo == info) {
		stopLivePreview();
	}

	if(previewInformation == m_shownPreviewInformation) {
		clearLivePreview();
	}

	m_textInfoToPreviewInformationHash.remove(info);
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


void LivePreviewManager::handleProjectItemAdditionOrRemoval(KileProject *project, KileProjectItem *item)
{
	KILE_DEBUG();
	bool previewNeedsToBeRefreshed = false;

	// we can't use TextInfo pointers here as they might not be set in 'item' yet
	KileDocument::TextInfo *info = m_ki->docManager()->textInfoForURL(item->url());
	if(info) {
		if(m_textInfoToPreviewInformationHash.contains(info)) {
			PreviewInformation *previewInformation = m_textInfoToPreviewInformationHash[info];
			if(previewInformation == m_shownPreviewInformation) {
				previewNeedsToBeRefreshed = true;
			}
		}
		removeTextInfo(info);
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
		if(view->document()->url() == item->url()) {
			previewNeedsToBeRefreshed = true;
		}
	}

	KILE_DEBUG() << "previewNeedsToBeRefreshed" << previewNeedsToBeRefreshed;
	if(previewNeedsToBeRefreshed) {
		// we can't do this here directly as 'item' might not be fully set up yet (e.g., if it has been added)
		QTimer::singleShot(0, this, SLOT(refreshLivePreview()));
	}
}

void LivePreviewManager::handleProjectItemAdded(KileProject *project, KileProjectItem *item)
{
	KILE_DEBUG();
	handleProjectItemAdditionOrRemoval(project, item);
}

void LivePreviewManager::handleProjectItemRemoved(KileProject *project, KileProjectItem *item)
{
	KILE_DEBUG();
	handleProjectItemAdditionOrRemoval(project, item);
}

void LivePreviewManager::toolDestroyed()
{
	KILE_DEBUG() << "\tLivePreviewManager: tool destroyed" << endl;
}

void LivePreviewManager::handleSpawnedChildTool(KileTool::Base *parent, KileTool::Base *child)
{
	KILE_DEBUG();
	connect(child, SIGNAL(done(KileTool::Base*,int,bool)), this, SLOT(childToolDone(KileTool::Base*,int,bool)));
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
	// a LaTeX variant must have finished for the preview to be complete
	else if(!childToolSpawned && dynamic_cast<KileTool::LaTeX*>(base)) {
		updatePreviewInformationAfterCompilationFinished();
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
	// a LaTeX variant must have finished for the preview to be complete
	else if(!childToolSpawned && dynamic_cast<KileTool::LaTeX*>(base)) {
		updatePreviewInformationAfterCompilationFinished();
	}
}

void LivePreviewManager::updatePreviewInformationAfterCompilationFinished()
{
	m_shownPreviewInformation = m_runningPreviewInformation;
	m_shownPreviewInformation->pathToPreviewPathHash = m_runningPathToPreviewPathHash;
	m_shownPreviewInformation->previewPathToPathHash = m_runningPreviewPathToPathHash;
	m_shownPreviewInformation->textHash = m_runningTextHash;
	m_shownPreviewInformation->previewFile = m_runningPreviewFile;
	m_shownPreviewInformation->setPreviewEnabled(true);
	if(m_livePreviewPart && QFile::exists(m_shownPreviewInformation->previewFile)) {
		showPreviewSuccessful();
		m_livePreviewPart->openUrl(KUrl(m_shownPreviewInformation->previewFile));
		synchronizeViewWithCursor(m_runningLaTeXInfo, m_runningTextView, m_runningTextView->cursorPosition());
	}
}

void LivePreviewManager::displayErrorMessage(const QString &text)
{
	m_ki->logWidget()->printMessage(KileTool::Error, text, i18n("LivePreviewManager"));
}

}

#include "livepreview.moc"
