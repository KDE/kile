/***************************************************************************
                          kilemultitabbar.h -  description
                             -------------------
    begin                :  2001
    copyright            : (C) 2001,2002,2003 by Joseph Wenninger <jowenn@kde.org>
 ***************************************************************************/

/***************************************************************************
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
 ***************************************************************************/

//FIXME: remove for the KDE4 version again

#ifndef _KMultitabbar_h_
#define _KMultitabbar_h_

#include <qscrollview.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qlayout.h>
#include <qstring.h>
#include <qptrlist.h>
#include <qpushbutton.h>

class QPixmap;
class QPainter;
class QFrame;

class KileMultiTabBarPrivate;
class KileMultiTabBarTabPrivate;
class KileMultiTabBarButtonPrivate;
class KileMultiTabBarInternal;

/**
 * @ingroup main
 * @ingroup multitabbar
 * A Widget for horizontal and vertical tabs.
 * It is possible to add normal buttons to the top/left
 * The handling if only one tab at a time or multiple tabs
 * should be raisable is left to the "user".
 *@author Joseph Wenninger
 */
class KileMultiTabBar: public QWidget
{
	Q_OBJECT
public:
	/**
	 * The tab bar's orientation. Also constraints the bar's position.
	 */
	enum KileMultiTabBarMode {
		Horizontal,  ///< Horizontal orientation (i.e. on top or bottom)
		Vertical     ///< Vertical orientation (i.e. on the left or right hand side)
	};
	
	/**
	 * The tab bar's position
	 */
	enum KileMultiTabBarPosition {
		Left,   ///< Left hand side
		Right,  ///< Right hand side
		Top,    ///< On top
		Bottom  ///< On bottom
	};

	/**
	 * The list of available styles for KileMultiTabBar
	 */
	enum KileMultiTabBarStyle {
		VSNET=0,          ///< Visual Studio .Net like (only show the text of active tabs)
		KDEV3=1,          ///< KDevelop 3 like (always show the text)
		KONQSBC=2,        ///< Konqueror's classic sidebar style (unthemed) (currently disabled)
		KDEV3ICON=3,      ///< KDevelop 3 like with icons
		STYLELAST=0xffff  ///< Last style
	};

	/**
	 * Constructor.
	 * @param bm The tab bar's orientation
	 * @param parent The parent widget
	 * @param name The widget's name
	 */
	KileMultiTabBar(KileMultiTabBarMode bm,QWidget *parent=0,const char *name=0);
	
	/**
	 * Destructor.
	 */
	virtual ~KileMultiTabBar();

	/**
 	 * append  a new button to the button area. The button can later on be accessed with button(ID)
	 * eg for connecting signals to it
	 * @param pic a pixmap for the button
 	 * @param id an arbitraty ID value. It will be emitted in the clicked signal for identifying the button
	 *	if more than one button is connected to a signals.
	 * @param popup A popup menu which should be displayed if the button is clicked
	 * @param not_used_yet will be used for a popup text in the future
	 */
 	int appendButton(const QPixmap &pic,int id=-1,QPopupMenu* popup=0,const QString& not_used_yet=QString::null);
	/** 
         * remove a button with the given ID
	 */
	void removeButton(int id);
	/**
	 * append a new tab to the tab area. It can be accessed lateron with tabb(id);
	 * @param pic a bitmap for the tab
	 * @param id an arbitrary ID which can be used later on to identify the tab
	 * @param text if a mode with text is used it will be the tab text, otherwise a mouse over hint
	 * @return Always zero. Can be safely ignored.
	 */
	int appendTab(const QPixmap &pic,int id=-1,const QString& text=QString::null);
	/**
	 * remove a tab with a given ID
	 * @param id The ID of the tab to remove
	 */
	void removeTab(int id);
	/**
	 * set a tab to "raised"
	 * @param id The ID of the tab to manipulate
	 * @param state true == activated/raised, false == not active
	 */
	void setTab(int id ,bool state);
	/**
	 * return the state of a tab, identified by it's ID
	 * @param id The ID of the tab to raise
	 */
	bool isTabRaised(int id) const;
	/**
	 * get a pointer to a button within the button area identified by its ID
	 * @param id The id of the tab
	 */
	class KileMultiTabBarButton *button(int id) const;

	/**
	 * get a pointer to a tab within the tab area, identified by its ID
	 */
	class KileMultiTabBarTab *tab(int id) const;
	/**
	 * set the real position of the widget.
	 * @param pos if the mode is horizontal, only use top, bottom, if it is vertical use left or right
	 */
	void setPosition(KileMultiTabBarPosition pos);
	/**
	 * get the tabbar position.
	 * @return The tab bar's position
	 */
	KileMultiTabBarPosition position() const;
	/**
	 * set the display style of the tabs
	 * @param style The new display style
	 */
	void setStyle(KileMultiTabBarStyle style);
	/**
	 * get the display style of the tabs
	 * @return display style
	 */
	KileMultiTabBarStyle tabStyle() const;
	/**
	 * Returns the list of pointers to the tabs of type KileMultiTabBarTab.
	 * @return The list of tabs.
	 * @warning be careful, don't delete tabs yourself and don't delete the list itself
	 */
        QPtrList<KileMultiTabBarTab>* tabs();
	/**
	 * Returns the list of pointers to the tab buttons of type KileMultiTabBarButton.
	 * @return The list of tab buttons.
	 * @warning be careful, don't delete buttons yourself and don't delete the list itself
	 */
	QPtrList<KileMultiTabBarButton>* buttons();

