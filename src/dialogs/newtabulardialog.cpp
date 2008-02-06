/********************************************************************************************
    begin                : Sunday Jun 27 2008
    copyright            : (C) 2008 by Mathias Soeken (msoeken@informatik.uni-bremen.de)
    copyright            : (C) 2005-2006 by Holger Danielsson (holger.danielsson@t-online.de)
 ********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "newtabulardialog.h"

#include <QCheckBox>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemDelegate>
#include <QLabel>
#include <QList>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QSpinBox>
#include <QStyleOptionViewItem>
#include <QTableWidget>
#include <QToolBar>
#include <QVBoxLayout>

#include <KAction>
#include <KColorCells>
#include <KColorDialog>
#include <KComboBox>
#include <KIcon>
#include <KLocale>
#include <KMenu>
#include <KMessageBox>
#include <KPushButton>

#include "kiledebug.h"
#include "latexcmd.h"

namespace KileDialog {

//BEGIN Icons for standard frames
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
//END

//BEGIN TabularFrameWidget
class TabularFrameWidget : public QFrame
{
	public:
		TabularFrameWidget(QWidget* parent = 0);
		void setBorder(int value);
		int border() const { return m_border; }

	protected:
		void paintEvent(QPaintEvent *event);
		void mousePressEvent(QMouseEvent *event);

	private:
		int m_border;
		QRect m_left, m_top, m_right, m_bottom;
};

TabularFrameWidget::TabularFrameWidget(QWidget* parent)
	: QFrame(parent)
{
	m_border = TabularCell::None;

	setBackgroundColor(Qt::white);
	setFixedWidth(120);
	setFixedHeight(120);
	setLineWidth(2);
	setFrameStyle(QFrame::Box | QFrame::Raised);

	QRect r = contentsRect();
	int x1 = r.left();
	int y1 = r.top();
	int x2 = r.right();
	int y2 = r.bottom();

	m_left.setRect(x1, y1 + 20, 20, y2 - 43);
	m_top.setRect(x1 + 20, y1, x2 - 43, 20);
	m_right.setRect(x2 - 20, y1 + 20, 20, y2 - 43);
	m_bottom.setRect(x1 + 20, y2 - 20, x2 - 43, 20);
}

void TabularFrameWidget::setBorder(int value)
{
	m_border = value;
	update();
}

void TabularFrameWidget::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	QPainter painter(this);

	QRect r = contentsRect();
	int x1 = r.left();
	int y1 = r.top();
	int x2 = r.right();
	int y2 = r.bottom();
	
	// left/top
	painter.setPen(Qt::black);
	painter.drawLine(x1 + 6, y1 + 14, x1 + 14, y1 + 14);
	painter.drawLine(x1 + 14, y1 + 14, x1 + 14, y1 + 6);
	
	// left/bottom
	painter.drawLine(x1 + 6, y2 - 14, x1 + 14, y2 - 14);
	painter.drawLine(x1 + 14, y2 - 14, x1 + 14, y2 - 6);
	
	// right/top
	painter.drawLine(x2 - 6, y1 + 14, x2 - 14, y1 + 14);
	painter.drawLine(x2 - 14, y1 + 14, x2 - 14, y1 + 6);
	
	// right/bottom
	painter.drawLine(x2 - 6, y2 - 14, x2 - 14, y2 - 14);
	painter.drawLine(x2 - 14, y2 - 14, x2 - 14, y2 - 6);
	
	// centered rectangle
	painter.setPen(Qt::gray);
	painter.setBrush(Qt::gray);
	painter.drawRect(x1 + 20, y1 + 20, x2 - 43, y2 - 43);
	
	//QPen pen = QPen(Qt::red,4);
	QPen pen = QPen(Qt::black, 4);
	painter.setPen(pen);
	if(m_border & TabularCell::Left) {
		painter.drawLine(x1 + 10, y1 + 20, x1 + 10, y2 - 20);
	}
	if(m_border & TabularCell::Top) {
		painter.drawLine(x1 + 20, y1 + 10, x2 - 20, y1 + 10);
	}
	if(m_border & TabularCell::Right) {
		painter.drawLine(x2 - 10, y1 + 20, x2 - 10, y2 - 20);
	}
	if(m_border & TabularCell::Bottom) {
		painter.drawLine(x1 + 20, y2 - 10, x2 - 20, y2 - 10);
	}
}

void TabularFrameWidget::mousePressEvent(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton)
		return;

	int x = event->x();
	int y = event->y();

	int state = 0;
	if(m_left.contains(x, y))
		state = TabularCell::Left;
	else if(m_top.contains(x, y))
		state = TabularCell::Top;
	else if(m_right.contains(x, y))
		state = TabularCell::Right;
	else if(m_bottom.contains(x, y))
		state = TabularCell::Bottom;

	if(state > 0) {
		if(m_border & state) {
			m_border &= ~state;
		}
		else {
			m_border |= state;
		}
		update();
	}
}
//END

SelectFrameAction::SelectFrameAction(const QString &text, QToolBar *parent)
	: KToolBarPopupAction(KIcon(), text, parent),
	  m_Parent(parent),
	  m_CurrentBorder(TabularCell::None)
{
	setIcon(generateIcon());

	QWidget *page = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout();
	layout->setMargin(0);
	layout->setSpacing(0);
	page->setLayout(layout);

	QWidget *buttonBox = new QWidget(page);
	QHBoxLayout *buttonBoxLayout = new QHBoxLayout();
	buttonBoxLayout->setMargin(0);
	buttonBoxLayout->setSpacing(KDialog::spacingHint());
	buttonBox->setLayout(buttonBoxLayout);

	m_pbNone = new KPushButton(buttonBox);
	m_pbLeftRight = new KPushButton(buttonBox);
	m_pbTopBottom = new KPushButton(buttonBox);
	m_pbAll = new KPushButton(buttonBox);

	int height = m_pbNone->sizeHint().height();
	m_pbNone->setFixedSize(height, height);
	m_pbLeftRight->setFixedSize(height, height);
	m_pbTopBottom->setFixedSize(height, height);
	m_pbAll->setFixedSize(height, height);

	m_pbNone->setPixmap(QPixmap(const_cast<const char**>(no_border_xpm)));
	m_pbLeftRight->setPixmap(QPixmap(const_cast<const char**>(lr_border_xpm)));
	m_pbTopBottom->setPixmap(QPixmap(const_cast<const char**>(tb_border_xpm)));
	m_pbAll->setPixmap(QPixmap(const_cast<const char**>(all_border_xpm)));

	buttonBoxLayout->addStretch();
	buttonBoxLayout->addWidget(m_pbNone);
	buttonBoxLayout->addWidget(m_pbLeftRight);
	buttonBoxLayout->addWidget(m_pbTopBottom);
	buttonBoxLayout->addWidget(m_pbAll);
	buttonBoxLayout->addStretch();

	QWidget *frameWidget = new QWidget(page);
	QHBoxLayout *frameWidgetLayout = new QHBoxLayout();
	frameWidgetLayout->setMargin(0);
	frameWidgetLayout->setSpacing(KDialog::spacingHint());
	frameWidget->setLayout(frameWidgetLayout);

	m_FrameWidget = new TabularFrameWidget(frameWidget);

	frameWidgetLayout->addStretch();
	frameWidgetLayout->addWidget(m_FrameWidget);
	frameWidgetLayout->addStretch();

	m_pbDone = new KPushButton(KIcon("dialog-ok-apply"), i18n("Apply"), page);

	layout->addWidget(buttonBox);
	layout->addWidget(frameWidget);
	layout->addWidget(m_pbDone);

	QWidgetAction *widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(page);
	popupMenu()->addAction(widgetAction);

	connect(this, SIGNAL(triggered(bool)),
	        this, SLOT(slotTriggered()));
	connect(m_pbNone, SIGNAL(clicked()),
	        this, SLOT(slotNoneClicked()));
	connect(m_pbLeftRight, SIGNAL(clicked()),
	        this, SLOT(slotLeftRightClicked()));
	connect(m_pbTopBottom, SIGNAL(clicked()),
	        this, SLOT(slotTopBottomClicked()));
	connect(m_pbAll, SIGNAL(clicked()),
	        this, SLOT(slotAllClicked()));
	connect(m_pbDone, SIGNAL(clicked()),
	        this, SLOT(slotDoneClicked()));
}

QIcon SelectFrameAction::generateIcon()
{
	QPixmap pixmap(m_Parent->iconSize());

	QPainter painter(&pixmap);
	painter.fillRect(pixmap.rect(), Qt::gray);

	painter.setPen(Qt::black);
	if(m_CurrentBorder & TabularCell::Left)
		painter.drawLine(0, 0, 0, pixmap.height() - 1);
	if(m_CurrentBorder & TabularCell::Top)
		painter.drawLine(0, 0, pixmap.width() - 1, 0);
	if(m_CurrentBorder & TabularCell::Right)
		painter.drawLine(pixmap.width() - 1, 0, pixmap.width() - 1, pixmap.height() - 1);
	if(m_CurrentBorder & TabularCell::Bottom)
		painter.drawLine(0, pixmap.height() - 1, pixmap.width() - 1, pixmap.height() - 1);

	painter.end();

	return QIcon(pixmap);
}

void SelectFrameAction::slotTriggered()
{
	emit borderSelected(m_CurrentBorder);
}

void SelectFrameAction::slotNoneClicked()
{
	m_FrameWidget->setBorder(TabularCell::None);
}

void SelectFrameAction::slotLeftRightClicked()
{
	m_FrameWidget->setBorder(TabularCell::Left | TabularCell::Right);
}

void SelectFrameAction::slotTopBottomClicked()
{
	m_FrameWidget->setBorder(TabularCell::Top | TabularCell::Bottom);
}

void SelectFrameAction::slotAllClicked()
{
	m_FrameWidget->setBorder(TabularCell::Left | TabularCell::Right | TabularCell::Top | TabularCell::Bottom);
}

void SelectFrameAction::slotDoneClicked()
{
	int newBorder = m_FrameWidget->border();
	if(m_CurrentBorder != newBorder) {
		m_CurrentBorder = newBorder;
		setIcon(generateIcon());
	}
	emit borderSelected(newBorder);
	popupMenu()->hide();
}

SelectColorAction::SelectColorAction(const KIcon &icon, const QString &text, QWidget *parent)
	: KToolBarPopupAction(icon, text, parent)
{
	QWidget *page = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout();
	layout->setMargin(0);
	layout->setSpacing(0);
	page->setLayout(layout);

	m_ccColors = new KColorCells(page, 4, 4);
	m_ccColors->setSelectionMode(QAbstractItemView::NoSelection);
	m_ccColors->setColor(0, Qt::white);
	m_ccColors->setColor(1, Qt::black);
	m_ccColors->setColor(2, Qt::red);
	m_ccColors->setColor(3, Qt::darkRed);
	m_ccColors->setColor(4, Qt::green);
	m_ccColors->setColor(5, Qt::darkGreen);
	m_ccColors->setColor(6, Qt::blue);
	m_ccColors->setColor(7, Qt::darkBlue);
	m_ccColors->setColor(8, Qt::cyan);
	m_ccColors->setColor(9, Qt::darkCyan);
	m_ccColors->setColor(10, Qt::magenta);
	m_ccColors->setColor(11, Qt::darkMagenta);
	m_ccColors->setColor(12, Qt::yellow);
	m_ccColors->setColor(13, Qt::darkYellow);
	m_ccColors->setColor(14, Qt::gray);
	m_ccColors->setColor(15, Qt::darkGray);

	m_pbCustom = new KPushButton(KIcon("kcolorchooser"), i18n("Select custom color..."), page);

	layout->addWidget(m_ccColors);
	layout->addWidget(m_pbCustom);

	QWidgetAction *widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(page);
	popupMenu()->addAction(widgetAction);

	connect(popupMenu(), SIGNAL(aboutToShow()),
	        this, SLOT(slotPopupAboutToShow()));
	connect(m_ccColors, SIGNAL(colorSelected(int, const QColor&)),
	        this, SLOT(slotColorSelected(int, const QColor&)));
	connect(m_pbCustom, SIGNAL(clicked()),
	        this, SLOT(slotCustomClicked()));
}

void SelectColorAction::slotPopupAboutToShow()
{
	m_ccColors->selectionModel()->clearSelection();
}

void SelectColorAction::slotColorSelected(int index, const QColor &color)
{
	Q_UNUSED(index);

	emit colorSelected(color);
	popupMenu()->hide();
}

void SelectColorAction::slotCustomClicked()
{
	QColor color;
	int result = KColorDialog::getColor(color);
	if (result == KColorDialog::Accepted) {
		emit colorSelected(color);
	}
	popupMenu()->hide();
}

//BEGIN TabularCellDelegate
class TabularCellDelegate : public QItemDelegate {
	public:
		TabularCellDelegate(QTableWidget *parent = 0);

		virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

	private:
		QTableWidget *m_Table;
};

TabularCellDelegate::TabularCellDelegate(QTableWidget *parent)
	: QItemDelegate(parent),
	  m_Table(parent)
{
}

void TabularCellDelegate::paint(QPainter *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
	QItemDelegate::paint(painter, option, index);

	int rowCount = m_Table->rowCount();
	int columnCount = m_Table->columnCount();

	int row = index.row();
	int column = index.column();

	TabularCell *cell = static_cast<TabularCell*>(m_Table->item(row, column));

	if(column == 0) {
		painter->setPen(cell->border() & TabularCell::Left ? Qt::black : Qt::lightGray);
		painter->drawLine(option.rect.topLeft(), option.rect.bottomLeft());
	}

	if(row == 0) {
		painter->setPen(cell->border() & TabularCell::Top ? Qt::black : Qt::lightGray);
		painter->drawLine(option.rect.topLeft(), option.rect.topRight());
	}

	bool right = (cell->border() & TabularCell::Right)
		|| (column < (columnCount - 1) && static_cast<TabularCell*>(m_Table->item(row, column + 1))->border() & TabularCell::Left);
	painter->setPen(right ? Qt::black : Qt::lightGray);
	painter->drawLine(option.rect.topRight(), option.rect.bottomRight());

	bool bottom = (cell->border() & TabularCell::Bottom)
		|| (row < (rowCount - 1) && static_cast<TabularCell*>(m_Table->item(row + 1, column))->border() & TabularCell::Top);
	painter->setPen(bottom ? Qt::black : Qt::lightGray);
	painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
}
//END

//BEGIN TabularCell
TabularCell::TabularCell()
	: QTableWidgetItem(),
	  m_Border(None)
{
}

TabularCell::TabularCell(const QString &text)
	: QTableWidgetItem(text),
	  m_Border(None)
{
}

void TabularCell::setBorder(int border)
{
	m_Border = border;
	tableWidget()->update();
}

int TabularCell::border() const
{
	return m_Border;
}
//END

//BEGIN TabularHeaderItem
TabularHeaderItem::TabularHeaderItem(QWidget *parent)
	: QObject(parent),
	  QTableWidgetItem(KIcon("format-justify-left"), "l"),
	  m_Alignment(Qt::AlignLeft),
	  m_InsertBefore(false),
	  m_InsertAfter(false),
	  m_SuppressSpace(false),
	  m_DontSuppressSpace(false),
	  m_hasXAlignment(false)
{
	m_Popup = new QMenu(parent);
	m_Popup->addAction(KIcon("format-justify-left"), i18n("Align Left"), this, SLOT(slotAlignLeft()));
	m_Popup->addAction(KIcon("format-justify-center"), i18n("Align Center"), this, SLOT(slotAlignCenter()));
	m_Popup->addAction(KIcon("format-justify-right"), i18n("Align Right"), this, SLOT(slotAlignRight()));
	m_Popup->addAction(i18n("p{w} Alignment"), this, SLOT(slotAlignP()));
	m_Popup->addAction(i18n("b{w} Alignment"), this, SLOT(slotAlignB()));
	m_Popup->addAction(i18n("m{w} Alignment"), this, SLOT(slotAlignM()));
	m_acXAlignment = m_Popup->addAction(i18n("X Alignment"), this, SLOT(slotAlignX()));
	m_Popup->addSeparator();
	m_acDeclPre = m_Popup->addAction(i18n("Insert Before Declaration"), this, SLOT(slotDeclPre()));
	m_acDeclPost = m_Popup->addAction(i18n("Insert After Declaration"), this, SLOT(slotDeclPost()));
	m_acDeclAt = m_Popup->addAction(i18n("Suppress Space"), this, SLOT(slotDeclAt()));
	m_acDeclBang = m_Popup->addAction(i18n("Do not Suppress Space"), this, SLOT(slotDeclBang()));

	m_acDeclPre->setCheckable(true);
	m_acDeclPost->setCheckable(true);
	m_acDeclAt->setCheckable(true);
	m_acDeclBang->setCheckable(true);
}

void TabularHeaderItem::setAlignment(int alignment)
{
	m_Alignment = alignment;
	format();
}

int TabularHeaderItem::alignment() const
{
	return m_Alignment;
}

bool TabularHeaderItem::insertBefore() const
{
	return m_InsertBefore;
}

bool TabularHeaderItem::insertAfter() const
{
	return m_InsertAfter;
}

bool TabularHeaderItem::suppressSpace() const
{
	return m_SuppressSpace;
}

bool TabularHeaderItem::dontSuppressSpace() const
{
	return m_DontSuppressSpace;
}

void TabularHeaderItem::setHasXAlignment(bool hasXAlignment)
{
	m_hasXAlignment = hasXAlignment;
	if(!hasXAlignment && m_Alignment == AlignX) {
		slotAlignLeft();
	}
}

bool TabularHeaderItem::hasXAlignment() const
{
	return m_hasXAlignment;
}

QMenu* TabularHeaderItem::popupMenu() const
{
	m_acXAlignment->setVisible(m_hasXAlignment);
	return m_Popup;
}

void TabularHeaderItem::format()
{
	setIcon(iconForAlignment(m_Alignment));

	QString text = "";

	if(m_SuppressSpace) {
		text += '@';
	} else if(m_DontSuppressSpace) {
		text += '!';
	}
	if(m_InsertBefore) {
		text += '>';
	}

	switch(m_Alignment) {
		case Qt::AlignLeft:
			text += 'l';
			break;
		case Qt::AlignHCenter:
			text += 'c';
			break;
		case Qt::AlignRight:
			text += 'r';
			break;
		case AlignP:
			text += 'p';
			break;
		case AlignB:
			text += 'b';
			break;
		case AlignM:
			text += 'm';
			break;
		case AlignX:
			text += 'X';
			break;
	}

	if(m_InsertAfter) {
		text += '<';
	}

	setText(text);
}

inline KIcon TabularHeaderItem::iconForAlignment(int alignment) const
{
	switch(alignment) {
		case Qt::AlignLeft:
			return KIcon("format-justify-left");
		case Qt::AlignHCenter:
			return KIcon("format-justify-center");
		case Qt::AlignRight:
			return KIcon("format-justify-right");
		default:
			return KIcon();
	}
}

void TabularHeaderItem::slotAlignLeft()
{
	setAlignment(Qt::AlignLeft);
	emit alignColumn(Qt::AlignLeft);
}

void TabularHeaderItem::slotAlignCenter()
{
	setAlignment(Qt::AlignHCenter);
	emit alignColumn(Qt::AlignHCenter);
}

void TabularHeaderItem::slotAlignRight()
{
	setAlignment(Qt::AlignRight);
	emit alignColumn(Qt::AlignRight);
}

void TabularHeaderItem::slotAlignP()
{
	setAlignment(AlignP);
	emit alignColumn(Qt::AlignLeft);
}

void TabularHeaderItem::slotAlignB()
{
	setAlignment(AlignB);
	emit alignColumn(Qt::AlignLeft);
}

void TabularHeaderItem::slotAlignM()
{
	setAlignment(AlignM);
	emit alignColumn(Qt::AlignLeft);
}

void TabularHeaderItem::slotAlignX()
{
	setAlignment(AlignX);
	emit alignColumn(Qt::AlignLeft);
}

void TabularHeaderItem::slotDeclPre()
{
	m_InsertBefore = m_acDeclPre->isChecked();
	format();
}

void TabularHeaderItem::slotDeclPost()
{
	m_InsertAfter = m_acDeclPost->isChecked();
	format();
}

void TabularHeaderItem::slotDeclAt()
{
	m_SuppressSpace = m_acDeclAt->isChecked();
	if(m_SuppressSpace) {
		m_DontSuppressSpace = false;
		m_acDeclBang->setChecked(false);
	}
	format();
}

void TabularHeaderItem::slotDeclBang()
{
	m_DontSuppressSpace = m_acDeclBang->isChecked();
	if(m_DontSuppressSpace) {
		m_SuppressSpace = false;
		m_acDeclAt->setChecked(false);
	}
	format();
}
//END

NewTabularDialog::NewTabularDialog(KileDocument::LatexCommands *commands, KConfig *config, QWidget *parent)
	: Wizard(config, parent),
	  m_latexCommands(commands),
	  m_clCurrentBackground(Qt::white),
	  m_clCurrentForeground(Qt::black)
{
	setCaption(i18n("Tabular Environments"));

	QWidget *page = new QWidget(this);
	QVBoxLayout *pageLayout = new QVBoxLayout();
	pageLayout->setMargin(0);
	pageLayout->setSpacing(KDialog::spacingHint());
	page->setLayout(pageLayout);

	m_tbFormat = new QToolBar(page);
	m_tbFormat->setMovable(false);
	m_tbFormat->setFloatable(false);
	m_tbFormat->setOrientation(Qt::Horizontal);

	m_acLeft = addAction(KIcon("format-justify-left"), i18n("Align Left"), SLOT(slotAlignLeft()), page);
	m_acCenter = addAction(KIcon("format-justify-center"), i18n("Align Center"), SLOT(slotAlignCenter()), page);
	m_acRight = addAction(KIcon("format-justify-right"), i18n("Align Right"), SLOT(slotAlignRight()), page);
	m_tbFormat->addSeparator();
	m_acBold = addAction(KIcon("format-text-bold"), i18n("Bold"), SLOT(slotBold()), page);
	m_acItalic = addAction(KIcon("format-text-italic"), i18n("Italic"), SLOT(slotItalic()), page);
	m_acUnderline = addAction(KIcon("format-text-underline"), i18n("Underline"), SLOT(slotUnderline()), page);
	m_tbFormat->addSeparator();
	m_acJoin = addAction(KIcon("table-join-cells"), i18n("Join Cells"), SLOT(slotJoinCells()), page); // FIXME icon
	m_acSplit = addAction(KIcon("table-split-cells"), i18n("Split Cells"), SLOT(slotSplitCells()), page); // FIXME icon
	m_acSplit->setEnabled(false);
	m_acFrame = new SelectFrameAction(i18n("Edit Frame"), m_tbFormat);
	connect(m_acFrame, SIGNAL(borderSelected(int)), this, SLOT(slotFrame(int)));
	m_tbFormat->addAction(m_acFrame);
	m_tbFormat->addSeparator();

	m_acBackground = new SelectColorAction(KIcon("format-fill-color"), i18n("Background Color"), page);
	m_acBackground->setIcon(generateColorIcon(true));
	connect(m_acBackground, SIGNAL(triggered(bool)), this, SLOT(slotCurrentBackground()));
	connect(m_acBackground, SIGNAL(colorSelected(const QColor&)), this, SLOT(slotBackground(const QColor&)));
	m_tbFormat->addAction(m_acBackground);
	m_acForeground = new SelectColorAction(KIcon("format-stroke-color"), i18n("Text Color"), page);
	m_acForeground->setIcon(generateColorIcon(false));
	connect(m_acForeground, SIGNAL(colorSelected(const QColor&)), this, SLOT(slotForeground(const QColor&)));
	connect(m_acForeground, SIGNAL(triggered(bool)), this, SLOT(slotCurrentForeground()));
	m_tbFormat->addAction(m_acForeground);

	m_tbFormat->addSeparator();
	m_acClearText = addAction(KIcon("edit-clear"), i18n("Clear Text"), SLOT(slotClearText()), page); // FIXME icon
	m_acClearAttributes = addAction(KIcon("edit-clear"), i18n("Clear Attributes"), SLOT(slotClearAttributes()), page); // FIXME icon
	m_acClearAll = addAction(KIcon("edit-clear"), i18n("Clear All"), SLOT(slotClearAll()), page);

	/* checkable items */
	m_acLeft->setCheckable(true);
	m_acCenter->setCheckable(true);
	m_acRight->setCheckable(true);
	m_acBold->setCheckable(true);
	m_acItalic->setCheckable(true);
	m_acUnderline->setCheckable(true);

	m_Table = new QTableWidget(page);
	m_Table->setItemDelegate(new TabularCellDelegate(m_Table));
	m_Table->setShowGrid(false);
	m_Table->installEventFilter(this);
	m_Table->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

	QGroupBox *configPage = new QGroupBox(i18n("Environment"), page);
	QGridLayout *configPageLayout = new QGridLayout();
	configPageLayout->setMargin(KDialog::marginHint());
	configPageLayout->setSpacing(KDialog::spacingHint());
	configPage->setLayout(configPageLayout);

	QLabel *label = new QLabel(i18n("Name:"), configPage);
	m_cmbName = new KComboBox(configPage);
	label->setBuddy(m_cmbName);
	configPageLayout->addWidget(label, 0, 0);
	configPageLayout->addWidget(m_cmbName, 0, 1);
	label = new QLabel(i18n("Parameter:"), configPage);
	m_cmbParameter = new KComboBox(configPage);
	label->setBuddy(m_cmbParameter);
	configPageLayout->addWidget(label, 1, 0);
	configPageLayout->addWidget(m_cmbParameter, 1, 1);

	label = new QLabel(i18n("Number of rows:"), configPage);
	m_sbRows = new QSpinBox(configPage);
	m_sbRows->setMinimum(1);
	m_sbRows->setValue(3);
	label->setBuddy(m_sbRows);
	configPageLayout->addWidget(label, 0, 2);
	configPageLayout->addWidget(m_sbRows, 0, 3);
	label = new QLabel(i18n("Number of cols:"), configPage);
	m_sbCols = new QSpinBox(configPage);
	m_sbCols->setMinimum(1);
	m_sbCols->setValue(3);
	label->setBuddy(m_sbCols);
	configPageLayout->addWidget(label, 1, 2);
	configPageLayout->addWidget(m_sbCols, 1, 3);

	m_cbStarred = new QCheckBox(i18n("Use starred version"), configPage);
	m_cbCenter = new QCheckBox(i18n("Center"), configPage);
	m_cbCenter->setChecked(true);
	m_cbBooktabs = new QCheckBox(i18n("Use booktabs package"), configPage);
	m_cbBullets = new QCheckBox(i18n("Insert bullets"), configPage);
	m_cbBullets->setChecked(true);
	configPageLayout->addWidget(m_cbStarred, 2, 0, 1, 2);
	configPageLayout->addWidget(m_cbCenter, 2, 2, 1, 2);
	configPageLayout->addWidget(m_cbBooktabs, 3, 0, 1, 2);
	configPageLayout->addWidget(m_cbBullets, 3, 2, 1, 2);

	pageLayout->addWidget(m_tbFormat);
	pageLayout->addWidget(m_Table);
	pageLayout->addWidget(configPage);

	// whats this texts
	m_Table->setWhatsThis(i18n("Input data. Just type the text when a cell is selected. When pressing return, the adjacent cell will be selected."));
	m_cmbName->setWhatsThis(i18n("Choose an environment."));
	m_cmbParameter->setWhatsThis(i18n("Optional parameter for the chosen environment."));
	m_sbRows->setWhatsThis(i18n("Choose the number of table rows."));
	m_sbCols->setWhatsThis(i18n("Choose the number of table columns."));
	m_cbCenter->setWhatsThis(i18n("The tabular will be centered."));
	m_cbBooktabs->setWhatsThis(i18n("Use line commands of the booktabs package."));
	m_cbStarred->setWhatsThis(i18n("Use the starred version of this environment."));
	m_cbBullets->setWhatsThis(i18n("Insert bullets in each cell. Alt+Ctrl+Right and Alt+Ctrl+Left will move very quick from one cell to another."));
	m_acBold->setWhatsThis(i18n("Set bold font series."));
	m_acItalic->setWhatsThis(i18n("Set italic font shape."));
	m_acUnderline->setWhatsThis(i18n("Set underlined font shape."));
	m_acLeft->setWhatsThis(i18n("The text will be aligned at the left border of the cell."));
	m_acCenter->setWhatsThis(i18n("The text will be centered."));
	m_acRight->setWhatsThis(i18n("The text will be aligned at the right border of the cell."));
	m_acJoin->setWhatsThis(i18n("Joins adjacent cells when they are in the same row."));
	m_acSplit->setWhatsThis(i18n("Splits joined cells."));
	m_acFrame->setWhatsThis(i18n("Choose the border for the selected cells. When clicking on the button, the current border will be applied to the selected cells."));
	m_acBackground->setWhatsThis(i18n("Choose a background color (needs color package)."));
	m_acForeground->setWhatsThis(i18n("Choose a text color (needs color package)."));
	m_acClearText->setWhatsThis(i18n("Clears the text of the selected cells but keeps the attributes like alignment and font shape."));
	m_acClearAttributes->setWhatsThis(i18n("Resets the attributes of the selected cells to the default values but keeps the text."));
	m_acClearAll->setWhatsThis(i18n("Clears the text of the selected cells and resets the attributes."));

	setMainWidget(page);
	initEnvironments();
	updateColsAndRows();
	m_Table->item(0, 0)->setSelected(true);

	connect(m_Table, SIGNAL(itemSelectionChanged()),
	        this, SLOT(slotItemSelectionChanged()));
	connect(m_cmbName, SIGNAL(activated(const QString&)),
	        this, SLOT(slotEnvironmentChanged(const QString&)));
	connect(m_sbCols, SIGNAL(valueChanged(int)),
	        this, SLOT(updateColsAndRows()));
	connect(m_sbRows, SIGNAL(valueChanged(int)),
	        this, SLOT(updateColsAndRows()));
	connect(m_Table->horizontalHeader(), SIGNAL(customContextMenuRequested(const QPoint&)),
					this, SLOT(slotHeaderCustomContextMenuRequested(const QPoint&)));
}

