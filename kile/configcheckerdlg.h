/***************************************************************************
    begin                : Fri Jun 4 2004
    copyright            : (C) 2004 by Jeroen Wijnout
    email                : Jeroen.Wijnhout@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KILEDIALOG_CONFIGCHECKER_H
#define KILEDIALOG_CONFIGCHECKER_H

#include <q3simplerichtext.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <QLabel>

#include <kdialog.h>

#include "configtester.h"

class QLabel;
class QPainter;

class KProgress;
class K3ListBox;

class ConfigCheckerWidget;

namespace KileDialog
{
	class ResultItem : public Q3ListBoxItem
	{
	public:
		ResultItem(K3ListBox *lb, const QString &tool, int status, const Q3ValueList<ConfigTest> &tests);
		int width(const Q3ListBox *) const { return m_richText->widthUsed(); }
		int height(const Q3ListBox *) const { return m_richText->height(); }

	protected:
		void paint(QPainter *);

	private:
		Q3SimpleRichText	*m_richText;
	};

	class ConfigChecker : public KDialog
	{
		Q_OBJECT

	public:
		ConfigChecker(QWidget* parent = 0);
		~ConfigChecker();

	public slots:
		void run();
		void started();
		void finished(bool);
		void setPercentageDone(int);
		void saveResults();
		void slotCancel();

	private:
		KProgress* progressBar();
		QLabel* label();
		K3ListBox* listBox();

	private:
		ConfigCheckerWidget	*m_widget;
		Tester				*m_tester;
	};
}
#endif
