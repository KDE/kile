/****************************************************************************
** $Id$
**
** Definition of QIconView widget class
**
** Created : 990707
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the iconview module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef KILEICONVIEW_H
#define KILEICONVIEW_H

#ifndef QT_H
#include <qscrollview.h>
#include <qstring.h>
#include <qrect.h>
#include <qpoint.h>
#include <qsize.h>
#include <qfont.h>
#include <qbitmap.h>
#include <qpicture.h>
#endif // QT_H

class KileIconView;
class QPainter;
class QMimeSource;
class QMouseEvent;
class QKeyEvent;
class QFocusEvent;
class QShowEvent;
class KileIconViewItem;
class QStringList;

class KileIconViewItemPrivate;

class KileIconViewItem : public Qt
{
    friend class KileIconView;

public:
    KileIconViewItem( KileIconView *parent );
    KileIconViewItem( KileIconView *parent, KileIconViewItem *after );
    KileIconViewItem( KileIconView *parent, const QString &text );
    KileIconViewItem( KileIconView *parent, KileIconViewItem *after, const QString &text );
    KileIconViewItem( KileIconView *parent, const QString &text, const QPixmap &icon );
    KileIconViewItem( KileIconView *parent, KileIconViewItem *after, const QString &text, const QPixmap &icon );
#ifndef QT_NO_PICTURE
    KileIconViewItem( KileIconView *parent, const QString &text, const QPicture &picture );
    KileIconViewItem( KileIconView *parent, KileIconViewItem *after, const QString &text, const QPicture &picture );
#endif
    virtual ~KileIconViewItem();

    virtual void setRenameEnabled( bool allow );
    virtual void setDragEnabled( bool allow );
    virtual void setDropEnabled( bool allow );

    virtual QString text() const;
    virtual QPixmap *pixmap() const;
#ifndef QT_NO_PICTURE
    virtual QPicture *picture() const;
#endif
    virtual QString key() const;

    bool renameEnabled() const;
    bool dragEnabled() const;
    bool dropEnabled() const;

    KileIconView *iconView() const;
    KileIconViewItem *prevItem() const;
    KileIconViewItem *nextItem() const;

    int index() const;

    virtual void setSelected( bool s, bool cb );
    virtual void setSelected( bool s );
    virtual void setSelectable( bool s );

    bool isSelected() const;
    bool isSelectable() const;

    virtual void repaint();

    virtual bool move( int x, int y );
    virtual void moveBy( int dx, int dy );
    virtual bool move( const QPoint &pnt );
    virtual void moveBy( const QPoint &pnt );

    QRect rect() const;
    int x() const;
    int y() const;
    int width() const;
    int height() const;
    QSize size() const;
    QPoint pos() const;
    QRect textRect( bool relative = TRUE ) const;
    QRect pixmapRect( bool relative = TRUE ) const;
    bool contains( const QPoint& pnt ) const;
    bool intersects( const QRect& r ) const;

    virtual bool acceptDrop( const QMimeSource *mime ) const;

    virtual int compare( KileIconViewItem *i ) const;

    virtual void setText( const QString &text );
    virtual void setPixmap( const QPixmap &icon );
#ifndef QT_NO_PICTURE
    virtual void setPicture( const QPicture &icon );
#endif
    virtual void setText( const QString &text, bool recalc, bool redraw = TRUE );
    virtual void setPixmap( const QPixmap &icon, bool recalc, bool redraw = TRUE );
    virtual void setKey( const QString &k );

    virtual int rtti() const;
    static int RTTI;

protected:
    virtual void calcRect( const QString &text_ = QString::null );
    virtual void paintItem( QPainter *p, const QColorGroup &cg );
    virtual void paintFocus( QPainter *p, const QColorGroup &cg );
    virtual void dragEntered();
    virtual void dragLeft();
    void setItemRect( const QRect &r );
    void setTextRect( const QRect &r );
    void setPixmapRect( const QRect &r );
    void calcTmpText();

private:
    void init( KileIconViewItem *after = 0
#ifndef QT_NO_PICTURE
	       , QPicture *pic = 0
#endif
	       );
    void checkRect();

    KileIconView *view;
    QString itemText, itemKey;
    QString tmpText;
    QPixmap *itemIcon;
#ifndef QT_NO_PICTURE
    QPicture *itemPic;
#endif
    KileIconViewItem *prev, *next;
    uint allow_rename : 1;
    uint allow_drag : 1;
    uint allow_drop : 1;
    uint selected : 1;
    uint selectable : 1;
    uint dirty : 1;
    uint wordWrapDirty : 1;
    QRect itemRect, itemTextRect, itemIconRect;
    QRect oldRect;

    KileIconViewItemPrivate *d;private:
};

class KileIconViewPrivate;          /* don't touch */

