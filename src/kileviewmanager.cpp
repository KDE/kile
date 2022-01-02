/**************************************************************************
*   Copyright (C) 2004 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)   *
*             (C) 2006-2018 by Michel Ludwig (michel.ludwig@kdemail.net)  *
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kileviewmanager.h"
#include <config.h>

#include <okular/interfaces/viewerinterface.h>

#include <KAcceleratorManager>
#include <KActionCollection>
#include <KConfigGroup>
#include <KIconLoader>
#include <kio/global.h>
#include <KLocalizedString>
#include <KMessageBox>
#include <KTextEditor/Application>
#include <KTextEditor/CodeCompletionInterface>
#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/MainWindow>
#include <KTextEditor/View>
#include <KToolBar>
#include <KToggleAction>
#include <KXMLGUIClient>
#include <KXMLGUIFactory>

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QLayout>
#include <QMenu>
#include <QMimeData>
#include <QPixmap>
#include <QSplitter>
#include <QTimer> //for QTimer::singleShot trick
#include <QToolButton>

#include "editorkeysequencemanager.h"
#include "kileinfo.h"
#include "kileconstants.h"
#include "kileproject.h"
#include "kiledocmanager.h"
#include "kileextensions.h"
#include "kiletool_enums.h"
#include "usermenu/usermenu.h"
#include "livepreview.h"
#include "widgets/projectview.h"
#include "widgets/structurewidget.h"
#include "editorextension.h"
#include "plaintolatexconverter.h"
#include "widgets/previewwidget.h"
#include "quickpreview.h"
#include "codecompletion.h"


namespace KileView
{

bool sortDocuments(const KTextEditor::View * const lhs, const KTextEditor::View * const rhs)
{
    return lhs->document()->documentName().compare(rhs->document()->documentName(), Qt::CaseInsensitive) < 0;
}

//BEGIN DocumentViewerWindow

DocumentViewerWindow::DocumentViewerWindow(QWidget *parent, Qt::WindowFlags f)
    : KMainWindow(parent, f)
{
}

DocumentViewerWindow::~DocumentViewerWindow()
{
}

void DocumentViewerWindow::showEvent(QShowEvent *event)
{
    KMainWindow::showEvent(event);
    emit visibilityChanged(true);
}

void DocumentViewerWindow::closeEvent(QCloseEvent *event)
{
    KMainWindow::closeEvent(event);
    emit visibilityChanged(false);
}

//END DocumentViewerWindow

Manager::Manager(KileInfo *info, KActionCollection *actionCollection, QObject *parent, const char *name) :
    QObject(parent),
    m_ki(info),
// 	m_projectview(Q_NULLPTR),
    m_tabsAndEditorWidget(Q_NULLPTR),
    m_tabBar(Q_NULLPTR),
    m_documentListButton(Q_NULLPTR),
    m_viewerPartWindow(Q_NULLPTR),
    m_widgetStack(Q_NULLPTR),
    m_pasteAsLaTeXAction(Q_NULLPTR),
    m_convertToLaTeXAction(Q_NULLPTR),
    m_quickPreviewAction(Q_NULLPTR),
    m_showCursorPositionInViewerAction(Q_NULLPTR),
    m_viewerControlToolBar(Q_NULLPTR),
    m_cursorPositionChangedTimer(Q_NULLPTR),
    m_clearLastShownSourceLocationTimer(Q_NULLPTR),
    m_synchronizeViewWithCursorAction(Q_NULLPTR)
{
    setObjectName(name);
    createViewerPart(actionCollection);

    m_showCursorPositionInViewerAction = new QAction(QIcon::fromTheme("go-jump-symbolic"), i18n("Show Cursor Position in Viewer"), this);
    connect(m_showCursorPositionInViewerAction, &QAction::triggered, this, &KileView::Manager::showCursorPositionInDocumentViewer);
    actionCollection->addAction("show_cursor_position_in_document_viewer", m_showCursorPositionInViewerAction);

    m_synchronizeViewWithCursorAction = new KToggleAction(i18n("Synchronize Cursor Position with Viewer"), this);
    connect(m_synchronizeViewWithCursorAction, &KToggleAction::toggled, this, &KileView::Manager::synchronizeViewWithCursorActionToggled);
    connect(m_synchronizeViewWithCursorAction, &KToggleAction::changed,
    this, [=] () {
        m_showCursorPositionInViewerAction->setEnabled(!m_synchronizeViewWithCursorAction->isChecked());
    });
    actionCollection->addAction("synchronize_cursor_with_document_viewer", m_synchronizeViewWithCursorAction);

    m_cursorPositionChangedTimer = new QTimer(this);
    m_cursorPositionChangedTimer->setSingleShot(true);
    connect(m_cursorPositionChangedTimer, &QTimer::timeout, this, &KileView::Manager::handleCursorPositionChangedTimeout);

    m_clearLastShownSourceLocationTimer = new QTimer(this);
    m_clearLastShownSourceLocationTimer->setInterval(3000);
    m_clearLastShownSourceLocationTimer->setSingleShot(true);
    connect(m_clearLastShownSourceLocationTimer, &QTimer::timeout, this, &KileView::Manager::clearLastShownSourceLocationInDocumentViewer);

    createViewerControlToolBar();
}

Manager::~Manager()
{
    KILE_DEBUG_MAIN;

    // the parent of the widget might be Q_NULLPTR; see 'destroyDocumentViewerWindow()'
    if(m_viewerPart) {
        delete m_viewerPart->widget();
        delete m_viewerPart;
    }

    destroyDocumentViewerWindow();
}

KTextEditor::View * Manager::textViewAtTab(int index) const
{
    return m_tabBar->tabData(index).value<KTextEditor::View*>();
}

void Manager::createViewerControlToolBar()
{
    m_viewerControlToolBar = new KToolBar(Q_NULLPTR, false, false);
    m_viewerControlToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_viewerControlToolBar->setFloatable(false);
    m_viewerControlToolBar->setMovable(false);
    m_viewerControlToolBar->setIconDimensions(KIconLoader::SizeSmall);

    m_viewerControlToolBar->addAction(m_showCursorPositionInViewerAction);
}

void Manager::setClient(KXMLGUIClient *client)
{
    m_client = client;
    if(Q_NULLPTR == m_client->actionCollection()->action("popup_pasteaslatex")) {
        m_pasteAsLaTeXAction = new QAction(i18n("Paste as LaTe&X"), this);
        connect(m_pasteAsLaTeXAction, &QAction::triggered, this, &Manager::pasteAsLaTeX);
    }
    if(Q_NULLPTR == m_client->actionCollection()->action("popup_converttolatex")) {
        m_convertToLaTeXAction = new QAction(i18n("Convert Selection to &LaTeX"), this);
        connect(m_convertToLaTeXAction, &QAction::triggered, this, &Manager::convertSelectionToLaTeX);
    }
    if(Q_NULLPTR == m_client->actionCollection()->action("popup_quickpreview")) {
        m_quickPreviewAction = new QAction(this);
        connect(m_quickPreviewAction, &QAction::triggered, this, &Manager::quickPreviewPopup);
    }
}

void Manager::readConfig(QSplitter *splitter)
{
    // we might have to change the location of the viewer part
    setupViewerPart(splitter);

    setDocumentViewerVisible(KileConfig::showDocumentViewer());

    m_synchronizeViewWithCursorAction->setChecked(KileConfig::synchronizeCursorWithView());

    Okular::ViewerInterface *viewerInterface = dynamic_cast<Okular::ViewerInterface*>(m_viewerPart.data());
    if(viewerInterface && !m_ki->livePreviewManager()->isLivePreviewActive()) {
        viewerInterface->setWatchFileModeEnabled(KileConfig::watchFileForDocumentViewer());
        // also reload the document; this is necessary for switching back on watch-file mode as otherwise
        // it would only enabled after the document has been reloaded anyway
        if(m_viewerPart->url().isValid()) {
            m_viewerPart->openUrl(m_viewerPart->url());
        }
    }
}

void Manager::writeConfig()
{
    if(m_viewerPart) {
        KileConfig::setShowDocumentViewer(isViewerPartShown());
    }
    if(m_viewerPartWindow) {
        KConfigGroup group(KSharedConfig::openConfig(), "KileDocumentViewerWindow");
        m_viewerPartWindow->saveMainWindowSettings(group);
    }

    KileConfig::setSynchronizeCursorWithView(m_synchronizeViewWithCursorAction->isChecked());
}

void Manager::setTabsAndEditorVisible(bool b)
{
    m_tabsAndEditorWidget->setVisible(b);
}

QWidget * Manager::createTabs(QWidget *parent)
{
    m_widgetStack = new QStackedWidget(parent);
    DropWidget *emptyDropWidget = new DropWidget(m_widgetStack);
    m_widgetStack->insertWidget(0, emptyDropWidget);
    connect(emptyDropWidget, &KileView::DropWidget::testCanDecode, this, static_cast<void (Manager::*)(const QDragEnterEvent *, bool &)>(&Manager::testCanDecodeURLs));
    connect(emptyDropWidget, &KileView::DropWidget::receivedDropEvent, m_ki->docManager(), &KileDocument::Manager::openDroppedURLs);
    connect(emptyDropWidget, &KileView::DropWidget::mouseDoubleClick, [=]() {
        m_ki->docManager()->fileNew();
    });
    m_tabBar = new QTabBar(parent);
    QWidget *tabBarWidget = new QWidget();
    tabBarWidget->setLayout(new QHBoxLayout);
    tabBarWidget->layout()->setSpacing(0);
    tabBarWidget->layout()->setContentsMargins(0, 0, 0, 0);
    KAcceleratorManager::setNoAccel(m_tabBar);

    // quick menu for all open documents
    m_documentListButton = new QToolButton(parent);
    m_documentListButton->setIcon(QIcon::fromTheme("format-list-unordered"));
    m_documentListButton->setMenu(new QMenu(parent));
    m_documentListButton->setPopupMode(QToolButton::InstantPopup);
    m_documentListButton->setAutoRaise(true);
    m_documentListButton->setToolTip(i18n("Show sorted list of opened documents"));
    m_documentListButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    // lambda: update context menu
    connect(m_documentListButton->menu(), &QMenu::aboutToShow, [=]() {
        qDeleteAll(m_documentListButton->menu()->actions());
        m_documentListButton->menu()->clear();

        // create a lexicographically sorted list
        QVector<KTextEditor::View*> views;
        views.reserve(m_tabBar->count());
        for(int i = 0; i < m_tabBar->count(); ++i) {
            views << textViewAtTab(i);
        }
        std::sort(views.begin(), views.end(), sortDocuments);

        foreach(KTextEditor::View* view, views) {
            QAction *action = m_documentListButton->menu()->addAction(view->document()->documentName());
            action->setData(QVariant::fromValue(view));
        }
    });
    // lambda: handle context menu action triggers
    connect(m_documentListButton->menu(), &QMenu::triggered, [=](QAction *action) {
        KTextEditor::View *view = action->data().value<KTextEditor::View*>();
        Q_ASSERT(view);
        m_tabBar->setCurrentIndex(tabIndexOf(view));
    });
    // lambda: menu button is enabled if and only if at least two documents are open
    connect(this, &KileView::Manager::textViewCreated, [=]() {
        m_documentListButton->setEnabled(m_tabBar->count() > 1);
    });
    connect(this, &KileView::Manager::textViewClosed, [=]() {
        m_documentListButton->setEnabled(m_tabBar->count() > 1);
        m_cursorPositionChangedTimer->stop();
    });
    connect(this, &KileView::Manager::textViewClosed, [=]() {
        m_documentListButton->setEnabled(m_tabBar->count() > 1);
    });
    tabBarWidget->layout()->addWidget(m_documentListButton);

    // tabbar
    m_tabBar->setFocusPolicy(Qt::ClickFocus);
    m_tabBar->setMovable(true);
    m_tabBar->setTabsClosable(true);
    m_tabBar->setUsesScrollButtons(true);
    m_tabBar->setFocus();
    m_tabBar->setContextMenuPolicy(Qt::CustomContextMenu);
    tabBarWidget->layout()->addWidget(m_tabBar);

    // connect tabbar with document views
    connect(m_tabBar, &QTabBar::currentChanged, this, &Manager::currentTabChanged);
    connect(m_tabBar, &QTabBar::tabCloseRequested, this, &Manager::closeTab);
    connect(m_tabBar, &QTabBar::customContextMenuRequested, this, &Manager::tabContext);

    // main widget in which we put everything
    m_tabsAndEditorWidget = new QWidget(parent);
    m_tabsAndEditorWidget->setLayout(new QVBoxLayout);
    m_tabsAndEditorWidget->layout()->setSpacing(0);
    m_tabsAndEditorWidget->layout()->setContentsMargins(0, 0, 0, 0);
    m_tabsAndEditorWidget->layout()->addWidget(tabBarWidget);
    m_tabsAndEditorWidget->layout()->addWidget(m_widgetStack);

    return m_tabsAndEditorWidget;
}

void Manager::closeTab(int index)
{
    QWidget *widget = textViewAtTab(index);
    if(widget->inherits("KTextEditor::View")) {
        KTextEditor::View *view = static_cast<KTextEditor::View*>(widget);
        m_ki->docManager()->fileClose(view->document());
    }
}

void Manager::currentTabChanged(int index)
{
    QWidget *newlyActivatedWidget = textViewAtTab(index);
    if(!newlyActivatedWidget) {
        return;
    }
    QWidget *oldViewWidget = m_widgetStack->widget(1);
    if(oldViewWidget == newlyActivatedWidget) {
        return;
    }
    if(oldViewWidget) {
        m_widgetStack->removeWidget(oldViewWidget);
    }
    m_widgetStack->insertWidget(1, newlyActivatedWidget);
    m_widgetStack->setCurrentIndex(1);
    emit currentViewChanged(newlyActivatedWidget);
    KTextEditor::View *view = dynamic_cast<KTextEditor::View*>(newlyActivatedWidget);
    if(view) {
        emit textViewActivated(view);
    }
}

void Manager::handleCursorPositionChangedTimeout()
{
    if(m_ki->livePreviewManager()->isLivePreviewEnabledForCurrentDocument()) {
        m_ki->livePreviewManager()->showCursorPositionInDocumentViewer();
    }
}

void Manager::handleCursorPositionChanged(KTextEditor::View *view, const KTextEditor::Cursor &pos)
{
    Q_UNUSED(view);
    Q_UNUSED(pos);

    if(!m_synchronizeViewWithCursorAction->isChecked()) {
        return;
    }
    m_cursorPositionChangedTimer->start(100);
}

KTextEditor::View * Manager::createTextView(KileDocument::TextInfo *info, int index)
{
    KTextEditor::Document *doc = info->getDoc();
    KTextEditor::View *view = info->createView(m_tabBar, Q_NULLPTR);
    Q_ASSERT(view);

    if(!view) {
        KMessageBox::error(m_ki->mainWindow(), i18n("Could not create an editor view."), i18n("Fatal Error"));
    }

    //install a key sequence recorder on the view
    view->focusProxy()->installEventFilter(new KileEditorKeySequence::Recorder(view, m_ki->editorKeySequenceManager()));

    // in the case of simple text documents, we mimic the behaviour of LaTeX documents
    if(info->getType() == KileDocument::Text) {
// 		view->focusProxy()->installEventFilter(m_ki->eventFilter());
    }

    index = m_tabBar->insertTab(index, QString()); // if index=-1 for appending tab, it gets assigned a new index
    m_tabBar->setTabData(index, QVariant::fromValue(view));

    connect(view, &KTextEditor::View::cursorPositionChanged, this, &Manager::cursorPositionChanged);
    connect(view, &KTextEditor::View::viewModeChanged, this, &Manager::viewModeChanged);
    connect(view, &KTextEditor::View::selectionChanged, this, &Manager::selectionChanged);
    connect(view, &KTextEditor::View::viewModeChanged, this, &Manager::updateCaption);
    connect(view, &KTextEditor::View::viewInputModeChanged, this, &Manager::updateModeStatus);
//TODO KF5: signals not available anymore
// 	connect(view, SIGNAL(informationMessage(KTextEditor::View*,QString)), this, SIGNAL(informationMessage(KTextEditor::View*,QString)));
// 	connect(view, SIGNAL(dropEventPass(QDropEvent*)), m_ki->docManager(), SLOT(openDroppedURLs(QDropEvent*)));
    connect(view, &KTextEditor::View::textInserted, m_ki->codeCompletionManager(), &KileCodeCompletion::Manager::textInserted);
    connect(doc, &KTextEditor::Document::documentNameChanged, this, &Manager::updateTabTexts);
    connect(doc, &KTextEditor::Document::documentUrlChanged, this, &Manager::updateTabTexts);

    connect(this, &KileView::Manager::textViewClosed, m_cursorPositionChangedTimer, &QTimer::stop);

    // code completion
    KTextEditor::CodeCompletionInterface *completionInterface = qobject_cast<KTextEditor::CodeCompletionInterface*>(view);
    if(completionInterface) {
        completionInterface->setAutomaticInvocationEnabled(true);
    }

    // install a working text editor part popup dialog thingy
    installContextMenu(view);

    // delete the 'Configure Editor...' action
    delete view->actionCollection()->action("set_confdlg");
    // delete the "save as with encoding" action as it's too technical for Kile
    // also, there is currently no way to preset the desired extension in the save-as dialog
    // (the functionality is still available via Tools/Encoding + save)
    delete view->actionCollection()->action("file_save_as_with_encoding");

    // use Kile's save and save-as functions instead of the text editor's
    QAction *action = view->actionCollection()->action(KStandardAction::name(KStandardAction::Save));
    if(action) {
        KILE_DEBUG_MAIN << "   reconnect action 'file_save'...";
        disconnect(action, &QAction::triggered, 0, 0);
        connect(action, &QAction::triggered, [=]() {
            m_ki->docManager()->fileSave();
        });
    }
    action = view->actionCollection()->action(KStandardAction::name(KStandardAction::SaveAs));
    if(action) {
        KILE_DEBUG_MAIN << "   reconnect action 'file_save_as'...";
        disconnect(action, &QAction::triggered, 0, 0);
        connect(action, &QAction::triggered, [=]() {
            m_ki->docManager()->fileSaveAs();
        });
    }

    // use Kile's smart-new-line feature
    action = view->actionCollection()->action("smart_newline");
    if(action) {
        disconnect(action, &QAction::triggered, 0, 0);
        connect(action, &QAction::triggered, [=]() {
            m_ki->editorExtension()->insertIntelligentNewline();
        });
    }

    updateTabTexts(doc);
    // we do this twice as otherwise the tool tip for the first view did not appear (Qt issue ?)
    // (BUG 205245)
    updateTabTexts(doc);

    m_tabBar->setCurrentIndex(index);
    if(m_tabBar->count() == 1) { // when the tab bar is empty initially, 'setCurrentIndex' won't have any effect
        currentTabChanged(0);      // at this point; so we do it manually
    }

    //activate the newly created view
    emit(textViewCreated(view));
    emit(activateView(view, false));
    emit(updateCaption());  //make sure the caption gets updated

    reflectDocumentModificationStatus(view->document(), false, KTextEditor::ModificationInterface::OnDiskUnmodified);

    return view;
}

void Manager::installContextMenu(KTextEditor::View *view)
{
    QMenu *popupMenu = view->defaultContextMenu();

    if(popupMenu) {
        connect(popupMenu, &QMenu::aboutToShow, this, &Manager::onTextEditorPopupMenuRequest);

        // install some more actions on it
        popupMenu->addSeparator();
        popupMenu->addAction(m_pasteAsLaTeXAction);
        popupMenu->addAction(m_convertToLaTeXAction);
        popupMenu->addSeparator();
        popupMenu->addAction(m_quickPreviewAction);

        // insert actions from user-defined latex menu
        KileMenu::UserMenu *usermenu = m_ki->userMenu();
        if(usermenu) {
            KILE_DEBUG_MAIN << "Insert actions from user-defined latex menu";
            popupMenu->addSeparator();
            foreach(QAction *action, usermenu->contextMenuActions()) {
                if(action) {
                    popupMenu->addAction(action);
                }
                else {
                    popupMenu->addSeparator();
                }
            }
        }

        view->setContextMenu(popupMenu);
    }
}

void Manager::tabContext(const QPoint &pos)
{
    KILE_DEBUG_MAIN << pos;
    const int tabUnderPos = m_tabBar->tabAt(pos);
    if(tabUnderPos < 0) {
        KILE_DEBUG_MAIN << tabUnderPos;
        return;
    }

    KTextEditor::View *view = textViewAtTab(tabUnderPos);

    if(!view || !view->document()) {
        return;
    }

    QMenu tabMenu;

    tabMenu.addSection(m_ki->getShortName(view->document()));

    // 'action1' can become null if it belongs to a view that has been closed, for example
    QPointer<QAction> action1 = m_ki->mainWindow()->action("move_view_tab_left");
    if(action1) {
        action1->setData(QVariant::fromValue(view));
        tabMenu.addAction(action1);
    }

    QPointer<QAction> action2 = m_ki->mainWindow()->action("move_view_tab_right");
    if(action2) {
        action2->setData(QVariant::fromValue(view));
        tabMenu.addAction(action2);
    }

    tabMenu.addSeparator();

    QPointer<QAction> action3;
    if(view->document()->isModified()) {
        action3 = view->actionCollection()->action(KStandardAction::name(KStandardAction::Save));
        if(action3) {
            action3->setData(QVariant::fromValue(view));
            tabMenu.addAction(action3);
        }
    }

    QPointer<QAction> action4 = view->actionCollection()->action(KStandardAction::name(KStandardAction::SaveAs));
    if(action4) {
        action4->setData(QVariant::fromValue(view));
        tabMenu.addAction(action4);
    }

    QPointer<QAction> action5 = view->action("file_save_copy_as");
    if(action5) {
        tabMenu.addAction(action5);
    }

    tabMenu.addSeparator();

    QPointer<QAction> action6 = m_ki->mainWindow()->action("file_close");
    if(action6) {
        action6->setData(QVariant::fromValue(view));
        tabMenu.addAction(action6);
    }

    QPointer<QAction> action7 = m_ki->mainWindow()->action("file_close_all_others");
    if(action7) {
        action7->setData(QVariant::fromValue(view));
        tabMenu.addAction(action7);
    }
    /*
    	FIXME create proper actions which delete/add the current file without asking stupidly
    	QAction* removeAction = m_ki->mainWindow()->action("project_remove");
    	QAction* addAction = m_ki->mainWindow()->action("project_add");

    	tabMenu.insertSeparator(addAction);
    	tabMenu.addAction(addAction);
    	tabMenu.addAction(removeAction);*/

    tabMenu.exec(m_tabBar->mapToGlobal(pos));

    if(action1) {
        action1->setData(QVariant());
    }
    if(action2) {
        action2->setData(QVariant());
    }
    if(action3) {
        action3->setData(QVariant());
    }
    if(action4) {
        action4->setData(QVariant());
    }
    // action5 doesn't need to be given extra data
    if(action6) {
        action6->setData(QVariant());
    }
    if(action7) {
        action7->setData(QVariant());
    }
}

