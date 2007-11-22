/***************************************************************************
    begin                : Sun Dec 28 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
                               2005-2007  by Holger Danielsson
    email                : Jeroen.Wijnhout@kdemail.net
                           holger.danielsson@versanet.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// 2005-11-02: dani
//  - cleaning up source (local variables etc.)
//  - added different QToolTips for each item
//  - add more types of KilelistViewItems
//  - KileStruct::Sect and KileStruct::BeginFloat will remember assigned labels,
//    which are displayed as QToolTips, if these labels are defined in the
//    same or the next line
//  - Caption for type KileStruct::BeginFloat are displayed in the title
//    of this item
//  - \includegraphics and float environment items are displayed
//  - if an item has a companion label, you can use the context menu (right mouse)
//    to insert this label as reference, as a page reference or only the keyword
//    into the text or copy it to the clipboard.
//  - graphics files have also a context menu to open them with a special program 

// 2005-12-08: dani
//  - make some items like labels, bibitems, graphics and float environments 
//    configurable for the user

// 2005-12-16: dani
//  - add listview item for undefined references

// 2007-02-15: dani
// - class KileListViewItem gets two new members to 
//   save the real cursor position of the command

// 2007-03-12 dani
//  - use KileDocument::Extensions

// 2007-03-17 dani
//  - remember how document structure is collapsed, when structure view is refreshed

// 2007-03-24: dani
// - preliminary minimal support for Beamer class
// - \begin{frame}...\end{frame} and \frame{...} are shown in the structure view
// - if a \frametitle command follows as next LaTeX command, its parameter
//   is taken to replace the standard title of this entry in the structure view
// - \begin{block}...\end{block} is taken as child of a frame

// 2007-04-06 dani
// - add TODO/FIXME section to structure view

#include "kilestructurewidget.h"

#include <qfileinfo.h>
#include <q3header.h>
#include <qregexp.h>
#include <qclipboard.h>
 
#include <kapplication.h>
#include "kiledebug.h"
#include <kmessagebox.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kurl.h>
#include <krun.h>
#include <kmimetype.h>
#include <ktexteditor/markinterface.h>

#include "kileinfo.h"
#include "kiledocmanager.h"
#include "kiledocumentinfo.h"
#include "kileconfig.h"

////////////////////// KileListViewItem with all info //////////////////////

KileListViewItem::KileListViewItem(Q3ListViewItem * parent, Q3ListViewItem * after, const QString &title, const KURL &url, uint line, uint column, int type, int level, uint startline, uint startcol) : 
	KListViewItem(parent,after),
	m_title(title), m_url(url), m_line(line), m_column(column), m_type(type), m_level(level),
	m_startline(startline), m_startcol(startcol)
{
	setItemEntry();
}

KileListViewItem::KileListViewItem(Q3ListView * parent, const QString & label) : 
	KListViewItem(parent,label),
	m_title(label), m_url(KURL()), m_line(0),  m_column(0), m_type(KileStruct::None), m_level(0) 
{}

KileListViewItem::KileListViewItem(Q3ListViewItem * parent, const QString & label) :
	KListViewItem(parent,label),
	m_title(label), m_url(KURL()), m_line(0),  m_column(0), m_type(KileStruct::None), m_level(0) 
{}

void KileListViewItem::setTitle(const QString &title)
{
	m_title = title;
	setItemEntry();
}

void KileListViewItem::setItemEntry()
{
	setText(0, m_title + " (" + i18n("line") + ' ' + QString::number(m_line) + ')');
}

////////////////////// introduce a new ToolTip //////////////////////

KileListViewToolTip::KileListViewToolTip(KListView *listview) : QToolTip(listview->viewport()), m_listview(listview) 
{}

void KileListViewToolTip::maybeTip(const QPoint &p) 
{
	if ( ! m_listview )
		return;
	
	const KileListViewItem *item = (KileListViewItem *)m_listview->itemAt(p);
//	if ( !item || item->label().isNull() )
	if ( !item )
		return;
	
	const QRect rect = m_listview->itemRect(item);
	if ( ! rect.isValid() )
		return;
	
	if ( ! item->label().isNull() )
	{
		tip(rect,i18n("Label: ") + item->label());
	}
	else if ( item->line()==0 && item->column()==0 && item->parent()==0L )  // only root
	{
		tip(rect,i18n("Click left to jump to the line. A double click will open\n a text file or a graphics file. When a label is assigned\nto this item, it will be shown when the mouse is over\nthis item. Items for a graphics file or an assigned label\nalso offer a context menu (right mouse button).")); 
	}
}


namespace KileWidget
{
	////////////////////// StructureList listview //////////////////////
	
	StructureList::StructureList(Structure *stack, KileDocument::Info *docinfo) : 
		KListView(stack),
		m_stack(stack), m_docinfo(docinfo)
	{
		show();
		header()->hide();
		addColumn(i18n("Structure"),-1);
		setSorting(-1,true);
		setAllColumnsShowFocus(true);
		setFullWidth(true);
		setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);

		QToolTip::remove(this);
		new KileListViewToolTip(this);
		
		//connect(this, SIGNAL(clicked(QListViewItem *)), m_stack, SLOT(slotClicked(QListViewItem *)));
		connect(this, SIGNAL(doubleClicked(Q3ListViewItem *)), m_stack, SLOT(slotDoubleClicked(Q3ListViewItem *)));
		connect(this, SIGNAL(contextMenu(KListView *, Q3ListViewItem *, const QPoint & )), m_stack, SLOT(slotPopup(KListView *, Q3ListViewItem * , const QPoint & )));
		
		connect(this, SIGNAL(executed(Q3ListViewItem*)), m_stack, SLOT(slotClicked(Q3ListViewItem*)));
		connect(m_stack, SIGNAL(configChanged()), this, SLOT(slotConfigChanged()));
		
		init();
	}

	StructureList::~StructureList() {}

	void StructureList::init()
	{
		QString title = (m_docinfo == 0L ) ? i18n("No \"structure data\" to display.") : m_docinfo->url().fileName();
		m_root =  new KileListViewItem(this, title);
		if ( m_docinfo != 0L )
		{
			m_root->setURL(m_docinfo->url());
			m_root->setOpen(true);
			m_root->setPixmap(0, SmallIcon("contents"));
			connect(m_docinfo, SIGNAL(foundItem(const QString&, uint, uint, int, int, uint, uint, const QString &, const QString &)), 
			        this, SLOT(addItem(const QString&, uint, uint, int, int, uint, uint, const QString &, const QString &)));
// 			connect(m_docinfo, SIGNAL(doneUpdating()), this, SLOT(insertInMasterList()));
		}

		m_parent[0]=m_parent[1]=m_parent[2]=m_parent[3]=m_parent[4]=m_parent[5]=m_parent[6]=m_root;
		m_lastType = KileStruct::None;
		m_lastSectioning = 0L;
		m_lastFloat = 0L;
		m_lastFrame = 0L;
		m_lastFrameEnv = 0L;
		m_stop = false;

		m_folders.clear();
		m_references.clear();

		if ( m_docinfo )
		{
 			m_openStructureLabels = m_docinfo->openStructureLabels();
 			m_openStructureReferences = m_docinfo->openStructureReferences();
			m_openStructureBibitems = m_docinfo->openStructureBibitems();
			m_openStructureTodo = m_docinfo->openStructureTodo();
			m_showStructureLabels = m_docinfo->showStructureLabels();
		}
		else
		{
			m_openStructureLabels = false;
			m_openStructureReferences = false;
			m_openStructureBibitems = false;
			m_openStructureTodo = false;
			m_showStructureLabels = false;
 		}
	}

	void StructureList::updateRoot() 
	{
		m_root->setURL( m_docinfo->url() ); 
		m_root->setText(0, m_docinfo->url().fileName() );		
	}

	void StructureList::cleanUp(bool preserveState/* = true */)
	{
        	KILE_DEBUG() << "==void StructureList::cleanUp()========" << endl;
		if(preserveState)
			saveState();
		clear();
		if(m_docinfo)
			disconnect(m_docinfo, 0, this, 0);
		init();
	}
	
	void StructureList::slotConfigChanged(){
		QWidget *current = m_stack->visibleWidget();
		if(!current)
			return;
		cleanUp(false);
 		m_stack->update(m_docinfo,true,false);
		m_stack->raiseWidget(current);
	}

	void StructureList::saveState()
	{
		KILE_DEBUG() << "===void StructureList::saveState()" << endl;
		m_openByTitle.clear();
		m_openByLine.clear();
		m_openByFolders.clear();

		Q3ListViewItemIterator it(this);
		KileListViewItem *item = 0L;
		while ( it.current() ) 
		{
			item = (KileListViewItem*)it.current();
			if ( item->firstChild() )
			{
				//we don't accept duplicate entries in the map, remove this item alltogether
				//and rely on the openByLine map instead
				if ( m_openByTitle.contains ( item->title() ) )
					m_openByTitle.remove ( item->title() );
				else
					m_openByTitle [ item->title() ] = item->isOpen();

				m_openByLine [ item->line() ] = item->isOpen();
			}
			++it;
		}

		if ( m_folders.contains("labels") ) 
			m_openByFolders["labels"] = m_folders["labels"]->isOpen();
		if ( m_folders.contains("refs") ) 
			m_openByFolders["refs"] = m_folders["refs"]->isOpen();
		if ( m_folders.contains("bibs") ) 
			m_openByFolders["bibs"] = m_folders["bibs"]->isOpen();
		if ( m_folders.contains("todo") ) 
			m_openByFolders["todo"] = m_folders["todo"]->isOpen();
		if ( m_folders.contains("fixme") ) 
			m_openByFolders["fixme"] = m_folders["fixme"]->isOpen();
	}

	bool StructureList::shouldBeOpen(KileListViewItem *item, const QString & folder, int level)
	{
		if ( item->parent() == 0L )
			return true;
		if ( folder == "labels" )
		{
			if ( m_openByFolders.contains("labels") )
				return m_openByFolders["labels"];
			else
				return m_openStructureLabels;
		}
		else if ( folder == "refs" )
		{
			if ( m_openByFolders.contains("refs") )
				return m_openByFolders["refs"];
			else
				return m_openStructureReferences;
		}
		else if ( folder == "bibs" )
		{
			if ( m_openByFolders.contains("bibs") )
				return m_openByFolders["bibs"];
			else
				return m_openStructureBibitems;
		}
		else if ( folder=="todo" || folder=="fixme" )
		{
			if ( m_openByFolders.contains(folder) )
				return m_openByFolders[folder];
			else
				return m_openStructureTodo;
		}

		if ( m_openByTitle.contains(item->title()) )
			return m_openByTitle [ item->title() ];
		else if ( m_openByLine.contains(item->line()) )
			return m_openByLine [ item->line() ]; //TODO check surrounding lines as well
		else
			return ((folder == "root") && level <= m_stack->level());
	}

	KileListViewItem* StructureList::createFolder(const QString &folder)
	{
		KileListViewItem *fldr=  new KileListViewItem(m_root, folder);
		fldr->setOpen(false);
		if ( folder == "labels" )
		{
			fldr->setText(0, i18n("Labels"));
			fldr->setPixmap(0, SmallIcon("label"));
		}
		else if ( folder == "bibs" )
		{
			fldr->setText(0, i18n("BibTeX References"));
			fldr->setPixmap(0, SmallIcon("viewbib"));
		}
		else if ( folder == "refs" )
		{
			fldr->setText(0, i18n("Undefined References"));
			fldr->setPixmap(0, SmallIcon("stop"));
		}
		else if ( folder == "todo" )
		{
			fldr->setText(0, i18n("TODO"));
			fldr->setPixmap(0, SmallIcon("bookmark"));
		}
		else if ( folder == "fixme" )
		{
			fldr->setText(0, i18n("FIXME"));
			fldr->setPixmap(0, SmallIcon("bookmark"));
		}

		m_folders[ folder ] = fldr;

		return m_folders[folder];
	}

	KileListViewItem* StructureList::folder(const QString &folder)
	{
		KileListViewItem *item = m_folders[folder];
		if ( item == 0L ) item = createFolder(folder);
		return item;
	}

	void StructureList::activate()
	{
		m_stack->raiseWidget(this);
	}

	KileListViewItem *StructureList::parentFor(int lev, const QString & fldr)
	{
		KileListViewItem *par = 0L;

		if ( fldr == "root" )
		{
			switch (lev)
			{
				case KileStruct::Object :  
				case KileStruct::File :  
					par = ( m_lastSectioning == 0L ) ? m_root : m_lastSectioning;
				break;
				
				case 0 : 
				case 1 :
					par = m_root;
				break;

				default:
					par = m_parent[lev - 2];
				break;
			}
		}
		else
		{
			par = folder(fldr);
		}

		return par;
	}

