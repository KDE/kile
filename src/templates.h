/***************************************************************************************
    begin                : Sat Apr 26 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (wijnhout@science.uva.nl)
                               2007, 2008 by Michel Ludwig (michel.ludwig@kdemail.net)
 ***************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TEMPLATES_H
#define TEMPLATES_H

#include <QListWidget>
#include <QObject>

#include <QProcess>

#include <KLocalizedString>
#include <QUrl>

#include "kileconstants.h"

#define DEFAULT_EMPTY_ICON "type_Empty"

class KProcess;

class KileInfo;

namespace KileTemplate {

struct Info {
public:
    Info();

    QString name;
    QString path;
    QString icon;
    KileDocument::Type type;

    bool operator==(const Info ti) const;
};

typedef QList<Info> TemplateList;
typedef QList<Info>::iterator TemplateListIterator;
typedef QList<Info>::const_iterator TemplateListConstIterator;

class Manager : public QObject {
    Q_OBJECT

public:
    explicit Manager(KileInfo *info, QObject* parent = Q_NULLPTR, const char* name = NULL);
    virtual ~Manager();

    void scanForTemplates();

    /**
    * Get all the templates.
    **/
    TemplateList getAllTemplates() const;

    /**
    * Get all the templates of a certain type.
    *
    * @param type The type of the templates that should be returned. You can pass "KileDocument::Undefined" to
    *             retrieve every template.
    **/
    TemplateList getTemplates(KileDocument::Type type) const;

    /**
     * Checks whether a template with a given name and type exists.
     *
     * @return true iff a template with the given name and type could be found
     **/
    bool searchForTemplate(const QString& name, KileDocument::Type& type) const;

    // add a template in $HOME/kile/templates/
    bool add(const QUrl &templateSourceURL, const QString &name, const QUrl &icon);

    // remove a template from $HOME/kile/templates/
    bool remove(KileTemplate::Info ti);

    // replaces a template
    bool replace(const KileTemplate::Info& toBeReplaced, const QUrl &newTemplateSourceURL, const QString& newName, const QUrl& newIcon);

    // these have to be methods as we cannot use i18n calls in global objects
    static QString defaultEmptyTemplateCaption();
    static QString defaultEmptyLaTeXTemplateCaption();
    static QString defaultEmptyBibTeXTemplateCaption();

protected:
    KileInfo* m_kileInfo;

private:
    bool copyAppData(const QUrl &src, const QString& subdir, const QString& fileName);
    bool removeAppData(const QString &file);

    /**
     * Adds a new template. This method differs from the other add method in that it does not try to determine
     * the type of the template from the passed source URL.
     **/
    bool add(const QUrl &templateSourceURL, KileDocument::Type type, const QString& name, const QUrl& icon);


private:
    TemplateList m_TemplateList;
    QString m_TempFilePath;
};

}


class TemplateItem : public QListWidgetItem
{
public:
    TemplateItem( QListWidget * parent, const KileTemplate::Info & info);
    ~TemplateItem() {}

    bool operator<(const QListWidgetItem &other) const override;

    QString name() {
        return m_info.name;
    }
    QString path() {
        return m_info.path;
    }
    QString icon() {
        return m_info.icon;
    }
    KileDocument::Type type() {
        return m_info.type;
    }

private:
    KileTemplate::Info m_info;
};

class TemplateIconView : public QListWidget {
    Q_OBJECT

public:
    explicit TemplateIconView(QWidget *parent = Q_NULLPTR);
    virtual ~TemplateIconView();

    void setTemplateManager(KileTemplate::Manager *templateManager);

    void fillWithTemplates(KileDocument::Type type);

Q_SIGNALS:
    void classFileSearchFinished();

protected:
    KileTemplate::Manager *m_templateManager;
    QString m_output;
    KProcess *m_proc;

    void addTemplateIcons(KileDocument::Type type);
    void searchLaTeXClassFiles();

protected Q_SLOTS:
    void slotProcessError();
    void slotProcessOutput();
    void slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus);
};

#endif