NewTabularDialog::~NewTabularDialog()
{
}

void NewTabularDialog::initEnvironments()
{
	/* read all tabular environments and insert them into the combobox */
	QStringList list;
	QStringList::ConstIterator it;
	m_latexCommands->commandList(list, KileDocument::CmdAttrTabular, false);
	m_cmbName->addItems(list);
	
	// FIXME differ between array and tabular environment

	// refresh other gui elements regarding environment combo box
	slotEnvironmentChanged(m_cmbName->currentText());
}

KAction* NewTabularDialog::addAction(const KIcon &icon, const QString &text, const char *method, QObject *parent)
{
	KAction *action = new KAction(icon, text, parent);
	connect(action, SIGNAL(triggered(bool)), this, method);
	m_tbFormat->addAction(action);

	return action;
}

void NewTabularDialog::alignItems(int alignment)
{
	QList<int> checkColumns;

	foreach(QTableWidgetItem *item, m_Table->selectedItems()) {
		item->setTextAlignment(alignment | Qt::AlignVCenter);

		int column = item->column();
		if(!checkColumns.contains(column)) {
			checkColumns.append(column);
		}
	}

	foreach(int column, checkColumns) {
		if(checkForColumnAlignment(column)) {
			static_cast<TabularHeaderItem*>(m_Table->horizontalHeaderItem(column))->setAlignment(alignment);
		}
	}

	slotItemSelectionChanged();
}

