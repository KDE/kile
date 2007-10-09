/***************************************************************************
                          kilemultitabbar.cpp -  description
                             -------------------
    begin                :  2001
    copyright            : (C) 2001,2002,2003 by Joseph Wenninger <jowenn@kde.org>
                           (C) 2007 by Michel Ludwig <michel.ludwig@kdemail.net>
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

//FIXME: remove for the KDE4 version again and push the fix for hidden widgets
//       upstream

#include "kilemultitabbar.h"
#include "kilemultitabbar_p.h"
#include "kilemultitabbar.moc"
#include "kilemultitabbar_p.moc"
#include <qbutton.h>
#include <qpopupmenu.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qtooltip.h>
#include <qfontmetrics.h>
#include <qregexp.h>
#include <qstyle.h>

#include <kiconloader.h>
#include "kiledebug.h"
#include <qapplication.h>
#include <math.h>

#define NEARBYINT(i) ((int(float(i) + 0.5)))

class KileMultiTabBarTabPrivate {
public:
	QPixmap pix;
};


KileMultiTabBarInternal::KileMultiTabBarInternal(QWidget *parent, KileMultiTabBar::KileMultiTabBarMode bm):QScrollView(parent)
{
	m_expandedTabSize=-1;
	m_showActiveTabTexts=false;
	m_tabs.setAutoDelete(true);
	m_barMode=bm;
	setHScrollBarMode(AlwaysOff);
	setVScrollBarMode(AlwaysOff);
	if (bm==KileMultiTabBar::Vertical)
	{
		box=new QWidget(viewport());
		mainLayout=new QVBoxLayout(box);
		mainLayout->setAutoAdd(true);
		box->setFixedWidth(24);
		setFixedWidth(24);
	}
	else
	{
		box=new QWidget(viewport());
		mainLayout=new QHBoxLayout(box);
		mainLayout->setAutoAdd(true);
		box->setFixedHeight(24);
		setFixedHeight(24);
	}
	addChild(box);
	setFrameStyle(NoFrame);
	viewport()->setBackgroundMode(Qt::PaletteBackground);
/*	box->setPaletteBackgroundColor(Qt::red);
	setPaletteBackgroundColor(Qt::green);*/
}

void KileMultiTabBarInternal::setStyle(enum KileMultiTabBar::KileMultiTabBarStyle style)
{
	m_style=style;
        for (uint i=0;i<m_tabs.count();i++)
                m_tabs.at(i)->setStyle(m_style);

	if  ( (m_style==KileMultiTabBar::KDEV3) ||
		(m_style==KileMultiTabBar::KDEV3ICON ) ) {
		delete mainLayout;
		mainLayout=0;
		resizeEvent(0);
	} else if (mainLayout==0) {
		if (m_barMode==KileMultiTabBar::Vertical)
		{
			box=new QWidget(viewport());
			mainLayout=new QVBoxLayout(box);
			box->setFixedWidth(24);
			setFixedWidth(24);
		}
		else
		{
			box=new QWidget(viewport());
			mainLayout=new QHBoxLayout(box);
			box->setFixedHeight(24);
			setFixedHeight(24);
		}
		addChild(box);
	        for (uint i=0;i<m_tabs.count();i++)
        	        mainLayout->add(m_tabs.at(i));
		mainLayout->setAutoAdd(true);

	}
        viewport()->repaint();
}

void KileMultiTabBarInternal::drawContents ( QPainter * paint, int clipx, int clipy, int clipw, int cliph )
{
	QScrollView::drawContents (paint , clipx, clipy, clipw, cliph );

	if (m_position==KileMultiTabBar::Right)
	{

                paint->setPen(colorGroup().shadow());
                paint->drawLine(0,0,0,viewport()->height());
                paint->setPen(colorGroup().background().dark(120));
                paint->drawLine(1,0,1,viewport()->height());


	}
	else
	if (m_position==KileMultiTabBar::Left)
	{
                paint->setPen(colorGroup().light());
		paint->drawLine(23,0,23,viewport()->height());
                paint->drawLine(22,0,22,viewport()->height());

                paint->setPen(colorGroup().shadow());
                paint->drawLine(0,0,0,viewport()->height());
	}
	else
	if (m_position==KileMultiTabBar::Bottom)
	{
		paint->setPen(colorGroup().shadow());
		paint->drawLine(0,0,viewport()->width(),0);
                paint->setPen(colorGroup().background().dark(120));
                paint->drawLine(0,1,viewport()->width(),1);
	}
	else
	{
	        paint->setPen(colorGroup().light());
		paint->drawLine(0,23,viewport()->width(),23);
                paint->drawLine(0,22,viewport()->width(),22);

/*                paint->setPen(colorGroup().shadow());
                paint->drawLine(0,0,0,viewport()->height());*/

	}


}

