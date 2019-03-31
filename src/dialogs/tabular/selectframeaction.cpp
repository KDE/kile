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

#include "selectframeaction.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#include <QAction>
#include <QDialog>
#include <QIcon>
#include <KLocalizedString>
#include <QMenu>
#include <QPushButton>
#include <KConfigGroup>

#include "tabularcell.h"

namespace KileDialog {

//BEGIN Icons for standard frames
static const char* const all_border_xpm[] = {
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

static const char* const lr_border_xpm[] = {
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

static const char* const tb_border_xpm[] = {
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

static const char* const no_border_xpm[] = {
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
    TabularFrameWidget(QWidget* parent = Q_NULLPTR);
    void setBorder(int value);
    int border() const {
        return m_border;
    }

protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;

private:
    int m_border;
    QRect m_left, m_top, m_right, m_bottom;
};

TabularFrameWidget::TabularFrameWidget(QWidget* parent)
    : QFrame(parent)
{
    m_border = TabularCell::None;

    QPalette p = palette();
    p.setColor(backgroundRole(), QColor(Qt::white));
    setPalette(p);
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
    : KToolBarPopupAction(QIcon(), text, parent),
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
    buttonBox->setLayout(buttonBoxLayout);

    m_pbNone = new QToolButton(buttonBox);
    m_pbLeftRight = new QToolButton(buttonBox);
    m_pbTopBottom = new QToolButton(buttonBox);
    m_pbAll = new QToolButton(buttonBox);

    m_pbNone->setIcon(QIcon(QPixmap(const_cast<const char**>(no_border_xpm))));
    m_pbLeftRight->setIcon(QIcon(QPixmap(const_cast<const char**>(lr_border_xpm))));
    m_pbTopBottom->setIcon(QIcon(QPixmap(const_cast<const char**>(tb_border_xpm))));
    m_pbAll->setIcon(QIcon(QPixmap(const_cast<const char**>(all_border_xpm))));

    buttonBoxLayout->addStretch();
    buttonBoxLayout->addWidget(m_pbNone);
    buttonBoxLayout->addWidget(m_pbLeftRight);
    buttonBoxLayout->addWidget(m_pbTopBottom);
    buttonBoxLayout->addWidget(m_pbAll);
    buttonBoxLayout->addStretch();

    QWidget *frameWidget = new QWidget(page);
    QHBoxLayout *frameWidgetLayout = new QHBoxLayout();
    frameWidgetLayout->setMargin(0);
    frameWidget->setLayout(frameWidgetLayout);

    m_FrameWidget = new TabularFrameWidget(frameWidget);

    frameWidgetLayout->addStretch();
    frameWidgetLayout->addWidget(m_FrameWidget);
    frameWidgetLayout->addStretch();

    m_pbDone = new QPushButton(QIcon::fromTheme("dialog-ok-apply"), i18n("Apply"), page);

    layout->addWidget(buttonBox);
    layout->addWidget(frameWidget);
    layout->addWidget(m_pbDone);

    QWidgetAction *widgetAction = new QWidgetAction(this);
    widgetAction->setDefaultWidget(page);
    menu()->addAction(widgetAction);

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
    menu()->hide();
}

}
