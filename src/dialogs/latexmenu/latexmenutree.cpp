/***************************************************************************
    begin                : Oct 03 2011
    author               : dani
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <QFile>
#include <QFileInfo>
#include <QHeaderView>
#include <QDomDocument>
#include <QSignalMapper>

#if QT_VERSION >= 0x040600
#include <QProcessEnvironment>
#endif
	
#include <KIcon>
#include <KIconLoader>
#include <KMenu>
#include <KLocale>
#include <KInputDialog>
#include <KMessageBox>


#include "dialogs/latexmenu/latexmenuitem.h"
#include "dialogs/latexmenu/latexmenutree.h"

#include "kiledebug.h"


// Qt::UserRole+1: menutype
// Qt::UserRole+2: errorcode for menu item

namespace KileMenu {

// - Separators are shown as a horizontal line (Qt:UserRole+1)
// - Menu items with errors are displayed in red (Qt:UserRole+2)
void MenuentryDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex& index) const
{
	QString menutitle = index.data(Qt::DisplayRole).toString();
	int error = index.data(Qt::UserRole+2).toInt();
	
	// any errors?
	if (index.column()==0 && error!=LatexmenuItem::MODEL_ERROR_NONE ) {
		QStyleOptionViewItem optionCustom = option;
		optionCustom.palette.setColor(QPalette::Text, Qt::red);
		QStyledItemDelegate::paint( painter, optionCustom, index );
	} 
	else {
		QStyledItemDelegate::paint( painter, option, index ); 
	}

	// display separators
	QString itemtype = index.data(Qt::UserRole+1).toString();
	if ( itemtype == "separator" ) {
		QRect r = option.rect;     
		int y =  ( r.bottom() + r.top() ) / 2;  

		painter->save();
		QPen pen = QPen(Qt::gray);
		painter->setPen(pen);
		painter->drawLine(r.left()+3,y, r.right()-20,y);
		painter->restore();	     
	} 

}
///////////////////////////// LatexmenuTree //////////////////////////////

LatexmenuTree::LatexmenuTree(QWidget *parent)
	: QTreeWidget(parent)
{
	setColumnCount(2);
	
	header()->setResizeMode(0,QHeaderView::Stretch);
	header()->setResizeMode(1,QHeaderView::Fixed);
	header()->setMovable(false);
	header()->setStretchLastSection(false);
	setColumnWidth(1,140);
	setItemDelegateForColumn(0, new MenuentryDelegate(parent));
	
	// drag and drop
	setDragEnabled(true);
	setDropIndicatorShown(true);
	//setAcceptDrops(true);
	setDragDropMode(QAbstractItemView::InternalMove);
	setDragDropOverwriteMode(false);
	
	initEnvPathlist();
}

bool LatexmenuTree::isEmpty()
{
	return ( topLevelItemCount() == 0 );
}

///////////////////////////// PATH environment variable //////////////////////////////

// save PATH to search for executable files
void LatexmenuTree::initEnvPathlist()
{
	QString envpath;
#if QT_VERSION >= 0x040600
	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
	if ( env.contains("PATH") ) {
		envpath = env.value("PATH");
	}
#else
	// Returns the environment of the calling process as a list of key=value pairs.
	QStringList environment = QProcess::systemEnvironment();
	foreach ( const QString &s, environment ) {
		if ( s.startsWith("PATH=") ) {
			envpath = s.mid(5);
			break;
		}
	}
#endif
	
#ifdef Q_WS_WIN
	m_envPathlist = envpath.split(';');
#else
	m_envPathlist = envpath.split(':');
#endif
	m_envPathlist.append(".");
}

bool LatexmenuTree::isItemExecutable(const QString &filename)
{
	// absolute paths can be checked immediately
	QFileInfo fi(filename);
	if ( fi.isAbsolute() ) {
		return fi.isExecutable();
	}

	// search in all paths
	for (int i=0; i<m_envPathlist.size(); ++i ) {
		bool executable = QFileInfo(m_envPathlist[i]+"/"+filename).isExecutable();
		if ( executable ) {
			// move to front
			if ( i > 0 ) {
				QString temp = m_envPathlist[0];
				m_envPathlist[0] = m_envPathlist[i];
				m_envPathlist[i] = temp;				
			}
			return true;
		}
	}
	
	return false;
}

///////////////////////////// context menu event //////////////////////////////

// build a context menu
void LatexmenuTree::contextMenuRequested(const QPoint &pos)
{
	KILE_DEBUG() << "context menu requested ..." ;
	
	m_popupItem = dynamic_cast<LatexmenuItem*>(itemAt(pos));
	if ( !m_popupItem ) {
		KILE_DEBUG() << "... no item found";
		return;
	} 
	
	KILE_DEBUG() << "... popup item found: " << m_popupItem->text(0);
	bool submenu = ( m_popupItem->menutype() ==  LatexmenuData::Submenu );
	bool separator = ( m_popupItem->menutype() ==  LatexmenuData::Separator );
	
	KMenu popup;
	QAction *action = NULL;
	QSignalMapper signalMapper;
	connect(&signalMapper, SIGNAL(mapped(int)), this, SLOT(slotPopupActivated(int)));
	
	// insert standard menu items
	action = popup.addAction(KIcon("latexmenu-insert-above.png"),i18n("Insert above"), &signalMapper, SLOT(map()));
	signalMapper.setMapping(action, POPUP_INSERT_ABOVE);
	action = popup.addAction(KIcon("latexmenu-insert-below.png"),i18n("Insert below"), &signalMapper, SLOT(map()));
	signalMapper.setMapping(action, POPUP_INSERT_BELOW);
	popup.addSeparator();

	// insert separators
	if ( !separator ) {
		action = popup.addAction(KIcon("latexmenu-separator-above.png"),i18n("Insert a separator above"), &signalMapper, SLOT(map()));
		signalMapper.setMapping(action, POPUP_SEPARATOR_ABOVE);
		action = popup.addAction(KIcon("latexmenu-separator-below.png"),i18n("Insert a separator below"), &signalMapper, SLOT(map()));
		signalMapper.setMapping(action, POPUP_SEPARATOR_BELOW);
		popup.addSeparator();
	}
	
	// insert submenus
	action = popup.addAction(KIcon("latexmenu-submenu-above.png"),i18n("Insert a submenu above"), &signalMapper, SLOT(map()));
	signalMapper.setMapping(action, POPUP_SUBMENU_ABOVE);
	action = popup.addAction(KIcon("latexmenu-submenu-below.png"),i18n("Insert a submenu below"), &signalMapper, SLOT(map()));
	signalMapper.setMapping(action, POPUP_SUBMENU_BELOW);
	popup.addSeparator();

	// insert into submenus
	if ( submenu ) {
		action = popup.addAction(KIcon("latexmenu-into-submenu.png"),i18n("Insert into this submenu"), &signalMapper, SLOT(map()));
		signalMapper.setMapping(action, POPUP_INTO_SUBMENU);
		action = popup.addAction(i18n("Insert a separator into this submenu"), &signalMapper, SLOT(map()));
		signalMapper.setMapping(action, POPUP_SEPARATOR_INTO_SUBMENU);
		action = popup.addAction(i18n("Insert a submenu into this submenu"), &signalMapper, SLOT(map()));
		signalMapper.setMapping(action, POPUP_SUBMENU_INTO_SUBMENU);
		popup.addSeparator();
	}

	// delete actions
	action = popup.addAction(KIcon("latexmenu-delete.png"),i18n("Delete this item"), &signalMapper, SLOT(map()));
	signalMapper.setMapping(action,POPUP_DELETE_ITEM);
	popup.addSeparator();
	action = popup.addAction(KIcon("latexmenu-clear.png"),i18n("Delete the complete tree"), &signalMapper, SLOT(map()));
	signalMapper.setMapping(action, POPUP_DELETE_TREE);
	
	// expand/collapse tree
	if ( submenu ) {
		popup.addSeparator();
		if ( m_popupItem->isExpanded() ) {
			action = popup.addAction(i18n("Collapse submenu"), &signalMapper, SLOT(map()));
			signalMapper.setMapping(action,POPUP_COLLAPSE_ITEM);
		}
		else  {
			action = popup.addAction(i18n("Expand submenu"), &signalMapper, SLOT(map()));
			signalMapper.setMapping(action,POPUP_EXPAND_ITEM);
		}
		popup.addSeparator();
		action = popup.addAction(i18n("Collapse complete tree"), &signalMapper, SLOT(map()));
		signalMapper.setMapping(action,POPUP_COLLAPSE_TREE);
		action = popup.addAction(i18n("Expand complete tree"), &signalMapper, SLOT(map()));
		signalMapper.setMapping(action,POPUP_EXPAND_TREE);
	}
	
	// if there are any errors with this item, some info is available
	int error = m_popupItem->data(0,Qt::UserRole+2).toInt();
	if ( error != LatexmenuItem::MODEL_ERROR_NONE ) {
		popup.addSeparator();
		action = popup.addAction(KIcon("help-about.png"),i18n("Info"), &signalMapper, SLOT(map()));
		signalMapper.setMapping(action, POPUP_ITEM_INFO);
	}
	
	// const QPoint& pos parameter in the customContextMenuRequested() signal is normally in widget coordinates. 
	// But classes like QTreeWidget, which inherit from QAbstractScrollArea1 instead use the coordinates of their viewport()
	if ( !popup.isEmpty() ) {
		popup.exec( viewport()->mapToGlobal(pos) );
	}
}

// a context menu action was selected
void LatexmenuTree::slotPopupActivated(int id)
{
	KILE_DEBUG() << "popup activated with id: " << id;
	switch (id ) {
		case POPUP_INSERT_ABOVE:           insertMenuItem (m_popupItem, false);                      break;
		case POPUP_INSERT_BELOW:           insertMenuItem (m_popupItem, true);                       break;
		case POPUP_SEPARATOR_ABOVE:        insertSeparator(m_popupItem, false);                      break;
		case POPUP_SEPARATOR_BELOW:        insertSeparator(m_popupItem, true);                       break;
		case POPUP_SUBMENU_ABOVE:          insertSubmenu  (m_popupItem, false);                      break;
		case POPUP_SUBMENU_BELOW:          insertSubmenu  (m_popupItem, true);                       break;
		case POPUP_INTO_SUBMENU:           insertIntoSubmenu(m_popupItem, LatexmenuData::Text);      break;
		case POPUP_SEPARATOR_INTO_SUBMENU: insertIntoSubmenu(m_popupItem, LatexmenuData::Separator); break;
		case POPUP_SUBMENU_INTO_SUBMENU:   insertIntoSubmenu(m_popupItem, LatexmenuData::Submenu);   break;
		case POPUP_DELETE_ITEM:            itemDelete(m_popupItem);                                  break;
		case POPUP_DELETE_TREE:            deleteMenuTree();                                         break;
		case POPUP_COLLAPSE_ITEM:          m_popupItem->setExpanded(false);                          break;
		case POPUP_EXPAND_ITEM:            m_popupItem->setExpanded(true);                           break;
		case POPUP_COLLAPSE_TREE:          collapseAll();                                            break;
		case POPUP_EXPAND_TREE:            expandAll();                                              break;
		case POPUP_ITEM_INFO:              itemInfo(m_popupItem);                                    break;
	}
}

///////////////////////////// read XML //////////////////////////////

// read a xml  file and check for errors
bool LatexmenuTree::readXml(const QString &filename)
{
	KILE_DEBUG() << "read xml file " << filename;

	QDomDocument doc("Latexmenu");
	QFile file(filename);
	if ( !file.open(QFile::ReadOnly | QFile::Text) ) {
		KMessageBox::error(this, i18n("File '%1' does not exist.", filename));
		return false;
	}
	if ( !doc.setContent( &file ) ) {
		file.close();
		return false;
	}
	file.close();
   
	KILE_DEBUG() << "parse xml ...";
	blockSignals(true);
  
	// clear menutree
	clear();

	// read toplevelitems
	QDomElement root = doc.documentElement();
	QDomElement e = root.firstChildElement();
	while ( !e.isNull()) {
		QString tag = e.tagName();
     
		LatexmenuItem *item = 0L;
		if ( tag == "submenu" ) {
			item = readXmlSubmenu(e);
		}
		else if ( tag == "separator" ) {
			item = readXmlSeparator();
		}
		else /* if ( tag == "menu" ) */ {
			item = readXmlMenuentry(e);
		}
  
		if ( item ) {
			addTopLevelItem(item);
		}
		e = e.nextSiblingElement();
	}

	// the xml menu is build, now check for errors 
	setErrorCodes();

	// polish menutree
	expandAll();
	if ( topLevelItemCount() > 0 ) {
		setCurrentItem( topLevelItem(0) );
	}
	blockSignals(false);
	
	return true;
}

