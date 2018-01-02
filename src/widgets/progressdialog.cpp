/*
 *  Copyright 2015  Andreas Cord-Landwehr <cordlandwehr@kde.org>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "progressdialog.h"

#include <QCloseEvent>

KileWidget::ProgressDialog::ProgressDialog(QWidget* parent)
    : QProgressDialog(parent)
{
    setCancelButtonText(QString()); // empty string disables cancel button
}

KileWidget::ProgressDialog::~ProgressDialog()
{
}

void KileWidget::ProgressDialog::closeEvent(QCloseEvent *event)
{
    // only allow closing the dialog if progressbar is full
    if (value() >= maximum()) {
        QProgressDialog::closeEvent(event);
    }
    event->ignore();
}
