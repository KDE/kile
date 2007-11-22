/***************************************************************************
    begin                : Tuesday Nov 15 2005
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
 
 
#ifndef KILEWIDGETSTATISTICS_H
#define KILEWIDGETSTATISTICS_H

#include <qvariant.h>
#include <qwidget.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3GridLayout>
#include <QLabel>

class Q3VBoxLayout;
class Q3GridLayout;
class QLabel;
class Q3GroupBox;

class KileWidgetStatistics : public QWidget
{
	Q_OBJECT

public:
	KileWidgetStatistics( QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0 );
	~KileWidgetStatistics();

	QLabel* m_commentAboutHelp;
	QLabel* m_warning;

	QLabel* m_wordChar;
	QLabel* m_commandChar;
	QLabel* m_whitespaceChar;
	QLabel* m_totalChar;
	QLabel* m_wordCharText;
	QLabel* m_commandCharText;
	QLabel* m_whitespaceCharText;
	QLabel* m_totalCharText;

	QLabel* m_wordString;
	QLabel* m_environmentString;
	QLabel* m_commandString;
	QLabel* m_totalString;
	QLabel* m_wordStringText;
	QLabel* m_environmentStringText;
	QLabel* m_commandStringText;
	QLabel* m_totalStringText;

	void updateColumns();

private:
	Q3GridLayout *chargrouplayout;
	Q3GridLayout *stringgrouplayout;


};

#endif // KILEWIDGETSTATISTICS_H