// a separator tag was found
LatexmenuItem *LatexmenuTree::readXmlSeparator() 
{
	return new LatexmenuItem(LatexmenuData::Separator,QString::null);  
}

// read tags for a submenu
LatexmenuItem *LatexmenuTree::readXmlSubmenu(const QDomElement &element) 
{
	LatexmenuItem *submenuitem = new LatexmenuItem(LatexmenuData::Submenu,QString::null) ;

	QString title = QString::null;
	if ( element.hasChildNodes() ) {
		QDomElement e = element.firstChildElement();
		while ( !e.isNull()) {
			LatexmenuItem *item = 0L;
	  
			QString tag = e.tagName();  
			if ( tag == "title" ) {
				title = e.text();
			}
			else if ( tag == "submenu" ) {
				item = readXmlSubmenu(e);
			}
			else if ( tag == "separator" ) {
				item = readXmlSeparator();
			}
			else /* if ( tag == "menu" ) */ {
				item = readXmlMenuentry(e);
			}
	  
			submenuitem->setMenutitle(title);
			submenuitem->setText(0,title);
	  
			if ( item ) {
				submenuitem->addChild(item);
			}
			e = e.nextSiblingElement();
		} 
	}
    
	return submenuitem;
}

// read tags for a standard menu item
LatexmenuItem *LatexmenuTree::readXmlMenuentry(const QDomElement &element) 
{
	QString menutypename = element.attribute("type");
	LatexmenuData::MenuType menutype = LatexmenuData::xmlMenuType(menutypename);
   
	LatexmenuItem *menuentryitem = new LatexmenuItem(menutype,QString::null) ;
    
	// default values
	QString title = QString::null;
	QString plaintext = QString::null;
	QString filename = QString::null;
	QString parameter = QString::null;
	QString icon = QString::null;
	QString shortcut = QString::null;
	bool needsSelection = false;
	bool useContextMenu = false;
	bool replaceSelection = false;
	bool selectInsertion = false;
	bool insertOutput = false;
    
	// read values
	if ( element.hasChildNodes() ) {
		QDomElement e = element.firstChildElement();
		while ( !e.isNull()) {
			QString tag = e.tagName();
			QString text = e.text();
	  
			int index = LatexmenuData::xmlMenuTag(tag);  
			switch (index) {
				case  LatexmenuData::XML_TITLE:            title = text;                       break;
				case  LatexmenuData::XML_PLAINTEXT:        plaintext = text;                   break;      
				case  LatexmenuData::XML_FILENAME:         filename = text;                    break;      
				case  LatexmenuData::XML_PARAMETER:        parameter = text;                   break;      
				case  LatexmenuData::XML_ICON:             icon = text;                        break;         
				case  LatexmenuData::XML_SHORTCUT:         shortcut = text;                    break;    
				case  LatexmenuData::XML_NEEDSSELECTION:   needsSelection   = str2bool(text);  break;    
				case  LatexmenuData::XML_USECONTEXTMENU:   useContextMenu   = str2bool(text);  break;      
				case  LatexmenuData::XML_REPLACESELECTION: replaceSelection = str2bool(text);  break;    
				case  LatexmenuData::XML_SELECTINSERTION:  selectInsertion  = str2bool(text);  break;    
				case  LatexmenuData::XML_INSERTOUTPUT:     insertOutput     = str2bool(text);  break;     
			}
	  
			e = e.nextSiblingElement();
		}
       
		// save values
		menuentryitem->setMenutitle(title);
	
		// add code newline 
		plaintext.replace("\\n","\n");
		menuentryitem->setPlaintext(plaintext);

		menuentryitem->setFilename(filename);  
		menuentryitem->setParameter(parameter);  
		if ( !icon.isEmpty() ) {
			menuentryitem->setMenuicon(icon);
			menuentryitem->setIcon(0,KIcon(icon));
		}
		if ( !shortcut.isEmpty() ) {
			QKeySequence seq = QKeySequence::fromString(shortcut,QKeySequence::PortableText);
			shortcut = seq.toString(QKeySequence::NativeText);

			menuentryitem->setShortcut(shortcut); 
			menuentryitem->setText(1,shortcut);
		}
		menuentryitem->setNeedsSelection(needsSelection);  
		menuentryitem->setUseContextMenu(useContextMenu);  
		menuentryitem->setReplaceSelection(replaceSelection);  
		menuentryitem->setSelectInsertion(selectInsertion);  
		menuentryitem->setInsertOutput(insertOutput);  
       
		menuentryitem->setText(0,title);
	}
    
	return menuentryitem;
}


