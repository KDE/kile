/*************************************************************************************
    Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                  2008-2012 by Michel Ludwig (michel.ludwig@kdemail.net)
 *************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ****************************************************************************/

#ifndef LOGWIDGET_H
#define LOGWIDGET_H

#include <QItemDelegate>
#include <QTextDocument>

#include <KListWidget>

#include "outputinfo.h"

class QString;
class QPoint;

class KileInfo;
class KUrl;

namespace KileWidget {
	class LogWidgetItemDelegate : public QItemDelegate
	{
		Q_OBJECT

		public:
			LogWidgetItemDelegate(QObject* parent = NULL);

			virtual QSize sizeHint(const QStyleOptionViewItem& option,
			                       const QModelIndex& index) const;

		protected:
			virtual void paint(QPainter* painter,
			                   const QStyleOptionViewItem& option,
			                   const QModelIndex & index) const;

			QTextDocument* constructTextDocument(const QModelIndex& index) const;
	};

	class LogWidget : public KListWidget
	{
		Q_OBJECT

	public:
		struct ProblemInformation {
			int type;
			QString message;
			OutputInfo outputInfo;
		};

		enum PopupType { AllPopupActions = 0, NoHideActions = 1};

		LogWidget(PopupType popupType = AllPopupActions, QWidget *parent = NULL, const char *name = NULL);
		~LogWidget();

		bool isShowingOutput() const;

	public Q_SLOTS:
		void highlight(const OutputInfo& info, bool startFromBottom = false);

		void printMessage(const QString& message);
		void printMessage(int type, const QString& message, const QString &tool = "Kile",
		                  const OutputInfo& outputInfo = OutputInfo(), bool allowSelection = false,
		                  bool scroll = true);
		void printProblem(int type, const QString& problem, const OutputInfo& outputInfo = OutputInfo());
		void printProblems(const QList<KileWidget::LogWidget::ProblemInformation>& list);

		void addEmptyLine();

		void copy();

		void startToolLogOutput();
		void endToolLogOutput();

	Q_SIGNALS:
		void showingErrorMessage(QWidget*);
		void outputInfoSelected(const OutputInfo&);

	protected:
		virtual void enterEvent(QEvent *event);
		virtual void leaveEvent(QEvent *event);
		virtual void mouseMoveEvent(QMouseEvent* event);

		void adaptMouseCursor(const QPoint& p);
		void keyPressEvent(QKeyEvent *event);

		virtual void contextMenuEvent(QContextMenuEvent *event);

		void printMessageLine(int type, const QString& message, const QString &tool = "Kile",
		                      const OutputInfo& outputInfo = OutputInfo(), bool allowSelection = false,
		                      bool scroll = true);

	protected Q_SLOTS:
		void slotItemClicked(QListWidgetItem *item);
		void deselectAllItems();

		void toggleBadBoxHiding();
		void toggleWarningsHiding();

	private:
		int 			m_popupType;
		int			m_idWarning, m_idBadBox;
		LogWidgetItemDelegate	*m_itemDelegate;
		OutputInfo 		m_firstErrorMessgeInToolLog;

		bool containsSelectableItems() const;
	};
}

#endif
