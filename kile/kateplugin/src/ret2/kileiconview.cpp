/****************************************************************************
** $Id$
** kileiconview.cpp (c) P.Brachet 2002 from qiconview.cpp
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

// Solaris redefines connect -> __xnet_connect with _XOPEN_SOURCE_EXTENDED.
#if defined(connect)
#undef connect
#endif

#include "kileiconview.h"

#include "qfontmetrics.h"
#include "qpainter.h"
#include "qevent.h"
#include "qpalette.h"
#include "qmime.h"
#include "qimage.h"
#include "qpen.h"
#include "qbrush.h"
#include "qtimer.h"
#include "qcursor.h"
#include "qapplication.h"
#include "qtextedit.h"
#include "qmemarray.h"
#include "qptrlist.h"
#include "qvbox.h"
#include "qtooltip.h"
#include "qbitmap.h"
#include "qpixmapcache.h"
#include "qptrdict.h"
#include "qstringlist.h"
#include "qcleanuphandler.h"
#include "qstyle.h"

#include <stdlib.h>
#include <math.h>
#include <limits.h>

#define RECT_EXTENSION 300

static const char * const unknown_xpm[] = {
    "32 32 11 1",
    "c c #ffffff",
    "g c #c0c0c0",
    "a c #c0ffc0",
    "h c #a0a0a4",
    "d c #585858",
    "f c #303030",
    "i c #400000",
    "b c #00c000",
    "e c #000000",
    "# c #000000",
    ". c None",
    "...###..........................",
    "...#aa##........................",
    ".###baaa##......................",
    ".#cde#baaa##....................",
    ".#cccdeebaaa##..##f.............",
    ".#cccccdeebaaa##aaa##...........",
    ".#cccccccdeebaaaaaaaa##.........",
    ".#cccccccccdeebaaaaaaa#.........",
    ".#cccccgcgghhebbbbaaaaa#........",
    ".#ccccccgcgggdebbbbbbaa#........",
    ".#cccgcgcgcgghdeebiebbba#.......",
    ".#ccccgcggggggghdeddeeba#.......",
    ".#cgcgcgcggggggggghghdebb#......",
    ".#ccgcggggggggghghghghd#b#......",
    ".#cgcgcggggggggghghghhd#b#......",
    ".#gcggggggggghghghhhhhd#b#......",
    ".#cgcggggggggghghghhhhd#b#......",
    ".#ggggggggghghghhhhhhhdib#......",
    ".#gggggggggghghghhhhhhd#b#......",
    ".#hhggggghghghhhhhhhhhd#b#......",
    ".#ddhhgggghghghhhhhhhhd#b#......",
    "..##ddhhghghhhhhhhhhhhdeb#......",
    "....##ddhhhghhhhhhhhhhd#b#......",
    "......##ddhhhhhhhhhhhhd#b#......",
    "........##ddhhhhhhhhhhd#b#......",
    "..........##ddhhhhhhhhd#b#......",
    "............##ddhhhhhhd#b###....",
    "..............##ddhhhhd#b#####..",
    "................##ddhhd#b######.",
    "..................##dddeb#####..",
    "....................##d#b###....",
    "......................####......"};

static QPixmap *unknown_icon = 0;
static QPixmap *qiv_buffer_pixmap = 0;
#if !defined(Q_WS_X11)
static QPixmap *qiv_selection = 0;
#endif
static bool optimize_layout = FALSE;

static QCleanupHandler<QPixmap> qiv_cleanup_pixmap;

#if !defined(Q_WS_X11)
static void createSelectionPixmap( const QColorGroup &cg )
{
    QBitmap m( 2, 2 );
    m.fill( Qt::color1 );
    QPainter p( &m );
    p.setPen( Qt::color0 );
    for ( int j = 0; j < 2; ++j ) {
	p.drawPoint( j % 2, j );
    }
    p.end();

    qiv_selection = new QPixmap( 2, 2 );
    qiv_cleanup_pixmap.add( &qiv_selection );
    qiv_selection->fill( Qt::color0 );
    qiv_selection->setMask( m );
    qiv_selection->fill( cg.highlight() );
}
#endif

static QPixmap *get_qiv_buffer_pixmap( const QSize &s )
{
    if ( !qiv_buffer_pixmap ) {
	qiv_buffer_pixmap = new QPixmap( s );
	qiv_cleanup_pixmap.add( &qiv_buffer_pixmap );
	return qiv_buffer_pixmap;
    }

    qiv_buffer_pixmap->resize( s );
    return qiv_buffer_pixmap;
}


class KileIconViewPrivate
{
public:
    KileIconViewItem *firstItem, *lastItem;
    uint count;
    KileIconView::SelectionMode selectionMode;
    KileIconViewItem *currentItem, *tmpCurrentItem, *highlightedItem,  *startDragItem, *pressedItem, *selectAnchor;
    QRect *rubber;
    QTimer *scrollTimer, *adjustTimer, *updateTimer, *inputTimer,
	*fullRedrawTimer;
    int rastX, rastY, spacing;
    int dragItems;
    QPoint oldDragPos;
    KileIconView::Arrangement arrangement;
    KileIconView::ResizeMode resizeMode;
    QSize oldSize;
    int numDragItems, cachedW, cachedH;
    int maxItemWidth, maxItemTextLength;
    QPoint dragStart;
    QString currInputString;
    KileIconView::ItemTextPos itemTextPos;
#ifndef QT_NO_CURSOR
    QCursor oldCursor;
#endif
    int cachedContentsX, cachedContentsY;
    QBrush itemTextBrush;
    QRegion clipRegion;
    QPoint dragStartPos;
    QFontMetrics *fm;
    int minLeftBearing, minRightBearing;

    uint mousePressed		:1;
    uint cleared		:1;
    uint dropped		:1;
    uint clearing		:1;
    uint oldDragAcceptAction	:1;
    uint isIconDrag		:1;
    uint drawDragShapes		:1;
    uint dirty			:1;
    uint rearrangeEnabled	:1;
    uint reorderItemsWhenInsert	:1;
    uint drawAllBack		:1;
    uint resortItemsWhenInsert	:1;
    uint sortDirection		:1;
    uint wordWrapIconText	:1;
    uint containerUpdateLocked	:1;
    uint firstSizeHint		:1;
    uint showTips		:1;
    uint pressedSelected	:1;
    uint dragging		:1;
    uint drawActiveSelection	:1;

    QPixmapCache maskCache;
    QPtrDict<KileIconViewItem> selectedItems;

    struct ItemContainer {
	ItemContainer( ItemContainer *pr, ItemContainer *nx, const QRect &r )
	    : p( pr ), n( nx ), rect( r ) {
		items.setAutoDelete( FALSE );
		if ( p )
		    p->n = this;
		if ( n )
		    n->p = this;
	}
	ItemContainer *p, *n;
	QRect rect;
	QPtrList<KileIconViewItem> items;
    } *firstContainer, *lastContainer;

    struct SortableItem {
	KileIconViewItem *item;
    };

};

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

#ifdef Q_OS_TEMP
static int _cdecl cmpIconViewItems( const void *n1, const void *n2 )
#else
static int cmpIconViewItems( const void *n1, const void *n2 )
#endif
{
    if ( !n1 || !n2 )
	return 0;

    KileIconViewPrivate::SortableItem *i1 = (KileIconViewPrivate::SortableItem *)n1;
    KileIconViewPrivate::SortableItem *i2 = (KileIconViewPrivate::SortableItem *)n2;

    return i1->item->compare( i2->item );
}

#if defined(Q_C_CALLBACKS)
}
#endif




class KileIconViewItemPrivate
{
public:
    KileIconViewPrivate::ItemContainer *container1, *container2;
};


KileIconViewItem::KileIconViewItem( KileIconView *parent )
    : view( parent ), itemText(), itemIcon( unknown_icon )
{
    init();
}

/*!
    Constructs a KileIconViewItem and inserts it into the icon view \a
    parent with no text and a default icon, after the icon view item
    \a after.
*/

KileIconViewItem::KileIconViewItem( KileIconView *parent, KileIconViewItem *after )
    : view( parent ), itemText(), itemIcon( unknown_icon ),
      prev( 0 ), next( 0 )
{
    init( after );
}

/*!
    Constructs an icon view item  and inserts it into the icon view \a
    parent using \a text as the text and a default icon.
*/

KileIconViewItem::KileIconViewItem( KileIconView *parent, const QString &text )
    : view( parent ), itemText( text ), itemIcon( unknown_icon )
{
    init( 0 );
}

/*!
    Constructs an icon view item and inserts it into the icon view \a
    parent using \a text as the text and a default icon, after the
    icon view item \a after.
*/

KileIconViewItem::KileIconViewItem( KileIconView *parent, KileIconViewItem *after,
			      const QString &text )
    : view( parent ), itemText( text ), itemIcon( unknown_icon )
{
    init( after );
}

/*!
    Constructs an icon view item and inserts it into the icon view \a
    parent using \a text as the text and \a icon as the icon.
*/

KileIconViewItem::KileIconViewItem( KileIconView *parent, const QString &text,
			      const QPixmap &icon )
    : view( parent ),
      itemText( text ), itemIcon( new QPixmap( icon ) )
{
    init( 0 );
}


/*!
    Constructs an icon view item and inserts it into the icon view \a
    parent using \a text as the text and \a icon as the icon, after
    the icon view item \a after.
*/

KileIconViewItem::KileIconViewItem( KileIconView *parent, KileIconViewItem *after,
			      const QString &text, const QPixmap &icon )
    : view( parent ), itemText( text ), itemIcon( new QPixmap( icon ) )
{
    init( after );
}

/*!
    Constructs an icon view item and inserts it into the icon view \a
    parent using \a text as the text and \a picture as the icon.
*/

#ifndef QT_NO_PICTURE
KileIconViewItem::KileIconViewItem( KileIconView *parent, const QString &text,
			      const QPicture &picture )
    : view( parent ), itemText( text ), itemIcon( 0 )
{
    init( 0, new QPicture( picture ) );
}

/*!
    Constructs an icon view item and inserts it into the icon view \a
    parent using \a text as the text and \a picture as the icon, after
    the icon view item \a after.
*/

KileIconViewItem::KileIconViewItem( KileIconView *parent, KileIconViewItem *after,
			      const QString &text, const QPicture &picture )
    : view( parent ), itemText( text ), itemIcon( 0 )
{
    init( after, new QPicture( picture ) );
}
#endif

/*!
  This private function initializes the icon view item and inserts it
  into the icon view.
*/

void KileIconViewItem::init( KileIconViewItem *after
#ifndef QT_NO_PICTURE
			  , QPicture *pic
#endif
			  )
{
    d = new KileIconViewItemPrivate;
    d->container1 = 0;
    d->container2 = 0;
    prev = next = 0;
    allow_rename = FALSE;
    allow_drag = TRUE;
    allow_drop = TRUE;
    selected = FALSE;
    selectable = TRUE;
#ifndef QT_NO_PICTURE
    itemPic = pic;
#endif
    if ( view ) {
	itemKey = itemText;
	dirty = TRUE;
	wordWrapDirty = TRUE;
	itemRect = QRect( -1, -1, 0, 0 );
	calcRect();
	view->insertItem( this, after );
    }
}

/*!
    Destroys the icon view item and tells the parent icon view that
    the item has been destroyed.
*/

KileIconViewItem::~KileIconViewItem()
{
    if ( view && !view->d->clearing )
	view->takeItem( this );
    view = 0;
    if ( itemIcon && itemIcon->serialNumber() != unknown_icon->serialNumber() )
	delete itemIcon;
#ifndef QT_NO_PICTURE
    delete itemPic;
#endif
    delete d;
}

int KileIconViewItem::RTTI = 0;

/*!
    Returns 0.

    Make your derived classes return their own values for rtti(), so
    that you can distinguish between icon view item types. You should
    use values greater than 1000, preferably a large random number, to
    allow for extensions to this class.
*/

int KileIconViewItem::rtti() const
{
    return RTTI;
}


/*!
    Sets \a text as the text of the icon view item. This function
    might be a no-op if you reimplement text().

    \sa text()
*/

void KileIconViewItem::setText( const QString &text )
{
    if ( text == itemText )
	return;

    wordWrapDirty = TRUE;
    itemText = text;
    if ( itemKey.isEmpty() )
	itemKey = itemText;

    QRect oR = rect();
    calcRect();
    oR = oR.unite( rect() );

    if ( view ) {
	if ( QRect( view->contentsX(), view->contentsY(),
		    view->visibleWidth(), view->visibleHeight() ).
	     intersects( oR ) )
	    view->repaintContents( oR.x() - 1, oR.y() - 1,
				   oR.width() + 2, oR.height() + 2, FALSE );
      }
}

/*!
    Sets \a k as the sort key of the icon view item. By default
    text() is used for sorting.

    \sa compare()
*/

void KileIconViewItem::setKey( const QString &k )
{
    if ( k == itemKey )
	return;

    itemKey = k;
}

/*!
    Sets \a icon as the item's icon in the icon view. This function
    might be a no-op if you reimplement pixmap().

    \sa pixmap()
*/

void KileIconViewItem::setPixmap( const QPixmap &icon )
{
    if ( itemIcon && itemIcon == unknown_icon )
	itemIcon = 0;

    if ( itemIcon )
	*itemIcon = icon;
    else
	itemIcon = new QPixmap( icon );
    QRect oR = rect();
    calcRect();
    oR = oR.unite( rect() );

    if ( view ) {
	if ( QRect( view->contentsX(), view->contentsY(),
		    view->visibleWidth(), view->visibleHeight() ).
	     intersects( oR ) )
	    view->repaintContents( oR.x() - 1, oR.y() - 1,
				   oR.width() + 2, oR.height() + 2, FALSE );
    }
}

/*!
    Sets \a icon as the item's icon in the icon view. This function
    might be a no-op if you reimplement picture().

    \sa picture()
*/

#ifndef QT_NO_PICTURE
void KileIconViewItem::setPicture( const QPicture &icon )
{
    // clear assigned pixmap if any
    if ( itemIcon ) {
	if ( itemIcon == unknown_icon ) {
	    itemIcon = 0;
	} else {
	    delete itemIcon;
	    itemIcon = 0;
	}
    }
    if ( itemPic )
	delete itemPic;
    itemPic = new QPicture( icon );

    QRect oR = rect();
    calcRect();
    oR = oR.unite( rect() );

    if ( view ) {
	if ( QRect( view->contentsX(), view->contentsY(),
		    view->visibleWidth(), view->visibleHeight() ).
	     intersects( oR ) )
	    view->repaintContents( oR.x() - 1, oR.y() - 1,
				   oR.width() + 2, oR.height() + 2, FALSE );
    }
}
#endif

/*!
    \overload

    Sets \a text as the text of the icon view item. If \a recalc is
    TRUE, the icon view's layout is recalculated. If \a redraw is TRUE
    (the default), the icon view is repainted.

    \sa text()
*/

void KileIconViewItem::setText( const QString &text, bool recalc, bool redraw )
{
    if ( text == itemText )
	return;

    wordWrapDirty = TRUE;
    itemText = text;

    if ( recalc )
	calcRect();
    if ( redraw )
	repaint();
}

/*!
    \overload

    Sets \a icon as the item's icon in the icon view. If \a recalc is
    TRUE, the icon view's layout is recalculated. If \a redraw is TRUE
    (the default), the icon view is repainted.

    \sa pixmap()
*/

void KileIconViewItem::setPixmap( const QPixmap &icon, bool recalc, bool redraw )
{
    if ( itemIcon && itemIcon == unknown_icon )
	itemIcon = 0;

    if ( itemIcon )
	*itemIcon = icon;
    else
	itemIcon = new QPixmap( icon );

    if ( recalc )
	calcRect();
    if ( redraw ) {
	if ( recalc ) {
	    QRect oR = rect();
	    calcRect();
	    oR = oR.unite( rect() );

	    if ( view ) {
		if ( QRect( view->contentsX(), view->contentsY(),
			    view->visibleWidth(), view->visibleHeight() ).
		     intersects( oR ) )
		    view->repaintContents( oR.x() - 1, oR.y() - 1,
				       oR.width() + 2, oR.height() + 2, FALSE );
	    }
	} else {
	    repaint();
	}
    }
}

/*!
    If \a allow is TRUE, the user can rename the icon view item by
    clicking on the text (or pressing F2) while the item is selected
    (in-place renaming). If \a allow is FALSE, in-place renaming is
    not possible.
*/

void KileIconViewItem::setRenameEnabled( bool allow )
{
    allow_rename = (uint)allow;
}

/*!
    If \a allow is TRUE, the icon view permits the user to drag the
    icon view item either to another position within the icon view or
    to somewhere outside of it. If \a allow is FALSE, the item cannot
    be dragged.
*/

void KileIconViewItem::setDragEnabled( bool allow )
{
    allow_drag = (uint)allow;
}

/*!
    If \a allow is TRUE, the icon view lets the user drop something on
    this icon view item.
*/

void KileIconViewItem::setDropEnabled( bool allow )
{
    allow_drop = (uint)allow;
}

/*!
    Returns the text of the icon view item. Normally you set the text
    of the item with setText(), but sometimes it's inconvenient to
    call setText() for every item; so you can subclass KileIconViewItem,
    reimplement this function, and return the text of the item. If you
    do this, you must call calcRect() manually each time the text
    (and therefore its size) changes.

    \sa setText()
*/

QString KileIconViewItem::text() const
{
    return itemText;
}

/*!
    Returns the key of the icon view item or text() if no key has been
    explicitly set.

    \sa setKey(), compare()
*/

QString KileIconViewItem::key() const
{
    return itemKey;
}

/*!
    Returns the icon of the icon view item if it is a pixmap, or 0 if
    it is a picture. In the latter case use picture() instead.
    Normally you set the pixmap of the item with setPixmap(), but
    sometimes it's inconvenient to call setPixmap() for every item. So
    you can subclass KileIconViewItem, reimplement this function and
    return a pointer to the item's pixmap. If you do this, you \e must
    call calcRect() manually each time the size of this pixmap
    changes.

    \sa setPixmap()
*/

QPixmap *KileIconViewItem::pixmap() const
{
    return itemIcon;
}