class KileIconView : public QScrollView
{
    friend class KileIconViewItem;
    friend class KileIconViewPrivate;

    Q_OBJECT
    // #### sorting and sort direction do not work
    Q_ENUMS( SelectionMode ItemTextPos Arrangement ResizeMode )
    Q_PROPERTY( bool sorting READ sorting )
    Q_PROPERTY( bool sortDirection READ sortDirection )
    Q_PROPERTY( SelectionMode selectionMode READ selectionMode WRITE setSelectionMode )
    Q_PROPERTY( int gridX READ gridX WRITE setGridX )
    Q_PROPERTY( int gridY READ gridY WRITE setGridY )
    Q_PROPERTY( int spacing READ spacing WRITE setSpacing )
    Q_PROPERTY( ItemTextPos itemTextPos READ itemTextPos WRITE setItemTextPos )
    Q_PROPERTY( QBrush itemTextBackground READ itemTextBackground WRITE setItemTextBackground )
    Q_PROPERTY( Arrangement arrangement READ arrangement WRITE setArrangement )
    Q_PROPERTY( ResizeMode resizeMode READ resizeMode WRITE setResizeMode )
    Q_PROPERTY( int maxItemWidth READ maxItemWidth WRITE setMaxItemWidth )
    Q_PROPERTY( int maxItemTextLength READ maxItemTextLength WRITE setMaxItemTextLength )
    Q_PROPERTY( bool autoArrange READ autoArrange WRITE setAutoArrange )
    Q_PROPERTY( bool itemsMovable READ itemsMovable WRITE setItemsMovable )
    Q_PROPERTY( bool wordWrapIconText READ wordWrapIconText WRITE setWordWrapIconText )
    Q_PROPERTY( bool showToolTips READ showToolTips WRITE setShowToolTips )
    Q_PROPERTY( uint count READ count )

public:
    enum SelectionMode {
	Single = 0,
	Multi,
	Extended,
	NoSelection
    };
    enum Arrangement {
	LeftToRight = 0,
	TopToBottom
    };
    enum ResizeMode {
	Fixed = 0,
	Adjust
    };
    enum ItemTextPos {
	Bottom = 0,
	Right
    };

    KileIconView( QWidget* parent=0, const char* name=0, WFlags f = 0 );
    virtual ~KileIconView();

    virtual void insertItem( KileIconViewItem *item, KileIconViewItem *after = 0L );
    virtual void takeItem( KileIconViewItem *item );

    int index( const KileIconViewItem *item ) const;

    KileIconViewItem *firstItem() const;
    KileIconViewItem *lastItem() const;
    KileIconViewItem *currentItem() const;
    virtual void setCurrentItem( KileIconViewItem *item );
    virtual void setSelected( KileIconViewItem *item, bool s, bool cb = FALSE );

    uint count() const;

public:
    virtual void showEvent( QShowEvent * );

    virtual void setSelectionMode( SelectionMode m );
    SelectionMode selectionMode() const;

    KileIconViewItem *findItem( const QPoint &pos ) const;
    KileIconViewItem *findItem( const QString &text, ComparisonFlags compare = BeginsWith ) const;
    virtual void selectAll( bool select );
    virtual void clearSelection();
    virtual void invertSelection();

    virtual void repaintItem( KileIconViewItem *item );

    void ensureItemVisible( KileIconViewItem *item );
    KileIconViewItem* findFirstVisibleItem( const QRect &r ) const;
    KileIconViewItem* findLastVisibleItem( const QRect &r ) const;

    virtual void clear();