void Manager::removeView(KTextEditor::View *view)
{
    if(view) {
        m_client->factory()->removeClient(view);

        const bool isActiveView = (KTextEditor::Editor::instance()->application()->activeMainWindow()->activeView() == view);
        m_tabBar->removeTab(tabIndexOf(view));

        emit(updateCaption());  //make sure the caption gets updated
        if(m_tabBar->count() == 0) {
            m_ki->structureWidget()->clear();
            m_widgetStack->setCurrentIndex(0); // if there are no open views, then show the DropWidget
        }

        emit(textViewClosed(view, isActiveView));
        delete view;
    }
    else {
        KILE_DEBUG_MAIN << "View should be removed but is Q_NULLPTR";
    }
}

KTextEditor::View * Manager::currentTextView() const
{
    return textViewAtTab(m_tabBar->currentIndex());
}

KTextEditor::View * Manager::textView(int index) const
{
    Q_ASSERT(textViewAtTab(index));
    return textViewAtTab(index);
}

KTextEditor::View * Manager::textView(KileDocument::TextInfo *info) const
{
    KTextEditor::Document *doc = info->getDoc();
    if(!doc) {
        return Q_NULLPTR;
    }
    for(int i = 0; i < m_tabBar->count(); ++i) {
        KTextEditor::View *view = textViewAtTab(i);
        if(!view) {
            continue;
        }

        if(view->document() == doc) {
            return view;
        }
    }

    return Q_NULLPTR;
}

