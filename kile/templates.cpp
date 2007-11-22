/*******************************************************************************************
    begin                : Sat Apr 26 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (wijnhout@science.uva.nl)
                               2005 by Holger Danielsson (holger.danielsson@t-online.de)
                               2007 by Michel Ludwig (michel.ludwig@kdemail.net)
 *******************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "templates.h"

#include <kapplication.h>
#include "kiledebug.h"
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>

#include <qdir.h>
#include <qfileinfo.h>
#include <qstringlist.h>
#include <qregexp.h>
//Added by qt3to4:
#include <QPixmap>

#include "kileinfo.h"

// 2005-08-04: dani
//  - added script support to search existing class files 
//    (classes: Koma, Beamer, Prosper, HA-prosper)
//  - sort items ('Empty Document' will always be the first entry)

// 2006-30-04: tbraun
//  - drag and drop makes no sense here
//  - use the Select mode

namespace KileTemplate {

////////////////////// Info //////////////////////

Info::Info() : type(KileDocument::Undefined)
{
}

bool Info::operator==(const Info ti) const
{
	return name==ti.name;
}

////////////////////// Manager //////////////////////

Manager::Manager(KileInfo* kileInfo, QObject* parent, const char* name) : QObject(parent, name), m_kileInfo(kileInfo)
{
}

Manager::~Manager() {
}

bool Manager::copyAppData(const KUrl& src, const QString& subdir, const QString& fileName) {
	QString dir;
	//let saveLocation find and create the appropriate place to
	//store the templates (usually $HOME/.kde/share/apps/kile/templates)
	dir = KGlobal::dirs()->saveLocation("appdata", subdir, true);
	KUrl targetURL = KUrl::fromPathOrUrl(dir);
	targetURL.addPath(fileName);

	//if a directory is found
	if (!dir.isNull()) {
		return KIO::NetAccess::copy(src, targetURL, kapp->mainWidget());
	}
	else {
		KMessageBox::error(0, i18n("Could not find a folder to save %1 to.\nCheck whether you have a .kde folder with write permissions in your home folder.").arg(fileName));
		return false;
	}
}

bool Manager::removeAppData(const QString &file) {
	QFileInfo fileInfo(file);
	if(fileInfo.exists()) {
		return KIO::NetAccess::del(KUrl::fromPathOrUrl(file), kapp->mainWidget());
	}
	return true;
}

bool Manager::searchForTemplate(const QString& name, KileDocument::Type& type) const {
	for (KileTemplate::TemplateListConstIterator i = m_TemplateList.constBegin(); i != m_TemplateList.constEnd(); ++i)
	{
		KileTemplate::Info info = *i;
		if(info.name == name && info.type == type) {
			return true;
		}
	}
	return false;
}

bool Manager::add(const KUrl& templateSourceURL, const QString& name, const KUrl& icon) {
	KileDocument::Extensions *extensions = m_kileInfo->extensions();
	KileDocument::Type type = extensions->determineDocumentType(templateSourceURL);
	return add(templateSourceURL, type, name, icon);
}

bool Manager::add(const KUrl& templateSourceURL, KileDocument::Type type, const QString& name, const KUrl& icon) {
	KileDocument::Extensions *extensions = m_kileInfo->extensions();
	QString extension = extensions->defaultExtensionForDocumentType(type);

	return copyAppData(templateSourceURL, "templates", "template_" + name + extension) && copyAppData(icon, "pics", "type_" + name + extension + ".kileicon");
}

bool Manager::remove(Info ti) {
	return removeAppData(ti.path) && removeAppData(ti.icon);
}

bool Manager::replace(const KileTemplate::Info& toBeReplaced, const KUrl& newTemplateSourceURL, const QString& newName, const KUrl& newIcon) {
	KileDocument::Type type = m_kileInfo->extensions()->determineDocumentType(newTemplateSourceURL);

	//start by copying the files that belong to the new template to a safe place
	QString templateTempFile, iconTempFile;

	if(!KIO::NetAccess::download(newTemplateSourceURL, templateTempFile, kapp->mainWidget())) {
		return false;
	}
	if(!KIO::NetAccess::download(newIcon, iconTempFile, kapp->mainWidget())) {
		KIO::NetAccess::removeTempFile(templateTempFile);
		return false;
	}

	//now delete the template that should be replaced
	if(!remove(toBeReplaced)) {
		KIO::NetAccess::removeTempFile(templateTempFile);
		KIO::NetAccess::removeTempFile(iconTempFile);
	}

	//finally, create the new template
	if(!add(KUrl::fromPathOrUrl(templateTempFile), type, newName, KUrl::fromPathOrUrl(iconTempFile))) {
		KIO::NetAccess::removeTempFile(templateTempFile);
		KIO::NetAccess::removeTempFile(iconTempFile);
		return false;
	}

	KIO::NetAccess::removeTempFile(templateTempFile);
	KIO::NetAccess::removeTempFile(iconTempFile);

	return true;
}

void Manager::scanForTemplates() {
	KILE_DEBUG() << "===scanForTemplates()===================" << endl;
	QStringList dirs = KGlobal::dirs()->findDirs("appdata", "templates");
	QDir templates;
	KileTemplate::Info ti;
	KileDocument::Extensions *extensions = m_kileInfo->extensions();

	m_TemplateList.clear();
	for ( Q3ValueListIterator<QString> i = dirs.begin(); i != dirs.end(); ++i)
	{
		templates = QDir(*i, "template_*");
		for ( uint j = 0; j< templates.count(); ++j)
		{
			ti.path = templates.path() + '/' + templates[j];
			QFileInfo fileInfo(ti.path);
			ti.name = fileInfo.baseName(true).mid(9); //remove "template_", do it this way to avoid problems with user input!
			ti.type = extensions->determineDocumentType(KUrl::fromPathOrUrl(ti.path));
			ti.icon = KGlobal::dirs()->findResource("appdata","pics/type_" + ti.name + extensions->defaultExtensionForDocumentType(ti.type) + ".kileicon");
			if (m_TemplateList.contains(ti))
			{
				KILE_DEBUG() << "\tignoring: " << ti.path << endl;
			}
			else
			{
				m_TemplateList.append(ti);
				KILE_DEBUG() << "\tadding: " << ti.name << " " << ti.path << endl;
			}
		}
	}
}

TemplateList Manager::getAllTemplates() const {
	return m_TemplateList;
}

TemplateList Manager::getTemplates(KileDocument::Type type) const {
	if(type == KileDocument::Undefined) 
	{
		return getAllTemplates();
	}

	TemplateList toReturn;
	for (KileTemplate::TemplateListConstIterator i = m_TemplateList.constBegin(); i != m_TemplateList.constEnd(); ++i)
	{
		KileTemplate::Info info = *i;
		if(info.type == type) {
			toReturn.push_back(info);
		}
	}
	return toReturn;
}

}
////////////////////// TemplateItem //////////////////////

// new compare function to make the "Empty (...) Document" items appear at the beginning

TemplateItem::TemplateItem(Q3IconView * parent, const KileTemplate::Info& info) : Q3IconViewItem(parent,info.name, QPixmap(info.icon))
{
	setDragEnabled(false);
	m_info = info;
}

int TemplateItem::compare( Q3IconViewItem *i ) const
{
	if ( key() == DEFAULT_EMPTY_CAPTION ) {
		return -1;
	}
	else if ( i->key() == DEFAULT_EMPTY_CAPTION ) {
		return 1;
	}
	else {
		return key().compare( i->key() );
	}
}

////////////////////// TemplateIconView //////////////////////

TemplateIconView::TemplateIconView(QWidget *parent, const char *name, Qt::WFlags f) : K3IconView(parent, name, f), m_templateManager(NULL), m_proc(NULL) {
	setItemsMovable(false);
	setMode(K3IconView::Select);
	setResizeMode(Q3IconView::Adjust);
	setSelectionMode(Q3IconView::Single);
	setResizePolicy(Q3ScrollView::Default);
	setArrangement(Q3IconView::TopToBottom);
	setMinimumHeight(100);
}

TemplateIconView::~TemplateIconView() {
}

void TemplateIconView::setTemplateManager(KileTemplate::Manager *templateManager) {
	m_templateManager = templateManager;
}

void TemplateIconView::fillWithTemplates(KileDocument::Type type) {
	if(!m_templateManager) {
		return;
	}

	clear();

	if(type == KileDocument::LaTeX) {
		searchLaTeXClassFiles();
	}
	else {
		addTemplateIcons(type);
	}
}

void TemplateIconView::searchLaTeXClassFiles()
{
	if(!m_templateManager) return;

	QString command = "kpsewhich -format=tex scrartcl.cls beamer.cls prosper.cls HA-prosper.sty";

	delete m_proc;

	m_proc = new K3Process(this);
	m_proc->clearArguments();
	m_proc->setUseShell(true);
	(*m_proc) << QStringList::split(' ', command);
	m_output = QString::null;

	connect(m_proc, SIGNAL(receivedStdout(K3Process*,char*,int)),
	        this,   SLOT(slotProcessOutput(K3Process*,char*,int)) );
	connect(m_proc, SIGNAL(receivedStderr(K3Process*,char*,int)),
	        this,   SLOT(slotProcessOutput(K3Process*,char*,int)) );
	connect(m_proc, SIGNAL(processExited(K3Process*)),
	        this,   SLOT(slotProcessExited(K3Process*)) );

	KILE_DEBUG() << "=== NewFileWidget::searchClassFiles() ====================" << endl;
	KILE_DEBUG() << "\texecute: " << command << endl;
	if ( ! m_proc->start(K3Process::NotifyOnExit, K3Process::AllOutput) ) 
	{
		KILE_DEBUG() << "\tstart of shell process failed" << endl;
		addTemplateIcons(KileDocument::LaTeX);
	}
}

void TemplateIconView::slotProcessOutput(K3Process*, char* buf, int len)
{
	m_output += QString::fromLocal8Bit(buf,len);
}

void TemplateIconView::slotProcessExited(K3Process *proc)
{
	if ( ! proc->normalExit() ) 
		m_output = QString::null;

	addTemplateIcons(KileDocument::LaTeX);
	emit classFileSearchFinished();
}

void TemplateIconView::addTemplateIcons(KileDocument::Type type)
{
	if(!m_templateManager) return;

	QString emptyIcon = KGlobal::dirs()->findResource("appdata", "pics/"+ QString(DEFAULT_EMPTY_ICON) + ".png" );

	KileTemplate::Info emptyDocumentInfo;
	emptyDocumentInfo.name = DEFAULT_EMPTY_CAPTION;
	emptyDocumentInfo.icon = emptyIcon;
	emptyDocumentInfo.type = type;
	TemplateItem *emp = new TemplateItem(this, emptyDocumentInfo);
	setSelected(emp, true);

	if(type == KileDocument::LaTeX) {
		// disable non standard templates
		QMap<QString,bool> map;
		map["Scrartcl"] = false;
		map["Scrbook"]  = false;
		map["Scrreprt"] = false;
		map["Scrlttr2"] = false;
		map["Beamer"]   = false;
		map["Prosper"]  = false;
		map["HA-prosper"] = false;
		
		// split search results and look, which class files are present
		QStringList list = QStringList::split("\n",m_output);
		for ( QStringList::Iterator it=list.begin(); it!=list.end(); ++it ) 
		{
			QString filename = QFileInfo(*it).fileName();
			if ( filename=="scrartcl.cls" )
			{
				map["Scrartcl"] = true;
				map["Scrbook"]  = true;
				map["Scrreprt"] = true;
				map["Scrlttr2"] = true;
			}
			else if ( filename=="beamer.cls" )  
				map["Beamer"] = true;
			else if ( filename=="prosper.cls" )
				map["Prosper"] = true;
			else if ( filename=="HA-prosper.sty" )
				map["HA-prosper"] = true;
		}
		
	
		KileTemplate::TemplateList templateList = m_templateManager->getTemplates(KileDocument::LaTeX);
		// insert all standard templates, all user defined templates 
		// and those templates, which have a present class 
		for (KileTemplate::TemplateListIterator i=templateList.begin(); i != templateList.end(); ++i)
		{
			KileTemplate::Info info = *i;
			QString classname = info.name;
			if ( !map.contains(classname) || map[classname]==true )
			{
				new TemplateItem(this, info);
			}
		}
	}
	else {
		KileTemplate::TemplateList templateList = m_templateManager->getTemplates(type); 
		for (KileTemplate::TemplateListIterator i=templateList.begin(); i != templateList.end(); ++i)
		{
			new TemplateItem(this, *i);
		}
	}

	// sort all items (item for 'Empty Document' will always be the first one)
	sort();
	
	// set the default item, if its given
	for ( Q3IconViewItem *item = firstItem(); item; item = item->nextItem() ) {
		if ( static_cast<TemplateItem*>(item)->name() == m_selicon ) {
			setSelected(item, true);
			ensureItemVisible(item);
		}
	}
}

#include "templates.moc"