/*!
    Returns the icon of the icon view item if it is a picture, or 0 if
    it is a pixmap. In the latter case use pixmap() instead. Normally
    you set the picture of the item with setPicture(), but sometimes
    it's inconvenient to call setPicture() for every item. So you can
    subclass KileIconViewItem, reimplement this function and return a
    pointer to the item's picture. If you do this, you \e must call
    calcRect() manually each time the size of this picture changes.

    \sa setPicture()
*/

#ifndef QT_NO_PICTURE
QPicture *KileIconViewItem::picture() const
{
    return itemPic;
}
#endif

/*!
    Returns TRUE if the item can be renamed by the user with in-place
    renaming; otherwise returns FALSE.

    \sa setRenameEnabled()
*/

bool KileIconViewItem::renameEnabled() const
{
    return (bool)allow_rename;
}

/*!
    Returns TRUE if the user is allowed to drag the icon view item;
    otherwise returns FALSE.

    \sa setDragEnabled()
*/

bool KileIconViewItem::dragEnabled() const
{
    return (bool)allow_drag;
}

/*!
    Returns TRUE if the user is allowed to drop something onto the
    item; otherwise returns FALSE.

    \sa setDropEnabled()
*/

bool KileIconViewItem::dropEnabled() const
{
    return (bool)allow_drop;
}

/*!
    Returns a pointer to this item's icon view parent.
*/

KileIconView *KileIconViewItem::iconView() const
{
    return view;
}

/*!
    Returns a pointer to the previous item, or 0 if this is the first
    item in the icon view.

    \sa nextItem() KileIconView::firstItem()
*/

KileIconViewItem *KileIconViewItem::prevItem() const
{
    return prev;
}

/*!
    Returns a pointer to the next item, or 0 if this is the last item
    in the icon view.

    To find the first item use KileIconView::firstItem().

    Example:
    \code
    KileIconViewItem *item;
    for ( item = iconView->firstItem(); item; item = item->nextItem() )
	do_something_with( item );
    \endcode

    \sa prevItem()
*/

KileIconViewItem *KileIconViewItem::nextItem() const
{
    return next;
}

/*!
    Returns the index of this item in the icon view, or -1 if an error
    occurred.
*/

int KileIconViewItem::index() const
{
    if ( view )
	return view->index( this );

    return -1;
}



/*!
    \overload

    This variant is equivalent to calling the other variant with \e cb
    set to FALSE.
*/

void KileIconViewItem::setSelected( bool s )
{
    setSelected( s, FALSE );
}

/*!
    Selects or unselects the item, depending on \a s; it may also
    unselect other items, depending on KileIconView::selectionMode() and
    \a cb.

    If \a s is FALSE, the item is unselected.

    If \a s is TRUE and KileIconView::selectionMode() is \c Single, the
    item is selected and the item previously selected is unselected.

    If \a s is TRUE and KileIconView::selectionMode() is \c Extended, the
    item is selected. If \a cb is TRUE, the selection state of the
    other items is left unchanged. If \a cb is FALSE (the default) all
    other items are unselected.

    If \a s is TRUE and KileIconView::selectionMode() is \c Multi, the
    item is selected.

    Note that \a cb is used only if KileIconView::selectionMode() is \c
    Extended; cb defaults to FALSE.

    All items whose selection status changes repaint themselves.
*/

void KileIconViewItem::setSelected( bool s, bool cb )
{
    if ( !view )
	return;
    if ( view->selectionMode() != KileIconView::NoSelection &&
	 selectable && s != (bool)selected ) {

	if ( view->d->selectionMode == KileIconView::Single && this != view->d->currentItem ) {
	    KileIconViewItem *o = view->d->currentItem;
	    if ( o && o->selected )
		o->selected = FALSE;
	    view->d->currentItem = this;
	    if ( o )
		o->repaint();
	    emit view->currentChanged( this );
	}

	if ( !s ) {
	    selected = FALSE;
	} else {
	    if ( view->d->selectionMode == KileIconView::Single && view->d->currentItem ) {
		view->d->currentItem->selected = FALSE;
	    }
	    if ( ( view->d->selectionMode == KileIconView::Extended && !cb ) ||
		 view->d->selectionMode == KileIconView::Single ) {
		bool b = view->signalsBlocked();
		view->blockSignals( TRUE );
		view->selectAll( FALSE );
		view->blockSignals( b );
	    }
	    selected = s;
	}

	repaint();
	if ( !view->signalsBlocked() ) {
	    bool emitIt = view->d->selectionMode == KileIconView::Single && s;
	    KileIconView *v = view;
	    emit v->selectionChanged();
	    if ( emitIt )
		emit v->selectionChanged( this );
	}
    }
}

/*!
    Sets this item to be selectable if \a enable is TRUE (the default)
    or unselectable if \a enable is FALSE.

    The user is unable to select a non-selectable item using either
    the keyboard or the mouse. (The application programmer can select
    an item in code regardless of this setting.)

    \sa isSelectable()
*/

void KileIconViewItem::setSelectable( bool enable )
{
    selectable = (uint)enable;
}

/*!
    Returns TRUE if the item is selected; otherwise returns FALSE.

    \sa setSelected()
*/

bool KileIconViewItem::isSelected() const
{
    return (bool)selected;
}

/*!
    Returns TRUE if the item is selectable; otherwise returns FALSE.

    \sa setSelectable()
*/

bool KileIconViewItem::isSelectable() const
{
    return (bool)selectable;
}

/*!
    Repaints the item.
*/

void KileIconViewItem::repaint()
{
    if ( view )
	view->repaintItem( this );
}

/*!
    Moves the item to position (\a x, \a y) in the icon view (these
    are contents coordinates).
*/

bool KileIconViewItem::move( int x, int y )
{
    if ( x == this->x() && y == this->y() )
	return FALSE;
    itemRect.setRect( x, y, itemRect.width(), itemRect.height() );
    checkRect();
    if ( view )
	view->updateItemContainer( this );
    return TRUE;
}

/*!
    Moves the item \a dx pixels in the x-direction and \a dy pixels in
    the y-direction.
*/

void KileIconViewItem::moveBy( int dx, int dy )
{
    itemRect.moveBy( dx, dy );
    checkRect();
    if ( view )
	view->updateItemContainer( this );
}

/*!
    \overload

    Moves the item to the point \a pnt.
*/

bool KileIconViewItem::move( const QPoint &pnt )
{
    return move( pnt.x(), pnt.y() );
}

/*!
    \overload

    Moves the item by the x, y values in point \a pnt.
*/

void KileIconViewItem::moveBy( const QPoint &pnt )
{
    moveBy( pnt.x(), pnt.y() );
}

/*!
    Returns the bounding rectangle of the item (in contents
    coordinates).
*/

QRect KileIconViewItem::rect() const
{
    return itemRect;
}

/*!
    Returns the x-coordinate of the item (in contents coordinates).
*/

int KileIconViewItem::x() const
{
    return itemRect.x();
}

/*!
    Returns the y-coordinate of the item (in contents coordinates).
*/

int KileIconViewItem::y() const
{
    return itemRect.y();
}

/*!
    Returns the width of the item.
*/

int KileIconViewItem::width() const
{
    return QMAX( itemRect.width(), QApplication::globalStrut().width() );
}

/*!
    Returns the height of the item.
*/

int KileIconViewItem::height() const
{
    return QMAX( itemRect.height(), QApplication::globalStrut().height() );
}

/*!
    Returns the size of the item.
*/

QSize KileIconViewItem::size() const
{
    return QSize( itemRect.width(), itemRect.height() );
}

/*!
    Returns the position of the item (in contents coordinates).
*/

QPoint KileIconViewItem::pos() const
{
    return QPoint( itemRect.x(), itemRect.y() );
}

/*!
    Returns the bounding rectangle of the item's text.

    If \a relative is TRUE, (the default), the returned rectangle is
    relative to the origin of the item's rectangle. If \a relative is
    FALSE, the returned rectangle is relative to the origin of the
    icon view's contents coordinate system.
*/

QRect KileIconViewItem::textRect( bool relative ) const
{
    if ( relative )
	return itemTextRect;
    else
	return QRect( x() + itemTextRect.x(), y() + itemTextRect.y(), itemTextRect.width(), itemTextRect.height() );
}

/*!
    Returns the bounding rectangle of the item's icon.

    If \a relative is TRUE, (the default), the rectangle is relative to
    the origin of the item's rectangle. If \a relative is FALSE, the
    returned rectangle is relative to the origin of the icon view's
    contents coordinate system.
*/

QRect KileIconViewItem::pixmapRect( bool relative ) const
{
    if ( relative )
	return itemIconRect;
    else
	return QRect( x() + itemIconRect.x(), y() + itemIconRect.y(), itemIconRect.width(), itemIconRect.height() );
}

/*!
    Returns TRUE if the item contains the point \a pnt (in contents
    coordinates); otherwise returns FALSE.
*/

bool KileIconViewItem::contains( const QPoint& pnt ) const
{
    return ( textRect( FALSE ).contains( pnt ) ||
	     pixmapRect( FALSE ).contains( pnt ) );
}

/*!
    Returns TRUE if the item intersects the rectangle \a r (in
    contents coordinates); otherwise returns FALSE.
*/

bool KileIconViewItem::intersects( const QRect& r ) const
{
    return ( textRect( FALSE ).intersects( r ) ||
	     pixmapRect( FALSE ).intersects( r ) );
}

/*!
    \fn bool KileIconViewItem::acceptDrop( const QMimeSource *mime ) const

    Returns TRUE if you can drop things with a QMimeSource of \a mime
    onto this item; otherwise returns FALSE.

    The default implementation always returns FALSE. You must subclass
    KileIconViewItem and reimplement acceptDrop() to accept drops.
*/

bool KileIconViewItem::acceptDrop( const QMimeSource * ) const
{
    return FALSE;
}



int KileIconViewItem::compare( KileIconViewItem *i ) const
{
    return key().localeAwareCompare( i->key() );
}



void KileIconViewItem::calcRect( const QString &text_ )
{
    if ( !view ) // #####
	return;

    wordWrapDirty = TRUE;
    int pw = 0;
    int ph = 0;

#ifndef QT_NO_PICTURE
    if ( picture() ) {
	QRect br = picture()->boundingRect();
	pw = br.width() + 2;
	ph = br.height() + 2;
    } else
#endif
    {
	pw = ( pixmap() ? pixmap() : unknown_icon )->width() + 2;
	ph = ( pixmap() ? pixmap() : unknown_icon )->height() + 2;
    }

    itemIconRect.setWidth( pw );
    itemIconRect.setHeight( ph );

    calcTmpText();

    QString t = text_;
    if ( t.isEmpty() ) {
	if ( view->d->wordWrapIconText )
	    t = itemText;
	else
	    t = tmpText;
    }

    int tw = 0;
    int th = 0;
    // ##### TODO: fix font bearings!
    QRect r;
    if ( view->d->wordWrapIconText ) {
	r = QRect( view->d->fm->boundingRect( 0, 0, iconView()->maxItemWidth() -
					      ( iconView()->itemTextPos() == KileIconView::Bottom ? 0 :
						pixmapRect().width() ),
					      0xFFFFFFFF, AlignHCenter | WordBreak | BreakAnywhere, t ) );
	r.setWidth( r.width() + 4 );
    } else {
	r = QRect( 0, 0, view->d->fm->width( t ), view->d->fm->height() );
	r.setWidth( r.width() + 4 );
    }

    if ( r.width() > iconView()->maxItemWidth() -
	 ( iconView()->itemTextPos() == KileIconView::Bottom ? 0 :
	   pixmapRect().width() ) )
	r.setWidth( iconView()->maxItemWidth() - ( iconView()->itemTextPos() == KileIconView::Bottom ? 0 :
						   pixmapRect().width() ) );

    tw = r.width();
    th = r.height();
//    if ( tw < view->d->fm->width( "X" ) )
//	tw = view->d->fm->width( "X" );

    itemTextRect.setWidth( 0 );
    itemTextRect.setHeight( 0 );
//    itemTextRect.setWidth( tw );
//    itemTextRect.setHeight( th );

    int w = 0;
    int h = 0;
    if ( view->itemTextPos() == KileIconView::Bottom ) {
	w = QMAX( itemTextRect.width(), itemIconRect.width() );
	h = itemTextRect.height() + itemIconRect.height() + 1;

	itemRect.setWidth( w );
	itemRect.setHeight( h );

	itemTextRect = QRect( ( width() - itemTextRect.width() ) / 2, height() - itemTextRect.height(),
			      itemTextRect.width(), itemTextRect.height() );
	itemIconRect = QRect( ( width() - itemIconRect.width() ) / 2, 0,
			      itemIconRect.width(), itemIconRect.height() );
    } else {
	h = QMAX( itemTextRect.height(), itemIconRect.height() );
	w = itemTextRect.width() + itemIconRect.width() + 1;

	itemRect.setWidth( w );
	itemRect.setHeight( h );

	itemTextRect = QRect( width() - itemTextRect.width(), ( height() - itemTextRect.height() ) / 2,
			      itemTextRect.width(), itemTextRect.height() );
	itemIconRect = QRect( 0, ( height() - itemIconRect.height() ) / 2,
			      itemIconRect.width(), itemIconRect.height() );
    }
    if ( view )
	view->updateItemContainer( this );
}

/*!
    Paints the item using the painter \a p and the color group \a cg.
    If you want the item to be drawn with a different font or color,
    reimplement this function, change the values of the color group or
    the painter's font, and then call the KileIconViewItem::paintItem()
    with the changed values.
*/

void KileIconViewItem::paintItem( QPainter *p, const QColorGroup &cg )
{
    if ( !view )
	return;

    p->save();

    if ( isSelected() ) {
	p->setPen( cg.highlightedText() );
    } else {
	p->setPen( cg.text() );
    }

    calcTmpText();

#ifndef QT_NO_PICTURE
    if ( picture() ) {
	QPicture *pic = picture();
	if ( isSelected() ) {
	    p->fillRect( pixmapRect( FALSE ), QBrush( cg.highlight(), QBrush::Dense4Pattern) );
	}
	p->drawPicture( x()-pic->boundingRect().x(), y()-pic->boundingRect().y(), *pic );
	if ( isSelected() ) {
	    p->fillRect( textRect( FALSE ), cg.highlight() );
	    p->setPen( QPen( cg.highlightedText() ) );
	} else if ( view->d->itemTextBrush != NoBrush )
	    p->fillRect( textRect( FALSE ), view->d->itemTextBrush );

	int align = view->itemTextPos() == KileIconView::Bottom ? AlignHCenter : AlignAuto;
	if ( view->d->wordWrapIconText )
	    align |= WordBreak | BreakAnywhere;
//	p->drawText( textRect( FALSE ), align, view->d->wordWrapIconText ? itemText : tmpText );
	p->restore();
	return;
    }
#endif
    // ### get rid of code duplication
    if ( view->itemTextPos() == KileIconView::Bottom ) {
	int w = ( pixmap() ? pixmap() : unknown_icon )->width();

	if ( isSelected() ) {
	    QPixmap *pix = pixmap() ? pixmap() : unknown_icon;
	    if ( pix && !pix->isNull() ) {
		QPixmap *buffer = get_qiv_buffer_pixmap( pix->size() );
		QBitmap mask = view->mask( pix );

		QPainter p2( buffer );
		p2.fillRect( pix->rect(), white );
		p2.drawPixmap( 0, 0, *pix );
		p2.end();
		buffer->setMask( mask );
		p2.begin( buffer );
#if defined(Q_WS_X11)
		p2.fillRect( pix->rect(), QBrush( cg.highlight(), QBrush::Dense4Pattern) );
#else // in WIN32 Dense4Pattern doesn't work correctly (transparency problem), so work around it
		if ( iconView()->d->drawActiveSelection ) {
		    if ( !qiv_selection )
			createSelectionPixmap( cg );
		    p2.drawTiledPixmap( 0, 0, pix->width(), pix->height(), *qiv_selection );
		}
#endif
		p2.end();
		QRect cr = pix->rect();
		p->drawPixmap( x() + ( width() - w ) / 2, y(), *buffer, 0, 0, cr.width(), cr.height() );
	    }
	} else {
	    p->drawPixmap( x() + ( width() - w ) / 2, y(), *( pixmap() ? pixmap() : unknown_icon ) );
	}

	p->save();
	if ( isSelected() ) {
	    p->fillRect( textRect( FALSE ), cg.highlight() );
	    p->setPen( QPen( cg.highlightedText() ) );
	} else if ( view->d->itemTextBrush != NoBrush )
	    p->fillRect( textRect( FALSE ), view->d->itemTextBrush );

	int align = AlignHCenter;
	if ( view->d->wordWrapIconText )
	    align |= WordBreak | BreakAnywhere;
//	p->drawText( textRect( FALSE ), align, view->d->wordWrapIconText ? itemText : tmpText );

	p->restore();
    } else {
	int h = ( pixmap() ? pixmap() : unknown_icon )->height();

	if ( isSelected() ) {
	    QPixmap *pix = pixmap() ? pixmap() : unknown_icon;
	    if ( pix && !pix->isNull() ) {
		QPixmap *buffer = get_qiv_buffer_pixmap( pix->size() );
		QBitmap mask = view->mask( pix );

		QPainter p2( buffer );
		p2.fillRect( pix->rect(), white );
		p2.drawPixmap( 0, 0, *pix );
		p2.end();
		buffer->setMask( mask );
		p2.begin( buffer );
#if defined(Q_WS_X11)
		p2.fillRect( pix->rect(), QBrush( cg.highlight(), QBrush::Dense4Pattern) );
#else // in WIN32 Dense4Pattern doesn't work correctly (transparency problem), so work around it
		if ( iconView()->d->drawActiveSelection ) {
		    if ( !qiv_selection )
			createSelectionPixmap( cg );
		    p2.drawTiledPixmap( 0, 0, pix->width(), pix->height(), *qiv_selection );
		}
#endif
		p2.end();
		QRect cr = pix->rect();
		p->drawPixmap( x() , y() + ( height() - h ) / 2, *buffer, 0, 0, cr.width(), cr.height() );
	    }
	} else {
	    p->drawPixmap( x() , y() + ( height() - h ) / 2, *( pixmap() ? pixmap() : unknown_icon ) );
	}

	p->save();
	if ( isSelected() ) {
	    p->fillRect( textRect( FALSE ), cg.highlight() );
	    p->setPen( QPen( cg.highlightedText() ) );
	} else if ( view->d->itemTextBrush != NoBrush )
	    p->fillRect( textRect( FALSE ), view->d->itemTextBrush );

	int align = AlignAuto;
	if ( view->d->wordWrapIconText )
	    align |= WordBreak | BreakAnywhere;
//	p->drawText( textRect( FALSE ), align, view->d->wordWrapIconText ? itemText : tmpText );

	p->restore();
    }

    p->restore();
}