int Manager::textViewCount() const
{
    return m_tabBar->count();
}

int Manager::tabIndexOf(KTextEditor::View* view) const
{
    for(int i = 0; i < m_tabBar->count(); ++i) {
        if(textViewAtTab(i) == view) {
            return i;
        }
    }
    return -1;
}

unsigned int Manager::getTabCount() const
{
    return m_tabBar->count();
}

KTextEditor::View * Manager::switchToTextView(const QUrl &url, bool requestFocus)
{
    return switchToTextView(m_ki->docManager()->docFor(url), requestFocus);
}

KTextEditor::View * Manager::switchToTextView(KTextEditor::Document *doc, bool requestFocus)
{
    KTextEditor::View *view = Q_NULLPTR;
    if(doc) {
        if(doc->views().count() > 0) {
            view = doc->views().first();
            if(view) {
                switchToTextView(view, requestFocus);
            }
        }
    }
    return view;
}

void Manager::switchToTextView(KTextEditor::View *view, bool requestFocus)
{
    int index = tabIndexOf(view);
    if(index < 0) {
        return;
    }
    m_tabBar->setCurrentIndex(index);
    if(requestFocus) {
        focusTextView(view);
    }
}

void Manager::setTabIcon(QWidget *view, const QIcon& icon)
{
    m_tabBar->setTabIcon(tabIndexOf(qobject_cast<KTextEditor::View *>(view)), icon);
}

