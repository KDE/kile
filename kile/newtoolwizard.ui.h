/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

void NewToolWizard::init()
{
	m_toolList = KileTool::toolList(KGlobal::config(), false);

	//setup the Name page (page 0)
	connect(m_leName, SIGNAL(textChanged(const QString &)), this, SLOT(nameChanged(const QString &)));
	setNextEnabled(page(0), false);
	setFinishEnabled(page(0), false);
	setHelpEnabled(page(0), false);

	//setup the Behavior page (page 1)
	m_cbTools->insertItem(customTool());
	m_cbTools->insertStringList(m_toolList);
	setFinishEnabled(page(1), true);
	setHelpEnabled(page(1), false);
}

void NewToolWizard::showPage(QWidget *pg)
{
    	QWizard::showPage(pg);
	
	if ( pg == page(0) )
		m_leName->setFocus();
	else if ( pg == page(1) )
		m_cbTools->setFocus();
}

QString NewToolWizard::customTool()
{
	return i18n("<Custom>");
}

QString NewToolWizard::toolName()
{
	return m_leName->text();
}

QString NewToolWizard::parentTool()
{
	return m_cbTools->currentText();
}
	
void NewToolWizard::nameChanged(const QString &name)
{
	bool ok = true;
	if (m_toolList.contains(name))
	{
		m_lbWarning->setText("Error: A tool by this name already exists.");
		ok = false;
	}
	else if (name.find("/") != -1)
	{
		m_lbWarning->setText("Error: The name may not contain a slash '/'.");
		ok = false;
	}
	else m_lbWarning->setText("");
	setNextEnabled(page(0), ok);
}