///////////////////////////// write XML //////////////////////////////

bool LatexmenuTree::writeXml(const QString &filename)
{
	KILE_DEBUG() << "write xml file " << filename;

	QFile file(filename);
	if ( !file.open(QFile::WriteOnly | QFile::Text) ) {
		KMessageBox::error(this, i18n("File '%1' could not be opened to save the latexmenu file.", filename));
		return false;
	}

	QXmlStreamWriter xmlWriter(&file);
	xmlWriter.setAutoFormatting(true);
	xmlWriter.setAutoFormattingIndent(2) ;

	xmlWriter.writeStartDocument();
	xmlWriter.writeStartElement("Latexmenu");
	
	for (int i = 0; i < topLevelItemCount(); ++i) {
		writeXmlItem(&xmlWriter, dynamic_cast<LatexmenuItem *>(topLevelItem(i)) );
	}
	xmlWriter.writeEndDocument();

	file.close();
	return true;
}

void LatexmenuTree::writeXmlItem(QXmlStreamWriter *xml, LatexmenuItem *item)
{
	switch (item->menutype()) {
		case LatexmenuData::Separator: writeXmlSeparator(xml);       break;
		case LatexmenuData::Submenu:   writeXmlSubmenu(xml,item);    break;
		default:                       writeXmlMenuentry(xml,item);  break;
	}
}

