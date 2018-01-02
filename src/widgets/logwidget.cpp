/**************************************************************************************
    Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                  2008-2012 by Michel Ludwig (michel.ludwig@kdemail.net)
 **************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ****************************************************************************/

#include "widgets/logwidget.h"

#include <QAbstractTextDocumentLayout>
#include <QClipboard>
#include <QHash>
#include <QMenu>
#include <QPainter>
#include <QTextDocument>
#include <QTextStream>

#include <QAction>
#include <KLocalizedString>
#include <KColorScheme>
#include <KStandardAction>
#include <QUrl>

#include "kileconfig.h"
#include "kiledebug.h"
#include "kileinfo.h"
#include "kiletool_enums.h"

namespace KileWidget
{
LogWidgetItemDelegate::LogWidgetItemDelegate(QObject* parent)
    : QItemDelegate(parent)
{
}

QSize LogWidgetItemDelegate::sizeHint(const QStyleOptionViewItem& /* option */,
                                      const QModelIndex& index) const
{
    QTextDocument *textDocument = constructTextDocument(index);
    QSize size = textDocument->documentLayout()->documentSize().toSize();
    delete textDocument;
    return size;
}

void LogWidgetItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const
{
    painter->save();
    QAbstractTextDocumentLayout::PaintContext context;
    QVector<QAbstractTextDocumentLayout::Selection> selectionVector;

    painter->translate(option.rect.x(), option.rect.y());
    QTextDocument *textDocument = constructTextDocument(index);

    if(option.state & QStyle::State_MouseOver && index.data(Qt::UserRole).isValid()) {
        QTextCursor cursor(textDocument);
        cursor.select(QTextCursor::Document);
        QTextCharFormat format;
        format.setFontUnderline(true);
        cursor.mergeCharFormat(format);
    }

    if(option.state & QStyle::State_Selected) {
        QTextCursor cursor(textDocument);
        cursor.setPosition(0);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        QTextCharFormat selectionTextCharFormat;
        selectionTextCharFormat.setFontWeight(QFont::Bold);
        selectionTextCharFormat.setBackground(option.palette.highlight());
        selectionTextCharFormat.setForeground(option.palette.highlightedText());
        QAbstractTextDocumentLayout::Selection selection;
        selection.cursor = cursor;
        selection.format = selectionTextCharFormat;
        selectionVector.push_back(selection);
        context.selections = selectionVector;
    }

    textDocument->documentLayout()->draw(painter, context);
    delete textDocument;

    painter->restore();
}

QTextDocument* LogWidgetItemDelegate::constructTextDocument(const QModelIndex& index) const
{
    QTextDocument *textDocument = new QTextDocument();
    textDocument->setHtml(index.data().toString());
    return textDocument;
}

LogWidget::LogWidget(PopupType popupType, QWidget *parent, const char *name) :
    QListWidget(parent), m_popupType(popupType)
{
    setObjectName(name);
    connect(this, SIGNAL(itemClicked(QListWidgetItem*)),
            this, SLOT(slotItemClicked(QListWidgetItem*)));
    QPalette customPalette = palette();
    customPalette.setColor(QPalette::Window, QColor(Qt::white));
    setPalette(customPalette);
    m_itemDelegate = new LogWidgetItemDelegate(this);
    setSelectionMode(QAbstractItemView::MultiSelection);
    QAbstractItemDelegate *delegate = itemDelegate();
    if(delegate) {
        delete delegate;
    }
    setItemDelegate(m_itemDelegate);
    setMouseTracking(true);
}

LogWidget::~LogWidget()
{
}

bool LogWidget::isShowingOutput() const
{
    return (count() > 0);
}

void LogWidget::highlight(const OutputInfo& info, bool startFromBottom)
{
    for(int i = 0; i < count(); ++i) {
        QListWidgetItem *listItem = item(startFromBottom ? count() - 1 - i : i);
        QVariant variant = listItem->data(Qt::UserRole);
        if(!variant.isValid()) {
            continue;
        }
        OutputInfo info2 = variant.value<OutputInfo>();
        if(info == info2) {
            deselectAllItems();
            scrollToItem(listItem);
            listItem->setSelected(true);
            break;
        }
    }
}

void LogWidget::slotItemClicked(QListWidgetItem *item)
{
    QVariant variant = item->data(Qt::UserRole);
    if(!variant.isValid()) {
        return;
    }

    OutputInfo info = variant.value<OutputInfo>();

    emit(outputInfoSelected(info));
}

void LogWidget::enterEvent(QEvent *)
{
    adaptMouseCursor(mapFromGlobal(QCursor::pos()));
}

void LogWidget::leaveEvent(QEvent *)
{
    unsetCursor();
}

void LogWidget::mouseMoveEvent(QMouseEvent* event)
{
    QPoint p = event->pos();
    adaptMouseCursor(p);
}

void LogWidget::adaptMouseCursor(const QPoint& p)
{
    QListWidgetItem *item = itemAt(p);
    if(!item) {
        unsetCursor();
        return;
    }

    QVariant variant = item->data(Qt::UserRole);
    if(variant.isValid()) {
        setCursor(Qt::PointingHandCursor);
    }
    else {
        unsetCursor();
    }
}

void LogWidget::keyPressEvent(QKeyEvent *event)
{
    QAbstractScrollArea::keyPressEvent(event);
}

void LogWidget::deselectAllItems()
{
    QList<QListWidgetItem*> items = selectedItems();
    for(QList<QListWidgetItem*>::iterator i = items.begin();
            i != items.end(); ++i) {
        QListWidgetItem *item = *i;
        item->setSelected(false);
    }
}

void LogWidget::printMessage(const QString& message)
{
    KILE_DEBUG_MAIN << "\t" << message;
    printMessage(-1, message, QString());
}

void LogWidget::printMessage(int type, const QString& message, const QString &tool,
                             const OutputInfo& outputInfo, bool allowSelection,
                             bool scroll)
{
    QStringList messageList = message.split('\n');
    for(QStringList::iterator it = messageList.begin(); it != messageList.end(); ++it) {
        printMessageLine(type, *it, tool, outputInfo, allowSelection, scroll);
    }
}

void LogWidget::printMessageLine(int type, const QString& message, const QString &tool,
                                 const OutputInfo& outputInfo, bool allowSelection,
                                 bool scroll)
{
    if(type == KileTool::Error) {
        KILE_DEBUG_MAIN << "showing error message emitted";
        emit showingErrorMessage(this);
    }

    QString myMsg = message.toHtmlEscaped();
    QString fontColor;

    switch(type) {
    case KileTool::Warning :
        fontColor = "<font color='" + KStatefulBrush(KColorScheme::View, KColorScheme::NeutralText).brush(this).color().name() + "'>";
        break;
    case KileTool::ProblemWarning :
        if(KileConfig::hideProblemWarning()) {
            return;
        }
        fontColor = "<font color='" + KStatefulBrush(KColorScheme::View, KColorScheme::NeutralText).brush(this).color().name() + "'>";
        break;
    case KileTool::Error: // fall through
    case KileTool::ProblemError:
        fontColor = "<font color='" + KStatefulBrush(KColorScheme::View, KColorScheme::NegativeText).brush(this).color().name() + "'>";
        break;
    case KileTool::ProblemBadBox:
        if (KileConfig::hideProblemBadBox()) {
            return;
        }
        {
            // 'KColorScheme::scheme' doesn't take the background colour into account, so we have to do it manually
            const QColor color = (KStatefulBrush(KColorScheme::View, KColorScheme::NormalBackground).brush(this).color().lightnessF() > 0.5)
                                 ? KColorScheme::shade(KStatefulBrush(KColorScheme::View, KColorScheme::NeutralText).brush(this).color(), KColorScheme::DarkShade)
                                 : KColorScheme::shade(KStatefulBrush(KColorScheme::View, KColorScheme::NeutralText).brush(this).color(), KColorScheme::LightShade);
            fontColor = "<font color='" + color.name() + "'>";
        }
        break;
    default:
        fontColor = "<font color='" + KStatefulBrush(KColorScheme::View, KColorScheme::NormalText).brush(this).color().name() + "'>";
        break;
    }

    QListWidgetItem *item = new QListWidgetItem(this);

    if(tool.isEmpty()) {
        item->setText(fontColor + myMsg + "</font>");
    }
    else {
        item->setText(fontColor + "<b>[" + tool + "]</b> " + myMsg + "</font>");
    }


    if(outputInfo.isValid()) {
        item->setData(Qt::UserRole, QVariant::fromValue(outputInfo));
    }
    if(!allowSelection) {
        // Don't allow the user to manually select this item
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    }

    if((type == KileTool::Error || type == KileTool::ProblemError)
            && !m_firstErrorMessgeInToolLog.isValid()) {
        m_firstErrorMessgeInToolLog = outputInfo;
    }

    if(scroll) {
        scrollToItem(item);
    }
}

void LogWidget::printProblem(int type, const QString& problem, const OutputInfo& outputInfo)
{
    KILE_DEBUG_MAIN << "\t" << problem;
    printMessage(type, problem, QString(), outputInfo);
}

void LogWidget::printProblems(const QList<KileWidget::LogWidget::ProblemInformation>& list)
{
    setUpdatesEnabled(false);
    for(QList<ProblemInformation>::const_iterator i = list.begin(); i != list.end(); ++i) {
        printMessage((*i).type, (*i).message, QString(), (*i).outputInfo, false, false);
    }
    setUpdatesEnabled(true);
    scrollToBottom();
}

void LogWidget::addEmptyLine()
{
    printMessage(-1, QString(), QString());
}

void LogWidget::copy()
{
    QList<QListWidgetItem*> selectedList = selectedItems();
    QString toCopy;
    int maxIndex = 0;
    QHash<int, QListWidgetItem*> itemHash;
    for(QList<QListWidgetItem*>::iterator i = selectedList.begin();
            i != selectedList.end(); ++i) {
        QListWidgetItem* item = *i;
        int row = indexFromItem(item).row();
        itemHash[row] = item;
        maxIndex = qMax(maxIndex, row);
    }
    for(int i = 0; i <= maxIndex; ++i) {
        QHash<int, QListWidgetItem*>::iterator it = itemHash.find(i);
        if(it != itemHash.end()) {
            toCopy += (*it)->data(Qt::UserRole).value<OutputInfo>().message() + '\n';
        }
    }
    if(!toCopy.isEmpty()) {
        QApplication::clipboard()->setText(toCopy);
    }
}

void LogWidget::startToolLogOutput()
{
    m_firstErrorMessgeInToolLog = OutputInfo();
}

void LogWidget::endToolLogOutput()
{
    if(m_firstErrorMessgeInToolLog.isValid()) {
        highlight(m_firstErrorMessgeInToolLog, true);
    }
}

void LogWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu popup;

