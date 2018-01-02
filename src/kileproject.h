/***************************************************************************************
    begin                : Fri Aug 1 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                           (C) 2009-2016 by Michel Ludwig (michel.ludwig@kdemail.net)
 ***************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KILEPROJECT_H
#define KILEPROJECT_H

#include <QDir>
#include <QFileInfo>
#include <QList>
#include <QObject>
#include <QRegExp>

#include <KConfig>
#include <QUrl>
#include <KTextEditor/View>

#include "kiledebug.h"
#include "kileversion.h"
#include "livepreview_utils.h"
#include "outputinfo.h"

class QString;
class QStringList;
namespace KileDocument {
class Info;
class TextInfo;
class Extensions;
}

/**
 * KileProjectItem
 **/
class KileProject;
class KileProjectItem : public QObject
{
    Q_OBJECT

public:
    explicit KileProjectItem(KileProject *project = 0, const QUrl &url = QUrl(), int type = Source);
    ~KileProjectItem() {
        KILE_DEBUG_MAIN << "DELETING PROJITEM " << m_path << endl;
    }

    bool operator==(const KileProjectItem& item) {
        return m_url  == item.url();
    }

    enum Type { ProjectFile = 0, Source, Package, Image, Bibliography, Other /* should be the last item*/ };

    int type() const {
        return m_type;
    }
    void setType(int type) {
        m_type = type;
    }

    bool archive() const {
        return m_archive;
    }
    void setArchive(bool ar) {
        m_archive = ar;
    }

    void setInfo(KileDocument::TextInfo * docinfo);
    KileDocument::TextInfo* getInfo() const {
        return m_docinfo;
    }

    KileProject* project() const {
        return m_project;
    }

    /**
     * @returns absolute URL of this item
     **/
    const QUrl &url() const {
        return m_url;
    }

    /**
     * @returns path of this item relative to the project file
     **/
    const QString& path() const {
        return m_path;
    }

    bool isOpen() const {
        return m_bOpen;
    }
    void setOpenState(bool state) {
        m_bOpen = state;
    }

    const QString& encoding() const {
        return m_encoding;
    }
    void setEncoding(const QString& encoding) {
        m_encoding = encoding;
    }

    const QString& highlight() {
        return m_highlight;
    }
    void setHighlight(const QString& highlight) {
        m_highlight = highlight;
    }

    const QString& mode() {
        return m_mode;
    }
    void setMode(const QString& mode) {
        m_mode = mode;
    }

    int order() const {
        return m_order;
    }
    void setOrder(int i);

    //project tree functions
    void setParent(KileProjectItem * item);

    void load();
    void save();

    void loadDocumentAndViewSettings();
    void saveDocumentAndViewSettings();

protected:
    void setChild(KileProjectItem *item) {
        m_child = item;
    }
    void setSibling(KileProjectItem *item) {
        m_sibling = item;
    }

    void loadViewSettings(KTextEditor::View *view, int viewIndex);
    void saveViewSettings(KTextEditor::View *view, int viewIndex);

    void loadDocumentSettings(KTextEditor::Document *document);
    void saveDocumentSettings(KTextEditor::Document *document);

public:
    KileProjectItem* parent() const {
        return m_parent;
    }
    KileProjectItem* firstChild() const {
        return m_child;
    }
    KileProjectItem* sibling() const {
        return m_sibling;
    }

    void allChildren(QList<KileProjectItem*>* list) const;

    void print(int level);

public Q_SLOTS:
    /**
     * @warning Does nothing if "url" is empty !
     **/
    void changeURL(const QUrl &url);
    void changePath(const QString& path) {
        m_path = path;
    }

private Q_SLOTS:
    void slotChangeURL(KileDocument::Info* info, const QUrl &url);

Q_SIGNALS:
    void urlChanged(KileProjectItem*);

private:
    KileProject		*m_project;
    QUrl			m_url;
    QString			m_path;
    QString			m_encoding;
    QString			m_mode;
    QString			m_highlight;
    bool			m_bOpen, m_archive;
    int			m_type;
    KileDocument::TextInfo	*m_docinfo;
    KileProjectItem		*m_parent, *m_child, *m_sibling;
    int			m_order;
};

/**
 * KileProject
 **/
class KileProject : public QObject, public KileTool::LivePreviewUserStatusHandler, public LaTeXOutputHandler
{
    Q_OBJECT
    friend class KileProjectItem;

public:
    KileProject(const QString& name, const QUrl &url, KileDocument::Extensions *extensions);
    KileProject(const QUrl &url, KileDocument::Extensions *extensions);

    ~KileProject();

    void setName(const QString & name) {
        m_name = name;
        emit (nameChanged(name));
    }
    const QString& name() const {
        return m_name;
    }

    void setMasterDocument(const QString & master);
    const QString& masterDocument() const {
        return m_masterDocument;
    }

    void setExtensions(KileProjectItem::Type type, const QString & ext);
    const QString & extensions(KileProjectItem::Type type) {
        return m_extensions[type-1];
    }

    void setDefaultGraphicExt(const QString & ext);
    const QString & defaultGraphicExt();