void Manager::updateStructure(bool parse /* = false */, KileDocument::Info *docinfo /* = Q_NULLPTR */)
{
    if(!docinfo) {
        docinfo = m_ki->docManager()->getInfo();
    }

    if(docinfo) {
        m_ki->structureWidget()->update(docinfo, parse);
    }

    if(m_tabBar->count() == 0) {
        m_ki->structureWidget()->clear();
    }
}

void Manager::gotoNextView()
{
    if(m_tabBar->count() < 2) {
        return;
    }

    int cPage = m_tabBar->currentIndex() + 1;
    if(cPage >= m_tabBar->count()) {
        m_tabBar->setCurrentIndex(0);
    }
    else {
        m_tabBar->setCurrentIndex(cPage);
    }
}

void Manager::gotoPrevView()
{
    if(m_tabBar->count() < 2) {
        return;
    }

    int cPage = m_tabBar->currentIndex() - 1;
    if(cPage < 0) {
        m_tabBar->setCurrentIndex(m_tabBar->count() - 1);
    }
    else {
        m_tabBar->setCurrentIndex(cPage);
    }
}

void Manager::moveTabLeft(QWidget *widget)
{
    if(m_tabBar->count() < 2) {
        return;
    }

    // the 'data' property can be set by 'tabContext'
    QAction *action = dynamic_cast<QAction*>(QObject::sender());
    if(action) {
        QVariant var = action->data();
        if(!widget && var.isValid()) {
            // the action's 'data' property is cleared
            // when the context menu is destroyed
            widget = var.value<QWidget*>();
        }
    }
    if(!widget) {
        widget = currentTextView();
    }
    if(!widget) {
        return;
    }
    int currentIndex = tabIndexOf(qobject_cast<KTextEditor::View *>(widget));
    int newIndex = (currentIndex == 0 ? m_tabBar->count() - 1 : currentIndex - 1);
    m_tabBar->moveTab(currentIndex, newIndex);
}