// write xml tags for a standard menu item
void LatexmenuTree::writeXmlMenuentry(QXmlStreamWriter *xml, LatexmenuItem *item)
{
	int menutype = item->menutype();
	QString menutypename = LatexmenuData::xmlMenuTypeName(menutype);
	
	xml->writeStartElement("menu");
	xml->writeAttribute("type", menutypename);
	
	QString menutitle = ( item->text(0) == EMPTY_MENUENTRY ) ? QString::null : item->text(0);
	xml->writeTextElement(LatexmenuData::xmlMenuTagName(LatexmenuData::XML_TITLE),menutitle);

	if ( menutype == LatexmenuData::Text ) {
		QString plaintext = item->plaintext();
		if ( !plaintext.isEmpty() ) {
			// replace newline code
			plaintext.replace('\n',"\\n");
			xml->writeTextElement(LatexmenuData::xmlMenuTagName(LatexmenuData::XML_PLAINTEXT), plaintext);
		}
	} 
	else /* if ( menutype!=LatexmenuData::FileContent || menutype==LatexmenuData:Program) */ {
		// both types use a filename
		QString filename = item->filename();
		if ( !filename.isEmpty() ) {
			xml->writeTextElement(LatexmenuData::xmlMenuTagName(LatexmenuData::XML_FILENAME), filename);
 
			// but LatexmenuItem::Program may have an additional parameter
			if ( menutype == LatexmenuData::Program) {
				QString parameter = item->parameter();  
				if ( !parameter.isEmpty() ) {
					xml->writeTextElement(LatexmenuData::xmlMenuTagName(LatexmenuData::XML_PARAMETER), parameter);
				}
			}
		}
	}
  
	QString icon = item->menuicon();
	if ( !icon.isEmpty() ) {
		xml->writeTextElement(LatexmenuData::xmlMenuTagName(LatexmenuData::XML_ICON),icon);
	}
  
	QKeySequence seq = QKeySequence::fromString( item->shortcut(), QKeySequence::NativeText );
	if ( !seq.isEmpty() ) {
		xml->writeTextElement(LatexmenuData::xmlMenuTagName(LatexmenuData::XML_SHORTCUT), seq.toString(QKeySequence::PortableText));
	}
     
	if ( item->needsSelection() ) {
		xml->writeTextElement(LatexmenuData::xmlMenuTagName(LatexmenuData::XML_NEEDSSELECTION), "true");
	}
	if ( item->useContextMenu() ) {
		xml->writeTextElement(LatexmenuData::xmlMenuTagName(LatexmenuData::XML_USECONTEXTMENU), "true");
	}
	if ( item->replaceSelection() ) {
		xml->writeTextElement(LatexmenuData::xmlMenuTagName(LatexmenuData::XML_REPLACESELECTION), "true");
	}
	if ( item->selectInsertion() ) {
		xml->writeTextElement(LatexmenuData::xmlMenuTagName(LatexmenuData::XML_SELECTINSERTION), "true");
	}
	if ( item->insertOutput() ) {
		xml->writeTextElement(LatexmenuData::xmlMenuTagName(LatexmenuData::XML_INSERTOUTPUT), "true");
	}
   
	xml->writeEndElement();
}

