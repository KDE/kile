/***************************************************************************
 *   Copyright (C) 2003 by Roland Schulz                                   *
 *   mail@r2s2.de                                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include <qheader.h>
#include <qtooltip.h>
#include <qregexp.h>
#include <qfileinfo.h>
#include <klocale.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kfileitem.h>
#include <ktexteditor/editinterface.h>
#include <kate/view.h>
#include <kate/application.h>
#include "structview.h"
#include "plugin_kile.h"

StructView::StructView(Kate::ViewManager* viewManager, QWidget *parent, const char *name)
 : QListView(parent, name)
{
   setFocusPolicy(QWidget::ClickFocus);
	header()->hide();
	addColumn(i18n("Structure"),-1);
	setSorting(-1,true);
   m_viewManager = viewManager;
   connect( this, SIGNAL(clicked(QListViewItem *)), SLOT(ClickedOnStructure(QListViewItem *)));
	connect( this, SIGNAL(doubleClicked(QListViewItem *)), SLOT(DoubleClickedOnStructure(QListViewItem *)));
   connect( viewManager, SIGNAL(viewChanged()), SLOT(UpdateStructure()));
   connect( Kate::application()->documentManager(), SIGNAL(documentChanged()), SLOT(UpdateStructure()));
	QToolTip::add(this, i18n("Click to jump to the line"));

	KConfig* config = KGlobal::config();
   config->setGroup( "Structure" );
   QString level_name[5]={"part","chapter","section","subsection","subsubsection"};
   for (int i=0; i<5; i++) {
		struct_level[i]=config->readEntry("Structure Level "+(i+1),level_name[i]);
	}
}


StructView::~StructView()
{
}

void StructView::UpdateStructure()
{
if (!m_viewManager->activeView()) return;
Kate::Document* m_doc = m_viewManager->activeView()->getDoc();
clear();
QString shortName = m_doc->docName();
if ((shortName.right(4)!=".tex") && (shortName!="untitled"))  return;
int pos;
while ( (pos = (int)shortName.find('/')) != -1 )
shortName.remove(0,pos+1);
level[0] =  new QListViewItem( this, shortName,"0",0 );
level[0]->setOpen(TRUE);
level[0]->setPixmap(0,MyUserIcon("doc"));
Child=lastChild=level[1]=level[2]=level[3]=level[4]=level[5]=level[0];
structlist.clear();
structitem.clear();
labelitem.clear();
structlist.append(QString::number(0));
structitem.append(shortName);
QListViewItem *toplabel=  new QListViewItem(level[0],"LABELS","0",0 );
structlist.append(QString::number(0));
structitem.append("LABELS");
QString s;
for(uint i = 0; i < m_doc->numLines(); i++)
 {
  int tagStart, tagEnd;
 //// label ////
 tagStart=tagEnd=0;
 s=m_doc->textLine(i);
 tagStart=s.find("\\label{", tagEnd);
 if (tagStart!=-1)
  {
    s=s.mid(tagStart+7,s.length());
    tagStart=s.find("}", tagEnd);
    if (tagStart!=-1)
    {
    s=s.mid(0,tagStart);
    labelitem.append(s);
    structlist.append(QString::number(i));
    s=s+" (line "+QString::number(i+1)+")";
    structitem.append(s);
    Child = toplabel->firstChild();
    while( Child ) {
    lastChild=Child;
    Child = Child->nextSibling();
    };
    Child=new QListViewItem( toplabel,lastChild,s );
    }
  };

 //// include ////
 tagStart=tagEnd=0;
 s=m_doc->textLine(i);
 tagStart=s.find("\\include{", tagEnd);
 if (tagStart!=-1)
  {
    s=s.mid(tagStart+8,s.length());
    tagStart=s.find("}", tagEnd);
    if (tagStart!=-1)
    {
    s=s.mid(0,tagStart+1);
    structlist.append("include");
    structitem.append(s);
    Child = level[0]->firstChild();
    while( Child ) {
    lastChild=Child;
    Child = Child->nextSibling();
    };
    Child=new QListViewItem( level[0],lastChild,s );
    Child->setPixmap(0,MyUserIcon("include"));
    }
  };
 //// input ////
 tagStart=tagEnd=0;
 s=m_doc->textLine(i);
 tagStart=s.find("\\input{", tagEnd);
 if (tagStart!=-1)
  {
    s=s.mid(tagStart+6,s.length());
    tagStart=s.find("}", tagEnd);
    if (tagStart!=-1)
    {
    s=s.mid(0,tagStart+1);
    structlist.append("input");
    structitem.append(s);
    Child = level[0]->firstChild();
    while( Child ) {
    lastChild=Child;
    Child = Child->nextSibling();
    };
    Child=new QListViewItem( level[0],lastChild,s );
    Child->setPixmap(0,MyUserIcon("include"));
    }
  };

 //// part,chapter,section,subsection,subsubsection ////
 QString level_name[5]={"part","chapter","section","subsection","subsubsection"};
 for (int j=0; j<5; j++) {
 tagStart=tagEnd=0;
 s=m_doc->textLine(i);
 tagStart=s.find(QRegExp("\\\\"+struct_level[j]+"\\*?[\\{\\[]"), tagEnd);
 if (tagStart!=-1)
  {
    structlist.append(QString::number(i));
    tagStart=s.find(struct_level[j], tagEnd);
    s=s.mid(tagStart+struct_level[j].length(),s.length());
    s=s+" (line "+QString::number(i+1)+")";
    structitem.append(s);
    Child = level[j]->firstChild();
    while( Child ) {
    lastChild=Child;
    Child = Child->nextSibling();
    };
    level[j+1]=new QListViewItem( level[j],lastChild,s );
    level[j+1]->setPixmap(0,MyUserIcon(level_name[j]));
    for (int k=j+2; k<6; k++)
    	level[k]=level[j+1];
  };
  }
 }
//if (currentEditorView() ){currentEditorView()->editor->viewport()->setFocus();}
}

void StructView::ClickedOnStructure(QListViewItem *)
{
Kate::Document* m_doc = m_viewManager->activeView()->getDoc();
//if ( !currentEditorView() ) return;
QListViewItem *item = currentItem();
QString it;
if ((item) && (!structlist.isEmpty()))
 {
 QStringList::ConstIterator it1 = structitem.begin();
 QStringList::ConstIterator it2 = structlist.begin();
 for ( ; it1 !=structitem.end(); ++it1 )
    {
    if (*it1==item->text(0)) break;
    ++it2;
    }
bool ok;
QString s=*it2;
if (s!="include" && s!="input")
 {
 uint l=s.toUInt(&ok,10);
 if (ok && l<=m_doc->numLines())
  {
  //currentEditorView()->editor->viewport()->setFocus();
  m_viewManager->activeView()->setCursorPosition(l,0);
  //UpdateLineColStatus();
  }
 }
 }
}

void StructView::DoubleClickedOnStructure(QListViewItem *)
{
Kate::Document* m_doc = m_viewManager->activeView()->getDoc();
//if ( !currentEditorView() ) return;
QListViewItem *item = currentItem();
QString it;
if ((item) && (!structlist.isEmpty()))
 {
 QStringList::ConstIterator it1 = structitem.begin();
 QStringList::ConstIterator it2 = structlist.begin();
 for ( ; it1 !=structitem.end(); ++it1 )
    {
    if (*it1==item->text(0)) break;
    ++it2;
    }
QString s=*it2;
if (s=="include" || s =="input")
    {
    QString fname=*it1;
    KURL url = m_doc->url();
    if (fname.right(5)==".tex}") url.setFileName(fname.mid(1,fname.length()-2));
    else 								url.setFileName(fname.mid(1,fname.length()-2)+".tex");
    //KFileItem fi(url);
    //if (fi.isFile() && fi.isReadable())
      {
        m_viewManager->openURL(url);
      }
    }
}
}


