/***************************************************************************************
    begin                : Sat Apr 26 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (wijnhout@science.uva.nl)
                               2007 by Michel Ludwig (michel.ludwig@kdemail.net)
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


/**
  *@author Jeroen Wijnhout
  */

#include <kstandarddirs.h>
#include <kiconview.h>
#include <klocale.h>
#include <kprocess.h>
#include <kurl.h>

#include <qobject.h>
#include <qvaluelist.h>

#include "kileconstants.h"

#define DEFAULT_EMPTY_ICON "type_Empty"

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

typedef QValueList<Info> TemplateList;
typedef QValueListIterator<Info> TemplateListIterator;
typedef QValueListConstIterator<Info> TemplateListConstIterator;

class Manager : public QObject {
	Q_OBJECT
	
	public:
		Manager(KileInfo *info, QObject* parent = NULL, const char* name = NULL);
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

		//add a template in $HOME/kile/templates/
		bool add(const KURL& templateSourceURL, const QString& name, const KURL& icon);
		
		//remove a template from $HOME/kile/templates/
		bool remove(KileTemplate::Info ti);

		//replaces a template
		bool replace(const KileTemplate::Info& toBeReplaced, const KURL& newTemplateSourceURL, const QString& newName, const KURL& newIcon);

	protected:
		KileInfo* m_kileInfo;

	private:
		bool copyAppData(const KURL& src, const QString& subdir, const QString& fileName);
		bool removeAppData(const QString &file);

		/**
		 * Adds a new template. This method differs from the other add method in that it does not try to determine
		 * the type of the template from the passed source URL.
		 **/
		bool add(const KURL& templateSourceURL, KileDocument::Type type, const QString& name, const KURL& icon);


	private:
		TemplateList m_TemplateList;
};

}

const QString DEFAULT_EMPTY_CAPTION = i18n("Empty Document");
const QString DEFAULT_EMPTY_LATEX_CAPTION = i18n("Empty LaTeX Document");
const QString DEFAULT_EMPTY_BIBTEX_CAPTION = i18n("Empty BibTeX Document");

class TemplateItem : public QIconViewItem
{
public:
	TemplateItem( QIconView * parent, const KileTemplate::Info & info);
	~TemplateItem() {}

	int compare( QIconViewItem *i ) const;
	
	QString name() { return m_info.name; }
	QString path() { return m_info.path; }
	QString icon() { return m_info.icon; }
	KileDocument::Type type() { return m_info.type; }

private:
	KileTemplate::Info m_info;
};

class TemplateIconView : public KIconView {
	Q_OBJECT
	
	public:
		TemplateIconView(QWidget *parent=0, const char *name=0, WFlags f=0);
		virtual ~TemplateIconView();

		void setTemplateManager(KileTemplate::Manager *templateManager);

		void fillWithTemplates(KileDocument::Type type);

	signals:
		void classFileSearchFinished();

	protected:
		KileTemplate::Manager *m_templateManager;
		QString m_output;
		QString m_selicon;
		KProcess *m_proc;

		void addTemplateIcons(KileDocument::Type type);
		void searchLaTeXClassFiles();

	protected slots:
		void slotProcessOutput(KProcess*,char* buf,int len);
		void slotProcessExited (KProcess *proc);
};

#endif