void KileMultiTabBarInternal::contentsMousePressEvent(QMouseEvent *ev)
{
	ev->ignore();
}

void KileMultiTabBarInternal::mousePressEvent(QMouseEvent *ev)
{
	ev->ignore();
}


#define CALCDIFF(m_tabs,diff,i) if (m_lines>(int)lines) {\
					/*KILE_DEBUG()<<"i="<<i<<" tabCount="<<tabCount<<" space="<<space<<endl;*/ \
					uint ulen=0;\
					diff=0; \
					for (uint i2=i;i2<tabCount;i2++) {\
						uint l1=(*(m_tabs.at(i2)))->neededSize();\
						if ((ulen+l1)>space){\
							if (ulen==0) diff=0;\
							else diff=((float)(space-ulen))/(i2-i);\
							break;\
						}\
						ulen+=l1;\
					}\
				} else {diff=0; }


void KileMultiTabBarInternal::resizeEvent(QResizeEvent *ev) {
/*	KILE_DEBUG()<<"KileMultiTabBarInternal::resizeEvent"<<endl;
	KILE_DEBUG()<<"KileMultiTabBarInternal::resizeEvent - box geometry"<<box->geometry()<<endl;
	KILE_DEBUG()<<"KileMultiTabBarInternal::resizeEvent - geometry"<<geometry()<<endl;*/
	if (ev) QScrollView::resizeEvent(ev);
	QValueList<KileMultiTabBarTab*> visibleTabList;
	for(KileMultiTabBarTab *tab = m_tabs.first(); tab; tab = m_tabs.next()) {
		if(tab->isVisible()) {
			visibleTabList.push_back(tab);
		}
	}

	if ( (m_style==KileMultiTabBar::KDEV3) ||
		(m_style==KileMultiTabBar::KDEV3ICON) ){
		box->setGeometry(0,0,width(),height());
		int lines=1;
		uint space;
		float tmp=0;
		if ((m_position==KileMultiTabBar::Bottom) || (m_position==KileMultiTabBar::Top))
			space=width();
		else
			space=height();

		int cnt=0;

//CALCULATE LINES
		const uint tabCount = visibleTabList.count();
	        for (uint i=0;i<tabCount;i++) {
			cnt++;
			tmp+=(*(visibleTabList.at(i)))->neededSize();
			if (tmp>space) {
				if (cnt>1)i--;
				else if (i==(tabCount-1)) break;
				cnt=0;
				tmp=0;
				lines++;
			}
		}
//SET SIZE & PLACE
		float diff=0;
		cnt=0;

		if ((m_position==KileMultiTabBar::Bottom) || (m_position==KileMultiTabBar::Top)) {

			setFixedHeight(lines*24);
			box->setFixedHeight(lines*24);
			m_lines=height()/24-1;
			lines=0;
			CALCDIFF(visibleTabList,diff,0)
			tmp=-diff;

			//KILE_DEBUG()<<"m_lines recalculated="<<m_lines<<endl;
		        for (uint i=0;i<tabCount;i++) {
				KileMultiTabBarTab *tab=*(visibleTabList.at(i));
				cnt++;
				tmp+=tab->neededSize()+diff;
				if (tmp>space) {
					//KILE_DEBUG()<<"about to start new line"<<endl;
					if (cnt>1) {
						CALCDIFF(visibleTabList,diff,i)
						i--;
					}
					else {
						//KILE_DEBUG()<<"placing line on old line"<<endl;
						KILE_DEBUG()<<"diff="<<diff<<endl;
						tab->removeEventFilter(this);
						tab->move(NEARBYINT(tmp-tab->neededSize()),lines*24);
//						tab->setFixedWidth(tab->neededSize()+diff);
						tab->setFixedWidth(NEARBYINT(tmp+diff)-tab->x());
						tab->installEventFilter(this);
						CALCDIFF(visibleTabList,diff,(i+1))

					}
					tmp=-diff;
					cnt=0;
					lines++;
					//KILE_DEBUG()<<"starting new line:"<<lines<<endl;

				} else 	{
					//KILE_DEBUG()<<"Placing line on line:"<<lines<<" pos: (x/y)=("<<tmp-(*(visibleTabList.at(i)))->neededSize()<<"/"<<lines*24<<")"<<endl;
					//KILE_DEBUG()<<"diff="<<diff<<endl;
					tab->removeEventFilter(this);
					tab->move(NEARBYINT(tmp-tab->neededSize()),lines*24);
					tab->setFixedWidth(NEARBYINT(tmp+diff)-tab->x());

					//tab->setFixedWidth(tab->neededSize()+diff);
					tab->installEventFilter(this);

				}
			}
		}
		else {
			setFixedWidth(lines*24);
			box->setFixedWidth(lines*24);
			m_lines=lines=width()/24;
			lines=0;
			CALCDIFF(visibleTabList,diff,0)
			tmp=-diff;

		        for (uint i=0;i<tabCount;i++) {
				KileMultiTabBarTab *tab=*(visibleTabList.at(i));
				cnt++;
				tmp+=tab->neededSize()+diff;
				if (tmp>space) {
					if (cnt>1) {
						CALCDIFF(visibleTabList,diff,i);
						tmp=-diff;
						i--;
					}
					else {
						tab->removeEventFilter(this);
						tab->move(lines*24,NEARBYINT(tmp-tab->neededSize()));
                                                tab->setFixedHeight(NEARBYINT(tmp+diff)-tab->y());
						tab->installEventFilter(this);
					}
					cnt=0;
					tmp=-diff;
					lines++;
				} else 	{
					tab->removeEventFilter(this);
					tab->move(lines*24,NEARBYINT(tmp-tab->neededSize()));
                                        tab->setFixedHeight(NEARBYINT(tmp+diff)-tab->y());
					tab->installEventFilter(this);
				}
			}
		}


		//KILE_DEBUG()<<"needed lines:"<<m_lines<<endl;
	} else {

		int size=0; /*move the calculation into another function and call it only on add tab and tab click events*/
		for (QValueList<KileMultiTabBarTab*>::iterator i = visibleTabList.begin(); i != visibleTabList.end(); ++i) {
			size += (m_barMode == KileMultiTabBar::Vertical ? (*i)->height() : (*i)->width());
		}
		if ((m_position==KileMultiTabBar::Bottom) || (m_position==KileMultiTabBar::Top))
			box->setGeometry(0,0,size,height());
		else box->setGeometry(0,0,width(),size);

	}
}