void Manager::moveTabRight(QWidget *widget)
{
    if(m_tabBar->count() < 2) {
        return;
    }

    // the 'data' property can be set by 'tabContext'
    QAction *action = dynamic_cast<QAction*>(QObject::sender());
    if(action) {
        QVariant var = action->data();
        if(!widget && var.isValid()) {
            // the action's 'data' property is cleared
            // when the context menu is destroyed
            widget = var.value<QWidget*>();
        }
    }
    if(!widget) {
        widget = currentTextView();
    }
    if(!widget) {
        return;
    }
    int currentIndex = tabIndexOf(qobject_cast<KTextEditor::View *>(widget));
    int newIndex = (currentIndex == m_tabBar->count() - 1 ? 0 : currentIndex + 1);
    m_tabBar->moveTab(currentIndex, newIndex);
}

void Manager::reflectDocumentModificationStatus(KTextEditor::Document *doc,
        bool isModified,
        KTextEditor::ModificationInterface::ModifiedOnDiskReason reason)
{
    QIcon icon;
    if(reason == KTextEditor::ModificationInterface::OnDiskUnmodified && isModified) { //nothing
        icon = QIcon::fromTheme("modified"); // This icon is taken from Kate. Therefore
        // our thanks go to the authors of Kate.
    }
    else if(reason == KTextEditor::ModificationInterface::OnDiskModified
            || reason == KTextEditor::ModificationInterface::OnDiskCreated) { //dirty file
        icon = QIcon::fromTheme("emblem-warning"); // This icon is taken from Kate. Therefore
        // our thanks go to the authors of Kate.
    }
    else if(reason == KTextEditor::ModificationInterface::OnDiskDeleted) { //file deleted
        icon = QIcon::fromTheme("emblem-warning");
    }
    else if(m_ki->extensions()->isScriptFile(doc->url())) {
        icon = QIcon::fromTheme("js");
    }
    else {
        icon = QIcon::fromTheme(KIO::iconNameForUrl(doc->url()));
    }

    const QList<KTextEditor::View*> &viewsList = doc->views();
    for(QList<KTextEditor::View*>::const_iterator i = viewsList.begin(); i != viewsList.end(); ++i) {
        setTabIcon(*i, icon);
    }
}

