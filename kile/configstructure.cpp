/***************************************************************************
    date                 : Feb 09 2004
    version              : 0.10.0
    copyright            : (C) 2004 by Holger Danielsson
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

#include <kdialog.h>
#include <klocale.h>
#include <kiconloader.h>

#include <qlayout.h>
#include <qtabwidget.h>
#include <qgroupbox.h>
#include <qvgroupbox.h>
#include <qframe.h>
#include <qstringlist.h>

#include "configstructure.h"

namespace KileStructure
{

//////////////////// KileCenteredTableItem ////////////////////

void KileCenteredTableItem::paint(QPainter* p,const QColorGroup& cg,const QRect& cr,bool selected)
{
   int w = cr.width();
   int h = cr.height();

   if (selected && colSpan()==1 ) {
      p->fillRect( 0,0,w,h, cg.brush( QColorGroup::Highlight ) );
      p->setPen( cg.highlightedText() );
   } else {
      p->fillRect( 0,0,w,h, Qt::white );
      p->setPen( Qt::black );
      /*
      QFont f( p->font() );
      f.setBold(true);
      p->setFont(f);
      */
   }

   p->drawText( 2,0,w-4,h, Qt::AlignHCenter|AlignVCenter, text() );
}

//////////////////// KileTable ////////////////////

KileTable::KileTable(QWidget *dialog, QWidget *parent, const char *name) : QTable(parent,name)
{
   setShowGrid(false);
   setSelectionMode(QTable::NoSelection);
   setFocusStyle(QTable::FollowStyle);

   verticalHeader()->hide();
   setLeftMargin(0);
   setVScrollBarMode(QScrollView::AlwaysOff);
   setHScrollBarMode(QScrollView::AlwaysOff);
   setFocusPolicy(QWidget::NoFocus);

   horizontalHeader()->setResizeEnabled(false);
   horizontalHeader()->setClickEnabled(false);

   connect(this,SIGNAL(clickedCell(KileTable *,int,int)),dialog,SLOT(clickedTable(KileTable *,int,int)));
}

void KileTable::contentsMousePressEvent(QMouseEvent *ev)
{
   ev->accept();
   emit( clickedCell( this,rowAt(ev->y()), columnAt(ev->x()) ));
}

//////////////////// ConfigStructure ////////////////////
          