void KileMultiTabBarInternal::showActiveTabTexts(bool show)
{
	m_showActiveTabTexts=show;
}


KileMultiTabBarTab* KileMultiTabBarInternal::tab(int id) const
{
	for (QPtrListIterator<KileMultiTabBarTab> it(m_tabs);it.current();++it){
		if (it.current()->id()==id) return it.current();
	}
        return 0;
}

bool KileMultiTabBarInternal::eventFilter(QObject *, QEvent *e) {
	if (e->type()==QEvent::Resize || e->type()==QEvent::Hide || e->type()==QEvent::Show) resizeEvent(0);
	return false;
}

int KileMultiTabBarInternal::appendTab(const QPixmap &pic ,int id,const QString& text)
{
	KileMultiTabBarTab  *tab;
	m_tabs.append(tab= new KileMultiTabBarTab(pic,text,id,box,m_position,m_style));
	tab->installEventFilter(this);
	tab->showActiveTabText(m_showActiveTabTexts);

	if (m_style==KileMultiTabBar::KONQSBC)
	{
		if (m_expandedTabSize<tab->neededSize()) {
			m_expandedTabSize=tab->neededSize();
			for (uint i=0;i<m_tabs.count();i++)
				m_tabs.at(i)->setSize(m_expandedTabSize);

		} else tab->setSize(m_expandedTabSize);
	} else tab->updateState();
	tab->show();
	resizeEvent(0);
	return 0;
}

void KileMultiTabBarInternal::removeTab(int id)
{
	for (uint pos=0;pos<m_tabs.count();pos++)
	{
		if (m_tabs.at(pos)->id()==id)
		{
			m_tabs.remove(pos);
			resizeEvent(0);
			break;
		}
	}
}