/**
 * Adds/removes the "Convert to LaTeX" entry in Kate's popup menu according to the selection.
 */
void Manager::onTextEditorPopupMenuRequest()
{
    KTextEditor::View *view = currentTextView();
    if(!view) {
        return;
    }

    const QString quickPreviewSelection = i18n("&QuickPreview Selection");
    const QString quickPreviewEnvironment = i18n("&QuickPreview Environment");
    const QString quickPreviewMath = i18n("&QuickPreview Math");

    // Setting up the "QuickPreview selection" entry
    if(view->selection()) {
        m_quickPreviewAction->setText(quickPreviewSelection);
        m_quickPreviewAction->setEnabled(true);

    }
    else if(m_ki->editorExtension()->hasMathgroup(view)) {
        m_quickPreviewAction->setText(quickPreviewMath);
        m_quickPreviewAction->setEnabled(true);
    }
    else if(m_ki->editorExtension()->hasEnvironment(view)) {
        m_quickPreviewAction->setText(quickPreviewEnvironment);
        m_quickPreviewAction->setEnabled(true);
    }
    else {
        m_quickPreviewAction->setText(quickPreviewSelection);
        m_quickPreviewAction->setEnabled(false);
    }


    // Setting up the "Convert to LaTeX" entry
    m_convertToLaTeXAction->setEnabled(view->selection());

    // Setting up the "Paste as LaTeX" entry
    QClipboard *clipboard = QApplication::clipboard();
    if(clipboard) {
        m_pasteAsLaTeXAction->setEnabled(!clipboard->text().isEmpty());
    }
}

void Manager::convertSelectionToLaTeX()
{
    KTextEditor::View *view = currentTextView();

    if(view == Q_NULLPTR) {
        return;
    }

    KTextEditor::Document *doc = view->document();

    if(doc == Q_NULLPTR) {
        return;
    }

    // Getting the selection
    KTextEditor::Range range = view->selectionRange();
    uint selStartLine = range.start().line(), selStartCol = range.start().column();
    uint selEndLine = range.end().line(), selEndCol = range.start().column();

    /* Variable to "restore" the selection after replacement: if {} was selected,
       we increase the selection of two characters */
    uint newSelEndCol;

    PlainToLaTeXConverter cvt;

    // "Notifying" the editor that what we're about to do must be seen as ONE operation
    KTextEditor::Document::EditingTransaction transaction(doc);

    // Processing the first line
    int firstLineLength;
    if(selStartLine != selEndLine) {
        firstLineLength = doc->lineLength(selStartLine);
    } else {
        firstLineLength = selEndCol;
    }
    QString firstLine = doc->text(KTextEditor::Range(selStartLine, selStartCol, selStartLine, firstLineLength));
    QString firstLineCvt = cvt.ConvertToLaTeX(firstLine);
    doc->removeText(KTextEditor::Range(selStartLine, selStartCol, selStartLine, firstLineLength));
    doc->insertText(KTextEditor::Cursor(selStartLine, selStartCol), firstLineCvt);
    newSelEndCol = selStartCol + firstLineCvt.length();

    // Processing the intermediate lines
    for(uint nLine = selStartLine + 1; nLine < selEndLine; ++nLine) {
        QString line = doc->line(nLine);
        QString newLine = cvt.ConvertToLaTeX(line);
        doc->removeLine(nLine);
        doc->insertLine(nLine, newLine);
    }

    // Processing the final line
    if(selStartLine != selEndLine) {
        QString lastLine = doc->text(KTextEditor::Range(selEndLine, 0, selEndLine, selEndCol));
        QString lastLineCvt = cvt.ConvertToLaTeX(lastLine);
        doc->removeText(KTextEditor::Range(selEndLine, 0, selEndLine, selEndCol));
        doc->insertText(KTextEditor::Cursor(selEndLine, 0), lastLineCvt);
        newSelEndCol = lastLineCvt.length();
    }

    // End of the "atomic edit operation"
    transaction.finish();

    view->setSelection(KTextEditor::Range(selStartLine, selStartCol, selEndLine, newSelEndCol));
}

/**
 * Pastes the clipboard's contents as LaTeX (ie. % -> \%, etc.).
 */
void Manager::pasteAsLaTeX()
{
    KTextEditor::View *view = currentTextView();

    if(!view) {
        return;
    }

    KTextEditor::Document *doc = view->document();

    if(!doc) {
        return;
    }

    // Getting a proper text insertion point BEFORE the atomic editing operation
    uint cursorLine, cursorCol;
    if(view->selection()) {
        KTextEditor::Range range = view->selectionRange();
        cursorLine = range.start().line();
        cursorCol = range.start().column();
    } else {
        KTextEditor::Cursor cursor = view->cursorPosition();
        cursorLine = cursor.line();
        cursorCol = cursor.column();
    }

    // "Notifying" the editor that what we're about to do must be seen as ONE operation
    KTextEditor::Document::EditingTransaction transaction(doc);

    // If there is a selection, one must remove it
    if(view->selection()) {
        doc->removeText(view->selectionRange());
    }

    PlainToLaTeXConverter cvt;
    QString toPaste = cvt.ConvertToLaTeX(QApplication::clipboard()->text());
    doc->insertText(KTextEditor::Cursor(cursorLine, cursorCol), toPaste);

    // End of the "atomic edit operation"
    transaction.finish();
}

void Manager::quickPreviewPopup()
{
    KTextEditor::View *view = currentTextView();
    if(!view) {
        return;
    }

    if(view->selection()) {
        emit(startQuickPreview(KileTool::qpSelection));
    }
    else if(m_ki->editorExtension()->hasMathgroup(view)) {
        emit(startQuickPreview(KileTool::qpMathgroup));
    }
    else if(m_ki->editorExtension()->hasEnvironment(view)) {
        emit(startQuickPreview(KileTool::qpEnvironment));
    }
}

