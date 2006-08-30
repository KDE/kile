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

#include <qobject.h>
#include <qwidgetstack.h>

#include <ktabwidget.h>

class QPixmap;

class KXMLGUIClient;

class KileInfo;
class KileEventFilter;
class KileProjectView;

namespace Kate {
	class Document;
	class View;
}

namespace KileDocument {
	class Info;
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

	Kate::View* currentView() const;
	QPtrList<Kate::View>& views() {return m_viewList;}
	Kate::View* view(int i) { return m_viewList.at(i); }

	void createTabs(QWidget *);
	Kate::View* createView(Kate::Document *doc, int index = -1);
	KTabWidget* tabs() { return m_tabs; }

// 	void setProjectView(KileProjectView *view) { m_projectview = view; }
// 	KileProjectView *projectView() { return m_projectview; } commented out by tbraun, better use signal/slot stuff

public slots:
	Kate::View* switchToView(const KURL & url);

	void closeWidget(QWidget *);
	void removeView(Kate::View *view);

	void setTabLabel(QWidget *view, const QString & name) { m_tabs->setTabLabel(view, name); }
	void changeTab(QWidget *view, const QPixmap & icon, const QString & name) { m_tabs->changeTab(view, icon, name); }

	void updateStructure(bool parse = false, KileDocument::Info *docinfo = 0L);

	void gotoNextView();
	void gotoPrevView();

	void reflectDocumentStatus(Kate::Document*, bool, unsigned char);

	void onKatePopupMenuRequest(void);
	void convertSelectionToLaTeX(void);
	void pasteAsLaTeX(void);

protected slots:
	void testCanDecodeURLs(const QDragMoveEvent *e, bool &accept);
	void replaceLoadedURL(QWidget *w, QDropEvent *e);

signals:
	void activateView(QWidget *, bool);
	void prepareForPart(const QString &);
	void startSpellCheck();

private:
	KileInfo			*m_ki;
	Kate::View			*m_activeView;
// 	KileProjectView		*m_projectview;
	QPtrList<Kate::View> m_viewList;
	KTabWidget 			*m_tabs;
	QObject				*m_receiver;
	KXMLGUIClient		*m_client;
	QWidgetStack			*m_widgetStack;
	QWidget				*m_emptyDropWidget;

};

/**
 * Little helper widget to overcome the limitation that KTabWidget doesn't honour drop events when
 * there are no tabs: the DropWidget is shown instead of KTabWidget when there are no tabs.
 */
class DropWidget : public QWidget {
	Q_OBJECT

	public:
		DropWidget(QWidget * parent = 0, const char * name = 0, WFlags f = 0);
		virtual ~DropWidget();

		virtual void dragMoveEvent(QDragMoveEvent *e);

		virtual void dropEvent(QDropEvent *e);

	signals:
		void testCanDecode(const QDragMoveEvent *, bool &);
		void receivedDropEvent(QDropEvent *);
};

}

#endif
