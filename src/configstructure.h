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

#ifndef CONFIGSTRUCTURE_H
#define CONFIGSTRUCTURE_H

#include <qwidget.h>
#include <q3table.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
//Added by qt3to4:
#include <QMouseEvent>

class KConfig;

class QRect;
class QPainter;
class QSpinBox;
class QComboBox;
class QPushButton;

/**
  *@author Holger Danielsson
  */

namespace KileStructure
{

  enum  { None=0x0, Visible=0x1, Opened=0x2 };
  
class KileCenteredTableItem : public Q3TableItem
{
public:
   KileCenteredTableItem(Q3Table *table,EditType et,const QString& text) : Q3TableItem(table,et,text) {}
   void paint(QPainter* p,const QColorGroup& cg,const QRect& cr,bool selected);  
   int alignment() const { return Qt::AlignHCenter; } 
};

class KileTable : public Q3Table
{
   Q_OBJECT
public:
   KileTable(QWidget *dialog, QWidget *parent=0, const char *name=0);
   void paintFocus(QPainter *, const QRect &) {}
protected:
   void contentsMousePressEvent(QMouseEvent *ev);
signals:
   void clickedCell(KileTable *,int row,int cell);
};


class ConfigStructure : public QWidget
{
    Q_OBJECT
public: 
   ConfigStructure(QWidget *parent=0, const char *name=0);
   ~ConfigStructure();

   void readConfig(KConfig *config);
   void writeConfig(KConfig *config);

private:
   QStringList m_entries;
   QComboBox *comboclasses;
   QSpinBox *m_structurelevel;
   QPushButton *add, *remove;

   KileTable *m_entriestable,*m_sectioningtable;
   Q3CheckTableItem *m_visible[6], *m_defaultopen[6];
   QMap<QString,const QStringList *> m_docclasses;

   void showSectioning(const QStringList *list);
   void changeSectioning(const QStringList *list);

public slots:
   void polish();

private slots:
  void clickedTable(KileTable *table,int row,int col);
  void clickedAdd();
  void spinboxChanged(int);
  void comboboxChanged(const QString &name);
};

}
#endif