void Manager::testCanDecodeURLs(const QDragEnterEvent *e, bool &accept)
{
    accept = e->mimeData()->hasUrls(); // only accept URL drops
}

void Manager::testCanDecodeURLs(const QDragMoveEvent *e, bool &accept)
{
    accept = e->mimeData()->hasUrls(); // only accept URL drops
}

void Manager::replaceLoadedURL(QWidget *w, QDropEvent *e)
{
    QList<QUrl> urls = e->mimeData()->urls();
    if(urls.isEmpty()) {
        return;
    }
    int index = tabIndexOf(qobject_cast<KTextEditor::View *>(w));
    KileDocument::Extensions *extensions = m_ki->extensions();
    bool hasReplacedTab = false;
    for(QList<QUrl>::iterator i = urls.begin(); i != urls.end(); ++i) {
        QUrl url = *i;
        if(extensions->isProjectFile(url)) {
            m_ki->docManager()->projectOpen(url);
        }
        else if(!hasReplacedTab) {
            closeTab(index);
            m_ki->docManager()->fileOpen(url, QString(), index);
            hasReplacedTab = true;
        }
        else {
            m_ki->docManager()->fileOpen(url);
        }
    }
}

void Manager::updateTabTexts(KTextEditor::Document *changedDoc)
{
    const QList<KTextEditor::View*> &viewsList = changedDoc->views();
    for(QList<KTextEditor::View*>::const_iterator i = viewsList.begin(); i != viewsList.end(); ++i) {
        QString documentName = changedDoc->documentName();
        if(documentName.isEmpty()) {
            documentName = i18n("Untitled");
        }
        const int viewIndex = tabIndexOf(*i);
        m_tabBar->setTabText(viewIndex, documentName);
        m_tabBar->setTabToolTip(viewIndex, changedDoc->url().toString());
    }
}

DropWidget::DropWidget(QWidget *parent, const char *name, Qt::WindowFlags f) : QWidget(parent, f)
{
    setObjectName(name);
    setAcceptDrops(true);
}

DropWidget::~DropWidget()
{
}

void DropWidget::dragEnterEvent(QDragEnterEvent *e)
{
    bool b;
    emit testCanDecode(e, b);
    if(b) {
        e->acceptProposedAction();
    }
}

void DropWidget::dropEvent(QDropEvent *e)
{
    emit receivedDropEvent(e);
}

void DropWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    emit mouseDoubleClick();
}

void Manager::installEventFilter(KTextEditor::View *view, QObject *eventFilter)
{
    QWidget *focusProxy = view->focusProxy();
    if(focusProxy) {
        focusProxy->installEventFilter(eventFilter);
    }
    else {
        view->installEventFilter(eventFilter);
    }
}

void Manager::removeEventFilter(KTextEditor::View *view, QObject *eventFilter)
{
    QWidget *focusProxy = view->focusProxy();
    if(focusProxy) {
        focusProxy->removeEventFilter(eventFilter);
    }
    else {
        view->removeEventFilter(eventFilter);
    }
}

//BEGIN ViewerPart methods

void Manager::createViewerPart(KActionCollection *actionCollection)
{
    m_viewerPart = Q_NULLPTR;

    KPluginLoader pluginLoader(OKULAR_LIBRARY_NAME);
    KPluginFactory *factory = pluginLoader.factory();
    if(!factory) {
        KILE_DEBUG_MAIN << "Could not find the Okular library.";
        m_viewerPart = Q_NULLPTR;
        return;
    }
    else {
        QVariantList argList;
        argList << "ViewerWidget" << "ConfigFileName=kile-livepreview-okularpartrc";
        m_viewerPart = factory->create<KParts::ReadOnlyPart>(this, argList);
        Okular::ViewerInterface *viewerInterface = dynamic_cast<Okular::ViewerInterface*>(m_viewerPart.data());
        if(!viewerInterface) {
            // OkularPart doesn't provide the ViewerInterface
            delete m_viewerPart;
            m_viewerPart = Q_NULLPTR;
            return;
        }
        viewerInterface->setWatchFileModeEnabled(false);
        viewerInterface->setShowSourceLocationsGraphically(true);
        connect(m_viewerPart, SIGNAL(openSourceReference(QString,int,int)), this, SLOT(handleActivatedSourceReference(QString,int,int)));

        QAction *paPrintCompiledDocument = actionCollection->addAction(KStandardAction::Print, "print_compiled_document", m_viewerPart, SLOT(slotPrint()));
        paPrintCompiledDocument->setText(i18n("Print Compiled Document..."));
        paPrintCompiledDocument->setShortcut(QKeySequence());
        paPrintCompiledDocument->setEnabled(false);
        connect(m_viewerPart, SIGNAL(enablePrintAction(bool)), paPrintCompiledDocument, SLOT(setEnabled(bool)));
        QAction *printPreviewAction = m_viewerPart->actionCollection()->action("file_print_preview");
        if(printPreviewAction) {
            printPreviewAction->setText(i18n("Print Preview For Compiled Document..."));
        }
    }
}

void Manager::setupViewerPart(QSplitter *splitter)
{
    if(!m_viewerPart) {
        return;
    }
    if(KileConfig::showDocumentViewerInExternalWindow()) {
        if(m_viewerPartWindow && m_viewerPart->widget()->window() == m_viewerPartWindow) { // nothing to be done
            return;
        }
        m_viewerPartWindow = new DocumentViewerWindow();
        m_viewerPartWindow->setObjectName("KileDocumentViewerWindow");
        m_viewerPartWindow->setCentralWidget(m_viewerPart->widget());
        m_viewerPartWindow->setAttribute(Qt::WA_DeleteOnClose, false);
        m_viewerPartWindow->setAttribute(Qt::WA_QuitOnClose, false);
        connect(m_viewerPartWindow, &KileView::DocumentViewerWindow::visibilityChanged, this, &Manager::documentViewerWindowVisibilityChanged);

        m_viewerPartWindow->setWindowTitle(i18n("Document Viewer"));
        m_viewerPartWindow->applyMainWindowSettings(KSharedConfig::openConfig()->group("KileDocumentViewerWindow"));
    }
    else {
        if(m_viewerPart->widget()->parent() && m_viewerPart->widget()->parent() != m_viewerPartWindow) { // nothing to be done
            return;
        }
        splitter->addWidget(m_viewerPart->widget()); // remove it from the window first!
        destroyDocumentViewerWindow();
    }
}