// write xml tags for a submenu
void LatexmenuTree::writeXmlSubmenu(QXmlStreamWriter *xml, LatexmenuItem *item)
{
	xml->writeStartElement("submenu");
	
	QString menutitle = item->text(0);
	if ( menutitle == EMPTY_MENUENTRY ) {
		menutitle = QString::null;
	}
	else if ( menutitle.right(LENGTH_SUBSTITUTE) == EMPTY_SUBMENU ) {
		menutitle = menutitle.left(menutitle.length()-LENGTH_SUBSTITUTE);
	}
	xml->writeTextElement(LatexmenuData::xmlMenuTagName(LatexmenuData::XML_TITLE),menutitle);

	for ( int i=0; i<item->childCount(); ++i ) {
		writeXmlItem(xml, dynamic_cast<LatexmenuItem *>(item->child(i)) );
	}

	xml->writeEndElement();
}

// write xml tag for a separator
void LatexmenuTree::writeXmlSeparator(QXmlStreamWriter *xml)
{
	xml->writeStartElement("separator");
	xml->writeEndElement();
}

///////////////////////////// check menutree and set error codes //////////////////////////////

// the complete menutree was build, now check for errors
//  - empty menutitles
//  - useless submenus without children
//  - empty filenames, not existing or not executable files
void LatexmenuTree::setErrorCodes()
{
	KILE_DEBUG() << "check menutree for errors and set error codes ...";
	
	for (int i = 0; i < topLevelItemCount(); ++i) {
		LatexmenuItem *item = dynamic_cast<LatexmenuItem *>(topLevelItem(i));
		LatexmenuData::MenuType type = item->menutype();

		bool executable = ( type==LatexmenuData::Program && isItemExecutable(item->filename()) ); 
		item->setModelData(executable);                         
		
		if ( type != LatexmenuData::Separator ) {        
			checkMenuTitle(item);
		}
		if ( type == LatexmenuData::Submenu ) {
			checkSubmenu(item); 
		}
	}
}

