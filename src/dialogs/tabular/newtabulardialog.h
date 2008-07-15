/***************************************************************************
    begin                : Sunday Jun 27 2008
    copyright            : (C) 2008 by Mathias Soeken
    email                : msoeken@informatik.uni-bremen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef NEW_TABULARDIALOG_H
#define NEW_TABULARDIALOG_H

#include <KToolBarPopupAction>

#include <QTableWidgetItem>

#include <utility>

#include "kilewizard.h"

class QCheckBox;
class QMenu;
class QSpinBox;
class QTableWidget;
class QToolBar;

class KAction;
class KColorCells;
class KComboBox;
class KIcon;
class KPushButton;

namespace KileDocument {
	class LatexCommands;
}

namespace KileDialog {

class TabularFrameWidget;
class TabularProperties;

class SelectFrameAction : public KToolBarPopupAction {
	Q_OBJECT

	public:
		SelectFrameAction(const QString &text, QToolBar *parent);

	private:
		QIcon generateIcon();

	private:
		KPushButton *m_pbNone, *m_pbLeftRight, *m_pbTopBottom, *m_pbAll;
		TabularFrameWidget *m_FrameWidget;
		KPushButton *m_pbDone;
		QToolBar *m_Parent;
		int m_CurrentBorder;

	private Q_SLOTS:
		void slotTriggered();
		void slotNoneClicked();
		void slotLeftRightClicked();
		void slotTopBottomClicked();
		void slotAllClicked();
		void slotDoneClicked();

	Q_SIGNALS:
		void borderSelected(int border);
};

class SelectColorAction : public KToolBarPopupAction {
	Q_OBJECT

	public:
		SelectColorAction(const KIcon &icon, const QString &text, QWidget *parent);

	private Q_SLOTS:
		void slotPopupAboutToShow();
		void slotColorSelected(int index, const QColor &color);
		void slotCustomClicked();

	Q_SIGNALS:
		void colorSelected(const QColor &color);

	private:
		KColorCells *m_ccColors;
		KPushButton *m_pbCustom;
};

class TabularCell : public QTableWidgetItem {
	public:
		enum { None = 0, Left = 1, Top = 2, Right = 4, Bottom = 8 };

		TabularCell();
		TabularCell(const QString &text);

		void setBorder(int border);
		int border() const;
		
		QString toLaTeX( TabularProperties &properties ) const;

	private:
		int m_Border;
};

class TabularHeaderItem : public QObject, public QTableWidgetItem {
	Q_OBJECT

	public:
		enum { AlignP = 0x0200, AlignB = 0x0400, AlignM = 0x0800, AlignX = 0x1000 };

		TabularHeaderItem(QWidget *parent);

		void setAlignment(int alignment);
		int alignment() const;

		bool insertBefore() const;
		bool insertAfter() const;
		bool suppressSpace() const;
		bool dontSuppressSpace() const;

		void setHasXAlignment(bool hasXAlignment);
		bool hasXAlignment() const;

		QMenu* popupMenu() const;

	private:
		void format();
		KIcon iconForAlignment(int alignment) const;

	private Q_SLOTS:
		void slotAlignLeft();
		void slotAlignCenter();
		void slotAlignRight();
		void slotAlignP();
		void slotAlignB();
		void slotAlignM();
		void slotAlignX();
		void slotDeclPre();
		void slotDeclPost();
		void slotDeclAt();
		void slotDeclBang();

	Q_SIGNALS:
		void alignColumn(int alignment);

	private:
		int m_Alignment;
		bool m_InsertBefore, m_InsertAfter, m_SuppressSpace, m_DontSuppressSpace;
		QMenu *m_Popup;
		QAction *m_acXAlignment,
		        *m_acDeclPre, *m_acDeclPost, *m_acDeclAt, *m_acDeclBang;
		bool m_hasXAlignment;
};

/**
 * @brief A helper class for managing multi column borders
 */
