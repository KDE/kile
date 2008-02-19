/*************************************************************************************
    begin                : Sun Jun 3 2001
    copyright            : (C) 2001 - 2003 by Brachet Pascal
                               2003 Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
 *************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "dialogs/usertagsdialog.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qregexp.h>
#include <qradiobutton.h>
#include <q3buttongroup.h>
//Added by qt3to4:
#include <Q3GridLayout>

#include <kpushbutton.h>
#include <klocale.h>
#include <ktextedit.h>
#include "kiledebug.h"

namespace KileDialog
{

UserTags::UserTags(const QList<KileAction::TagData> &list, QWidget* parent,  const char* name, const QString &caption)
    : 	KDialog(parent), m_list(list)
{
	setObjectName(name);
	setCaption(caption);
	setModal(true);
	setButtons(Apply | Cancel);
	setDefaultButton(Apply);
	showButtonSeparator(true);

 	QWidget *page = new QWidget( this );
	setMainWidget(page);
	Q3GridLayout *gbox = new Q3GridLayout( page, 6, 3,5,5,"");
  	gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );

	m_combo=new QComboBox(page,"combo");
	connect(m_combo, SIGNAL(activated(int)),this,SLOT(change(int)));

	m_labelName = new QLabel( page, "label1" );
	m_labelName->setText(i18n("Menu item:"));
	m_editName =new QLineEdit(page,"name");

	m_labelTag = new QLabel( page, "label2" );
	m_labelTag->setText(i18n("Value:"));
	m_editTag=new KTextEdit(page);
	m_editTag->setObjectName("tag");
	m_editTag->setTextFormat(Qt::PlainText);

	m_buttonAdd = new KPushButton(i18n("Add"),page);
	m_buttonInsert = new KPushButton(i18n("Insert"),page);
	m_buttonRemove = new KPushButton(i18n("Remove"),page);

	connect(m_buttonAdd, SIGNAL(clicked()) , this , SLOT(slotAdd()));
	connect(m_buttonInsert, SIGNAL(clicked()) , this , SLOT(slotInsert()));
	connect(m_buttonRemove, SIGNAL(clicked()) , this , SLOT(slotRemove()));

	gbox->addMultiCellWidget(m_combo,0,0,0,2,0);
	gbox->addMultiCellWidget(m_labelName,1,1,0,2,0);
	gbox->addMultiCellWidget(m_editName,2,2,0,2,0);
	gbox->addMultiCellWidget(m_labelTag,3,3,0,2,0);
	gbox->addMultiCellWidget(m_editTag,4,4,0,2,0);
	gbox->addWidget(m_buttonAdd, 5,0, Qt::AlignLeft);
	gbox->addWidget(m_buttonInsert, 5,1, Qt::AlignLeft);
	gbox->addWidget(m_buttonRemove, 5,2, Qt::AlignLeft);

	resize(350,150);

	m_prevIndex=0;
	redraw();
}

UserTags::~UserTags()
{
}

void UserTags::redraw()
{
	KILE_DEBUG() << QString("usermenudialog redraw() m_prevIndex = %1, m_list.size() = %2").arg(m_prevIndex).arg(m_list.size()) << endl;
	m_combo->clear();

	if (m_list.size() > 0) {
		for (int i = 0; i < m_list.size(); ++i) {
			m_combo->insertItem(QString::number(i + 1) + ": " + m_list[i].text);
		}
		m_combo->setCurrentItem(m_prevIndex);

		m_editTag->setText(completeTag(m_list[m_prevIndex]));
		m_editName->setText(m_list[m_prevIndex].text);
	}
	else {
		m_editTag->setText("");
		m_editName->setText("");
	}
}

void UserTags::change(int index)
{
	KILE_DEBUG() << QString("usermenudialog: change(%1) prev %2").arg(index).arg(m_prevIndex) << endl;
	m_list[m_prevIndex] = splitTag(m_editName->text(), m_editTag->text());

	m_combo->changeItem(QString::number(m_prevIndex+1)+": "+m_list[m_prevIndex].text, m_prevIndex);

	m_editTag->setText( completeTag(m_list[index]) );
	m_editName->setText(m_list[index].text);

	m_prevIndex=index;
}

void UserTags::slotApply()
{
	//store current values before exiting
	if(m_list.count() > 0) {
		m_list[m_prevIndex] = splitTag(m_editName->text(), m_editTag->text());
	}

	KILE_DEBUG() << "usermenudialog: slotApply" << endl;
	accept();
}

void UserTags::slotAdd()
{
	m_list.append(splitTag(m_editName->text(), m_editTag->text()));

	m_prevIndex = m_list.count() - 1;
	redraw();
}

void UserTags::slotInsert()
{
	m_list.insert(m_prevIndex, splitTag(m_editName->text(), m_editTag->text()));
	redraw();
}

void UserTags::slotRemove()
{
	if (m_list.size() > 0) {
		m_list.removeAt(m_prevIndex);

		--m_prevIndex;
		if(m_prevIndex >= static_cast<int>(m_list.count())) {
			m_prevIndex = m_list.count() - 1;
		}

		if(m_prevIndex < 0 ) {
			m_prevIndex=0;
		}

		redraw();
	}
}

QString UserTags::completeTag(const KileAction::TagData & td)
{
	if(td.tagEnd.length() == 0) {
		return td.tagBegin;
	}
	else {
		return td.tagBegin + "%M" + td.tagEnd;
	}
}

KileAction::TagData UserTags::splitTag(const QString & name, const QString & tag)
{
	QStringList parts = tag.split("%M");
	int dx = parts[0].length();
	if(parts[1].length() == 0) {
		int i = parts[0].indexOf(QRegExp("[\\[\\{\\(]"));
		if(i != -1) {
			dx = i + 1;
		}
	}

	return KileAction::TagData(name, parts[0], parts[1], dx, 0, QString());
}

}

#include "usertagsdialog.moc"