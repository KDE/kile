/***************************************************************************************
    begin                : Fri Aug 1 2003
    copyright            : (C) 2002 - 2003 by Pascal Brachet
                               2003 Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                               2006 - 2009 Thomas Braun
 ***************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SYMBOLVIEW_H
#define SYMBOLVIEW_H

#include <QListWidget>

#include <KColorScheme>

#include "../symbolviewclasses.h"

class QMouseEvent;

class KileInfo;

namespace KileWidget {

class SymbolView : public QListWidget
{
    Q_OBJECT

public:
    explicit SymbolView(KileInfo *kileInfo, QWidget *parent = 0, int type = -1, const char *name = Q_NULLPTR);
    ~SymbolView();
    enum { MFUS = 0, Relation, Operator, Arrow, MiscMath, MiscText, Delimiters, Greek, Special, Cyrillic, User };
    void writeConfig();

private:
    void fillWidget(const QString &prefix);
    void extractPackageString(const QString &string, QList<Package> &pkgs);
    void extract(const QString& key, Command &cmd);
    void extract(const QString& key, int& refCnt);
    void initPage(int page);
    QString getToolTip(const QString &key);

protected:
    KileInfo *m_ki;
    KStatefulBrush m_brush;


    virtual void mousePressEvent(QMouseEvent *event);

Q_SIGNALS:
    void insertText(const QString& text, const QList<Package> &pkgs);
    void addToList(const QListWidgetItem *item);

public Q_SLOTS:
    void slotAddToList(const QListWidgetItem *item);
};

}

#endif