/*!
    Paints the focus rectangle of the item using the painter \a p and
    the color group \a cg.
*/

void KileIconViewItem::paintFocus( QPainter *p, const QColorGroup &cg )
{
    if ( !view )
	return;

    view->style().drawPrimitive(QStyle::PE_FocusRect, p,
				QRect( textRect( FALSE ).x(), textRect( FALSE ).y(),
				       textRect( FALSE ).width(),
				       textRect( FALSE ).height() ), cg,
				(isSelected() ?
				 QStyle::Style_FocusAtBorder :
				 QStyle::Style_Default),
				QStyleOption(isSelected() ? cg.highlight() : cg.base()));

    if ( this != view->d->currentItem ) {
	view->style().drawPrimitive(QStyle::PE_FocusRect, p,
				    QRect( pixmapRect( FALSE ).x(),
					   pixmapRect( FALSE ).y(),
					   pixmapRect( FALSE ).width(),
					   pixmapRect( FALSE ).height() ),
				    cg, QStyle::Style_Default,
				    QStyleOption(cg.base()));
    }
}

/*!
    \fn void KileIconViewItem::dropped( QDropEvent *e, const QValueList<KileIconDragItem> &lst )

    This function is called when something is dropped on the item. \a
    e provides all the information about the drop. If the drag object
    of the drop was a KileIconDrag, \a lst contains the list of the
    dropped items. You can get the data by calling
    KileIconDragItem::data() on each item. If the \a lst is empty, i.e.
    the drag was not a KileIconDrag, you must decode the data in \a e and
    work with that.

    The default implementation does nothing; subclasses may
    reimplement this function.
*/

/*!
    This function is called when a drag enters the item's bounding
    rectangle.

    The default implementation does nothing; subclasses may
    reimplement this function.
*/

void KileIconViewItem::dragEntered()
{
}

/*!
    This function is called when a drag leaves the item's bounding
    rectangle.

    The default implementation does nothing; subclasses may
    reimplement this function.
*/

void KileIconViewItem::dragLeft()
{
}

/*!
    Sets the bounding rectangle of the whole item to \a r. This
    function is provided for subclasses which reimplement calcRect(),
    so that they can set the calculated rectangle. \e{Any other use is
    discouraged.}

    \sa calcRect() textRect() setTextRect() pixmapRect() setPixmapRect()
*/

void KileIconViewItem::setItemRect( const QRect &r )
{
    itemRect = r;
    checkRect();
    if ( view )
	view->updateItemContainer( this );
}

/*!
    Sets the bounding rectangle of the item's text to \a r. This
    function is provided for subclasses which reimplement calcRect(),
    so that they can set the calculated rectangle. \e{Any other use is
    discouraged.}

    \sa calcRect() textRect() setItemRect() setPixmapRect()
*/

void KileIconViewItem::setTextRect( const QRect &r )
{
    itemTextRect = r;
    if ( view )
	view->updateItemContainer( this );
}

/*!
    Sets the bounding rectangle of the item's icon to \a r. This
    function is provided for subclasses which reimplement calcRect(),
    so that they can set the calculated rectangle. \e{Any other use is
    discouraged.}

    \sa calcRect() pixmapRect() setItemRect() setTextRect()
*/

void KileIconViewItem::setPixmapRect( const QRect &r )
{
    itemIconRect = r;
    if ( view )
	view->updateItemContainer( this );
}

/*!
    \internal
*/

void KileIconViewItem::calcTmpText()
{
    if ( !view || view->d->wordWrapIconText || !wordWrapDirty )
	return;
    wordWrapDirty = FALSE;

    int w = iconView()->maxItemWidth() - ( iconView()->itemTextPos() == KileIconView::Bottom ? 0 :
					   pixmapRect().width() );
    if ( view->d->fm->width( itemText ) < w ) {
	tmpText = itemText;
	return;
    }

    tmpText = "...";
    int i = 0;
    while ( view->d->fm->width( tmpText + itemText[ i ] ) < w )
	tmpText += itemText[ i++ ];
    tmpText.remove( 0, 3 );
    tmpText += "...";
}

void KileIconViewItem::checkRect()
{
    int x = itemRect.x();
    int y = itemRect.y();
    int w = itemRect.width();
    int h = itemRect.height();

    bool changed = FALSE;
    if ( x < 0 ) {
	x = 0;
	changed = TRUE;
    }
    if ( y < 0 ) {
	y = 0;
	changed = TRUE;
    }

    if ( changed )
	itemRect.setRect( x, y, w, h );
}


/*! \file iconview/simple_dd/main.h */
/*! \file iconview/simple_dd/main.cpp */


/*!
    \class KileIconView qiconview.h
    \brief The KileIconView class provides an area with movable labelled icons.

    \module iconview
    \ingroup advanced
    \mainclass

    A KileIconView can display and manage a grid or other 2D layout of
    labelled icons. Each labelled icon is a KileIconViewItem. Items
    (KileIconViewItems) can be added or deleted at any time; items can be
    moved within the KileIconView. Single or multiple items can be
    selected. Items can be renamed in-place. KileIconView also supports
    drag and drop.

    Each item contains a label string, a pixmap or picture (the icon
    itself) and optionally a sort key. The sort key is used for
    sorting the items and defaults to the label string. The label
    string can be displayed below or to the right of the icon (see \l
    ItemTextPos).

    The simplest way to create a KileIconView is to create a KileIconView
    object and create some KileIconViewItems with the KileIconView as their
    parent, set the icon view's geometry and show it.
    For example:
    \code
    KileIconView *iv = new KileIconView( this );
    QDir dir( path, "*.xpm" );
    for ( uint i = 0; i < dir.count(); i++ ) {
	(void) new KileIconViewItem( iv, dir[i], QPixmap( path + dir[i] ) );
    }
    iv->resize( 600, 400 );
    iv->show();
    \endcode

    The KileIconViewItem call passes a pointer to the KileIconView we wish to
    populate, along with the label text and a QPixmap.

    When an item is inserted the KileIconView allocates a position for it.
    Existing items are rearranged if autoArrange() is TRUE. The
    default arrangement is \c LeftToRight -- the KileIconView fills up
    the \e left-most column from top to bottom, then moves one column
    \e right and fills that from top to bottom and so on. The
    arrangement can be modified with any of the following approaches:
    \list
    \i Call setArrangement(), e.g. with \c TopToBottom which will fill
    the \e top-most row from left to right, then moves one row \e down
    and fills that row from left to right and so on.
    \i Construct each KileIconViewItem using a constructor which allows
    you to specify which item the new one is to follow.
    \i Call setSorting() or sort() to sort the items.
    \endlist

    The spacing between items is set with setSpacing(). Items can be
    laid out using a fixed grid using setGridX() and setGridY(); by
    default the KileIconView calculates a grid dynamically. The position
    of items' label text is set with setItemTextPos(). The text's
    background can be set with setItemTextBackground(). The maximum
    width of an item and of its text are set with setMaxItemWidth()
    and setMaxItemTextLength(). The label text will be word-wrapped if
    it is too long; this is controlled by setWordWrapIconText(). If
    the label text is truncated, the user can still see the entire
    text in a tool tip if they hover the mouse over the item. This is
    controlled with setShowToolTips().

    Items which are \link KileIconViewItem::isSelectable()
    selectable\endlink may be selected depending on the SelectionMode;
    the default is \c Single. Because KileIconView offers multiple
    selection it must display keyboard focus and selection state
    separately. Therefore there are functions to set the selection
    state of an item (setSelected()) and to select which item displays
    keyboard focus (setCurrentItem()). When multiple items may be
    selected the icon view provides a rubberband, too.

    When in-place renaming is enabled (it is disabled by default), the
    user may change the item's label. They do this by selecting the item
    (single clicking it or navigating to it with the arrow keys), then
    single clicking it (or pressing F2), and entering their text. If no
    key has been set with KileIconViewItem::setKey() the new text will also
    serve as the key. (See KileIconViewItem::setRenameEnabled().)

    You can control whether users can move items themselves with
    setItemsMovable().

    Because the internal structure used to store the icon view items is
    linear, no iterator class is needed to iterate over all the items.
    Instead we iterate by getting the first item from the \e{icon view}
    and then each subsequent (\l KileIconViewItem::nextItem()) from each
    \e item in turn:
    \code
	for ( KileIconViewItem *item = iv->firstItem(); item; item = item->nextItem() )
	    do_something( item );
    \endcode
    KileIconView also provides currentItem(). You can search for an item
    using findItem() (searching by position or for label text) and
    with findFirstVisibleItem() and findLastVisibleItem(). The number
    of items is returned by count(). An item can be removed from an
    icon view using takeItem(); to delete an item use \c delete. All
    the items can be deleted with clear().

    The KileIconView emits a wide range of useful signals, including
    selectionChanged(), currentChanged(), clicked(), moved() and
    itemRenamed().

    \section1 Drag and Drop

    KileIconView supports the drag and drop of items within the KileIconView
    itself. It also supports the drag and drop of items out of or into
    the KileIconView and drag and drop onto items themselves. The drag and
    drop of items outside the KileIconView can be achieved in a simple way
    with basic functionality, or in a more sophisticated way which
    provides more power and control.

    The simple approach to dragging items out of the icon view is to
    subclass KileIconView and reimplement KileIconView::dragObject().

    \code
    QDragObject *MyIconView::dragObject()
    {
	return new QTextDrag( currentItem()->text(), this );
    }
    \endcode

    In this example we create a QTextDrag object, (derived from
    QDragObject), containing the item's label and return it as the drag
    object. We could just as easily have created a QImageDrag from the
    item's pixmap and returned that instead.

    KileIconViews and their KileIconViewItems can also be the targets of drag
    and drops. To make the KileIconView itself able to accept drops connect
    to the dropped() signal. When a drop occurs this signal will be
    emitted with a QDragEvent and a QValueList of KileIconDragItems. To
    make a KileIconViewItem into a drop target subclass KileIconViewItem and
    reimplement KileIconViewItem::acceptDrop() and
    KileIconViewItem::dropped().

    \code
    bool MyIconViewItem::acceptDrop( const QMimeSource *mime ) const
    {
	if ( mime->provides( "text/plain" ) )
	    return TRUE;
	return FALSE;
    }

    void MyIconViewItem::dropped( QDropEvent *evt, const QValueList<KileIconDragItem>& )
    {
	QString label;
	if ( QTextDrag::decode( evt, label ) )
	    setText( label );
    }
    \endcode

    See \l iconview/simple_dd/main.h and \l
    iconview/simple_dd/main.cpp for a simple drag and drop example
    which demonstrates drag and drop between a KileIconView and a
    QListBox.

    If you want to use extended drag-and-drop or have drag shapes drawn
    you must take a more sophisticated approach.

    The first part is starting drags -- you should use a KileIconDrag (or a
    class derived from it) for the drag object. In dragObject() create the
    drag object, populate it with KileIconDragItems and return it. Normally
    such a drag should offer each selected item's data. So in dragObject()
    you should iterate over all the items, and create a KileIconDragItem for
    each selected item, and append these items with KileIconDrag::append() to
    the KileIconDrag object. You can use KileIconDragItem::setData() to set the
    data of each item that should be dragged. If you want to offer the
    data in additional mime-types, it's best to use a class derived from
    KileIconDrag, which implements additional encoding and decoding
    functions.

    When a drag enters the icon view, there is little to do. Simply
    connect to the dropped() signal and reimplement
    KileIconViewItem::acceptDrop() and KileIconViewItem::dropped(). If you've
    used a KileIconDrag (or a subclass of it) the second argument to the
    dropped signal contains a QValueList of KileIconDragItems -- you can
    access their data by calling KileIconDragItem::data() on each one.

    For an example implementation of complex drag-and-drop look at the
    qfileiconview example (qt/examples/qfileiconview).

    \sa KileIconViewItem::setDragEnabled(), KileIconViewItem::setDropEnabled(),
	KileIconViewItem::acceptDrop(), KileIconViewItem::dropped().

    <img src=qiconview-m.png> <img src=qiconview-w.png>
*/

/*! \enum KileIconView::ResizeMode

    This enum type is used to tell KileIconView how it should treat the
    positions of its icons when the widget is resized. The modes are:

    \value Fixed  The icons' positions are not changed.
    \value Adjust  The icons' positions are adjusted to be within the
    new geometry, if possible.
*/

/*!
    \enum KileIconView::SelectionMode

    This enumerated type is used by KileIconView to indicate how it
    reacts to selection by the user. It has four values:

    \value Single  When the user selects an item, any already-selected
    item becomes unselected and the user cannot unselect the selected
    item. This means that the user can never clear the selection. (The
    application programmer can, using KileIconView::clearSelection().)

    \value Multi  When the user selects an item, e.g. by navigating to
    it with the keyboard arrow keys or by clicking it, the selection
    status of that item is toggled and the other items are left alone.

    \value Extended  When the user selects an item the selection is
    cleared and the new item selected. However, if the user presses
    the Ctrl key when clicking on an item, the clicked item gets
    toggled and all other items are left untouched. If the user
    presses the Shift key while clicking on an item, all items between
    the current item and the clicked item get selected or unselected,
    depending on the state of the clicked item. Also, multiple items
    can be selected by dragging the mouse while the left mouse button
    stays pressed.

    \value NoSelection  Items cannot be selected.

    To summarise: \c Single is a real single-selection icon view; \c
    Multi a real multi-selection icon view; \c Extended is an icon
    view in which users can select multiple items but usually want to
    select either just one or a range of contiguous items; and \c
    NoSelection mode is for an icon view where the user can look but
    not touch.
*/

/*!
    \enum KileIconView::Arrangement

    This enum type determines in which direction the items flow when
    the view runs out of space.

    \value LeftToRight  Items which don't fit into the view go further
    down (you get a vertical scrollbar)

    \value TopToBottom  Items which don't fit into the view go further
    right (you get a horizontal scrollbar)
*/

/*!
    \enum KileIconView::ItemTextPos

    This enum type specifies the position of the item text in relation
    to the icon.

    \value Bottom  The text is drawn below the icon.
    \value Right  The text is drawn to the right of the icon.
*/

/*!
    \fn void  KileIconView::dropped ( QDropEvent * e, const QValueList<KileIconDragItem> &lst )

    This signal is emitted when a drop event occurs in the viewport
    (but not on any icon) which the icon view itself can't handle.

    \a e provides all the information about the drop. If the drag
    object of the drop was a KileIconDrag, \a lst contains the list of
    the dropped items. You can get the data using
    KileIconDragItem::data() on each item. If the \a lst is empty, i.e.
    the drag was not a KileIconDrag, you have to decode the data in \a e
    and work with that.

    Note KileIconViewItems may be drop targets; if a drop event occurs on
    an item the item handles the drop.
*/

/*!
    \fn void KileIconView::moved()

    This signal is emitted after successfully dropping one (or more)
    items of the icon view. If the items should be removed, it's best
    to do so in a slot connected to this signal.
*/

/*!
    \fn void  KileIconView::doubleClicked(KileIconViewItem * item)

    This signal is emitted when the user double-clicks on \a item.
*/

/*!
    \fn void  KileIconView::returnPressed (KileIconViewItem * item)

    This signal is emitted if the user presses the Return or Enter
    key. \a item is the currentItem() at the time of the keypress.
*/

/*!
    \fn void  KileIconView::selectionChanged()

    This signal is emitted when the selection has been changed. It's
    emitted in each selection mode.
*/

/*!
    \overload void KileIconView::selectionChanged( KileIconViewItem *item )

    This signal is emitted when the selection changes. \a item is the
    newly selected item. This signal is emitted only in single
    selection mode.
*/

/*!
    \fn void KileIconView::currentChanged( KileIconViewItem *item )

    This signal is emitted when a new item becomes current. \a item is
    the new current item (or 0 if no item is now current).

    \sa currentItem()
*/

/*!
    \fn void  KileIconView::onItem( KileIconViewItem *item )

    This signal is emitted when the user moves the mouse cursor onto
    an \a item, similar to the QWidget::enterEvent() function.
*/

// ### bug here - enter/leave event aren't considered. move the mouse
// out of the window and back in, to the same item.

/*!
    \fn void KileIconView::onViewport()

    This signal is emitted when the user moves the mouse cursor from
    an item to an empty part of the icon view.

    \sa onItem()
*/

/*!
    \overload void KileIconView::itemRenamed (KileIconViewItem * item)

    This signal is emitted when \a item has been renamed, usually by
    in-place renaming.

    \sa KileIconViewItem::setRenameEnabled() KileIconViewItem::rename()
*/

/*!
    \fn void KileIconView::itemRenamed (KileIconViewItem * item, const QString &name)

    This signal is emitted when \a item has been renamed to \a name,
    usually by in-place renaming.

    \sa KileIconViewItem::setRenameEnabled() KileIconViewItem::rename()
*/

/*!
    \fn void KileIconView::rightButtonClicked (KileIconViewItem * item, const QPoint & pos)

    This signal is emitted when the user clicks the right mouse
    button. If \a item is non-null, the cursor is on \a item. If \a
    item is null, the mouse cursor isn't on any item.

    \a pos is the position of the mouse cursor in the global
    coordinate system (QMouseEvent::globalPos()). (If the click's
    press and release differ by a pixel or two, \a pos is the
    position at release time.)

    \sa rightButtonPressed() mouseButtonClicked() clicked()
*/