bool NewTabularDialog::checkForColumnAlignment(int column)
{
	int alignment = m_Table->item(0, column)->textAlignment();

	for(int row = 1; row < m_Table->rowCount(); ++row) {
		if(m_Table->item(row, column)->textAlignment() != alignment) {
			return false;
		}
	}

	return true;
}

QIcon NewTabularDialog::generateColorIcon(bool background) const
{
	QString iconName = background ? "format-fill-color" : "format-stroke-color";
	QPixmap pixmap = KIconLoader().loadIcon(iconName, KIconLoader::Toolbar);

	QPainter painter(&pixmap);
	QColor color = background ? m_clCurrentBackground : m_clCurrentForeground;
	painter.fillRect(1, pixmap.height() - 7, pixmap.width() - 2, 6, color);
	painter.end();

	return QIcon(pixmap);
}

bool NewTabularDialog::eventFilter(QObject *obj, QEvent *event)
{
	if(obj == m_Table && event->type() == QEvent::KeyPress && m_Table->selectedItems().count() == 1) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

		if(keyEvent->key() == Qt::Key_Return) {
			QTableWidgetItem *item = m_Table->selectedItems()[0];
			int row = item->row();
			int column = item->column();
			if(column < (m_Table->columnCount() - 1)) {
				item->setSelected(false);
				m_Table->item(row, column + 1)->setSelected(true);
				m_Table->setCurrentItem(m_Table->item(row, column + 1));
			} else {
				if(row == (m_Table->rowCount() - 1)) {
					m_sbRows->setValue(m_sbRows->value() + 1);
					/* This is called twice, but now we can be sure that the new row has been created */
					updateColsAndRows();
				}
				item->setSelected(false);
				m_Table->item(row + 1, 0)->setSelected(true);
				m_Table->setCurrentItem(m_Table->item(row + 1, 0));
			}
		}
	}

	return Wizard::eventFilter(obj, event);
}

