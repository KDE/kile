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

#include <QTableWidgetItem>

#include "kilewizard.h"

class QAction;
class QCheckBox;
class QComboBox;
class QIcon;
class QLineEdit;
class QMenu;
class QPushButton;
class QSpinBox;
class QTableWidget;
class QToolBar;

class KComboBox;

namespace KileDocument {
class LatexCommands;
}

namespace KileDialog {

class SelectColorAction;
class SelectFrameAction;
class TabularProperties;
class TabularCell;
class TabularTable;

class NewTabularDialog : public Wizard {
    Q_OBJECT

public:
    NewTabularDialog(const QString &environment, KileDocument::LatexCommands *commands, KConfig *config, QWidget *parent = 0);
    ~NewTabularDialog();

    const QStringList& requiredPackages() const;
    QString environment() const;

private:
    void initEnvironments();
    QAction * addAction(const QIcon &icon, const QString &text, const char *method, QObject *parent = Q_NULLPTR);
    QAction * addAction(const QIcon &icon, const QString &text, QObject *receiver, const char *method, QObject *parent = Q_NULLPTR);
    void alignItems(int alignment);
    bool checkForColumnAlignment(int column);
    QIcon generateColorIcon(bool background) const;
    bool canJoin() const;

public Q_SLOTS:
    int exec() override;
    void slotAccepted();

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
    void slotRowAppended();
    void slotColAppended();
    void slotStarredChanged();

private:
    KileDocument::LatexCommands *m_latexCommands;

    QAction *m_acLeft, *m_acCenter, *m_acRight,
            *m_acBold, *m_acItalic, *m_acUnderline,
            *m_acJoin, *m_acSplit,
            *m_acClearText, *m_acClearAttributes, *m_acClearAll,
            *m_acPaste;
    SelectFrameAction *m_acFrame;
    SelectColorAction *m_acBackground, *m_acForeground;
    QToolBar *m_tbFormat;
    TabularTable *m_Table;
    QComboBox *m_cmbName, *m_cmbParameter;
    QSpinBox *m_sbRows, *m_sbCols;
    QCheckBox *m_cbStarred, *m_cbCenter, *m_cbBooktabs, *m_cbBullets;
    QLineEdit *m_leTableWidth;
    QColor m_clCurrentBackground, m_clCurrentForeground;
    QString m_defaultEnvironment;
    QStringList m_requiredPackages;
};

}

#endif