void LatexmenuTree::checkMenuTitle(LatexmenuItem *item)
{
	if ( item->menutitle().isEmpty() ) {
		item->setText(0,EMPTY_MENUENTRY);
		KILE_DEBUG() << "empty menutitle changed to " << EMPTY_MENUENTRY;
	}
}

void LatexmenuTree::checkSubmenu(LatexmenuItem *item)
{
	QString menutitle = item->menutitle();
	int children = item->childCount();
		
	if ( menutitle.isEmpty() ) {
		menutitle = EMPTY_MENUENTRY;
	} 
	else if ( children == 0 ) {
		menutitle += EMPTY_SUBMENU;
	}
	item->setText(0,menutitle);
		
	for ( int i=0; i<children; ++i ) {
		LatexmenuItem *child = dynamic_cast<LatexmenuItem *>(item->child(i));
		child->setModelData();
		LatexmenuData::MenuType type = child->menutype();
	
		if ( type != LatexmenuData::Separator ) {        
			checkMenuTitle(child);
		}
		if ( type == LatexmenuData::Submenu ) {
			checkSubmenu(child); 
		}
	}
}

///////////////////////////// check menutree  //////////////////////////////

// check for errors (true=no errors)
bool LatexmenuTree::errorCheck()
{
	KILE_DEBUG() << "check menutree for errors ...";
	
	for (int i = 0; i < topLevelItemCount(); ++i) {
		LatexmenuItem *item = dynamic_cast<LatexmenuItem *>(topLevelItem(i));
		LatexmenuData::MenuType type = item->menutype();
		
		if ( type != LatexmenuData::Separator ) {        
			if ( item->data(0,Qt::UserRole+2).toInt() != LatexmenuItem::MODEL_ERROR_NONE ) {
				return false;
			}
		}
		
		if ( type == LatexmenuData::Submenu ) {
			if ( checkSubmenuError(item) == false ) {
				return false;
			}
		}
	}
	
	return true;
}

bool LatexmenuTree::checkSubmenuError(LatexmenuItem *item)
{
	int children = item->childCount();
	for ( int i=0; i<children; ++i ) {
		LatexmenuItem *child = dynamic_cast<LatexmenuItem *>(item->child(i));
		LatexmenuData::MenuType type = child->menutype();
		
		if ( type != LatexmenuData::Separator ) {        
			if ( child->data(0,Qt::UserRole+2).toInt() != LatexmenuItem::MODEL_ERROR_NONE ) {
				return false;
			}
		}

		if ( type == LatexmenuData::Submenu ) {
			if ( checkSubmenuError(child) == false ) {
				return false;
			}
		}
	}
	
	return true;
}

///////////////////////////// tree ops //////////////////////////////

// insert a standard menu item
bool LatexmenuTree::insertMenuItem(QTreeWidgetItem *current, bool below)
{
	QString menulabel = getMenuTitle(i18n("Please enter a label for this menu item:"));
	if ( menulabel.isEmpty() )  {
		return false;
	}
	
	if ( below ) {
		insertMenuItemBelow(current,LatexmenuData::Text,menulabel);
	}
	else {
		insertMenuItemAbove(current,LatexmenuData::Text,menulabel);
	}
	return true;
}

// insert a submenu item
bool LatexmenuTree::insertSubmenu(QTreeWidgetItem *current, bool below)
{
	QString menulabel = getMenuTitle(i18n("Please enter a label for this submenu:"));
	if ( menulabel.isEmpty() )  {
		return false;
	}
	
	if ( below ) {
		insertMenuItemBelow(current,LatexmenuData::Submenu,menulabel);
	}
	else {
		insertMenuItemAbove(current,LatexmenuData::Submenu,menulabel);
	}
	return true;
}

// insert a separator item
bool LatexmenuTree::insertSeparator(QTreeWidgetItem *current, bool below)
{
	if ( below ) {
		insertMenuItemBelow(current,LatexmenuData::Separator,QString::null);
	}
	else {
		insertMenuItemAbove(current,LatexmenuData::Separator,QString::null);
	}
	return true;
}

