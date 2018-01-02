/***************************************************************************
    date                 : Dec 06 2005
    version              : 0.12
    copyright            : (C) 2005 by Holger Danielsson
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

#include "dialogs/floatdialog.h"
#include "kiledebug.h"
#include "editorextension.h"
#include <QRegExp>
#include <QDialogButtonBox>
#include <KLocalizedString>
#include <KConfigGroup>


namespace KileDialog
{

FloatEnvironmentDialog::FloatEnvironmentDialog(KConfig *config, KileInfo *ki,
        QWidget *parent)
    : Wizard(config, parent), m_ki(ki)
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(page);

    m_FloatDialog.setupUi(page);

    m_prefix = "fig:";
    m_FloatDialog.m_edLabel->setText(m_prefix);

    slotEnvironmentClicked();
    setFocusProxy(m_FloatDialog.m_edCaption);

    mainLayout->addWidget(buttonBox());
    connect(buttonBox(), &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox(), &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_FloatDialog.m_rbFigure, &QRadioButton::clicked, this, &FloatEnvironmentDialog::slotEnvironmentClicked);
    connect(m_FloatDialog.m_rbTable, &QRadioButton::clicked, this, &FloatEnvironmentDialog::slotEnvironmentClicked);
    connect(this, &QDialog::accepted, this, &FloatEnvironmentDialog::slotAccepted);
}

////////////////////////////// determine the whole tag //////////////////////////////

void FloatEnvironmentDialog::slotAccepted()
{
    QString envname = (m_FloatDialog.m_rbFigure->isChecked()) ? "figure" : "table";
    QString indent = m_ki->editorExtension()->autoIndentEnvironment();

    QString position;
    if (m_FloatDialog.m_cbHere->isChecked())
        position += 'h';
    if (m_FloatDialog.m_cbTop->isChecked())
        position += 't';
    if (m_FloatDialog.m_cbBottom->isChecked())
        position += 'b';
    if (m_FloatDialog.m_cbPage->isChecked())
        position += 'p';

    m_td.tagBegin = "\\begin{" + envname + '}';
    if (!position.isEmpty()) {
        m_td.tagBegin += '[' + position + ']';
    }
    m_td.tagBegin += '\n';

    int row = 1;
    if (m_FloatDialog.m_cbCenter->isChecked()) {
        m_td.tagBegin += indent + "\\centering\n";
        row = 2;
    }

    m_td.tagEnd = indent + '\n';

    QString caption = m_FloatDialog.m_edCaption->text();
    if (! caption.isEmpty())
        m_td.tagEnd += indent  + "\\caption{" + caption + "}\n";

    QString label = m_FloatDialog.m_edLabel->text();
    if (!label.isEmpty() && label != m_prefix)
        m_td.tagEnd += indent + "\\label{" + label + "}\n";

    m_td.tagEnd += "\\end{" + envname + "}\n";

    m_td.dy = row;
    m_td.dx = indent.length();
}

void FloatEnvironmentDialog::slotEnvironmentClicked()
{
    KILE_DEBUG_MAIN << "entering";
    QString caption, oldprefix;

    if (m_FloatDialog.m_rbFigure->isChecked()) {
        caption = i18n("Figure Environment");
        oldprefix = "^tab:";
        m_prefix = "fig:";
    } else {
        caption = i18n("Table Environment");
        oldprefix = "^fig:";
        m_prefix = "tab:";
    }

    setWindowTitle(caption);
    QString s = m_FloatDialog.m_edLabel->text();
    s.replace(QRegExp(oldprefix), m_prefix);
    m_FloatDialog.m_edLabel->setText(s);
}

}
