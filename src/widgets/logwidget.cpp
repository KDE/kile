/**************************************************************************************
    begin                : Sat Dec 20 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                               2008 by Michel Ludwig (michel.ludwig@kdemail.net)
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
#include <QPainter>
#include <QTextDocument>
#include <QTextStream>

#include <KUrl>
#include <KLocale>

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

	LogWidget::LogWidget(KileInfo *info, QWidget *parent, const char *name) :
		KListWidget(parent),
		m_info(info)
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

	void LogWidget::highlight(const OutputInfo& info)
	{
		for(int i = 0; i < count(); ++i) {
			QListWidgetItem *listItem = item(i);
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

	void LogWidget::enterEvent(QEvent */* event */)
	{
		adaptMouseCursor(mapFromGlobal(QCursor::pos()));
	}

	void LogWidget::leaveEvent(QEvent */* event */)
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
		KILE_DEBUG() << "\t" << message;
		printMessage(-1, message, QString());
	}

	void LogWidget::printMessage(int type, const QString& message, const QString &tool,
	                             const OutputInfo& outputInfo, bool allowSection)
	{
		if(type == KileTool::Error) {
			emit showingErrorMessage(this);
		}

		QString ot = "", ct = "</font>";

		switch(type) {
			case KileTool::Warning :
				ot = "<font color='blue'>";
				break;
			case KileTool::ProblemWarning :
				if(KileConfig::hideProblemWarning()) {
					return;
				}
				ot = "<font color='blue'>";
				break;
			case KileTool::Error: // fall through
			case KileTool::ProblemError:
				ot = "<font color='red'>";
				break;
			case KileTool::ProblemBadBox:
				if (KileConfig::hideProblemBadBox()) {
					return;
				}
				ot = "<font color='#666666'>";
				break;
			default:
				ot = "<font color='black'>";
				break;
		}

		QListWidgetItem *item = new QListWidgetItem(this);

		if(tool.isEmpty()) {
			item->setText(ot + message + ct);
		}
		else {
			item->setText(ot + "<b>[" + tool + "]</b> " + message + ct);
		}

		if(outputInfo.isValid()) {
			item->setData(Qt::UserRole, QVariant::fromValue(outputInfo));
		}
		else if(!allowSection) {
			// Don't allow the user to manually select this item
			item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
		}

		addItem(item);
		scrollToItem(item);
	}

	void LogWidget::printProblem(int type, const QString& problem, const OutputInfo& outputInfo)
	{
		KILE_DEBUG() << "\t" << problem;
		printMessage(type, problem, QString(), outputInfo);
	}

	void LogWidget::addEmptyLine()
	{
		printMessage(-1, QString(), QString());
	}

#ifdef __GNUC__
#warning Method still needs to be ported!
#endif
//FIXME: port for KDE4
/*
	Q3PopupMenu* LogWidget::createPopupMenu (const QPoint & pos)
	{
		//get standard popup menu
		Q3PopupMenu * popup = K3TextEdit::createPopupMenu(pos);

		//add toggle operations for hiding warnings/badboxes
		popup->insertSeparator();

		m_idBadBox = popup->insertItem(i18n("Hide &Bad Boxes"));
		popup->setItemChecked(m_idBadBox, KileConfig::hideProblemBadBox());

		m_idWarning = popup->insertItem(i18n("Hide (La)TeX &Warnings"));
		popup->setItemChecked(m_idWarning, KileConfig::hideProblemWarning());

		disconnect ( popup , SIGNAL(activated(int)), this , SLOT(handlePopup(int )));
		connect ( popup , SIGNAL(activated(int)), this , SLOT(handlePopup(int )));

		return popup;
	}
*/

	void LogWidget::handlePopup(int id)
	{
		if ( id == m_idBadBox ) KileConfig::setHideProblemBadBox(!KileConfig::hideProblemBadBox());
		else if ( id == m_idWarning ) KileConfig::setHideProblemWarning(!KileConfig::hideProblemWarning());
	}
}

#include "logwidget.moc"
