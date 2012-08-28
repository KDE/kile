/***************************************************************************
  Copyright (C) 2005 by Holger Danielsson (holger.danielsson@t-online.de)
                2010 by Michel Ludwig (michel.ludwig@kdemail.net)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// kommandos mit weiteren Parametern

#include "dialogs/latexcommanddialog.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QRegExp>
#include <QTreeWidget>
#include <QValidator>
#include <QVBoxLayout>

#include <KComboBox>
#include <KConfig>
#include <KIcon>
#include <KLineEdit>
#include <KLocale>
#include <KMessageBox>
#include <KPushButton>
#include <KTabWidget>

#include "kileconfig.h"
#include "kiledebug.h"
#include "latexcmd.h"

namespace KileDialog
{

// BEGIN NewLatexCommand

NewLatexCommand::NewLatexCommand(QWidget *parent, const QString &caption,
																 const QString &groupname, QTreeWidgetItem *lvitem,
																 KileDocument::CmdAttribute cmdtype,
																 QMap<QString, bool> *dict)
		: KDialog(parent), m_dict(dict)
{
	setCaption(caption);
	setModal(true);
	setButtons(Ok | Cancel);
	setDefaultButton(Ok);
	showButtonSeparator(true);

	// 'add' is only allowed, if the QTreeWidgetItem is defined
	m_addmode = (lvitem == 0);
	m_envmode = (cmdtype < KileDocument::CmdAttrLabel);
	m_cmdType = cmdtype;

	// set modes for input dialog
	//           AMS Math Tab List Verb Label Ref Cit Inc
	// MathOrTab  +   +    +
	// Option     +   +    +   +    +     +    +   +
	// Parameter  +   +    +   +          +    +   +   +

	m_useMathOrTab = false;
	m_useOption = m_useParameter = true;
	if (cmdtype == KileDocument::CmdAttrAmsmath || cmdtype == KileDocument::CmdAttrMath  || cmdtype == KileDocument::CmdAttrTabular) {
		m_useMathOrTab = true;
	}
	else {
		if(cmdtype == KileDocument::CmdAttrVerbatim) {
			m_useParameter = false;
		}
	}

	QWidget *page = new QWidget(this);
	setMainWidget(page);

	// layout
	QVBoxLayout *vbox = new QVBoxLayout();
	vbox->setMargin(0);
	vbox->setSpacing(KDialog::spacingHint());
	page->setLayout(vbox);

	QLabel *label1 = new QLabel(page);

	QGroupBox* group = new QGroupBox(i18n("Attributes"), page);
	QGridLayout *grid = new QGridLayout();
	grid->setMargin(marginHint());
	grid->setSpacing(spacingHint());
	group->setLayout(grid);

	QLabel *label2 = new QLabel(i18n("Group:"), group);
	QLabel *label3 = new QLabel(i18n("&Name:"), group);
	QLabel *grouplabel = new QLabel(groupname, group);
	QLabel *label4 = new QLabel(i18n("Include *-&version:"), group);
	m_edName = new KLineEdit(group);
	m_chStarred =  new QCheckBox(group);

	grid->addWidget(label2, 0, 0);
	grid->addWidget(grouplabel, 0, 2);
	grid->addWidget(label3, 1, 0);
	grid->addWidget(m_edName, 1, 2);
	grid->addWidget(label4, 2, 0);
	grid->addWidget(m_chStarred, 2, 2);

	label3->setBuddy(m_edName);
	label4->setBuddy(m_chStarred);
	grouplabel->setWhatsThis(i18n("Name of the group, to which this environment or command belongs."));
	if (m_addmode) {
		m_edName->setWhatsThis(i18n("Name of the new environment or command."));
	}
	else {
		m_edName->setWhatsThis(i18n("Name of the environment or command to edit."));
	}
	m_chStarred->setWhatsThis(i18n("Does this environment or command also exist in a starred version?"));

	int currentRow = 3;
	if (m_useMathOrTab) {
		QLabel *label5 = new QLabel(i18n("\\\\ is end of &line:"), group);
		QLabel *label6 = new QLabel(i18n("Needs &math mode:"), group);
		QLabel *label7 = new QLabel(i18n("&Tabulator:"), group);
		m_chEndofline =  new QCheckBox(group);
		m_chMath =  new QCheckBox(group);
		m_coTab = new KComboBox(group);

		grid->addWidget(label5, 3, 0);
		grid->addWidget(m_chEndofline, 3, 2);
		grid->addWidget(label6, 4, 0);
		grid->addWidget(m_chMath, 4, 2);
		grid->addWidget(label7, 5, 0);
		grid->addWidget(m_coTab, 5, 2);

		label5->setBuddy(m_chEndofline);
		label6->setBuddy(m_chMath);
		label7->setBuddy(m_coTab);
		m_chEndofline->setWhatsThis(i18n("Shall 'Smart New Line' insert \\\\?"));
		m_chMath->setWhatsThis(i18n("Does this environment need math mode?"));
		m_coTab->setWhatsThis(i18n("Define the standard tabulator of this environment."));

		m_coTab->addItem(QString());
		m_coTab->addItem("&");
		m_coTab->addItem("&=");
		m_coTab->addItem("&=&");

		currentRow += 3;
	}

	if (m_useOption) {
		QLabel *label8 = new QLabel(i18n("Opt&ion:"), group);
		m_coOption = new KComboBox(group);
		grid->addWidget(label8, currentRow, 0);
		grid->addWidget(m_coOption, currentRow, 2);

		label8->setBuddy(m_coOption);

		m_coOption->addItem(QString());
		if (m_envmode) {
			m_coOption->addItem("[tcb]");
			m_coOption->addItem("[lcr]");
			m_coOption->setWhatsThis(i18n("Define an optional alignment parameter."));
		}
		else {
			m_coOption->setWhatsThis(i18n("Does this command need an optional parameter?"));
		}
		m_coOption->addItem("[ ]");

		currentRow++;
	}

	if(m_useParameter) {
		QLabel *label9 = new QLabel(i18n("&Parameter:"), group);
		m_coParameter = new KComboBox(group);
		grid->addWidget(label9, currentRow, 0);
		grid->addWidget(m_coParameter, currentRow, 2);

		label9->setBuddy(m_coParameter);

		if(m_envmode) {
			m_coParameter->addItem(QString());
			m_coParameter->addItem("{n}");
			m_coParameter->addItem("{w}");
			m_coParameter->addItem("{ }");
			m_coParameter->setWhatsThis(i18n("Does this environment need an additional parameter like {n} for an integer number, {w} for a width or { } for any other parameter?"));
		}
		else
		{
			m_coParameter->addItem("{ }");
			// m_coParameter->addItem(QString());
			m_coParameter->setWhatsThis(i18n("Does this command need an argument?"));
		}

		currentRow++;
	}

	// stretch last row
	//grid->setRowStretch(maxrows-1,1);

	// add or edit mode
	if(m_addmode) {                    // add mode
		QString pattern;
		if(m_envmode) {
			label1->setText(i18n("Define a new LaTeX environment:"));
			pattern = "[A-Za-z]+";
		}
		else {
			label1->setText(i18n("Define a new LaTeX command:"));
			pattern = "\\\\?[A-Za-z]+";
		}
		QRegExp reg(pattern);
		m_edName->setValidator(new QRegExpValidator(reg, m_edName));
		m_edName->setFocus();
	}
	else {                         // edit mode
		// always insert name and starred attribute
		m_edName->setText(lvitem->text(0));
		m_edName->setReadOnly(true);
		m_chStarred->setChecked(lvitem->text(1) == "*");

		int index;
		if (m_envmode) {            // insert existing arguments for environments
			label1->setText(i18n("Edit a LaTeX Environment"));
			if(m_useMathOrTab) {
				m_chEndofline->setChecked(lvitem->text(2) == "\\\\");
				m_chMath->setChecked(lvitem->text(3) == "$");
				if ((index = m_coTab->findText(lvitem->text(4))) > -1)
					m_coTab->setCurrentIndex(index);
			}
			if(m_useOption) {
				if ((index = m_coOption->findText(lvitem->text(5))) > -1)
					m_coOption->setCurrentIndex(index);
			}
			if (m_useParameter) {
				if ((index = m_coParameter->findText(lvitem->text(6))) > -1)
					m_coParameter->setCurrentIndex(index);
			}
		}
		else {                     // insert existing arguments for commands
			label1->setText(i18n("Edit a LaTeX Command"));
			if (m_useOption) {
				if ((index = m_coOption->findText(lvitem->text(2))) > -1) {
					m_coOption->setCurrentIndex(index);
				}
			}
			if (m_useParameter) {
				if ((index = m_coParameter->findText(lvitem->text(3))) > -1) {
					m_coParameter->setCurrentIndex(index);
				}
			}
		}
	}

	// fill vbox
	vbox->addWidget(label1, 0, Qt::AlignHCenter);
	vbox->addWidget(group);
	vbox->addStretch();
}

// get all attributes of this command

void NewLatexCommand::getParameter(QString &name, KileDocument::LatexCmdAttributes &attr)
{
	name = m_edName->text();
	if(m_envmode == false && name.at(0) != '\\') {
		name.prepend('\\');
	}

	// set main attributes
	attr.standard = false;
	attr.type = m_cmdType;
	attr.starred = m_chStarred->isChecked();

	// read all atributes attributes
	if(m_useMathOrTab) {
		attr.cr = m_chEndofline->isChecked();
		attr.mathmode = m_chMath->isChecked();
		attr.displaymathmode = false;
		attr.tabulator = m_coTab->currentText();
	}
	else {
		attr.cr = false;
		attr.mathmode = false;
		attr.displaymathmode = false;
		attr.tabulator.clear();
	}

	attr.option = (m_useOption) ? m_coOption->currentText() : QString();
	attr.parameter = (m_useParameter) ? m_coParameter->currentText() : QString();
}

void NewLatexCommand::slotButtonClicked(int button)
{
	if(button == KDialog::Ok){

		// check for an empty string
		if(m_edName->text().isEmpty()) {
			KMessageBox::error(this, i18n("An empty string is not allowed."));
			return;
		}

		QString name = m_edName->text();
		if (m_envmode == false && name.at(0) != '\\') {
			name.prepend('\\');
		}

		if (m_addmode && m_dict->contains(name)) {
			QString msg = (m_envmode) ? i18n("This environment already exists.")
										: i18n("This command already exists.");
			KMessageBox::error(this, msg);
			return;
		}
		accept();
	}
	else{
		KDialog::slotButtonClicked(button);
	}
}
//END NewLatexCommand

////////////////////////////// LaTeX environments/commands dialog //////////////////////////////

//BEGIN LatexCommandsDialog

LatexCommandsDialog::LatexCommandsDialog(KConfig *config, KileDocument::LatexCommands *commands, QWidget *parent)
		: KDialog(parent), m_config(config), m_commands(commands)
{
	setCaption(i18n("LaTeX Configuration"));
	setModal(true);
	setButtons(Ok | Cancel | Default);
	setDefaultButton(Ok);
	showButtonSeparator(true);

	QWidget *page = new QWidget(this);
	m_widget.setupUi(page);
	setMainWidget(page);

	slotEnableButtons();

	connect(m_widget.tab, SIGNAL(currentChanged(int)), this, SLOT(slotEnableButtons()));
	connect(m_widget.environments, SIGNAL(itemSelectionChanged()), this, SLOT(slotEnableButtons()));
	connect(m_widget.commands, SIGNAL(itemSelectionChanged()), this, SLOT(slotEnableButtons()));
	connect(m_widget.addButton, SIGNAL(clicked()), this, SLOT(slotAddClicked()));
	connect(m_widget.deleteButton, SIGNAL(clicked()), this, SLOT(slotDeleteClicked()));
	connect(m_widget.editButton, SIGNAL(clicked()), this, SLOT(slotEditClicked()));
	connect(m_widget.showOnlyUserDefined, SIGNAL(clicked()), this, SLOT(slotUserDefinedClicked()));

	// read config and initialize changes (add, edit or delete an entry)
	readConfig();
	m_commandChanged = false;

	// init listview
	resetListviews();
	slotEnableButtons();

	for (int col = 0; col <= 6; col++)
		m_widget.environments->resizeColumnToContents(col);
	for (int col = 0; col <= 3; col++)
		m_widget.commands->resizeColumnToContents(col);
}

////////////////////////////// listview //////////////////////////////

void LatexCommandsDialog::resetListviews()
{
	m_dictCommands.clear();
	m_widget.environments->clear();
	m_widget.commands->clear();

	m_lviAmsmath    = new QTreeWidgetItem(m_widget.environments, QStringList(i18n("AMS-Math")));
	m_lviMath       = new QTreeWidgetItem(m_widget.environments, QStringList(i18n("Math")));
	m_lviList       = new QTreeWidgetItem(m_widget.environments, QStringList(i18n("Lists")));
	m_lviTabular    = new QTreeWidgetItem(m_widget.environments, QStringList(i18n("Tabular")));
	m_lviVerbatim   = new QTreeWidgetItem(m_widget.environments, QStringList(i18n("Verbatim")));

	m_lviLabels     = new QTreeWidgetItem(m_widget.commands, QStringList(i18n("Labels")));
	m_lviReferences = new QTreeWidgetItem(m_widget.commands, QStringList(i18n("References")));
	m_lviBibliographies = new QTreeWidgetItem(m_widget.commands, QStringList(i18n("Bibliographies")));
	m_lviCitations  = new QTreeWidgetItem(m_widget.commands, QStringList(i18n("Citations")));
	m_lviInputs     = new QTreeWidgetItem(m_widget.commands, QStringList(i18n("Includes")));

	QStringList list;
	QStringList::ConstIterator it;
	KileDocument::LatexCmdAttributes attr;

	m_commands->commandList(list, KileDocument::CmdAttrNone, m_widget.showOnlyUserDefined->isChecked());
	for (it = list.constBegin(); it != list.constEnd(); ++it)
	{
		if (m_commands->commandAttributes(*it, attr))
		{
			QTreeWidgetItem *parent;
			switch (attr.type) {
			case KileDocument::CmdAttrAmsmath:
				parent = m_lviAmsmath;
				break;
			case KileDocument::CmdAttrMath:
				parent = m_lviMath;
				break;
			case KileDocument::CmdAttrList:
				parent = m_lviList;
				break;
			case KileDocument::CmdAttrTabular:
				parent = m_lviTabular;
				break;
			case KileDocument::CmdAttrVerbatim:
				parent = m_lviVerbatim;
				break;
			case KileDocument::CmdAttrLabel:
				parent = m_lviLabels;
				break;
			case KileDocument::CmdAttrReference:
				parent = m_lviReferences;
				break;
			case KileDocument::CmdAttrCitations:
				parent = m_lviCitations;
				break;
			case KileDocument::CmdAttrIncludes:
				parent = m_lviInputs;
				break;
			case KileDocument::CmdAttrBibliographies:
				parent = m_lviBibliographies;
				break;
			default:
				continue;
			}
			setEntry(parent, *it, attr);
		}
	}
}

LatexCommandsDialog::LVmode LatexCommandsDialog::getListviewMode()
{
	return (m_widget.tab->currentIndex() == 0) ? lvEnvMode : lvCmdMode;
}

KileDocument::CmdAttribute LatexCommandsDialog::getCommandMode(QTreeWidgetItem *item)
{
	if (item == m_lviAmsmath) {
		return KileDocument::CmdAttrAmsmath;
	}
	if (item == m_lviMath) {
		return KileDocument::CmdAttrMath;
	}
	if (item == m_lviList) {
		return KileDocument::CmdAttrList;
	}
	if (item == m_lviTabular) {
		return KileDocument::CmdAttrTabular;
	}
	if (item == m_lviVerbatim) {
		return KileDocument::CmdAttrVerbatim;
	}
	if (item == m_lviLabels) {
		return KileDocument::CmdAttrLabel;
	}
	if (item == m_lviReferences) {
		return KileDocument::CmdAttrReference;
	}
	if (item == m_lviCitations) {
		return KileDocument::CmdAttrCitations;
	}
	if (item == m_lviInputs) {
		return KileDocument::CmdAttrIncludes;
	}
	if (item == m_lviBibliographies) {
		return KileDocument::CmdAttrBibliographies;
	}

	return KileDocument::CmdAttrNone;
}

bool LatexCommandsDialog::isParentItem(QTreeWidgetItem *item)
{
	return (item == m_lviMath       ||
					item == m_lviList       ||
					item == m_lviTabular    ||
					item == m_lviVerbatim   ||
					item == m_lviLabels     ||
					item == m_lviReferences ||
					item == m_lviCitations  ||
					item == m_lviInputs     ||
					item == m_lviBibliographies
				 );
}

////////////////////////////// entries //////////////////////////////

void LatexCommandsDialog::setEntry(QTreeWidgetItem *parent, const QString &name,
																	 KileDocument::LatexCmdAttributes &attr)
{
	// set dictionary
	m_dictCommands[name] = attr.standard;

	// create an item
	QTreeWidgetItem *item = new QTreeWidgetItem(parent, QStringList(name));

	// always set the starred entry
	if (attr.starred)
		item->setText(1, "*");

	// environments have more attributes
	if (attr.type < KileDocument::CmdAttrLabel)         // environments
	{
		if (attr.cr)
			item->setText(2, "\\\\");
		if (attr.mathmode)
			item->setText(3, "$");
		else
			if (attr.displaymathmode)
				item->setText(3, "$$");
		item->setText(4, attr.tabulator);
		item->setText(5, attr.option);
		item->setText(6, attr.parameter);
	}
	else                                                // commands
	{
		item->setText(2, attr.option);
		item->setText(3, attr.parameter);
	}

	for(int i = 1; i < parent->treeWidget()->columnCount(); ++i) {
		item->setTextAlignment(i, Qt::AlignHCenter);
	}
}

void LatexCommandsDialog::getEntry(QTreeWidgetItem *item, KileDocument::LatexCmdAttributes &attr)
{
	// always set the starred entry
	attr.starred = (item->text(1) == "*");

	// get all attributes
	if (item->text(0).at(0) != '\\')                   // environment
	{
		attr.cr = (item->text(2) == "\\\\");
		attr.mathmode = (item->text(3) == "$");
		attr.displaymathmode = (item->text(3) == "$$");
		attr.tabulator = item->text(4);
		attr.option = item->text(5);
		attr.parameter = item->text(6);
	}
	else                                              // commands
	{
		attr.cr = false;
		attr.mathmode = false;
		attr.displaymathmode = false;
		attr.tabulator.clear();
		attr.option = item->text(2);
		attr.parameter = item->text(3);
	}
}

////////////////////////////// type of environment or command //////////////////////////////

bool LatexCommandsDialog::isUserDefined(const QString &name)
{
	return (m_dictCommands.contains(name) && m_dictCommands[name] == false);
}

// look for user-defined environment or commands in this listview

bool LatexCommandsDialog::hasUserDefined(QTreeWidget *listview)
{
	QTreeWidgetItem *tli;
	for (int i = 0; i < listview->topLevelItemCount(); ++i) {
		tli = listview->topLevelItem(i);
		for (int j = 0; j < tli->childCount(); ++j) {
			if (isUserDefined(tli->child(j)->text(0))) {
				return true;
			}
		}
	}
	return false;
}

////////////////////////////// slots //////////////////////////////

void LatexCommandsDialog::slotEnableButtons()
{
	bool addState = false;
	bool deleteState = false;
	bool editState = false;
	bool resetState = false;

	QTreeWidget *listview = (getListviewMode() == lvEnvMode) ? m_widget.environments : m_widget.commands;
	resetState = (hasUserDefined(listview));

	QTreeWidgetItem *item = (QTreeWidgetItem *)listview->currentItem();

	if (item && item != m_lviAmsmath)
	{
		addState = isParentItem(item);
		if (!addState && isUserDefined(item->text(0)))
		{
			deleteState = true;
			editState = true;
		}
	}

	m_widget.addButton->setEnabled(addState);
	m_widget.deleteButton->setEnabled(deleteState);
	m_widget.editButton->setEnabled(editState);
	enableButton(Default, resetState);
}

void LatexCommandsDialog::slotAddClicked()
{
	QTreeWidget *listview;
	QString caption;

	if (getListviewMode() == lvEnvMode) {
		listview = m_widget.environments;
		caption  = i18n("LaTeX Environments");
	}
	else {
		listview = m_widget.commands;
		caption  = i18n("LaTeX Commands");
	}

	QTreeWidgetItem *item = (QTreeWidgetItem *)listview->currentItem();
	if (item && isParentItem(item)) {
		// get current command type
		KileDocument::CmdAttribute type = getCommandMode(item);
		if (type == KileDocument::CmdAttrNone) {
			KILE_DEBUG() << "\tLatexCommandsDialog error: no item in slotAddClicked() (" << item->text(0) << ")" << endl;
			return;
		}

		// add a new environment or command
		NewLatexCommand *dialog = new NewLatexCommand(this, caption, item->text(0), NULL, type, &m_dictCommands);
		if (dialog->exec() == QDialog::Accepted) {
			m_commandChanged = true;

			// insert new item with attributes
			QString name;
			KileDocument::LatexCmdAttributes attr;
			dialog->getParameter(name, attr);
			setEntry((QTreeWidgetItem *)item, name, attr);
			// open this parent item
			if (!item->isExpanded()) {
				item->setExpanded(true);
			}
			slotEnableButtons();
		}
		delete dialog;
	}
}

void LatexCommandsDialog::slotDeleteClicked()
{
	QTreeWidget *listview;
	QString message;

	if (getListviewMode() == lvEnvMode)
	{
		listview = m_widget.environments;
		message  = i18n("Do you want to delete this environment?");
	}
	else
	{
		listview = m_widget.commands;
		message  = i18n("Do you want to delete this command?");
	}

	QTreeWidgetItem *item = (QTreeWidgetItem *)listview->currentItem();
	if (item && !isParentItem(item))
	{
		if (KMessageBox::warningContinueCancel(this, message, i18n("Delete")) == KMessageBox::Continue)
		{
			m_commandChanged = true;

			if (isUserDefined(item->text(0)))
				m_dictCommands.remove(item->text(0));
			delete item;
			slotEnableButtons();
		}
	}
}

void LatexCommandsDialog::slotEditClicked()
{
	QTreeWidget *listview;
	QString caption;

	if (getListviewMode() == lvEnvMode)
	{
		listview = m_widget.environments;
		caption  = i18n("LaTeX Environment");
	}
	else
	{
		listview = m_widget.commands;
		caption  = i18n("LaTeX Commands");
	}

	QTreeWidgetItem *item = (QTreeWidgetItem *)listview->currentItem();
	if (item && !isParentItem(item))
	{
		QTreeWidgetItem *parentitem = (QTreeWidgetItem *)item->parent();
		if (parentitem)
		{
			// get current command type
			KileDocument::CmdAttribute type = getCommandMode(parentitem);
			if (type == KileDocument::CmdAttrNone)
			{
				KILE_DEBUG() << "\tLatexCommandsDialog error: no item in slotAddClicked() (" << item->text(0) << ")" << endl;
				return;
			}

			// edit a new environment or command
			NewLatexCommand *dialog = new NewLatexCommand(this, caption, parentitem->text(0), item, type, &m_dictCommands);
			if (dialog->exec() == QDialog::Accepted)
			{
				m_commandChanged = true;

				// delete old item
				delete item;
				// insert new item with changed attributes
				QString name;
				KileDocument::LatexCmdAttributes attr;
				dialog->getParameter(name, attr);
				setEntry(parentitem, name, attr);
			}
			delete dialog;
		}
	}
}

void LatexCommandsDialog::slotUserDefinedClicked()
{
	bool states[9];

	getListviewStates(states);
	resetListviews();
	setListviewStates(states);
}

void LatexCommandsDialog::slotButtonClicked(int button)
{
	if (button == Default) {
		QString mode = (getListviewMode() == lvEnvMode) ? i18n("'environment'") : i18n("'command'");
		if (KMessageBox::warningContinueCancel(this, i18n("All your %1 settings will be overwritten with the default settings, are you sure you want to continue?", mode)) == KMessageBox::Continue) {
			if (getListviewMode() == lvEnvMode) {
				resetEnvironments();
			}
			else {
				resetCommands();
			}
			slotEnableButtons();
		}
	} else
		if (button == Ok) {
			// OK-Button clicked, we have to look for user-defined environments/commands

			// save checkbox for user-defined commands
			KileConfig::setShowUserCommands(m_widget.showOnlyUserDefined->isChecked());

			// write config entries for environments and commands
			writeConfig(m_widget.environments, m_commands->envGroupName(), true);
			writeConfig(m_widget.commands, m_commands->cmdGroupName(), false);
			m_config->sync();

			// reset known LaTeX environments and commands
			m_commands->resetCommands();

			// save if there is a change in user-defined commands and environments
			KileConfig::setCompleteChangedCommands(m_commandChanged);

			accept();
		}
	KDialog::slotButtonClicked(button);
}

////////////////////////////// read/write config //////////////////////////////

void LatexCommandsDialog::readConfig()
{
	// read checkbox for user-defined commands
	m_widget.showOnlyUserDefined->setChecked(KileConfig::showUserCommands());
}

void LatexCommandsDialog::writeConfig(QTreeWidget *listview, const QString &groupname, bool env)
{
	// first delete old entries
	if (m_config->hasGroup(groupname)) {
		m_config->deleteGroup(groupname);
	}

	// prepare for new entries
	KConfigGroup group = m_config->group(groupname);

	// now get all attributes
	KileDocument::LatexCmdAttributes attr;
	attr.standard = false;

	int nrOfdefinedCommands = 0;

	// scan the listview for non standard entries
	for (int i = 0; i < listview->topLevelItemCount(); ++i) {
		QTreeWidgetItem *cur = listview->topLevelItem(i);
		// get the type of the parent entry
		attr.type = getCommandMode(cur);
		if (attr.type == KileDocument::CmdAttrNone)
		{
			KILE_DEBUG() << "\tLatexCommandsDialog error: no parent item (" << cur->text(0) << ")" << endl;
			continue;
		}

		// look for children
		for (int j = 0; j < cur->childCount(); ++j) {
			QTreeWidgetItem *curchild = cur->child(j);
			QString key = curchild->text(0);
			if (isUserDefined(key))
			{
				getEntry(curchild, attr);
				QString value = m_commands->configString(attr, env);
				KILE_DEBUG() << "\tLatexCommandsDialog write config: " << key << " --> " << value << endl;
				if (!value.isEmpty()) {
					group.writeEntry("Command" + QString::number(nrOfdefinedCommands), key);
					group.writeEntry("Parameters" + QString::number(nrOfdefinedCommands), value);
					++nrOfdefinedCommands;
				}
			}
		}
	}
	if(nrOfdefinedCommands > 0) {
		group.writeEntry("Number of Commands", nrOfdefinedCommands);
	}
}

////////////////////////////// reset environments and commands //////////////////////////////

// delete all user-defined environments

void LatexCommandsDialog::resetEnvironments()
{
	// remember current states
	bool states[9];
	getListviewStates(states);

	// delete user-defined commands and re-read the list
	if (m_config->hasGroup(m_commands->envGroupName()))
		m_config->deleteGroup(m_commands->envGroupName());
	m_commands->resetCommands();

	// reset Listview and set old states again (if possible)
	resetListviews();
	setListviewStates(states);
}

// delete all user-defined commands

void LatexCommandsDialog::resetCommands()
{
	// remember current states
	bool states[9];
	getListviewStates(states);

	// delete user-defined commands and re-read the list
	if (m_config->hasGroup(m_commands->cmdGroupName()))
		m_config->deleteGroup(m_commands->cmdGroupName());
	m_commands->resetCommands();

	// reset Listview and set old states again (if possible)
	resetListviews();
	setListviewStates(states);
}

// states of all parent items

void LatexCommandsDialog::getListviewStates(bool states[])
{
	states[0] = m_lviAmsmath->isExpanded();
	states[1] = m_lviMath->isExpanded();
	states[2] = m_lviList->isExpanded();
	states[3] = m_lviTabular->isExpanded();
	states[4] = m_lviVerbatim->isExpanded();

	states[5] = m_lviLabels->isExpanded();
	states[6] = m_lviReferences->isExpanded();
	states[7] = m_lviCitations->isExpanded();
	states[8] = m_lviInputs->isExpanded();
}

void LatexCommandsDialog::setListviewStates(bool states[])
{
	m_lviAmsmath->setExpanded(states[0]);
	m_lviMath->setExpanded(states[1]);
	m_lviList->setExpanded(states[2]);
	m_lviTabular->setExpanded(states[3]);
	m_lviVerbatim->setExpanded(states[4]);

	m_lviLabels->setExpanded(states[5]);
	m_lviReferences->setExpanded(states[6]);
	m_lviCitations->setExpanded(states[7]);
	m_lviInputs->setExpanded(states[8]);
}

//END LatexCommandsDialog


}
#include "latexcommanddialog.moc"
