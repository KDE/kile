/***************************************************************************
                          kilestructurewidget.h  -  description
                             -------------------
    begin                : Sun Dec 28 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
    email                : Jeroen.Wijnhout@kdemail.net
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
  * @author Jeroen Wijnhout
  **/

#include <qwidgetstack.h>
#include <qvbox.h>

#include <klistview.h>

#include "kiledocumentinfo.h"

class QString;
class KURL;
class KileInfo;
class QListViewItem;

/**
 * ListView items that can hold some additional information appropriate for the Structure View. The
 * additional information is: line number, title string.
 **/
 
class KileListViewItem : public KListViewItem
{
public:
	KileListViewItem(QListViewItem * parent, QListViewItem * after, QString title, uint line, uint m_column, int type);
	KileListViewItem(QListView * parent, QString label) : KListViewItem(parent,label), m_title(label), m_line(0),  m_column(0),m_type(KileStruct::None) {}
	KileListViewItem(QListViewItem * parent, QString label) : KListViewItem(parent,label), m_title(label), m_line(0),  m_column(0), m_type(KileStruct::None) {}

	/** @returns the title of this element (for a label it return the label), without the (line ...) part **/
	const QString& title() { return m_title; }
	/** @returns the line number of the structure element. **/
	const uint line() { return m_line; }
	/** @returns the column number of the structure element, right after the { **/
	const uint column() { return m_column; }
	/** @returns the type of element, see @ref KileStruct **/
	const int type() { return m_type; }

private:
	QString		m_title;
	uint			m_line;
	uint			m_column;
	int			m_type;
};

namespace KileWidget
{
	class Structure; //forward declaration

	class StructureList : public KListView
	{
		Q_OBJECT

	public:
		StructureList(Structure *stack, KileDocument::Info *docinfo);

		void cleanUp();

	public slots:
		void addItem(const QString &title, uint line, uint column, int type, int level, const QString & pix, const QString &folder = "root");

	private:
		void init();
		QListViewItem* createFolder(const QString &folder);
		QListViewItem* folder(const QString &folder);

		void saveState();
		bool shouldBeOpen(KileListViewItem *item, QString folder, int level);

	private:
		Structure *m_stack;
		KileDocument::Info *m_docinfo;
		QMap<QString, QListViewItem *> m_folders;
		QMap<QString, bool> m_openByTitle;
		QListViewItem	*m_parent[5], *m_current, *m_root, *m_child, *m_lastChild;
	};

	class Structure : public QWidgetStack
	{
		Q_OBJECT

		public:
			Structure(KileInfo *, QWidget * parent, const char * name = 0);

			void setLevel(int level) { m_level = level; }
			int level() { return m_level; }

		public slots:
			void slotClicked(QListViewItem *);
			void slotDoubleClicked(QListViewItem *);

			void addDocumentInfo(KileDocument::Info *);
			void closeDocumentInfo(KileDocument::Info *);
			void update(KileDocument::Info *, bool);

			/**
			* Clears the structure widget and empties the map between KileDocument::Info objects and their structure trees (QListViewItem).
			**/
			void clear();

		signals:
			void setCursor(int, int);
			void fileOpen(const KURL &, const QString &);
			void fileNew(const KURL &);

		private:
			StructureList* viewFor(KileDocument::Info *info);

		private:
			KileInfo			*m_ki;
			int				m_level;
			KileDocument::Info	*m_docinfo;
			QMap<KileDocument::Info *, StructureList *> m_map;
	};
}

#endif