ConfigStructure::ConfigStructure(QWidget *parent, const char *name )
   : QWidget(parent,name)
{
   m_entries << "Files" << "Labels"   << "References"
             << "Index" << "Graphics" << "Sectioning";

   QHBoxLayout *hbox = new QHBoxLayout(this, 5,KDialog::spacingHint() );
   
   // Groupbox with entries
   QVGroupBox *gb_entries= new QVGroupBox(i18n("Entries"), this );
   QWidget *widget1 = new QWidget(gb_entries);
   QVBoxLayout *vbox1 = new QVBoxLayout(widget1, 5,KDialog::spacingHint() );

   // table with entries
   m_entriestable = new KileTable(this,widget1);
   m_entriestable->setNumCols(3);
   m_entriestable->setNumRows(6);
   m_entriestable->setColumnReadOnly(0,true);

   QStringList list1;
   list1 << "Title" << "Visible" << "Open";
   m_entriestable->setColumnLabels(list1);
   m_entriestable->horizontalHeader()->setLabel(1,SmallIcon("structure"),"Visible");
   m_entriestable->horizontalHeader()->setLabel(2,SmallIcon("structure"),"Node");
   
   for ( uint i=0; i<m_entries.count(); ++i ) {
       QTableItem *item = new QTableItem(m_entriestable,QTableItem::Never,i18n(m_entries[i].ascii()));
       m_entriestable->setItem( i,0,item  );
       m_visible[i] = new QCheckTableItem(m_entriestable,"");
       m_entriestable->setItem( i,1, m_visible[i]  );
       m_defaultopen[i] = new QCheckTableItem(m_entriestable,"close");
       m_entriestable->setItem( i,2, m_defaultopen[i]  );
   }

   // groupbox with sectioning
   QVGroupBox *gb_sectioning= new QVGroupBox(i18n("Sectioning"), this );
   QWidget *widget2 = new QWidget(gb_sectioning);   
   QVBoxLayout *vbox2 = new QVBoxLayout(widget2, 5,KDialog::spacingHint() );

   // document class
   QWidget *widget3 = new QWidget(widget2);
   QHBoxLayout *hbox3 = new QHBoxLayout(widget3, 5,KDialog::spacingHint() );
   QLabel *label7 = new QLabel(i18n("Document class:"), widget3);
   comboclasses = new QComboBox(widget3);
   hbox3->addWidget(label7);
   hbox3->addSpacing(10);
   hbox3->addWidget(comboclasses);
   hbox3->setStretchFactor(comboclasses,1);

   // table with sectioning commands 
   m_sectioningtable = new KileTable(this,widget2);
   m_sectioningtable->setNumCols(3);
   m_sectioningtable->setNumRows(5);
   m_sectioningtable->setColumnReadOnly(0,true);
   m_sectioningtable->setColumnReadOnly(1,true);
   
   QStringList list2;
   list2 << "Level" << "LaTeX Command" << "Structure Node";
   m_sectioningtable->setColumnLabels(list2);
   m_sectioningtable->horizontalHeader()->setLabel(2,SmallIcon("structure"),"Structure Node");
    
   // default structure level
   QGroupBox *structGroup = new QGroupBox(2, Qt::Horizontal, i18n("Structure View"), widget2);
   QLabel *label9 = new QLabel(i18n("Default expansion &level: "),structGroup);
   m_structurelevel = new QSpinBox(1,5, 1, structGroup);
   label9->setBuddy(m_structurelevel);

   QGroupBox *classGroup = new QGroupBox(1,Qt::Horizontal,i18n("Document Classes"), widget2);
   QWidget *widget4 = new QWidget(classGroup);
   QHBoxLayout *hbox4 = new QHBoxLayout(widget4, 5,KDialog::spacingHint() );
   QLabel *label10 = new QLabel("Manage classes:",widget4);
   add = new QPushButton(i18n("Add"), widget4);
   remove = new QPushButton(i18n("Remove"), widget4);
   hbox4->addWidget(label10);
   hbox4->addStretch();
   hbox4->addWidget(add);
   hbox4->addWidget(remove);
   
   // add widgets to the left vertical layout
   vbox1->addWidget(m_entriestable);
   vbox1->addStretch();
  
   // add widgets to the right vertical layout
   vbox2->addWidget(widget3);
   vbox2->addSpacing(10);
   vbox2->addWidget(m_sectioningtable);
   vbox2->addSpacing(10);
   vbox2->addWidget(structGroup);
   vbox2->addWidget(classGroup);
   vbox2->addStretch();

   // add both groupboxes to horizontal layout
   hbox->addWidget(gb_entries);
   hbox->addWidget(gb_sectioning);
   hbox->setStretchFactor(gb_entries,3);
   hbox->setStretchFactor(gb_sectioning,4);

   // set default sectioning commands
   comboclasses->insertItem("latex");
   QStringList *sectcommands = new QStringList;
   *sectcommands << "part" << "chapter" << "section" << "subsection" << "subsubsection";
   m_docclasses["latex"] = sectcommands;
   showSectioning(m_docclasses["latex"]);
   remove->setEnabled(false);
   
    connect(m_structurelevel,SIGNAL(valueChanged(int)),this,SLOT(spinboxChanged(int)));
    connect(comboclasses,SIGNAL(activated(const QString &)),this,SLOT(comboboxChanged(const QString &)));
    connect(add,SIGNAL(clicked()),this,SLOT(clickedAdd()));

   add->setEnabled(false); 
}

ConfigStructure::~ConfigStructure()
{
	QMap<QString, const QStringList*>::Iterator it, end = m_docclasses.end();
	for(it = m_docclasses.begin() ; it != end ; ++it)
		delete *it;
}

void ConfigStructure::polish()
{
 //  QWidget::polish();

   uint w = m_entriestable->sizeHint().width();
   m_entriestable->setColumnWidth(0,3*w/7);
   m_entriestable->setColumnWidth(1,2*w/7);
   m_entriestable->setColumnWidth(2,2*w/7+1);
   m_entriestable->setColumnStretchable(2,true);

   w = m_sectioningtable->sizeHint().width();
   m_sectioningtable->setColumnWidth(0,w/6);
   m_sectioningtable->setColumnWidth(1,3*w/6);
   m_sectioningtable->setColumnWidth(2,2*w/6+1);
   m_sectioningtable->setColumnStretchable(2,true);

}

