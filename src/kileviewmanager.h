//
// C++ Interface: kileviewmanager
//
// Description: 
//
//
// Author: Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>, (C) 2004
//         Michel Ludwig <michel.ludwig@kdemail.net>, (C) 2006
//

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

#include <QObject>
#include <QStackedWidget>
//Added by qt3to4:
#include <QPixmap>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <QList>

#include <ktabwidget.h>

class QPixmap;

class KUrl;
class KXMLGUIClient;

class KileInfo;
class KileEventFilter;
class KileProjectView;

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
	void setClient(QObject *receiver, KXMLGUIClient *client);

	KTextEditor::View* currentTextView() const;
	const QList<KTextEditor::View*>& textViews() { return m_textViewList; }
	KTextEditor::View* textView(int i) { return m_textViewList.at(i); }
	KTextEditor::View* textView(KileDocument::TextInfo *info);
	int getIndexOf(KTextEditor::View* view) const;
	unsigned int getTabCount() const;

	QWidget* createTabs(QWidget *parent);
	KTextEditor::View* createTextView(KileDocument::TextInfo *info, int index = -1);
	KTabWidget* tabs() { return m_tabs; }

	void unplugKatePartMenu(KTextEditor::View* view);

// 	void setProjectView(KileProjectView *view) { m_projectview = view; }
// 	KileProjectView *projectView() { return m_projectview; } commented out by tbraun, better use signal/slot stuff

public slots:
	KTextEditor::View* switchToTextView(const KUrl & url, bool requestFocus = false);

	void closeWidget(QWidget *);
	void removeView(KTextEditor::View *view);

	void setTabLabel(QWidget *view, const QString & name);
	void changeTab(QWidget *view, const QPixmap & icon, const QString & name);

	void updateStructure(bool parse = false, KileDocument::Info *docinfo = 0L);

	void gotoNextView();
	void gotoPrevView();

	void reflectDocumentStatus(KTextEditor::Document*, bool, unsigned char);

	void onKatePopupMenuRequest(void);
	void convertSelectionToLaTeX(void);
	void pasteAsLaTeX(void);
	void quickPreviewPopup();

protected slots:
	void testCanDecodeURLs(const QDragMoveEvent *e, bool &accept);
	void replaceLoadedURL(QWidget *w, QDropEvent *e);

	/**
	 * 
	 **/
	void urlChanged(KileDocument::Info* info, const KUrl& url);

signals:
	void activateView(QWidget *, bool);
	void prepareForPart(const QString &);
	void startQuickPreview(int);

private:
	KileInfo			*m_ki;
	KTextEditor::View		*m_activeTextView;
// 	KileProjectView			*m_projectview;
	QList<KTextEditor::View*>	m_textViewList;
	KTabWidget 			*m_tabs;
	QObject				*m_receiver;
	KXMLGUIClient			*m_client;
	QStackedWidget			*m_widgetStack;
	QWidget				*m_emptyDropWidget;

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

		virtual void dragMoveEvent(QDragMoveEvent *e);

		virtual void dropEvent(QDropEvent *e);

	signals:
		void testCanDecode(const QDragMoveEvent *, bool &);
		void receivedDropEvent(QDropEvent *);
};

}

#endif