int NewTabularDialog::exec()
{
	/* all toolbar items should be visible when showing the dialog */
	show();
	mainWidget()->setMinimumWidth(m_tbFormat->width() + 2 * KDialog::marginHint());

	return Wizard::exec();
}

void NewTabularDialog::updateColsAndRows()
{
	int addedCols = m_sbCols->value() - m_Table->columnCount();
	int addedRows = m_sbRows->value() - m_Table->rowCount();

	// check whether content could be deleted when shrinking the table
	if(addedCols < 0) {
		bool hasContent = false;
		for(int column = m_Table->columnCount() + addedCols; column < m_Table->columnCount(); ++column) {
			for(int row = 0; row < m_Table->rowCount(); ++row) {
				if(m_Table->item(row, column) && !(m_Table->item(row, column)->text().isEmpty())) {
					hasContent = true;
					break;
				}
			}
			if(hasContent) break;
		}

		if(hasContent) {
			if(KMessageBox::questionYesNo(m_Table, i18n("Setting the new size for the table will delete content. Are you sure to set the new size?"), i18n("Resizing table")) == KMessageBox::No) {
				m_sbCols->setValue(m_Table->columnCount());
				return;
			}
		}
	}

	// check whether content could be deleted when shrinking the table
	if(addedRows < 0) {
		bool hasContent = false;
		for(int row = m_Table->rowCount() + addedRows; row < m_Table->rowCount(); ++row) {
			for(int column = 0; column < m_Table->columnCount(); ++column) {
				if(m_Table->item(row, column) && !(m_Table->item(row, column)->text().isEmpty())) {
					hasContent = true;
					break;
				}
			}
			if(hasContent) break;
		}

		if(hasContent) {
			if(KMessageBox::questionYesNo(m_Table, i18n("Setting the new size for the table will delete content. Are you sure to set the new size?"), i18n("Resizing table")) == KMessageBox::No) {
				m_sbRows->setValue(m_Table->rowCount());
				return;
			}
		}
	}

	m_Table->setColumnCount(m_sbCols->value());
	m_Table->setRowCount(m_sbRows->value());

	if(addedCols > 0) {
		for(int i = m_Table->columnCount() - addedCols; i < m_Table->columnCount(); ++i) {
			TabularHeaderItem *headerItem = new TabularHeaderItem(m_Table->horizontalHeader());
			connect(headerItem, SIGNAL(alignColumn(int)), this, SLOT(slotAlignColumn(int)));
			m_Table->setHorizontalHeaderItem(i, headerItem);

			// each cell should be an item. This is necessary for selection checking
			for(int row = 0; row < m_Table->rowCount(); ++row) {
				QTableWidgetItem *item = new TabularCell(QString());
				item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
				m_Table->setItem(row, i, item);
			}
		}
	}

	if(addedRows > 0) {
		for(int i = m_Table->rowCount() - addedRows; i < m_Table->rowCount(); ++i) {
			m_Table->resizeRowToContents(i);

			// each cell should be an item. This is necessary for selection checking
			for(int column = 0; column < m_Table->columnCount(); ++column) {
				QTableWidgetItem *item = new TabularCell(QString());
				item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
				m_Table->setItem(i, column, item);
			}
		}
	}
}

