/***************************************************************************
    begin                :  2003-07-01 17:33:00 CEST 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
    email                :  Jeroen.Wijnhout@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// 2005-07-26 dani
//  - cleanup dialog
//  - added new action 'ShowLabel'

#include <qstring.h>
#include <qstringlist.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qfileinfo.h>

#include <klineedit.h>
#include <kglobal.h>
#include <kcombobox.h>
#include <kpushbutton.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kiconloader.h>

#include <kdebug.h>

#include "kileactions.h"
#include "kileinfo.h"
#include "kiledocmanager.h"

namespace KileAction
{

////////////////
//    Tag     //
////////////////
Tag::Tag( const QString &text, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent
	, const char *name, const QString &tagBegin, const QString &tagEnd
	, int dx, int dy, const QString &description)
	: KAction(text, cut, parent, name),
	  m_data(text,tagBegin, tagEnd, dx, dy, description)
{
	init(receiver,slot);
}

Tag::Tag( const QString &text, const QString& pix, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent
	, const char *name, const QString &tagBegin, const QString &tagEnd
	, int dx, int dy, const QString &description)
	: KAction(text, pix, cut, parent, name),
	  m_data(text,tagBegin, tagEnd, dx, dy, description)
{
	init(receiver,slot);
}

Tag::Tag( const QString &text, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent
	, const char *name, const TagData& data)
	: KAction(text, cut, parent, name),
	  m_data(data)
{
	init(receiver,slot);
}

Tag::Tag( const QString &text, const QString& pix, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent
	, const char *name, const TagData& data)
	: KAction(text, pix, cut, parent, name),
	  m_data(data)
{
	init(receiver,slot);
}

Tag::~Tag()
{
}

void Tag::init(const QObject *receiver, const char *slot)
{
	connect(this, SIGNAL(activated()), SLOT(emitData()));
	connect(this, SIGNAL(activated(const KileAction::TagData&)), receiver, slot);
}

void Tag::emitData()
{
	emit(activated(m_data));
}

////////////////
//    InputTag     //
////////////////
InputTag::InputTag(KileInfo* ki, const QString &text, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name, QWidget *wparent, uint options
	, const QString &tagBegin, const QString &tagEnd
	, int dx, int dy, const QString &description, const QString &hint, const QString &alter)
	: Tag(text, cut, receiver, slot, parent, name, tagBegin, tagEnd, dx, dy, description), m_ki(ki),
	  m_parent(wparent), m_options(options), m_hint(hint), m_alter(alter)
{
	init();
}

InputTag::InputTag( KileInfo* ki, const QString &text, const QString& pix, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name, QWidget *wparent,uint options
	, const QString &tagBegin, const QString &tagEnd
	, int dx, int dy, const QString &description, const QString &hint, const QString &alter)
	: Tag(text, pix, cut, receiver, slot, parent, name, tagBegin, tagEnd, dx, dy, description), m_ki(ki),
	  m_parent(wparent), m_options(options), m_hint(hint), m_alter(alter)
{
	init();
}

InputTag::InputTag( KileInfo* ki, const QString &text, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name, QWidget *wparent,uint options
	, const TagData& data, const QString &hint, const QString &alter)
	: Tag(text,cut,receiver, slot, parent, name,data),  m_ki(ki),
	  m_parent(wparent), m_options(options), m_hint(hint), m_alter(alter)
{
	init();
}

InputTag::InputTag( KileInfo* ki, const QString &text, const QString& pix, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name, QWidget *wparent,uint options
	, const TagData& data, const QString &hint, const QString &alter)
	: Tag(text, pix, cut,receiver, slot, parent, name,data), m_ki(ki),
	  m_parent(wparent), m_options(options), m_hint(hint), m_alter(alter)
{
	init();
}

InputTag::~InputTag()
{
}

void InputTag::init()
{
	m_history.clear();
}

void InputTag::emitData()
{
	kdDebug() << "InputTag::emitData() " << m_ki->getName() << endl;

	InputDialog *dlg = new InputDialog(m_data.text, m_options, m_history, m_hint, m_alter, m_ki, m_parent, "input_dialog");
	if (dlg->exec())
	{
		if ( (! dlg->tag().isEmpty()) && hasHistory()) addToHistory(dlg->tag());

		TagData td(m_data);

		td.tagBegin.replace("%R",dlg->tag());
		td.tagEnd.replace("%R",dlg->tag());

		QString alt = dlg->useAlternative() ? "*" : "";
		td.tagBegin.replace("%A", alt);
		td.tagEnd.replace("%A", alt);

		if ( dlg->useLabel() ) 
		{
			td.tagEnd += dlg->label();
			td.dy++;
		}

		if (dlg->usedSelection())
			m_ki->clearSelection();
			
		// insert tag
		emit(activated(td));
		// refresh document structure and project tree when a file was inserted
		if ( dlg->useAddProjectFile() ) 
		{
			m_ki->docManager()->projectAddFile( QFileInfo(m_ki->getCompileName()).dirPath(true) + '/' + dlg->tag() );
		}
	}
	delete dlg;
}


/*
	InputDialog
*/
InputDialog::InputDialog(const QString &caption, uint options, const QStringList& history, const QString& hint, const QString& alter, KileInfo *ki, QWidget *parent, const char *name)
	: KDialogBase (parent, name, true, caption, KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, true), m_ki(ki)
{
	QString newcaption = caption;
	setCaption(newcaption.remove("&"));
	m_labelprefix = ( newcaption == "chapter" ) ? "chap:" : "sec:";
	
	m_usedSelection = false;

	QWidget *page = new QWidget(this);
	setMainWidget(page);
	QGridLayout *gbox = new QGridLayout( page, 6,3,5,5,"");

	QLabel *lb = new QLabel(hint, page);
	gbox->addMultiCellWidget(lb,0,0,0,2);

	m_tag=QString::null;
	QWidget *focus;
	if ( (options & KileAction::KeepHistory) || (options & KileAction::FromLabelList) || (options & KileAction::FromBibItemList) )
	{
		KComboBox *input = new KComboBox(true, page, "input_dialog_input");
		input->setCompletionMode(KGlobalSettings::CompletionAuto);
		input->setMinimumWidth(300);
		focus = input;

		connect(input, SIGNAL(textChanged(const QString&)), this, SLOT(setTag(const QString&)));
		connect(this,  SIGNAL(setInput(const QString&)), input, SLOT(setEditText(const QString&)));
		if ( options & KileAction::ShowBrowseButton )
			gbox->addWidget(input,1,0);
		else
			gbox->addMultiCellWidget(input,1,1,0,2);

		const QStringList *list;

		if (options & KileAction::FromLabelList)
		{
			list = ki->allLabels();
			input->insertStringList(*list);
			m_tag = list->first();
		}
		else
		if (options & KileAction::FromBibItemList)
		{
			list = ki->allBibItems();
			input->insertStringList(*list);
			m_tag = list->first();
		}
		else
		{
			if (history.size()>0)
			{
				input->insertStringList(history);
				m_tag = history.first();
			}
		}
	}
	else
	{
		KLineEdit *input = new KLineEdit(page);
		input->setMinimumWidth(300);
		focus = input;

		connect(input, SIGNAL(textChanged(const QString&)), this, SLOT(setTag(const QString&)));
		connect(this,  SIGNAL(setInput(const QString&)), input, SLOT(setText(const QString&)));
		if ( options & KileAction::ShowBrowseButton )
			gbox->addWidget(input,1,0);
		else
			gbox->addMultiCellWidget(input,1,1,0,2);

		input->setText(ki->getSelection());
		m_usedSelection=true;
	}

	if (focus)
		lb->setBuddy(focus);

	if ( options & KileAction::ShowBrowseButton)
	{
		KPushButton *pbutton = new KPushButton("", page);
		pbutton->setPixmap( SmallIcon("fileopen") );
		gbox->addWidget(pbutton,1,2);
		gbox->setColSpacing(1,8);	
		gbox->setColSpacing(2, pbutton->sizeHint().width()+5 ); 
		connect(pbutton, SIGNAL(clicked()), this, SLOT(slotBrowse()));
	}

	if ( options & KileAction::ShowAlternative)
	{
		QCheckBox * m_checkbox = new QCheckBox(alter, page, "input_dialog_checkbox");
		connect(m_checkbox, SIGNAL(clicked()), this, SLOT(slotAltClicked()));
		m_useAlternative=false;
		gbox->addMultiCellWidget(m_checkbox,2,2,0,2);
	}

	m_edLabel = 0L;
	m_useLabel = ( options & KileAction::ShowLabel );
	if ( m_useLabel )
	{
		// Label
		QLabel *label = new QLabel(i18n("&Label:"),page);
		m_edLabel = new KLineEdit("",page);
		m_edLabel->setMinimumWidth(300);
		m_edLabel->setText(m_labelprefix);
		label->setBuddy(m_edLabel);
		gbox->addMultiCellWidget(label,3,3,0,2);
		gbox->addMultiCellWidget(m_edLabel,4,4,0,2);
	}

	m_useAddProjectFile = ( options & KileAction::AddProjectFile );
	
	gbox->setRowStretch(5,1);
	gbox->setColStretch(0,1);
	
	focus->setFocus();
}