void KileMultiTabBarInternal::setPosition(enum KileMultiTabBar::KileMultiTabBarPosition pos)
{
	m_position=pos;
	for (uint i=0;i<m_tabs.count();i++)
		m_tabs.at(i)->setTabsPosition(m_position);
	viewport()->repaint();
}

KileMultiTabBarButton::KileMultiTabBarButton(const QPixmap& pic,const QString& text, QPopupMenu *popup,
		int id,QWidget *parent,KileMultiTabBar::KileMultiTabBarPosition pos,KileMultiTabBar::KileMultiTabBarStyle style)
	:QPushButton(QIconSet(),text,parent),m_style(style)
{
	setIconSet(pic);
	setText(text);
	m_position=pos;
  	if (popup) setPopup(popup);
	setFlat(true);
	setFixedHeight(24);
	setFixedWidth(24);
	m_id=id;
	QToolTip::add(this,text);
	connect(this,SIGNAL(clicked()),this,SLOT(slotClicked()));
}

KileMultiTabBarButton::KileMultiTabBarButton(const QString& text, QPopupMenu *popup,
		int id,QWidget *parent,KileMultiTabBar::KileMultiTabBarPosition pos,KileMultiTabBar::KileMultiTabBarStyle style)
	:QPushButton(QIconSet(),text,parent),m_style(style)
{
	setText(text);
	m_position=pos;
  	if (popup) setPopup(popup);
	setFlat(true);
	setFixedHeight(24);
	setFixedWidth(24);
	m_id=id;
	QToolTip::add(this,text);
	connect(this,SIGNAL(clicked()),this,SLOT(slotClicked()));
}

KileMultiTabBarButton::~KileMultiTabBarButton() {
}

int KileMultiTabBarButton::id() const{
	return m_id;
}

void KileMultiTabBarButton::setText(const QString& text)
{
	QString s = text;
	s = s.replace("&", QString()); // we do not allow accelerators to be set
                                       // This is a little hack to avoid KAcceleratorManager
                                       // provide us with accelerators as the class name of the
                                       // widget is no longer KMultiTabBar.
	QPushButton::setText(s);
	m_text = s;
	QToolTip::add(this,s);
}

void KileMultiTabBarButton::slotClicked()
{
	emit clicked(m_id);
}

void KileMultiTabBarButton::setPosition(KileMultiTabBar::KileMultiTabBarPosition pos)
{
	m_position=pos;
	repaint();
}

void KileMultiTabBarButton::setStyle(KileMultiTabBar::KileMultiTabBarStyle style)
{
	m_style=style;
	repaint();
}

void KileMultiTabBarButton::hideEvent( QHideEvent* he) {
	QPushButton::hideEvent(he);
	KileMultiTabBar *tb=dynamic_cast<KileMultiTabBar*>(parentWidget());
	if (tb) tb->updateSeparator();
}

void KileMultiTabBarButton::showEvent( QShowEvent* he) {
	QPushButton::showEvent(he);
	KileMultiTabBar *tb=dynamic_cast<KileMultiTabBar*>(parentWidget());
	if (tb) tb->updateSeparator();
}


QSize KileMultiTabBarButton::sizeHint() const
{
    constPolish();

    int w = 0, h = 0;

    // calculate contents size...
#ifndef QT_NO_ICONSET
    if ( iconSet() && !iconSet()->isNull() ) {
        int iw = iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).width() + 4;
        int ih = iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).height();
        w += iw;
        h = QMAX( h, ih );
    }
#endif
    if ( isMenuButton() )
        w += style().pixelMetric(QStyle::PM_MenuButtonIndicator, this);

    if ( pixmap() ) {
        QPixmap *pm = (QPixmap *)pixmap();
        w += pm->width();
        h += pm->height();
    } else {
        QString s( text() );
        bool empty = s.isEmpty();
        if ( empty )
            s = QString::fromLatin1("XXXX");
        QFontMetrics fm = fontMetrics();
        QSize sz = fm.size( ShowPrefix, s );
        if(!empty || !w)
            w += sz.width();
        if(!empty || !h)
            h = QMAX(h, sz.height());
    }

    return (style().sizeFromContents(QStyle::CT_ToolButton, this, QSize(w, h)).
            expandedTo(QApplication::globalStrut()));
}


