/***************************************************************************
                          kileactions.cpp  -  description
                             -------------------
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

#include <qstring.h>
#include <qstringlist.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>

#include <klineedit.h>
#include <kcombobox.h>
#include <kpushbutton.h>
#include <klocale.h>
#include <kfiledialog.h>

#include <kdebug.h>

#include "kileactions.h"

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
		kdDebug() << "een" << endl;
		if ( (! dlg->tag().isEmpty()) && hasHistory()) addToHistory(dlg->tag());

		kdDebug() << "twee" << endl;
		TagData td(m_data);

		td.tagBegin.replace("%R",dlg->tag());
		td.tagEnd.replace("%R",dlg->tag());

		kdDebug() << "drie" << endl;
		QString alt = dlg->useAlternative() ? "*" : "";
		td.tagBegin.replace("%A", alt);
		td.tagEnd.replace("%A", alt);

		kdDebug() << "vier" << endl;
		emit(activated(td));
	}
	delete dlg;
}


////////////////
//    InputFigure     //
////////////////
InputFigure::InputFigure(KileInfo* ki, const QString &text, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name, QWidget *wparent,uint options
			, const QString &tagBegin, const QString &tagEnd, int dx, int dy, const QString &description, const QString &hint, const QString &alter)
	: InputTag(ki, text, cut, receiver, slot, parent, name, wparent, options, tagBegin, tagEnd, dx, dy, description, hint, alter)
{
	init();
}

void InputFigure::emitData()
{
	InputDialog *dlg = new InputDialog(m_data.text, m_options, m_history, m_hint, m_alter, m_ki, m_parent, "input_dialog");
	if (dlg->exec())
	{
//		kdDebug() << "een" << endl;
		if ( (! dlg->tag().isEmpty()) && hasHistory()) addToHistory(dlg->tag());

//		kdDebug() << "twee" << endl;
		TagData td(m_data);

		td.tagBegin.append(dlg->tag());

		if(dlg->Env != NULL) if(dlg->Env->isChecked())
		{
			td.tagBegin.prepend("\\begin{figure}\n   \\centering\n   ");

			td.tagEnd.append("\n   \\caption{"+dlg->figCaption->text()+"}\n   \\label{"+dlg->figLabel->text()+"}\n\\end{figure}" );
			td.dx += 3;
			td.dy += 2;
		}

//		kdDebug() << "vier" << endl;
		emit(activated(td));
	}
	delete dlg;
}

/*
	InputDialog
*/
InputDialog::InputDialog(const QString &caption, uint options, const QStringList& history, const QString& hint, const QString& alter, KileInfo *ki, QWidget *parent, const char *name)
	: KDialogBase (parent, name, true, caption, KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, true), m_ki(ki)
{
	Env = NULL;
	figLabel = NULL;

	QWidget *page = new QWidget(this);
	setMainWidget(page);
	QGridLayout *gbox = new QGridLayout( page, 3, 4,5,5,"");

	gbox->addMultiCellWidget(new QLabel(hint, page),0,0,0,3);


	m_tag=QString::null;
	if ( (options & KileAction::KeepHistory) || (options & KileAction::FromLabelList) || (options & KileAction::FromBibItemList) )
	{
		KComboBox *input = new KComboBox(true, page, "input_dialog_input");
		connect(input, SIGNAL(textChanged(const QString&)), this, SLOT(setTag(const QString&)));
		connect(this,  SIGNAL(setInput(const QString&)), input, SLOT(setEditText(const QString&)));
		gbox->addMultiCellWidget(input,1,1,0,2);

		const QStringList *list;

		if (options & KileAction::FromLabelList)
		{
			list = ki->getLabelList();
			input->insertStringList(*list);
			m_tag = list->first();
		}
		else
		if (options & KileAction::FromBibItemList)
		{
			list = ki->getBibItemList();
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
		connect(input, SIGNAL(textChanged(const QString&)), this, SLOT(setTag(const QString&)));
		connect(this,  SIGNAL(setInput(const QString&)), input, SLOT(setText(const QString&)));
		gbox->addWidget(input,1,0);
	}

	if ( options & KileAction::ShowBrowseButton)
	{
		KPushButton *pbutton = new KPushButton(i18n("Browse..."),page);
		connect(pbutton, SIGNAL(clicked()), this, SLOT(slotBrowse()));
		gbox->addWidget(pbutton,1,3);
	}

	if ( options & KileAction::ShowAlternative)
	{
		QCheckBox * m_checkbox = new QCheckBox(alter, page, "input_dialog_checkbox");
		connect(m_checkbox, SIGNAL(clicked()), this, SLOT(slotAltClicked()));
		m_useAlternative=false;
		gbox->addMultiCellWidget(m_checkbox,2,2,0,3);
	}
	if ( options & KileAction::ShowFigureInput)
	{
		Env = new QCheckBox(page, "" );
		Env->setText(i18n("with environment") );
		Env->setChecked( false );
		gbox->addMultiCellWidget( Env,2,2,0,1,1 );
		connect( Env, SIGNAL( clicked() ), this, SLOT( slotEnvClicked() ) );

		Text2 = new QLabel(page, "" );
		Text2->setText(i18n("Label:") );
		gbox->addWidget( Text2,3,0 );
		figLabel = new QLineEdit(page, "" );
		figLabel->setText("");
		gbox->addMultiCellWidget( figLabel,3,3,1,3,0 );

		Text3 = new QLabel(page, "" );
		Text3->setText(i18n("Caption:") );
		gbox->addWidget( Text3,4,0 );
		figCaption = new QLineEdit(page, "" );
		figCaption->setText("");
		gbox->addMultiCellWidget( figCaption,4,4,1,3,0 );

		slotEnvClicked();
	}
}


InputDialog::~InputDialog()
{
}

void InputDialog::slotBrowse()
{
	QString fn;
	QFileInfo fi(m_ki->getName());
	fn = KFileDialog::getOpenFileName(fi.absFilePath(), QString::null, this,i18n("Select File") );
  	if ( !fn.isEmpty() )
    {
		setTag(fn);
		emit(setInput(fn));
    }
}

void InputDialog::slotAltClicked()
{
	m_useAlternative = !m_useAlternative;
}

void InputDialog::setTag(const QString &tag)
{
	m_tag = tag;
	if(figLabel != NULL)
	{
		QFileInfo picFile( tag );
		if(figLabel->text().isEmpty())
			figLabel->setText( "fig:"+picFile.baseName() );
	}
}

void InputDialog::slotEnvClicked()
{
  if (Env->isChecked()) {
    Text2->setEnabled( true );
    Text3->setEnabled( true );
    figLabel->setEnabled( true );
    figCaption->setEnabled( true );
  }
  else {
    Text2->setDisabled( true );
    Text3->setDisabled( true );
    figLabel->setDisabled( true );
    figCaption->setDisabled( true );
  }
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

	for (uint i=0; i < list.count(); i++)
	{
		tmp.append(list.at(i)->text());
		m_dict.insert(list.at(i)->text(), list.at(i));
	}

	KSelectAction::setItems(tmp);
}

}

#include "kileactions.moc"
