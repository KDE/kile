/*****************************************************************************************
    begin                : Sun Apr 27 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (wijnhout@science.uva.nl)
                               2007 by Michel Ludwig (michel.ludwig@kdemail.net)
 *****************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "managetemplatesdialog.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qtooltip.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

#include <kapplication.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <kicondialog.h>
#include <kmessagebox.h>
#include "kiledebug.h"
#include <kio/netaccess.h>

#include "kileextensions.h"
#include "kileinfo.h"
#include "templates.h"

class TemplateListViewItem : public Q3ListViewItem {
	public:
		TemplateListViewItem(Q3ListView* listView, Q3ListViewItem* previousItem, const QString& mode, const KileTemplate::Info& info) : Q3ListViewItem(listView, previousItem, mode, info.name, KileInfo::documentTypeToString(info.type)), m_info(info) {
		}

		virtual ~TemplateListViewItem() {
		}

		KileTemplate::Info getTemplateInfo() {
			return m_info;
		}

	protected:
		KileTemplate::Info m_info;
};

// dialog to create a template
ManageTemplatesDialog::ManageTemplatesDialog(KileTemplate::Manager *templateManager, const KUrl& sourceURL, const QString &caption, QWidget *parent, const char *name ) : KDialogBase(parent,name,true,caption,KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true), m_templateManager(templateManager), m_sourceURL(sourceURL) {

	m_templateType = KileDocument::Extensions().determineDocumentType(sourceURL);

	QWidget *page = new QWidget(this , "managetemplates_mainwidget");
	setMainWidget(page);
	Q3VBoxLayout *topLayout = new Q3VBoxLayout(page, 0, spacingHint());

	Q3HBoxLayout *nameLayout = new Q3HBoxLayout(topLayout, spacingHint());
	nameLayout->addWidget(new QLabel(i18n("Name:"),page));

	QString fileName = m_sourceURL.fileName();
	//remove the extension
	int dotPos = fileName.findRev('.');
	if(dotPos >= 0) {
		fileName = fileName.mid(0, dotPos);
	}
	m_nameEdit = new KLineEdit(fileName, page);
	nameLayout->addWidget(m_nameEdit);

	nameLayout->addWidget(new QLabel(i18n("Type: %1").arg(KileInfo::documentTypeToString(m_templateType)), page));

	Q3HBoxLayout *iconLayout = new Q3HBoxLayout(topLayout, spacingHint());
	iconLayout->addWidget(new QLabel(i18n("Icon:"), page));

	m_iconEdit = new KLineEdit(KGlobal::dirs()->findResource("appdata", "pics/type_Default.png"), page);
	iconLayout->addWidget(m_iconEdit);

	KPushButton *iconbut = new KPushButton(i18n("Select..."),page);
	iconLayout->addWidget(iconbut);

	m_templateList = new K3ListView(page);
	m_templateList->setSorting(-1);
	m_templateList->addColumn(i18n("marked", "M"));
	m_templateList->addColumn(i18n("Existing Templates"));
	m_templateList->addColumn(i18n("Document Type"));
	m_templateList->setColumnWidthMode(0, Q3ListView::Manual);
	m_templateList->setFullWidth(true);
	m_templateList->setAllColumnsShowFocus(true);

	populateTemplateListView(m_templateType);

	topLayout->addWidget(m_templateList);

	Q3HBoxLayout *controlLayout = new Q3HBoxLayout(topLayout, spacingHint());
	m_showAllTypesCheckBox = new QCheckBox(i18n("Show all the templates"), page);
	m_showAllTypesCheckBox->setChecked(false);
	connect(m_showAllTypesCheckBox, SIGNAL(toggled(bool)), this, SLOT(updateTemplateListView(bool)));
	controlLayout->addWidget(m_showAllTypesCheckBox);

	controlLayout->addStretch();

	KPushButton *clearSelectionButton = new KPushButton(page);
	clearSelectionButton->setPixmap(SmallIcon("clear_left.png"));
	QToolTip::add(clearSelectionButton, i18n("Clear Selection"));
	connect(clearSelectionButton, SIGNAL(clicked()),this, SLOT(clearSelection()));
	controlLayout->addWidget(clearSelectionButton);

	topLayout->addWidget( new QLabel(i18n("Select an existing template if you want to overwrite it with your new template.\nNote that you cannot overwrite templates marked with an asterisk:\nif you do select such a template, a new template with the same name\nwill be created in a location you have write access to."),page));

	connect(m_templateList, SIGNAL(selectionChanged(Q3ListViewItem*)), this, SLOT(slotSelectedTemplate(Q3ListViewItem*)));
	connect(iconbut, SIGNAL(clicked()),this, SLOT(slotSelectIcon()));
	connect(this, SIGNAL(aboutToClose()), this, SLOT(addTemplate()));
}

// dialog to remove a template
ManageTemplatesDialog::ManageTemplatesDialog(KileTemplate::Manager *templateManager, const QString &caption, QWidget *parent, const char *name ) : KDialogBase(parent,name,true,caption,KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true), m_templateManager(templateManager), m_templateType(KileDocument::Undefined), m_showAllTypesCheckBox(NULL)
{
	QWidget *page = new QWidget(this, "managetemplates_mainwidget");
	setMainWidget(page);
	Q3VBoxLayout *topLayout = new Q3VBoxLayout(page, 0, spacingHint());

	m_templateList = new K3ListView(page);
	m_templateList->setSorting(-1);
	m_templateList->addColumn(i18n("marked", "M"));
	m_templateList->addColumn(i18n("Existing Templates"));
	m_templateList->addColumn(i18n("Document Type"));
	m_templateList->setColumnWidthMode(0, Q3ListView::Manual);
	m_templateList->setFullWidth(true);
	m_templateList->setAllColumnsShowFocus(true);

	populateTemplateListView(KileDocument::Undefined);

	topLayout->addWidget(m_templateList);
	topLayout->addWidget( new QLabel(i18n("Please select the template that you want to remove.\nNote that you cannot delete templates marked with an asterisk (for which you lack the necessary deletion permissions)."),page));

	connect(this, SIGNAL(aboutToClose()), this, SLOT(removeTemplate()));
}

ManageTemplatesDialog::~ManageTemplatesDialog(){
}

void ManageTemplatesDialog::updateTemplateListView(bool showAllTypes) {
	populateTemplateListView((showAllTypes ? KileDocument::Undefined : m_templateType));
}

void ManageTemplatesDialog::clearSelection() {
	m_templateList->clearSelection();
}

void ManageTemplatesDialog::slotOk() {
	emit aboutToClose();
}

void ManageTemplatesDialog::populateTemplateListView(KileDocument::Type type) {
	m_templateManager->scanForTemplates();
	KileTemplate::TemplateList templateList = m_templateManager->getTemplates(type);
	QString mode;
	Q3ListViewItem* previousItem = NULL;

	m_templateList->clear();
	for (KileTemplate::TemplateListIterator i = templateList.begin(); i != templateList.end(); ++i)
	{
		KileTemplate::Info info = *i;
		QFileInfo iconFileInfo(info.icon);
		mode = (QFileInfo(info.path).isWritable() && (!iconFileInfo.exists() || iconFileInfo.isWritable())) ? " " : "*";
		if((type == KileDocument::Undefined) || (info.type == type)) { 
			previousItem = new TemplateListViewItem(m_templateList, previousItem, mode, info);
		}
	}
}

void ManageTemplatesDialog::slotSelectedTemplate(Q3ListViewItem *item) {
	TemplateListViewItem *templateItem = dynamic_cast<TemplateListViewItem*>(item);
	if(templateItem) {
		KileTemplate::Info info = templateItem->getTemplateInfo();
		m_nameEdit->setText(info.name);
		m_iconEdit->setText(info.icon);
	}
}

void ManageTemplatesDialog::slotSelectIcon() {
	KIconDialog *dlg = new KIconDialog();
	QString res = dlg->openDialog();
	KIconLoader kil;

	if (!res.isNull() ) {
		m_iconEdit->setText(kil.iconPath(res,-KIconLoader::SizeLarge, false));
	}
}

void ManageTemplatesDialog::addTemplate() {

	QString templateName = (m_nameEdit->text()).trimmed();

	if(templateName.isEmpty()) {
		KMessageBox::error(this, i18n("Sorry, but the template name that you have entered is invalid.\nPlease enter a new name."));
		return;
	}

	QString icon = (m_iconEdit->text()).trimmed();
	KUrl iconURL = KUrl::fromPathOrUrl(icon);

	if (icon.isEmpty()) {
		KMessageBox::error(this, i18n("Please choose an icon first."));
		return;
	}

	if (!KIO::NetAccess::exists(iconURL, true, kapp->mainWidget())) {
		KMessageBox::error(this, i18n("Sorry, but the icon file: %1\ndoes not seem to exist. Please choose a new icon.").arg(icon));
		return;
	}

	if (!KIO::NetAccess::exists(m_sourceURL, true, kapp->mainWidget())) {
		KMessageBox::error(this, i18n("Sorry, but the file: %1\ndoes not seem to exist. Maybe you forgot to save the file?").arg(m_sourceURL.prettyUrl()));
		return;
	}

	Q3ListViewItem* item = m_templateList->selectedItem();

	if(!item && m_templateManager->searchForTemplate(templateName, m_templateType)) {
		KMessageBox::error(this, i18n("Sorry, but a template named \"%1\" already exists.\nPlease remove it first.").arg(templateName));
		return;
	}

	bool returnValue;
	if(item) {
		TemplateListViewItem *templateItem = dynamic_cast<TemplateListViewItem*>(item);
		Q_ASSERT(templateItem);
		KileTemplate::Info templateInfo = templateItem->getTemplateInfo();
		if (KMessageBox::warningYesNo(this, i18n("You are about to replace the template \"%1\"; are you sure?").arg(templateInfo.name)) == KMessageBox::No) {
			reject();
			return;
		}
		returnValue = m_templateManager->replace(templateInfo, m_sourceURL, templateName, iconURL);
	}
	else {
		returnValue = m_templateManager->add(m_sourceURL, templateName, iconURL);
	}
	if (!returnValue) {
		KMessageBox::error(this, i18n("Failed to create the template."));
		reject();
		return;
	}
	accept();
}

bool ManageTemplatesDialog::removeTemplate()
{
	Q3ListViewItem* item = m_templateList->selectedItem();
	if(!item) {
		KMessageBox::information(this, i18n("Please select a template that should be removed."));
		return true;
	}

	TemplateListViewItem *templateItem = dynamic_cast<TemplateListViewItem*>(item);
	Q_ASSERT(templateItem);
	
	KileTemplate::Info templateInfo = templateItem->getTemplateInfo();

	if (!(KIO::NetAccess::exists(KUrl::fromPathOrUrl(templateInfo.path), false, kapp->mainWidget()) && (KIO::NetAccess::exists(KUrl::fromPathOrUrl(templateInfo.icon), false, kapp->mainWidget()) || !QFileInfo(templateInfo.icon).exists()))) {
		KMessageBox::error(this, i18n("Sorry, but you do not have the necessary permissions to remove the selected template."));
		return false;
	}

	if (KMessageBox::warningYesNo(this, i18n("You are about to remove the template \"%1\"; are you sure?").arg(templateInfo.name)) == KMessageBox::No) {
		return false;
	}

	if (!m_templateManager->remove(templateInfo))
	{
		KMessageBox::error(this, i18n("Sorry, but the template could not be removed."));
		reject();
		return false;
	}
	accept();
	return true;
}

#include "managetemplatesdialog.moc"
