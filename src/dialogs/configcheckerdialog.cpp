/*************************************************************************************
  Copyright (C) 2004 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                2012 by Michel Ludwig (michel.ludwig@kdemail.net)
 *************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dialogs/configcheckerdialog.h"

#include <QFileInfo>
#include <QLabel>
#include <QLayout>
#include <QItemDelegate>
#include <QPainter>
#include <QTextDocument>

#include <KCursor>
#include <KFileDialog>
#include <KGlobal>
#include <KLocale>
#include <KMessageBox>
#include <KProgressDialog>

#include "config.h"
#include "kiledebug.h"
#include "kileinfo.h"

namespace KileDialog
{

class ResultItemDelegate : public QItemDelegate {
	public:
		ResultItemDelegate(QListWidget *parent) : QItemDelegate(parent) {}

		virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
		{
			painter->save();
			drawBackground(painter, option, index);

			QTextDocument document;
			document.setHtml(index.data(Qt::UserRole).toString());
			painter->resetMatrix();
			painter->translate(option.rect.topLeft());
			document.drawContents(painter);
			painter->restore();
		}

		virtual QSize sizeHint(const QStyleOptionViewItem& /* option */, const QModelIndex &index) const
		{
			QTextDocument document;
			document.setHtml(index.data(Qt::UserRole).toString());
			return document.size().toSize();
		}
};

ResultItem::ResultItem(QListWidget *listWidget, const QString &toolGroup, int status, bool isCritical, const QList<ConfigTest*> &tests)
: QListWidgetItem(listWidget)
{
	QString rt = "<hr /><b><font color=\"%1\">%2</font></b> (%3)<br /><ul>";
	for (int i = 0; i < tests.count(); ++i) {
		QString itemcolor = "black";
		if (tests[i]->status() == ConfigTest::Failure) {
			if (tests[i]->isCritical()) {
				itemcolor = "#AA0000";
			}
			else {
				itemcolor = "#FFA201";
			}
		}
		rt += QString("<li><b><font color=\"%1\">%2</font></b>: &nbsp;%3</li>").arg(itemcolor).arg(tests[i]->name()).arg(tests[i]->resultText());
	}
	rt += "</ul>";

	QString color = "#00AA00", statustr = i18n("Passed");
	if(status == ConfigTest::Failure) {
		if(isCritical) {
			color = "#AA0000";
			statustr = i18n("Critical failure, Kile will not function properly");
		}
		else {
			color = "#FFA201";
			statustr = i18n("Failed, but not critical");
		}
	}

	setData(Qt::UserRole, rt.arg(color).arg(toolGroup).arg(statustr));

	//this is for sorting only
	setText(QString::number(status) + ':' + toolGroup);
}

