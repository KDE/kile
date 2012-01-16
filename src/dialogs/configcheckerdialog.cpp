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

#include "kiledebug.h"

#include "widgets/configcheckerwidget.h"

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
: KDialog(parent),
  m_ki(kileInfo),
  m_tester(NULL)
{
	setCaption(i18n("System Check"));
	setModal(true);
	setButtons(Ok | Cancel | User1);
	setDefaultButton(Ok);
	showButtonSeparator(true);
	setButtonGuiItem(User1, KGuiItem(i18n("&Save Results...")));

	m_widget = new ConfigCheckerWidget(this);
	setMainWidget(m_widget);

	enableButton(Ok, false);
	enableButton(User1, false);

	listWidget()->setAlternatingRowColors(true);
	listWidget()->setSelectionMode(QAbstractItemView::NoSelection);
	listWidget()->setItemDelegate(new ResultItemDelegate(listWidget()));

	run();
}

ConfigChecker::~ConfigChecker()
{
}

QProgressBar* ConfigChecker::progressBar()
{
	return m_widget->progressBar();
}

QLabel* ConfigChecker::label()
{
	return m_widget->label();
}

QListWidget* ConfigChecker::listWidget()
{
	return m_widget->listWidget();
}

void ConfigChecker::run()
{
	m_tester = new Tester(m_ki, this);

	connect(m_tester, SIGNAL(started()), this, SLOT(started()));
	connect(m_tester, SIGNAL(percentageDone(int)), this, SLOT(setPercentageDone(int)));
	connect(m_tester, SIGNAL(finished(bool)), this, SLOT(finished(bool)));
	connect(this, SIGNAL(user1Clicked()), this, SLOT(saveResults()));

	m_tester->runTests();
}

void ConfigChecker::slotCancel()
{
	finished(false);
	reject();
}

void ConfigChecker::saveResults()
{
	KUrl url = KFileDialog::getSaveUrl();
	if (!url.isEmpty()) {
//FIXME
	}
}

void ConfigChecker::started()
{
	setCursor(Qt::BusyCursor);
	setPercentageDone(0);
}

void ConfigChecker::finished(bool ok)
{
	setCursor(Qt::ArrowCursor);
	enableButton(Cancel, false);

	if(ok) {
		label()->setText(i18n("Finished testing your system."));

		QStringList tools = m_tester->testedTools();
		QStringList critical, failure;
		for (int i = 0; i < tools.count(); ++i) {
			bool isCritical = false;
			int status = m_tester->statusForTool(tools[i], &isCritical);
			if (status == ConfigTest::Failure) {
				if(isCritical) {
					critical.append(tools[i]);
				}
				else {
					failure.append(tools[i]);
				}
			}
			new ResultItem(listWidget(), tools[i], status, isCritical, m_tester->resultForTool(tools[i]));
		}

		listWidget()->sortItems();

		QString cap = i18n("Test Results");
		if (critical.count() > 0) {
			KMessageBox::error(this, i18n("<qt>The following <b>critical</b> tests did not succeed:"
			                              "<br/><br/>%1<br/><br/>Kile cannot function correctly on your system. Please consult the"
			                              "<br/>test results to determine which programs have to be fixed.</qt>", critical.join(", ")), cap);
		}
		else {
			if (failure.count() > 0) {
				KMessageBox::information(this, i18n("The following tests did not succeed:\n\n%1\n\nYou will still "
				                                    "be able to use Kile; however, not all features\n are guaranteed "
				                                    "to work.", failure.join(", ")), cap);
			}
			else {
				KMessageBox::information(this, i18n("No problems detected. Kile will work correctly on your system."), cap);
			}
		}
	}
	else {
		label()->setText(i18n("Tests finished abruptly..."));
		KMessageBox::error(this, i18n("The tests could not be finished correctly. Please check the available disk space."));
	}

	enableButton(Ok, true);
	enableButton(User1, true);
}

void ConfigChecker::setPercentageDone(int p)
{
	progressBar()->setValue(p);
}

}

#include "configcheckerdialog.moc"
