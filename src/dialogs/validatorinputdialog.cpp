/*
 *  Copyright 2003  Nadeem Hasan <nhasan@kde.org>
 *  Copyright 2015  Andreas Cord-Landwehr <cordlandwehr@kde.org>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "validatorinputdialog.h"
#include "validatorinputdialog_p.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QValidator>
#include <QVBoxLayout>

using namespace KileDialog;

ValidatorInputDialogHelper::ValidatorInputDialogHelper(const QString &caption, const QString &label,
        const QString &value, QWidget *parent,
        QValidator *validator, const QString &mask)
    : QDialog(parent)
    , m_lineEdit(new QLineEdit(this))
    , m_buttonBox(new QDialogButtonBox(this))
{
    setWindowTitle(caption);
    setModal(true);

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    QLabel *m_label = new QLabel(label, this);
    m_label->setWordWrap(true);
    layout->addWidget(m_label);

    m_lineEdit->setText(value);
    m_lineEdit->setClearButtonEnabled(true);
    layout->addWidget(m_lineEdit);

    m_lineEdit->setFocus();
    m_label->setBuddy(m_lineEdit);

    layout->addStretch();

    m_buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(m_buttonBox);

    if (validator) {
        m_lineEdit->setValidator(validator);
    }

    if (!mask.isEmpty()) {
        m_lineEdit->setInputMask(mask);
    }

    connect(m_lineEdit, &QLineEdit::textChanged, this, &ValidatorInputDialogHelper::slotEditTextChanged);

    slotEditTextChanged(value);
    setMinimumWidth(350);
}

QLineEdit * ValidatorInputDialogHelper::lineEdit() const
{
    return m_lineEdit;
}

void ValidatorInputDialogHelper::slotEditTextChanged(const QString &text)
{
    bool enabled;

    if (m_lineEdit->validator()) {
        QString str = m_lineEdit->text();
        int index = m_lineEdit->cursorPosition();
        enabled = (m_lineEdit->validator()->validate(str, index) == QValidator::Acceptable);
    } else {
        enabled = !text.trimmed().isEmpty();
    }

    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enabled);
}

QString KileDialog::getText(const QString &caption,
                            const QString &label, const QString &value, QWidget *parent,
                            QValidator *validator, const QString &mask)
{
    ValidatorInputDialogHelper dlg(caption, label, value, parent, validator, mask);

    bool ok = (dlg.exec() == QDialog::Accepted);

    QString result;
    if (ok) {
        result = dlg.lineEdit()->text();
    }

    //  validator may explicitly allow leading and trailing whitespace
    if (!validator) {
        result = result.trimmed();
    }

    return result;
}

#include "moc_validatorinputdialog_p.cpp"
