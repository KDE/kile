/****************************************************************************************
    begin                : Sun Apr 27 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (wijnhout@science.uva.nl)
                               2007 by Michel Ludwig (michel.ludwig@kdemail.net)
 ****************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MANAGETEMPLATESDIALOG_H
#define MANAGETEMPLATESDIALOG_H

#include <QDialog>
#include <QUrl>

class QCheckBox;
class QLineEdit;
class QTreeWidget;
class QTreeWidgetItem;

#include "kileconstants.h"

namespace KileTemplate {
class Manager;
struct Info;
}

/**
  *@author Jeroen Wijnhout
  */
class ManageTemplatesDialog : public QDialog  {
    Q_OBJECT
public:
    ManageTemplatesDialog(KileTemplate::Manager *templateManager, const QUrl &sourceURL, const QString &caption, QWidget *parent = 0, const char *name = 0);
    ManageTemplatesDialog(KileTemplate::Manager *templateManager, const QString &caption, QWidget *parent = 0, const char *name = 0);
    virtual ~ManageTemplatesDialog();

public Q_SLOTS:
    void slotSelectedTemplate(QTreeWidgetItem *item);
    void slotSelectIcon();
    void addTemplate();
    bool removeTemplate();

Q_SIGNALS:
    void aboutToClose();

protected Q_SLOTS:
    void updateTemplateListView(bool showAllTypes);
    void clearSelection();

protected:
    KileTemplate::Manager* m_templateManager;
    QLineEdit *m_nameEdit, *m_iconEdit;
    QTreeWidget *m_templateList;
    KileDocument::Type m_templateType;
    QCheckBox *m_showAllTypesCheckBox;
    QUrl m_sourceURL;

    /**
    	* Fills the template list view with template entries.
    	*
    	* @param type The type of the templates that should be displayed. You can pass "KileDocument::Undefined" to
    	*             display every template.
    	**/
    void populateTemplateListView(KileDocument::Type type);
};

#endif