/*!
    \fn void KileIconView::contextMenuRequested( KileIconViewItem *item, const QPoint & pos )

    This signal is emitted when the user invokes a context menu with
    the right mouse button or with special system keys, with \a item
    being the item under the mouse cursor or the current item,
    respectively.

    \a pos is the position for the context menu in the global
    coordinate system.
*/

/*!
    \fn void KileIconView::mouseButtonPressed (int button, KileIconViewItem * item, const QPoint & pos)

    This signal is emitted when the user presses mouse button \a
    button. If \a item is non-null, the cursor is on \a item. If \a
    item is null, the mouse cursor isn't on any item.

    \a pos is the position of the mouse cursor in the global
    coordinate system (QMouseEvent::globalPos()).

    \sa rightButtonClicked() mouseButtonPressed() pressed()
*/

/*!
    \fn void KileIconView::mouseButtonClicked (int button, KileIconViewItem * item, const QPoint & pos )

    This signal is emitted when the user clicks mouse button \a
    button. If \a item is non-null, the cursor is on \a item. If \a
    item is null, the mouse cursor isn't on any item.

    \a pos is the position of the mouse cursor in the global
    coordinate system (QMouseEvent::globalPos()). (If the click's
    press and release differ by a pixel or two, \a pos is the
    position at release time.)

    \sa mouseButtonPressed() rightButtonClicked() clicked()
*/

/*!
    \overload void KileIconView::clicked ( KileIconViewItem * item, const QPoint & pos )

    This signal is emitted when the user clicks any mouse button on an
    icon view item. \a item is a pointer to the item that has been
    clicked.

    \a pos is the position of the mouse cursor in the global coordinate
    system (QMouseEvent::globalPos()). (If the click's press and release
    differ by a pixel or two, \a pos is the  position at release time.)

    \sa mouseButtonClicked() rightButtonClicked() pressed()
*/

/*!
    \overload void KileIconView::pressed ( KileIconViewItem * item, const QPoint & pos )

    This signal is emitted when the user presses any mouse button. If
    \a item is non-null, the cursor is on \a item. If \a item is null,
    the mouse cursor isn't on any item.

    \a pos is the position of the mouse cursor in the global
    coordinate system (QMouseEvent::globalPos()). (If the click's
    press and release differ by a pixel or two, \a pos is the
    position at release time.)

    \sa mouseButtonPressed() rightButtonPressed() clicked()
*/

/*!
    \fn void KileIconView::clicked ( KileIconViewItem * item )

    This signal is emitted when the user clicks any mouse button. If
    \a item is non-null, the cursor is on \a item. If \a item is null,
    the mouse cursor isn't on any item.

    \sa mouseButtonClicked() rightButtonClicked() pressed()
*/

/*!
    \fn void KileIconView::pressed ( KileIconViewItem * item )

    This signal is emitted when the user presses any mouse button. If
    \a item is non-null, the cursor is on \a item. If \a item is null,
    the mouse cursor isn't on any item.

    \sa mouseButtonPressed() rightButtonPressed() clicked()
*/

/*!
    \fn void KileIconView::rightButtonPressed( KileIconViewItem * item, const QPoint & pos )

    This signal is emitted when the user presses the right mouse
    button. If \a item is non-null, the cursor is on \a item. If \a
    item is null, the mouse cursor isn't on any item.

    \a pos is the position of the mouse cursor in the global
    coordinate system (QMouseEvent::globalPos()).
*/

/*!
    Constructs an empty icon view called \a name, with parent \a
    parent and using the widget flags \a f.
*/

KileIconView::KileIconView( QWidget *parent, const char *name, WFlags f )
    : QScrollView( parent, name, WStaticContents | WRepaintNoErase  | f )
{
    if ( !unknown_icon ) {
	unknown_icon = new QPixmap( (const char **)unknown_xpm );
	qiv_cleanup_pixmap.add( &unknown_icon );
    }

    d = new KileIconViewPrivate;
    d->dragging = FALSE;
    d->firstItem = 0;
    d->lastItem = 0;
    d->count = 0;
    d->mousePressed = FALSE;
    d->selectionMode = Single;
    d->currentItem = 0;
    d->highlightedItem = 0;
    d->rubber = 0;
    d->scrollTimer = 0;
    d->startDragItem = 0;
    d->tmpCurrentItem = 0;
    d->rastX = d->rastY = -1;
    d->spacing = 5;
    d->cleared = FALSE;
    d->arrangement = LeftToRight;
    d->resizeMode = Fixed;
    d->dropped = FALSE;
    d->adjustTimer = new QTimer( this );
    d->isIconDrag = FALSE;
    d->numDragItems = 0;
    d->updateTimer = new QTimer( this );
    d->cachedW = d->cachedH = 0;
    d->maxItemWidth = 100;
    d->maxItemTextLength = 255;
    d->inputTimer = new QTimer( this );
    d->currInputString = QString::null;
    d->dirty = FALSE;
    d->rearrangeEnabled = TRUE;
    d->itemTextPos = Bottom;
    d->reorderItemsWhenInsert = TRUE;
#ifndef QT_NO_CURSOR
    d->oldCursor = arrowCursor;
#endif
    d->resortItemsWhenInsert = FALSE;
    d->sortDirection = TRUE;
    d->wordWrapIconText = TRUE;
    d->cachedContentsX = d->cachedContentsY = -1;
    d->clearing = FALSE;
    d->fullRedrawTimer = new QTimer( this );
    d->itemTextBrush = NoBrush;
    d->drawAllBack = TRUE;
    d->fm = new QFontMetrics( font() );
    d->minLeftBearing = d->fm->minLeftBearing();
    d->minRightBearing = d->fm->minRightBearing();
    d->firstContainer = d->lastContainer = 0;
    d->containerUpdateLocked = FALSE;
    d->firstSizeHint = TRUE;
    d->selectAnchor = 0;
    d->drawActiveSelection = TRUE;
    d->drawDragShapes = FALSE;

    connect( d->adjustTimer, SIGNAL( timeout() ),
	     this, SLOT( adjustItems() ) );
    connect( d->updateTimer, SIGNAL( timeout() ),
	     this, SLOT( slotUpdate() ) );
    connect( d->inputTimer, SIGNAL( timeout() ),
	     this, SLOT( clearInputString() ) );
    connect( d->fullRedrawTimer, SIGNAL( timeout() ),
	     this, SLOT( updateContents() ) );
    connect( this, SIGNAL( contentsMoving( int, int ) ),
	     this, SLOT( movedContents( int, int ) ) );

    setAcceptDrops( TRUE );
    viewport()->setAcceptDrops( TRUE );

    setMouseTracking( TRUE );
    viewport()->setMouseTracking( TRUE );

    setPaletteBackgroundColor(white);
    setBackgroundMode(PaletteBackground,PaletteBase);
    viewport()->setBackgroundMode(PaletteBackground);
//    viewport()->setBackgroundMode( PaletteBase);
//    setBackgroundMode( PaletteBackground, PaletteBase );
    viewport()->setFocusProxy( this );
    viewport()->setFocusPolicy( QWidget::WheelFocus );

    d->showTips = TRUE;
}

/*!
    \reimp
*/

void KileIconView::styleChange( QStyle& old )
{
    QScrollView::styleChange( old );
    *d->fm = QFontMetrics( font() );
    d->minLeftBearing = d->fm->minLeftBearing();
    d->minRightBearing = d->fm->minRightBearing();

    KileIconViewItem *item = d->firstItem;
    for ( ; item; item = item->next ) {
	item->wordWrapDirty = TRUE;
	item->calcRect();
    }

#if !defined(Q_WS_X11)
    delete qiv_selection;
    qiv_selection = 0;
#endif
}

/*!
    \reimp
*/

void KileIconView::setFont( const QFont & f )
{
    QScrollView::setFont( f );
    *d->fm = QFontMetrics( font() );
    d->minLeftBearing = d->fm->minLeftBearing();
    d->minRightBearing = d->fm->minRightBearing();

    KileIconViewItem *item = d->firstItem;
    for ( ; item; item = item->next ) {
	item->wordWrapDirty = TRUE;
	item->calcRect();
    }
}

/*!
    \reimp
*/

void KileIconView::setPalette( const QPalette & p )
{
    QScrollView::setPalette( p );
    *d->fm = QFontMetrics( font() );
    d->minLeftBearing = d->fm->minLeftBearing();
    d->minRightBearing = d->fm->minRightBearing();

    KileIconViewItem *item = d->firstItem;
    for ( ; item; item = item->next ) {
	item->wordWrapDirty = TRUE;
	item->calcRect();
    }
}

/*!
    Destroys the icon view and deletes all items.
*/

KileIconView::~KileIconView()
{
    KileIconViewItem *tmp, *item = d->firstItem;
    d->clearing = TRUE;
    KileIconViewPrivate::ItemContainer *c = d->firstContainer, *tmpc;
    while ( c ) {
	tmpc = c->n;
	delete c;
	c = tmpc;
    }
    while ( item ) {
	tmp = item->next;
	delete item;
	item = tmp;
    }
    delete d->fm;
    d->fm = 0;
    delete d;
}

/*!
    Inserts the icon view item \a item after \a after. If \a after is
    0, \a item is appended after the last item.

    \e{You should never need to call this function.} Instead create
    KileIconViewItem's and associate them with your icon view like this:

    \code
	(void) new KileIconViewItem( myIconview, "The text of the item", aPixmap );
    \endcode
*/

void KileIconView::insertItem( KileIconViewItem *item, KileIconViewItem *after )
{
    if ( !item )
	return;

    if ( !d->firstItem ) {
	d->firstItem = d->lastItem = item;
	item->prev = 0;
	item->next = 0;
    } else {
	if ( !after || after == d->lastItem ) {
	    d->lastItem->next = item;
	    item->prev = d->lastItem;
	    item->next = 0;
	    d->lastItem = item;
	} else {
	    KileIconViewItem *i = d->firstItem;
	    while ( i != after )
		i = i->next;

	    if ( i ) {
		KileIconViewItem *next = i->next;
		item->next = next;
		item->prev = i;
		i->next = item;
		next->prev = item;
	    }
	}
    }

    if ( isVisible() ) {
	if ( d->reorderItemsWhenInsert ) {
	    if ( d->updateTimer->isActive() )
		d->updateTimer->stop();
	    d->fullRedrawTimer->stop();
	    // #### uncomment this ASA insertInGrid uses cached values and is efficient
	    //insertInGrid( item );

	    d->cachedW = QMAX( d->cachedW, item->x() + item->width() );
	    d->cachedH= QMAX( d->cachedH, item->y() + item->height() );

	    d->updateTimer->start( 0, TRUE );
	} else {
	    insertInGrid( item );

	    viewport()->update(item->x() - contentsX(),
			       item->y() - contentsY(),
			       item->width(), item->height());
	}
    } else if ( !autoArrange() ) {
	item->dirty = FALSE;
    }

    d->count++;
    d->dirty = TRUE;
}

/*!
    This slot is used for a slightly-delayed update.

    The icon view is not redrawn immediately after inserting a new item
    but after a very small delay using a QTimer. This means that when
    many items are inserted in a loop the icon view is probably redrawn
    only once at the end of the loop. This makes the insertions both
    flicker-free and faster.
*/

void KileIconView::slotUpdate()
{
    d->updateTimer->stop();
    d->fullRedrawTimer->stop();

    if ( !d->firstItem || !d->lastItem )
	return;

    // #### remove that ASA insertInGrid uses cached values and is efficient
    if ( d->resortItemsWhenInsert )
	sort( d->sortDirection );
    else {
	int y = d->spacing;
	KileIconViewItem *item = d->firstItem;
	int w = 0, h = 0;
	while ( item ) {
	    bool changed;
	    KileIconViewItem *next = makeRowLayout( item, y, changed );
	    if ( !next || !next->next )
		break;

	    if( !QApplication::reverseLayout() )
		item = next;
	    w = QMAX( w, item->x() + item->width() );
	    h = QMAX( h, item->y() + item->height() );
	    item = next;
	    if ( d->arrangement == LeftToRight )
		h = QMAX( h, y );

	    item = item->next;
	}

	if ( d->lastItem && d->arrangement == TopToBottom ) {
	    item = d->lastItem;
	    int x = item->x();
	    while ( item && item->x() >= x ) {
		w = QMAX( w, item->x() + item->width() );
		h = QMAX( h, item->y() + item->height() );
		item = item->prev;
	    }
	}

	w = QMAX( QMAX( d->cachedW, w ), d->lastItem->x() + d->lastItem->width() );
	h = QMAX( QMAX( d->cachedH, h ), d->lastItem->y() + d->lastItem->height() );

	if ( d->arrangement == TopToBottom )
	    w += d->spacing;
	else
	    h += d->spacing;
	viewport()->setUpdatesEnabled( FALSE );
	resizeContents( w, h );
	viewport()->setUpdatesEnabled( TRUE );
	viewport()->repaint( FALSE );
    }

    int cx = d->cachedContentsX == -1 ? contentsX() : d->cachedContentsX;
    int cy = d->cachedContentsY == -1 ? contentsY() : d->cachedContentsY;

    if ( cx != contentsX() || cy != contentsY() )
	setContentsPos( cx, cy );

    d->cachedContentsX = d->cachedContentsY = -1;
    d->cachedW = d->cachedH = 0;
}

/*!
    Takes the icon view item \a item out of the icon view and causes
    an update of the screen display. The item is not deleted. You
    should normally not need to call this function because
    KileIconViewItem::~KileIconViewItem() calls it. The normal way to delete
    an item is to delete it.
*/

void KileIconView::takeItem( KileIconViewItem *item )
{
    if ( !item )
	return;

    if ( item->d->container1 )
	item->d->container1->items.removeRef( item );
    if ( item->d->container2 )
	item->d->container2->items.removeRef( item );
    item->d->container2 = 0;
    item->d->container1 = 0;

    bool block = signalsBlocked();
    blockSignals( d->clearing );

    QRect r = item->rect();

    if ( d->currentItem == item ) {
	if ( item->prev ) {
	    d->currentItem = item->prev;
	    emit currentChanged( d->currentItem );
	    repaintItem( d->currentItem );
	} else if ( item->next ) {
	    d->currentItem = item->next;
	    emit currentChanged( d->currentItem );
	    repaintItem( d->currentItem );
	} else {
	    d->currentItem = 0;
	    emit currentChanged( d->currentItem );
	}
    }
    if ( item->isSelected() ) {
	item->selected = FALSE;
	emit selectionChanged();
    }

    if ( item == d->firstItem ) {
	d->firstItem = d->firstItem->next;
	if ( d->firstItem )
	    d->firstItem->prev = 0;
    } else if ( item == d->lastItem ) {
	d->lastItem = d->lastItem->prev;
	if ( d->lastItem )
	    d->lastItem->next = 0;
    } else {
	KileIconViewItem *i = item;
	if ( i ) {
	    if ( i->prev )
		i->prev->next = i->next;
	    if ( i->next )
		i->next->prev = i->prev;
	}
    }

    if ( d->selectAnchor == item )
	d->selectAnchor = d->currentItem;

    if ( !d->clearing )
	repaintContents( r.x(), r.y(), r.width(), r.height(), TRUE );

    d->count--;

    blockSignals( block );
}

/*!
    Returns the index of \a item, or -1 if \a item doesn't exist in
    this icon view.
*/

int KileIconView::index( const KileIconViewItem *item ) const
{
    if ( !item )
	return -1;

    if ( item == d->firstItem )
	return 0;
    else if ( item == d->lastItem )
	return d->count - 1;
    else {
	KileIconViewItem *i = d->firstItem;
	int j = 0;
	while ( i && i != item ) {
	    i = i->next;
	    ++j;
	}

	return i ? j : -1;
    }
}

/*!
    Returns a pointer to the first item of the icon view, or 0 if
    there are no items in the icon view.

    \sa lastItem() currentItem()
*/

KileIconViewItem *KileIconView::firstItem() const
{
    return d->firstItem;
}

/*!
    Returns a pointer to the last item of the icon view, or 0 if there
    are no items in the icon view.

    \sa firstItem() currentItem()
*/

KileIconViewItem *KileIconView::lastItem() const
{
    return d->lastItem;
}

/*!
    Returns a pointer to the current item of the icon view, or 0 if no
    item is current.

    \sa setCurrentItem() firstItem() lastItem()
*/

KileIconViewItem *KileIconView::currentItem() const
{
    return d->currentItem;
}

/*!
    Makes \a item the new current item of the icon view.
*/

void KileIconView::setCurrentItem( KileIconViewItem *item )
{
    if ( !item || item == d->currentItem )
	return;

    setMicroFocusHint( item->x(), item->y(), item->width(), item->height(), FALSE );

    KileIconViewItem *old = d->currentItem;
    d->currentItem = item;
    emit currentChanged( d->currentItem );
    if ( d->selectionMode == Single ) {
	bool changed = FALSE;
	if ( old && old->selected ) {
	    old->selected = FALSE;
	    changed = TRUE;
	}
	if ( item && !item->selected && item->isSelectable() && d->selectionMode != NoSelection ) {
	    item->selected = TRUE;
	    changed = TRUE;
	    emit selectionChanged( item );
	}
	if ( changed )
	    emit selectionChanged();
    }

    if ( old )
	repaintItem( old );
    repaintItem( d->currentItem );
}

/*!
    Selects or unselects \a item depending on \a s, and may also
    unselect other items, depending on KileIconView::selectionMode() and
    \a cb.

    If \a s is FALSE, \a item is unselected.

    If \a s is TRUE and KileIconView::selectionMode() is \c Single, \a
    item is selected, and the item which was selected is unselected.

    If \a s is TRUE and KileIconView::selectionMode() is \c Extended, \a
    item is selected. If \a cb is TRUE, the selection state of the
    icon view's other items is left unchanged. If \a cb is FALSE (the
    default) all other items are unselected.

    If \a s is TRUE and KileIconView::selectionMode() is \c Multi \a item
    is selected.

    Note that \a cb is used only if KileIconView::selectionMode() is \c
    Extended. \a cb defaults to FALSE.

    All items whose selection status is changed repaint themselves.
*/