KileMultiTabBarTab::KileMultiTabBarTab(const QPixmap& pic, const QString& text,
		int id,QWidget *parent,KileMultiTabBar::KileMultiTabBarPosition pos,
		KileMultiTabBar::KileMultiTabBarStyle style)
	:KileMultiTabBarButton(text,0,id,parent,pos,style),
 	m_showActiveTabText(false)
{
	d=new KileMultiTabBarTabPrivate();
	setIcon(pic);
	m_expandedSize=24;
	setToggleButton(true);
}

KileMultiTabBarTab::~KileMultiTabBarTab() {
	delete d;
}


void KileMultiTabBarTab::setTabsPosition(KileMultiTabBar::KileMultiTabBarPosition pos)
{
	if ((pos!=m_position) && ((pos==KileMultiTabBar::Left) || (pos==KileMultiTabBar::Right))) {
		if (!d->pix.isNull()) {
			QWMatrix temp;// (1.0F, 0.0F, 0.0F, -1.0F, 0.0F, 0.0F);
			temp.rotate(180);
			d->pix=d->pix.xForm(temp);
			setIconSet(d->pix);
		}
	}

	setPosition(pos);
//	repaint();
}

void KileMultiTabBarTab::setIcon(const QString& icon)
{
	QPixmap pic=SmallIcon(icon);
	setIcon(pic);
}

void KileMultiTabBarTab::setIcon(const QPixmap& icon)
{

	if (m_style!=KileMultiTabBar::KDEV3) {
		if ((m_position==KileMultiTabBar::Left) || (m_position==KileMultiTabBar::Right)) {
		        QWMatrix rotateMatrix;
			if (m_position==KileMultiTabBar::Left)
		        	rotateMatrix.rotate(90);
			else
				rotateMatrix.rotate(-90);
			QPixmap pic=icon.xForm(rotateMatrix); //TODO FIX THIS, THIS SHOWS WINDOW
			d->pix=pic;
		        setIconSet(pic);
		} else setIconSet(icon);
	}
}

void KileMultiTabBarTab::slotClicked()
{
	updateState();
	KileMultiTabBarButton::slotClicked();
}

void KileMultiTabBarTab::setState(bool b)
{
	setOn(b);
	updateState();
}

void KileMultiTabBarTab::updateState()
{
	if (m_style!=KileMultiTabBar::KONQSBC) {
		if ((m_style==KileMultiTabBar::KDEV3) || (m_style==KileMultiTabBar::KDEV3ICON) || (isOn())) {
			QPushButton::setText(m_text);
		} else {
			KILE_DEBUG()<<"KileMultiTabBarTab::updateState(): setting text to an empty QString***************"<<endl;
			QPushButton::setText(QString::null);
		}

		if ((m_position==KileMultiTabBar::Right || m_position==KileMultiTabBar::Left)) {
			setFixedWidth(24);
			if ((m_style==KileMultiTabBar::KDEV3)  || (m_style==KileMultiTabBar::KDEV3ICON) || (isOn())) {
				setFixedHeight(KileMultiTabBarButton::sizeHint().width());
			} else setFixedHeight(36);
		} else {
			setFixedHeight(24);
			if ((m_style==KileMultiTabBar::KDEV3)  || (m_style==KileMultiTabBar::KDEV3ICON) || (isOn())) {
				setFixedWidth(KileMultiTabBarButton::sizeHint().width());
			} else setFixedWidth(36);
		}
	} else {
                if ((!isOn()) || (!m_showActiveTabText))
                {
	                setFixedWidth(24);
	                setFixedHeight(24);
                        return;
                }
                if ((m_position==KileMultiTabBar::Right || m_position==KileMultiTabBar::Left))
                        setFixedHeight(m_expandedSize);
                else
                        setFixedWidth(m_expandedSize);
	}
//	QApplication::sendPostedEvents(0,QEvent::Paint | QEvent::Move | QEvent::Resize | QEvent::LayoutHint);
	QApplication::sendPostedEvents(0,QEvent::Paint);
	QApplication::flush();
}

