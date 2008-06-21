/***************************************************************************
                           tabulardialog.h
----------------------------------------------------------------------------
    date                 : Sep 17 2006
    version              : 0.26
    copyright            : (C) 2005-2006 by Holger Danielsson
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

#ifndef TABULARDIALOG_H
#define TABULARDIALOG_H

#include "kilewizard.h"
#include "latexcmd.h"

#include <qevent.h>
#include <qpainter.h>
#include <q3table.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qregexp.h>
#include <qvalidator.h>
#include <QList>
//Added by qt3to4:
#include <Q3Frame>
#include <QMouseEvent>

#include <kdialog.h>
#include <kcolorcombo.h>
#include <kpushbutton.h>

class QMenu;
 
namespace KileDialog
{

namespace TabularCell_OLD
{
	enum { cbNone=0, cbLeft=1, cbTop=2, cbRight=4, cbBottom=8 };
	enum { cfNormal=0, cfBold=1, cfItalic=2 };
	
	struct Data
	{
		int align;
		int font;
		int border;
		QColor bgcolor;
		QColor textcolor;
	};
		
	struct Count
	{
		int bold;
		int italic;
		int bgcolor;
		int textcolor;
		int cells;
		QString nameBgcolor;
		QString nameTextcolor;
	};

	struct Preamble
	{
		bool vline;
		bool bold;
		bool italic;
		int align;
		QString bgcolor;
		QString textcolor;
	};
	
	struct CountLines
	{
		int cnt;
		int cells;
		QList<int> list;
	};

}

class TabCellFrame : public Q3Frame  
{
	Q_OBJECT
public:
	
	TabCellFrame(QWidget* parent);
	void setBorder(int value);
	int border() { return m_border; }
	
protected:
	void drawContents(QPainter *p);
	void mousePressEvent(QMouseEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	
private:
	int m_border;
	QRect m_left,m_top,m_right,m_bottom;
};


class TabCellDialog : public KDialog 
{
	Q_OBJECT

public:
	TabCellDialog(QWidget *parent, TabularCell_OLD::Data *data,
	              const QString &headerlabel, const QStringList &alignlist);
	~TabCellDialog() {}
	
	TabularCell_OLD::Data data();
	QString header();
	
private Q_SLOTS:
	void slotFramebuttonClicked();
	void slotSeparatorClicked();
	void slotResetClicked();

private:
	QComboBox *m_coHeader;
	QCheckBox *m_cbBold, *m_cbItalic;
	QRadioButton *m_rbAlignleft, *m_rbAligncenter, *m_rbAlignright;
	KColorCombo *m_ccBgcolor, *m_ccTextcolor;
	KPushButton *m_pbFrame1,*m_pbFrame2,*m_pbFrame3,*m_pbFrame4;
	QCheckBox *m_cbPre, *m_cbPost, *m_cbAt, *m_cbSep;	
	TabCellFrame *m_cellframe;

	TabularCell_OLD::Data m_data;
	bool m_header;
	QString m_headerlabel;
	QStringList m_preamblelist;
	
	void initWidgets();
	void initWidgetData();
};

//////////////////////////////////////////////////////////////////////

class TabularItem :public Q3TableItem 
{
public:
	TabularItem(Q3Table* table);
	TabularItem(Q3Table* table, const TabularCell_OLD::Data &data);

	int alignment() const;
	bool isDefault();
	bool isMulticolumn();
	
	void paint(QPainter *p,const QColorGroup &cg,const QRect &cr,bool selected);
	TabularCell_OLD::Data m_data;
};

class TabularDialog;
class TabularTable : public Q3Table 
{
	Q_OBJECT
	
public:
	TabularTable(int numRows,int numCols,QWidget* parent, TabularDialog *tabdialog);

	void setText(int row,int col,const QString &text);	
	void setAlignment(int row,int col,int align);
	TabularCell_OLD::Data defaultAttributes();
	
	bool isMulticolumn(int row,int col);
		
	void paintCell( QPainter *p, int row, int col,
	                const QRect &cr, bool selected, const QColorGroup &cg );
	void updateCurrentCell();
	
	bool isRowEmpty(int row);
	bool isRowEmpty(int row,int col1, int col2);
	bool isColEmpty(int col);
	void clearHorizontalHeader(int col1,int col2);
	void clearVerticalHeader(int row1,int row2);

	bool isVLine(int row,int col, bool left);
	TabularCell_OLD::Count countCells(int x1,int y1,int x2,int y2);
	TabularCell_OLD::CountLines countHLines(int row, bool top);
	TabularCell_OLD::CountLines countVLines(int col, bool left);

protected:
	void endEdit(int row,int col,bool accept,bool replace);
	void activateNextCell();
	
private:
	enum { DataAlign, DataFont, DataBorder, DataBgcolor, DataTextcolor };
	enum { PopupNone, PopupEdit, PopupSet, PopupBreak, 
	       PopupLeft, PopupCenter, PopupRight, 
	       PopupText, PopupAttributes, PopupAll };
	 
	TabularItem *cellItem(int row,int col);
	bool isDefaultAttr(const TabularCell_OLD::Data &data);
	bool updateCell(int row,int col);
	void setAttributes(int row,int col,const TabularCell_OLD::Data &data);
	void clearAttributes(int row,int col);
	void cellParameterDialog(int x1,int y1,int x2,int y2, TabularCell_OLD::Data *data,
	                         const QString &headerlabel);
	bool equalParameter(int x1,int y1,int x2,int y2, int code);
	
	void mouseContextHorizontalHeader(int pos);
	void mouseContextVerticalHeader(int pos);
	void updateHeaderAlignment(int col1,int col2,QChar alignchar);
	
	bool getCurrentSelection(int &x1,int &y1,int &x2,int &y2);
	void clearSelectionCells(bool cleartext,bool clearattributes);
	void clearHeaderCells(bool cleartext,bool clearattributes);
	void clearCellrange(int x1,int y1,int x2,int y2,bool cleartext,bool clearattributes);
	void setCellrangeAlignment(int x1,int y1,int x2,int y2,int align);
	void setCellrangeAttributes(int x1,int y1,int x2,int y2,const TabularCell_OLD::Data &data);
	
	void setColspan(int row,int col1,int col2,int numcols,const QString &text);
	void getCellRange(int row,int col1, int col2, int &xl, int &xr);
	QString getCellRangeText(int row,int col1, int col2);

	QMenu *createPopupMenu(bool header);
	void insertPopupAlign(QMenu *popup,bool header);
	void insertPopupClear(QMenu *popup);
	int popupId(QMenu *popup, int id);
	
	void cellPopupAlign(int align);
	
	void setupContextHeaderPopup(bool horizontal, int section);
	void headerPopupAlign(QChar alignchar);
	
	bool m_horizontal;
	int m_section;
	int m_x1,m_y1,m_x2,m_y2;
	
	QMenu *m_headerpopup;
	QMenu *m_cellpopup;
	TabularDialog *m_tabdialog;
	
private Q_SLOTS:
	void slotContextMenuClicked(int row,int col,const QPoint &);	
	void slotCellPopupActivated(int id);
	void slotHeaderPopupActivated(int id);
	void cellPopupEdit();
	void cellPopupSetMulticolumn();
	void cellPopupBreakMulticolumn();
	void headerPopupEdit();
	
protected:
	bool eventFilter(QObject *o, QEvent *e); 

};


class TabularDialog : public Wizard  
{
	Q_OBJECT

public:
	TabularDialog(QWidget *parent, KConfig *config, KileDocument::LatexCommands *commands, bool tabularenv= true);
	~TabularDialog() {}
	QStringList columnAlignments();

public Q_SLOTS:
	void slotButtonClicked(int button);
	void slotRowValueChanged(int value);

private Q_SLOTS:
	void slotColValueChanged(int value);
	void slotEnvironmentChanged(const QString &env);
	
private:
	KileDocument::LatexCommands *m_latexCommands;
	
	TabularTable *m_table;
	QComboBox *m_coEnvironment, *m_coParameter;
	QSpinBox *m_spRows, *m_spCols;
	QCheckBox *m_cbWarning, *m_cbBullets, *m_cbStarred;
	QCheckBox *m_cbCenter, *m_cbBooktabs;
	
	int m_rows;
	int m_cols;
	
	void initEnvironments(bool tabularenv);
	bool isMathmodeEnvironment(const QString &env);
	
	QStringList m_alignlist;
	
	QStringList sortColorTable(QMap<QString,char> &colors);
	QString convertColor(int value);
	char defineColor(const QString &name, QMap<QString,char> &colors, char &colorcode); 	
	QString getEol(int row,bool top);
};

}

#endif
