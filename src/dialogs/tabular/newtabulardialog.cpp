/********************************************************************************************
  Copyright (C) 2008 by Mathias Soeken (msoeken@informatik.uni-bremen.de)
            (C) 2005-2006 by Holger Danielsson (holger.danielsson@t-online.de)
            (C) 2012 by Michel Ludwig (michel.ludwig@kdemail.net)
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

#include <algorithm>

#include <QAction>
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QIcon>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QList>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QToolBar>
#include <QVBoxLayout>

#include <KConfigGroup>
#include <KIconLoader>
#include <KLocalizedString>
#include <KMessageBox>

#include "codecompletion.h"
#include "kiledebug.h"
#include "latexcmd.h"

#include "multicolumnborderhelper.h"
#include "selectcoloraction.h"
#include "selectframeaction.h"
#include "tabularcell.h"
#include "tabularcelldelegate.h"
#include "tabularheaderitem.h"
#include "tabularproperties.h"
#include "tabulartable.h"

namespace KileDialog {

NewTabularDialog::NewTabularDialog(const QString &environment, KileDocument::LatexCommands *commands, KConfig *config, QWidget *parent)
    : Wizard(config, parent),
      m_latexCommands(commands),
      m_clCurrentBackground(Qt::white),
      m_clCurrentForeground(Qt::black),
      m_defaultEnvironment(environment)
{
    setWindowTitle(i18n("Tabular Environments"));

    QWidget *page = new QWidget(this);
    QVBoxLayout *pageLayout = new QVBoxLayout();
    pageLayout->setMargin(0);
    page->setLayout(pageLayout);

    m_Table = new TabularTable(page);

    m_tbFormat = new QToolBar(page);
    m_tbFormat->setMovable(false);
    m_tbFormat->setFloatable(false);
    m_tbFormat->setOrientation(Qt::Horizontal);

    m_acLeft = addAction(QIcon::fromTheme("format-justify-left"), i18n("Align Left"), SLOT(slotAlignLeft()), page);
    m_acCenter = addAction(QIcon::fromTheme("format-justify-center"), i18n("Align Center"), SLOT(slotAlignCenter()), page);
    m_acRight = addAction(QIcon::fromTheme("format-justify-right"), i18n("Align Right"), SLOT(slotAlignRight()), page);
    m_tbFormat->addSeparator();
    m_acBold = addAction(QIcon::fromTheme("format-text-bold"), i18n("Bold"), SLOT(slotBold()), page);
    m_acItalic = addAction(QIcon::fromTheme("format-text-italic"), i18n("Italic"), SLOT(slotItalic()), page);
    m_acUnderline = addAction(QIcon::fromTheme("format-text-underline"), i18n("Underline"), SLOT(slotUnderline()), page);
    m_tbFormat->addSeparator();
    m_acJoin = addAction(QIcon::fromTheme("joincells"), i18n("Join Cells"), SLOT(slotJoinCells()), page);
    m_acSplit = addAction(QIcon::fromTheme("splitcells"), i18n("Split Cells"), SLOT(slotSplitCells()), page);
    m_acSplit->setEnabled(false);
    m_acFrame = new SelectFrameAction(i18n("Edit Frame"), m_tbFormat);
    connect(m_acFrame, SIGNAL(borderSelected(int)), this, SLOT(slotFrame(int)));
    m_tbFormat->addAction(m_acFrame);
    m_tbFormat->addSeparator();

    m_acBackground = new SelectColorAction(QIcon::fromTheme("format-fill-color"), i18n("Background Color"), page);
    m_acBackground->setIcon(generateColorIcon(true));
    connect(m_acBackground, SIGNAL(triggered(bool)), this, SLOT(slotCurrentBackground()));
    connect(m_acBackground, SIGNAL(colorSelected(const QColor&)), this, SLOT(slotBackground(const QColor&)));
    m_tbFormat->addAction(m_acBackground);
    m_acForeground = new SelectColorAction(QIcon::fromTheme("format-stroke-color"), i18n("Text Color"), page);
    m_acForeground->setIcon(generateColorIcon(false));
    connect(m_acForeground, SIGNAL(colorSelected(const QColor&)), this, SLOT(slotForeground(const QColor&)));
    connect(m_acForeground, SIGNAL(triggered(bool)), this, SLOT(slotCurrentForeground()));
    m_tbFormat->addAction(m_acForeground);

    m_tbFormat->addSeparator();
    m_acClearText = addAction(QIcon::fromTheme("edit-clear"), i18n("Clear Text"), SLOT(slotClearText()), page); // FIXME icon
    m_acClearAttributes = addAction(QIcon::fromTheme("edit-clear"), i18n("Clear Attributes"), SLOT(slotClearAttributes()), page); // FIXME icon
    m_acClearAll = addAction(QIcon::fromTheme("edit-clear"), i18n("Clear All"), SLOT(slotClearAll()), page);
    m_tbFormat->addSeparator();
    m_acPaste = addAction(QIcon::fromTheme("edit-paste"), i18n("Paste content from clipboard"), m_Table, SLOT(paste()), page);

    /* checkable items */
    m_acLeft->setCheckable(true);
    m_acCenter->setCheckable(true);
    m_acRight->setCheckable(true);
    m_acBold->setCheckable(true);
    m_acItalic->setCheckable(true);
    m_acUnderline->setCheckable(true);

    QGroupBox *configPage = new QGroupBox(i18n("Environment"), page);
    QGridLayout *configPageLayout = new QGridLayout();
    configPage->setLayout(configPageLayout);

    QLabel *label = new QLabel(i18n("Name:"), configPage);
    m_cmbName = new QComboBox(configPage);
    label->setBuddy(m_cmbName);
    configPageLayout->addWidget(label, 0, 0);
    configPageLayout->addWidget(m_cmbName, 0, 1);
    label = new QLabel(i18n("Parameter:"), configPage);
    m_cmbParameter = new QComboBox(configPage);
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
    label = new QLabel(i18n("Table width:"), configPage);
    m_leTableWidth = new QLineEdit(configPage);
    m_leTableWidth->setEnabled(false);
    connect(m_cbStarred, SIGNAL(stateChanged(int)), this, SLOT(slotStarredChanged()));
    label->setBuddy(m_leTableWidth);
    m_cbCenter = new QCheckBox(i18n("Center"), configPage);
    m_cbCenter->setChecked(true);
    m_cbBooktabs = new QCheckBox(i18n("Use booktabs package"), configPage);
    m_cbBullets = new QCheckBox(i18n("Insert bullets"), configPage);
    m_cbBullets->setChecked(true);
    configPageLayout->addWidget(m_cbStarred, 2, 0, 1, 2);
    configPageLayout->addWidget(label, 2, 2, 1, 1);
    configPageLayout->addWidget(m_leTableWidth, 2, 3, 1, 1);
    configPageLayout->addWidget(m_cbCenter, 3, 0, 1, 2);
    configPageLayout->addWidget(m_cbBooktabs, 3, 2, 1, 2);
    configPageLayout->addWidget(m_cbBullets, 4, 0, 1, 2);

    // whats this texts
    m_Table->setWhatsThis(i18n("Input data. Enter text when a cell is selected. When return is pressed, the adjacent cell will become selected."));
    m_cmbName->setWhatsThis(i18n("Choose an environment."));
    m_cmbParameter->setWhatsThis(i18n("Optional parameter for the chosen environment."));
    m_sbRows->setWhatsThis(i18n("Choose the number of table rows."));
    m_sbCols->setWhatsThis(i18n("Choose the number of table columns."));
    m_cbCenter->setWhatsThis(i18n("The tabular will be centered."));
    m_cbBooktabs->setWhatsThis(i18n("Use line commands of the booktabs package."));
    m_cbStarred->setWhatsThis(i18n("Use the starred version of this environment."));
    m_leTableWidth->setWhatsThis(i18n("Set the width of the table."));
    m_cbBullets->setWhatsThis(i18n("Insert bullets in each cell. Alt+Ctrl+Right and Alt+Ctrl+Left will move very quickly from one cell to another."));
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
    m_acClearText->setWhatsThis(i18n("Clears the text of the selected cells but keeps attributes such as alignment and font shape."));
    m_acClearAttributes->setWhatsThis(i18n("Resets the attributes of the selected cells to the default values but keeps the text."));
    m_acClearAll->setWhatsThis(i18n("Clears the text of the selected cells and resets the attributes."));
    m_acPaste->setWhatsThis(i18n("Pastes a table stored in the clipboard into this wizard."));

    pageLayout->addWidget(m_tbFormat);
    pageLayout->addWidget(m_Table);
    pageLayout->addWidget(configPage);
    pageLayout->addWidget(buttonBox());
    setLayout(pageLayout);

    initEnvironments();
    updateColsAndRows();
    m_Table->item(0, 0)->setSelected(true);

    connect(m_Table, &KileDialog::TabularTable::itemSelectionChanged, this, &NewTabularDialog::slotItemSelectionChanged);
    connect(m_Table, &KileDialog::TabularTable::rowAppended, this, &NewTabularDialog::slotRowAppended);
    connect(m_Table, &KileDialog::TabularTable::colAppended, this, &NewTabularDialog::slotColAppended);
    connect(m_cmbName, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::activated), this, &NewTabularDialog::slotEnvironmentChanged);
    connect(m_sbCols, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &NewTabularDialog::updateColsAndRows);
    connect(m_sbRows, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &NewTabularDialog::updateColsAndRows);
    connect(m_Table->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &NewTabularDialog::slotHeaderCustomContextMenuRequested);
    connect(this, &QDialog::accepted, this, &NewTabularDialog::slotAccepted);
}