int KileMultiTabBarTab::neededSize()
{
	QString text = m_text;
	text = text.replace(QRegExp("&(?=[^&])"), QString());
	text = text.replace("&&", "&");
	return (((m_style!=KileMultiTabBar::KDEV3)?24:0)+QFontMetrics(QFont()).width(text)+6);
}

void KileMultiTabBarTab::setSize(int size)
{
	m_expandedSize=size;
	updateState();
}

void KileMultiTabBarTab::showActiveTabText(bool show)
{
	m_showActiveTabText=show;
}

void KileMultiTabBarTab::drawButtonLabel(QPainter *p) {
	drawButton(p);
}
void KileMultiTabBarTab::drawButton(QPainter *paint)
{
	if (m_style!=KileMultiTabBar::KONQSBC) drawButtonStyled(paint);
	else  drawButtonClassic(paint);
}

void KileMultiTabBarTab::drawButtonStyled(QPainter *paint) {

	QSize sh;
	const int width = 36; // rotated
	const int height = 24;
	if ((m_style==KileMultiTabBar::KDEV3) || (m_style==KileMultiTabBar::KDEV3ICON) || (isOn())) {
		 if ((m_position==KileMultiTabBar::Left) || (m_position==KileMultiTabBar::Right))
			sh=QSize(this->height(),this->width());//KileMultiTabBarButton::sizeHint();
			else sh=QSize(this->width(),this->height());
	}
	else
		sh=QSize(width,height);

	QPixmap pixmap( sh.width(),height); ///,sh.height());
	pixmap.fill(eraseColor());
	QPainter painter(&pixmap);


	QStyle::SFlags st=QStyle::Style_Default;

	st|=QStyle::Style_Enabled;

	if (isOn()) st|=QStyle::Style_On;

	style().drawControl(QStyle::CE_PushButton,&painter,this, QRect(0,0,pixmap.width(),pixmap.height()), colorGroup(),st);
	style().drawControl(QStyle::CE_PushButtonLabel,&painter,this, QRect(0,0,pixmap.width(),pixmap.height()), colorGroup(),st);

	switch (m_position) {
		case KileMultiTabBar::Left:
			paint->rotate(-90);
			paint->drawPixmap(1-pixmap.width(),0,pixmap);
			break;
		case KileMultiTabBar::Right:
			paint->rotate(90);
			paint->drawPixmap(0,1-pixmap.height(),pixmap);
			break;

		default:
			paint->drawPixmap(0,0,pixmap);
			break;
	}
//	style().drawControl(QStyle::CE_PushButtonLabel,painter,this, QRect(0,0,pixmap.width(),pixmap.height()),
//		colorGroup(),QStyle::Style_Enabled);


}