void KileIconView::setSelected( KileIconViewItem *item, bool s, bool cb )
{
    if ( !item )
	return;
    item->setSelected( s, cb );
}

/*!
    \property KileIconView::count
    \brief the number of items in the icon view
*/

uint KileIconView::count() const
{
    return d->count;
}

/*!
    Performs autoscrolling when selecting multiple icons with the
    rubber band.
*/

void KileIconView::doAutoScroll()
{
    QRect oldRubber = QRect( *d->rubber );

    QPoint pos = QCursor::pos();
    pos = viewport()->mapFromGlobal( pos );
    pos = viewportToContents( pos );

    d->rubber->setRight( pos.x() );
    d->rubber->setBottom( pos.y() );

    int minx = contentsWidth(), miny = contentsHeight();
    int maxx = 0, maxy = 0;
    bool changed = FALSE;
    bool block = signalsBlocked();

    QRect rr;
    QRegion region( 0, 0, visibleWidth(), visibleHeight() );

    blockSignals( TRUE );
    viewport()->setUpdatesEnabled( FALSE );
    bool alreadyIntersected = FALSE;
    QRect nr = d->rubber->normalize();
    QRect rubberUnion = nr.unite( oldRubber.normalize() );
    KileIconViewPrivate::ItemContainer *c = d->firstContainer;
    for ( ; c; c = c->n ) {
	if ( c->rect.intersects( rubberUnion ) ) {
	    alreadyIntersected = TRUE;
	    KileIconViewItem *item = c->items.first();
	    for ( ; item; item = c->items.next() ) {
		if ( d->selectedItems.find( item ) )
		    continue;
		if ( !item->intersects( nr ) ) {
		    if ( item->isSelected() ) {
			item->setSelected( FALSE );
			changed = TRUE;
			rr = rr.unite( item->rect() );
		    }
		} else if ( item->intersects( nr ) ) {
		    if ( !item->isSelected() && item->isSelectable() ) {
			item->setSelected( TRUE, TRUE );
			changed = TRUE;
			rr = rr.unite( item->rect() );
		    } else {
			region = region.subtract( QRect( contentsToViewport( item->pos() ),
							 item->size() ) );
		    }

		    minx = QMIN( minx, item->x() - 1 );
		    miny = QMIN( miny, item->y() - 1 );
		    maxx = QMAX( maxx, item->x() + item->width() + 1 );
		    maxy = QMAX( maxy, item->y() + item->height() + 1 );
		}
	    }
	} else {
	    if ( alreadyIntersected )
		break;
	}
    }
    viewport()->setUpdatesEnabled( TRUE );
    blockSignals( block );

    QRect r = *d->rubber;
    *d->rubber = oldRubber;

    QPainter p;
    p.begin( viewport() );
    p.setRasterOp( NotROP );
    p.setPen( QPen( color0, 1 ) );
    p.setBrush( NoBrush );
    drawRubber( &p );
    d->dragging = FALSE;
    p.end();

    *d->rubber = r;

    if ( changed ) {
	d->drawAllBack = FALSE;
	d->clipRegion = region;
	repaintContents( rr, FALSE );
	d->drawAllBack = TRUE;
    }

    ensureVisible( pos.x(), pos.y() );

    p.begin( viewport() );
    p.setRasterOp( NotROP );
    p.setPen( QPen( color0, 1 ) );
    p.setBrush( NoBrush );
    drawRubber( &p );
    d->dragging = TRUE;

    p.end();

    if ( changed ) {
	emit selectionChanged();
	if ( d->selectionMode == Single )
	    emit selectionChanged( d->currentItem );
    }

    if ( !QRect( 0, 0, viewport()->width(), viewport()->height() ).contains( pos ) &&
	 !d->scrollTimer ) {
	d->scrollTimer = new QTimer( this );

	connect( d->scrollTimer, SIGNAL( timeout() ),
		 this, SLOT( doAutoScroll() ) );
	d->scrollTimer->start( 100, FALSE );
    } else if ( QRect( 0, 0, viewport()->width(), viewport()->height() ).contains( pos ) &&
		d->scrollTimer ) {
	disconnect( d->scrollTimer, SIGNAL( timeout() ),
		    this, SLOT( doAutoScroll() ) );
	d->scrollTimer->stop();
	delete d->scrollTimer;
	d->scrollTimer = 0;
    }

}

/*!
    \reimp
*/

void KileIconView::drawContents( QPainter *p, int cx, int cy, int cw, int ch )
{
    if ( d->dragging && d->rubber )
	drawRubber( p );

    QRect r = QRect( cx, cy, cw, ch );

    KileIconViewPrivate::ItemContainer *c = d->firstContainer;
    QRegion remaining( QRect( cx, cy, cw, ch ) );
    bool alreadyIntersected = FALSE;
    while ( c ) {
	if ( c->rect.intersects( r ) ) {
	    p->save();
	    p->resetXForm();
	    QRect r2 = c->rect;
	    r2 = r2.intersect( r );
	    QRect r3( contentsToViewport( QPoint( r2.x(), r2.y() ) ), QSize( r2.width(), r2.height() ) );
	    if ( d->drawAllBack ) {
		p->setClipRect( r3 );
	    } else {
		QRegion reg = d->clipRegion.intersect( r3 );
		p->setClipRegion( reg );
	    }
	    drawBackground( p, r3 );
	    remaining = remaining.subtract( r3 );
	    p->restore();

	    QColorGroup cg;
	    d->drawActiveSelection = hasFocus() || !style().styleHint( QStyle::SH_ItemView_ChangeHighlightOnFocus, this ) ||
		( qApp->focusWidget() && qApp->focusWidget()->isPopup() );
	    if ( !d->drawActiveSelection )
		cg = palette().inactive();
	    else
		cg = colorGroup();

	    KileIconViewItem *item = c->items.first();
	    for ( ; item; item = c->items.next() ) {
		if ( item->rect().intersects( r ) && !item->dirty ) {
		    p->save();
		    p->setFont( font() );
		    item->paintItem( p, cg );
		    p->restore();
		}
	    }
	    alreadyIntersected = TRUE;
	} else {
	    if ( alreadyIntersected )
		break;
	}
	c = c->n;
    }

    if ( !remaining.isNull() && !remaining.isEmpty() ) {
	p->save();
	p->resetXForm();
	if ( d->drawAllBack ) {
	    p->setClipRegion( remaining );
	} else {
	    remaining = d->clipRegion.intersect( remaining );
	    p->setClipRegion( remaining );
	}
	drawBackground( p, remaining.boundingRect() );
	p->restore();
    }

    if ( ( hasFocus() || viewport()->hasFocus() ) && d->currentItem &&
	 d->currentItem->rect().intersects( r ) ) {
	d->currentItem->paintFocus( p, colorGroup() );
    }

    if ( d->dragging && d->rubber )
	drawRubber( p );
}

/*!
    \overload

    Arranges all the items in the grid given by gridX() and gridY().

    Even if sorting() is enabled, the items are not sorted by this
    function. If you want to sort or rearrange the items, use
    iconview->sort(iconview->sortDirection()).

    If \a update is TRUE (the default), the viewport is repainted as
    well.

    \sa KileIconView::setGridX(), KileIconView::setGridY(), KileIconView::sort()
*/

void KileIconView::arrangeItemsInGrid( bool update )
{
    if ( !d->firstItem || !d->lastItem )
	return;

    d->containerUpdateLocked = TRUE;

    int w = 0, h = 0, y = d->spacing;

    KileIconViewItem *item = d->firstItem;
    bool changedLayout = FALSE;
    while ( item ) {
	bool changed;
	KileIconViewItem *next = makeRowLayout( item, y, changed );
	changedLayout = changed || changedLayout;
	if( !QApplication::reverseLayout() )
	    item = next;
	w = QMAX( w, item->x() + item->width() );
	h = QMAX( h, item->y() + item->height() );
	item = next;
	if ( d->arrangement == LeftToRight )
	    h = QMAX( h, y );

	if ( !item || !item->next )
	    break;

	item = item->next;
    }

    if ( d->lastItem && d->arrangement == TopToBottom ) {
	item = d->lastItem;
	int x = item->x();
	while ( item && item->x() >= x ) {
	    w = QMAX( w, item->x() + item->width() );
	    h = QMAX( h, item->y() + item->height() );
	    item = item->prev;
	}
    }
    d->containerUpdateLocked = FALSE;

    w = QMAX( QMAX( d->cachedW, w ), d->lastItem->x() + d->lastItem->width() );
    h = QMAX( QMAX( d->cachedH, h ), d->lastItem->y() + d->lastItem->height() );

    if ( d->arrangement == TopToBottom )
	w += d->spacing;
    else
	h += d->spacing;

    viewport()->setUpdatesEnabled( FALSE );
    int vw = visibleWidth();
    int vh = visibleHeight();
    resizeContents( w, h );
    bool doAgain = FALSE;
    if ( d->arrangement == LeftToRight )
	doAgain = visibleWidth() != vw;
    if ( d->arrangement == TopToBottom )
	doAgain = visibleHeight() != vh;
    if ( doAgain ) // in the case that the visibleExtend changed because of the resizeContents (scrollbar show/hide), redo layout again
	arrangeItemsInGrid( FALSE );
    viewport()->setUpdatesEnabled( TRUE );
    d->dirty = !isVisible();
    rebuildContainers();
    if ( update && ( !optimize_layout || changedLayout ) )
	repaintContents( contentsX(), contentsY(), viewport()->width(), viewport()->height(), FALSE );
}

/*!
    This variant uses \a grid instead of (gridX(), gridY()). If \a
    grid is invalid (see QSize::isValid()), arrangeItemsInGrid()
    calculates a valid grid itself and uses that.

    If \a update is TRUE (the default) the viewport is repainted.
*/

void KileIconView::arrangeItemsInGrid( const QSize &grid, bool update )
{
    d->containerUpdateLocked = TRUE;
    QSize grid_( grid );
    if ( !grid_.isValid() ) {
	int w = 0, h = 0;
	KileIconViewItem *item = d->firstItem;
	for ( ; item; item = item->next ) {
	    w = QMAX( w, item->width() );
	    h = QMAX( h, item->height() );
	}

	grid_ = QSize( QMAX( d->rastX + d->spacing, w ),
		       QMAX( d->rastY + d->spacing, h ) );
    }

    int w = 0, h = 0;
    KileIconViewItem *item = d->firstItem;
    for ( ; item; item = item->next ) {
	int nx = item->x() / grid_.width();
	int ny = item->y() / grid_.height();
	item->move( nx * grid_.width(),
		    ny * grid_.height() );
	w = QMAX( w, item->x() + item->width() );
	h = QMAX( h, item->y() + item->height() );
	item->dirty = FALSE;
    }
    d->containerUpdateLocked = FALSE;

    resizeContents( w, h );
    rebuildContainers();
    if ( update )
	repaintContents( contentsX(), contentsY(), viewport()->width(), viewport()->height(), FALSE );
}

/*!
    \reimp
*/

void KileIconView::setContentsPos( int x, int y )
{
    if ( d->updateTimer->isActive() ) {
	d->cachedContentsX = x;
	d->cachedContentsY = y;
    } else {
	d->cachedContentsY = d->cachedContentsX = -1;
	QScrollView::setContentsPos( x, y );
    }
}

/*!
    \reimp
*/

void KileIconView::showEvent( QShowEvent * )
{
    if ( d->dirty ) {
	resizeContents( QMAX( contentsWidth(), viewport()->width() ),
			QMAX( contentsHeight(), viewport()->height() ) );
	if ( d->resortItemsWhenInsert )
	    sort( d->sortDirection );
	if ( autoArrange() )
	    arrangeItemsInGrid( FALSE );
    }
    QScrollView::show();
}

/*!
    \property KileIconView::selectionMode
    \brief the selection mode of the icon view

    This can be \c Single (the default), \c Extended, \c Multi or \c
    NoSelection.
*/

void KileIconView::setSelectionMode( SelectionMode m )
{
    d->selectionMode = m;
}

KileIconView::SelectionMode KileIconView::selectionMode() const
{
    return d->selectionMode;
}

/*!
    Returns a pointer to the item that contains point \a pos, which is
    given in contents coordinates, or 0 if no item contains point \a
    pos.
*/

KileIconViewItem *KileIconView::findItem( const QPoint &pos ) const
{
    if ( !d->firstItem )
	return 0;

    KileIconViewPrivate::ItemContainer *c = d->lastContainer;
    for ( ; c; c = c->p ) {
	if ( c->rect.contains( pos ) ) {
	    KileIconViewItem *item = c->items.last();
	    for ( ; item; item = c->items.prev() )
		if ( item->contains( pos ) )
		    return item;
	}
    }

    return 0;
}

/*!
    \overload

    Returns a pointer to the first item whose text begins with \a
    text, or 0 if no such item could be found. Use the \a compare flag
    to control the comparison behaviour. (See \l
    {Qt::StringComparisonMode}.)
*/

KileIconViewItem *KileIconView::findItem( const QString &text, ComparisonFlags compare ) const
{
    if ( !d->firstItem )
	return 0;

    if ( compare == CaseSensitive || compare == 0 )
	compare |= ExactMatch;

    QString itmtxt;
    QString comtxt = text;
    if ( ! (compare & CaseSensitive) )
	comtxt = text.lower();

    KileIconViewItem *item;
    if ( d->currentItem )
	item = d->currentItem;
    else
	item = d->firstItem;

    if ( item ) {
	for ( ; item; item = item->next ) {
	    if ( ! (compare & CaseSensitive) )
		itmtxt = item->text().lower();
	    else
		itmtxt = item->text();

	    if ( compare & ExactMatch ) {
		if ( itmtxt == comtxt )
		    return item;
	    }

	    if ( compare & BeginsWith ) {
		if ( itmtxt.startsWith( comtxt ) )
		    return item;
	    }

	    if ( compare & EndsWith ) {
		if ( itmtxt.endsWith( comtxt ) )
		    return item;
	    }

	    if ( compare & Contains ) {
		if ( itmtxt.contains( comtxt, (compare & CaseSensitive) ) )
		    return item;
	    }
	}

	if ( d->currentItem && d->firstItem ) {
	    item = d->firstItem;
	    for ( ; item && item != d->currentItem; item = item->next ) {
		if ( ! (compare & CaseSensitive) )
		    itmtxt = item->text().lower();
		else
		    itmtxt = item->text();

		if ( compare & ExactMatch ) {
		    if ( itmtxt == comtxt )
			return item;
		}

		if ( compare & BeginsWith ) {
		    if ( itmtxt.startsWith( comtxt ) )
			return item;
		}

		if ( compare & EndsWith ) {
		    if ( itmtxt.endsWith( comtxt ) )
			return item;
		}

		if ( compare & Contains ) {
		    if ( itmtxt.contains( comtxt, (compare & CaseSensitive) ) )
			return item;
		}
	    }
	}
    }
    return 0;
}

/*!
    Unselects all the items.
*/

void KileIconView::clearSelection()
{
    selectAll( FALSE );
}

/*!
    In Multi and Extended modes, this function sets all items to be
    selected if \a select is TRUE, and to be unselected if \a select
    is FALSE.

    In Single and NoSelection modes, this function only changes the
    selection status of currentItem().
*/

void KileIconView::selectAll( bool select )
{
    if ( d->selectionMode == NoSelection )
	return;

    if ( d->selectionMode == Single ) {
	if ( d->currentItem )
	    d->currentItem->setSelected( select );
	return;
    }

    bool b = signalsBlocked();
    blockSignals( TRUE );
    KileIconViewItem *item = d->firstItem;
    KileIconViewItem *i = d->currentItem;
    bool changed = FALSE;
    bool ue = viewport()->isUpdatesEnabled();
    viewport()->setUpdatesEnabled( FALSE );
    QRect rr;
    for ( ; item; item = item->next ) {
	if ( select != item->isSelected() ) {
	    item->setSelected( select, TRUE );
	    rr = rr.unite( item->rect() );
	    changed = TRUE;
	}
    }
    viewport()->setUpdatesEnabled( ue );
    repaintContents( rr, FALSE );
    if ( i )
	setCurrentItem( i );
    blockSignals( b );
    if ( changed ) {
	emit selectionChanged();
    }
}

/*!
    Inverts the selection. Works only in Multi and Extended selection
    mode.
*/

void KileIconView::invertSelection()
{
    if ( d->selectionMode == Single ||
	 d->selectionMode == NoSelection )
	return;

    bool b = signalsBlocked();
    blockSignals( TRUE );
    KileIconViewItem *item = d->firstItem;
    for ( ; item; item = item->next )
	item->setSelected( !item->isSelected(), TRUE );
    blockSignals( b );
    emit selectionChanged();
}

/*!
    Repaints the \a item.
*/

void KileIconView::repaintItem( KileIconViewItem *item )
{
    if ( !item || item->dirty )
	return;

    if ( QRect( contentsX(), contentsY(), visibleWidth(), visibleHeight() ).
	 intersects( QRect( item->x() - 1, item->y() - 1, item->width() + 2, item->height() + 2 ) ) )
	repaintContents( item->x() - 1, item->y() - 1, item->width() + 2, item->height() + 2, FALSE );
}

/*!
    Makes sure that \a item is entirely visible. If necessary,
    ensureItemVisible() scrolls the icon view.

    \sa ensureVisible()
*/

void KileIconView::ensureItemVisible( KileIconViewItem *item )
{
    if ( !item )
	return;

    if ( d->updateTimer && d->updateTimer->isActive() ||
	 d->fullRedrawTimer && d->fullRedrawTimer->isActive() )
	slotUpdate();

    int w = item->width();
    int h = item->height();
    ensureVisible( item->x() + w / 2, item->y() + h / 2,
		   w / 2 + 1, h / 2 + 1 );
}

/*!
    Finds the first item whose bounding rectangle overlaps \a r and
    returns a pointer to that item. \a r is given in content
    coordinates. Returns 0 if no item overlaps \a r.

    If you want to find all items that touch \a r, you will need to
    use this function and nextItem() in a loop ending at
    findLastVisibleItem() and test QItem::rect() for each of these
    items.

    \sa findLastVisibleItem() KileIconViewItem::rect()
*/

