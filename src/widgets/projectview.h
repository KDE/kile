/*****************************************************************************************
    begin                : Tue Aug 12 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                               2006 - 2010 by Michel Ludwig (michel.ludwig@kdemail.net)
 *****************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef PROJECTVIEW_H
#define PROJECTVIEW_H

#include <QDropEvent>
#include <QTreeWidget>

#include <KService>

#include "kileproject.h"

class QUrl;
class QMenu;
class KToggleAction;
class KileInfo;

namespace KileType {
enum ProjectView {Project = 0, ProjectItem, Bibliography, ProjectExtra, File, Folder};
}

namespace KileWidget {

class ProjectViewItem : public QObject, public QTreeWidgetItem
{
    Q_OBJECT

public:
    ProjectViewItem(QTreeWidget *parent, KileProjectItem *item, bool ar = false);
    ProjectViewItem(QTreeWidget *parent, QTreeWidgetItem *after, KileProjectItem *item, bool ar = false);
    ProjectViewItem(QTreeWidgetItem *parent, KileProjectItem *item, bool ar = false);

    //use this to create folders
    ProjectViewItem(QTreeWidgetItem *parent, const QString& name);

    //use this to create non-project files
    ProjectViewItem(QTreeWidget *parent, const QString& name);

    ProjectViewItem(QTreeWidget *parent, const KileProject *project);

    ~ProjectViewItem();

    KileProjectItem* projectItem();

    ProjectViewItem* parent();
    ProjectViewItem* firstChild();

    void setInfo(KileDocument::Info *docinfo);
    KileDocument::Info * getInfo();

    void setType(KileType::ProjectView type);
    KileType::ProjectView type() const;

    virtual bool operator<(const QTreeWidgetItem& other) const;

    void setURL(const QUrl &url);
    const QUrl &url();

    void setArchiveState(bool ar);

    void setFolder(int folder);
    int folder() const;

public Q_SLOTS:
    /**
     * @warning Does nothing if "url" is empty !
     **/
    void urlChanged(const QUrl &url);
    void nameChanged(const QString& name);
    void isrootChanged(bool isroot);

private Q_SLOTS:
    /**
     * Dummy slot, simply forwarding to urlChanged(const QUrl &url).
     **/
    void slotURLChanged(KileDocument::Info*, const QUrl &url);


private:
    QUrl m_url;
    KileType::ProjectView m_type;
    KileDocument::Info *m_docinfo;
    int m_folder;
    KileProjectItem *m_projectItem;
};

class ProjectView : public QTreeWidget
{
    Q_OBJECT

public:
    ProjectView(QWidget *parent, KileInfo *ki);

    void addTree(KileProjectItem *item, ProjectViewItem *projvi);

    ProjectViewItem* projectViewItemFor(const QUrl&);
    ProjectViewItem* itemFor(const QUrl&);
    ProjectViewItem* parentFor(const KileProjectItem *projitem, ProjectViewItem *projvi);

public Q_SLOTS:
    void slotClicked(QTreeWidgetItem* item = Q_NULLPTR);

    void slotFile(int id);
    void slotProjectItem(int id);
    void slotProject(int id);

    void slotRun(int id);

    void refreshProjectTree(const KileProject *);
    void add(const QUrl &url);
    void add(const KileProject *project);
    void remove(const QUrl &url);
    void remove(const KileProject *project);
    void removeItem(const KileProjectItem *, bool);
    ProjectViewItem* add(KileProjectItem *item, ProjectViewItem *projvi = Q_NULLPTR);

Q_SIGNALS:
    void fileSelected(const KileProjectItem *);
    void fileSelected(const QUrl&);
    void saveURL(const QUrl&);
    void closeURL(const QUrl&);
    void projectOptions(const QUrl&);
    void projectArchive(const QUrl&);
    void addFiles(const QUrl&);
    void openAllFiles(const QUrl&);
    void toggleArchive(KileProjectItem*);
    void closeProject(const QUrl&);
    void addToProject(const QUrl&);
    void removeFromProject(KileProjectItem*);
    void buildProjectTree(const QUrl&);

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);

private:

    KileInfo	*m_ki;
    uint		m_nProjects;

    KService::List m_offerList;

    void makeTheConnection(ProjectViewItem *projectViewItem, KileDocument::TextInfo *textInfo = Q_NULLPTR);
    ProjectViewItem* folder(const KileProjectItem *item, ProjectViewItem *);

};

}

#endif