void KileMultiTabBarTab::drawButtonClassic(QPainter *paint)
{
        QPixmap pixmap;
	if ( iconSet())
        	pixmap = iconSet()->pixmap( QIconSet::Small, QIconSet::Normal );
	paint->fillRect(0, 0, 24, 24, colorGroup().background());

	if (!isOn())
	{

		if (m_position==KileMultiTabBar::Right)
		{
			paint->fillRect(0,0,21,21,QBrush(colorGroup().background()));

			paint->setPen(colorGroup().background().dark(150));
			paint->drawLine(0,22,23,22);

			paint->drawPixmap(12-pixmap.width()/2,12-pixmap.height()/2,pixmap);

			paint->setPen(colorGroup().shadow());
			paint->drawLine(0,0,0,23);
			paint->setPen(colorGroup().background().dark(120));
			paint->drawLine(1,0,1,23);

		}
		else
		if ((m_position==KileMultiTabBar::Bottom) || (m_position==KileMultiTabBar::Top))
		{
                        paint->fillRect(0,1,23,22,QBrush(colorGroup().background()));

                        paint->drawPixmap(12-pixmap.width()/2,12-pixmap.height()/2,pixmap);

                        paint->setPen(colorGroup().background().dark(120));
                        paint->drawLine(23,0,23,23);


                        paint->setPen(colorGroup().light());
                        paint->drawLine(0,22,23,22);
                        paint->drawLine(0,23,23,23);
                	paint->setPen(colorGroup().shadow());
                	paint->drawLine(0,0,23,0);
                        paint->setPen(colorGroup().background().dark(120));
                        paint->drawLine(0,1,23,1);

		}
		else
		{
			paint->setPen(colorGroup().background().dark(120));
			paint->drawLine(0,23,23,23);
			paint->fillRect(0,0,23,21,QBrush(colorGroup().background()));
			paint->drawPixmap(12-pixmap.width()/2,12-pixmap.height()/2,pixmap);

			paint->setPen(colorGroup().light());
			paint->drawLine(23,0,23,23);
			paint->drawLine(22,0,22,23);

			paint->setPen(colorGroup().shadow());
			paint->drawLine(0,0,0,23);

		}


	}
	else
	{
		if (m_position==KileMultiTabBar::Right)
		{
			paint->setPen(colorGroup().shadow());
			paint->drawLine(0,height()-1,23,height()-1);
			paint->drawLine(0,height()-2,23,height()-2);
			paint->drawLine(23,0,23,height()-1);
			paint->drawLine(22,0,22,height()-1);
			paint->fillRect(0,0,21,height()-3,QBrush(colorGroup().light()));
			paint->drawPixmap(10-pixmap.width()/2,10-pixmap.height()/2,pixmap);

			if (m_showActiveTabText)
			{
				if (height()<25+4) return;

				QPixmap tpixmap(height()-25-3, width()-2);
				QPainter painter(&tpixmap);

				painter.fillRect(0,0,tpixmap.width(),tpixmap.height(),QBrush(colorGroup().light()));

				painter.setPen(colorGroup().text());
				painter.drawText(0,+width()/2+QFontMetrics(QFont()).height()/2,m_text);

				paint->rotate(90);
				KILE_DEBUG()<<"tpixmap.width:"<<tpixmap.width()<<endl;
				paint->drawPixmap(25,-tpixmap.height()+1,tpixmap);
			}

		}
		else
		if (m_position==KileMultiTabBar::Top)
		{
			paint->fillRect(0,0,width()-1,23,QBrush(colorGroup().light()));
			paint->drawPixmap(10-pixmap.width()/2,10-pixmap.height()/2,pixmap);
			if (m_showActiveTabText)
			{
				paint->setPen(colorGroup().text());
				paint->drawText(25,height()/2+QFontMetrics(QFont()).height()/2,m_text);
			}
		}
		else
		if (m_position==KileMultiTabBar::Bottom)
		{
			paint->setPen(colorGroup().shadow());
			paint->drawLine(0,23,width()-1,23);
			paint->drawLine(0,22,width()-1,22);
			paint->fillRect(0,0,width()-1,21,QBrush(colorGroup().light()));
			paint->drawPixmap(10-pixmap.width()/2,10-pixmap.height()/2,pixmap);
			if (m_showActiveTabText)
			{
				paint->setPen(colorGroup().text());
				paint->drawText(25,height()/2+QFontMetrics(QFont()).height()/2,m_text);
			}

		}
		else
		{


			paint->setPen(colorGroup().shadow());
			paint->drawLine(0,height()-1,23,height()-1);
			paint->drawLine(0,height()-2,23,height()-2);
			paint->fillRect(0,0,23,height()-3,QBrush(colorGroup().light()));
			paint->drawPixmap(10-pixmap.width()/2,10-pixmap.height()/2,pixmap);
			if (m_showActiveTabText)
			{

		       		if (height()<25+4) return;

                                QPixmap tpixmap(height()-25-3, width()-2);
                                QPainter painter(&tpixmap);

                                painter.fillRect(0,0,tpixmap.width(),tpixmap.height(),QBrush(colorGroup().light()));

                                painter.setPen(colorGroup().text());
                                painter.drawText(tpixmap.width()-QFontMetrics(QFont()).width(m_text),+width()/2+QFontMetrics(QFont()).height()/2,m_text);

                                paint->rotate(-90);
                                KILE_DEBUG()<<"tpixmap.width:"<<tpixmap.width()<<endl;

				paint->drawPixmap(-24-tpixmap.width(),2,tpixmap);

			}

		}

	}
}







KileMultiTabBar::KileMultiTabBar(KileMultiTabBarMode bm, QWidget *parent,const char *name):QWidget(parent,name)
{
	m_buttons.setAutoDelete(false);
	if (bm==Vertical)
	{
		m_l=new QVBoxLayout(this);
		setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding, true);
//		setFixedWidth(24);
	}
	else
	{
		m_l=new QHBoxLayout(this);
		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed, true);