ConfigChecker::ConfigChecker(KileInfo *kileInfo, QWidget* parent)
: KAssistantDialog(parent),
  m_ki(kileInfo),
  m_tester(NULL)
{
	// don't show the 'help' button in the title bar
	setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setCaption(i18n("System Check"));
	setModal(true);
	showButtonSeparator(true);
	setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));

	QWidget *introWidget = new QWidget(this);
	QLabel *label = new QLabel(i18n("<p>This configuration assistant will check whether your system is set up "
	                                "correctly to process LaTeX documents. It will also allow to fine-tune the configuration "
	                                "of Kile while taking the results of the tests into account.</p>"
	                                "<p>It is recommended to run this assistant before using Kile for the first time.</p>"
	                                "<p>Please press 'Next' now to start the test procedure.</p>"));
	label->setWordWrap(true);
	QVBoxLayout *vboxLayout = new QVBoxLayout(this);
	vboxLayout->setSizeConstraint(QLayout::SetMinimumSize);
	introWidget->setLayout(vboxLayout);
	vboxLayout->addWidget(label);
	vboxLayout->addStretch();
	m_introPageWidgetItem = addPage(introWidget, i18n("System Check & Configuration Assistant"));

	QWidget *runningTestsWidget = new QWidget(this);
	label = new QLabel(i18n("Checking whether the system is set up correctly..."));
	vboxLayout = new QVBoxLayout(this);
	vboxLayout->setSizeConstraint(QLayout::SetMinimumSize);
	runningTestsWidget->setLayout(vboxLayout);
	vboxLayout->addStretch();
	vboxLayout->addWidget(label);
	m_progressBar = new QProgressBar(this);
	vboxLayout->addWidget(m_progressBar);
	vboxLayout->addStretch();
	m_runningTestsPageWidgetItem = addPage(runningTestsWidget, "");

	QWidget *testResultsWidget = new QWidget(this);
	vboxLayout = new QVBoxLayout(this);
	vboxLayout->setSizeConstraint(QLayout::SetMinimumSize);
	testResultsWidget->setLayout(vboxLayout);
	m_listWidget = new QListWidget(this);
	m_listWidget->setMinimumHeight(200);
	vboxLayout->addWidget(m_listWidget);
	m_overallResultLabel = new QLabel(this);
	vboxLayout->addWidget(m_overallResultLabel);
	m_useEmbeddedViewerCheckBox = new QCheckBox(i18n("Configure the viewer tools to use the document viewer"));
	vboxLayout->addWidget(m_useEmbeddedViewerCheckBox);
	m_useModernConfigurationForLaTeXCheckBox = new QCheckBox(i18n("Use the 'modern' configuration for the TeX, PDFTeX, and LaTeX tools"));
	vboxLayout->addWidget(m_useModernConfigurationForLaTeXCheckBox);
	m_useModernConfigurationForPDFLaTeX = new QCheckBox(i18n("Use the 'modern' configuration for the PDFLaTeX, LuaLaTeX and XeLaTeX tools"));
	vboxLayout->addWidget(m_useModernConfigurationForPDFLaTeX);
	vboxLayout->addWidget(new QLabel(i18n("<br/>Please press 'Finish' now to accept the recommended configuration changes.")));
	vboxLayout->addStretch();

	m_testResultsPageWidgetItem = addPage(testResultsWidget, i18n("Test Results"));
	showButton(User1, false); // hide the 'finish' button initially
	showButton(User3, false); // don't show the 'back' button
	showButton(Help, false);

	m_listWidget->setAlternatingRowColors(true);
	m_listWidget->setSelectionMode(QAbstractItemView::NoSelection);
	m_listWidget->setItemDelegate(new ResultItemDelegate(m_listWidget));
}

ConfigChecker::~ConfigChecker()
{
}

void ConfigChecker::next()
{
	setCurrentPage(m_runningTestsPageWidgetItem);
	enableButton(User2, false); // disable the 'next' button
	run();
}

void ConfigChecker::run()
{
	m_tester = new Tester(m_ki, this);

	connect(m_tester, SIGNAL(started()), this, SLOT(started()));
	connect(m_tester, SIGNAL(percentageDone(int)), this, SLOT(setPercentageDone(int)));
	connect(m_tester, SIGNAL(finished(bool)), this, SLOT(finished(bool)));
	connect(this, SIGNAL(user1Clicked()), this, SLOT(assistantFinished()));

	m_tester->runTests();
}

void ConfigChecker::slotCancel()
{
	finished(false);
	reject();
}

void ConfigChecker::started()
{
	setCursor(Qt::BusyCursor);
	setPercentageDone(0);
}