void NewTabularDialog::slotEnvironmentChanged(const QString &environment)
{
	// clear parameter combobox
	m_cmbParameter->clear();
	m_cmbParameter->setEnabled(false);

	// look for environment parameter in dictionary
	KileDocument::LatexCmdAttributes attr;
	if(m_latexCommands->commandAttributes(environment, attr)) {
		// starred version
		m_cbStarred->setEnabled(attr.starred);

		// option
		if(attr.option.indexOf('[') == 0) {
			QStringList optionlist = attr.option.split("");
			if(optionlist.count() > 2) {
				// ok, let's enable it
				m_cmbParameter->setEnabled(true);
				m_cmbParameter->insertItem(QString());
				// insert some options
				for(int i = 1; i < optionlist.count() - 1; ++i) {
					m_cmbParameter->insertItem(optionlist[i]);
				}
			}
		}
	}

	// has X alignment
	bool hasXAlignment = (environment == "tabularx" || environment == "xtabular");
	for(int column = 0; column < m_Table->columnCount(); ++column) {
		static_cast<TabularHeaderItem*>(m_Table->horizontalHeaderItem(column))->setHasXAlignment(hasXAlignment);
	}
}

void NewTabularDialog::slotItemSelectionChanged()
{
	/* unset some items */
	m_acLeft->setChecked(false);
	m_acCenter->setChecked(false);
	m_acRight->setChecked(false);

	/* set all font format items and eventually unset them later */
	m_acBold->setChecked(true);
	m_acItalic->setChecked(true);
	m_acUnderline->setChecked(true);

	/* nothing selected, nothing to do! */
	QList<QTableWidgetItem*> selectedItems = m_Table->selectedItems();
	if(selectedItems.count() == 0) return;

	/* check for alignment */
	int alignment = selectedItems[0]->textAlignment();
	bool sameAlignment = true;
	for(int i = 1; i < selectedItems.count(); ++i) {
		if(selectedItems[i]->textAlignment() != alignment) {
			sameAlignment = false;
			break;
		}
	}
	if(sameAlignment) {
		m_acLeft->setChecked(alignment & Qt::AlignLeft);
		m_acCenter->setChecked(alignment & Qt::AlignHCenter);
		m_acRight->setChecked(alignment & Qt::AlignRight);
	}

	/* check for font format */
	bool unsetBold = false;
	bool unsetItalic = false;
	bool unsetUnderline = false;
	foreach(QTableWidgetItem *item, selectedItems) {
		if(!unsetBold && !item->font().bold()) {
			m_acBold->setChecked(false);
			unsetBold = true;
		}
		if(!unsetItalic && !item->font().italic()) {
			m_acItalic->setChecked(false);
			unsetItalic = true;
		}
		if(!unsetUnderline && !item->font().underline()) {
			m_acUnderline->setChecked(false);
			unsetUnderline = true;
		}
		if(unsetBold && unsetItalic && unsetUnderline) {
			break;
		}
	}

	// TODO set/unset join action

	/* split action */
	m_acSplit->setEnabled(selectedItems.count() == 1 &&
		m_Table->columnSpan(selectedItems[0]->row(), selectedItems[0]->column()) > 1);
}