    QAction *action = KStandardAction::copy(this, SLOT(copy()), this);
    action->setShortcuts(QList<QKeySequence>());
    if(selectedItems().size() == 0) {
        action->setEnabled(false);
    }
    popup.addAction(action);

    action = KStandardAction::selectAll(this, SLOT(selectAll()), this);
    action->setShortcuts(QList<QKeySequence>());
    if(!containsSelectableItems()) {
        action->setEnabled(false);
    }
    popup.addAction(action);


    if(!(m_popupType & NoHideActions)) {
        popup.addSeparator();

        action = new QAction(i18n("Hide &Bad Boxes"), &popup);
        action->setCheckable(true);
        action->setChecked(KileConfig::hideProblemBadBox());
        connect(action, SIGNAL(triggered()), this, SLOT(toggleBadBoxHiding()));
        popup.addAction(action);

        action = new QAction(i18n("Hide (La)TeX &Warnings"), &popup);
        action->setCheckable(true);
        action->setChecked(KileConfig::hideProblemWarning());
        connect(action, SIGNAL(triggered()), this, SLOT(toggleWarningsHiding()));
        popup.addAction(action);
    }

    popup.exec(event->globalPos());
}

void LogWidget::toggleBadBoxHiding()
{
    KileConfig::setHideProblemBadBox(!KileConfig::hideProblemBadBox());
}

void LogWidget::toggleWarningsHiding()
{
    KileConfig::setHideProblemWarning(!KileConfig::hideProblemWarning());
}

bool LogWidget::containsSelectableItems() const
{
    for(int i = 0; i < count(); ++i) {
        if(item(i)->flags() & Qt::ItemIsSelectable) {
            return true;
        }
    }

    return false;
}

}