void ConfigChecker::finished(bool ok)
{
	setCurrentPage(m_testResultsPageWidgetItem);
	setCursor(Qt::ArrowCursor);
	showButton(User2, false); // hide the 'next' button
	showButton(User1, true); // show the 'finish' button
	QString testResultText = "<br/>";

	QStringList tools = m_tester->testGroups();
	QStringList critical, failure;
	for (int i = 0; i < tools.count(); ++i) {
		bool isCritical = false;
		int status = m_tester->statusForGroup(tools[i], &isCritical);
		if (status == ConfigTest::Failure) {
			if(isCritical) {
				critical.append(tools[i]);
			}
			else {
				failure.append(tools[i]);
			}
		}
		new ResultItem(m_listWidget, tools[i], status, isCritical, m_tester->resultForGroup(tools[i]));
	}

	m_listWidget->sortItems();

	if(ok) {
		QString cap = i18n("Test Results");
		QString overallResultText;
		if (critical.count() > 0) {
			testResultText += i18n("The following <b>critical</b> tests did not succeed:"
			                       "<br/><br/>%1<br/><br/>Kile cannot function correctly on your system. Please consult the "
			                       "test results<br/>to determine which programs have to be fixed.", critical.join(", "));
		}
		else {
			if (failure.count() > 0) {
				testResultText += i18n("The following tests did not succeed:<br/><br/>%1<br/><br/>You will still "
				                       "be able to use Kile; however, not all features are guaranteed "
				                       "to work.", failure.join(", "));
			}
			else {
				testResultText += i18n("<b>No problems were detected. Kile will work correctly on your system.</b>");
			}
		}

		testResultText += "<br/><br/>";
		m_useModernConfigurationForLaTeXCheckBox->setChecked(m_tester->areSrcSpecialsSupportedForLaTeX());
		m_useModernConfigurationForPDFLaTeX->setChecked(m_tester->isSyncTeXSupportedForPDFLaTeX());

#ifdef HAVE_VIEWERINTERFACE_H
		if(m_tester->isViewerModeSupportedInOkular()) {
			m_useEmbeddedViewerCheckBox->setVisible(true);
			m_useEmbeddedViewerCheckBox->setChecked(true);
			if(m_tester->isSyncTeXSupportedForPDFLaTeX()) {
				testResultText += i18n("The embedded viewer is available and live preview is supported.");
			}
			else {
				testResultText += i18n("The embedded viewer is available, but the installed version of PDFLaTeX is<br/>"
				                       "<b>not compatible</b> with live preview.");
			}
		}
		else {
			m_useEmbeddedViewerCheckBox->setVisible(false);
			m_useEmbeddedViewerCheckBox->setChecked(false);

			testResultText += i18n("The embedded viewer is <b>not available</b> (as Okular is either not available or the installed<br/>version is too old). "
			                       "Live preview is hence not supported.");
		}
#else
		m_useEmbeddedViewerCheckBox->setVisible(false);
		m_useEmbeddedViewerCheckBox->setChecked(false);

		testResultText += i18n("The embedded viewer is <b>not available</b> (as Kile was compiled without support for the<br/>embedded viewer). "
		                       "Live preview is hence not supported.");
#endif
		testResultText += "<br/><br/>";

		m_overallResultLabel->setText(testResultText);
		enableButton(Ok, true);
		enableButton(User1, true);
	}
	else {
		// start by hiding all the labels
		Q_FOREACH(QWidget *widget, m_testResultsPageWidgetItem->widget()->findChildren<QLabel*>()) {
			widget->setVisible(false);
		}
		// and then we show those again that we want
		m_overallResultLabel->setVisible(true);
		m_useEmbeddedViewerCheckBox->setVisible(false);
		m_useModernConfigurationForLaTeXCheckBox->setVisible(false);
		m_useModernConfigurationForPDFLaTeX->setVisible(false);

		m_overallResultLabel->setText(i18n("<br/><font color=\"#FF0000\"><b>The tests could not be finished correctly. "
		                                   "Please check the available disk space.</b></font>"));
		enableButton(Ok, true);
		enableButton(User1, false);
	}
}

void ConfigChecker::assistantFinished()
{
	if(m_useEmbeddedViewerCheckBox->isChecked()) {
		m_ki->toolManager()->setConfigName("ViewPS", "Document Viewer");
		m_ki->toolManager()->setConfigName("ViewPDF", "Document Viewer");
		m_ki->toolManager()->setConfigName("ViewDVI", "Document Viewer");
	}
	if(m_useModernConfigurationForLaTeXCheckBox->isChecked()) {
		m_ki->toolManager()->setConfigName("TeX", "Modern");
		m_ki->toolManager()->setConfigName("PDFTeX", "Modern");
		m_ki->toolManager()->setConfigName("LaTeX", "Modern");
	}
	if(m_useModernConfigurationForPDFLaTeX->isChecked()) {
		m_ki->toolManager()->setConfigName("PDFLaTeX", "Modern");
		m_ki->toolManager()->setConfigName("XeLaTeX", "PDF Modern");
		m_ki->toolManager()->setConfigName("LuaLaTeX", "PDF Modern");
	}
}

void ConfigChecker::setPercentageDone(int p)
{
	m_progressBar->setValue(p);
}

}

#include "configcheckerdialog.moc"
