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

#include "dialogs/newtoolwizard.h"

#include "kiletoolmanager.h"

NewToolWizard::NewToolWizard(QWidget *parent, Qt::WFlags fl) : KAssistantDialog(parent, fl)
{
	QWidget *toolNameWidget = new QWidget(this);
	Ui::NewToolWizardToolNamePage::setupUi(toolNameWidget);
	toolNamePage = new KPageWidgetItem(toolNameWidget, i18n("Tool Name"));

	QWidget *classWidget = new QWidget(this);
	Ui::NewToolWizardClassPage::setupUi(classWidget);
	classPage = new KPageWidgetItem(classWidget, i18n("Class"));

	addPage(toolNamePage);
	addPage(classPage);

	m_toolList = KileTool::toolList(KGlobal::config().data(), false);

	showButton(KDialog::Help, false);

	connect(this, SIGNAL(currentPageChanged(KPageWidgetItem*, KPageWidgetItem*)), this, SLOT(slotCurrentPageChanged(KPageWidgetItem*, KPageWidgetItem*)));
	connect(m_leName, SIGNAL(textChanged(const QString &)), this, SLOT(nameChanged(const QString &)));
	setValid(toolNamePage, false);

	//setup the Behavior page (page 1)
	m_cbTools->addItem(customTool());
	m_cbTools->addItems(m_toolList);
}

NewToolWizard::~NewToolWizard()
{
	delete toolNamePage;
	delete classPage;
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
	static QRegExp reBracket = QRegExp("\\(|\\)|\\[|\\]");
	bool ok = true;

	if(m_toolList.contains(name)) {
		m_lbWarning->setText(i18n( "Error: A tool by this name already exists." ));
		ok = false;
	}
	else if(name.indexOf("/") != -1) {
		m_lbWarning->setText(i18n( "Error: The name may not contain a slash '/'." ));
		ok = false;
	}
	else if(name.indexOf(reBracket) != -1) {
		m_lbWarning->setText(i18n("Error: The name may not contain a (,),[ or ]."));
		ok = false;
	}
	else {
		m_lbWarning->setText("");
	}
	setValid(toolNamePage, true);
}

void NewToolWizard::slotCurrentPageChanged(KPageWidgetItem* current, KPageWidgetItem* /* before */)
{
	if (current == toolNamePage) {
		m_leName->setFocus();
	}
	else if (current == classPage) {
		m_cbTools->setFocus();
	}
}

#include "newtoolwizard.moc"