////////////////////// add a new item to the listview //////////////////////

	/* some items have a special action:
		- KileStruct::Sect: 
		      The new item is saved in m_lastSectioning, so that all following entries 
		      can be inserted as children. \part will drop back to level 0 of the Listview,
		      all other sectioning commands will be children of the last sectioning item.
		      If a \label command follows in the same or the next line, it is assigned
		      to this item.
		- KileStruct::BeginFloat:
		      The new item is saved in m_lastFloat. If a \caption command follows before
		      the floating environment is closed, it is inserted into the title of this item. 
		      If a \label command follows, it is assigned to this float item.
		- KileStruct::EndFloat
		      Reset m_lastFloat to 0L to close this environment. No more \caption or \label 
		      commands are assigned to this float after this.
		- KileStruct::Caption
		      If a float environment is opened, the caption is assigned to the float item.
		      A caption item has hidden attribute, so that no other action is performed and
		      function addItem() will return immediately.
		- KileStruct::Label
		      If we are inside a float, this label is assigned to this environment. If the last 
		      type was a sectioning command on the current line or the line before, the label is 
		      assigned to this sectioning item. Assigning means that a popup menu will open, 
		      when the mouse is over this item.
		- KileStruct::BeamerBeginFrame
		      The new item is saved in m_lastFrameEnv. If a \frametitle command follows before
		      the frame environment is closed, it is inserted into the title of this item. 
		      If a \label command follows, it is assigned to this float item.
		- KileStruct::BeamerEndFrame
		      Reset m_lastFloatEnv to 0L to close this environment. No more \frametitle 
		      or \label commands are assigned to this frame after this.
		- KileStruct::BeamerBeginBlock
		      Inside a beamer frame this environment is taken as child of this frame
		- KileStruct::BeamerFrame
		      The new item is saved in m_lastFrame. If a \frametitle command follows
		      immediately as next command, it is inserted into the title of this item. 
		*/
		
	void StructureList::addItem(const QString &title, uint line, uint column, int type, int lev,
	                            uint startline, uint startcol, 
	                            const QString & pix, const QString &fldr /* = "root" */)
	{
//  		KILE_DEBUG() << "\t\taddItem: " << title << ", with type " <<  type << endl;
		if ( m_stop ) return;
		
		// some types need a special action
		if ( type == KileStruct::Reference )
		{
			m_references.prepend( KileReferenceData(title,line,column) );
		}
		else if ( type==KileStruct::Caption && m_lastFloat )
		{
			QString floattitle = m_lastFloat->title();
			if ( floattitle=="figure" || floattitle=="table" )
				m_lastFloat->setTitle(floattitle+": "+title);
		}
		else if ( type == KileStruct::EndFloat )
		{
			m_lastFloat = 0L;
		}
		else if ( type==KileStruct::BeamerFrametitle )
		{
			if (  m_lastFrameEnv )
				m_lastFrameEnv->setTitle(title);
			else if (  m_lastFrame )
				m_lastFrame->setTitle(title);		
		}
		else if ( type == KileStruct::BeamerEndFrame )
		{
			m_lastFrameEnv = 0L;
		}
		m_lastFrame = 0L;
		
		// that's all for hidden types: we must immediately return
		if ( lev == KileStruct::Hidden ) 
		{
			//KILE_DEBUG() << "\t\thidden item: not created" << endl;
			return;
		}
			
		//KILE_DEBUG() << "\t\tcreate new item" << endl;
		// check if we have to update a label before loosing this item
		if ( type==KileStruct::Label )
		{
			if ( m_lastFloat )
			{
				m_lastFloat->setLabel(title);
			}
			else if ( m_lastType==KileStruct::Sect )
			{
				// check if this is a follow up label for the last sectioning item
				if ( m_lastSectioning && (m_lastLine==line-1 || m_lastLine==line) )
					m_lastSectioning->setLabel(title);
			}
			else if ( m_lastType==KileStruct::BeamerBeginFrame )
			{
				m_lastFrameEnv->setLabel(title);
			}

			if(!m_showStructureLabels) // if we don't want to have it displayed return here
					return;
		}
		
		// remember current type and line for the next call of addItem()
		m_lastType = type;
		m_lastLine = line;

		//find the parent for the new element
		KileListViewItem *parentItem = ( type==KileStruct::BeamerBeginBlock && m_lastFrameEnv ) ? m_lastFrameEnv : parentFor(lev, fldr);
		if ( parentItem == 0L )
		{
			KMessageBox::error(0,i18n("Can't create ListviewItem: no parent found."));
			return;
		}

		//find the last element at this level
		KileListViewItem *lastChild = 0L;
		KileListViewItem *nextChild = (KileListViewItem *)parentItem->firstChild();
		while ( nextChild )
		{
			lastChild = nextChild;
			nextChild = (KileListViewItem *)nextChild->nextSibling();
		}
		
		// create a new item
		KileListViewItem *newChild = new KileListViewItem(parentItem, lastChild, title, m_docinfo->url(), line, column, type, lev, startline, startcol);
		if ( ! pix.isNull() ) 
			newChild->setPixmap(0,SmallIcon(pix));
		//m_stop = true;
		
		//if the level is not greater than the defaultLevel
		//open the parentItem to make this item visible
		parentItem->setOpen( shouldBeOpen(parentItem,fldr,lev) );
		
		//update the m_parent levels, such that section etc. get inserted at the correct level
		//m_current = newChild;
		if ( lev > 0)
		{
			m_parent[lev-1] = newChild;
			for (int l = lev; l < 7; ++l)
				m_parent[l] = newChild;
		}
		else if ( lev == 0 )
		{
			m_lastSectioning = 0L;
			for ( int l = 0; l < 7; ++l ) m_parent[l] = m_root;
		}
		
		// check if we have to remember the new item for setting a label or caption
		if ( type == KileStruct::Sect )
			m_lastSectioning = newChild;
		else if ( type == KileStruct::BeginFloat )
			m_lastFloat = newChild;
		else if ( type == KileStruct::BeamerBeginFrame )
			m_lastFrameEnv = newChild;
		else if ( type == KileStruct::BeamerFrame )
			m_lastFrame = newChild;
	}

	void StructureList::showReferences(KileInfo *ki)
	{
		// remove old listview item for references, if it exists
		if ( m_folders.contains("refs") )
		{
			KileListViewItem *refitem = m_folders["refs"];
			m_root->takeItem(refitem);
			delete refitem;

			m_folders.remove("refs");
		}

		//KILE_DEBUG() << "==void StructureList::showReferences()========" << endl;
		//KILE_DEBUG() << "\tfound " << m_references.count() << " references" << endl;
		if ( m_references.count() == 0 )
			return;
			
		// read list with all labels
		const QStringList *list = ki->allLabels();
		//KILE_DEBUG() << "\tfound " << list->count() << " labels" << endl;
		QMap<QString,bool> labelmap;
		for ( QStringList::ConstIterator itmap=list->begin(); itmap!=list->end(); ++itmap ) 
		{
			labelmap[(*itmap)] = true;
		}

		// now check if there are unsolved references
		Q3ValueListConstIterator<KileReferenceData> it;
		for ( it=m_references.begin(); it!=m_references.end(); ++it )
		{
			if ( ! labelmap.contains((*it).name()) )
			{ 
				KileListViewItem *refitem = folder("refs");
				refitem->setOpen( shouldBeOpen(refitem,"refs",0) );
				new KileListViewItem(refitem,0L,(*it).name(),m_docinfo->url(),(*it).line(),(*it).column(),KileStruct::Reference,KileStruct::NotSpecified, 0,0 );
			}
		}
	}

	////////////////////// Structure: QWidgetStack //////////////////////

	Structure::Structure(KileInfo *ki, QWidget * parent, const char * name) : 
		Q3WidgetStack(parent,name),
		m_ki(ki),
		m_docinfo(0L)
	{
		KILE_DEBUG() << "==KileWidget::Structure::Structure()===========" << endl;
		setLineWidth(0);
		setMidLineWidth(0);
		setMargin(0);
		setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);

		m_default = new StructureList(this, 0L);
		m_default->activate();
	
		m_popup = new KPopupMenu(this, "structureview_popup");
	}

	Structure::~Structure()
	{ 
	}

	int Structure::level()
	{
		return KileConfig::defaultLevel();
	}

	void Structure::addDocumentInfo(KileDocument::Info *docinfo)
	{
		StructureList *view = new StructureList(this, docinfo);
		addWidget(view);
		m_map.insert(docinfo, view, true);
	}

	void Structure::slotClicked(Q3ListViewItem * itm)
	{
		KILE_DEBUG() << "\tStructure::slotClicked" << endl;

		KileListViewItem *item = (KileListViewItem *)itm;
		//return if user didn't click on an item
		if (! item) return;

		if (! (item->type() & KileStruct::None ))
			emit(setCursor(item->url(), item->line()-1, item->column()));
		else if ( item->parent() == 0L ) //root item
			emit(setCursor(item->url(), 0, 0));
	}

	void Structure::slotDoubleClicked(Q3ListViewItem * itm)
	{
		KILE_DEBUG() << "\tStructure::slotDoubleClicked" << endl;
		KileListViewItem *item = (KileListViewItem*)(itm);
		static QRegExp::QRegExp suffix("\\.[\\d\\w]*$");
		
		if (!item)
			return;
		
		KILE_DEBUG() <<"item->url() is " << item->url() << ", item->title() is " << item->title() << endl;
		
		if ( item->type() & (KileStruct::Input | KileStruct::Bibliography | KileStruct::Graphics) )
		{
			QString fname = item->title();
			
			
			if(fname.find(suffix) != -1) // check if we have a suffix, if not add standard suffixes
			{
				KILE_DEBUG() << "Suffix found: " << suffix.cap(0) << endl;
			}
			else
			{
				// filename in structureview entry has no extension: this shouldn't happen anymore,
				// because all have got one in function updateStruct(). But who knows?
				if ( item->type() == KileStruct::Input )
					fname += m_ki->extensions()->latexDocumentDefault();
				else if ( item->type() == KileStruct::Bibliography )
					fname += m_ki->extensions()->bibtexDefault();
				else
				{
					// FIXME get graphics suffix from an project config option
				}
			}
			
			if(fname.left(1) != "/") // no absolute path
			{
				QString fn = m_ki->getCompileName();
				fname= QFileInfo(fn).path() + '/' + fname;
			}
			
			QFileInfo fi(fname);
			KURL url;
			url.setPath(fname);
			
			if (fi.isReadable())
			{	
				if (  item->type() == KileStruct::Graphics )
				{
					KMimeType::Ptr pMime = KMimeType::findByURL(url);
					KRun::runURL(url,pMime->name());
				}
				else
					emit(fileOpen(url, QString::null));
			}
			else
			{
				if ( KMessageBox::warningYesNo(this, i18n("Cannot find the included file. The file does not exist, is not readable or Kile is unable to determine the correct path to it. The filename causing this error was: %1.\nDo you want to create this file?").arg(fname), i18n("Cannot Find File"))
			== KMessageBox::Yes)
				{
					emit(fileNew(url));
				}
			}
		}
	}

	// all popup items get different id's, so that we can see, what item is activated
	//  - label:       1 -  6
	//  - sectioning: 10 - 16
	//  - graphics:   100ff

	void Structure::slotPopup(KListView *, Q3ListViewItem *itm, const QPoint &point)
	{
		KILE_DEBUG() << "\tStructure::slotPopup" << endl;
		
		m_popupItem = (KileListViewItem *)(itm);
		if ( ! m_popupItem )
			return;
			
		m_popup->clear();
		m_popup->disconnect();
		
		bool hasLabel = ! m_popupItem->label().isEmpty();
		if ( m_popupItem->type() == KileStruct::Sect )
		{
			if ( hasLabel )
				m_popup->insertTitle(i18n("Sectioning"));
			m_popup->insertItem(i18n("Cu&t"),SectioningCut);
			m_popup->insertItem(i18n("&Copy"),SectioningCopy);
			m_popup->insertItem(i18n("&Paste below"),SectioningPaste);
			m_popup->insertSeparator();
			m_popup->insertItem(i18n("&Select"),SectioningSelect);
			m_popup->insertItem(i18n("&Delete"),SectioningDelete);
			m_popup->insertSeparator();
			m_popup->insertItem(i18n("C&omment"),SectioningComment);
			m_popup->insertSeparator();
			m_popup->insertItem(i18n("Run QuickPreview"), SectioningPreview);
		}
		else if ( m_popupItem->type() == KileStruct::Graphics )
		{
			m_popupInfo = m_popupItem->title();
						
			if(m_popupInfo.left(1) != "/") // no absolute path
			{
				QString fn = m_ki->getCompileName();
				m_popupInfo = QFileInfo(fn).path() + '/' + m_popupInfo;
			}
			
			QFileInfo fi(m_popupInfo);
			if ( fi.isReadable() )
			{
				KURL url;
				url.setPath(m_popupInfo);
				
				m_offerList = KTrader::self()->query(KMimeType::findByURL(url)->name(), "Type == 'Application'");
				for (uint i=0; i < m_offerList.count(); ++i)
				{
					m_popup->insertItem(SmallIcon(m_offerList[i]->icon()), m_offerList[i]->name(), i+SectioningGraphicsOfferlist);
				}
				m_popup->insertSeparator();
				m_popup->insertItem(i18n("Other..."), SectioningGraphicsOther);
			}
		}
		
		if ( hasLabel)
		{
			m_popup->insertTitle(i18n("Insert Label"));
			m_popup->insertItem(i18n("As &reference"),1);
			m_popup->insertItem(i18n("As &page reference"),2);
			m_popup->insertItem(i18n("Only the &label"),3);
			m_popup->insertSeparator();
			m_popup->insertTitle(i18n("Copy Label to Clipboard"));
			m_popup->insertItem(i18n("As reference"),4);
			m_popup->insertItem(i18n("As page reference"),5);
			m_popup->insertItem(i18n("Only the label"),6);
		}

		if ( m_popup->count() > 0 )
		{
			connect(m_popup,SIGNAL(activated(int)),this,SLOT(slotPopupActivated(int)));
			m_popup->exec(point);
		}
	}

	void Structure::slotPopupActivated(int id)
	{
		if ( id>=1 && id<=6 )
			slotPopupLabel(id);
		else if ( id>=SectioningCut && id<=SectioningPreview )
			slotPopupSectioning(id);
		else if ( id>=SectioningGraphicsOther && id<=SectioningGraphicsOfferlist+(int)m_offerList.count() )
			slotPopupGraphics(id);
	}

	// id's 1..6 (already checked)
	void Structure::slotPopupLabel(int id)
	{
		KILE_DEBUG() << "\tStructure::slotPopupLabel (" << id << ")"<< endl;
		
		QString s = m_popupItem->label();    
		if ( id==1 || id==4 )
			s = "\\ref{" + s + '}';
		else if ( id==2 || id==5 )
			s = "\\pageref{" + s + '}';
			
		if ( id <= 3 )
			emit( sendText(s) );
		else
			QApplication::clipboard()->setText(s);
 	}

	// id's 10..16 (already checked)
	void Structure::slotPopupSectioning(int id)
	{
		KILE_DEBUG() << "\tStructure::slotPopupSectioning (" << id << ")"<< endl;
		if ( m_popupItem->level()>=1 && m_popupItem->level()<=7 )
			emit( sectioningPopup(m_popupItem,id) );
	}

	// id's 100ff (already checked)
	void Structure::slotPopupGraphics(int id)
	{
		KILE_DEBUG() << "\tStructure::slotPopupGraphics (" << id << ")"<< endl;

		KURL url;
		url.setPath(m_popupInfo);
		
		if ( id == SectioningGraphicsOther )
			KRun::displayOpenWithDialog(url);
		else
			KRun::run(*m_offerList[id-SectioningGraphicsOfferlist],url);
	}

	StructureList* Structure::viewFor(KileDocument::Info *info)
	{
		if ( info == 0L ) return 0L;

		if ( ! viewExistsFor(info) )
			m_map.insert(info, new StructureList(this, info), true);

		return  m_map[info];
	}

	bool Structure::viewExistsFor(KileDocument::Info *info)
	{
		if ( info == 0L ) return false;
		else
			return m_map.contains(info);
	}

	void Structure::closeDocumentInfo(KileDocument::Info *docinfo)
	{
		m_docinfo = 0L;
		if ( m_map.contains(docinfo) )
		{
			StructureList *data = m_map[docinfo];
			m_map.remove(docinfo);
			delete data;
		}
		
		if ( m_map.isEmpty() ) m_default->activate();
	}

	void Structure::clear()
	{
		QMapIterator<KileDocument::Info *, StructureList *> it;
		QMapIterator<KileDocument::Info *, StructureList *> itend(m_map.end());
		for ( it = m_map.begin(); it != itend; ++it)
			if ( it.data() != 0L ) delete it.data();

		m_map.clear();
		m_docinfo = 0L;

		m_default->activate();
	}

	void Structure::updateUrl(KileDocument::Info *docinfo)
	{
		StructureList *view = viewFor(docinfo);
		if ( view )
			view->updateRoot();
	}

	void Structure::update(KileDocument::Info *docinfo)
	{
		update(docinfo, true);
	}

	void Structure::update(KileDocument::Info *docinfo, bool parse, bool activate /* =true */)
	{
		KILE_DEBUG() << "==KileWidget::Structure::update(" << docinfo << ")=============" << endl;

		if ( docinfo == 0L ) 
		{
			m_default->activate();
			return;
		}

		m_docinfo = docinfo;

		bool needParsing = parse || ( ! viewExistsFor(m_docinfo) );

		//find structview-item for this docinfo
		StructureList *view = viewFor(m_docinfo);

		if ( needParsing ) //need to reparse the doc
		{
			int xtop = view->contentsX();
			int ytop = view->contentsY();
			//KILE_DEBUG() << "\tStructure::update parsing doc" << endl;
			view->cleanUp();
			m_docinfo->updateStruct();
			view->showReferences(m_ki);
			view->setContentsPos(xtop,ytop);
		}

		if(activate)
		{
			KILE_DEBUG() << "===Structure::update() activating view" << endl;
			view->activate();
		}
	}

    void Structure::clean(KileDocument::Info *docinfo)
    {
        KILE_DEBUG() << "==void Structure::clean()========" << endl;
        StructureList *view = viewFor(docinfo);
        if (view) view->cleanUp();
    }

	void Structure::updateReferences(KileDocument::Info *docinfo)
	{
		KILE_DEBUG() << "==void StructureList::updateReferences()========" << endl;
		StructureList *view = viewFor(docinfo);
		if (view) 
		{
			view->showReferences(m_ki);
		}
	}

	////////////////////// Structure: find sectioning //////////////////////

	bool Structure::findSectioning(Kate::Document *doc, uint row, uint col, bool backwards, uint &sectRow, uint &sectCol)
	{
		KileDocument::TextInfo *docinfo = m_ki->docManager()->textInfoFor(doc);
		if ( ! docinfo )
			return false;

		bool found = false;
		uint foundRow,foundCol;
		StructureList *structurelist = viewFor(docinfo);
		Q3ListViewItemIterator it( structurelist );
		while ( it.current() ) 
		{
			KileListViewItem *item = (KileListViewItem *)(it.current());
			if  ( item->type() == KileStruct::Sect )
			{
				foundRow = item->startline() - 1;
				foundCol = item->startcol() - 1;
				if ( backwards )
				{
					if ( foundRow<row || (foundRow==row &&foundCol<col) )
					{
						sectRow = foundRow;
						sectCol = foundCol;
						found = true;
					}
					else
					{
						return found;
					}
					
				}
				else if ( !backwards && (foundRow>row || (foundRow==row &&foundCol>col)) )
				{
					sectRow = foundRow;
					sectCol = foundCol;
					return true;
				}
			}
			++it;
		}

		return found;
	}

}

#include "kilestructurewidget.moc"
