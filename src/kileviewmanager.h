/**************************************************************************
*   Copyright (C) 2004 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)   *
*             (C) 2006-2022 by Michel Ludwig (michel.ludwig@kdemail.net)  *
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KILEVIEWKILEVIEWMANAGER_H
#define KILEVIEWKILEVIEWMANAGER_H

#include <okular/interfaces/viewerinterface.h>

#include <KTextEditor/Cursor>
#include <KTextEditor/View>
#include <KTextEditor/Document>
#include <KXmlGuiWindow>

#include <QAction>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QList>
#include <QObject>
#include <QIcon>
#include <QPointer>
#include <QTabWidget>

class QSplitter;
class QStackedWidget;
class QTabBar;
class QToolButton;
class QUrl;

class KActionCollection;
class KToggleAction;
class KXMLGUIClient;

class KileInfo;

namespace KParts {
class ReadOnlyPart;
}

namespace KileWidget {
class ProjectView;
}

namespace KTextEditor {
class Document;
class View;
}

namespace KileDocument {
class Info;
class TextInfo;
}

namespace KileView
{

class DocumentViewerWindow : public KMainWindow
{
    Q_OBJECT

public:
    explicit DocumentViewerWindow(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~DocumentViewerWindow();

Q_SIGNALS:
    void visibilityChanged(bool shown);

protected:
    virtual void showEvent(QShowEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;
};

//TODO inherit from KParts::Manager
class Manager
    : public QObject
{
    Q_OBJECT

public:
    explicit Manager(KileInfo *ki, KActionCollection *actionCollection, QObject *parent = nullptr, const char *name = nullptr);

    ~Manager();

public:
    void setClient(KXMLGUIClient *client);

    KTextEditor::View* currentTextView() const;
    KTextEditor::View* textView(int index) const;
    KTextEditor::View* textView(KileDocument::TextInfo *info) const;
    int textViewCount() const;
    int tabIndexOf(KTextEditor::View* view) const;
    unsigned int getTabCount() const;

    QWidget* createTabs(QWidget *parent);
    void setTabsAndEditorVisible(bool b);
    KTextEditor::View* createTextView(KileDocument::TextInfo *info, int index = -1);

// 	void setProjectView(KileWidget::ProjectView *view) { m_projectview = view; }
// 	KileWidget::ProjectView *projectView() { return m_projectview; } commented out by tbraun, better use signal/slot stuff

    static void installEventFilter(KTextEditor::View *view, QObject *eventFilter);
    static void removeEventFilter(KTextEditor::View *view, QObject *eventFilter);

    void installContextMenu(KTextEditor::View *view);

    inline KParts::ReadOnlyPart* viewerPart() const {
        return m_viewerPart.data();
    }

    inline Okular::ViewerInterface* viewerInterfaceForViewerPart() const {
        return qobject_cast<Okular::ViewerInterface*>(m_viewerPart.data());
    }

    void readConfig(QSplitter *splitter);
    void writeConfig();

    bool isViewerPartShown() const;
    void setupViewerPart(QSplitter *splitter);
    bool openInDocumentViewer(const QUrl &url);
    void clearLastShownSourceLocationInDocumentViewer();
    void showSourceLocationInDocumentViewer(const QString& fileName, int line, int column);
    void setLivePreviewModeForDocumentViewer(bool b);

    KToolBar* getViewerControlToolBar();

    bool isSynchronisingCursorWithDocumentViewer() const;

Q_SIGNALS:
    void activateView(QWidget*, bool);
    void startQuickPreview(int);
    void currentViewChanged(QWidget*);
    void textViewActivated(KTextEditor::View *view);
    void textViewCreated(KTextEditor::View *view);
    void textViewClosed(KTextEditor::View *view, bool wasActiveView);
    void updateModeStatus();
    void updateCaption();

    void informationMessage(KTextEditor::View*, const QString&);
    void cursorPositionChanged(KTextEditor::View *view, const KTextEditor::Cursor &newPosition);
    void viewModeChanged(KTextEditor::View*, KTextEditor::View::ViewMode);
    void selectionChanged(KTextEditor::View *view);

    void documentViewerWindowVisibilityChanged(bool shown);

public Q_SLOTS:
    KTextEditor::View* switchToTextView(const QUrl &url, bool requestFocus = false);
    KTextEditor::View* switchToTextView(KTextEditor::Document *doc, bool requestFocus = false);
    void switchToTextView(KTextEditor::View *view, bool requestFocus = false);

    void removeView(KTextEditor::View *view);

    void updateStructure(bool parse = false, KileDocument::Info *docinfo = nullptr);

    void gotoNextView();
    void gotoPrevView();

    void reflectDocumentModificationStatus(KTextEditor::Document*,
                                           bool,
                                           KTextEditor::Document::ModifiedOnDiskReason reason);

    void convertSelectionToLaTeX();
    void pasteAsLaTeX();
    void quickPreviewPopup();

    void moveTabLeft(QWidget *widget = nullptr);
    void moveTabRight(QWidget *widget = nullptr);

    void setDocumentViewerVisible(bool b);

    void handleCursorPositionChanged(KTextEditor::View *view, const KTextEditor::Cursor &pos);

private Q_SLOTS:
    void tabContext(const QPoint& pos);
    void closeTab(int index);
    void currentTabChanged(int index);
    void handleCursorPositionChangedTimeout();

private:
    KTextEditor::View * textViewAtTab(int index) const;

    void createViewerControlToolBar();

public:
    bool viewForLocalFilePresent(const QString& localFileName);

protected:
    void setTabIcon(QWidget *view, const QIcon& icon);

    void createViewerPart(KActionCollection *actionCollection);
    void destroyDocumentViewerWindow();

protected Q_SLOTS:
    void testCanDecodeURLs(const QDragEnterEvent *e, bool &accept);
    void testCanDecodeURLs(const QDragMoveEvent *e, bool &accept);
    void replaceLoadedURL(QWidget *w, QDropEvent *e);
    void onTextEditorPopupMenuRequest();

    /**
     * Updates the labels of every tab that contains a view for 'changedDoc' to reflect there
     * the name of 'changedDoc'.
     **/
    void updateTabTexts(KTextEditor::Document* changedDoc);

    void handleActivatedSourceReference(const QString& absFileName, int line, int col);

    void showCursorPositionInDocumentViewer();
    void synchronizeViewWithCursorActionToggled(bool b);