    virtual void setGridX( int rx );
    virtual void setGridY( int ry );
    int gridX() const;
    int gridY() const;
    virtual void setSpacing( int sp );
    int spacing() const;
    virtual void setItemTextPos( ItemTextPos pos );
    ItemTextPos itemTextPos() const;
    virtual void setItemTextBackground( const QBrush &b );
    QBrush itemTextBackground() const;
    virtual void setArrangement( Arrangement am );
    Arrangement arrangement() const;
    virtual void setResizeMode( ResizeMode am );
    ResizeMode resizeMode() const;
    virtual void setMaxItemWidth( int w );
    int maxItemWidth() const;
    virtual void setMaxItemTextLength( int w );
    int maxItemTextLength() const;
    virtual void setAutoArrange( bool b );
    bool autoArrange() const;
    virtual void setShowToolTips( bool b );
    bool showToolTips() const;

    void setSorting( bool sort, bool ascending = TRUE );
    bool sorting() const;
    bool sortDirection() const;

    virtual void setItemsMovable( bool b );
    bool itemsMovable() const;
    virtual void setWordWrapIconText( bool b );
    bool wordWrapIconText() const;

    bool eventFilter( QObject * o, QEvent * );

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    virtual void sort( bool ascending = TRUE );

    virtual void setFont( const QFont & );
    virtual void setPalette( const QPalette & );

    bool isRenaming() const;

public slots:
    virtual void arrangeItemsInGrid( const QSize &grid, bool update = TRUE );
    virtual void arrangeItemsInGrid( bool update = TRUE );
    virtual void setContentsPos( int x, int y );
    virtual void updateContents();

signals:
    void selectionChanged();
    void selectionChanged( KileIconViewItem *item );
    void currentChanged( KileIconViewItem *item );
    void clicked( KileIconViewItem * );
    void clicked( KileIconViewItem *, const QPoint & );
    void pressed( KileIconViewItem * );
    void pressed( KileIconViewItem *, const QPoint & );

    void doubleClicked( KileIconViewItem *item );
    void returnPressed( KileIconViewItem *item );
    void rightButtonClicked( KileIconViewItem* item, const QPoint& pos );
    void rightButtonPressed( KileIconViewItem* item, const QPoint& pos );
    void mouseButtonPressed( int button, KileIconViewItem* item, const QPoint& pos );
    void mouseButtonClicked( int button, KileIconViewItem* item, const QPoint& pos );
    void contextMenuRequested( KileIconViewItem* item, const QPoint &pos );
    void moved();
    void onItem( KileIconViewItem *item );
    void onViewport();
    void itemRenamed( KileIconViewItem *item, const QString & );
    void itemRenamed( KileIconViewItem *item );

protected slots:
    virtual void doAutoScroll();
    virtual void adjustItems();
    virtual void slotUpdate();

private slots:
    void clearInputString();
    void movedContents( int dx, int dy );

protected:
    void drawContents( QPainter *p, int cx, int cy, int cw, int ch );
    void contentsMousePressEvent( QMouseEvent *e );
    void contentsMouseReleaseEvent( QMouseEvent *e );
    void contentsMouseMoveEvent( QMouseEvent *e );
    void contentsMouseDoubleClickEvent( QMouseEvent *e );
    void contentsContextMenuEvent( QContextMenuEvent *e );

    void resizeEvent( QResizeEvent* e );
    void keyPressEvent( QKeyEvent *e );
    void focusInEvent( QFocusEvent *e );
    void focusOutEvent( QFocusEvent *e );
    void enterEvent( QEvent *e );

    virtual void drawRubber( QPainter *p );
    virtual void insertInGrid( KileIconViewItem *item );
    virtual void drawBackground( QPainter *p, const QRect &r );

    void emitSelectionChanged( KileIconViewItem * i = 0 );
    void emitRenamed( KileIconViewItem *item );

    KileIconViewItem *makeRowLayout( KileIconViewItem *begin, int &y, bool &changed );

    void styleChange( QStyle& );
    void windowActivationChange( bool );

private:
    void contentsMousePressEventEx( QMouseEvent *e );
    virtual void drawDragShapes( const QPoint &pnt );
    void drawContents( QPainter* );
    void findItemByName( const QString &text );
    void handleItemChange( KileIconViewItem *old, bool shift, bool control );

    int calcGridNum( int w, int x ) const;
    KileIconViewItem *rowBegin( KileIconViewItem *item ) const;
    void updateItemContainer( KileIconViewItem *item );
    void appendItemContainer();
    void rebuildContainers();
    QBitmap mask( QPixmap *pix ) const;

    KileIconViewPrivate *d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    KileIconView( const KileIconView & );
    KileIconView& operator=( const KileIconView & );
#endif
};

#endif
