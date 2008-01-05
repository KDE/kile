/***************************************************************************
                         latexcmddialog.cpp
                         --------------
    date                 : Jul 25 2005
    version              : 0.20
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

// kommandos mit weiteren Parametern

#include "latexcmddialog.h"
#include "latexcmd.h"

#include <qlayout.h>
#include <qvalidator.h>
#include <qregexp.h>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>

#include <kmessagebox.h>
#include <klocale.h>
#include "kiledebug.h"

#include "kileconfig.h"

namespace KileDialog
{

// BEGIN NewLatexCommand

NewLatexCommand::NewLatexCommand(QWidget *parent, const QString &caption,
                                 const QString &groupname, K3ListViewItem *lvitem,
	                              KileDocument::CmdAttribute cmdtype,
	                              QMap<QString,bool> *dict)
   : KDialog(parent), m_dict(dict)
{
	setCaption(caption);
	setModal(true);
	setButtons(Ok | Cancel);
	setDefaultButton(Ok);
	showButtonSeparator(true);

	// 'add' is only allowed, if the K3ListViewItem is defined
	m_addmode = ( lvitem == 0 );
	m_envmode = ( cmdtype < KileDocument::CmdAttrLabel );
	m_cmdType = cmdtype;

	// set modes for input dialog
	//           AMS Math Tab List Verb Label Ref Cit Inc
	// MathOrTab  +   +    +
	// Option     +   +    +   +    +     +    +   +
	// Parameter  +   +    +   +          +    +   +   +

	m_useMathOrTab = false;
	m_useOption = m_useParameter = true;
	if ( cmdtype==KileDocument::CmdAttrAmsmath || cmdtype==KileDocument::CmdAttrMath  || cmdtype==KileDocument::CmdAttrTabular)
		m_useMathOrTab = true;
	else if ( cmdtype==KileDocument::CmdAttrVerbatim )
		m_useParameter = false;

	QWidget *page = new QWidget( this );
	setMainWidget(page);

	// layout
	QVBoxLayout *vbox = new QVBoxLayout(page);
	QLabel *label1 = new QLabel(page);

	QGroupBox* group = new QGroupBox(i18n("Attributes"), page);
	QWidget *widget = new QWidget(group);
	QGridLayout *grid = new QGridLayout(widget);
//FIXME: dropped the marginHint parameter
// marginHint()
	grid->setSpacing(spacingHint());

	QLabel *label2 = new QLabel(i18n("Group:"), widget);
	QLabel *label3 = new QLabel(i18n("&Name:"), widget);
	QLabel *grouplabel = new QLabel(groupname, widget);
	QLabel *label4 = new QLabel(i18n("Include *-&version:"), widget);
	m_edName = new KLineEdit(widget);
	m_chStarred =  new QCheckBox(widget);

	grid->addWidget(label2,0,0);
	grid->addWidget(grouplabel,0,2);
	grid->addWidget(label3,1,0);
	grid->addWidget(m_edName,1,2);
	grid->addWidget(label4,2,0);
	grid->addWidget(m_chStarred,2,2);

	label3->setBuddy(m_edName);
	label4->setBuddy(m_chStarred);
	grouplabel->setWhatsThis(i18n("Name of the group, to which this environment or command belongs."));
	if ( m_addmode )
		m_edName->setWhatsThis(i18n("Name of the new environment or command."));
	else
		m_edName->setWhatsThis(i18n("Name of the environment or command to edit."));
	m_chStarred->setWhatsThis(i18n("Does this environment or command also exist in a starred version?"));

	int currentRow = 3;
	if ( m_useMathOrTab )
	{
		QLabel *label5 = new QLabel(i18n("\\\\ is end of &line:"), widget);
		QLabel *label6 = new QLabel(i18n("Needs &math mode:"), widget);
		QLabel *label7 = new QLabel(i18n("&Tabulator:"), widget);
		m_chEndofline =  new QCheckBox(widget);
		m_chMath =  new QCheckBox(widget);
		m_coTab = new QComboBox(widget);

		grid->addWidget(label5,3,0);
		grid->addWidget(m_chEndofline,3,2);
		grid->addWidget(label6,4,0);
		grid->addWidget(m_chMath,4,2);
		grid->addWidget(label7,5,0);
		grid->addWidget(m_coTab,5,2);

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

	if ( m_useOption )
	{
		QLabel *label8 = new QLabel(i18n("Opt&ion:"), widget);
		m_coOption = new QComboBox(widget);
		grid->addWidget(label8,currentRow,0);
		grid->addWidget(m_coOption,currentRow,2);

		label8->setBuddy(m_coOption);

		m_coOption->addItem(QString());
		if ( m_envmode )
		{
			m_coOption->addItem("[tcb]");
			m_coOption->addItem("[lcr]");
			m_coOption->setWhatsThis(i18n("Define an optional alignment parameter."));
		}
		else
		{
			m_coOption->setWhatsThis(i18n("Does this command need an optional parameter."));
		}
		m_coOption->addItem("[ ]");

		currentRow++;
	}

	if ( m_useParameter )
	{
		QLabel *label9 = new QLabel(i18n("&Parameter:"), widget);
		m_coParameter = new QComboBox(widget);
		grid->addWidget(label9,currentRow,0);
		grid->addWidget(m_coParameter,currentRow,2);

		label9->setBuddy(m_coParameter);

		if ( m_envmode )
		{
			m_coParameter->addItem(QString::null);
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
	if ( m_addmode )                   // add mode
	{
		QString pattern;
		if  ( m_envmode )
		{
			label1->setText( i18n("Define a new LaTeX environment:") );
			pattern = "[A-Za-z]+";
		}
		else
		{
			label1->setText( i18n("Define a new LaTeX command:") );
			pattern = "\\\\?[A-Za-z]+";
		}
		QRegExp reg(pattern);
		m_edName->setValidator( new QRegExpValidator(reg,m_edName) );
		m_edName->setFocus();
	}
	else                          // edit mode
	{
		// always insert name and starred attribute
		m_edName->setText(lvitem->text(0));
		m_edName->setReadOnly(true);
		m_chStarred->setChecked( lvitem->text(1) == "*" );

#ifdef __GNUC__
#warning fix QComboBox->setCurrentText()
#endif
		if  ( m_envmode )          // insert existing arguments for environments
		{
			label1->setText( i18n("Edit a LaTeX Environment") );
			if ( m_useMathOrTab )
			{
				m_chEndofline->setChecked( lvitem->text(2) == "\\\\" );
				m_chMath->setChecked( lvitem->text(3) == "$" );
// 				m_coTab->setCurrentText( lvitem->text(4) );
			}
			if ( m_useOption ) {
// 				m_coOption->setCurrentText( lvitem->text(5) );
			}
			if ( m_useParameter ) {
// 				m_coParameter->setCurrentText( lvitem->text(6) );
			}
		}
		else                      // insert existing arguments for commands
		{
			label1->setText( i18n("Edit a LaTeX Command") );
			if ( m_useOption ) {
// 				m_coOption->setCurrentText( lvitem->text(2) );
			}
			if ( m_useParameter ) {
// 				m_coParameter->setCurrentText( lvitem->text(3) );
			}
		}
	}

	// fill vbox
	vbox->addWidget(label1,0,Qt::AlignHCenter);
	vbox->addWidget(group);
	vbox->addStretch();
}

// get all attributes of this command

void NewLatexCommand::getParameter(QString &name, KileDocument::LatexCmdAttributes &attr)
{
	name = m_edName->text();
	if ( m_envmode==false && name.at(0)!='\\' )
		name.prepend('\\');

	// set main attributes
	attr.standard = false;
	attr.type = m_cmdType;
	attr.starred = m_chStarred->isChecked();

	// read all atributes attributes
	if ( m_useMathOrTab )
	{
		attr.cr = m_chEndofline->isChecked();
		attr.mathmode = m_chMath->isChecked();
		attr.displaymathmode = false;
		attr.tabulator = m_coTab->currentText();
	}
	else
	{
		attr.cr = false;
		attr.mathmode = false;
		attr.displaymathmode = false;
		attr.tabulator = QString::null;
	}

	attr.option = ( m_useOption ) ? m_coOption->currentText() : QString::null;
	attr.parameter = ( m_useParameter ) ? m_coParameter->currentText() : QString::null;
}

void NewLatexCommand::slotOk()
{
	// check for an empty string
	if ( m_edName->text().isEmpty() )
	{
		KMessageBox::error( this, i18n("An empty string is not allowed.") );
		return;
	}

	QString name = m_edName->text();
	if ( m_envmode==false && name.at(0)!='\\' )
		name.prepend('\\');

	if ( m_addmode && m_dict->contains(name) ) {
		QString msg = ( m_envmode ) ? i18n("This environment already exists.")
		                            : i18n("This command already exists.");
		KMessageBox::error( this,msg );
		return;
	}

	accept();
}
//END NewLatexCommand

////////////////////////////// LaTeX environments/commands dialog //////////////////////////////

//BEGIN LatexCommandsDialog

LatexCommandsDialog::LatexCommandsDialog(KConfig *config, KileDocument::LatexCommands *commands, QWidget *parent, const char *name)
   : KDialog( parent ), m_config(config), m_commands(commands)
{
	setCaption(i18n("LaTeX Configuration"));
	setModal(true);
	setButtons(Ok | Cancel | Help);
	setDefaultButton(Ok);
	showButtonSeparator(true);

	QWidget *page = new QWidget( this );
	setMainWidget(page);

	QVBoxLayout *pageLayout = new QVBoxLayout();
	pageLayout->setMargin(0);
	pageLayout->setSpacing(KDialog::spacingHint());
	page->setLayout(pageLayout);

	QLabel *label = new QLabel(i18n("Define LaTeX Environments and Commands for Kile"), page);

	// create TabWidget
	m_tab = new QTabWidget(page);
   m_cbUserDefined = new QCheckBox(i18n("&Show only user defined environments and commands"),page);

	// tab 1: environment listview
	m_lvEnvironments = new K3ListView(m_tab);
	m_lvEnvironments->setRootIsDecorated(true);
	m_lvEnvironments->addColumn(i18n("Environment"));
	m_lvEnvironments->addColumn(i18n("Starred"));
	m_lvEnvironments->addColumn(i18n("EOL"));
	m_lvEnvironments->addColumn(i18n("Math"));
	m_lvEnvironments->addColumn(i18n("Tab"));
	m_lvEnvironments->addColumn(i18n("Option"));
	m_lvEnvironments->addColumn(i18n("Parameter"));
	m_lvEnvironments->setAllColumnsShowFocus(true);
	m_lvEnvironments->setSelectionMode(Q3ListView::Single);

	for ( int col=1; col<=6; col++ )
		m_lvEnvironments->setColumnAlignment(col,Qt::AlignHCenter);

	// tab 2: command listview
	m_lvCommands = new K3ListView(m_tab);
	m_lvCommands->setRootIsDecorated(true);
	m_lvCommands->addColumn(i18n("Command"));
	m_lvCommands->addColumn(i18n("Starred"));
	m_lvCommands->addColumn(i18n("Option"));
	m_lvCommands->addColumn(i18n("Parameter"));
	m_lvCommands->setAllColumnsShowFocus(true);
	m_lvCommands->setSelectionMode(Q3ListView::Single);

	for ( int col=1; col<=3; col++ )
		m_lvCommands->setColumnAlignment(col,Qt::AlignHCenter);

	// add all pages to TabWidget
	m_tab->addTab(m_lvEnvironments,i18n("&Environments"));
	m_tab->addTab(m_lvCommands,i18n("&Commands"));
	// page2->setEnabled(false);                        // disable command page

	// button
	QWidget *buttonBox = new QWidget(page);
	QHBoxLayout *buttonBoxLayout = new QHBoxLayout();
	buttonBoxLayout->setMargin(0);
	buttonBoxLayout->setSpacing(KDialog::spacingHint());
	buttonBox->setLayout(buttonBoxLayout);

	m_btnAdd = new KPushButton(i18n("&Add..."), buttonBox);
	m_btnDelete = new KPushButton(i18n("&Delete"), buttonBox);
	m_btnEdit = new KPushButton(i18n("&Edit..."), buttonBox);

	buttonBoxLayout->addStretch();
	buttonBoxLayout->addWidget(m_btnAdd);
	buttonBoxLayout->addWidget(m_btnDelete);
	buttonBoxLayout->addWidget(m_btnEdit);
	buttonBoxLayout->addStretch();

	// add to grid
	pageLayout->addWidget(label);
	pageLayout->addWidget(m_tab);
	pageLayout->addWidget(buttonBox);
	pageLayout->addWidget(m_cbUserDefined);

	setButtonText(Help,"Default Settings");
	slotEnableButtons();

	m_lvEnvironments->setWhatsThis(i18n("List of known environments with a lot of additional information, which Kile could perhaps use. You can add your own environments, which will be recognized by autocompletion of environments, 'Smart Newline' and 'Smart Tabulator' for example. Of course you can only edit and delete user defined environments."));
	m_btnAdd->setWhatsThis(i18n("Add a new environment."));
	m_btnDelete->setWhatsThis(i18n("Delete an user defined environment."));
	m_btnEdit->setWhatsThis(i18n("Edit an user defined environment."));

   connect(m_tab,SIGNAL(currentChanged(QWidget*)),this,SLOT(slotPageChanged(QWidget*)));
	connect(m_lvEnvironments, SIGNAL(selectionChanged()),this, SLOT(slotEnableButtons()));
	connect(m_lvCommands, SIGNAL(selectionChanged()),this, SLOT(slotEnableButtons()));
	connect(m_btnAdd, SIGNAL(clicked()),this, SLOT(slotAddClicked()));
	connect(m_btnDelete, SIGNAL(clicked()),this, SLOT(slotDeleteClicked()));
	connect(m_btnEdit, SIGNAL(clicked()),this, SLOT(slotEditClicked()));
	connect(m_cbUserDefined, SIGNAL(clicked()),this, SLOT(slotUserDefinedClicked()));

	// read config and initialize changes (add, edit or delete an entry)
	readConfig();
	m_commandChanged = false;

	// init listview
	resetListviews();
	slotEnableButtons();

	//resize(sizeHint().width()+20,sizeHint().height()+50);
}

////////////////////////////// listview //////////////////////////////

void LatexCommandsDialog::resetListviews()
{
	m_dictCommands.clear();
	m_lvEnvironments->clear();
	m_lvCommands->clear();

	m_lviAmsmath    = new K3ListViewItem(m_lvEnvironments,i18n("AMS-Math"));
	m_lviMath       = new K3ListViewItem(m_lvEnvironments,i18n("Math"));
	m_lviList       = new K3ListViewItem(m_lvEnvironments,i18n("Lists"));
	m_lviTabular    = new K3ListViewItem(m_lvEnvironments,i18n("Tabular"));
	m_lviVerbatim   = new K3ListViewItem(m_lvEnvironments,i18n("Verbatim"));

	m_lviLabels     = new K3ListViewItem(m_lvCommands,i18n("Labels"));
	m_lviReferences = new K3ListViewItem(m_lvCommands,i18n("References"));
	m_lviCitations  = new K3ListViewItem(m_lvCommands,i18n("Citations"));
	m_lviInputs	= new K3ListViewItem(m_lvCommands,i18n("Includes"));

	QStringList list;
	QStringList::ConstIterator it;
	KileDocument::LatexCmdAttributes attr;

	m_commands->commandList(list,KileDocument::CmdAttrNone,m_cbUserDefined->isChecked());
	for ( it=list.begin(); it != list.end(); ++it )
	{
		if ( m_commands->commandAttributes(*it,attr) )
		{
			K3ListViewItem *parent;
			switch ( attr.type ) {
				case KileDocument::CmdAttrAmsmath:   parent = m_lviAmsmath;    break;
				case KileDocument::CmdAttrMath:      parent = m_lviMath;       break;
				case KileDocument::CmdAttrList:      parent = m_lviList;       break;
				case KileDocument::CmdAttrTabular:   parent = m_lviTabular;    break;
				case KileDocument::CmdAttrVerbatim:  parent = m_lviVerbatim;   break;
				case KileDocument::CmdAttrLabel:     parent = m_lviLabels;     break;
				case KileDocument::CmdAttrReference: parent = m_lviReferences; break;
				case KileDocument::CmdAttrCitations: parent = m_lviCitations;  break;
				case KileDocument::CmdAttrIncludes:  parent = m_lviInputs;   break;
				default: continue;
			}
			setEntry(parent,*it,attr);
		}
	}
}

LatexCommandsDialog::LVmode LatexCommandsDialog::getListviewMode()
{
	return ( m_tab->currentIndex() == 0 ) ? lvEnvMode : lvCmdMode;
}

KileDocument::CmdAttribute LatexCommandsDialog::getCommandMode(K3ListViewItem *item)
{
	KileDocument::CmdAttribute type;

	if ( item == m_lviAmsmath )
		type = KileDocument::CmdAttrAmsmath;
	else if ( item == m_lviMath )
		type = KileDocument::CmdAttrMath;
	else if ( item == m_lviList )
		type = KileDocument::CmdAttrList;
	else if ( item == m_lviTabular )
		type = KileDocument::CmdAttrTabular;
	else if ( item == m_lviVerbatim )
		type = KileDocument::CmdAttrVerbatim;
	else if ( item == m_lviLabels )
		type = KileDocument::CmdAttrLabel;
	else if ( item == m_lviReferences )
		type = KileDocument::CmdAttrReference;
	else if ( item == m_lviCitations )
		type = KileDocument::CmdAttrCitations;
	else if ( item == m_lviInputs )
		type = KileDocument::CmdAttrIncludes;
	else
		type =  KileDocument::CmdAttrNone;

	return type;
}

bool LatexCommandsDialog::isParentItem(K3ListViewItem *item)
{
	return ( item==m_lviMath       ||
	         item==m_lviList       ||
	         item==m_lviTabular    ||
	         item==m_lviVerbatim   ||
	         item==m_lviLabels     ||
	         item==m_lviReferences ||
	         item==m_lviCitations  || 
		 item==m_lviInputs
	       );
}

////////////////////////////// entries //////////////////////////////

void LatexCommandsDialog::setEntry(K3ListViewItem *parent,const QString &name,
	                                KileDocument::LatexCmdAttributes &attr)
{
	// set dictionary
	m_dictCommands[name] = attr.standard;

	// create an item
	K3ListViewItem *item = new K3ListViewItem(parent,name);

	// always set the starred entry
	if ( attr.starred )
		item->setText(1,"*");

	// environments have more attributes
	if ( attr.type < KileDocument::CmdAttrLabel )       // environments
	{
		if ( attr.cr )
			item->setText(2,"\\\\");
		if ( attr.mathmode )
			item->setText(3,"$");
		else if ( attr.displaymathmode )
			item->setText(3,"$$");
		item->setText(4,attr.tabulator);
		item->setText(5,attr.option);
		item->setText(6,attr.parameter);
	}
	else                                                // commands
	{
		item->setText(2,attr.option);
		item->setText(3,attr.parameter);
	}
}

void LatexCommandsDialog::getEntry(K3ListViewItem *item,KileDocument::LatexCmdAttributes &attr)
{
	// always set the starred entry
	attr.starred = ( item->text(1) == "*" );

	// get all attributes
	if ( item->text(0).at(0) != '\\' )                 // environment
	{
		attr.cr = ( item->text(2) == "\\\\" );
		attr.mathmode = ( item->text(3) == "$" );
		attr.displaymathmode = ( item->text(3) == "$$" );
		attr.tabulator = item->text(4);
		attr.option = item->text(5);
		attr.parameter = item->text(6);
	}
	else                                              // commands
	{
		attr.cr = false;
		attr.mathmode = false;
		attr.displaymathmode = false;
		attr.tabulator = QString::null;
		attr.option = item->text(2);
		attr.parameter = item->text(3);
	}
}

////////////////////////////// type of environment or command //////////////////////////////

bool LatexCommandsDialog::isUserDefined(const QString &name)
{
	return ( m_dictCommands.contains(name) && m_dictCommands[name]==false );
}

// look for user defined environment or commands in this listview

bool LatexCommandsDialog::hasUserDefined(K3ListView *listview)
{
	for ( Q3ListViewItem *cur=listview->firstChild(); cur; cur=cur->nextSibling() )
	{
		for ( Q3ListViewItem *curchild=cur->firstChild(); curchild; curchild=curchild->nextSibling() )
		{
			if ( isUserDefined(curchild->text(0)) )
				return true;
		}
	}
	return false;
}

////////////////////////////// slots //////////////////////////////

void LatexCommandsDialog::slotPageChanged(QWidget *)
{
	slotEnableButtons();
}

void LatexCommandsDialog::slotEnableButtons()
{
	bool addState = false;
	bool deleteState = false;
	bool editState = false;
	bool resetState = false;

	K3ListView *listview = ( getListviewMode() == lvEnvMode ) ? m_lvEnvironments : m_lvCommands;
	resetState = ( hasUserDefined(listview) );

	K3ListViewItem *item = (K3ListViewItem *)listview->selectedItem();

	if ( item && item!=m_lviAmsmath )
	{
		addState = isParentItem(item);
		if ( !addState && isUserDefined(item->text(0)) )
		{
			deleteState = true;
			editState = true;
		}
	}

	m_btnAdd->setEnabled(addState);
	m_btnDelete->setEnabled(deleteState);
	m_btnEdit->setEnabled(editState);
	enableButton(Help,resetState);
}

void LatexCommandsDialog::slotAddClicked()
{
	K3ListView *listview;
	QString caption;
	bool envmode;

	if ( getListviewMode() == lvEnvMode )
	{
		listview = m_lvEnvironments;
		caption  = i18n("LaTeX Environments");
		envmode  = true;
	}
	else
	{
		listview = m_lvCommands;
		caption  = i18n("LaTeX Commands");
		envmode  = false;
	}

	K3ListViewItem *item = (K3ListViewItem *)listview->selectedItem();
	if ( item && isParentItem(item) )
	{
		// get current command type
		KileDocument::CmdAttribute type = getCommandMode(item);
		if ( type == KileDocument::CmdAttrNone )
		{
			KILE_DEBUG() << "\tLatexCommandsDialog error: no item in slotAddClicked() (" << item->text(0) << ")" << endl;
			return;
		}

		// add a new environment or command
		NewLatexCommand *dialog = new NewLatexCommand(this,caption,item->text(0),0L,type,&m_dictCommands);
		if ( dialog->exec() == QDialog::Accepted )
		{
			m_commandChanged = true;

			// insert new item with attributes
			QString name;
			KileDocument::LatexCmdAttributes attr;
			dialog->getParameter(name,attr);
			setEntry((K3ListViewItem *)item,name,attr);
			// open this parent item
			if ( !item->isOpen() )
			{
				item->setOpen(true);
			}
			slotEnableButtons();
		}
		delete dialog;
	}
}

void LatexCommandsDialog::slotDeleteClicked()
{
	K3ListView *listview;
	QString message;

	if ( getListviewMode() == lvEnvMode )
	{
		listview = m_lvEnvironments;
		message  = i18n("Do you want to delete this environment?");
	}
	else
	{
		listview = m_lvCommands;
		message  = i18n("Do you want to delete this command?");
	}

	K3ListViewItem *item = (K3ListViewItem *)listview->selectedItem();
	if ( item && !isParentItem(item) )
	{
		if (KMessageBox::warningContinueCancel(this, message, i18n("Delete"))==KMessageBox::Continue)
		{
			m_commandChanged = true;

			if ( isUserDefined(item->text(0)) )
				m_dictCommands.remove(item->text(0));
			delete item;
			slotEnableButtons();
		}
	}
}

void LatexCommandsDialog::slotEditClicked()
{
	K3ListView *listview;
	QString caption;

	if ( getListviewMode() == lvEnvMode )
	{
		listview = m_lvEnvironments;
		caption  = i18n("LaTeX Environment");
	}
	else
	{
		listview = m_lvCommands;
		caption  = i18n("LaTeX Commands");
	}

	K3ListViewItem *item = (K3ListViewItem *)listview->selectedItem();
	if ( item && !isParentItem(item) )
	{
		K3ListViewItem *parentitem = (K3ListViewItem *)item->parent();
		if ( parentitem )
		{
			// get current command type
			KileDocument::CmdAttribute type = getCommandMode(parentitem);
			if ( type == KileDocument::CmdAttrNone )
			{
				KILE_DEBUG() << "\tLatexCommandsDialog error: no item in slotAddClicked() (" << item->text(0) << ")" << endl;
				return;
			}

			// edit a new environment or command
			NewLatexCommand *dialog = new NewLatexCommand(this,caption,parentitem->text(0),item,type, &m_dictCommands);
			if ( dialog->exec() == QDialog::Accepted )
			{
				m_commandChanged = true;

				// delete old item
				delete item;
				// insert new item with changed attributes
				QString name;
				KileDocument::LatexCmdAttributes attr;
				dialog->getParameter(name,attr);
				setEntry(parentitem,name,attr);
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

// reset to default settings

void LatexCommandsDialog::slotHelp()
{
	QString mode = ( getListviewMode() == lvEnvMode ) ? i18n("'environment'") : i18n("'command'");
	if(KMessageBox::warningContinueCancel(this, i18n("All your %1 settings will be overwritten with the default settings, are you sure you want to continue?", mode)) == KMessageBox::Continue) {
		if(getListviewMode() == lvEnvMode ) {
			resetEnvironments();
		}
		else {
			resetCommands();
		}
		slotEnableButtons();
	}
}

// OK-Button clicked, we have to look for user defined environments/commands

void LatexCommandsDialog::slotOk()
{
	// save checkbox for user defined commands
	KileConfig::setShowUserCommands(m_cbUserDefined->isChecked());

	// write config entries for environments and commands
	writeConfig(m_lvEnvironments,m_commands->envGroupName(),true);
	writeConfig(m_lvCommands,m_commands->cmdGroupName(),false);
	m_config->sync();

	// reset known LaTeX environments and commands
	m_commands->resetCommands();

	// save if there is a change in user defined commands and environments
	KileConfig::setCompleteChangedCommands( m_commandChanged );

	accept();
}

////////////////////////////// read/write config //////////////////////////////

void LatexCommandsDialog::readConfig()
{
	// read checkbox for user defined commands
	m_cbUserDefined->setChecked( KileConfig::showUserCommands() );
}

void LatexCommandsDialog::writeConfig(K3ListView *listview, const QString &groupname, bool env)
{
	// first delete old entries
	if ( m_config->hasGroup(groupname) )
		m_config->deleteGroup(groupname);

	// prepare for new entries
	KConfigGroup group = m_config->group(groupname);

	// now get all attributes
	KileDocument::LatexCmdAttributes attr;
	attr.standard = false;

	// scan the listview for non standard entries
	for ( Q3ListViewItem *cur=listview->firstChild(); cur; cur=cur->nextSibling() )
	{
		// get the type of the parent entry
		attr.type = getCommandMode((K3ListViewItem *)cur);
		if ( attr.type == KileDocument::CmdAttrNone )
		{
			KILE_DEBUG() << "\tLatexCommandsDialog error: no parent item (" << cur->text(0) << ")" << endl;
			continue;
		}

		// look for children
		for ( Q3ListViewItem *curchild=cur->firstChild(); curchild; curchild=curchild->nextSibling() )
		{
			QString key = curchild->text(0);
			if ( isUserDefined(key) )
			{
				getEntry((K3ListViewItem *)curchild,attr);
				QString value = m_commands->configString(attr,env);
				KILE_DEBUG() << "\tLatexCommandsDialog write config: " << key << " --> " << value << endl;
				if ( ! value.isEmpty() )
				  group.writeEntry(key, value);
			}
		}
	}
}

////////////////////////////// reset environments and commands //////////////////////////////

// delete all user defined environments

void LatexCommandsDialog::resetEnvironments()
{
	// remember current states
	bool states[9];
	getListviewStates(states);

	// delete user defined commands ands re-read the list
	if ( m_config->hasGroup(m_commands->envGroupName()) )
		m_config->deleteGroup(m_commands->envGroupName());
	m_commands->resetCommands();

	// reset Listview and set old states again (if possible)
	resetListviews();
	setListviewStates(states);
}

// delete all user defined commands

void LatexCommandsDialog::resetCommands()
{
	// remember current states
	bool states[9];
	getListviewStates(states);

	// delete user defined commands ands re-read the list
	if ( m_config->hasGroup(m_commands->cmdGroupName()) )
		m_config->deleteGroup(m_commands->cmdGroupName());
	m_commands->resetCommands();

	// reset Listview and set old states again (if possible)
	resetListviews();
	setListviewStates(states);
}

// states of all parent items

void LatexCommandsDialog::getListviewStates(bool states[])
{
	states[0] = m_lvEnvironments->isOpen(m_lviAmsmath);
	states[1] = m_lvEnvironments->isOpen(m_lviMath);
	states[2] = m_lvEnvironments->isOpen(m_lviList);
	states[3] = m_lvEnvironments->isOpen(m_lviTabular);
	states[4] = m_lvEnvironments->isOpen(m_lviVerbatim);

	states[5] = m_lvCommands->isOpen(m_lviLabels);
	states[6] = m_lvCommands->isOpen(m_lviReferences);
	states[7] = m_lvCommands->isOpen(m_lviCitations);
	states[8] = m_lvCommands->isOpen(m_lviInputs);
}

void LatexCommandsDialog::setListviewStates(bool states[])
{
	m_lvEnvironments->setOpen(m_lviAmsmath,states[0]);
	m_lvEnvironments->setOpen(m_lviMath,states[1]);
	m_lvEnvironments->setOpen(m_lviList,states[2]);
	m_lvEnvironments->setOpen(m_lviTabular,states[3]);
	m_lvEnvironments->setOpen(m_lviVerbatim,states[4]);

	m_lvCommands->setOpen(m_lviLabels,states[5]);
	m_lvCommands->setOpen(m_lviReferences,states[6]);
	m_lvCommands->setOpen(m_lviCitations,states[7]);
	m_lvCommands->setOpen(m_lviInputs,states[8]);
}

//END LatexCommandsDialog


}
#include "latexcmddialog.moc"
