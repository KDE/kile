/********************************************************************************************
    begin                : Wed Aug 14 2002
    copyright            : (C) 2002 - 2003 by Pascal Brachet
                               2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                               2007, 2008 by Michel Ludwig (michel.ludwig@kdemail.net)

from Kate (C) 2001 by Matt Newell

 ********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FILEBROWSERWIDGET_H
#define FILEBROWSERWIDGET_H

#include <KFile>
#include <KDirOperator>
#include <KUrlNavigator>
#include <QUrl>
#include <KConfig>
#include <KConfigGroup>

#include "kileextensions.h"

class KFileItem;
class KToolBar;

namespace KileWidget {

class FileBrowserWidget : public QWidget
{
    Q_OBJECT

public:
    FileBrowserWidget(KileDocument::Extensions *extensions, QWidget *parent);
    ~FileBrowserWidget();

    QUrl currentUrl() const;

public Q_SLOTS:
    void setDir(const QUrl &url);
    void writeConfig();

private Q_SLOTS:
    void toggleShowLaTeXFilesOnly(bool filter);
    void dirUrlEntered(const QUrl &u);
    void emitFileSelectedSignal();

protected:
    void setupToolbar();
    void readConfig();

Q_SIGNALS:
    void fileSelected(const KFileItem& fileItem);

private:
    KToolBar 	*m_toolbar;
    KUrlNavigator	*m_urlNavigator;
    KDirOperator	*m_dirOperator;
    KConfig		*m_config;
    KConfigGroup 	m_configGroup;
    KileDocument::Extensions *m_extensions;
};

}

#endif