void NewTabularDialog::slotHeaderCustomContextMenuRequested(const QPoint &pos)
{
	int logicalIndex = m_Table->horizontalHeader()->logicalIndexAt(pos);
	if(logicalIndex == -1) return;

	QMenu *popup = static_cast<TabularHeaderItem*>(m_Table->horizontalHeaderItem(logicalIndex))->popupMenu();
	popup->exec(m_Table->horizontalHeader()->mapToGlobal(pos));
}

void NewTabularDialog::slotAlignColumn(int alignment)
{
	TabularHeaderItem *headerItem = static_cast<TabularHeaderItem*>(sender());

	// find column
	for(int column = 0; column < m_Table->columnCount(); ++column) {
		if(m_Table->horizontalHeaderItem(column) == headerItem) {
			for(int row = 0; row < m_Table->rowCount(); ++row) {
				m_Table->item(row, column)->setTextAlignment(Qt::AlignVCenter | alignment);
			}

			break;
		}
	}
}

void NewTabularDialog::slotAlignLeft()
{
	alignItems(Qt::AlignLeft);
}

void NewTabularDialog::slotAlignCenter()
{
	alignItems(Qt::AlignHCenter);
}

void NewTabularDialog::slotAlignRight()
{
	alignItems(Qt::AlignRight);
}

