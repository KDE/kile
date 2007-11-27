/***************************************************************************
    begin                : Sun Dec 28 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
                               2005-2007  by Holger Danielsson
    email                : Jeroen.Wijnhout@kdemail.net
                         : holger.danielsson@versanet.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
  ***************************************************************************/

#ifndef KILEWIDGET_STRUCTURE_H
#define KILEWIDGET_STRUCTURE_H
  
 /**
  * @author Jeroen Wijnhout, Holger Danielsson
  **/

#include <q3widgetstack.h>
#include <q3vbox.h>
#include <qtooltip.h>
//Added by qt3to4:
#include <Q3ValueList>

#include <k3listview.h>
#include <kmenu.h>
#include <ktrader.h>

#include "kiledocumentinfo.h"

//2007-02-15: dani
// - class KileListViewItem not only saves the cursor position of the parameter,
//   but also the real cursor position of the command

class QString;
class KUrl;
class KileInfo;
class Q3ListViewItem;

/**
 * ListView items that can hold some additional information appropriate for the Structure View. The
 * additional information is: line number, title string.
 **/
 
class KileListViewItem : public K3ListViewItem
{
public:
	KileListViewItem(Q3ListViewItem * parent, Q3ListViewItem * after, const QString &title, const KUrl &url, uint line, uint m_column, int type, int level, uint startline, uint startcol);
	KileListViewItem(Q3ListView * parent, const QString & label);
	KileListViewItem(Q3ListViewItem * parent, const QString & label);

	/** @returns the title of this element (for a label it return the label), without the (line ...) part **/
	const QString& title() const { return m_title; }
	/** @returns the line number of the structure element. **/
	const uint line() const { return m_line; }
	/** @returns the column number of the structure element, right after the { **/
	const uint column() const { return m_column; }
	/** @returns the type of element, see @ref KileStruct **/
	const int type() const { return m_type; }
	const uint startline() const { return m_startline; }
	const uint startcol() const { return m_startcol; }
	/**@returns the file in which this item was found*/
	const KUrl & url() const { return m_url; }
	void setURL(const KUrl & url) { m_url = url; }

	const int level() const { return m_level; }
	const QString &label() const { return m_label; }
	
	void setTitle(const QString &title);
	void setLabel(const QString &label) { m_label = label; }

private:
	QString  m_title;
	KUrl     m_url;
	uint     m_line;
	uint     m_column;
	int      m_type, m_level;
	uint     m_startline;
	uint     m_startcol;
	QString  m_label;
	
	void setItemEntry();
};

class KileListViewToolTip : public QToolTip
{
public:
	KileListViewToolTip(K3ListView *listview);
protected:
	void maybeTip(const QPoint &p);
private:
	K3ListView *m_listview;
};

class KileReferenceData
{
public:
	KileReferenceData() {}
	KileReferenceData(const QString &name,uint line,uint column) : m_name(name), m_line(line), m_column(column) {}
	~KileReferenceData() {}
	
	const QString &name() const { return m_name; }
	const uint line() const { return m_line; }
	const uint column() const { return m_column; }
	
private:
	QString m_name;
	uint m_line;
	uint m_column;
};

namespace KileWidget
{
	class Structure; //forward declaration

	class StructureList : public K3ListView
	{
		Q_OBJECT

	public:
		StructureList(Structure *stack, KileDocument::Info *docinfo);
		~StructureList();

		void activate();
		void cleanUp(bool preserveState = true);
		void showReferences(KileInfo *ki);
		
		KUrl url() const { return m_docinfo->url(); }
		void updateRoot();

	public slots:
		void addItem(const QString &title, uint line, uint column, int type, int level, uint startline, uint startcol,
		             const QString & pix, const QString &folder = "root" );
		void slotConfigChanged();

	private:
		KileListViewItem *parentFor(int lev, const QString & fldr);

		void init();
		KileListViewItem* createFolder(const QString &folder);
		KileListViewItem* folder(const QString &folder);

		void saveState();
		bool shouldBeOpen(KileListViewItem *item, const QString & folder, int level);

	private:
		Structure							*m_stack;
		KileDocument::Info					*m_docinfo;
		QMap<QString, KileListViewItem *>	m_folders;
		QMap<QString, bool>					m_openByTitle;
		QMap<uint, bool>					m_openByLine;
		QMap<QString, bool>					m_openByFolders;
		KileListViewItem					*m_parent[7], *m_root;
		Q3ValueList<KileReferenceData> m_references;
		bool m_openStructureLabels;
		bool m_openStructureReferences;
		bool m_openStructureBibitems;
		bool m_openStructureTodo;
		bool m_showStructureLabels;

		int m_lastType;
		uint m_lastLine;
		KileListViewItem *m_lastSectioning;
		KileListViewItem *m_lastFloat;
		KileListViewItem *m_lastFrame;
		KileListViewItem *m_lastFrameEnv;
		
		bool m_stop;
	};

	class Structure : public Q3WidgetStack
	{
		Q_OBJECT

		public:
			Structure(KileInfo *, QWidget * parent, const char * name = 0);
			~Structure();

			int level();
			KileInfo *info() { return m_ki; }

			bool findSectioning(KTextEditor::Document *doc, uint row, uint col, bool backwards, uint &sectRow, uint &sectCol);
			void updateUrl(KileDocument::Info *docinfo);

		enum { SectioningCut=10, SectioningCopy=11, SectioningPaste=12, 
		       SectioningSelect=13, SectioningDelete=14, 
		       SectioningComment=15,
		       SectioningPreview=16,
		       SectioningGraphicsOther=100, SectioningGraphicsOfferlist=101
		     };

		public slots:
			void slotClicked(Q3ListViewItem *);
			void slotDoubleClicked(Q3ListViewItem *);
			void slotPopup(K3ListView *, Q3ListViewItem *itm, const QPoint &point);
			void slotPopupActivated(int id);

			void addDocumentInfo(KileDocument::Info *);
			void closeDocumentInfo(KileDocument::Info *);
			void update(KileDocument::Info*);
			void update(KileDocument::Info *, bool, bool activate =true);
			void clean(KileDocument::Info *);
			void updateReferences(KileDocument::Info *);

			/**
			* Clears the structure widget and empties the map between KileDocument::Info objects and their structure trees (QListViewItem).
			**/
			void clear();

		signals:
			void sendText(const QString &);
			void setCursor(const KUrl &, int, int);
			void fileOpen(const KUrl &, const QString &);
			void fileNew(const KUrl &);
			void configChanged();
			void sectioningPopup(KileListViewItem *item, int id);

		private:
			StructureList* viewFor(KileDocument::Info *info);
			bool viewExistsFor(KileDocument::Info *info);

			void slotPopupLabel(int id);
			void slotPopupSectioning(int id);
			void slotPopupGraphics(int id);

		private:
			KileInfo									*m_ki;
			KileDocument::Info							*m_docinfo;
			QMap<KileDocument::Info *, StructureList *>	m_map;
			StructureList								*m_default;
			
			KMenu *m_popup;
			KileListViewItem *m_popupItem;
			QString m_popupInfo;
			
			KTrader::OfferList m_offerList;
	};
}

#endif