class MultiColumnBorderHelper {
	public:
		MultiColumnBorderHelper();
		void addColumn(int column);
		void finish();
		QString toLaTeX() const;

	private:
		QVector<std::pair<int,int> > m_SpanColumns;
		int m_FirstNumber;
		int m_LastNumber;
};

/**
 * @brief This class stores data while generating LaTeX output.
 *
 * This class saves information like whether the \multicolumn command
 * and/or other commands have been used.
 */
class TabularProperties {
	public:
		TabularProperties();

		void setUseMultiColumn(bool useMultiColumn = true);
		bool useMultiColumn() const;

		void addRowColor(int row, const QColor &color);
		void addColor(const QColor &color);
		QColor rowColor(int row) const;
		QString colorName(const QColor &color) const;
		const QHash<QString, QString>& colorNames() const;

		const QStringList& requiredPackages() const;

		void setBullet(const QString &bullet);
		QString bullet() const;

		void addBorderUnderRow(int row);
		bool hasBorderUnderRow(int row) const;
		void setHasTopBorder();
		bool hasTopBorder() const;

		void addBorderBesideColumn(int column);
		bool hasBorderBesideColumn(int column) const;
		void setHasLeftBorder();
		bool hasLeftBorder() const;

	private:
		bool m_UseMultiColumn;
		QHash<int, QColor> m_RowColors;
		QHash<QString, QString> m_ColorNames;
		int m_ColorIndex;
		QStringList m_RequiredPackages;
		QString m_Bullet;
		QList<int> m_BorderUnderRow;
		bool m_TopBorder;
		QList<int> m_BorderBesideColumn;
		bool m_LeftBorder;
};

class NewTabularDialog : public Wizard {
	Q_OBJECT

	public:
		NewTabularDialog(const QString &environment, KileDocument::LatexCommands *commands, KConfig *config, QWidget *parent = 0);
		~NewTabularDialog();

	private:
		void initEnvironments();
		KAction* addAction(const KIcon &icon, const QString &text, const char *method, QObject *parent = 0);
		void alignItems(int alignment);
		bool checkForColumnAlignment(int column);
		QIcon generateColorIcon(bool background) const;

	protected:
		bool eventFilter(QObject *obj, QEvent *event);

	public Q_SLOTS:
		int exec();
		void slotButtonClicked(int button);

	private Q_SLOTS:
		void updateColsAndRows();
		void slotEnvironmentChanged(const QString &environment);
		void slotItemSelectionChanged();
		void slotHeaderCustomContextMenuRequested(const QPoint &pos);
		void slotAlignColumn(int alignment);
		void slotAlignLeft();
		void slotAlignCenter();
		void slotAlignRight();
		void slotBold();
		void slotItalic();
		void slotUnderline();
		void slotJoinCells();
		void slotSplitCells();
		void slotFrame(int border);
		void slotBackground(const QColor &color);
		void slotForeground(const QColor &color);
		void slotCurrentBackground();
		void slotCurrentForeground();
		void slotClearText();
		void slotClearAttributes();
		void slotClearAll();

	private:
		KileDocument::LatexCommands *m_latexCommands;

		KAction *m_acLeft, *m_acCenter, *m_acRight,
		        *m_acBold, *m_acItalic, *m_acUnderline,
		        *m_acJoin, *m_acSplit,
		        *m_acClearText, *m_acClearAttributes, *m_acClearAll;
		SelectFrameAction *m_acFrame;
		SelectColorAction *m_acBackground, *m_acForeground;
		QToolBar *m_tbFormat;
		QTableWidget *m_Table;
		KComboBox *m_cmbName, *m_cmbParameter;
		QSpinBox *m_sbRows, *m_sbCols;
		QCheckBox *m_cbStarred, *m_cbCenter, *m_cbBooktabs, *m_cbBullets;
		QColor m_clCurrentBackground, m_clCurrentForeground;
		QString m_defaultEnvironment;
};

}

#endif