void NewTabularDialog::slotBold()
{
	foreach(QTableWidgetItem *item, m_Table->selectedItems()) {
		QFont font = item->font();
		font.setBold(!font.bold());
		item->setFont(font);
	}
	slotItemSelectionChanged();
}

void NewTabularDialog::slotItalic()
{
	foreach(QTableWidgetItem *item, m_Table->selectedItems()) {
		QFont font = item->font();
		font.setItalic(!font.italic());
		item->setFont(font);
	}
	slotItemSelectionChanged();
}

void NewTabularDialog::slotUnderline()
{
	foreach(QTableWidgetItem *item, m_Table->selectedItems()) {
		QFont font = item->font();
		font.setUnderline(!font.underline());
		item->setFont(font);
	}
	slotItemSelectionChanged();
}

void NewTabularDialog::slotJoinCells()
{
	QList<QTableWidgetItem*> selectedItems = m_Table->selectedItems();
	if(selectedItems.count() < 2) {
		KILE_DEBUG() << "cannot join cells, because selectedItems.count() < 2";
		return;
	}

	/* check whether all selected items are in the same row */
	int row = selectedItems[0]->row();
	for(int i = 1; i < selectedItems.count(); ++i) {
		if(selectedItems[i]->row() != row) {
			KILE_DEBUG() << "cannot join cells, because of different rows";
			return;
		}
	}

	/* check whether all selected items are adjacent */
	QList<int> columns;
	foreach(QTableWidgetItem* item, selectedItems) {
		columns.append(item->column());
	}
	qSort(columns);
	if((columns.last() - columns.first()) != (columns.size() - 1)) {
		KILE_DEBUG() << "cannot join cells, because not all cells are adjacent";
		return;
	}

	int newColumnSpan = columns.size();

	/* check for already joined cells in range */
	foreach(int column, columns) {
		int thisColumnSpan = m_Table->columnSpan(row, column);
		if(thisColumnSpan > 1) {
			newColumnSpan = qMax(newColumnSpan, thisColumnSpan + column - columns.first());
			m_Table->setSpan(row, column, 1, 1);
		}
	}

	/* everything's fine -> join the cells */
	m_Table->setSpan(row, columns.first(), 1, newColumnSpan);

	slotItemSelectionChanged();
}

