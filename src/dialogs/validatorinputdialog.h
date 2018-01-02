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


#ifndef KILEVALIDATORINPUTDIALOG_H
#define KILEVALIDATORINPUTDIALOG_H

#include <QDialog>

class QDialogButtonBox;
class QLineEdit;
class QString;
class QWidget;
class QValidator;

namespace KileDialog {
/**
 * Static convenience function to get a validated string from the user.
 *
 * @p caption is the text that is displayed in the title bar. @p label is the
 * text that appears as a label for the line edit. @p value is the initial
 * value of the line edit.
 *
 * If you provide a validator, the Ok button is disabled as long as
 * the validator doesn't return Acceptable. If there is no validator,
 * the Ok button is enabled whenever the line edit isn't empty. If you
 * want to accept empty input, create a trivial QValidator that
 * always returns acceptable, e.g. QRegExpValidator with a regexp
 * of ".*".
 *
 * @param caption   Caption of the dialog
 * @param label     Text of the label for the line edit
 * @param value     Initial value of the line edit
 * @param parent    Parent of the dialog widget
 * @param validator A @ref QValidator to be associated with the line edit
 * @param mask      Mask associated with the line edit. See the
 *                  documentation for @ref QLineEdit about masks
 * @return String user entered if Ok was pressed, else a null string
 */
QString getText(const QString &caption, const QString &label,
                const QString &value = QString(), QWidget *parent = Q_NULLPTR,
                QValidator *validator = 0,
                const QString &mask = QString());
}

#endif
