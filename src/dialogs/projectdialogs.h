/***************************************************************************
  Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
            (C) 2015 by Andreas Cord-Landwehr (cordlandwehr@kde.org)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PROJECTDIALOGS_H
#define PROJECTDIALOGS_H

#include <QDialog>
#include <QCheckBox>
#include <QLineEdit>
#include <KUrlRequester>
#include <KLineEdit>

#include "kileextensions.h"
#include "kileproject.h"
#include "templates.h"

class QComboBox;
class QGridLayout;
class QGroupBox;
class QLabel;

class KileProject;
class TemplateItem;
class TemplateIconView;

namespace KileDocument {
class Extensions;
}
namespace KileTemplate {
class Manager;
}

class KileProjectDialogBase : public QDialog
{
    Q_OBJECT

public:
    KileProjectDialogBase(const QString &caption, KileDocument::Extensions *extensions, QWidget *parent = Q_NULLPTR, const char * name = Q_NULLPTR);
    virtual ~KileProjectDialogBase();

    void setProject(KileProject *project, bool override);
    virtual KileProject * project();

    void setProjectTitle(const QString &title)
    {
        m_title->setText(title);
    }
    QString projectTitle() const
    {
        return m_title->text();
    }

    void setExtensions(KileProjectItem::Type type, const QString &ext);
    QString extensions(KileProjectItem::Type type) const
    {
        return m_val_extensions[type-1];
    }

protected Q_SLOTS:
    virtual void fillProjectDefaults();
    virtual void onExtensionsIndexChanged(int index);
    virtual void onExtensionsTextEdited(const QString &text);

protected:
    KileDocument::Extensions *m_extmanager;
    KileProject *m_project;

    QGroupBox *m_projectGroup;
    QGroupBox *m_extensionGroup;
    KUrlRequester *m_projectFolder;

    QLineEdit *m_title, *m_userFileExtensions;
    QLabel *m_defaultLatexFileExtensions;

    QComboBox *m_defaultLatexFileExtensionsCombo;
    QComboBox *m_defaultGraphicsExtensionCombo;

    QString m_val_extensions[KileProjectItem::Other - 1];
    QString m_val_standardExtensions[KileProjectItem::Other - 1];

    bool acceptUserExtensions();
};

class KileNewProjectDialog : public KileProjectDialogBase
{
    Q_OBJECT

public:
    KileNewProjectDialog(KileTemplate::Manager *templateManager, KileDocument::Extensions *extensions, QWidget* parent = Q_NULLPTR, const char* name = Q_NULLPTR);
    ~KileNewProjectDialog();

    KileProject *project() Q_DECL_OVERRIDE;

    QString cleanProjectFile();
    QString folder() const {
        return m_projectFolder->lineEdit()->text();
    }

    TemplateItem* getSelection() const;
    QString file() const {
        return m_file->text();
    }
    bool createNewFile() const {
        return m_createNewFileCheckbox->isChecked();
    }

private Q_SLOTS:
    void clickedCreateNewFileCb();
    void fillProjectDefaults() Q_DECL_OVERRIDE;
    void handleOKButtonClicked();

private:
    bool testDirectoryIsUsable(const QString& path);
    bool testDirectoryIsUsable(const QDir& dir);

    KileTemplate::Manager *m_templateManager;
    QLineEdit *m_file, *m_name;
    TemplateIconView *m_templateIconView;
    QCheckBox *m_createNewFileCheckbox;
    QLabel *m_filenameLabel;
    QUrl m_projectFileWithPath;
};

class KileProjectOptionsDialog : public KileProjectDialogBase
{
    Q_OBJECT

public:
    KileProjectOptionsDialog(KileProject *project, KileDocument::Extensions *extensions, QWidget *parent = Q_NULLPTR, const char * name = Q_NULLPTR);
    ~KileProjectOptionsDialog();

private Q_SLOTS:
    void toggleMakeIndex(bool);
    void onAccepted();

private:
    KComboBox *m_selectMasterDocumentCombo, *m_QuickBuildCheckbox;
    QLineEdit *m_leMakeIndex;
    QCheckBox *m_ckMakeIndex;
    QString m_toolDefaultString;
};

#endif
