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

#include <klistview.h>

class QString;
class KURL;
class KileInfo;
class KileDocumentInfo;
class QListViewItem;

namespace KileWidget
{
	class Structure : public KListView
	{
		Q_OBJECT

		public:
			Structure(KileInfo *, QWidget * parent, const char * name = 0);

			void setLevel(int level) { m_level = level; }
			int level() { return m_level; }

		public slots:
			void slotClicked(QListViewItem *);
			void slotDoubleClicked(QListViewItem *);

			void closeDocument(KileDocumentInfo *);
			void update(KileDocumentInfo *, bool);
			void addItem(const QString &title, uint line, uint m_column, int type, int level, const QString & pix);

		signals:
			void setCursor(int, int);
			void fileOpen(const KURL &, const QString &);
			void fileNew(const KURL &);

		private:
			void init();

		private:
			KileInfo	*m_ki;
			int		m_level;
			KileDocumentInfo	*m_docinfo;
			QMap<KileDocumentInfo *, QListViewItem *> m_map;
			QListViewItem	*m_parent[5], *m_label, *m_current, *m_root, *m_child, *m_lastChild;
	};
}

#endif
