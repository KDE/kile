/**************************************************************************
*   Copyright (C) 2007 by Michel Ludwig (michel.ludwig@kdemail.net)       *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "quicktoolconfigwidget.h"

#include "kiletoolmanager.h"

QuickToolConfigWidget::QuickToolConfigWidget(QWidget *parent) : QWidget(parent)
{
	setupUi(this);
}

QuickToolConfigWidget::~QuickToolConfigWidget()
{
}

void QuickToolConfigWidget::updateSequence(const QString &sequence)
{
	QStringList toollist = KileTool::toolList(KGlobal::config().data(), true);
	toollist.sort();
	m_cbTools->clear();
	m_cbTools->insertStringList(toollist);

	updateConfigs(m_cbTools->currentText());
	connect(m_cbTools, SIGNAL(activated(const QString &)), this, SLOT(updateConfigs(const QString& )));

	m_sequence=sequence;
	QStringList list = QStringList::split(",",sequence);
	QString tl,cfg;
	m_lstbSeq->clear();
	for ( uint i=0; i < list.count(); ++i)
	{
		KileTool::extract(list[i], tl, cfg);
		if ( !cfg.isNull() )
			m_lstbSeq->insertItem(tl+" ("+cfg+")");
		else
			m_lstbSeq->insertItem(tl);
	}
}

void QuickToolConfigWidget::updateConfigs(const QString &tool)
{
	m_cbConfigs->clear();
	m_cbConfigs->insertItem(i18n("Not Specified"));
	m_cbConfigs->insertStringList(KileTool::configNames(tool, KGlobal::config().data()));
}

void QuickToolConfigWidget::down()
{
	int current = m_lstbSeq->currentItem();
	if ( (current != -1) && (current < ( ((int)m_lstbSeq->count())-1) ))
	{
		QString text = m_lstbSeq->text(current+1);
		m_lstbSeq->changeItem(m_lstbSeq->text(current), current+1);
		m_lstbSeq->changeItem(text, current);
		m_lstbSeq->setCurrentItem(current+1);
		changed();
	}
}

void QuickToolConfigWidget::up()
{
	int current = m_lstbSeq->currentItem();
	if ( (current != -1) && (current > 0) )
	{
		QString text = m_lstbSeq->text(current-1);
		m_lstbSeq->changeItem(m_lstbSeq->text(current), current-1);
		m_lstbSeq->changeItem(text, current);
		m_lstbSeq->setCurrentItem(current-1);
		changed();
	}
}

void QuickToolConfigWidget::remove()
{
	int current = m_lstbSeq->currentItem();
	if ( current != -1)
	{
		m_lstbSeq->removeItem(current);
		changed();
	}
}

void QuickToolConfigWidget::add()
{
	QString entry = m_cbTools->currentText();
	if ( m_cbConfigs->currentText() != i18n("Not Specified") )
		entry += " (" + m_cbConfigs->currentText() + ")";
	m_lstbSeq->insertItem(entry);
	changed();
}


void QuickToolConfigWidget::changed()
{
	QString sequence, tool, cfg;
	for (uint i = 0; i < m_lstbSeq->count(); ++i)
	{
	    KileTool::extract(m_lstbSeq->text(i), tool, cfg);
	    sequence += KileTool::format(tool,cfg)+",";
	}
	if (sequence.endsWith(",") ) sequence = sequence.left(sequence.length()-1);
	m_sequence = sequence;
	emit sequenceChanged(m_sequence);
}

#include "quicktoolconfigwidget.moc"
