//
// C++ Interface: kileviewmanager
//
// Description: 
//
//
// Author: Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>, (C) 2004
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
#include <qtabwidget.h>
#include <qpixmap.h>

#include <kate/view.h>

class KXMLGUIClient;

class KileInfo;
class KileEventFilter;
class KileProjectView;
class KileDocument::Info;

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
	Kate::View* createView(Kate::Document *doc);

	void setProjectView(KileProjectView *view) { m_projectview = view; }
	KileProjectView *projectView() { return m_projectview; }

public slots:
	Kate::View* switchToView(const KURL & url);

	void removeView(Kate::View *view);
	void removeFromProjectView(const KURL & url);

	void setTabLabel(QWidget *view, const QString & name) { m_tabs->setTabLabel(view, name); }
	void changeTab(QWidget *view, const QPixmap & icon, const QString & name) { m_tabs->changeTab(view, icon, name); }

	void updateStructure(bool parse = false, KileDocument::Info *docinfo = 0L);

	void gotoNextView();
	void gotoPrevView();

	void reflectDocumentStatus(Kate::Document*, bool, unsigned char);

signals:
	void activateView(QWidget *, bool);
	void prepareForPart(const QString &);
	void startSpellCheck();


private:
	KileInfo				*m_ki;
	Kate::View			*m_activeView;
	KileProjectView		*m_projectview;
	QPtrList<Kate::View> 	m_viewList;
	QTabWidget 			*m_tabs;
	QObject				*m_receiver;
	KXMLGUIClient		*m_client;
};

}

#endif
