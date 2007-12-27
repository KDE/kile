/***************************************************************************
                           tabulardialog.cpp
----------------------------------------------------------------------------
    date                 : Sep 17 2006
    version              : 0.26
    copyright            : (C) 2005-2006 by Holger Danielsson
    email                : holger.danielsson@t-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "tabulardialog.h"
#include "codecompletion.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qstyle.h>
#include <q3frame.h>
#include <q3groupbox.h>
#include <q3buttongroup.h>
#include <q3whatsthis.h>
#include <qcursor.h>
#include <qmap.h>
//Added by qt3to4:
#include <QPixmap>
#include <QMouseEvent>
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <QEvent>
#include <Q3VBoxLayout>
#include <Q3PopupMenu>

#include <kmessagebox.h>
#include <klocale.h>
#include "kiledebug.h"

namespace KileDialog 
{

static const char * const all_border_xpm[] = {
"14 14 2 1",
"# c #000000",
". c #ffffff",
"##############",
"#............#",
"#............#",
"#............#",
"#............#",
"#............#",
"#............#",
"#............#",
"#............#",
"#............#",
"#............#",
"#............#",
"#............#",
"##############"
};

static const char * const lr_border_xpm[] = {
"14 14 2 1",
"# c #000000",
". c #ffffff",
"#............#",
"#............#",
"#............#", 
"#............#",
"#............#",
"#............#",
"#............#",
"#............#",
"#............#",
"#............#",
"#............#",
"#............#",
"#............#",
"#............#"
};

static const char * const tb_border_xpm[] = {
"14 14 2 1",
"# c #000000",
". c #ffffff",
"##############",
"..............",
"..............",
"..............",
"..............",
"..............",
"..............",
"..............",
"..............",
"..............",
"..............",
"..............",
"..............",
"##############"
};

static const char * const no_border_xpm[] = {
"14 14 2 1",
"# c #000000",
". c #ffffff",
"..............",
"..............",
"..............",
"..............",
"..............",
"..............",
"..............",
"..............",
"..............",
"..............",
"..............",
"..............",
"..............",
".............."
};


////////////////////////////// TabCellFrame //////////////////////////////

TabCellFrame::TabCellFrame(QWidget* parent) : Q3Frame(parent) 
{
	m_border = TabularCell::cbNone;
	
	setBackgroundColor(Qt::white);
	setFixedWidth(120);
	setFixedHeight(120);
	setLineWidth(2);
	setFrameStyle(Q3Frame::Box | Q3Frame::Raised);
	
	QRect r = contentsRect();
	int x1 = r.left();
	int y1 = r.top();
	int x2 = r.right();
	int y2 = r.bottom();
	
	m_left.setRect(x1,y1+20,20,y2-43);
	m_top.setRect(x1+20,y1,x2-43,20);
	m_right.setRect(x2-20,y1+20,20,y2-43);
	m_bottom.setRect(x1+20,y2-20,x2-43,20);
		
}

void TabCellFrame::setBorder(int value)
{
	m_border = value;
	update();
}

void TabCellFrame::drawContents(QPainter *p)
{
	p->save();

	QRect r = contentsRect();
	int x1 = r.left();
	int y1 = r.top();
	int x2 = r.right();
	int y2 = r.bottom();
	
	// left/top
	p->setPen(Qt::black);
	p->drawLine(x1+6,y1+14,x1+14,y1+14);
	p->drawLine(x1+14,y1+14,x1+14,y1+6);
	
	// left/bottom
	p->drawLine(x1+6,y2-14,x1+14,y2-14);
	p->drawLine(x1+14,y2-14,x1+14,y2-6);

	// right/top
	p->drawLine(x2-6,y1+14,x2-14,y1+14);
	p->drawLine(x2-14,y1+14,x2-14,y1+6);
	
	// right/bottom
	p->drawLine(x2-6,y2-14,x2-14,y2-14);
	p->drawLine(x2-14,y2-14,x2-14,y2-6);
	
	// centered rectangle
	p->setPen(Qt::gray);
	p->setBrush(Qt::gray);
	p->drawRect(x1+20,y1+20,x2-43,y2-43);

	//QPen pen = QPen(Qt::red,4);
	QPen pen = QPen(Qt::black,4);
	p->setPen(pen);
	if ( m_border & TabularCell::cbLeft )
		p->drawLine(x1+10,y1+20,x1+10,y2-20);
	if ( m_border & TabularCell::cbTop )
		p->drawLine(x1+20,y1+10,x2-20,y1+10);
	if ( m_border & TabularCell::cbRight )
		p->drawLine(x2-10,y1+20,x2-10,y2-20);
	if ( m_border & TabularCell::cbBottom )
		p->drawLine(x1+20,y2-10,x2-20,y2-10);

	p->restore();
}

void TabCellFrame::mousePressEvent(QMouseEvent *event)
{
	if ( event->button() != Qt::LeftButton )
		return;
		
	int x = event->x();
	int y = event->y();
	//KILE_DEBUG() << "left mouse button: x=" << x << " y=" << endl;
		
	int state = 0;
	if ( m_left.contains(x,y) )
		state = TabularCell::cbLeft;
	else if ( m_top.contains(x,y) )
		state = TabularCell::cbTop;
	else if ( m_right.contains(x,y) )
		state = TabularCell::cbRight;
	else if ( m_bottom.contains(x,y) )
		state = TabularCell::cbBottom;
		
	if ( state > 0 ) 
	{
		if ( m_border & state ) 
		{
			m_border &= ~state;
		} 
		else 
		{
			m_border |= state;
		}
		update();
	}
}

void TabCellFrame::mouseDoubleClickEvent(QMouseEvent *event)
{
	event->accept();
}

//////////////////////////////////////////////////////////////////////

//BEGIN TabCellDialog

TabCellDialog::TabCellDialog(QWidget *parent, TabularCell::Data *data,
                             const QString &headerlabel, const QStringList &alignlist) 
	: KDialog( parent)
{
	setCaption(i18n("Cell Properties"));
	setModal(true);
	setButtons(Ok | Cancel | User1);
	setDefaultButton(Ok);
	showButtonSeparator(true);

	QWidget *page = new QWidget(this);
	setMainWidget(page);
	
	if ( headerlabel.isEmpty() ) 
	{
		m_header = false;
		m_headerlabel = "l";
	} 
	else 
	{
		m_header = true;
		m_headerlabel = headerlabel;
	}	
	
	Q3GridLayout *grid = new Q3GridLayout( page, 4,2, 6,6);
	
	// font group
	Q3GroupBox *fontgroup = new Q3GroupBox( i18n("Font"),page);
	fontgroup->setColumnLayout(0, Qt::Vertical );
	fontgroup->layout()->setSpacing( 6 );
	fontgroup->layout()->setMargin( 11 );

	m_cbBold = new QCheckBox(i18n("Bold"),fontgroup);	
	m_cbItalic = new QCheckBox(i18n("Italic"),fontgroup);	
	
	Q3VBoxLayout *fontgrouplayout = new Q3VBoxLayout( fontgroup->layout() );
	fontgrouplayout->setAlignment( Qt::AlignTop );
	fontgrouplayout->addWidget( m_cbBold ); 
	fontgrouplayout->addWidget( m_cbItalic );
	
	// color group
	Q3GroupBox *colorgroup = new Q3GroupBox( i18n("Color"),page);
	colorgroup->setColumnLayout(0, Qt::Vertical );
	colorgroup->layout()->setSpacing( 6 );
	colorgroup->layout()->setMargin( 11 );

	QLabel *label1 = new QLabel(i18n("Background:"),colorgroup);
	QLabel *label2 = new QLabel(i18n("Textcolor:"),colorgroup);
	m_ccBgcolor = new KColorCombo(colorgroup);	
	m_ccTextcolor = new KColorCombo(colorgroup);	
	
	Q3GridLayout *colorgrouplayout = new Q3GridLayout( colorgroup->layout() );
	colorgrouplayout->setAlignment( Qt::AlignTop );
	colorgrouplayout->addWidget( label1,0,0 ); 
	colorgrouplayout->addWidget( label2,1,0 ); 
	colorgrouplayout->addWidget( m_ccBgcolor,0,1 ); 
	colorgrouplayout->addWidget( m_ccTextcolor,1,1 );
	
	// alignment group
	Q3ButtonGroup *aligngroup = new Q3ButtonGroup( i18n("Alignment"),page);
	aligngroup->setColumnLayout(0, Qt::Vertical );
	aligngroup->layout()->setSpacing( 6 );
	aligngroup->layout()->setMargin( 11 );

	m_rbAlignleft = new QRadioButton(i18n("Left"),aligngroup);	
	m_rbAligncenter = new QRadioButton(i18n("Center"),aligngroup);	
	m_rbAlignright = new QRadioButton(i18n("Right"),aligngroup);	
	
	Q3VBoxLayout *aligngrouplayout = new Q3VBoxLayout( aligngroup->layout() );
	aligngrouplayout->setAlignment( Qt::AlignTop );
	aligngrouplayout->addWidget( m_rbAlignleft ); 
	aligngrouplayout->addWidget( m_rbAligncenter );
	aligngrouplayout->addWidget( m_rbAlignright );
	
	// frame group
	Q3GroupBox *framegroup = new Q3GroupBox( i18n("Frame"),page);
	framegroup->setColumnLayout(0, Qt::Vertical );
	framegroup->layout()->setSpacing( 6 );
	framegroup->layout()->setMargin( 11 );

	QLabel *label3 = new QLabel(i18n("Standard:"),framegroup);
	QLabel *label4 = new QLabel(i18n("User defined:"),framegroup);
	
	QWidget *lineframe = new QWidget(framegroup);
	m_pbFrame1 = new KPushButton( "x", lineframe );
	m_pbFrame1->setObjectName( "pb_no_border" );
	m_pbFrame2 = new KPushButton( "", lineframe );
	m_pbFrame2->setObjectName( "pb_lr_border" );
	m_pbFrame3 = new KPushButton( "", lineframe );
	m_pbFrame3->setObjectName( "pb_tb_border" );
	m_pbFrame4 = new KPushButton( "", lineframe );
	m_pbFrame4->setObjectName( "pb_all_border" );
	
	Q3HBoxLayout *linebox = new Q3HBoxLayout(lineframe);
	linebox->addWidget(m_pbFrame1);
	linebox->addSpacing(4);
	linebox->addWidget(m_pbFrame2);
	linebox->addSpacing(4);
	linebox->addWidget(m_pbFrame3);
	linebox->addSpacing(4);
	linebox->addWidget(m_pbFrame4);
	linebox->addStretch(1);

	QWidget *borderframe = new QWidget(framegroup);
	m_cellframe = new TabCellFrame(borderframe);

	Q3HBoxLayout *borderbox = new Q3HBoxLayout(borderframe);
	borderbox->addStretch(1);
	borderbox->addWidget(m_cellframe);
	borderbox->addStretch(1);
	
	Q3VBoxLayout *framegrouplayout = new Q3VBoxLayout( framegroup->layout() );
	framegrouplayout->setAlignment( Qt::AlignTop );
	framegrouplayout->addWidget( label3 ); 
	framegrouplayout->addWidget( lineframe );
	framegrouplayout->addSpacing( 10 );
	framegrouplayout->addWidget( label4 );
	framegrouplayout->addWidget( borderframe );
	
	// preamble group 
	Q3GroupBox *preamblegroup = new Q3GroupBox( i18n("Preamble"),page);
	preamblegroup->setColumnLayout(0, Qt::Vertical );
	preamblegroup->layout()->setSpacing( 6 );
	preamblegroup->layout()->setMargin( 11 );

	m_cbPre = new QCheckBox(i18n(">{decl}: insert before"),preamblegroup,"cb_pre");
	m_cbPost = new QCheckBox(i18n("<{decl}: insert after"),preamblegroup,"cb_post");
	m_cbAt = new QCheckBox(i18n("@{decl}: suppress space"),preamblegroup,"cb_at");
	m_cbSep = new QCheckBox(i18n("!{decl}: do not suppress space"),preamblegroup,"cb_sep");
	
	Q3VBoxLayout *preamblegrouplayout = new Q3VBoxLayout( preamblegroup->layout() );
	preamblegrouplayout->setAlignment( Qt::AlignTop );
	preamblegrouplayout->addWidget( m_cbPre ); 
	preamblegrouplayout->addWidget( m_cbPost );
	preamblegrouplayout->addWidget( m_cbAt ); 
	preamblegrouplayout->addWidget( m_cbSep );
	
	// header group
	Q3GroupBox *headergroup = new Q3GroupBox( i18n("Alignment"),page);
	headergroup->setColumnLayout(0, Qt::Vertical );
	headergroup->layout()->setSpacing( 6 );
	headergroup->layout()->setMargin( 11 );

	m_coHeader = new QComboBox(headergroup);	
	
	Q3VBoxLayout *headergrouplayout = new Q3VBoxLayout( headergroup->layout() );
	headergrouplayout->setAlignment( Qt::AlignTop );
	headergrouplayout->addStretch(1); 
	headergrouplayout->addWidget(m_coHeader); 
	headergrouplayout->addStretch(1); 
	
	// add widgets
	if ( m_header ) 
	{
		grid->addWidget( headergroup,0,0 );
		grid->addWidget( preamblegroup,1,0 ); 
		grid->addWidget( colorgroup,2,0 );
		grid->addWidget( fontgroup, 0,1 );
		grid->addMultiCellWidget( framegroup,1,2,1,1 );
		aligngroup->hide();
	} 
	else 
	{
		grid->addWidget( fontgroup, 0,0 );
		grid->addWidget( aligngroup,1,0 ); 
		grid->addWidget( colorgroup,2,0 );
		grid->addMultiCellWidget( framegroup,0,2,1,1 );
		headergroup->hide();
		preamblegroup->hide();
	}
	grid->setRowStretch(3,1);
		
	// init some special widgets
	int buttonheight = m_pbFrame1->sizeHint().height();
	m_pbFrame1->setFixedSize(buttonheight,buttonheight); 
	m_pbFrame2->setFixedSize(buttonheight,buttonheight); 
	m_pbFrame3->setFixedSize(buttonheight,buttonheight); 
	m_pbFrame4->setFixedSize(buttonheight,buttonheight);  

	m_pbFrame1->setPixmap( QPixmap(const_cast<const char**>(no_border_xpm)) );
	m_pbFrame2->setPixmap( QPixmap(const_cast<const char**>(lr_border_xpm)) );
	m_pbFrame3->setPixmap( QPixmap(const_cast<const char**>(tb_border_xpm)) );
	m_pbFrame4->setPixmap( QPixmap(const_cast<const char**>(all_border_xpm)) );

	borderframe->setFixedWidth(lineframe->sizeHint().width());
	setButtonText(User1,"Re&set");
	
	if ( m_header ) 
	{
		m_preamblelist = alignlist;
		m_coHeader->insertStringList(m_preamblelist);
	}
	
	// init widget data
	if ( data )
		m_data = *data;
	else
		initWidgetData();	
	// init widgets
	initWidgets();

		
	// signals and slots
	connect(m_pbFrame1, SIGNAL(clicked()), this, SLOT(slotFramebuttonClicked()));
	connect(m_pbFrame2, SIGNAL(clicked()), this, SLOT(slotFramebuttonClicked()));
	connect(m_pbFrame3, SIGNAL(clicked()), this, SLOT(slotFramebuttonClicked()));
	connect(m_pbFrame4, SIGNAL(clicked()), this, SLOT(slotFramebuttonClicked()));
	connect(this, SIGNAL(user1Clicked()),this, SLOT(slotResetClicked()));
	if ( m_header ) 
	{
		connect(m_cbAt, SIGNAL(clicked()),this, SLOT(slotSeparatorClicked()));
		connect(m_cbSep,SIGNAL(clicked()),this, SLOT(slotSeparatorClicked()));
	}
	
	Q3WhatsThis::add(m_coHeader,i18n("Column or cell alignment."));
	Q3WhatsThis::add(m_cbBold,i18n("Set bold font series."));
	Q3WhatsThis::add(m_cbItalic,i18n("Set italic font shape."));
	Q3WhatsThis::add(m_rbAlignleft,i18n("The text will be aligned at the left border of the cell."));
	Q3WhatsThis::add(m_rbAligncenter,i18n("The text will be centered."));
	Q3WhatsThis::add(m_rbAlignright,i18n("The text will be aligned at the right border of the cell."));
	Q3WhatsThis::add(m_ccBgcolor,i18n("Choose a background color (needs color package)."));
	Q3WhatsThis::add(m_ccTextcolor,i18n("Choose a text color (needs color package)."));
	Q3WhatsThis::add(m_cbPre,i18n("Insert decl directly in front of the column entry."));
	Q3WhatsThis::add(m_cbPost,i18n("Insert decl right after the column entry."));
	Q3WhatsThis::add(m_cbAt,i18n("Suppresses inter-column space and inserts decl directly."));
	Q3WhatsThis::add(m_cbSep,i18n("Inserts decl, but does not suppress the normally inserted space between columns in contrast to @{decl}."));
	Q3WhatsThis::add(m_pbFrame1,i18n("Clear all border lines."));
	Q3WhatsThis::add(m_pbFrame2,i18n("Set left and right border lines."));
	Q3WhatsThis::add(m_pbFrame3,i18n("Set upper and lower border line."));
	Q3WhatsThis::add(m_pbFrame4,i18n("Set all border lines."));
	Q3WhatsThis::add(m_cellframe,i18n("Set user defined border lines. A mouse click into one of the four border ranges will set or clear this special border line."));
	
	setButtonWhatsThis(User1,i18n("Reset all settings to standard cell attributes: left alignment, normal font series and shape, white background color, black text color, no border lines."));
}

////////////////////////////// read data //////////////////////////////

TabularCell::Data TabCellDialog::data()
{
	if ( m_header ) 
	{
		switch ( m_coHeader->currentItem() ) 
		{
			case 1  : m_data.align = Qt::AlignHCenter; break;
			case 2  : m_data.align = Qt::AlignRight;   break;
			default : m_data.align = Qt::AlignLeft;
		}
	} 
	else 
	{
		if ( m_rbAlignright->isChecked() )
			m_data.align = Qt::AlignRight;
		else if ( m_rbAligncenter->isChecked() )
			m_data.align = Qt::AlignHCenter;
		else
			m_data.align = Qt::AlignLeft;
	}
		
	m_data.font = TabularCell::cfNormal;
	if ( m_cbBold->isChecked() )
		m_data.font |= TabularCell::cfBold;
	if ( m_cbItalic->isChecked() )
		m_data.font |= TabularCell::cfItalic;
		
	m_data.border = m_cellframe->border();
		
	m_data.bgcolor = m_ccBgcolor->color();
	m_data.textcolor = m_ccTextcolor->color();
	
	return m_data;
}

QString TabCellDialog::header()
{
	QString s;
	
	if ( m_header ) 
	{
		if ( m_cbAt->isChecked() )
			s += "@{} ";
		if ( m_cbSep->isChecked() )
			s += "!{} ";
		if ( m_cbPre->isChecked() )
			s += ">{} ";
			
		s += m_coHeader->currentText().replace("{w}","{}");
			
		if ( m_cbPost->isChecked() )
			s += " <{}";
	}
	
	return s.trimmed();
}

//////////////////// init widgets  /////////////////////

void TabCellDialog::initWidgetData()
{
	m_data.align = Qt::AlignLeft;	
	m_data.font = TabularCell::cfNormal;
	m_data.border = TabularCell::cbNone;
	m_data.bgcolor = QColor(Qt::white);
	m_data.textcolor = QColor(Qt::black);
}

void TabCellDialog::initWidgets()
{
	if ( m_data.align & Qt::AlignRight )
		m_rbAlignright->setChecked(true);
	else if ( m_data.align & Qt::AlignHCenter )
		m_rbAligncenter->setChecked(true);
	else
		m_rbAlignleft->setChecked(true);
		
	m_cbBold->setChecked( m_data.font & TabularCell::cfBold );
	m_cbItalic->setChecked( m_data.font & TabularCell::cfItalic );
		
	m_ccBgcolor->setColor(m_data.bgcolor);	
	m_ccTextcolor->setColor(m_data.textcolor);
	
	m_cellframe->setBorder(m_data.border);	
	
	if ( m_header ) 
	{
		QString s = m_headerlabel.remove(' ');
		
		bool state = ( s.indexOf("@{}") >= 0 );
		if ( state )
			s = s.remove("@{}");
		m_cbAt->setChecked(state);

		state = ( s.indexOf("!{}") >= 0 );
		if ( state )
			s = s.remove("!{}");
		m_cbSep->setChecked(state);

		state = ( s.indexOf(">{}") >= 0 );
		if ( state )
			s = s.remove(">{}");
		m_cbPre->setChecked(state);

		state = ( s.indexOf("<{}") >= 0 );
		if ( state )
			s = s.remove("<{}");
		m_cbPost->setChecked(state);
		
		int pos = m_preamblelist.findIndex(s);
		if ( pos < 0 )
			pos = 0;
		m_coHeader->setCurrentItem(pos);
	} 
}

//////////////////// the border of the frame changes  /////////////////////

void TabCellDialog::slotFramebuttonClicked()
{
	QString name = QString( sender()->name() ).section('_',1,1);
	
	int border = -1;
	if ( name == "no" )
		border = TabularCell::cbNone;  
	else if ( name == "lr" )
		border = TabularCell::cbLeft | TabularCell::cbRight;  
	else if ( name == "tb" )
		border = TabularCell::cbTop | TabularCell::cbBottom;  
	else if ( name == "all" )
		border = TabularCell::cbLeft | TabularCell::cbTop | TabularCell::cbBottom | TabularCell::cbRight;
	
	if ( border >= 0 )
		m_cellframe->setBorder(border);                         
}

void TabCellDialog::slotResetClicked()
{
	initWidgetData();
	m_headerlabel = "l";
	initWidgets();
}

void TabCellDialog::slotSeparatorClicked()
{
	QString checkboxname = sender()->name();
	if ( m_cbAt->isChecked() && checkboxname=="cb_at" )
		m_cbSep->setChecked(false);
	else if ( m_cbSep->isChecked() && checkboxname=="cb_sep" )
		m_cbAt->setChecked(false);
}

//////////////////////////////////////////////////////////////////////

//BEGIN TabularItem

TabularItem::TabularItem(Q3Table* table)
	: Q3TableItem(table,Q3TableItem::OnTyping,QString::null) 
{
	TabularTable *tab = dynamic_cast<TabularTable *>(table);
	m_data = tab->defaultAttributes();
}

TabularItem::TabularItem(Q3Table* table, const TabularCell::Data &data)
	: Q3TableItem(table,Q3TableItem::OnTyping,QString::null) 
{
	m_data = data;
}

int TabularItem::alignment() const  
{
	return m_data.align | Qt::AlignVCenter;
}

bool TabularItem::isMulticolumn()
{
	return ( colSpan() > 1 );
}

// paint the current cell and write the text with special font attributes
void TabularItem::paint(QPainter *p,const QColorGroup &cg,const QRect &cr,bool selected)
{
	p->fillRect(0,0,cr.width(),cr.height(),
		selected ? cg.brush(QColorGroup::Highlight) : QBrush(m_data.bgcolor));

	int w = cr.width();
	int h = cr.height();
 
	if ( selected )
		p->setPen( cg.highlightedText() );
	else
		p->setPen(m_data.textcolor);         
		
	if ( m_data.font ) 
	{
		QFont f( p->font() );
		if ( m_data.font & TabularCell::cfBold )
			f.setBold(true);
		if ( m_data.font & TabularCell::cfItalic )
			f.setItalic(true);
		p->setFont(f);
	}
		
	p->drawText( 2,0,w-4,h,alignment(), text() );
}

//END TabularItem

//////////////////////////////////////////////////////////////////////

TabularTable::TabularTable(int numRows,int numCols,QWidget* parent,TabularDialog *tabdialog)
	: Q3Table(numRows,numCols,parent), m_tabdialog(tabdialog)
{

	setSelectionMode(Q3Table::Single);
	setRowMovingEnabled(false);
	setColumnMovingEnabled(false);
	setSorting(false);
	 
	clearHorizontalHeader(0,numCols);
	clearVerticalHeader(0,numRows);
	
	// catch right mouse clicks in the table
	connect( this, SIGNAL(contextMenuRequested(int,int,const QPoint &)), 
	         this, SLOT(slotContextMenuClicked(int,int,const QPoint &)) );
			
	// catch right mouse clicks in horizontal/vertical header
	horizontalHeader()->installEventFilter(this);
	verticalHeader()->installEventFilter(this);
}

////////////////////////////////////////////////////////////

void TabularTable::clearHorizontalHeader(int col1,int col2)
{
	for ( int col=col1; col<col2; ++col )
		horizontalHeader()->setLabel(col,"l");	
}

void TabularTable::clearVerticalHeader(int row1,int row2)
{
	for ( int row=row1; row<row2; ++row )
		verticalHeader()->setLabel(row,"");	
}

bool TabularTable::isRowEmpty(int row)
{
	return isRowEmpty(row,0,numCols()-1);
}

bool TabularTable::isRowEmpty(int row,int col1, int col2)
{
	for ( int col=col1; col<=col2; ++col ) 
	{
		if ( updateCell(row,col) )            // check if there is an item
			return false;
	}
	return true;
}

bool TabularTable::isColEmpty(int col)
{
	for ( int row=0; row<numRows(); ++row ) 
	{
		if ( updateCell(row,col) )            // check if there is an item
			return false;
	}
	return true;
}

////////////////////////////// events //////////////////////////////

bool TabularTable::eventFilter(QObject *o, QEvent *e)
{
	if ( e->type() == QEvent::MouseButtonPress ) 
	{
		QMouseEvent *me = (QMouseEvent*) e;
		if ( me->button() == Qt::RightButton )
		{
			if ( o == horizontalHeader() ) 
			{
				mouseContextHorizontalHeader( me->pos().x() );
				return true;
			} 
			else if ( o == verticalHeader() ) 
			{
				mouseContextVerticalHeader( me->pos().y() );
				return true;
			}
		}
	}
   
	return Q3Table::eventFilter(o,e);
}

void TabularTable::mouseContextHorizontalHeader(int pos)
{
	//KILE_DEBUG() << "horizontal header: rechts: " << horizontalHeader()->sectionAt(pos) << endl;
	setupContextHeaderPopup(true,horizontalHeader()->sectionAt(pos));     
}

void TabularTable::mouseContextVerticalHeader(int pos)
{
	//KILE_DEBUG() << "vertical header: rechts: " << verticalHeader()->sectionAt(pos) << endl;
	setupContextHeaderPopup(false,verticalHeader()->sectionAt(pos));     
}

////////////////////////////// cell items //////////////////////////////

TabularCell::Data TabularTable::defaultAttributes()
{
	TabularCell::Data data;
	
	data.align = Qt::AlignLeft;	
	data.font = TabularCell::cfNormal;
	data.border = TabularCell::cbNone;
	data.bgcolor = QColor(Qt::white);
	data.textcolor = QColor(Qt::black);

	return data;
}	

bool TabularTable::isDefaultAttr(const TabularCell::Data &data)
{
	TabularCell::Data defaultdata	= defaultAttributes();

	return ( data.align == defaultdata.align           &&	
	         data.font == defaultdata.font             &&
	         data.border == defaultdata.border         &&
	         data.bgcolor == defaultdata.bgcolor       &&
	         data.textcolor == defaultdata.textcolor  
	       );
}

// We need a TabularItem for each cell with attributs. So we   
// always check first, if we have to create a new item.  
TabularItem *TabularTable::cellItem(int row,int col)
{
	Q3TableItem *cellitem = item(row,col);
	if ( ! cellitem ) 
	{
		TabularItem *newitem = new TabularItem(this);
		setItem(row,col,newitem);
		return newitem;
	} 
	else 
		return dynamic_cast<TabularItem*>(cellitem);
}

// Empty text is not written, if there is no QTableItem 
// or the current QTableItem has default attributes
void TabularTable::setText(int row,int col,const QString &text) 
{
	TabularItem *cellitem = cellItem(row,col);
	cellitem->setText(text);
	if ( text.isEmpty() )
		updateCell(row,col);
}

void TabularTable::setAlignment(int row,int col,int align)
{
	TabularItem *cellitem = cellItem(row,col);
	cellitem->m_data.align = align;
	if ( align == Qt::AlignLeft )
		updateCell(row,col);
}

void TabularTable::setColspan(int row,int col1,int col2,int numcols,const QString &text)
{
	//KILE_DEBUG() << "set colspan " << col1 << "-" << col2 << " "<< endl; 
	for ( int col=col1; col<=col2; ) 
	{
		TabularItem *olditem = dynamic_cast<TabularItem*>( item(row,col) );
		if ( olditem ) 
		{
			if ( col != col1 )
				delete olditem;
			col += olditem->colSpan();
		}
		else
		{
			col++;
		}
	}
	
	// only the leftmost item exists 
	TabularItem *cellitem = cellItem(row,col1);
	cellitem->setText(text);
	cellitem->setSpan(1,numcols);
}

bool TabularTable::isMulticolumn(int row,int col)
{
	TabularItem *cellitem = dynamic_cast<TabularItem*>( item(row,col) );
	return ( cellitem && cellitem->isMulticolumn() );
}

// Set new attributes. If all settings are default, delete QTableItem
void TabularTable::setAttributes(int row,int col, const TabularCell::Data &data)
{
	TabularItem *cellitem = dynamic_cast<TabularItem*>( item(row,col) );
	if ( cellitem ) 
		cellitem->m_data = data;
	else
		setItem(row,col,new TabularItem(this,data));
		
	updateCell(row,col);
}

// clear all current attributes
void TabularTable::clearAttributes(int row,int col)
{
	//KILE_DEBUG() << "clear attr (" << row << "/" << col << ")" << endl;
	TabularCell::Data data = defaultAttributes();	
	setAttributes(row,col,data);
}

////////////////////////////// update cell //////////////////////////////

// update text of current cell, which can be in edit mode

void TabularTable::updateCurrentCell()
{
	if ( editMode() != Q3Table::NotEditing )
		endEdit(currentRow(),currentColumn(),true,true);
}

// A cell is updated or is is asked, if there is still an entry. 
// Check if we need the QTableItem anymore. The result will be true, 
// if there is still a QTableItem, else false.
bool TabularTable::updateCell(int row,int col)
{
	TabularItem *cellitem = dynamic_cast<TabularItem*>( item(row,col) );
	if ( ! cellitem ) 
		return false;          // no item
		
	// there is an item
	if ( cellitem->text().isEmpty() && isDefaultAttr(cellitem->m_data) && !cellitem->isMulticolumn() ) 
	{ 
		delete cellitem;       // no text and standard attributes
		return false;          // no item anymore
	} 
			
	return true;          
}

// when editing is finished, we check, if the QTableItem is still needed
void TabularTable::endEdit(int row,int col,bool accept,bool replace)
{
	Q3Table::endEdit(row,col,accept,replace);
	//updateCell(row,col);       
}

////////////////////////////// cellrange attributes //////////////////////////////

// no attributes of multicoumn cells are changed

void TabularTable::clearHeaderCells(bool cleartext,bool clearattributes)
{
	if ( m_horizontal )
		clearCellrange(m_section,0,m_section,numRows()-1,cleartext,clearattributes);
	else
		clearCellrange(0,m_section,numCols()-1,m_section,cleartext,clearattributes);	
}

void TabularTable::clearSelectionCells(bool cleartext,bool clearattributes)
{
	int x1,y1,x2,y2;
	if ( getCurrentSelection(x1,y1,x2,y2) ) 
	{
		clearCellrange(x1,y1,x2,y2,cleartext,clearattributes);
	}
}

void TabularTable::clearCellrange(int x1,int y1,int x2,int y2,bool cleartext,bool clearattributes)
{
	bool singlecell = (x1==x2 && y1==y2);
	
	for ( int row=y1; row<=y2; ++row ) 
	{
		for ( int col=x1; col<=x2; ++col ) 
		{
			if ( cleartext )
				setText(row,col,QString::null);
				
			if ( clearattributes && (singlecell || !isMulticolumn(row,col)) ) 
				clearAttributes(row,col);
		}
	}
}

void TabularTable::setCellrangeAttributes(int x1,int y1,int x2,int y2,const TabularCell::Data &data)
{
	bool singlecell = (x1==x2 && y1==y2);
	//KILE_DEBUG() << x1 << " " << y1 << " "<< x2 << " "<< y2 << " " << data.align << endl;
	
	for ( int col=x1; col<=x2; ++col ) 
	{
		for ( int row=y1; row<=y2; ++row ) 
		{
			if ( singlecell || !isMulticolumn(row,col) ) 
				setAttributes(row,col,data);
		}
	}
}

void TabularTable::setCellrangeAlignment(int x1,int y1,int x2,int y2,int align)
{
	bool singlecell = (x1==x2 && y1==y2);
	
	for (int col=x1; col<=x2; ++col ) 
	{ 
		for (int row=y1; row<=y2; ++row ) 
		{ 
			if ( singlecell || !isMulticolumn(row,col) ) 
				setAlignment(row,col,align);
		}
	}
}

////////////////////////////// movement //////////////////////////////

// Move horizontal instead of vertical to the next cell,
// when pressing the enter key. If this happens in the 
// lower right cell, a new table line is inserted.
void TabularTable::activateNextCell()
{
	int row = currentRow();
	int col = currentColumn();
	
	col = ( col+1 ) % numCols();
	if ( col == 0 ) 
	{
		row++;
		if ( row == numRows() ) 
		{
			m_tabdialog->slotRowValueChanged(row+1);
		}
	}
	
	setCurrentCell(row,col);
}

////////////////////////////// paint //////////////////////////////

// Paint a table cell with all attributes, if there is a QTableItem
void TabularTable::paintCell( QPainter *p, int row, int col,
			const QRect &cr, bool selected, const QColorGroup &cg )
{
	//KILE_DEBUG() << "r=" << row << " c" << col<< endl;
	
	if ( selected && row == currentRow() && col == currentColumn() 
	              && ( hasFocus() || viewport()->hasFocus() ) )
		selected = false;

	int w = cr.width();
	int h = cr.height();
	int x2 = w - 1;
	int y2 = h - 1;

	TabularItem *cellitem = dynamic_cast<TabularItem*>( item(row,col) );
	if ( cellitem ) 
	{
		p->save();
		cellitem->paint( p, cg, cr, selected );
		p->restore();
	} 
	else 
	{
		p->fillRect( 0,0,w,h, selected ? cg.brush( QColorGroup::Highlight ) 
		                               : cg.brush( QColorGroup::Base ) );
	}

	// draw gridlines
	if ( showGrid() ) 
	{
		// save current pen
		QPen pen( p->pen() );
		
		QColor gridlinecolor;
		int gridColor = style()->styleHint(QStyle::SH_Table_GridLineColor);
		if (gridColor != -1) 
		{
			const QPalette &pal = palette();
			if ( cg != colorGroup() && cg != pal.disabled() && cg != pal.inactive() )
				gridlinecolor = cg.mid();             // p->setPen(cg.mid());
			else
				gridlinecolor = (QRgb)gridColor;      // p->setPen((QRgb)gridColor);
		} 
		else 
		{
			gridlinecolor = cg.mid();                // p->setPen(cg.mid());
		} 
		
		// draw border
		int colspan = 1;
		int border = 0;
		if ( cellitem  ) 
		{
			colspan = cellitem->colSpan();
			border = cellitem->m_data.border;
			
			// draw left border only in column 0
			if ( col==0 && (border & TabularCell::cbLeft) ) 
				p->drawLine( 0,0,0,y2 );
			
			// draw top border only in row 0
			if ( row==0 && (border & TabularCell::cbTop) )
				p->drawLine( 0,0,x2,0 );
		}	
		
		// at the bottom border of the cell either the gridline or the 
		// bottom border (or the top border of the cell below) is drawn
		bool drawborder;
		if ( border & TabularCell::cbBottom ) 
			drawborder = true;
		else 
		{
			TabularItem *below = dynamic_cast<TabularItem*>( item(row+1,col) );
			drawborder = ( below && (below->m_data.border & TabularCell::cbTop) );
		}
		p->setPen( (drawborder) ? Qt::black : gridlinecolor );
		p->drawLine( 0,y2,x2,y2 );
		
		// at the right border of the cell either the gridline or the 
		// right border (or the left border of the cell below) is drawn
		if ( border & TabularCell::cbRight ) 
			drawborder = true;
		else 
		{
			TabularItem *right = dynamic_cast<TabularItem*>( item(row,col+colspan) );
			drawborder = ( right && (right->m_data.border & TabularCell::cbLeft) );
		}
		p->setPen( (drawborder) ? Qt::black : gridlinecolor );
		p->drawLine( x2,0,x2,y2 );
		
		// restore pen
		p->setPen( pen );
	}
}

////////////////////////////// popup menus //////////////////////////////

Q3PopupMenu *TabularTable::createPopupMenu()
{
	Q3PopupMenu *menu = new Q3PopupMenu(this);
	menu->insertItem( i18n("Edit..."));
	menu->insertSeparator();
	
	return menu;
}

void TabularTable::insertPopupAlign(Q3PopupMenu *popup,bool header)
{
	// alignment 
	int align = 0;
	
	//calculate 
	if ( header && m_x1==m_x2 ) 
	{
		QString label = horizontalHeader()->label(m_x1);
		if ( label.indexOf('l') < 0 )
		align += 1;
		if ( label.indexOf('c') < 0 ) 
			align += 2;
		if ( label.indexOf('r') < 0 )
			align += 4;
	} else 
	{
		align = 7;
	}
	
	// insert 
	if ( align & 1 ) 
		popup->insertItem( i18n("Align Left"));
	if ( align & 2 ) 
		popup->insertItem( i18n("Align Center"));
	if ( align & 4 ) 
		popup->insertItem( i18n("Align Right"));
	if ( align > 0 )
		popup->insertSeparator();
}

void TabularTable::insertPopupClear(Q3PopupMenu *popup)
{
	popup->insertItem( i18n("Clear Text"));
	popup->insertItem( i18n("Clear Attributes"));
	popup->insertItem( i18n("Clear All"));
}

int TabularTable::popupId(Q3PopupMenu *popup, int id)
{
	QString entry = popup->text(id);
	
	int result = PopupNone;
	if ( entry == i18n("Edit...") )
		result = PopupEdit;
	else if ( entry == i18n("Set Multicolumn") )
		result = PopupSet;
	else if ( entry == i18n("Break Multicolumn") )
		result = PopupBreak;
	else if ( entry == i18n("Align Left") )
		result = PopupLeft;
	else if ( entry == i18n("Align Center") )
		result = PopupCenter;
	else if ( entry == i18n("Align Right") )
		result = PopupRight;
	else if ( entry == i18n("Clear Text") )
		result = PopupText;
	else if ( entry == i18n("Clear Attributes") )
		result = PopupAttributes;
	else if ( entry == i18n("Clear All") )
		result = PopupAll;

	return result;
}

////////////////////////////// cellrange: right mouse button //////////////////////////////

void TabularTable::slotContextMenuClicked(int row,int col,const QPoint &)
{
	//KILE_DEBUG() << "context menu clicked" << endl;
	if ( row<0 || col<0 )
		return;

	if ( ! getCurrentSelection(m_x1,m_y1,m_x2,m_y2) )
		return;

	// create popup menu
	m_cellpopup = createPopupMenu();
	
	// multicolumns
	if ( m_y1 == m_y2 ) 
	{
		if ( m_x1 == m_x2 ) 
		{
			TabularItem *cellitem = dynamic_cast<TabularItem*>( item(m_y1,m_x1) );
			if ( cellitem && cellitem->isMulticolumn() ) 
			{
				m_cellpopup->insertItem( i18n("Break Multicolumn"));
				m_cellpopup->insertSeparator();
			}
		} 
		else if ( m_x2 > m_x1 ) 
		{
			TabularItem *cellitem1 = dynamic_cast<TabularItem*>( item(m_y1,m_x1) );
			TabularItem *cellitem2 = dynamic_cast<TabularItem*>( item(m_y2,m_x2) );
			if ( (!cellitem1 && !cellitem2) || (cellitem1!=cellitem2) )
			{
				m_cellpopup->insertItem( i18n("Set Multicolumn"));
				m_cellpopup->insertSeparator();
			}
		}
	}
	
	insertPopupAlign(m_cellpopup,false);
	insertPopupClear(m_cellpopup);
	
	connect(m_cellpopup,SIGNAL(activated(int)),this,SLOT(slotCellPopupActivated(int)));
	m_cellpopup->exec( QCursor::pos() );
	clearSelection();
}


void TabularTable::slotCellPopupActivated(int id)
{
	switch ( popupId(m_cellpopup,id) ) 
	{
		case PopupEdit       : cellPopupEdit();                  break;
		case PopupSet        : cellPopupSetMulticolumn();        break;
		case PopupBreak      : cellPopupBreakMulticolumn();      break;
		case PopupLeft       : cellPopupAlign(Qt::AlignLeft);    break;
		case PopupCenter     : cellPopupAlign(Qt::AlignHCenter); break;
		case PopupRight      : cellPopupAlign(Qt::AlignRight);   break;
		case PopupText       : clearSelectionCells(true,false);  break;
		case PopupAttributes : clearSelectionCells(false,true);  break;
		case PopupAll        : clearSelectionCells(true,true);   break;
	}
}

void TabularTable::cellPopupEdit()
{
	//KILE_DEBUG() << "cellPopupEdit" << endl;
		
	// default edit mode for a range: no data and no header 
	TabularCell::Data *pdata = 0;
	
	// if there one single cell, we use the attributes if they exist
	if ( m_x1==m_x2 && m_y1==m_y2 ) 
	{
		TabularItem *cellitem = dynamic_cast<TabularItem*>( item(m_y1,m_x1) );
		if ( cellitem ) 
			pdata = &(cellitem->m_data);
	}
	
	//KILE_DEBUG() << "sel " << x1 << " " << y1 << " " << x2 << " " << y2 << endl;
	cellParameterDialog(m_x1,m_y1,m_x2,m_y2,pdata,QString::null);
}

void TabularTable::cellPopupSetMulticolumn()
{
	//KILE_DEBUG() << "slotContextMenuSetMulticolumn" << endl;
	//KILE_DEBUG() << "set mc " << m_x1 << " " << m_y1 << "   " << m_x2 << " " << m_y2 << endl;

	if ( m_y1==m_y2 && m_x2>m_x1) 
	{
		// get full cell range including possible multicolumn cells
		int xl,xr;
		getCellRange(m_y1,m_x1,m_x2,xl,xr);

		QString s = getCellRangeText(m_y1,xl,xr);
		if ( ! s.isEmpty() ) 
		{
			QString message  = i18n("Concat all text to the new multicolumn cell?");
			if ( KMessageBox::questionYesNo(this,message,i18n("Save Text")) != KMessageBox::Yes )
				s = QString::null;
		}
		setColspan(m_y1,xl,xr,xr-xl+1,s);
		// update();
	}
}

void TabularTable::cellPopupBreakMulticolumn()
{
	//KILE_DEBUG() << "slotContextMenuBreakMulticolumn" << endl;
	//KILE_DEBUG() << "set mc " << m_x1 << " " << m_y1 << " " << m_x2 << " " << m_y2 << endl;
	
	if ( m_x1==m_x2 && m_y1==m_y2 ) 
	{
		// get full cell range including possible multicolumn cells
		int xl,xr;
		getCellRange(m_y1,m_x1,m_x2,xl,xr);

		QString s = getCellRangeText(m_y1,xl,xr);
		if ( ! s.isEmpty() ) 
		{
			if ( KMessageBox::questionYesNo(this,
			               i18n("Transfer text and all attributes of the multicolumn cell to the leftmost of the separated cell?"),
			               i18n("Shrink Multicolumn")) != KMessageBox::Yes )
				s = QString::null;
		}

		setColspan(m_y1,xl,xr,1,s);
	}
}

void TabularTable::cellPopupAlign(int align)
{
	setCellrangeAlignment(m_x1,m_y1,m_x2,m_y2,align);
}

// get real cell range including possible multicolumn cells
void TabularTable::getCellRange(int row,int col1, int col2, int &xl, int &xr)
{
	// search the first cell of this range
	xl = col1;
	TabularItem *cellitem1 = dynamic_cast<TabularItem*>( item(row,col1) );
	if ( cellitem1 && cellitem1->isMulticolumn() )
	{
		for ( int col=col1-1; col>=0; --col )
		{
			TabularItem *cellitem = dynamic_cast<TabularItem*>( item(row,col) );
			if ( cellitem == cellitem1 )
				xl = col;
			else
				break;
		} 
	}

	// search the last cell of this range
	xr = col2;
	TabularItem *cellitem2 = dynamic_cast<TabularItem*>( item(row,col2) );
	if ( cellitem2 && cellitem2->isMulticolumn() )
	{
		for ( int col=col2+1; col<numCols(); ++col )
		{
			TabularItem *cellitem = dynamic_cast<TabularItem*>( item(row,col) );
			if ( cellitem == cellitem2 )
				xr = col;
			else
				break;
		} 
	}
}

// get text from real cell range including possible multicolumn cells
QString TabularTable::getCellRangeText(int row,int col1, int col2)
{
	QString s;
	for ( int col=col1; col<=col2; ) 
	{
		TabularItem *cellitem = dynamic_cast<TabularItem*>( item(row,col) );
		if ( cellitem ) 
		{
			s += cellitem->text();
			col += cellitem->colSpan();
		}
		else
		{
			col++;
		}
	}
	return s;
}

////////////////////////////// header: right mouse button //////////////////////////////

void TabularTable::setupContextHeaderPopup(bool horizontal, int section)
{
	//KILE_DEBUG() << "setupContextHeaderPopup" << endl;
	
	// save header parametr
	m_horizontal = horizontal;
	m_section = section;
	
   // we always define a selection 
	bool selection = getCurrentSelection(m_x1,m_y1,m_x2,m_y2);
	if ( ! selection ) 
	{
		if ( m_horizontal )
		{
			m_x1 = m_x2 = m_section;
			m_y1 = 0;
			m_y2 = numRows() - 1; 
		} 
		else 
		{
			m_x1 = 0;
			m_x2 = numCols() - 1;
			m_y1 = m_y2 = m_section;
		}
	}
	
	// create popup menu
	m_headerpopup = createPopupMenu();
	insertPopupAlign(m_headerpopup,m_horizontal);
	insertPopupClear(m_headerpopup);
	
	connect(m_headerpopup,SIGNAL(activated(int)),this,SLOT(slotHeaderPopupActivated(int)));
	m_headerpopup->exec( QCursor::pos() );
}

void TabularTable::slotHeaderPopupActivated(int id)
{
	switch ( popupId(m_headerpopup,id) ) 
	{
		case PopupEdit       : headerPopupEdit();            break;
		case PopupSet        : break;
		case PopupBreak      : break;
		case PopupLeft       : headerPopupAlign('l');        break;
		case PopupCenter     : headerPopupAlign('c');        break;
		case PopupRight      : headerPopupAlign('r');        break;
		case PopupText       : clearHeaderCells(true,false); break;
		case PopupAttributes : clearHeaderCells(false,true); break;
		case PopupAll        : clearHeaderCells(true,true);  break;
	}
}

void TabularTable::headerPopupEdit()
{
	//KILE_DEBUG() << "HeaderPopupEdit" << endl;
	
	if ( m_horizontal )
	{
		// default header label
		QString label = horizontalHeader()->label(m_section);
			
		// look if all labels are equal
		for ( int col=m_x1; col<=m_x2; ++col ) 
		{
			if ( label != horizontalHeader()->label(col) ) 
			{
				label = "l";
				break;
			}
		}
		
		// call parameter dialog
		//KILE_DEBUG() << "col1=" << col1 << " col2=" << col2 << " " << label << endl; 
		cellParameterDialog( m_x1,0,m_x2,numRows()-1,0,label );
	}
	else
		cellParameterDialog(0,m_y1,numCols()-1,m_y2,0,QString::null);
	//	cellParameterDialog(0,m_section,numCols()-1,m_section,0,QString::null);
}

void TabularTable::headerPopupAlign(QChar alignchar)
{
	int align;
#ifdef __GNUC__
#warning Get rid of the QChar conversion at line 1478!
#endif
	switch ( alignchar.toLatin1() )
	{
		case 'c' : align = Qt::AlignHCenter; break;
		case 'r' : align = Qt::AlignRight;   break;
		default  : align = Qt::AlignLeft;
	}
	setCellrangeAlignment(m_x1,m_y1,m_x2,m_y2,align);
	if ( m_horizontal )
		updateHeaderAlignment(m_x1,m_x2,alignchar);
}

// adjust headerlabel: lcr
void TabularTable::updateHeaderAlignment(int col1,int col2,QChar alignchar)
{
	QStringList list = m_tabdialog->columnAlignments();
	
	for ( int col=col1; col<=col2; ++col ) 
	{
		QString label = horizontalHeader()->label(col);
		for ( uint i=0; i<list.count(); ++i ) 
		{
			if ( label.indexOf(list[i]) >= 0 ) 
			{
				horizontalHeader()->setLabel( col,label.replace(list[i],alignchar) );
				break;
			}
		}
	}
}

////////////////////////////// selection //////////////////////////////

bool TabularTable::getCurrentSelection(int &x1,int &y1,int &x2,int &y2)
{
	// look if there is a selection
	int nr = currentSelection();
	if ( nr >= 0 ) 
	{
		// get parameter of current selection
		Q3TableSelection sel = selection(nr);
		x1 = sel.leftCol();
		y1 = sel.topRow();
		x2 = sel.rightCol();
		y2 = sel.bottomRow();
		return true;
	} 
	else 
		return false;
}


////////////////////////////// dialog for cell parameters //////////////////////////////

// cell parameter dialog for a range of cells or a single cell
// - single cell: current attributes are shown
// - cellrange: attributes are shown, if they are all equal
void TabularTable::cellParameterDialog(int x1,int y1,int x2,int y2, TabularCell::Data *data,
                                       const QString &headerlabel)
{
	//KILE_DEBUG() << "selection " << x1 << " " << y1 << " " << x2 << " " << y2 << endl;
	
	// if no settings are given, we should test all cells int the range if 
	// they are defined and have the same values
	if ( ! data ) 
	{ 
		TabularCell::Data defaultdata = defaultAttributes();
		
		// look if there is a QTableItem in the upper left cell
		TabularItem *cellitem = dynamic_cast<TabularItem*>( item(y1,x1) );
		
		// Now test the range. If the result is true, cellitem must be defined
		if ( equalParameter(x1,y1,x2,y2,DataAlign) ) 
			defaultdata.align = cellitem->m_data.align;	
		if ( equalParameter(x1,y1,x2,y2,DataFont) ) 
			defaultdata.font = cellitem->m_data.font;
		if ( equalParameter(x1,y1,x2,y2,DataBorder) ) 
			defaultdata.border = cellitem->m_data.border;
		if ( equalParameter(x1,y1,x2,y2,DataBgcolor) ) 
			defaultdata.bgcolor = cellitem->m_data.bgcolor;
		if ( equalParameter(x1,y1,x2,y2,DataTextcolor) )
			defaultdata.textcolor = cellitem->m_data.textcolor;
		data = &defaultdata; 
	}
	enum { DataAlign, DataFont, DataBorder, DataBgcolor, DataTextcolor };

	KileDialog::TabCellDialog *dlg = new KileDialog::TabCellDialog(this,data,
	                                        headerlabel,m_tabdialog->columnAlignments());
	if ( dlg->exec() ) 
	{
		// set attributes
		setCellrangeAttributes(x1,y1,x2,y2,dlg->data());
		
		// adjust header
		for ( int col=x1; col<=x2; ++col ) 
		{
			if ( ! headerlabel.isEmpty()  ) 
			{
				//KILE_DEBUG() << "header col=" << col << " " << dlg->header() << endl; 
				horizontalHeader()->setLabel(col,dlg->header());
			}
		}
		clearSelection();
	}
	delete dlg;
}

bool TabularTable::equalParameter(int x1,int y1,int x2,int y2, int code)
{
	// no QTableItem, so we should take default values
	TabularItem *upperleft = dynamic_cast<TabularItem*>( item(y1,x1) );
	if ( ! upperleft ) 
		return false;

	// get attributes from upper left cell
	TabularCell::Data data;
	data = upperleft->m_data;

	// look if all cells in this range have the same attributes
	for ( int row=y1; row<=y2; ++row ) 
	{
		for ( int col=x1; col<=x2; ++col ) 
		{
			TabularItem *cellitem = dynamic_cast<TabularItem*>( item(row,col) );
			if ( ! cellitem ) 
				return false;
				
			switch ( code ) 
			{
				case DataAlign: if ( cellitem->m_data.align != data.align ) return false;
					  break;
				case DataFont: if ( cellitem->m_data.font != data.font ) return false;
					  break;
				case DataBorder: if ( cellitem->m_data.border != data.border ) return false;
					  break;
				case DataBgcolor: if ( cellitem->m_data.bgcolor != data.bgcolor ) return false;
					  break;
				case DataTextcolor: if ( cellitem->m_data.textcolor != data.textcolor ) return false;
					  break;
				default: return false;
			}
		}
	}
	
	// yes, unbelievable but true
	return true;
}

// test, if this cell has a vline at the left or right border)
bool TabularTable::isVLine(int row,int col, bool left)
{
	TabularItem *cellitem = dynamic_cast<TabularItem*>( item(row,col) );
	if ( !cellitem ) 
		return false;

	bool vlinefound = false;
	if ( left ) 
	{
		// look at the left border
		if ( cellitem->m_data.border & TabularCell::cbLeft )
			return true;	
		// look also at the right border of the left neighbour
		if ( col > 0 ) 
		{
			TabularItem *left = dynamic_cast<TabularItem*>( item(row,col-1) );
			vlinefound = ( left && (left->m_data.border & TabularCell::cbRight) );
		}
	} 
	else 
	{
		// look at the right border
		if ( cellitem->m_data.border & TabularCell::cbRight )
			return true;
		// look also at the left border of the right neighbour
		if ( col < numCols()-1 ) 
		{
			TabularItem *left = dynamic_cast<TabularItem*>( item(row,col-1) );
			vlinefound = ( left && (left->m_data.border & TabularCell::cbRight) );
		}
	}

	return vlinefound;
}

// Count vertical lines on the left side. If there is none, 
// we also check the right border of the left neighbour.
TabularCell::CountLines TabularTable::countVLines(int col, bool left)
{
	TabularCell::CountLines count;
	count.cnt = 0;
	count.cells = 0;
	count.list.clear();
	
	for ( int row=0; row<numRows(); ++row ) 
	{
		TabularItem *cellitem = dynamic_cast<TabularItem*>( item(row,col) );	
		if ( cellitem ) 
		{   
			if ( cellitem->isMulticolumn() )       // ignore multicolumn cells
				continue;
				
			//KILE_DEBUG() << "count vlines " << col << " row=" << row << " found" << endl;
			count.cells++;
			if ( isVLine(row,col,left) )
				count.cnt++;	
		} 
		else if ( col>0 && isVLine(row,col,false) ) 
		{
			count.cnt++;	
		}
	}
	
	return count;
}

// looking at to bottom of the line (or the top of the next line)
TabularCell::CountLines TabularTable::countHLines(int row, bool top)
{
	bool neighbour;
	bool hline;
	
	TabularCell::CountLines count;
	count.cnt = 0;
	count.cells = numCols();
	count.list.clear();
	
	int linestate = false;
	for ( int col=0; col<numCols(); ++col ) 
	{
		hline = false;
		
		TabularItem *cellitem = dynamic_cast<TabularItem*>( item(row,col) );
		neighbour = ( cellitem ) ? false : true;
		if ( cellitem ) 
		{    
			if ( top ) 
			{
				if ( cellitem->m_data.border & TabularCell::cbTop ) 
					hline = true;
			} 
			else 
			{
				if ( cellitem->m_data.border & TabularCell::cbBottom )
					hline = true;
				else 
					neighbour = true;
			}
		} 
		
		if ( neighbour ) 
		{
			TabularItem *below = dynamic_cast<TabularItem*>( item(row+1,col) );
			if( below && (below->m_data.border & TabularCell::cbTop) )
				hline = true;
		}
		
		// update counter and list of hline cells
		if ( hline ) 
		{
			if ( ! linestate ) 
			{
				count.list.append(col);
				linestate = true;
			}
			count.cnt++;
		} 
		else 
		{
			if ( linestate ) 
			{
				count.list.append(col-1);
				linestate = false;
			}
		}
	}
	if ( linestate )
		count.list.append(numCols()-1);
		
	return count;
}


TabularCell::Count TabularTable::countCells(int x1,int y1,int x2,int y2)
{
	//KILE_DEBUG() << "count font,colors,textcolors" << endl;
	
	QMap<QString,int> colors;
	QMap<QString,int> textcolors;
	QMap<QString,int>::iterator it, itw, itb;
	
	QString whitename = QColor(Qt::white).name();
	colors[whitename] = 0;	
	itw = colors.find(whitename);
	
	QString blackname = QColor(Qt::black).name();
	textcolors[blackname] = 0;	
	itb = textcolors.find(blackname);

	// initialize result
	TabularCell::Count count = { 0,0,0,0,0, whitename,blackname };
	
	// although it looks like a range, it is simply a row or a column,
	// because either x1=x2 or y1=y2
	for ( int row=y1; row<=y2; ++row ) 
	{
		for ( int col=x1; col<=x2; ++col ) 
		{
			TabularItem *cellitem = dynamic_cast<TabularItem*>( item(row,col) );
			if ( cellitem ) 
			{
				if ( cellitem->isMulticolumn() )       // ignore multicolumn cells
					continue;
				
				// check bold 	
				if ( cellitem->m_data.font & TabularCell::cfBold )
					count.bold++;
					
				// check italic
				if ( cellitem->m_data.font & TabularCell::cfItalic )
					count.italic++;
					
				// check backgroundcolor
				QString name = cellitem->m_data.bgcolor.name();
				it = colors.find(name);
				if ( it != colors.end() )
					(*it)++;
				else
					colors[name] = 1;
					
				// check textcolor
				name = cellitem->m_data.textcolor.name();
				it = textcolors.find(name);
				if ( it != textcolors.end() )
					(*it)++;
				else
					textcolors[name] = 1;
				
			}
			else  
			{
				(*itw)++;
				(*itb)++;
			}
			count.cells++;         // total number of cells without multicolumns
		}
	}
	
	// determine maximum result from dictionary for backgroundcolors
	for ( it=colors.begin(); it!=colors.end(); ++it) 
	{
		if ( it.data() > count.bgcolor ) 
		{
			count.bgcolor = it.data();
			count.nameBgcolor = it.key();
		}
	}
	
	// determine maximum result from dictionary for textcolors
	for ( it=textcolors.begin(); it!=textcolors.end(); ++it) 
	{
		if ( it.data() > count.textcolor ) 
		{
			count.textcolor = it.data();
			count.nameTextcolor = it.key();
		}
	}
	
	return count;
}


////////////////////////////// check parameter //////////////////////////////

//END TabularTable

//////////////////////////////////////////////////////////////////////

//BEGIN TabularDialog

TabularDialog::TabularDialog(QWidget *parent, KConfig *config, KileDocument::LatexCommands *commands, bool tabularenv) 
	: Wizard(config,parent), m_latexCommands(commands)
{
	QWidget *page = new QWidget(this);
	setMainWidget(page);
	setCaption(i18n("Tabular Environments"));
	
	Q3VBoxLayout *vbox = new Q3VBoxLayout( page, 6,6);
	
	// Table
	m_table = new TabularTable(3,3,page,this);

	// remark
	QLabel *labelRemark = new QLabel( i18n("<center>Hint: You can set some cell properties with a right mouse click.</center>") ,page);

	// environment group
	Q3ButtonGroup *group = new Q3ButtonGroup( i18n("Environment"),page);
	group->setColumnLayout(0, Qt::Vertical );
	group->layout()->setSpacing( 4 );
	group->layout()->setMargin( 11 );

	QLabel *label1 = new QLabel(i18n("&Name:"),group);
	QLabel *label2 = new QLabel(i18n("&Parameter:"),group);
	QLabel *label3 = new QLabel(i18n("Number of &rows:"),group);
	QLabel *label4 = new QLabel(i18n("Number of c&ols:"),group);
	m_coEnvironment = new QComboBox(group);	
	m_coParameter = new QComboBox(group);	
	m_spRows = new QSpinBox(1,99,1,group);
	m_spCols = new QSpinBox(1,49,1,group);
	m_cbWarning = new QCheckBox(i18n("&Delete also non empty rows or columns, but ask"),group);
	m_cbBooktabs = new QCheckBox(i18n("Use boo&ktabs package"),group);
	m_cbStarred = new QCheckBox(i18n("Use starred &version"),group);
	m_cbCenter = new QCheckBox(i18n("C&enter"),group);
	m_cbBullets = new QCheckBox(i18n("Insert &bullets"),group);
	
	Q3GridLayout *grouplayout = new Q3GridLayout( group->layout() );
	grouplayout->setAlignment( Qt::AlignTop );
	grouplayout->setColStretch(5,1);
	grouplayout->addWidget( label1, 0,0 ); 
	grouplayout->addWidget( label2, 1,0 );
	grouplayout->addWidget( m_coEnvironment, 0,1 );
	grouplayout->addWidget( m_coParameter, 1,1 );
	grouplayout->setColSpacing(2,24);
	grouplayout->addWidget( label3, 0,3 );
	grouplayout->addWidget( label4, 1,3 );
	grouplayout->addWidget( m_spRows, 0,4 );
	grouplayout->addWidget( m_spCols, 1,4 );
	grouplayout->setRowSpacing(2,8);
	
	grouplayout->addMultiCellWidget( m_cbWarning, 3,3,0,4, Qt::AlignLeft );
	
	grouplayout->addMultiCellWidget( m_cbStarred, 4,4,0,1, Qt::AlignLeft );	
	grouplayout->addMultiCellWidget( m_cbBooktabs, 5,5,0,1, Qt::AlignLeft );	
	grouplayout->addMultiCellWidget( m_cbCenter, 4,4,3,4, Qt::AlignLeft );
	grouplayout->addMultiCellWidget( m_cbBullets, 5,5,3,4, Qt::AlignLeft );
	
	// add widgets
	vbox->addWidget( m_table);
	vbox->addSpacing(4);
	vbox->addWidget(labelRemark);
	vbox->addSpacing(4);
	vbox->addWidget( group );
	
	label1->setBuddy(m_coEnvironment);
	label2->setBuddy(m_coParameter);
	label3->setBuddy(m_spRows);
	label4->setBuddy(m_spCols);

	// init widgets
	m_table->setMinimumHeight( m_table->sizeHint().height()-3 );
	m_spRows->setValue(3);
	m_spCols->setValue(3);
	m_cbCenter->setChecked(true);
	m_cbBullets->setChecked(true);
		
	// init all environments
	initEnvironments(tabularenv);
		
	// remember current values
	m_rows = m_spRows->value();
	m_cols = m_spCols->value();
	
	m_table->setFocus();
	resize(sizeHint().width(),sizeHint().height()+50);
	
	// signals and slots
	connect( m_coEnvironment, SIGNAL(activated(const QString &)), 
	         this, SLOT(slotEnvironmentChanged(const QString &)));
	connect( m_spRows, SIGNAL(valueChanged(int)), this, SLOT(slotRowValueChanged(int)));
	connect( m_spCols, SIGNAL(valueChanged(int)), this, SLOT(slotColValueChanged(int)));

	Q3WhatsThis::add(m_table,i18n("Input data. When you press Enter, the cursor will move to the cell right of the current cell. A click with the right mouse button on a cell or a range of cells will open a popup menu, where you can edit attributes, clear attributes, delete text or define multicolumn cells."));
	Q3WhatsThis::add(m_table->horizontalHeader(),i18n("Current layout of the preamble. A click with the right mouse button will open a popup menu, where you can edit some attributes of all cells, which belong to the selected columns."));
	Q3WhatsThis::add(m_table->verticalHeader(),i18n("A click with the right mouse button will open a popup menu, where you can edit some attributes of all cells, which belong to the selected rows."));
	Q3WhatsThis::add(m_coEnvironment,i18n("Choose an environment."));
	Q3WhatsThis::add(m_coParameter,i18n("Optional parameter for the chosen environment."));
	Q3WhatsThis::add(m_spRows,i18n("Choose the number of table rows."));
	Q3WhatsThis::add(m_spCols,i18n("Choose the number of table columns."));
	Q3WhatsThis::add(m_cbWarning,i18n("If you want, you will be asked before a non empty row a column is deleted."));
	Q3WhatsThis::add(m_cbCenter,i18n("The tabular will be centered."));
	Q3WhatsThis::add(m_cbBooktabs,i18n("Use line commands of the booktabs package."));
	Q3WhatsThis::add(m_cbStarred,i18n("Use the starred version of this environment."));
	Q3WhatsThis::add(m_cbBullets,i18n("Insert bullets in each cell. Alt+Ctrl+Right and Alt+Ctrl+Left will move very quick from one cell to another."));

}

void TabularDialog::initEnvironments(bool tabularenv)
{
	// read all tabular environments and insert them into the combobox
	QStringList list;
	QStringList::ConstIterator it;
	m_latexCommands->commandList(list,KileDocument::CmdAttrTabular,false);
	for ( it=list.begin(); it != list.end(); ++it ) 
	{
		m_coEnvironment->insertItem(*it);
	}
	
	// initialize first environment
	if ( tabularenv )
		m_coEnvironment->setCurrentText("tabular");
	else
		m_coEnvironment->setCurrentText("array");
	
	// initialize
	slotEnvironmentChanged( m_coEnvironment->currentText() );
}

////////////////////////////// table changed//////////////////////////////

void TabularDialog::slotEnvironmentChanged(const QString &env)
{
	//KILE_DEBUG() << "env changed " << env << endl;
	
	// clear parameter combobox
	m_coParameter->clear();
	m_coParameter->setEnabled(false);
	
	// look for environment parameter in dictionary
	KileDocument::LatexCmdAttributes attr;
	if ( m_latexCommands->commandAttributes(env,attr) ) 
	{
		// starred version
		m_cbStarred->setEnabled( attr.starred );
		
		// option
		if ( attr.option.indexOf('[') == 0 ) 
		{
			QStringList optionlist = attr.option.split("");
			if ( optionlist.count() > 2 ) 
			{
				// ok, let's enable it
				m_coParameter->setEnabled(true);
				m_coParameter->insertItem(QString::null);
				// insert some options
				for ( uint i=1; i<optionlist.count()-1; ++i ) 
					m_coParameter->insertItem(optionlist[i]);
			}
		}
	}
		
	m_alignlist.clear();
	m_alignlist << "l" << "c" << "r" << "p{w}" << "m{w}" << "b{w}";
	if ( env=="tabularx" || env=="xtabular")
		m_alignlist << "X";
}

bool TabularDialog::isMathmodeEnvironment(const QString &env)
{
	return m_latexCommands->isMathModeEnv(env);
}

void TabularDialog::slotRowValueChanged(int value)
{
	//KILE_DEBUG() << "row value changed " << value << endl;
	
	bool askBeforeDelete = m_cbWarning->isChecked();
	bool firstwarning = true;
	
	if ( value < m_rows )                   // problems may only happen when decreasing
	{                                             
		int testvalue = value;                 
		value = m_rows;  
		for ( int row=m_rows-1; row>=testvalue; row-- ) 
		{
			if ( m_table->isRowEmpty(row) )
			{
				value = row;
			}
			else
			{	
				if ( ! askBeforeDelete ) break;
				if ( firstwarning ) 
				{
					QString message  = i18n("Do you want to delete this row?");
					if (KMessageBox::warningContinueCancel(this, message, i18n("Delete"))!=KMessageBox::Continue) 
						break;
					firstwarning = false;
				}
				value = row;
			}
		}
	}
	m_spRows->setValue(value);      // perhaps corrected
	
	m_table->setNumRows(value);
	if ( value > m_rows )
		m_table->clearVerticalHeader(m_rows,value);
	m_rows = value;		
}

void TabularDialog::slotColValueChanged(int value)
{
	bool askBeforeDelete = m_cbWarning->isChecked();
	bool firstwarning = true;
	
	if ( value < m_cols )                   // problems may only happen when decreasing
	{            
		int testvalue = value;
		value = m_cols;
		for ( int col=m_cols-1; col>=testvalue; col-- ) 
		{
			if ( m_table->isColEmpty(col) )
				value = col;
			else
			{	
				if ( ! askBeforeDelete ) break;
				if ( firstwarning ) 
				{
					QString message  = i18n("Do you want to delete this column?");
					if (KMessageBox::warningContinueCancel(this, message, i18n("Delete"))!=KMessageBox::Continue) 
						break;
					firstwarning = false;
				}
				value = col;
			}
		}
		m_spCols->setValue(value);      // perhaps corrected
	}
	
	m_table->setNumCols(value);
	if ( value > m_cols )
		m_table->clearHorizontalHeader(m_cols,value);
	m_cols = value;
	
}

QStringList TabularDialog::columnAlignments()
{
	return m_alignlist;
}

////////////////////////////// color management//////////////////////////////

char TabularDialog::defineColor(const QString &name, QMap<QString,char> &colors, char &colorcode)
{
	if ( colorcode == '?' )
		return '?';

	// look for current color
	QMap<QString,char>::iterator it;
	it = colors.find(name);
	if ( it != colors.end() ) 
		return (*it);
		
	// not found: insert into color dictionary
	colors[name] = colorcode;
	if ( colorcode != 'W' )
		colorcode++;
	else
		colorcode = '?';
	return colors[name];
}

QString TabularDialog::convertColor(int value)
{
	if ( value == 0 )
		return "0";
	else if ( value == 255 )
		return "1";
		
	QString s;
	s = s.setNum( (float)value/256.0,'f',4);
	while ( s.right(1) == "0" )
		s.truncate( s.length()-1 );
		
	return s;
}

QStringList TabularDialog::sortColorTable(QMap<QString,char> &colors)
{
	QMap<QString,char>::const_iterator it;
	QStringList list;
	
	int r,g,b;
	QColor c;
	QString s,sred,sgreen,sblue;
	for ( it=colors.begin(); it!=colors.end(); ++it ) 
	{
		c.setNamedColor(it.key());
		c.getRgb(&r,&g,&b);
		if ( r!=g || r!=b ) 
		{
			sred = convertColor(r);
			sgreen = convertColor(g);
			sblue = convertColor(b);
			s = QString("{rgb}{%1,%2,%3}").arg(sred).arg(sgreen).arg(sblue);
		} 
		else 
		{
			s = QString("{gray}{%1}").arg(convertColor(r));
		}
		list << QString("\\definecolor{tc%1}%2").arg(it.data()).arg(s);
	}
	
	list.sort();
	return list;
}

////////////////////////////// determine the whole tag //////////////////////////////

void TabularDialog::slotOk()
{
	m_table->updateCurrentCell();

	QString preamble,textline,s,s1,s2,s3;
	TabularCell::CountLines lines;
	TabularCell::Count cnt;
	
	// list of packages
	bool pkgArray = false; 
	bool pkgColor = false;
	bool pkgColortbl = false;
	
	// we need this very often
	int numrows = m_table->numRows();
	int numcols = m_table->numCols();

	// colortable
	QMap<QString,char> colortable;
	char colorchar = 'A';
	
	// list with all column information 
	QList<TabularCell::Preamble> colinfo;
	QString whitename = QColor(Qt::white).name();
	QString blackname = QColor(Qt::black).name();
	
	// is multicolumn command used
	bool multicolumn = false;

	// cursor and bullets
	QString bullet = ( m_cbBullets->isChecked() ) ? s_bullet : QString::null;
	bool setcursor =  ( ! m_cbBullets->isChecked() );
	
	// count all column information
	m_td.tagEnd = QString::null;
	for ( int col=0; col<=numcols; ++col ) 
	{
		TabularCell::Preamble info;
		info.vline = false;
		info.align = Qt::AlignLeft;
		
		// Now get column information for real columns.
		// The last info is only needed for a right vline.
		if ( col < numcols ) 
		{
			cnt = m_table->countCells(col,0,col,numrows-1);
		
			// and set values 
			info.bold = ( cnt.bold > cnt.cells/2 );
			info.italic =  ( cnt.italic > cnt.cells/2 );
			info.bgcolor = ( cnt.bgcolor > cnt.cells/2 ) ? cnt.nameBgcolor : whitename;
			info.textcolor = ( cnt.textcolor > cnt.cells/2 ) ? cnt.nameTextcolor : blackname;
		}
		
		// save all information
		colinfo.append( info );
	}
	
	// search for left vlines all columns 
	Q3Header *hor = m_table->horizontalHeader();
	for ( int col=0; col<numcols; ++col ) 
	{
		// get current header
		s = hor->label(col).remove(' ');
		if ( s.indexOf('>') || s.indexOf('<') || s.indexOf('!')  || s.indexOf('m') || s.indexOf('b')) 
			pkgArray = true; 
			
		// look for @{} and !{} substrings
		bool separator = ( s.indexOf('@')>=0 || s.indexOf('!')>=0 );
		if ( !separator ) 
		{
			lines = m_table->countVLines(col,true);
			if ( lines.cnt > numrows/2 ) 
			{
				preamble += '|';
				colinfo[col].vline = true;
			}
		}
		
		// color
		QString colorcommand = QString::null;
		if ( colinfo[col].bgcolor != whitename ) 
		{
			QChar color = defineColor(colinfo[col].bgcolor,colortable,colorchar);
			colorcommand += QString("\\columncolor{tc%1}").arg(color);
			pkgColortbl = true;
		}
		if ( colinfo[col].textcolor != blackname ) 
		{
			QChar color = defineColor(colinfo[col].textcolor,colortable,colorchar);
			colorcommand += QString("\\color{tc%1}").arg(color);
			pkgColor = true;
		}
		if ( ! colorcommand.isEmpty() ) 
		{
			if ( s.indexOf('>') >= 0 ) 
				s = s.replace(">{}",QString(">{%1}").arg(colorcommand));
			else  
				preamble += QString(">{%1}").arg(colorcommand); 
			pkgArray = true;
		}
		
		// Alignment: default is left, we look only for center or right 
		if ( s.indexOf('c') >= 0 )
			colinfo[col].align = Qt::AlignHCenter;
		else if ( s.indexOf('r') >= 0 )
				colinfo[col].align = Qt::AlignRight;
			
		// pre >{} and post <{} commands can stay, but perhaps bullets are wanted
		preamble += ( m_cbBullets->isChecked() ) ? s.replace("{}", '{' + s_bullet + '}') : s;
	}
	// search for right vline in last column
	lines = m_table->countVLines( numcols-1,false );
	if ( lines.cnt > numrows/2 ) 
	{
		preamble += '|';
		colinfo[numcols].vline = true;
	}
		
	//KILE_DEBUG() << "preamble " << preamble << endl;
	//KILE_DEBUG() << " topline " << getEol(0,true) << endl;
		
	// output all rows
	for ( int row=0; row<numrows; ++row ) 
	{
		textline = QString::null;
		
		// first check for a rowcolor command
		cnt = m_table->countCells(0,row,numcols-1,row);
		QString bgcolor = ( cnt.bgcolor > cnt.cells/2 ) ? cnt.nameBgcolor : whitename;
		if ( bgcolor != whitename ) 
		{
			QChar color = defineColor(cnt.nameBgcolor,colortable,colorchar);
			textline += QString("\\rowcolor{tc%1}\n").arg(color); 
			pkgColortbl = true;
		}
			
		int col = 0;
		while	( col < numcols ) 
		{
			TabularItem *cellitem = dynamic_cast<TabularItem*>( m_table->item(row,col) );
			if ( cellitem ) 
			{
				// check for multicolumn and initialize string parameter
				int colspan = cellitem->colSpan();
				s1 = ( colspan > 1 ) ? QString("%1").arg(colspan) : QString::null;
				s2 = s3 = QString::null;
				
				// Now look, if this cell(s) must be defined as multicolumn, because 
				// colspan is greater than 1, or the left vline, bgcolor or alignment 
				// is different from the preamble
				bool useMulticolumn = 
					(	( colspan > 1                                               )  ||
						( colinfo[col].vline     != m_table->isVLine(row,col,true)  )  ||
						( colinfo[col].bgcolor   != cellitem->m_data.bgcolor.name() )  ||
						( cellitem->m_data.align != colinfo[col].align              ) 
					);
						
				// build the multicolumn command (if necessary)
				if ( useMulticolumn ) 
				{ 
					// left vline
					//if ( (colinfo[col].vline!=m_table->isVLine(row,col,true)) && m_table->isVLine(row,col,true) ) {
					if ( m_table->isVLine(row,col,true) ) 
					{
						s2 += '|';
					}
					// bgcolor
				//	if ( ! ( (colinfo[col].bgcolor==cellitem->m_data.bgcolor.name()) &&
				//	         (colinfo[col].bgcolor==whitename) ) ) {
					if ( cellitem->m_data.bgcolor.name() != whitename ) 
					{
						QChar color = defineColor(cellitem->m_data.bgcolor.name(),colortable,colorchar);
						s2 += QString(">{\\columncolor{tc%1}}").arg(color);
						pkgColortbl = true;
					}
					// alignment
					//if ( cellitem->m_data.align!=colinfo[col].align ) {
					switch ( cellitem->m_data.align ) 
					{
						case Qt::AlignHCenter : s2 += 'c'; break;
						case Qt::AlignRight   : s2 += 'r'; break;
						default               : s2 += 'l';
					}
					// we have to set a right line in a multicolumn cell
					if ( m_table->isVLine(row,col,false) )  
					{
						s2 += '|';
					}
				}
				
				// now build cell entries
				if ( colinfo[col].bold != (cellitem->m_data.font & TabularCell::cfBold) ) 
				{
					s3 += "\\bfseries";
				}
				if ( colinfo[col].italic != (cellitem->m_data.font & TabularCell::cfItalic) ) 
				{
					s3 += "\\itshape";
				}
				
				if ( colinfo[col].textcolor != cellitem->m_data.textcolor.name() ) 
				{
					QChar color = defineColor(cellitem->m_data.textcolor.name(),colortable,colorchar);
					s3 += QString("\\color{tc%1}").arg(color);
					pkgColor = true;
				}
				if ( ! s3.isEmpty() )
					s3 += ' ';
					
				// add text
				if ( ! cellitem->text().isEmpty() )
					s3 += cellitem->text();
				else
				{
					if ( setcursor )
					{
						s3 += "%C";
						setcursor = false;
					}
					else
						s3 += bullet;
				}
					
				// build the whole cell entry
				if ( useMulticolumn ) 
				{
					textline += QString("\\mc{%1}{%2}{%3}").arg(colspan).arg(s2).arg(s3);
					multicolumn = true;
				}
				else 
					textline += s3;
					
				// increase column number
				col += colspan;
			} 
			else 
			{
				if ( setcursor ) 
				{
					textline += "%C";
					setcursor = false;
				}
				else
					textline += bullet;
				col++;
			}
			if  ( col < numcols )
		   	textline += " & ";
		}
	
		// add eol
		s = getEol(row,false);
		if ( row<numrows-1 || !s.isEmpty() ) 
			textline += " \\\\ " + s;
	
		//KILE_DEBUG() << "text: " << textline << endl;
		m_td.tagEnd += textline + '\n';
	}

	// build the list of colors
	QStringList colorlist = sortColorTable(colortable);
	bool group = ( multicolumn || colorlist.count()>0 );
	
	// get current environment
	QString envname = m_coEnvironment->currentText();
		
	// build a list of packages
	QString packagelist = QString::null;
	if ( pkgColortbl )
		pkgColor = true;
		
	if ( pkgArray )
		packagelist += ",array";
	if ( pkgColor )
		packagelist += ",color";
	if ( pkgColortbl )
		packagelist += ",colortbl";
	if ( m_cbBooktabs->isChecked() )
		packagelist += ",booktabs";
	if ( envname=="tabularx" || envname=="longtable" || envname=="supertabular" )
		packagelist += ',' + envname;
	if ( ! packagelist.isEmpty() ) {
		packagelist = packagelist.replace(0,1,"% use packages: ") + '\n';
	}	 
			
	// get environment names
	QString centername = ( isMathmodeEnvironment(envname) ) ? "displaymath" : "center";
	if ( m_cbStarred->isChecked() )
		envname += '*';
	
	// build the tag to insert
	if ( m_cbCenter->isChecked() )
		m_td.tagBegin = QString("\\begin{%1}\n").arg(centername);
	else if ( group )
		m_td.tagBegin = "{% \n";
	else
		m_td.tagBegin = QString::null;
	
	// add list of packages as comment
	m_td.tagBegin += packagelist;
	
	// define some commands 
	if ( group ) 
	{
		//m_td.tagBegin += "%\n";
		// add multicolumn shortcut
		if ( multicolumn ) 
			m_td.tagBegin += "\\newcommand{\\mc}[3]{\\multicolumn{#1}{#2}{#3}}\n";
		// add definitions of used colors
		for ( uint i=0; i<colorlist.count(); ++i )
			m_td.tagBegin += colorlist[i] + '\n';
		m_td.tagBegin += "%\n";
	}
		
	// add environment command 
	m_td.tagBegin += QString("\\begin{%1}").arg(envname);
	// add width for starred versions
	if ( m_cbStarred->isChecked() )
		m_td.tagBegin += QString("{%1}").arg(bullet);
	// add optional alignment parameter
	QString envparameter = ( m_coParameter->isEnabled() ) ? m_coParameter->currentText() : QString::null;
	if ( ! envparameter.isEmpty() ) 
		m_td.tagBegin += QString("[%1]").arg(envparameter);
	// add preamble
	m_td.tagBegin += QString("{%1}").arg(preamble);
	m_td.tagBegin += getEol(0,true) + '\n';
	
	// close environment
	m_td.tagEnd += QString("\\end{%1}\n").arg(envname);
	if ( m_cbCenter->isChecked() )
		m_td.tagEnd += QString("\\end{%1}\n").arg(centername);
	else if ( group )
		m_td.tagEnd += "}\n";
	
	m_td.dy = 0;
	m_td.dx = 0;
	
	// set cursor to first bullet position
	if ( m_cbBullets->isChecked() ) 
	{
		int pos = m_td.tagBegin.indexOf(bullet);
		if ( pos >= 0 ) 
		{
			m_td.tagBegin = m_td.tagBegin.replace(pos,1,"%C");
			setcursor = false;
		} 
		else 
		{
			pos = m_td.tagEnd.indexOf(bullet);
			if ( pos >= 0 ) 
			{
				m_td.tagEnd = m_td.tagEnd.replace(pos,1,"%C");
				setcursor = false;
			} 
			else 
			{
				setcursor = true;
			}
		}
	}
	if ( setcursor )
		m_td.tagEnd += "%C";
			
	accept();
}

QString TabularDialog::getEol(int row, bool top)
{
	QString s;
	
	bool booktabs = m_cbBooktabs->isChecked();
	TabularCell::CountLines lines = m_table->countHLines(row,top);
	if ( lines.cnt == lines.cells )
	{
		if ( booktabs ) 
		{
			if ( row==0 && top )
				s = "\\toprule";
			else if ( row==m_table->numRows()-1 && !top )
				s = "\\bottomrule";
			else
				s = "\\midrule";
		}
		else
		   s = "\\hline";
	}
	else if ( lines.list.count() > 0 ) 
	{
		QString cmd = ( booktabs ) ? "cmidrule" : "cline";
		for ( uint i=0; i<lines.list.count(); i+=2 ) 
			s += QString("\\%1{%2-%3}").arg(cmd).arg(lines.list[i]+1).arg(lines.list[i+1]+1);
	} 
	
	return s;
}

}

#include "tabulardialog.moc"