	/**
	 * might vanish, not sure yet
	 */
	void showActiveTabTexts(bool show=true);
protected:
	friend class KileMultiTabBarButton;
	virtual void fontChange( const QFont& );
	void updateSeparator();
private:
	class KileMultiTabBarInternal *m_internal;
	QBoxLayout *m_l;
	QFrame *m_btnTabSep;
	QPtrList<KileMultiTabBarButton> m_buttons;
	KileMultiTabBarPosition m_position;
	KileMultiTabBarPrivate *d;
};

/**
 * @ingroup multitabbar
 * This class represents a tab bar button in a KileMultiTabBarWidget.
 * This class should never be created except with the appendButton call of KileMultiTabBar
 */
class KileMultiTabBarButton: public QPushButton
{
	Q_OBJECT
public:
	/** @internal */
	KileMultiTabBarButton(const QPixmap& pic,const QString&, QPopupMenu *popup,
		int id,QWidget *parent, KileMultiTabBar::KileMultiTabBarPosition pos, KileMultiTabBar::KileMultiTabBarStyle style);
	/** @internal */
	KileMultiTabBarButton(const QString&, QPopupMenu *popup,
		int id,QWidget *parent, KileMultiTabBar::KileMultiTabBarPosition pos, KileMultiTabBar::KileMultiTabBarStyle style);
	/**
	 * Destructor
	 */
	virtual  ~KileMultiTabBarButton();
	/**
	 * Returns the tab's ID
	 * @return The tab's ID
	 */
	int id() const;

public slots:
	/**
	 * this is used internaly, but can be used by the user, if (s)he wants to
	 * It the according call of KileMultiTabBar is invoked though this modifications will be overwritten
	 */
	void setPosition(KileMultiTabBar::KileMultiTabBarPosition);
        /**
         * this is used internaly, but can be used by the user, if (s)he wants to
         * It the according call of KileMultiTabBar is invoked though this modifications will be overwritten
         */
	void setStyle(KileMultiTabBar::KileMultiTabBarStyle);

        /**
	 * modify the text of the button
         */
	void setText(const QString &);

	QSize sizeHint() const;

protected:
	KileMultiTabBar::KileMultiTabBarPosition m_position;
	KileMultiTabBar::KileMultiTabBarStyle m_style;
	QString m_text;
	virtual void hideEvent( class QHideEvent*);
	virtual void showEvent( class QShowEvent*);
private:
	int m_id;
	KileMultiTabBarButtonPrivate *d;
signals:
	/**
	 * this is emitted if  the button is clicked
	 * @param id	the ID identifying the button
	 */
	void clicked(int id);
protected slots:
	virtual void slotClicked();
};

/**
 * @ingroup multitabbar
 * This class represents a tab bar's tab in a KileMultiTabBarWidget.
 * This class should never be created except with the appendTab call of KileMultiTabBar
 */
class KileMultiTabBarTab: public KileMultiTabBarButton
{
	Q_OBJECT
public:
  /** @internal */
	KileMultiTabBarTab(const QPixmap& pic,const QString&,int id,QWidget *parent,
		KileMultiTabBar::KileMultiTabBarPosition pos,KileMultiTabBar::KileMultiTabBarStyle style);
	/**
	 * Destructor.
	 */
	virtual ~KileMultiTabBarTab();
	/**
	 * set the active state of the tab
	 * @param  state @c true if the tab should become active, @c false otherwise
	 */
	void setState(bool state);
	/**
	 * choose if the text should always be displayed
	 * this is only used in classic mode if at all
	 * @param show Whether or not to show the text
	 */
	void showActiveTabText(bool show);
	/**
	 * Resized the tab to the needed size.
	 */
	void resize(){ setSize( neededSize() ); }
private:
	bool m_showActiveTabText;
	int m_expandedSize;
	KileMultiTabBarTabPrivate *d;
protected:
	friend class KileMultiTabBarInternal;
	void setSize(int);
	int neededSize();
	void updateState();
	virtual void drawButton(QPainter *);
	virtual void drawButtonLabel(QPainter *);
	void drawButtonStyled(QPainter *);
	void drawButtonClassic(QPainter *);
protected slots:
	virtual void slotClicked();
	void setTabsPosition(KileMultiTabBar::KileMultiTabBarPosition);

public slots:
	virtual void setIcon(const QString&);
	virtual void setIcon(const QPixmap&);
};

#endif