InputDialog::~InputDialog()
{
}

void InputDialog::slotBrowse()
{
	QString fn;
	QFileInfo fi(m_ki->getCompileName());
	
	fn = KFileDialog::getOpenFileName(fi.absFilePath(), QString::null, this,i18n("Select File") );
	if ( !fn.isEmpty() )
	{
		QString path = m_ki->relativePath(fi.dirPath(), fn);
		if ( path.find(".tex",-4) >= 0 )
			path.truncate( path.length()-4 );
		setTag(path);
		emit(setInput(path));
	}
}

void InputDialog::slotAltClicked()
{
	m_useAlternative = !m_useAlternative;
}

void InputDialog::setTag(const QString &tag)
{
	m_tag = tag;
}

QString InputDialog::label()
{
	if ( m_edLabel ) 
	{
		QString label = m_edLabel->text().stripWhiteSpace();
		if ( !label.isEmpty() && label!=m_labelprefix )
			return "\\label{" + label + "}\n";
	}
	
	return QString::null;
}

/////////////////
//  SelectTag  //
/////////////////

Select::Select(const QString &text, const KShortcut &cut, KActionCollection *parent, const char *name )
	: KSelectAction(text,cut,parent,name)
{
	init();
}

void Select::init()
{
	connect(this, SIGNAL(activated(const QString&)), SLOT(emitData(const QString &)));
}

void Select::emitData(const QString & name)
{
	m_dict[name]->activate();
}

void Select::setItems(QPtrList<KAction>& list)
{
	QStringList tmp;

	for (uint i=0; i < list.count(); ++i)
	{
		tmp.append(list.at(i)->text());
		m_dict.insert(list.at(i)->text(), list.at(i));
	}

	KSelectAction::setItems(tmp);
}

}

#include "kileactions.moc"
