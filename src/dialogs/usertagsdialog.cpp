/*************************************************************************************
    begin                : Sun Jun 3 2001
    copyright            : (C) 2001 - 2003 by Brachet Pascal
                               2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                               2008 by Michel Ludwig (michel.ludwig@kdemail.net)
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

#include <QHBoxLayout>
#include <QLabel>
#include <QRegExp>
#include <QVBoxLayout>

#include <KComboBox>
#include <KLocale>
#include <KLineEdit>
#include <KPushButton>
#include <KTextEdit>

#include "kiledebug.h"

namespace KileDialog
{

UserTags::UserTags(const QList<KileAction::TagData> &list, QWidget* parent,  const char* name, const QString &caption)
    : KDialog(parent), m_list(list)
{
	setObjectName(name);
	setCaption(caption);
	setModal(true);
	setButtons(Apply | Cancel);
	setDefaultButton(Apply);
	showButtonSeparator(true);

	connect(this, SIGNAL(applyClicked()), this, SLOT(slotApply()));

	QWidget *page = new QWidget(this);
	QVBoxLayout *vBoxLayout = new QVBoxLayout(page);
	vBoxLayout->setMargin(KDialog::marginHint());
	vBoxLayout->setSpacing(KDialog::spacingHint());
	setMainWidget(page);

	m_combo = new KComboBox(page);
	connect(m_combo, SIGNAL(activated(int)), this, SLOT(change(int)));

	m_labelName = new QLabel(page);
	m_labelName->setText(i18n("Menu item:"));
	m_editName = new KLineEdit(page);

	m_labelTag = new QLabel(page);
	m_labelTag->setText(i18n("Value:"));
	m_editTag = new KTextEdit(page);
	m_editTag->setAcceptRichText(false);
	m_editTag->setToolTip(i18n("Available placeholders:\n%B: Bullet\n%C: Cursor position\n%M: Marked text\n%S: Source file name without extension"));

	m_buttonAdd = new KPushButton(i18n("Add"), page);
	m_buttonInsert = new KPushButton(i18n("Insert"), page);
	m_buttonRemove = new KPushButton(i18n("Remove"), page);

	connect(m_buttonAdd, SIGNAL(clicked()), this, SLOT(slotAdd()));
	connect(m_buttonInsert, SIGNAL(clicked()), this, SLOT(slotInsert()));
	connect(m_buttonRemove, SIGNAL(clicked()), this, SLOT(slotRemove()));

	vBoxLayout->addWidget(m_combo);

	QWidget *widget = new QWidget(page);
	QVBoxLayout *vBoxLayout2 = new QVBoxLayout(widget);
	vBoxLayout2->setMargin(0);
	vBoxLayout2->setSpacing(0);
	QWidget *widget2 = new QWidget(widget);
	QHBoxLayout *hBoxLayout = new QHBoxLayout(widget2);
	hBoxLayout->addWidget(m_labelName);
	hBoxLayout->addStretch();
	vBoxLayout2->addWidget(widget2);
	vBoxLayout2->addWidget(m_editName);
	vBoxLayout->addWidget(widget);

	widget = new QWidget(page);
	vBoxLayout2 = new QVBoxLayout(widget);
	vBoxLayout2->setMargin(0);
	vBoxLayout2->setSpacing(0);
	widget2 = new QWidget(widget);
	hBoxLayout = new QHBoxLayout(widget2);
	hBoxLayout->addWidget(m_labelTag);
	hBoxLayout->addStretch();
	vBoxLayout2->addWidget(widget2);
	vBoxLayout2->addWidget(m_editTag);
	vBoxLayout->addWidget(widget);

	widget = new QWidget(page);
	hBoxLayout = new QHBoxLayout(widget);
	hBoxLayout->addStretch();
	hBoxLayout->addWidget(m_buttonAdd);
	hBoxLayout->addStretch();
	hBoxLayout->addWidget(m_buttonInsert);
	hBoxLayout->addStretch();
	hBoxLayout->addWidget(m_buttonRemove);
	hBoxLayout->addStretch();
	vBoxLayout->addWidget(widget);

	m_prevIndex = 0;
	redraw();
}

UserTags::~UserTags()
{
}

void UserTags::redraw()
{
	KILE_DEBUG() << QString("usermenudialog redraw() m_prevIndex = %1, m_list.size() = %2").arg(m_prevIndex).arg(m_list.size());
	m_combo->clear();

	if(m_list.size() > 0) {
		for(int i = 0; i < m_list.size(); ++i) {
			m_combo->addItem(QString::number(i + 1) + ": " + m_list[i].text);
		}
		m_combo->setCurrentIndex(m_prevIndex);

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
	KILE_DEBUG() << QString("usermenudialog: change(%1) prev %2").arg(index).arg(m_prevIndex);
	m_list[m_prevIndex] = splitTag(m_editName->text(), m_editTag->toPlainText());

	m_combo->setItemText(m_prevIndex,
	                     QString::number(m_prevIndex + 1) + ": " + m_list[m_prevIndex].text);

	m_editTag->setText(completeTag(m_list[index]));
	m_editName->setText(m_list[index].text);

	m_prevIndex = index;
}

void UserTags::slotApply()
{
	//store current values before exiting
	if(m_list.count() > 0) {
		m_list[m_prevIndex] = splitTag(m_editName->text(), m_editTag->toPlainText());
	}

	KILE_DEBUG() << "usermenudialog: slotApply";
	accept();
}

void UserTags::slotAdd()
{
	QString name = m_editName->text();
	QString tag = m_editTag->toPlainText();
	if(name.isEmpty() || tag.isEmpty()) {
		return;
	}

	m_list.append(splitTag(name, tag));

	m_prevIndex = m_list.count() - 1;
	redraw();
}

void UserTags::slotInsert()
{
	QString name = m_editName->text();
	QString tag = m_editTag->toPlainText();
	if(name.isEmpty() || tag.isEmpty()) {
		return;
	}

	m_list.insert(m_prevIndex, splitTag(name, tag));
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

		if(m_prevIndex < 0) {
			m_prevIndex = 0;
		}

		redraw();
	}
}

QString UserTags::completeTag(const KileAction::TagData& td)
{
	if(td.tagEnd.length() == 0) {
		return td.tagBegin;
	}
	else {
		return td.tagBegin + "%M" + td.tagEnd;
	}
}

KileAction::TagData UserTags::splitTag(const QString& name, const QString& tag)
{
	QStringList parts = tag.split("%M");
	int dx = parts[0].length();
	QString secondPart;
	if(parts.size() >= 2 && parts[1].length() > 0) {
		secondPart = parts[1];
		int i = parts[0].indexOf(QRegExp("[\\[\\{\\(]"));
		if(i != -1) {
			dx = i + 1;
		}
	}

	return KileAction::TagData(name, parts[0], secondPart, dx, 0, QString());
}

}

#include "usertagsdialog.moc"