void ConfigStructure::clickedTable(KileTable *table,int row, int col)
{
   if ( table==m_entriestable && row>=0 && row<6 ) {
      if ( col==1 ) 
         m_visible[row]->setChecked( !m_visible[row]->isChecked() );
      else if ( col == 2 ) {
         if ( m_defaultopen[row]->isChecked() ) {
            m_defaultopen[row]->setChecked(false);
            m_defaultopen[row]->setText("close");
         } else {
            m_defaultopen[row]->setChecked(true);
            m_defaultopen[row]->setText("open");
         } 
      }
   }   
}

void ConfigStructure::spinboxChanged(int)
{
    if ( m_docclasses.contains( comboclasses->currentText() )  ) {
       changeSectioning(m_docclasses[comboclasses->currentText()]);
    }
}

void ConfigStructure::comboboxChanged(const QString &name)
{
    if ( m_docclasses.contains(name)  ) {
       showSectioning(m_docclasses[name]);
       remove->setEnabled( name != "latex" );
    }
}

void ConfigStructure::changeSectioning(const QStringList *list)
{
   for (uint i=0; i<list->count(); ++i) {
      QString label =  ( i < (uint)m_structurelevel->value() ) ? "open" : "close";
      m_sectioningtable->setText(i,2,label);
   }
}

void ConfigStructure::showSectioning(const QStringList *list)
{
   QString label1,label2,label3;
   for (uint i=0; i<5; ++i) {
      if ( i < list->count() ) {
         label1 = QString("%1").arg(i+1);
         label2 = (*list)[i];
         label3 =  ( i < (uint)m_structurelevel->value() ) ? "open" : "close";
      } else {
         label1 = label2 = label3 = QString::null;
      }
      KileCenteredTableItem *item1 = new KileCenteredTableItem(m_sectioningtable,
                                                   QTableItem::Never,label1);
      m_sectioningtable->setItem( i,0,item1  );
      m_sectioningtable->setText( i,1,label2 );
      KileCenteredTableItem *item3 = new KileCenteredTableItem(m_sectioningtable,
                                                    QTableItem::Never,label3);
      m_sectioningtable->setItem( i,2,item3 );
   }
}

void ConfigStructure::clickedAdd()
{
}

//////////////////// read/write configuration ////////////////////

void ConfigStructure::readConfig(KConfig *config)
{
   // config section
   config->setGroup( "Structure Entries" );
   for ( uint i=0; i<m_entries.count(); ++i ) {
      int defaultvalue = ( m_entries[i] == "Sectioning" ) ? KileStructure::Visible | KileStructure::Opened
                                                          : KileStructure::Visible;
      int num = config->readNumEntry(m_entries[i],defaultvalue);
      m_visible[i]->setChecked( (num & KileStructure::Visible) ? true : false );
      if ( num & KileStructure::Opened ) {
         m_defaultopen[i]->setChecked(true);
         m_defaultopen[i]->setText("open");
      }
   }

   config->setGroup( "Structure Sectioning" );
   QStringList classlist = config->readListEntry("classes");
   classlist.sort();
   for ( uint i=0; i<classlist.count(); ++i ) {
      QStringList list = config->readListEntry(classlist[i]);
      if ( list.count() > 0 ) {
         comboclasses->insertItem(classlist[i]);
         QStringList *sectioningcommands = new QStringList(list);
         m_docclasses[classlist[i]] = sectioningcommands;
     }
   }

   m_structurelevel->setValue(config->readNumEntry("DefaultLevel",3));
}

void ConfigStructure::writeConfig(KConfig *config)
{
   // config section
   config->setGroup( "Structure Entries" );
   for ( uint i=0; i<m_entries.count(); ++i ) {
      int num = ( m_visible[i]->isChecked() ) ?  KileStructure::Visible : KileStructure::None;
      if  ( m_defaultopen[i]->isChecked() )
         num += KileStructure::Opened;
      config->writeEntry(m_entries[i],num);   
   }

   config->setGroup( "Structure Sectioning" );
   QStringList classlist;
   for ( int i=0; i<comboclasses->count(); ++i ) {
      QString entry = comboclasses->text(i);
      if ( entry != "latex" ) {
         classlist << entry;
         QString entrylist = m_docclasses[entry]->join(",");
         config->writeEntry( entry, entrylist );
      }    
   }
   config->writeEntry( "classes", classlist );

   config->writeEntry("DefaultLevel",m_structurelevel->value());
}

}
#include "configstructure.moc"
