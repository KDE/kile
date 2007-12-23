/***************************************************************************
date                 : Dec 23 2007
version              : 0.22
copyright            : (C) 2004 by Mathias Soeken
email                : msoeken@tzi.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KCATEGORYCOMBOBOX_H
#define KCATEGORYCOMBOBOX_H

#include <KComboBox>

class KDEUI_EXPORT KCategoryComboBox : public KComboBox
{
	public:
		enum {
			Category = Qt::UserRole + 1
		};

	public:
		explicit KCategoryComboBox(QWidget *parent = 0);
		explicit KCategoryComboBox(bool rw, QWidget *parent = 0);

		virtual ~KCategoryComboBox();

	public:
		void addCategoryItem(const QString &text);
};

#endif