void Manager::destroyDocumentViewerWindow()
{
    if(!m_viewerPartWindow) {
        return;
    }

    KConfigGroup group(KSharedConfig::openConfig(), "KileDocumentViewerWindow");
    m_viewerPartWindow->saveMainWindowSettings(group);
    // we don't want it to influence the document viewer visibility setting as
    // this is a forced close
    disconnect(m_viewerPartWindow, &KileView::DocumentViewerWindow::visibilityChanged, this, &Manager::documentViewerWindowVisibilityChanged);
    m_viewerPartWindow->hide();
    delete m_viewerPartWindow;
    m_viewerPartWindow = Q_NULLPTR;
}

void Manager::handleActivatedSourceReference(const QString& absFileName, int line, int col)
{
    KILE_DEBUG_MAIN << "absFileName:" << absFileName << "line:" << line << "column:" << col;

    QFileInfo fileInfo(absFileName);
    if(!fileInfo.isFile() || !fileInfo.isReadable()) {
        qWarning() << "Got passed an unreadable file:" << absFileName;
        return;
    }

    const QString canonicalFileName = fileInfo.canonicalFilePath(); // remove symbolic links, and
                                                                    // '.', '..' path components
                                                                    // (XeLaTeX + synctex sometimes produces paths containing ./)

    KILE_DEBUG_MAIN << "canonicalFileName:" << canonicalFileName;

    KileDocument::TextInfo *textInfo = m_ki->docManager()->textInfoFor(canonicalFileName);
    // check whether the file or the project item associated with 'canonicalFileName' is already open
    if(!textInfo || !m_ki->isOpen(canonicalFileName)) {
        m_ki->docManager()->fileOpen(canonicalFileName);
        textInfo = m_ki->docManager()->textInfoFor(canonicalFileName);
    }
    if(!textInfo) {
        KILE_DEBUG_MAIN << "no document found!";
        return;
    }
    KTextEditor::View *view = textView(textInfo);
    if(!view) {
        KILE_DEBUG_MAIN << "no view found!";
        return;
    }
    view->setCursorPosition(KTextEditor::Cursor(line, col));
    switchToTextView(view, true);
}

void Manager::showCursorPositionInDocumentViewer()
{
    if(m_ki->livePreviewManager()->isLivePreviewEnabledForCurrentDocument()) {
        m_ki->livePreviewManager()->showCursorPositionInDocumentViewer();
    }
}

void Manager::synchronizeViewWithCursorActionToggled(bool checked)
{
    m_showCursorPositionInViewerAction->setEnabled(!checked);
    if(checked) {
        showCursorPositionInDocumentViewer();
    }
}

void Manager::setDocumentViewerVisible(bool b)
{
    if(!m_viewerPart) {
        return;
    }
    KileConfig::setShowDocumentViewer(b);
    if(m_viewerPartWindow) {
        m_viewerPartWindow->setVisible(b);
    }
    m_viewerPart->widget()->setVisible(b);
}

bool Manager::isViewerPartShown() const
{
    if(!m_viewerPart) {
        return false;
    }

    if(m_viewerPartWindow) {
        return !m_viewerPartWindow->isHidden();
    }
    else {
        return !m_viewerPart->widget()->isHidden();
    }
}

bool Manager::openInDocumentViewer(const QUrl &url)
{
    Okular::ViewerInterface *v = dynamic_cast<Okular::ViewerInterface*>(m_viewerPart.data());
    if(!v) {
        return false;
    }
    bool r = m_viewerPart->openUrl(url);
    v->clearLastShownSourceLocation();
    return r;
}

void Manager::clearLastShownSourceLocationInDocumentViewer()
{
    Okular::ViewerInterface *v = dynamic_cast<Okular::ViewerInterface*>(m_viewerPart.data());
    if(v) {
        v->clearLastShownSourceLocation();
    }
}

void Manager::showSourceLocationInDocumentViewer(const QString& fileName, int line, int column)
{
    Okular::ViewerInterface *v = dynamic_cast<Okular::ViewerInterface*>(m_viewerPart.data());
    if(v) {
        m_clearLastShownSourceLocationTimer->stop();
        v->showSourceLocation(fileName, line, column, true);
        m_clearLastShownSourceLocationTimer->start();
    }
}

void Manager::setLivePreviewModeForDocumentViewer(bool b)
{
    Okular::ViewerInterface *viewerInterface = dynamic_cast<Okular::ViewerInterface*>(m_viewerPart.data());
    if(viewerInterface) {
        if(b) {
            viewerInterface->setWatchFileModeEnabled(false);
        }
        else {
            viewerInterface->setWatchFileModeEnabled(KileConfig::watchFileForDocumentViewer());

        }
    }
}

KToolBar* Manager::getViewerControlToolBar()
{
    return m_viewerControlToolBar;
}

bool Manager::isSynchronisingCursorWithDocumentViewer() const
{
    return m_synchronizeViewWithCursorAction->isChecked();
}

//END ViewerPart methods

bool Manager::viewForLocalFilePresent(const QString& localFileName)
{
    for(int i = 0; i < m_tabBar->count(); ++i) {
        KTextEditor::View *view = textViewAtTab(i);
        if(!view) {
            continue;
        }
        if(view->document()->url().toLocalFile() == localFileName) {
            return true;
        }
    }
    return false;
}

}

void focusTextView(KTextEditor::View *view)
{
    // we work around a potential Qt bug here which can result in dead keys
    // being treated as 'alive' keys in some circumstances, probably when 'setFocus'
    // is called when the widget hasn't been shown yet (see bug 269590)
    QTimer::singleShot(0, view, SLOT(setFocus()));
}
