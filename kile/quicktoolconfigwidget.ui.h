/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

void QuickToolConfigWidget::updateSequence(const QString &sequence)
{
	QStringList toollist = KileTool::toolList(KGlobal::config(), true);
	toollist.sort();
	m_cbTools->clear();
	m_cbTools->insertStringList(toollist);

	updateConfigs(m_cbTools->currentText());
	connect(m_cbTools, SIGNAL(activated(const QString &)), this, SLOT(updateConfigs(const QString& )));

	m_sequence=sequence;
	QStringList list = QStringList::split(",",sequence);
	QString tl,cfg;
	m_lstbSeq->clear();
	for ( uint i=0; i < list.count(); i++)
	{
		KileTool::extract(list[i], tl, cfg);
		if ( cfg != QString::null )
			m_lstbSeq->insertItem(tl+" ("+cfg+")");
		else
			m_lstbSeq->insertItem(tl);
	}
}

void QuickToolConfigWidget::updateConfigs(const QString &tool)
{
	m_cbConfigs->clear();
	m_cbConfigs->insertItem(i18n("not specified"));
	m_cbConfigs->insertStringList(KileTool::configNames(tool, KGlobal::config()));
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
	if ( m_cbConfigs->currentText() != i18n("not specified") )
		entry += " (" + m_cbConfigs->currentText() + ")";
	m_lstbSeq->insertItem(entry);
	changed();
}


void QuickToolConfigWidget::changed()
{
	QString sequence, tool, cfg;
	for (uint i = 0; i < m_lstbSeq->count(); i++)
	{
	    KileTool::extract(m_lstbSeq->text(i), tool, cfg);
	    sequence += KileTool::format(tool,cfg)+",";
	}
	if (sequence.endsWith(",") ) sequence = sequence.left(sequence.length()-1);
	m_sequence = sequence;
	emit sequenceChanged(m_sequence);
}
