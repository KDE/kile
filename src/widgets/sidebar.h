/**************************************************************************************
    Copyright (C) 2004 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
              (C) 2006 by Thomas Braun (thomas.braun@virtuell-zuhause.de)
              (C) 2006-2018 by Michel Ludwig (michel.ludwig@kdemail.net)
 **************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QIcon>
#include <QStackedWidget>
#include <QWidget>

#include <KMultiTabBar>

namespace KileWidget {

class SideBar : public QWidget
{
    Q_OBJECT

public:
    explicit SideBar(QWidget *parent = Q_NULLPTR, Qt::Orientation orientation = Qt::Vertical);
    virtual ~SideBar();

    int addPage(QWidget *tab, const QIcon &pic, const QString &text = QString());
    void removePage(QWidget *w);

    QWidget* currentPage();

    /**
     * Returns the index of the widget which is currently shown, or -1 if the side bar is minimized.
     **/
    int currentTab();

    bool isMinimized();

    int count();

    /**
     * Shows or hides the tab connected to the widget "w". If the tab to be hidden is
     * currently selected, the next tab will be shown (cyclically).
     *
     * @param b set to "true" to show the tab connected to the widget "w", "false" to
     *          hide it
     **/
    void setPageVisible(QWidget *w, bool b);

    /**
     * Returns the side bar's height if its orientation is vertical, its width otherwise.
     **/
    int directionalSize();

    /**
     * Sets the side bar's height if its orientation is vertical, its width otherwise.
     **/
    void setDirectionalSize(int i);

    /**
     * Add a widget to the (right or bottom) of the tab bar, which is not connected to any tabs.
     **/
    void addExtraWidget(QWidget *w);

Q_SIGNALS:
    void visibilityChanged(bool b);

public Q_SLOTS:
    void showPage(QWidget *w);

    /**
     * Shows the widget with index 'id'. If 'id' is not a valid index, the side bar will be
     * minimized.
     **/
    void switchToTab(int id);

    void shrink();

private:
    int findNextShownTab(int i);

protected Q_SLOTS:
    void expand();
    void tabClicked(int i);

protected:
    Qt::Orientation		m_orientation;
    bool			m_minimized; /* using m_tabStack->isVisible is not enough */
    int			m_directionalSize; /* directional size in the unminized state */
    int 			m_currentTab;
    QStackedWidget		*m_tabStack;
    KMultiTabBar		*m_tabBar;
    QWidget			*m_extraWidget;
};

class BottomBar : public SideBar
{
    Q_OBJECT

public:
    explicit BottomBar(QWidget *parent = Q_NULLPTR);

};

}

#endif