//		setFixedHeight(24);
	}
	m_l->setMargin(0);
	m_l->setAutoAdd(false);

	m_internal=new KileMultiTabBarInternal(this,bm);
	setPosition((bm==KileMultiTabBar::Vertical)?KileMultiTabBar::Right:KileMultiTabBar::Bottom);
	setStyle(VSNET);
	//	setStyle(KDEV3);
	//setStyle(KONQSBC);
	m_l->insertWidget(0,m_internal);
	m_l->insertWidget(0,m_btnTabSep=new QFrame(this));
	m_btnTabSep->setFixedHeight(4);
	m_btnTabSep->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	m_btnTabSep->setLineWidth(2);
	m_btnTabSep->hide();

	updateGeometry();
}

KileMultiTabBar::~KileMultiTabBar() {
}

/*int KileMultiTabBar::insertButton(QPixmap pic,int id ,const QString&)
{
  (new KToolbarButton(pic,id,m_internal))->show();
  return 0;
}*/

int KileMultiTabBar::appendButton(const QPixmap &pic ,int id,QPopupMenu *popup,const QString&)
{
	KileMultiTabBarButton  *btn;
	m_buttons.append(btn= new KileMultiTabBarButton(pic,QString::null,
			popup,id,this,m_position,m_internal->m_style));
	m_l->insertWidget(0,btn);
	btn->show();
	m_btnTabSep->show();
	return 0;
}

void KileMultiTabBar::updateSeparator() {
	bool hideSep=true;
	for (QPtrListIterator<KileMultiTabBarButton> it(m_buttons);it.current();++it){
		if (it.current()->isVisibleTo(this)) {
			hideSep=false;
			break;
		}
	}
	if (hideSep) m_btnTabSep->hide();
		else m_btnTabSep->show();

}

int KileMultiTabBar::appendTab(const QPixmap &pic ,int id ,const QString& text)
{
 m_internal->appendTab(pic,id,text);
 return 0;
}

KileMultiTabBarButton* KileMultiTabBar::button(int id) const
{
	for (QPtrListIterator<KileMultiTabBarButton> it(m_buttons);it.current();++it){
		if (it.current()->id()==id) return it.current();
	}
        return 0;
}

KileMultiTabBarTab* KileMultiTabBar::tab(int id) const
{
	return m_internal->tab(id);
}



void KileMultiTabBar::removeButton(int id)
{
	for (uint pos=0;pos<m_buttons.count();pos++)
	{
		if (m_buttons.at(pos)->id()==id)
		{
			m_buttons.take(pos)->deleteLater();
			break;
		}
	}
	if (m_buttons.count()==0) m_btnTabSep->hide();
}

void KileMultiTabBar::removeTab(int id)
{
	m_internal->removeTab(id);
}

void KileMultiTabBar::setTab(int id,bool state)
{
	KileMultiTabBarTab *ttab=tab(id);
	if (ttab)
	{
		ttab->setState(state);
	}
}

bool KileMultiTabBar::isTabRaised(int id) const
{
	KileMultiTabBarTab *ttab=tab(id);
	if (ttab)
	{
		return ttab->isOn();
	}

	return false;
}


void KileMultiTabBar::showActiveTabTexts(bool show)
{
	m_internal->showActiveTabTexts(show);
}

void KileMultiTabBar::setStyle(KileMultiTabBarStyle style)
{
	m_internal->setStyle(style);
}

KileMultiTabBar::KileMultiTabBarStyle KileMultiTabBar::tabStyle() const
{
	return m_internal->m_style;
}

void KileMultiTabBar::setPosition(KileMultiTabBarPosition pos)
{
	m_position=pos;
	m_internal->setPosition(pos);
	for (uint i=0;i<m_buttons.count();i++)
		m_buttons.at(i)->setPosition(pos);
}

KileMultiTabBar::KileMultiTabBarPosition KileMultiTabBar::position() const
{
	return m_position;
}
void KileMultiTabBar::fontChange(const QFont& /* oldFont */)
{
	for (uint i=0;i<tabs()->count();i++)
		tabs()->at(i)->resize();
	repaint();
}

QPtrList<KileMultiTabBarTab>* KileMultiTabBar::tabs() {return m_internal->tabs();}
QPtrList<KileMultiTabBarButton>* KileMultiTabBar::buttons() {return &m_buttons;}