    void setQuickBuildConfig(const QString & cfg) {
        m_quickBuildConfig = cfg;
    }
    const QString & quickBuildConfig() {
        return m_quickBuildConfig;
    }

    void setLastDocument(const QUrl &url);
    const QUrl &lastDocument() const {
        return m_lastDocument;
    }

    void setMakeIndexOptions(const QString & opt) {
        m_makeIndexOptions = opt;
    }
    const QString & makeIndexOptions() {
        return m_makeIndexOptions;
    }
    void readMakeIndexOptions();
    void setUseMakeIndexOptions(bool use) {
        m_useMakeIndexOptions = use;
    }
    void writeUseMakeIndexOptions();
    bool useMakeIndexOptions() {
        return m_useMakeIndexOptions;
    }

    QUrl url() const {
        return m_projecturl;
    }
    void setURL(const QUrl &url ) {
        m_projecturl = url;
    }
    QUrl baseURL() const {
        return m_baseurl;
    }

    KileProjectItem* item(const QUrl &);
    KileProjectItem* item(const KileDocument::Info *info);
    QList<KileProjectItem*> items() {
        return m_projectItems;
    }

    KConfig *config() {
        return m_config;
    }
    KConfig *guiConfig() {
        return m_guiConfig;
    }

    bool contains(const QUrl&);
    bool contains(const KileDocument::Info *info);
    KileProjectItem *rootItem(KileProjectItem *) const;
    const QList<KileProjectItem*>& rootItems() const {
        return m_rootItems;
    }
    bool isInvalid() {
        return m_invalid;
    }
    QString archiveFileList() const;

    bool appearsToBeValidProjectFile();

    inline bool isOfCurrentVersion()
    {
        return (getProjectFileVersion() == KILE_PROJECTFILE_VERSION);
    }

    int getProjectFileVersion();

    bool migrateProjectFileToCurrentVersion();

    static inline QString getPathForGUISettingsProjectFile(const QString& projectFilePath)
    {
        QFileInfo fi(projectFilePath);

        return getPathForPrivateKileDirectory(fi) + QStringLiteral("/") + fi.fileName() + QStringLiteral(".gui");
    }

    static inline QString getPathForGUISettingsProjectFile(const QUrl& projectUrl)
    {
        return getPathForGUISettingsProjectFile(projectUrl.toLocalFile());
    }

    static inline QString getPathForPrivateKileDirectory(const QUrl& projectUrl)
    {
        return getPathForPrivateKileDirectory(projectUrl.toLocalFile());
    }

    static inline QString getPathForPrivateKileDirectory(const QFileInfo& projectFilePath)
    {
        return projectFilePath.dir().absoluteFilePath(".kile");
    }

    static inline QString getPathForPrivateKileDirectory(const QString& projectFilePath)
    {
        return getPathForPrivateKileDirectory(QFileInfo(projectFilePath));
    }

    static inline bool ensurePrivateKileDirectoryExists(const QUrl& projectUrl)
    {
        return QFileInfo(projectUrl.toLocalFile()).dir().mkpath(".kile");
    }

Q_SIGNALS:
    void nameChanged(const QString &);
    void masterDocumentChanged(const QString &);
    void projectTreeChanged(const KileProject *);
    void projectItemAdded(KileProject *project, KileProjectItem *item);
    void projectItemRemoved(KileProject *project, KileProjectItem *item);
    void aboutToBeDestroyed(KileProject*);

public Q_SLOTS:
    bool load();
    bool save();

    void add(KileProjectItem*);
    void remove(KileProjectItem*);

    void itemRenamed(KileProjectItem*);

    void buildProjectTree(); // moved to slots by tbraun

    //debugging
    void dump();

Q_SIGNALS:
    void loadFile(const QUrl &url, const QString & encoding);

private:
    bool migrateProjectFileToVersion3();

    void init(const QUrl &url);
    QString	findRelativePath(const QUrl&);
    QString	findRelativePath(const QString&);

    void setType(KileProjectItem *item);
    QString addBaseURL(const QString &path);
    QString removeBaseURL(const QString &path);
    void writeConfigEntry(const QString &key,const QString &standardExt,KileProjectItem::Type type);

    enum ConfigScope {
        ProjectFile,
        GUIFile
    };

    KConfigGroup configGroupForItem(KileProjectItem *item, ConfigScope scope) const;
    KConfigGroup configGroupForItemDocumentSettings(KileProjectItem *item) const;
    KConfigGroup configGroupForItemViewSettings(KileProjectItem *item, int viewIndex) const;

    void removeConfigGroupsForItem(KileProjectItem *item);

private:

    QString		m_name, m_quickBuildConfig, m_defGraphicExt;
    QUrl		m_projecturl, m_baseurl, m_lastDocument;
    bool		m_invalid;
    QList<KileProjectItem*> m_rootItems;
    QList<KileProjectItem*>	m_projectItems;

    QString		m_extensions[4];
    QRegExp		m_reExtensions[4];

    QString				m_masterDocument, m_makeIndexOptions;
    bool				m_useMakeIndexOptions;

    KConfig	*m_config;     // stores project structure
    KConfig	*m_guiConfig;  // stores project GUI settings: last document, items view settings, etc.
    KileDocument::Extensions *m_extmanager;
};

#endif
