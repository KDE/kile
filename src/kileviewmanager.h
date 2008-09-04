/**************************************************************************
*   Copyright (C) 2004 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)   *
*             (C) 2006-2008 by Michel Ludwig (michel.ludwig@kdemail.net)  *
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

#include <QDropEvent>
#include <QDragMoveEvent>
#include <QList>
#include <QObject>
#include <QPixmap>
#include <QStackedWidget>

#include <KAction>
#include <KTabWidget>
#include <KTextEditor/Cursor>
#include <KTextEditor/ModificationInterface>

class QPixmap;

class KUrl;
class KXMLGUIClient;

class KileInfo;

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

//TODO inherit from KParts::Manager
class Manager : public QObject
{
	Q_OBJECT

public:
	Manager(KileInfo *ki, QObject *parent = 0, const char *name = 0);

	~Manager();

public:
	void setClient(KXMLGUIClient *client);

	KTextEditor::View* currentTextView() const;
	const QList<KTextEditor::View*>& textViews() { return m_textViewList; }
	KTextEditor::View* textView(int i) { return m_textViewList.at(i); }
	KTextEditor::View* textView(KileDocument::TextInfo *info);
	int getIndexOf(KTextEditor::View* view) const;
	unsigned int getTabCount() const;

	QWidget* createTabs(QWidget *parent);
	KTextEditor::View* createTextView(KileDocument::TextInfo *info, int index = -1);
	KTabWidget* tabs() { return m_tabs; }

// 	void setProjectView(KileWidget::ProjectView *view) { m_projectview = view; }
// 	KileWidget::ProjectView *projectView() { return m_projectview; } commented out by tbraun, better use signal/slot stuff

	void unplugTextEditorPartMenu(KTextEditor::View* view);

	static void installEventFilter(KTextEditor::View *view, QObject *eventFilter);
	static void removeEventFilter(KTextEditor::View *view, QObject *eventFilter);

Q_SIGNALS:
	void activateView(QWidget*, bool);
	void prepareForPart(const QString&);
	void startQuickPreview(int);
	void currentViewChanged(QWidget*);
	void updateModeStatus();
	void updateCaption();

	void informationMessage(KTextEditor::View*, const QString&);
	void cursorPositionChanged(KTextEditor::View *view, const KTextEditor::Cursor &newPosition);
	void viewModeChanged(KTextEditor::View *view);
	void selectionChanged(KTextEditor::View *view);

public Q_SLOTS:
	KTextEditor::View* switchToTextView(const KUrl & url, bool requestFocus = false);

	void closeWidget(QWidget *);
	void removeView(KTextEditor::View *view);

	void updateStructure(bool parse = false, KileDocument::Info *docinfo = NULL);

	void gotoNextView();
	void gotoPrevView();

	void reflectDocumentModificationStatus(KTextEditor::Document*,
	                                       bool,
	                                       KTextEditor::ModificationInterface::ModifiedOnDiskReason reason);

	void convertSelectionToLaTeX(void);
	void pasteAsLaTeX(void);
	void quickPreviewPopup();
	void tabContext(QWidget* widget,const QPoint & pos);

protected:
	void setTabLabel(QWidget *view, const QString& name);
	void setTabIcon(QWidget *view, const QPixmap& icon);

protected Q_SLOTS:
	void testCanDecodeURLs(const QDragEnterEvent *e, bool &accept);
	void testCanDecodeURLs(const QDragMoveEvent *e, bool &accept);
	void replaceLoadedURL(QWidget *w, QDropEvent *e);
	void onTextEditorPopupMenuRequest(void);

	/**
	 * Updates the labels of every tab that contains a view for 'changedDoc' to reflect there
	 * the name of 'changedDoc'.
	 **/
	void updateTabTexts(KTextEditor::Document* changedDoc);

private:
	KileInfo			*m_ki;
	KTextEditor::View		*m_activeTextView;
// 	KileWidget::ProjectView			*m_projectview;
	QList<KTextEditor::View*>	m_textViewList;
	KTabWidget 			*m_tabs;
	QObject				*m_receiver;
	KXMLGUIClient			*m_client;
	QStackedWidget			*m_widgetStack;
	QWidget				*m_emptyDropWidget;
	KAction				*m_pasteAsLaTeXAction, *m_convertToLaTeXAction,
					*m_quickPreviewAction;
};

/**
 * Little helper widget to overcome the limitation that KTabWidget doesn't honour drop events when
 * there are no tabs: the DropWidget is shown instead of KTabWidget when there are no tabs.
 */
class DropWidget : public QWidget {
	Q_OBJECT

	public:
		DropWidget(QWidget * parent = 0, const char * name = 0, Qt::WFlags f = 0);
		virtual ~DropWidget();

		virtual void dragEnterEvent(QDragEnterEvent *e);

		virtual void dropEvent(QDropEvent *e);

	Q_SIGNALS:
		void testCanDecode(const QDragEnterEvent *, bool &);
		void receivedDropEvent(QDropEvent *);
};

}

#endif