KileIconViewItem* KileIconView::findFirstVisibleItem( const QRect &r ) const
{
    KileIconViewPrivate::ItemContainer *c = d->firstContainer;
    KileIconViewItem *i = 0;
    bool alreadyIntersected = FALSE;
    for ( ; c; c = c->n ) {
	if ( c->rect.intersects( r ) ) {
	    alreadyIntersected = TRUE;
	    KileIconViewItem *item = c->items.first();
	    for ( ; item; item = c->items.next() ) {
		if ( r.intersects( item->rect() ) ) {
		    if ( !i ) {
			i = item;
		    } else {
			QRect r2 = item->rect();
			QRect r3 = i->rect();
			if ( r2.y() < r3.y() )
			    i = item;
			else if ( r2.y() == r3.y() &&
				  r2.x() < r3.x() )
			    i = item;
		    }
		}
	    }
	} else {
	    if ( alreadyIntersected )
		break;
	}
    }

    return i;
}

/*!
    Finds the last item whose bounding rectangle overlaps \a r and
    returns a pointer to that item. \a r is given in content
    coordinates. Returns 0 if no item overlaps \a r.

    \sa findFirstVisibleItem()
*/

KileIconViewItem* KileIconView::findLastVisibleItem( const QRect &r ) const
{
    KileIconViewPrivate::ItemContainer *c = d->firstContainer;
    KileIconViewItem *i = 0;
    bool alreadyIntersected = FALSE;
    for ( ; c; c = c->n ) {
	if ( c->rect.intersects( r ) ) {
	    alreadyIntersected = TRUE;
	    KileIconViewItem *item = c->items.first();
	    for ( ; item; item = c->items.next() ) {
		if ( r.intersects( item->rect() ) ) {
		    if ( !i ) {
			i = item;
		    } else {
			QRect r2 = item->rect();
			QRect r3 = i->rect();
			if ( r2.y() > r3.y() )
			    i = item;
			else if ( r2.y() == r3.y() &&
				  r2.x() > r3.x() )
			    i = item;
		    }
		}
	    }
	} else {
	    if ( alreadyIntersected )
		break;
	}
    }

    return i;
}

/*!
    Clears the icon view. All items are deleted.
*/

void KileIconView::clear()
{
    setContentsPos( 0, 0 );
    d->clearing = TRUE;
    bool block = signalsBlocked();
    blockSignals( TRUE );
    clearSelection();
    blockSignals( block );
    setContentsPos( 0, 0 );
    d->currentItem = 0;

    if ( !d->firstItem ) {
	d->clearing = FALSE;
	return;
    }

    KileIconViewItem *item = d->firstItem, *tmp;
    d->firstItem = 0;
    while ( item ) {
	tmp = item->next;
	delete item;
	item = tmp;
    }
    KileIconViewPrivate::ItemContainer *c = d->firstContainer, *tmpc;
    while ( c ) {
	tmpc = c->n;
	delete c;
	c = tmpc;
    }
    d->firstContainer = d->lastContainer = 0;

    d->count = 0;
    d->lastItem = 0;
    setCurrentItem( 0 );
    d->highlightedItem = 0;
    d->tmpCurrentItem = 0;
    d->drawDragShapes = FALSE;

    resizeContents( 0, 0 );
    // maybe we don´t need this update, so delay it
    d->fullRedrawTimer->start( 0, TRUE );

    d->cleared = TRUE;
    d->clearing = FALSE;
}

/*!
    \property KileIconView::gridX
    \brief the horizontal grid  of the icon view

    If the value is -1, (the default), KileIconView computes suitable
    column widths based on the icon view's contents.

    Note that setting a grid width overrides setMaxItemWidth().
*/

void KileIconView::setGridX( int rx )
{
    d->rastX = rx >= 0 ? rx : -1;
}

/*!
    \property KileIconView::gridY
    \brief the vertical grid  of the icon view

    If the value is -1, (the default), KileIconView computes suitable
    column heights based on the icon view's contents.
*/

void KileIconView::setGridY( int ry )
{
    d->rastY = ry >= 0 ? ry : -1;
}

int KileIconView::gridX() const
{
    return d->rastX;
}

int KileIconView::gridY() const
{
    return d->rastY;
}

/*!
    \property KileIconView::spacing
    \brief the space in pixels between icon view items

    The default is 5 pixels.

    Negative values for spacing are illegal.
*/

void KileIconView::setSpacing( int sp )
{
    d->spacing = sp;
}

int KileIconView::spacing() const
{
    return d->spacing;
}

/*!
    \property KileIconView::itemTextPos
    \brief the position where the text of each item is drawn.

    Valid values are \c Bottom or \c Right. The default is \c Bottom.
*/

void KileIconView::setItemTextPos( ItemTextPos pos )
{
    if ( pos == d->itemTextPos || ( pos != Bottom && pos != Right ) )
	return;

    d->itemTextPos = pos;

    KileIconViewItem *item = d->firstItem;
    for ( ; item; item = item->next ) {
	item->wordWrapDirty = TRUE;
	item->calcRect();
    }

    arrangeItemsInGrid( TRUE );
}

KileIconView::ItemTextPos KileIconView::itemTextPos() const
{
    return d->itemTextPos;
}

/*!
    \property KileIconView::itemTextBackground
    \brief the brush to use when drawing the background of an item's text.

    By default this brush is set to NoBrush, meaning that only the
    normal icon view background is used.
*/

void KileIconView::setItemTextBackground( const QBrush &brush )
{
    d->itemTextBrush = brush;
}

QBrush KileIconView::itemTextBackground() const
{
    return d->itemTextBrush;
}

/*!
    \property KileIconView::arrangement
    \brief the arrangement mode of the icon view

    This can be \c LeftToRight or \c TopToBottom. The default is \c
    LeftToRight.
*/

void KileIconView::setArrangement( Arrangement am )
{
    if ( d->arrangement == am )
	return;

    d->arrangement = am;

    viewport()->setUpdatesEnabled( FALSE );
    resizeContents( viewport()->width(), viewport()->height() );
    viewport()->setUpdatesEnabled( TRUE );
    arrangeItemsInGrid( TRUE );
}

KileIconView::Arrangement KileIconView::arrangement() const
{
    return d->arrangement;
}

/*!
    \property KileIconView::resizeMode
    \brief the resize mode of the icon view

    This can be \c Fixed or \c Adjust. The default is \c Fixed.
    See \l ResizeMode.
*/

void KileIconView::setResizeMode( ResizeMode rm )
{
    if ( d->resizeMode == rm )
	return;

    d->resizeMode = rm;
}

KileIconView::ResizeMode KileIconView::resizeMode() const
{
    return d->resizeMode;
}

/*!
    \property KileIconView::maxItemWidth
    \brief the maximum width that an item may have.

    The default is 100 pixels.

    Note that if the gridX() value is set KileIconView will ignore
    this property.
*/

void KileIconView::setMaxItemWidth( int w )
{
    d->maxItemWidth = w;
}

/*!
    \property KileIconView::maxItemTextLength
    \brief the maximum length (in characters) that an item's text may have.

    The default is 255 characters.
*/

void KileIconView::setMaxItemTextLength( int w )
{
    d->maxItemTextLength = w;
}

int KileIconView::maxItemWidth() const
{
    if ( d->rastX != -1 )
	return d->rastX - 2;
    else
	return d->maxItemWidth;
}

int KileIconView::maxItemTextLength() const
{
    return d->maxItemTextLength;
}

/*!
    \property KileIconView::itemsMovable
    \brief whether the user is allowed to move items around in the icon view

    The default is TRUE.
*/

void KileIconView::setItemsMovable( bool b )
{
    d->rearrangeEnabled = b;
}

bool KileIconView::itemsMovable() const
{
    return d->rearrangeEnabled;
}

/*!
    \property KileIconView::autoArrange
    \brief whether the icon view rearranges its items when a new item is inserted.

    The default is TRUE.

    Note that if the icon view is not visible at the time of
    insertion, KileIconView defers all position-related work until it is
    shown and then calls arrangeItemsInGrid().
*/

void KileIconView::setAutoArrange( bool b )
{
    d->reorderItemsWhenInsert = b;
}

bool KileIconView::autoArrange() const
{
    return d->reorderItemsWhenInsert;
}

/*!
    If \a sort is TRUE, this function sets the icon view to sort items
    when a new item is inserted. If \a sort is FALSE, the icon view
    will not be sorted.

    Note that autoArrange() must be TRUE for sorting to take place.

    If \a ascending is TRUE (the default), items are sorted in
    ascending order. If \a ascending is FALSE, items are sorted in
    descending order.

    KileIconViewItem::compare() is used to compare pairs of items. The
    sorting is based on the items' keys; these default to the items'
    text unless specifically set to something else.

    \sa KileIconView::setAutoArrange(), KileIconView::autoArrange(),
    sortDirection(), sort(), KileIconViewItem::setKey()
*/

void KileIconView::setSorting( bool sort, bool ascending )
{
    d->resortItemsWhenInsert = sort;
    d->sortDirection = ascending;
}

/*!
    \property KileIconView::sorting
    \brief whether the icon view sorts on insertion

    The default is FALSE, i.e. no sorting on insertion.

    To set the soring, use setSorting().
*/

bool KileIconView::sorting() const
{
    return d->resortItemsWhenInsert;
}

/*!
    \property KileIconView::sortDirection
    \brief whether the sort direction for inserting new items is ascending;

    The default is TRUE (i.e. ascending). This sort direction is only
    meaningful if both sorting() and autoArrange() are TRUE.

    To set the sort direction, use setSorting()
*/

bool KileIconView::sortDirection() const
{
    return d->sortDirection;
}

/*!
    \property KileIconView::wordWrapIconText
    \brief whether the item text will be word-wrapped if it is too long

    The default is TRUE.

    If this property is FALSE, icon text that is too long is
    truncated, and an ellipsis (...) appended to indicate that
    truncation has occurred. The full text can still be seen by the
    user if they hover the mouse because the full text is shown in a
    tooltip; see setShowToolTips().
*/

void KileIconView::setWordWrapIconText( bool b )
{
    if ( d->wordWrapIconText == (uint)b )
	return;

    d->wordWrapIconText = b;
    for ( KileIconViewItem *item = d->firstItem; item; item = item->next ) {
	item->wordWrapDirty = TRUE;
	item->calcRect();
    }
    arrangeItemsInGrid( TRUE );
}

bool KileIconView::wordWrapIconText() const
{
    return d->wordWrapIconText;
}

/*!
    \property KileIconView::showToolTips
    \brief whether the icon view will display a tool tip with the complete text for any truncated item text

    The default is TRUE. Note that this has no effect if
    setWordWrapIconText() is TRUE, as it is by default.
*/

void KileIconView::setShowToolTips( bool b )
{
    d->showTips = b;
}

bool KileIconView::showToolTips() const
{
    return d->showTips;
}

/*!
    \reimp
*/
void KileIconView::contentsMousePressEvent( QMouseEvent *e )
{
    contentsMousePressEventEx( e );
}

void KileIconView::contentsMousePressEventEx( QMouseEvent *e )
{
    if ( d->rubber ) {
	QPainter p;
	p.begin( viewport() );
	p.setRasterOp( NotROP );
	p.setPen( QPen( color0, 1 ) );
	p.setBrush( NoBrush );

	drawRubber( &p );
	d->dragging = FALSE;
	p.end();
	delete d->rubber;
	d->rubber = 0;
    }

    d->dragStartPos = e->pos();
    KileIconViewItem *item = findItem( e->pos() );
    d->pressedItem = item;

    if ( item )
	d->selectAnchor = item;


    if ( !d->currentItem && !item && d->firstItem ) {
	d->currentItem = d->firstItem;
	repaintItem( d->firstItem );
    }

    if (item && item->dragEnabled())
	d->startDragItem = item;
    else
	d->startDragItem = 0;

    if ( e->button() == LeftButton && !( e->state() & ShiftButton ) &&
	 !( e->state() & ControlButton ) && item && item->isSelected() &&
	 item->textRect( FALSE ).contains( e->pos() ) ) {

	if ( !item->renameEnabled() ) {
	    d->mousePressed = TRUE;
	}
    }

    d->pressedSelected = item && item->isSelected();

    if ( item && item->isSelectable() ) {
	if ( d->selectionMode == Single )
	    item->setSelected( TRUE, e->state() & ControlButton );
	else if ( d->selectionMode == Multi )
	    item->setSelected( !item->isSelected(), e->state() & ControlButton );
	else if ( d->selectionMode == Extended ) {
	    if ( e->state() & ShiftButton ) {
		d->pressedSelected = FALSE;
		bool block = signalsBlocked();
		blockSignals( TRUE );
		viewport()->setUpdatesEnabled( FALSE );
		QRect r;
		bool select = TRUE;
		if ( d->currentItem )
		    r = QRect( QMIN( d->currentItem->x(), item->x() ),
			       QMIN( d->currentItem->y(), item->y() ),
			       0, 0 );
		else
		    r = QRect( 0, 0, 0, 0 );
		if ( d->currentItem ) {
		    if ( d->currentItem->x() < item->x() )
			r.setWidth( item->x() - d->currentItem->x() + item->width() );
		    else
			r.setWidth( d->currentItem->x() - item->x() + d->currentItem->width() );
		    if ( d->currentItem->y() < item->y() )
			r.setHeight( item->y() - d->currentItem->y() + item->height() );
		    else
			r.setHeight( d->currentItem->y() - item->y() + d->currentItem->height() );
		    r = r.normalize();
		    KileIconViewPrivate::ItemContainer *c = d->firstContainer;
		    bool alreadyIntersected = FALSE;
		    QRect redraw;
		    for ( ; c; c = c->n ) {
			if ( c->rect.intersects( r ) ) {
			    alreadyIntersected = TRUE;
			    KileIconViewItem *i = c->items.first();
			    for ( ; i; i = c->items.next() ) {
				if ( r.intersects( i->rect() ) ) {
				    redraw = redraw.unite( i->rect() );
				    i->setSelected( select, TRUE );
				}
			    }
			} else {
			    if ( alreadyIntersected )
				break;
			}
		    }
		    redraw = redraw.unite( item->rect() );
		    viewport()->setUpdatesEnabled( TRUE );
		    repaintContents( redraw, FALSE );
		}
		blockSignals( block );
		viewport()->setUpdatesEnabled( TRUE );
		item->setSelected( select, TRUE );
		emit selectionChanged();
	    } else if ( e->state() & ControlButton ) {
		d->pressedSelected = FALSE;
		item->setSelected( !item->isSelected(), e->state() & ControlButton );
	    } else {
		item->setSelected( TRUE, e->state() & ControlButton );
	    }
	}
    } else if ( ( d->selectionMode != Single || e->button() == RightButton )
		&& !( e->state() & ControlButton ) )
	selectAll( FALSE );

    setCurrentItem( item );

    if ( e->button() == LeftButton ) {
	if ( !item && ( d->selectionMode == Multi ||
				  d->selectionMode == Extended ) ) {
	    d->tmpCurrentItem = d->currentItem;
	    d->currentItem = 0;
	    repaintItem( d->tmpCurrentItem );
	    if ( d->rubber )
		delete d->rubber;
	    d->rubber = 0;
	    d->rubber = new QRect( e->x(), e->y(), 0, 0 );
	    d->selectedItems.clear();
	    if ( ( e->state() & ControlButton ) == ControlButton ) {
		for ( KileIconViewItem *i = firstItem(); i; i = i->nextItem() )
		    if ( i->isSelected() )
			d->selectedItems.insert( i, i );
	    }
	}

	d->mousePressed = TRUE;
    }

 emit_signals:
    if ( !d->rubber ) {
	emit mouseButtonPressed( e->button(), item, e->globalPos() );
	emit pressed( item );
	emit pressed( item, e->globalPos() );

	if ( e->button() == RightButton )
	    emit rightButtonPressed( item, e->globalPos() );
    }
}

/*!
    \reimp
*/

void KileIconView::contentsContextMenuEvent( QContextMenuEvent *e )
{
    if ( receivers( SIGNAL(contextMenuRequested(KileIconViewItem* item, const QPoint &pos)) ) )
	e->accept();
    if ( e->reason() == QContextMenuEvent::Keyboard ) {
	KileIconViewItem *item = currentItem();
	QRect r = item ? item->rect() : QRect( 0, 0, visibleWidth(), visibleHeight() );
	emit contextMenuRequested( item, mapToGlobal( contentsToViewport( r.center() ) ) );
    } else {
	KileIconViewItem *item = findItem( e->pos() );
	emit contextMenuRequested( item, e->globalPos() );
    }
}

/*!
    \reimp
*/

void KileIconView::contentsMouseReleaseEvent( QMouseEvent *e )
{
    KileIconViewItem *item = findItem( e->pos() );
    d->selectedItems.clear();

    bool emitClicked = TRUE;
    d->mousePressed = FALSE;
    d->startDragItem = 0;

    if ( d->rubber ) {
	QPainter p;
	p.begin( viewport() );
	p.setRasterOp( NotROP );
	p.setPen( QPen( color0, 1 ) );
	p.setBrush( NoBrush );

	drawRubber( &p );
	d->dragging = FALSE;
	p.end();

	if ( ( d->rubber->topLeft() - d->rubber->bottomRight() ).manhattanLength() >
	     QApplication::startDragDistance() )
	    emitClicked = FALSE;
	delete d->rubber;
	d->rubber = 0;
	d->currentItem = d->tmpCurrentItem;
	d->tmpCurrentItem = 0;
	if ( d->currentItem )
	    repaintItem( d->currentItem );
    }

    if ( d->scrollTimer ) {
	disconnect( d->scrollTimer, SIGNAL( timeout() ), this, SLOT( doAutoScroll() ) );
	d->scrollTimer->stop();
	delete d->scrollTimer;
	d->scrollTimer = 0;
    }

    if ( d->selectionMode == Extended &&
	 d->currentItem == d->pressedItem &&
	 d->pressedSelected && d->currentItem ) {
	bool block = signalsBlocked();
	blockSignals( TRUE );
	clearSelection();
	blockSignals( block );
	if ( d->currentItem->isSelectable() ) {
	    d->currentItem->selected = TRUE;
	    repaintItem( d->currentItem );
	}
	emit selectionChanged();
    }
    d->pressedItem = 0;

    if ( emitClicked ) {
	emit mouseButtonClicked( e->button(), item, e->globalPos() );
	emit clicked( item );
	emit clicked( item, e->globalPos() );
	if ( e->button() == RightButton )
	    emit rightButtonClicked( item, e->globalPos() );
    }
}