void NewTabularDialog::slotSplitCells()
{
	/* one item has to be selected */
	if(m_Table->selectedItems().count() != 1) return;

	QTableWidgetItem *selectedItem = m_Table->selectedItems()[0];

	if(m_Table->columnSpan(selectedItem->row(), selectedItem->column()) > 1) {
		m_Table->setSpan(selectedItem->row(), selectedItem->column(), 1, 1);
	}

	slotItemSelectionChanged();
}

void NewTabularDialog::slotFrame(int border)
{
	foreach(QTableWidgetItem *item, m_Table->selectedItems()) {
		static_cast<TabularCell*>(item)->setBorder(border);
	}
}

void NewTabularDialog::slotBackground(const QColor &color)
{
	m_clCurrentBackground = color;
	foreach(QTableWidgetItem *item, m_Table->selectedItems()) {
		item->setBackground(color);
	}
	m_acBackground->setIcon(generateColorIcon(true));
	m_acForeground->setIcon(generateColorIcon(false));
}

void NewTabularDialog::slotForeground(const QColor &color)
{
	m_clCurrentForeground = color;
	foreach(QTableWidgetItem *item, m_Table->selectedItems()) {
		item->setForeground(color);
	}
	m_acBackground->setIcon(generateColorIcon(true));
	m_acForeground->setIcon(generateColorIcon(false));
}

void NewTabularDialog::slotCurrentBackground()
{
	slotBackground(m_clCurrentBackground);
}

void NewTabularDialog::slotCurrentForeground()
{
	slotForeground(m_clCurrentForeground);
}

void NewTabularDialog::slotClearText()
{
	foreach(QTableWidgetItem *item, m_Table->selectedItems()) {
		item->setText(QString());
	}
}

void NewTabularDialog::slotClearAttributes()
{
	foreach(QTableWidgetItem *item, m_Table->selectedItems()) {
		item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		QFont font = item->font();
		font.setBold(false);
		font.setItalic(false);
		font.setUnderline(false);
		item->setFont(font);
		item->setBackground(Qt::white);
		item->setForeground(Qt::black);
	}
}

void NewTabularDialog::slotClearAll()
{
	slotClearText();
	slotClearAttributes();
}

}

#include "newtabulardialog.moc"
