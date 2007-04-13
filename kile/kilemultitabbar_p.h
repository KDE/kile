/***************************************************************************
                          kilemultitabbar_p.h -  description
                             -------------------
    begin                :  2003
    copyright            : (C) 2003 by Joseph Wenninger <jowenn@kde.org>
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

#ifndef KILE_MULTI_TAB_BAR_P_H
#define KILE_MULTI_TAB_BAR_P_H
#include <qscrollview.h>
#include <kmultitabbar.h>

class KileMultiTabBarInternal: public QScrollView
{
        Q_OBJECT
public:
        KileMultiTabBarInternal(QWidget *parent,KileMultiTabBar::KileMultiTabBarMode bm);
        int appendTab(const QPixmap &,int=-1,const QString& =QString::null);
        KileMultiTabBarTab *tab(int) const;
        void removeTab(int);
        void setPosition(enum KileMultiTabBar::KileMultiTabBarPosition pos);
        void setStyle(enum KileMultiTabBar::KileMultiTabBarStyle style);
        void showActiveTabTexts(bool show);
        QPtrList<KileMultiTabBarTab>* tabs(){return &m_tabs;}
private:
        friend class KileMultiTabBar;
        QWidget *box;
	QBoxLayout *mainLayout;
        QPtrList<KileMultiTabBarTab> m_tabs;
        enum KileMultiTabBar::KileMultiTabBarPosition m_position;
        bool m_showActiveTabTexts;
        enum  KileMultiTabBar::KileMultiTabBarStyle m_style;
	int m_expandedTabSize;
	int m_lines;
	KileMultiTabBar::KileMultiTabBarMode m_barMode;
protected:
	virtual bool eventFilter(QObject *,QEvent*);
        virtual void drawContents ( QPainter *, int, int, int, int);

        /**
         * [contentsM|m]ousePressEvent are reimplemented from QScrollView
         * in order to ignore all mouseEvents on the viewport, so that the
         * parent can handle them.
         */
        virtual void contentsMousePressEvent(QMouseEvent *);
        virtual void mousePressEvent(QMouseEvent *);
	virtual void resizeEvent(QResizeEvent *);
};
#endif