/*!
    \reimp
*/

void KileIconView::contentsMouseMoveEvent( QMouseEvent *e )
{
    KileIconViewItem *item = findItem( e->pos() );
    if ( d->highlightedItem != item ) {
	if ( item )
	    emit onItem( item );
	else
	    emit onViewport();
	d->highlightedItem = item;
    }

    if ( d->mousePressed && e->state() == NoButton )
	d->mousePressed = FALSE;

    if ( d->startDragItem )
	item = d->startDragItem;

    if ( d->mousePressed && item && item == d->currentItem &&
	 ( item->isSelected() || d->selectionMode == NoSelection ) && item->dragEnabled() ) {
	if ( !d->startDragItem ) {
	    d->currentItem->setSelected( TRUE, TRUE );
	    d->startDragItem = item;
	}
	if ( ( d->dragStartPos - e->pos() ).manhattanLength() > QApplication::startDragDistance() ) {
	    d->mousePressed = FALSE;
	    d->cleared = FALSE;

	    if ( d->tmpCurrentItem )
		repaintItem( d->tmpCurrentItem );
	}
    } else if ( d->mousePressed && !d->currentItem && d->rubber ) {
	doAutoScroll();
    }
}

/*!
    \reimp
*/

void KileIconView::contentsMouseDoubleClickEvent( QMouseEvent *e )
{
    KileIconViewItem *item = findItem( e->pos() );
    if ( item ) {
	selectAll( FALSE );
	item->setSelected( TRUE, TRUE );
	emit doubleClicked( item );
    }
}

/*!
    \reimp
*/



void KileIconView::resizeEvent( QResizeEvent* e )
{
    QScrollView::resizeEvent( e );
    if ( d->resizeMode == Adjust ) {
	optimize_layout = TRUE;
	adjustItems();
	optimize_layout = FALSE;
#if 0 // no need for timer delay anymore
	d->oldSize = e->oldSize();
	if ( d->adjustTimer->isActive() )
	    d->adjustTimer->stop();
	d->adjustTimer->start( 0, TRUE );
#endif
    }
}

/*!
    Adjusts the positions of the items to the geometry of the icon
    view.
*/

void KileIconView::adjustItems()
{
    d->adjustTimer->stop();
    if ( d->resizeMode == Adjust )
	    arrangeItemsInGrid( TRUE );
}

/*!
    \reimp
*/

void KileIconView::keyPressEvent( QKeyEvent *e )
{
    if ( !d->firstItem )
	return;

    if ( !d->currentItem ) {
	setCurrentItem( d->firstItem );
	if ( d->selectionMode == Single )
	    d->currentItem->setSelected( TRUE, TRUE );
	return;
    }

    bool selectCurrent = TRUE;

    switch ( e->key() ) {
    case Key_Escape:
	e->ignore();
	break;

    case Key_Home: {
	d->currInputString = QString::null;
	if ( !d->firstItem )
	    break;

	selectCurrent = FALSE;

	KileIconViewItem *item = d->currentItem;
	setCurrentItem( d->firstItem );

	handleItemChange( item, e->state() & ShiftButton, e->state() & ControlButton );
    } break;
    case Key_End: {
	d->currInputString = QString::null;
	if ( !d->lastItem )
	    break;

	selectCurrent = FALSE;

	KileIconViewItem *item = d->currentItem;
	setCurrentItem( d->lastItem );

	handleItemChange( item, e->state() & ShiftButton, e->state() & ControlButton );
    } break;
    case Key_Right: {
	d->currInputString = QString::null;
	KileIconViewItem *item;
	selectCurrent = FALSE;
	if ( d->arrangement == LeftToRight ) {
	    if ( !d->currentItem->next )
		break;

	    item = d->currentItem;
	    setCurrentItem( d->currentItem->next );

	    handleItemChange( item, e->state() & ShiftButton, e->state() & ControlButton );
	} else {
	    item = d->firstItem;
	    QRect r( 0, d->currentItem->y(), contentsWidth(), d->currentItem->height() );
	    for ( ; item; item = item->next ) {
		if ( item->x() > d->currentItem->x() && r.intersects( item->rect() ) ) {
		    QRect ir = r.intersect( item->rect() );
		    if ( item->next && r.intersects( item->next->rect() ) ) {
			QRect irn = r.intersect( item->next->rect() );
			if ( irn.height() > ir.height() )
			    item = item->next;
		    }
		    KileIconViewItem *i = d->currentItem;
		    setCurrentItem( item );
		    item = i;
		    handleItemChange( item, e->state() & ShiftButton, e->state() & ControlButton );
		    break;
		}
	    }
	}
    } break;
    case Key_Left: {
	d->currInputString = QString::null;
	KileIconViewItem *item;
	selectCurrent = FALSE;
	if ( d->arrangement == LeftToRight ) {
	    if ( !d->currentItem->prev )
		break;

	    item = d->currentItem;
	    setCurrentItem( d->currentItem->prev );

	    handleItemChange( item, e->state() & ShiftButton, e->state() & ControlButton );
	} else {
	    item = d->lastItem;
	    QRect r( 0, d->currentItem->y(), contentsWidth(), d->currentItem->height() );
	    for ( ; item; item = item->prev ) {
		if ( item->x() < d->currentItem->x() && r.intersects( item->rect() ) ) {
		    QRect ir = r.intersect( item->rect() );
		    if ( item->prev && r.intersects( item->prev->rect() ) ) {
			QRect irn = r.intersect( item->prev->rect() );
			if ( irn.height() > ir.height() )
			    item = item->prev;
		    }
		    KileIconViewItem *i = d->currentItem;
		    setCurrentItem( item );
		    item = i;
		    handleItemChange( item, e->state() & ShiftButton, e->state() & ControlButton );
		    break;
		}
	    }
	}
    } break;
    case Key_Space: {
	d->currInputString = QString::null;
	if ( d->selectionMode == Single)
	    break;

	d->currentItem->setSelected( !d->currentItem->isSelected(), TRUE );
    } break;
    case Key_Enter: case Key_Return:
	d->currInputString = QString::null;
	emit returnPressed( d->currentItem );
	break;
    case Key_Down: {
	d->currInputString = QString::null;
	KileIconViewItem *item;
	selectCurrent = FALSE;

	if ( d->arrangement == LeftToRight ) {
	    item = d->firstItem;
	    QRect r( d->currentItem->x(), 0, d->currentItem->width(), contentsHeight() );
	    for ( ; item; item = item->next ) {
		if ( item->y() > d->currentItem->y() && r.intersects( item->rect() ) ) {
		    QRect ir = r.intersect( item->rect() );
		    if ( item->next && r.intersects( item->next->rect() ) ) {
			QRect irn = r.intersect( item->next->rect() );
			if ( irn.width() > ir.width() )
			    item = item->next;
		    }
		    KileIconViewItem *i = d->currentItem;
		    setCurrentItem( item );
		    item = i;
		    handleItemChange( item, e->state() & ShiftButton, e->state() & ControlButton );
		    break;
		}
	    }
	} else {
	    if ( !d->currentItem->next )
		break;

	    item = d->currentItem;
	    setCurrentItem( d->currentItem->next );

	    handleItemChange( item, e->state() & ShiftButton, e->state() & ControlButton );
	}
    } break;
    case Key_Up: {
	d->currInputString = QString::null;
	KileIconViewItem *item;
	selectCurrent = FALSE;

	if ( d->arrangement == LeftToRight ) {
	    item = d->lastItem;
	    QRect r( d->currentItem->x(), 0, d->currentItem->width(), contentsHeight() );
	    for ( ; item; item = item->prev ) {
		if ( item->y() < d->currentItem->y() && r.intersects( item->rect() ) ) {
		    QRect ir = r.intersect( item->rect() );
		    if ( item->prev && r.intersects( item->prev->rect() ) ) {
			QRect irn = r.intersect( item->prev->rect() );
			if ( irn.width() > ir.width() )
			    item = item->prev;
		    }
		    KileIconViewItem *i = d->currentItem;
		    setCurrentItem( item );
		    item = i;
		    handleItemChange( item, e->state() & ShiftButton, e->state() & ControlButton );
		    break;
		}
	    }
	} else {
	    if ( !d->currentItem->prev )
		break;

	    item = d->currentItem;
	    setCurrentItem( d->currentItem->prev );

	    handleItemChange( item, e->state() & ShiftButton, e->state() & ControlButton );
	}
    } break;
    case Key_Next: {
	d->currInputString = QString::null;
	selectCurrent = FALSE;
	QRect r;
	if ( d->arrangement == LeftToRight )
	    r = QRect( 0, d->currentItem->y() + visibleHeight(), contentsWidth(), visibleHeight() );
	else
	    r = QRect( d->currentItem->x() + visibleWidth(), 0, visibleWidth(), contentsHeight() );
	KileIconViewItem *item = d->currentItem;
	KileIconViewItem *ni = findFirstVisibleItem( r  );
	if ( !ni ) {
	    if ( d->arrangement == LeftToRight )
		r = QRect( 0, d->currentItem->y() + d->currentItem->height(), contentsWidth(), contentsHeight() );
	    else
		r = QRect( d->currentItem->x() + d->currentItem->width(), 0, contentsWidth(), contentsHeight() );
	    ni = findLastVisibleItem( r  );
	}
	if ( ni ) {
	    setCurrentItem( ni );
	    handleItemChange( item, e->state() & ShiftButton, e->state() & ControlButton );
	}
    } break;
    case Key_Prior: {
	d->currInputString = QString::null;
	selectCurrent = FALSE;
	QRect r;
	if ( d->arrangement == LeftToRight )
	    r = QRect( 0, d->currentItem->y() - visibleHeight(), contentsWidth(), visibleHeight() );
	else
	    r = QRect( d->currentItem->x() - visibleWidth(), 0, visibleWidth(), contentsHeight() );
	KileIconViewItem *item = d->currentItem;
	KileIconViewItem *ni = findFirstVisibleItem( r  );
	if ( !ni ) {
	    if ( d->arrangement == LeftToRight )
		r = QRect( 0, 0, contentsWidth(), d->currentItem->y() );
	    else
		r = QRect( 0, 0, d->currentItem->x(), contentsHeight() );
	    ni = findFirstVisibleItem( r  );
	}
	if ( ni ) {
	    setCurrentItem( ni );
	    handleItemChange( item, e->state() & ShiftButton, e->state() & ControlButton );
	}
    } break;
    default:
	if ( !e->text().isEmpty() && e->text()[ 0 ].isPrint() ) {
	    selectCurrent = FALSE;
	    findItemByName( e->text() );
	} else {
	    selectCurrent = FALSE;
	    d->currInputString = QString::null;
	    if ( e->state() & ControlButton ) {
		switch ( e->key() ) {
		case Key_A:
		    selectAll( TRUE );
		    break;
		}
	    }
	}
    }

    if ( !( e->state() & ShiftButton ) || !d->selectAnchor )
	d->selectAnchor = d->currentItem;

    if ( d->currentItem && !d->currentItem->isSelected() &&
	 d->selectionMode == Single && selectCurrent ) {
	d->currentItem->setSelected( TRUE );
    }

    if ( e->key() != Key_Shift &&
	 e->key() != Key_Control &&
	 e->key() != Key_Alt )
	ensureItemVisible( d->currentItem );
}

/*!
    \reimp
*/

void KileIconView::focusInEvent( QFocusEvent *e )
{
    Q_UNUSED(e) // I need this to get rid of a Borland warning
    d->mousePressed = FALSE;
    if ( d->currentItem )
	repaintItem( d->currentItem );
    else if ( d->firstItem && e->reason() != QFocusEvent::Mouse ) {
	d->currentItem = d->firstItem;
	emit currentChanged( d->currentItem );
	repaintItem( d->currentItem );
    }

    if ( style().styleHint( QStyle::SH_ItemView_ChangeHighlightOnFocus, this ) ) {
	QRect r = visibleRect();
	for ( KileIconViewItem *item = firstItem(); item; item = item->nextItem() ) {
	    if ( item->isSelected() && item->rect().intersects( r ) )
		repaintItem( item );
	}
    }

    if ( d->currentItem )
	setMicroFocusHint( d->currentItem->x(), d->currentItem->y(), d->currentItem->width(), d->currentItem->height(), FALSE );
}

/*!
    \reimp
*/

void KileIconView::focusOutEvent( QFocusEvent *e )
{
    Q_UNUSED(e) // I need this to get rid of a Borland warning
    if ( d->currentItem )
	repaintItem( d->currentItem );

    if ( style().styleHint( QStyle::SH_ItemView_ChangeHighlightOnFocus, this ) ) {
	if ( e->reason() != QFocusEvent::Popup ) {
	    QRect r = visibleRect();
	    for ( KileIconViewItem *item = firstItem(); item; item = item->nextItem() ) {
		if ( item->isSelected() && item->rect().intersects( r ) )
		    repaintItem( item );
	    }
	} else {
	    QWidget *widget = qApp->focusWidget();
	    if ( widget && widget->inherits( "QPopupMenu" ) )
		widget->installEventFilter( this );
	}
    }
}

/*!
    Draws the rubber band using the painter \a p.
*/

void KileIconView::drawRubber( QPainter *p )
{
    if ( !p || !d->rubber )
	return;

    QPoint pnt( d->rubber->x(), d->rubber->y() );
    pnt = contentsToViewport( pnt );

    style().drawPrimitive(QStyle::PE_FocusRect, p,
			  QRect( pnt.x(), pnt.y(),
				 d->rubber->width(), d->rubber->height() ),
			  colorGroup(), QStyle::Style_Default,
			  QStyleOption(colorGroup().base()));
}



/*!
    Inserts the KileIconViewItem \a item in the icon view's grid. \e{You
    should never need to call this function.} Instead, insert
    KileIconViewItems by creating them with a pointer to the KileIconView
    that they are to be inserted into.
*/

void KileIconView::insertInGrid( KileIconViewItem *item )
{
    if ( !item )
	return;

    if ( d->reorderItemsWhenInsert ) {
	// #### make this efficient - but it's not too dramatic
	int y = d->spacing;

	item->dirty = FALSE;
	if ( item == d->firstItem ) {
	    bool dummy;
	    makeRowLayout( item, y, dummy );
	    return;
	}

	KileIconViewItem *begin = rowBegin( item );
	y = begin->y();
	while ( begin ) {
	    bool dummy;
	    begin = makeRowLayout( begin, y, dummy );

	    if ( !begin || !begin->next )
		break;

	    begin = begin->next;
	}
	item->dirty = FALSE;
    } else {
	QRegion r( QRect( 0, 0, QMAX( contentsWidth(), visibleWidth() ),
			  QMAX( contentsHeight(), visibleHeight() ) ) );

	KileIconViewItem *i = d->firstItem;
	int y = -1;
	for ( ; i; i = i->next ) {
	    r = r.subtract( i->rect() );
	    y = QMAX( y, i->y() + i->height() );
	}

	QMemArray<QRect> rects = r.rects();
	QMemArray<QRect>::Iterator it = rects.begin();
	bool foundPlace = FALSE;
	for ( ; it != rects.end(); ++it ) {
	    QRect rect = *it;
	    if ( rect.width() >= item->width() &&
		 rect.height() >= item->height() ) {
		int sx = 0, sy = 0;
		if ( rect.width() >= item->width() + d->spacing )
		    sx = d->spacing;
		if ( rect.height() >= item->height() + d->spacing )
		    sy = d->spacing;
		item->move( rect.x() + sx, rect.y() + sy );
		foundPlace = TRUE;
		break;
	    }
	}

	if ( !foundPlace )
	    item->move( d->spacing, y + d->spacing );

	resizeContents( QMAX( contentsWidth(), item->x() + item->width() ),
			QMAX( contentsHeight(), item->y() + item->height() ) );
	item->dirty = FALSE;
    }
}

/*!
    Emits a signal to indicate selection changes. \a i is the
    KileIconViewItem that was selected or de-selected.

    \e{You should never need to call this function.}
*/

void KileIconView::emitSelectionChanged( KileIconViewItem *i )
{
    emit selectionChanged();
    if ( d->selectionMode == Single )
	emit selectionChanged( i ? i : d->currentItem );
}

/*!
    \internal
*/

void KileIconView::emitRenamed( KileIconViewItem *item )
{
    if ( !item )
	return;

    emit itemRenamed( item, item->text() );
    emit itemRenamed( item );
}

/*!
    If a drag enters the icon view the shapes of the objects which the
    drag contains are drawn, usnig \a pos as origin.
*/

void KileIconView::drawDragShapes( const QPoint &pos )
{

}

/*!
    This function is called to draw the rectangle \a r of the
    background using the painter \a p.

    The default implementation fills \a r with the viewport's
    backgroundBrush(). Subclasses may reimplement this to draw custom
    backgrounds.

    \sa contentsX() contentsY() drawContents()
*/

void KileIconView::drawBackground( QPainter *p, const QRect &r )
{
    p->fillRect( r, viewport()->backgroundBrush() );
}

/*!
    \reimp
*/

bool KileIconView::eventFilter( QObject * o, QEvent * e )
{
    if ( o == viewport() ) {
	switch( e->type() ) {
	case QEvent::FocusIn:
	    focusInEvent( (QFocusEvent*)e );
	    return TRUE;
	case QEvent::FocusOut:
	    focusOutEvent( (QFocusEvent*)e );
	    return TRUE;
	case QEvent::Enter:
	    enterEvent( e );
	    return TRUE;
	case QEvent::Paint:
	    if ( o == viewport() ) {
		if ( d->dragging ) {
		    if ( !d->rubber )
			drawDragShapes( d->oldDragPos );
		}
		viewportPaintEvent( (QPaintEvent*)e );
		if ( d->dragging ) {
		    if ( !d->rubber )
			drawDragShapes( d->oldDragPos );
		}
	    }
	    return TRUE;
	default:
	    // nothing
	    break;
	}
    } else if ( e->type() == QEvent::Hide && o->inherits( "QPopupMenu" ) ) {
	QRect r = visibleRect();
	for ( KileIconViewItem *item = firstItem(); item; item = item->nextItem() ) {
	    if ( item->isSelected() && item->rect().intersects( r ) )
		repaintItem( item );
	}
	o->removeEventFilter( this );
    }

    return QScrollView::eventFilter( o, e );
}