private:
    KileInfo *m_ki;
    QWidget *m_tabsAndEditorWidget;
    QTabBar *m_tabBar;
    QToolButton *m_documentListButton;
    QObject *m_receiver;
    KXMLGUIClient *m_client;
    DocumentViewerWindow *m_viewerPartWindow;
    QStackedWidget *m_widgetStack;
    QAction *m_pasteAsLaTeXAction;
    QAction *m_convertToLaTeXAction;
    QAction *m_quickPreviewAction;
    QAction *m_showCursorPositionInViewerAction;
    QPointer<KParts::ReadOnlyPart> m_viewerPart;
    QPointer<KToolBar> m_viewerControlToolBar;
    QTimer *m_cursorPositionChangedTimer, *m_clearLastShownSourceLocationTimer;
    KToggleAction *m_synchronizeViewWithCursorAction;
};

/**
 * Little helper widget to overcome the limitation that QTabWidget doesn't honour drop events when
 * there are no tabs: the DropWidget is shown instead of QTabWidget when there are no tabs.
 */
class DropWidget : public QWidget {
    Q_OBJECT

public:
    explicit DropWidget(QWidget * parent = 0, const char * name = 0, Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~DropWidget();

    virtual void dragEnterEvent(QDragEnterEvent *e) override;
    virtual void dropEvent(QDropEvent *e) override;

    virtual void mouseDoubleClickEvent(QMouseEvent *e) override;

Q_SIGNALS:
    void testCanDecode(const QDragEnterEvent *, bool &);
    void receivedDropEvent(QDropEvent *);
    void mouseDoubleClick();
};

}

void focusTextView(KTextEditor::View *view);

#endif