void LatexmenuTree::insertMenuItemAbove(QTreeWidgetItem *current, LatexmenuData::MenuType type, const QString &menulabel)
{
	QTreeWidgetItem *parent = ( current ) ? current->parent() : 0L;
	int index = itemIndex(parent,current);
	
	LatexmenuItem *item = new LatexmenuItem(type,menulabel);
	insertItem(parent,index,item);
	
	item->setText(0,menulabel);	  
	setCurrentItem(item);
}

void LatexmenuTree::insertMenuItemBelow(QTreeWidgetItem *current, LatexmenuData::MenuType type, const QString &menulabel)
{
	LatexmenuItem *item;
	QTreeWidgetItem *parent = ( current ) ? current->parent() : 0L;
	
	if ( parent == 0L ) {
		item = new LatexmenuItem(this,current,type,menulabel);
	}
	else {
		item = new LatexmenuItem(parent,current,type,menulabel);
	}
	
	item->setText(0,menulabel);	  
	setCurrentItem(item);
}

void LatexmenuTree::insertIntoSubmenu(QTreeWidgetItem *current, LatexmenuData::MenuType type)
{
	QString menulabel = QString::null;
	if ( type==LatexmenuData::Text || type==LatexmenuData::Submenu ) {
		menulabel = getMenuTitle(i18n("Please enter a label for this entry:"));
		if ( menulabel.isEmpty() ) {
			return;
		}
	}

	LatexmenuItem *item = new LatexmenuItem(type,menulabel);
	insertItem(current,0,item);
	setCurrentItem(item);
}

// delete an item
void LatexmenuTree::itemDelete(QTreeWidgetItem *current)
{
	int children,index;
	QTreeWidgetItem *item, *selectitem;
	QTreeWidgetItem *parent = current->parent();
	if ( parent == 0L ) {
		children = topLevelItemCount(); 
		index = indexOfTopLevelItem(current);  
		if ( index < children-1 ) {
			selectitem = topLevelItem(index+1);
		}
		else if ( index > 0  ) {
			selectitem = topLevelItem(index-1);
		}
		else {
			selectitem = 0L;
		}
		
		item = takeTopLevelItem(index);
	} 
	else {
		children = parent->childCount(); 
		index = parent->indexOfChild(current);  
		if ( index < children-1 ) {
			selectitem = parent->child(index+1);
		}
		else if ( index > 0  ) {
			selectitem = parent->child(index-1);
		}
		else
			selectitem = parent; {
		}
		
		item = parent->takeChild(index); 
	}
	
	delete item;
	
	if ( selectitem != 0L ) {
		setCurrentItem(selectitem);
	}
}

// move an item one position up
void LatexmenuTree::itemUp()
{
	QTreeWidgetItem *current = currentItem(); 
	LatexmenuItem *aboveitem = dynamic_cast<LatexmenuItem *>(itemAbove(current));
	if ( aboveitem == 0L ) {
		return;
	}
	
	bool expanded = current->isExpanded();
	blockSignals(true);
	
	QTreeWidgetItem *aboveparent = aboveitem->parent();
	int aboveindex = itemIndex(aboveparent,aboveitem);
		
	LatexmenuItem *parent = dynamic_cast<LatexmenuItem *>(current->parent());
	int index = itemIndex(parent,current);  
	takeItem(parent,current);
	  
	if ( parent!=aboveparent && index!=0 ) {
		aboveindex++;
	}

	if ( parent==aboveparent &&  aboveitem->menutype()==LatexmenuData::Submenu ) {
		insertItem(aboveitem,0,current);
	}
	else {
		insertItem(aboveparent,aboveindex,current);
	}
		
	// update model data of old and new parent, if it has changed
	LatexmenuItem *newparent = dynamic_cast<LatexmenuItem *>(current->parent());
	if ( parent != newparent ) {
		if ( parent ) {
			parent->setModelData();
			parent->setText(0, parent->updateMenutitle());
		}
		if ( newparent ) {
			newparent->setModelData();
			newparent->setText(0, newparent->updateMenutitle());
		}
	}
	
	current->setExpanded(expanded);
	setCurrentItem(current);
	blockSignals(false);
}