/*!
    \reimp
*/

QSize KileIconView::minimumSizeHint() const
{
    return QSize( 100, 100 );
}

/*!
    \internal
    Clears the string which is used for setting the current item when
    the user types something in.
*/

void KileIconView::clearInputString()
{
    d->currInputString = QString::null;
}

/*!
  \internal
  Finds the first item beginning with \a text and makes
  it the current item.
*/

void KileIconView::findItemByName( const QString &text )
{
    if ( d->inputTimer->isActive() )
	d->inputTimer->stop();
    d->inputTimer->start( 500, TRUE );
    d->currInputString += text.lower();
    KileIconViewItem *item = findItem( d->currInputString );
    if ( item ) {
	setCurrentItem( item );
	if ( d->selectionMode == Extended ) {
	    bool changed = FALSE;
	    bool block = signalsBlocked();
	    blockSignals( TRUE );
	    selectAll( FALSE );
	    blockSignals( block );
	    if ( !item->selected && item->isSelectable() ) {
		changed = TRUE;
		item->selected = TRUE;
		repaintItem( item );
	    }
	    if ( changed )
		emit selectionChanged();
	}
    }
}

/*!
    Lays out a row of icons (if Arrangement == \c TopToBottom this is
    a column). Starts laying out with the item \a begin. \a y is the
    starting coordinate. Returns the last item of the row (column) and
    sets the new starting coordinate to \a y. The \a changed parameter
    is used internally.

    \warning This function may be made private in a future version of
    Qt. We do not recommend calling it.
*/

KileIconViewItem *KileIconView::makeRowLayout( KileIconViewItem *begin, int &y, bool &changed )
{
    KileIconViewItem *end = 0;

    bool reverse = QApplication::reverseLayout();
    changed = FALSE;

    if ( d->arrangement == LeftToRight ) {

	if ( d->rastX == -1 ) {
	    // first calculate the row height
	    int h = 0;
	    int x = 0;
	    int ih = 0;
	    KileIconViewItem *item = begin;
	    for (;;) {
		x += d->spacing + item->width();
		if ( x > visibleWidth() && item != begin ) {
		    item = item->prev;
		    break;
		}
		h = QMAX( h, item->height() );
		ih = QMAX( ih, item->pixmapRect().height() );
		KileIconViewItem *old = item;
		item = item->next;
		if ( !item ) {
		    item = old;
		    break;
		}
	    }
	    end = item;

	    if ( d->rastY != -1 )
		h = QMAX( h, d->rastY );

	    // now move the items
	    item = begin;
	    for (;;) {
		item->dirty = FALSE;
		int x;
		if ( item == begin ) {
		    if ( reverse )
			x = visibleWidth() - d->spacing - item->width();
		    else
			x = d->spacing;
		} else {
		    if ( reverse )
			x = item->prev->x() - item->width() - d->spacing;
		    else
			x = item->prev->x() + item->prev->width() + d->spacing;
		}
		changed = item->move( x, y + ih - item->pixmapRect().height() ) || changed;
		if ( y + h < item->y() + item->height() )
		    h = QMAX( h, ih + item->textRect().height() );
		if ( item == end )
		    break;
		item = item->next;
	    }
	    y += h + d->spacing;
	} else {
	    // first calculate the row height
	    int h = begin->height();
	    int x = d->spacing;
	    int ih = begin->pixmapRect().height();
	    KileIconViewItem *item = begin;
	    int i = 0;
	    int sp = 0;
	    for (;;) {
		int r = calcGridNum( item->width(), d->rastX );
		if ( item == begin ) {
		    i += r;
		    sp += r;
		    x = d->spacing + d->rastX * r;
		} else {
		    sp += r;
		    i += r;
		    x = i * d->rastX + sp * d->spacing;
		}
		if ( x > visibleWidth() && item != begin ) {
		    item = item->prev;
		    break;
		}
		h = QMAX( h, item->height() );
		ih = QMAX( ih, item->pixmapRect().height() );
		KileIconViewItem *old = item;
		item = item->next;
		if ( !item ) {
		    item = old;
		    break;
		}
	    }
	    end = item;

	    if ( d->rastY != -1 )
		h = QMAX( h, d->rastY );

	    // now move the items
	    item = begin;
	    i = 0;
	    sp = 0;
	    for (;;) {
		item->dirty = FALSE;
		int r = calcGridNum( item->width(), d->rastX );
		if ( item == begin ) {
		    if ( d->itemTextPos == Bottom )
			changed = item->move( d->spacing + ( r * d->rastX - item->width() ) / 2,
					      y + ih - item->pixmapRect().height() ) || changed;
		    else
			changed = item->move( d->spacing, y + ih - item->pixmapRect().height() ) || changed;
		    i += r;
		    sp += r;
		} else {
		    sp += r;
		    int x = i * d->rastX + sp * d->spacing;
		    if ( d->itemTextPos == Bottom )
			changed = item->move( x + ( r * d->rastX - item->width() ) / 2,
					      y + ih - item->pixmapRect().height() ) || changed;
		    else
			changed = item->move( x, y + ih - item->pixmapRect().height() ) || changed;
		    i += r;
		}
		if ( y + h < item->y() + item->height() )
		    h = QMAX( h, ih + item->textRect().height() );
		if ( item == end )
		    break;
		item = item->next;
	    }
	    y += h + d->spacing;
	}


    } else { // -------------------------------- SOUTH ------------------------------

	int x = y;

	{
	    int w = 0;
	    int y = 0;
	    KileIconViewItem *item = begin;
	    for (;;) {
		y += d->spacing + item->height();
		if ( y > visibleHeight() && item != begin ) {
		    item = item->prev;
		    break;
		}
		w = QMAX( w, item->width() );
		KileIconViewItem *old = item;
		item = item->next;
		if ( !item ) {
		    item = old;
		    break;
		}
	    }
	    end = item;

	    if ( d->rastX != -1 )
		w = QMAX( w, d->rastX );

	    // now move the items
	    item = begin;
	    for (;;) {
		item->dirty = FALSE;
		if ( d->itemTextPos == Bottom ) {
		    if ( item == begin )
			changed = item->move( x + ( w - item->width() ) / 2, d->spacing )  || changed;
		    else
			changed = item->move( x + ( w - item->width() ) / 2,
					      item->prev->y() + item->prev->height() + d->spacing ) || changed;
		} else {
		    if ( item == begin )
			changed = item->move( x, d->spacing ) || changed;
		    else
			changed = item->move( x, item->prev->y() + item->prev->height() + d->spacing ) || changed;
		}
		if ( item == end )
		    break;
		item = item->next;
	    }
	    x += w + d->spacing;
	}

	y = x;
    }

    return end;
}

/*!
  \internal
  Calculates how many cells an item of width \a w needs in a grid with of
  \a x and returns the result.
*/

int KileIconView::calcGridNum( int w, int x ) const
{
    float r = (float)w / (float)x;
    if ( ( w / x ) * x != w )
	r += 1.0;
    return (int)r;
}

/*!
  \internal
  Returns the first item of the row which contains \a item.
*/

KileIconViewItem *KileIconView::rowBegin( KileIconViewItem * ) const
{
    // #### todo
    return d->firstItem;
}

/*!
    Sorts and rearranges all the items in the icon view. If \a
    ascending is TRUE, the items are sorted in increasing order,
    otherwise they are sorted in decreasing order.

    KileIconViewItem::compare() is used to compare pairs of items. The
    sorting is based on the items' keys; these default to the items'
    text unless specifically set to something else.

    Note that this function sets the sort order to \a ascending.

    \sa KileIconViewItem::key(), KileIconViewItem::setKey(),
    KileIconViewItem::compare(), KileIconView::setSorting(),
    KileIconView::sortDirection()
*/

void KileIconView::sort( bool ascending )
{
    if ( count() == 0 )
	return;

    d->sortDirection = ascending;
    KileIconViewPrivate::SortableItem *items = new KileIconViewPrivate::SortableItem[ count() ];

    KileIconViewItem *item = d->firstItem;
    int i = 0;
    for ( ; item; item = item->next )
	items[ i++ ].item = item;

    qsort( items, count(), sizeof( KileIconViewPrivate::SortableItem ), cmpIconViewItems );

    KileIconViewItem *prev = 0;
    item = 0;
    if ( ascending ) {
	for ( i = 0; i < (int)count(); ++i ) {
	    item = items[ i ].item;
	    if ( item ) {
		item->prev = prev;
		if ( item->prev )
		    item->prev->next = item;
		item->next = 0;
	    }
	    if ( i == 0 )
		d->firstItem = item;
	    if ( i == (int)count() - 1 )
		d->lastItem = item;
	    prev = item;
	}
    } else {
	for ( i = (int)count() - 1; i >= 0 ; --i ) {
	    item = items[ i ].item;
	    if ( item ) {
		item->prev = prev;
		if ( item->prev )
		    item->prev->next = item;
		item->next = 0;
	    }
	    if ( i == (int)count() - 1 )
		d->firstItem = item;
	    if ( i == 0 )
		d->lastItem = item;
	    prev = item;
	}
    }

    delete [] items;

    arrangeItemsInGrid( TRUE );
}

/*!
    \reimp
*/

QSize KileIconView::sizeHint() const
{
    constPolish();

    if ( !d->firstItem )
	return QSize( 50, 50 );

    if ( d->dirty && d->firstSizeHint ) {
	( (KileIconView*)this )->resizeContents( QMAX( 400, contentsWidth() ),
					      QMAX( 400, contentsHeight() ) );
	if ( autoArrange() )
	    ( (KileIconView*)this )->arrangeItemsInGrid( FALSE );
	d->firstSizeHint = FALSE;
    }

    d->dirty = TRUE;
    int sbextent = style().pixelMetric(QStyle::PM_ScrollBarExtent, this);
    return QSize( QMIN(400, contentsWidth() + sbextent),
		  QMIN(400, contentsHeight() + sbextent) );
}

/*!
  \internal
*/

void KileIconView::updateContents()
{
    viewport()->update();
}

/*!
    \reimp
*/

void KileIconView::enterEvent( QEvent *e )
{
    QScrollView::enterEvent( e );
    emit onViewport();
}

/*!
  \internal
  This function is always called when the geometry of an item changes.
  This function moves the item into the correct area in the internal
  data structure.
*/

void KileIconView::updateItemContainer( KileIconViewItem *item )
{
    if ( !item || d->containerUpdateLocked || (!isVisible() && autoArrange()) )
	return;

    if ( item->d->container1 && d->firstContainer ) {
	item->d->container1->items.removeRef( item );
    }
    item->d->container1 = 0;
    if ( item->d->container2 && d->firstContainer ) {
	item->d->container2->items.removeRef( item );
    }
    item->d->container2 = 0;

    KileIconViewPrivate::ItemContainer *c = d->firstContainer;
    if ( !c ) {
	appendItemContainer();
	c = d->firstContainer;
    }

    const QRect irect = item->rect();
    bool contains = FALSE;
    for (;;) {
	if ( c->rect.intersects( irect ) ) {
	    contains = c->rect.contains( irect );
	    break;
	}

	c = c->n;
	if ( !c ) {
	    appendItemContainer();
	    c = d->lastContainer;
	}
    }

    if ( !c ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "KileIconViewItem::updateItemContainer(): No fitting container found!" );
#endif
	return;
    }

    c->items.append( item );
    item->d->container1 = c;

    if ( !contains ) {
	c = c->n;
	if ( !c ) {
	    appendItemContainer();
	    c = d->lastContainer;
	}
	c->items.append( item );
	item->d->container2 = c;
    }
    if ( contentsWidth() < irect.right() || contentsHeight() < irect.bottom() )
	resizeContents( QMAX( contentsWidth(), irect.right() ), QMAX( contentsHeight(), irect.bottom() ) );
}

/*!
  \internal
  Appends a new rect area to the internal data structure of the items.
*/

void KileIconView::appendItemContainer()
{
    QSize s;
    // #### We have to find out which value is best here
    if ( d->arrangement == LeftToRight )
	s = QSize( INT_MAX - 1, RECT_EXTENSION );
    else
	s = QSize( RECT_EXTENSION, INT_MAX - 1 );

    if ( !d->firstContainer ) {
	d->firstContainer = new KileIconViewPrivate::ItemContainer( 0, 0, QRect( QPoint( 0, 0 ), s ) );
	d->lastContainer = d->firstContainer;
    } else {
	if ( d->arrangement == LeftToRight )
	    d->lastContainer = new KileIconViewPrivate::ItemContainer(
		d->lastContainer, 0, QRect( d->lastContainer->rect.bottomLeft(), s ) );
	else
	    d->lastContainer = new KileIconViewPrivate::ItemContainer(
		d->lastContainer, 0, QRect( d->lastContainer->rect.topRight(), s ) );
    }
}

/*!  \internal

  Rebuilds the whole internal data structure. This is done when it's
  likely that most/all items change their geometry (e.g. in
  arrangeItemsInGrid()), because calling this is then more efficient
  than calling updateItemContainer() for each item.
*/

void KileIconView::rebuildContainers()
{
    KileIconViewPrivate::ItemContainer *c = d->firstContainer, *tmpc;
    while ( c ) {
	tmpc = c->n;
	delete c;
	c = tmpc;
    }
    d->firstContainer = d->lastContainer = 0;

    KileIconViewItem *item = d->firstItem;
    appendItemContainer();
    c = d->lastContainer;
    while ( item ) {
	if ( c->rect.contains( item->rect() ) ) {
	    item->d->container1 = c;
	    item->d->container2 = 0;
	    c->items.append( item );
	    item = item->next;
	} else if ( c->rect.intersects( item->rect() ) ) {
	    item->d->container1 = c;
	    c->items.append( item );
	    c = c->n;
	    if ( !c ) {
		appendItemContainer();
		c = d->lastContainer;
	    }
	    c->items.append( item );
	    item->d->container2 = c;
	    item = item->next;
	    c = c->p;
	} else {
	    if ( d->arrangement == LeftToRight ) {
		if ( item->y() < c->rect.y() && c->p ) {
		    c = c->p;
		    continue;
		}
	    } else {
		if ( item->x() < c->rect.x() && c->p ) {
		    c = c->p;
		    continue;
		}
	    }

	    c = c->n;
	    if ( !c ) {
		appendItemContainer();
		c = d->lastContainer;
	    }
	}
    }
}

/*!
  \internal
*/

void KileIconView::movedContents( int, int )
{
    if ( d->drawDragShapes ) {
	drawDragShapes( d->oldDragPos );
	d->oldDragPos = QPoint( -1, -1 );
    }
}

void KileIconView::handleItemChange( KileIconViewItem *old, bool shift, bool control )
{
    if ( d->selectionMode == Single ) {
	bool block = signalsBlocked();
	blockSignals( TRUE );
	if ( old )
	    old->setSelected( FALSE );
	blockSignals( block );
	d->currentItem->setSelected( TRUE, TRUE );
    } else if ( d->selectionMode == Extended ) {
	if ( control ) {
	    // nothing
	} else if ( shift ) {
	    if ( !d->selectAnchor ) {
		if ( old && !old->selected && old->isSelectable() ) {
		    old->selected = TRUE;
		    repaintItem( old );
		}
		d->currentItem->setSelected( TRUE, TRUE );
	    } else {
		KileIconViewItem *from = d->selectAnchor, *to = d->currentItem;
		if ( !from || !to )
		    return;
		KileIconViewItem *i = 0;
		int index =0;
		int f_idx = -1, t_idx = -1;
		for ( i = d->firstItem; i; i = i->next, index++ ) {
		    if ( i == from )
			f_idx = index;
		    if ( i == to )
			t_idx = index;
		    if ( f_idx != -1 && t_idx != -1 )
			break;
		}
		if ( f_idx > t_idx ) {
		    i = from;
		    from = to;
		    to = i;
		}

		bool changed = FALSE;
		for ( i = d->firstItem; i && i != from; i = i->next ) {
			if ( i->selected ) {
			    i->selected = FALSE;
			    changed = TRUE;
			    repaintItem( i );
			}
		    }
		for ( i = to->next; i; i = i->next ) {
		    if ( i->selected ) {
			i->selected = FALSE;
			changed = TRUE;
			repaintItem( i );
		    }
		}

		for ( i = from; i; i = i->next ) {
		    if ( !i->selected && i->isSelectable() ) {
			i->selected = TRUE;
			changed = TRUE;
			repaintItem( i );
		    }
		    if ( i == to )
			break;
		}
		if ( changed )
		    emit selectionChanged();
	    }
	} else {
	    blockSignals( TRUE );
	    selectAll( FALSE );
	    blockSignals( FALSE );
	    d->currentItem->setSelected( TRUE, TRUE );
	}
    } else {
	if ( shift )
	    d->currentItem->setSelected( !d->currentItem->isSelected(), TRUE );
    }
}

QBitmap KileIconView::mask( QPixmap *pix ) const
{
    QBitmap m;
    if ( d->maskCache.find( QString::number( pix->serialNumber() ), m ) )
	return m;
    m = pix->createHeuristicMask();
    d->maskCache.insert( QString::number( pix->serialNumber() ), m );
    return m;
}

/*!
    \reimp
    \internal

    (Implemented to get rid of a compiler warning.)
*/
void KileIconView::drawContents( QPainter * )
{
}

/*!
    \reimp
*/
void KileIconView::windowActivationChange( bool )
{
    if ( !isVisible() )
	return;

    if ( palette().active() != palette().inactive() )
	viewport()->update();
}

/*!
    Returns TRUE if an iconview item is being renamed; otherwise
    returns FALSE.
*/

bool KileIconView::isRenaming() const
{
    return FALSE;
}


#include "kileiconview.moc"