NewTabularDialog::~NewTabularDialog()
{
}

const QStringList& NewTabularDialog::requiredPackages() const
{
    return m_requiredPackages;
}

QString NewTabularDialog::environment() const
{
    return m_cmbName->currentText();
}

void NewTabularDialog::initEnvironments()
{
    /* read all tabular environments and insert them into the combobox */
    QStringList list;
    QStringList::ConstIterator it;
    m_latexCommands->commandList(list, KileDocument::CmdAttrTabular, false);
    m_cmbName->addItems(list);

    // set default environment
    int index = m_cmbName->findText(m_defaultEnvironment);
    if(index != -1) {
        m_cmbName->setCurrentIndex(index);
    } else {
        if(m_defaultEnvironment == "array") {
            m_cmbName->insertItem(0, "array");
            m_cmbName->setCurrentIndex(0);
        }
    }

    // refresh other gui elements regarding environment combo box
    slotEnvironmentChanged(m_cmbName->currentText());
}

QAction * NewTabularDialog::addAction(const QIcon &icon, const QString &text, const char *method, QObject *parent)
{
    return addAction(icon, text, this, method, parent);
}

QAction * NewTabularDialog::addAction(const QIcon &icon, const QString &text, QObject *receiver, const char *method, QObject *parent)
{
    QAction *action = new QAction(icon, text, parent);
    connect(action, SIGNAL(triggered(bool)), receiver, method);
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

bool NewTabularDialog::canJoin() const
{
    const QList<QTableWidgetItem*> selectedItems = m_Table->selectedItems();
    if(selectedItems.count() < 2) {
        KILE_DEBUG_MAIN << "cannot join cells, because selectedItems.count() < 2";
        return false;
    }

    /* check whether all selected items are in the same row */
    int row = selectedItems[0]->row();
    for(int i = 1; i < selectedItems.count(); ++i) {
        if(selectedItems[i]->row() != row) {
            KILE_DEBUG_MAIN << "cannot join cells, because of different rows";
            return false;
        }
    }

    /* check whether all selected items are adjacent */
    QList<int> columns;
    for(QTableWidgetItem* item : selectedItems) {
        columns.append(item->column());
    }
    std::sort(columns.begin(), columns.end());
    if((columns.last() - columns.first()) != (columns.size() - 1)) {
        KILE_DEBUG_MAIN << "cannot join cells, because not all cells are adjacent";
        return false;
    }

    return true;
}

int NewTabularDialog::exec()
{
    show();
    return Wizard::exec();
}

void NewTabularDialog::slotAccepted()
{
    int rows = m_Table->rowCount();
    int columns = m_Table->columnCount();
    TabularProperties properties;

    //BEGIN preprocessing colors and border
    QColor firstColor;
    bool topBorder = true;
    for(int row = 0; row < rows; ++row) {
        bool sameColor = true;
        bool borderUnderRow = true;
        {
            const QBrush backgroundBrush = m_Table->item(row, 0)->background();
            if(backgroundBrush.style() != Qt::NoBrush) {
                firstColor = backgroundBrush.color();
            }
        }
        for(int column = 0; column < columns; ++column) {
            TabularCell *cell = static_cast<TabularCell*>(m_Table->item(row, column));

            // Adjust right and bottom border for current item
            if(column < columns - 1) {
                TabularCell *next = static_cast<TabularCell*>(m_Table->item(row, column + 1));
                if(next->border() & TabularCell::Left) {
                    cell->setBorder(cell->border() | TabularCell::Right);
                }
            }
            if(row < rows - 1) {
                TabularCell *next = static_cast<TabularCell*>(m_Table->item(row + 1, column));
                if(next->border() & TabularCell::Top) {
                    cell->setBorder(cell->border() | TabularCell::Bottom);
                }
            }

            const QBrush backgroundBrush = m_Table->item(row, column)->background();
            if(backgroundBrush.style() != Qt::NoBrush) {
                QColor currentColor = backgroundBrush.color();
                properties.addColor(currentColor);
                if(currentColor != firstColor) {
                    sameColor = false;
                }
            }

            const QBrush foregroundBrush = m_Table->item(row, column)->foreground();
            if(foregroundBrush.style() != Qt::NoBrush) {
                properties.addColor(foregroundBrush.color());
            }

            if(!(cell->border() & TabularCell::Bottom)) {
                borderUnderRow = false;
            }
            if (row == 0 && !(cell->border() & TabularCell::Top)) {
                topBorder = false;
            }
        }
        if(sameColor) {
            properties.addRowColor(row, firstColor);
        }
        if(borderUnderRow) {
            properties.addBorderUnderRow(row);
        }
    }

    if(topBorder) {
        properties.setHasTopBorder();
    }

    bool leftBorder = true;
    for(int column = 0; column < columns; ++column) {
        bool borderBesideColumn = true;
        for(int row = 0; row < rows; ++row) {
            TabularCell *cell = static_cast<TabularCell*>(m_Table->item(row, column));

            if(!(cell->border() & TabularCell::Right)) {
                borderBesideColumn = false;
            }
            if (column == 0 && !(cell->border() & TabularCell::Left)) {
                leftBorder = false;
            }
        }
        if(borderBesideColumn) {
            properties.addBorderBesideColumn(column);
        }
    }

    if(leftBorder) {
        properties.setHasLeftBorder();
    }
    //END

    /* bullet */
    if(m_cbBullets->isChecked()) {
        properties.setBullet(s_bullet);
    }

    /* environment */
    QString environment = m_cmbName->currentText();
    QString environmentFormatted = environment;
    QString tableWidth;
    if(m_cbStarred->isEnabled() && m_cbStarred->isChecked()) {
        environmentFormatted += '*';
    }

    // Environment needs a width
    if(m_leTableWidth->isEnabled()) {
        tableWidth = '{' + m_leTableWidth->text() + '}';
    }

    /* build table parameter */
    QString tableParameter;
    if(m_cmbParameter->currentIndex() != 0) {
        tableParameter = '[' + m_cmbParameter->currentText() + ']';
    }

    /* build table alignment */
    QString tableAlignment = QString('{');
    if(properties.hasLeftBorder()) {
        tableAlignment += '|';
    }
    for(int column = 0; column < columns; ++column) {
        TabularHeaderItem *headerItem = static_cast<TabularHeaderItem*>(m_Table->horizontalHeaderItem(column));
        if(headerItem->suppressSpace()) {
            tableAlignment += QString("@{%1}").arg(properties.bullet());
        } else if(headerItem->dontSuppressSpace()) {
            tableAlignment += QString("!{%1}").arg(properties.bullet());
        }
        if(headerItem->insertBefore()) {
            tableAlignment += QString(">{%1}").arg(properties.bullet());
        }

        switch(headerItem->alignment()) {
        case Qt::AlignLeft:
            tableAlignment += 'l';
            break;
        case Qt::AlignHCenter:
            tableAlignment += 'c';
            break;
        case Qt::AlignRight:
            tableAlignment += 'r';
            break;
        case TabularHeaderItem::AlignP:
            tableAlignment += QString("p{%1}").arg(properties.bullet());
            break;
        case TabularHeaderItem::AlignB:
            tableAlignment += QString("b{%1}").arg(properties.bullet());
            break;
        case TabularHeaderItem::AlignM:
            tableAlignment += QString("m{%1}").arg(properties.bullet());
            break;
        case TabularHeaderItem::AlignX:
            tableAlignment += 'X';
            break;
        }

        if(headerItem->insertAfter()) {
            tableAlignment += QString("<{%1}").arg(properties.bullet());
        }

        if(properties.hasBorderBesideColumn(column)) {
            tableAlignment += '|';
        }
    }
    tableAlignment += '}';

    /* build top border */
    QString topBorderStr;
    if(properties.hasTopBorder()) {
        if(m_cbBooktabs->isChecked()) { // we need a toprule with booktabs here
            topBorderStr = "\\toprule";
        }
        else {
            topBorderStr = "\\hline";
        }
    }
    else {
        MultiColumnBorderHelper topBorderHelper;
        for(int column = 0; column < columns; ++column) {
            TabularCell *cell = static_cast<TabularCell*>(m_Table->item(0, column));
            if(cell->border() & TabularCell::Top) {
                topBorderHelper.addColumn(column);
            }
        }
        topBorderHelper.finish();
        topBorderStr = topBorderHelper.toLaTeX();
    }

    if(m_cbCenter->isChecked()) {
        m_td.tagBegin += "\\begin{center}\n";
    }

    m_td.tagBegin += QString("\\begin{%1}%2%3%4%5\n")
                     .arg(environmentFormatted)
                     .arg(tableWidth)
                     .arg(tableParameter)
                     .arg(tableAlignment)
                     .arg(topBorderStr);

    /* required packages */
    m_requiredPackages.clear();
    if(properties.requiredPackages().count()) {
        m_td.tagBegin += "% use packages: " + properties.requiredPackages().join(",") + '\n';
        m_requiredPackages << properties.requiredPackages();
    }

    QColor rowColor;
    for(int row = 0; row < rows; ++row) {
        rowColor = properties.rowColor(row);
        if(rowColor.isValid()) {
            m_td.tagBegin += "\\rowcolor{" + properties.colorName(rowColor) + "}\n";
        }
        MultiColumnBorderHelper columnBorderHelper;
        for(int column = 0; column < columns;) {
            TabularCell *cell = static_cast<TabularCell*>(m_Table->item(row, column));
            QString content = cell->toLaTeX(properties);
            int columnSpan = m_Table->columnSpan(row, column);

            if(!properties.hasBorderUnderRow(row) && (cell->border() & TabularCell::Bottom)) {
                for(int c2 = 0; c2 < columnSpan; ++c2) {
                    columnBorderHelper.addColumn(column + c2);
                }
            }

            QString sep = " & ";
            if(column + columnSpan >= columns) {
                QString end;
                sep.clear();
                if(properties.hasBorderUnderRow(row)) {
                    if(m_cbBooktabs->isChecked()) { // we need a midrule with booktabs.
                        if(row < rows-1) {
                            end = "\\midrule";
                        }
                        else { // last line gets a bottomrule
                            end = "\\bottomrule";
                        }
                    }
                    else {
                        end = "\\hline";
                    }
                }
                else {
                    columnBorderHelper.finish();
                    end = columnBorderHelper.toLaTeX();
                }
                if(row < rows - 1 || !end.isEmpty()) {
                    sep = "\\\\";
                }
                sep += end + '\n';
            }
            m_td.tagBegin += content + sep;

            column += columnSpan;
        }
    }

    m_td.tagEnd += QString("\\end{%1}\n").arg(environmentFormatted);

    if(m_cbCenter->isChecked()) {
        m_td.tagEnd += "\\end{center}\n";
    }

    QHashIterator<QString, QString> itColorName(properties.colorNames());
    QString colorNames = "";
    while(itColorName.hasNext()) {
        itColorName.next();
        colorNames += "\\definecolor{" + itColorName.value() + "}{rgb}{";
        QColor color(itColorName.key());
        colorNames += QString::number(color.redF()) + ','
                      + QString::number(color.greenF()) + ','
                      + QString::number(color.blueF()) + "}\n";
    }
    m_td.tagBegin = colorNames + m_td.tagBegin;

    if(properties.useMultiColumn()) {
        m_td.tagBegin = "\\newcommand{\\mc}[3]{\\multicolumn{#1}{#2}{#3}}\n"
                        + m_td.tagBegin;
    }

    /* use {} if mc was defined */
    if(properties.useMultiColumn()) {
        m_td.tagBegin = "{%\n" + m_td.tagBegin;
        m_td.tagEnd += "}%\n";
    }
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
    // disable table width line edit
    m_leTableWidth->setEnabled(false);

    // look for environment parameter in dictionary
    KileDocument::LatexCmdAttributes attr;
    if(m_latexCommands->commandAttributes(environment, attr)) {
        // starred version
        m_cbStarred->setEnabled(attr.starred);
        slotStarredChanged();

        // option
        if(attr.option.indexOf('[') == 0) {
            QStringList optionlist = attr.option.split("");
            optionlist.removeAll("");
            if(optionlist.count() > 2) {
                // ok, let's enable it
                m_cmbParameter->setEnabled(true);
                m_cmbParameter->addItem(QString());
                // insert some options
                for(int i = 1; i < optionlist.count() - 1; ++i) {
                    m_cmbParameter->addItem(optionlist[i]);
                }
            }
        }

        // enable table width line edit if needed
        if( attr.parameter.indexOf('{') == 0 ) {
            m_leTableWidth->setEnabled(true);
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

    m_acJoin->setEnabled(canJoin());

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
    if(!canJoin()) return;

    const QList<QTableWidgetItem*> selectedItems = m_Table->selectedItems();
    int row = selectedItems[0]->row();

    QList<int> columns;
    for(QTableWidgetItem* item : selectedItems) {
        columns.append(item->column());
    }
    std::sort(columns.begin(), columns.end());

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
        item->setBackground(QBrush());
        item->setForeground(QBrush());
    }
}

void NewTabularDialog::slotClearAll()
{
    slotClearText();
    slotClearAttributes();
}

void NewTabularDialog::slotRowAppended()
{
    const int newValue = m_sbRows->value() + 1;

    m_sbRows->setMaximum(qMax(m_sbRows->maximum(), newValue));
    m_sbRows->setValue(newValue);

    updateColsAndRows();
}

void NewTabularDialog::slotColAppended()
{
    const int newValue = m_sbCols->value() + 1;

    m_sbCols->setMaximum(qMax(m_sbCols->maximum(), newValue));
    m_sbCols->setValue(newValue);

    updateColsAndRows();
}

}

void KileDialog::NewTabularDialog::slotStarredChanged()
{
    m_leTableWidth->setEnabled(m_cbStarred->isChecked() && m_cbStarred->isEnabled());
}