// move an item one position down
void LatexmenuTree::itemDown()
{
	QTreeWidgetItem *current = currentItem(); 
	bool expanded = current->isExpanded();
	blockSignals(true);
	
	// get all necessary parameter
	LatexmenuItem *parent = dynamic_cast<LatexmenuItem *>(current->parent());
	int index = itemIndex(parent,current);  
	int children = numChildren(parent);
	
	// successor exists?
	if ( index < children-1 ) {
		LatexmenuItem *successor = dynamic_cast<LatexmenuItem *>( itemAtIndex(parent,index+1) );
		takeItem(parent,current);
		if ( successor->menutype() == LatexmenuData::Submenu ) {
			successor->insertChild(0,current);
		}
		else {
			insertItem(parent,index+1,current);
		}
	} 
	else if ( parent ) {
			QTreeWidgetItem *grandparent = parent->parent();
			int parentindex = itemIndex(grandparent,parent);  
			takeItem(parent,current);
			insertItem(grandparent,parentindex+1,current);
	}

	// update model data of old and new parent, if it has changed
	LatexmenuItem *newparent = dynamic_cast<LatexmenuItem *>(current->parent());
	if ( parent != newparent ) {
		if ( parent ) {
			parent->setModelData();
			parent->setText(0, parent->updateMenutitle());
		}
		if ( newparent ) {
			newparent->setModelData();
			newparent->setText(0, newparent->updateMenutitle());
		}
	}

	current->setExpanded(expanded);
	setCurrentItem(current);
	blockSignals(false);
}

////////////////////////////// delete tree //////////////////////////////

// delete the whole menutree
void  LatexmenuTree::deleteMenuTree()
{
	if ( KMessageBox::questionYesNo(this, i18n("Do you really want to clear the complete menutree?") ) == KMessageBox::Yes ) {
		blockSignals(true);
		clear();
		blockSignals(false);
	}
}

////////////////////////////// info //////////////////////////////

// prepare info for en item with errors
void  LatexmenuTree::itemInfo(LatexmenuItem *item)
{
	int error = item->data(0,Qt::UserRole+2).toInt();
	
	QStringList list;
	if ( error & LatexmenuItem::MODEL_ERROR_EMPTY ) {
		list << i18n("This menuitem has no title.");
	}
	
	if ( error & LatexmenuItem::MODEL_ERROR_SUBMENU ) {
		list << i18n("This submenu item is useless without children.");
	}

	if ( error & LatexmenuItem::MODEL_ERROR_TEXT ) {
		list << i18n("This item offers no text to insert.");
	}

	if ( error & LatexmenuItem::MODEL_ERROR_FILE_EMPTY ) {
		list << i18n("No file is given for this task.");
	}

	if ( error & LatexmenuItem::MODEL_ERROR_FILE_EXIST ) {
		list << i18n("The file for this item does not exist.");
	}

	if ( error & LatexmenuItem::MODEL_ERROR_FILE_EXECUTABLE ) {
		list << i18n("The file for this item is not executable.");
	}

	QString msg = i18n("<p><strong>Error:</strong>");
	if ( list.size() == 1 ) {
		msg += "<br/><br/>" + list[0] + "</p>";
	}
	else {
		msg += "<ul>";
		foreach ( const QString &s, list ) {
			msg += "<li>&nbsp;" + s + "</li>";
		}
		msg += "</ul></p>";
	}
	
   KMessageBox::information(this, msg, i18n("Menutree Error"));
}

////////////////////////////// auxiliary //////////////////////////////

int LatexmenuTree::itemIndex(QTreeWidgetItem *parent, QTreeWidgetItem *item)
{
	return ( parent ) ? parent->indexOfChild(item) : indexOfTopLevelItem(item);
}
  
QTreeWidgetItem *LatexmenuTree::itemAtIndex(QTreeWidgetItem *parent, int index)
{
	return ( parent ) ? parent->child(index) : topLevelItem(index);
}

int LatexmenuTree::numChildren(QTreeWidgetItem *parent)
{
	return ( parent ) ? parent->childCount() : topLevelItemCount();
}
  
void LatexmenuTree::insertItem(QTreeWidgetItem *parent, int index, QTreeWidgetItem *item)
{
	if ( parent ) {
		parent->insertChild(index,item);
	} 
	else {
		insertTopLevelItem(index,item);
	}
}

void LatexmenuTree::takeItem(QTreeWidgetItem *parent, QTreeWidgetItem *item)
{
	if ( parent ) {
		int index = parent->indexOfChild(item);
		parent->takeChild(index);
	} 
	else {
		int index = indexOfTopLevelItem(item);
		takeTopLevelItem(index);
	}
}

bool LatexmenuTree::str2bool(const QString &value)
{
	return ( value == "true" );  
}


QString LatexmenuTree::getMenuTitle(const QString &title)
{
	bool ok;
	QString value = KInputDialog::getText(i18n("Name"), title, QString::null, &ok, this);
	return ( ok ) ? value : QString::null; 
		
}


}

#include "latexmenutree.moc"
