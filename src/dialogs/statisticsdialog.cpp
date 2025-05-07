/**********************************************************************************
    begin                : Tuesday Nov 1 2005
    copyright            : (C) 2005 by Thomas Braun (thomas.braun@virtuell-zuhause.de)
                               2015 by Andreas Cord-Landwehr (cordlandwehr@kde.org)
 **********************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dialogs/statisticsdialog.h"
#include "kileproject.h"
#include "widgets/statisticswidget.h"

#include <KConfigGroup>
#include <KHelpClient>
#include <KLocalizedString>
#include <KTextEditor/Application>
#include <KTextEditor/MainWindow>
#include <KTextEditor/View>
#include <QApplication>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

// A dialog that displays statistical information about the active project/file

namespace KileDialog {

StatisticsDialog::StatisticsDialog(KileProject *project, KileDocument::TextInfo* docinfo, QWidget* parent,
                                   KTextEditor::View *view, const QString &caption)
    : KPageDialog(parent), m_project(project), m_docinfo(docinfo), m_view(view)
{
    setFaceType(Tabbed);
    setWindowTitle(caption);
    setModal(true);
    setStandardButtons(QDialogButtonBox::Help | QDialogButtonBox::Close);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *copyButton = new QPushButton;
    copyButton->setText(i18n("Copy"));
    buttonBox()->addButton(copyButton, QDialogButtonBox::ActionRole);
    QPushButton *copyLatexButton = new QPushButton;
    copyLatexButton->setText(i18n("Copy as LaTeX"));
    buttonBox()->addButton(copyLatexButton, QDialogButtonBox::ActionRole);
    buttonBox()->button(QDialogButtonBox::Close)->setDefault(true);

    connect(copyButton, &QPushButton::clicked, this, [=]() {
        KILE_DEBUG_MAIN << "Open tab is" << currentPage()->name() << QLatin1Char(' ') + (m_pagetoname.contains(currentPage()) ?  m_pagetoname[currentPage()] : QStringLiteral("No such entry"));
        QClipboard *clip = QApplication::clipboard();
        QString text;
        convertText(&text, false);
        clip->setText(text, QClipboard::Selection); // the text will be available with the middle mouse button
    });
    connect(copyLatexButton, &QPushButton::clicked, this, [=]() {
        KILE_DEBUG_MAIN << "Open tab is" << currentPage()->name() << QLatin1Char(' ') + (m_pagetoname.contains(currentPage()) ?  m_pagetoname[currentPage()] : QStringLiteral("No such entry"));
        QClipboard *clip = QApplication::clipboard();
        QString text;
        convertText(&text, true);
        clip->setText(text, QClipboard::Selection); // the text will be available with the middle mouse button
    });
    connect(buttonBox(), &QDialogButtonBox::helpRequested, this, [=] () {
        KHelpClient::invokeHelp(QStringLiteral("statistics"), QStringLiteral("kile"));
    });

    m_summarystats = new long[SIZE_STAT_ARRAY];
    m_summarystats[0] = m_summarystats[1] = m_summarystats[2] = m_summarystats[3] = m_summarystats[4] = m_summarystats[5] = 0;

    const long* stats;
    QString tempName;
    KileWidget::StatisticsWidget* tempWidget;
    KileWidget::StatisticsWidget* summary;
    KileDocument::TextInfo* tempDocinfo;

    m_hasSelection = view->selection(); // class variable, if the user has selected text,
    summary = new KileWidget::StatisticsWidget(mainWidget);
    KPageWidgetItem *itemSummary = new KPageWidgetItem(summary, i18n("Summary"));
    addPage(itemSummary);
    summary->m_commentAboutHelp->setText(i18n("For information about the accuracy see the Help."));

    // we have in every case a summary tab
    m_pagetowidget[itemSummary] = summary;
    m_pagetoname[itemSummary] = i18n("Summary");

    if (!m_project) { // the active doc doesn't belong to a project
        setWindowTitle(i18n("Statistics for %1", m_docinfo->getDoc()->url().fileName()));
        stats = m_docinfo->getStatistics(m_view);
        fillWidget(stats, summary);
    }
    else { // active doc belongs to a project
        setWindowTitle(i18n("Statistics for the Project %1", m_project->name()));
        KILE_DEBUG_MAIN << "Project file is " << project->baseURL() << Qt::endl;


        if (m_hasSelection) { // if the active doc has a selection
            stats = m_docinfo->getStatistics(m_view);
            fillWidget(stats, summary); // if yes we fill the summary widget and are finished
        }
        else {
            const QList<KileProjectItem*> items = project->items();
            for(const KileProjectItem *item : items) {

                if (item->type() ==  KileProjectItem::ProjectFile) { // ignore project files
                    continue;
                }

                tempDocinfo = item->getInfo();
                if(tempDocinfo && tempDocinfo->getDoc()) { // closed items don't have a doc
                    tempName = tempDocinfo->getDoc()->url().fileName();
                    stats = tempDocinfo->getStatistics(m_view);

                    for (uint j = 0; j < SIZE_STAT_ARRAY; j++) {
                        m_summarystats[j] += stats[j];
                    }

                    tempWidget = new KileWidget::StatisticsWidget();
                    KPageWidgetItem *itemTemp = new KPageWidgetItem(tempWidget, tempName);
                    addPage(itemTemp);
                    KILE_DEBUG_MAIN << "TempName is " << tempName << Qt::endl;
                    m_pagetowidget[itemTemp] = tempWidget;
                    m_pagetoname[itemTemp] = tempName;
                    fillWidget(stats, tempWidget);
                }
                else {
                    m_notAllFilesOpenWarning = true; // print warning
                }
            }

            fillWidget(m_summarystats, summary);
            if (m_notAllFilesOpenWarning) {
                summary->m_warning->setText(i18n("To get statistics for all project files, you have to open them all."));
            }

            KILE_DEBUG_MAIN << "All keys in name " << m_pagetoname.keys() << " Nr. of keys " << m_pagetowidget.count() << Qt::endl;
            KILE_DEBUG_MAIN << "All keys in widget " << m_pagetowidget.keys() << " Nr. of keys " << m_pagetowidget.count() << Qt::endl;
        }
    }
}

StatisticsDialog::~StatisticsDialog()
{
    delete [] m_summarystats;
}

void StatisticsDialog::fillWidget(const long* stats, KileWidget::StatisticsWidget* widget)
{
// we don't have to write 0's in the number labels because this is the default value
    if (!stats || !widget) {
        return;
    }

    if (m_hasSelection) {
        widget->m_warning->setText(i18n("WARNING: These are the statistics for the selected text only."));
    }

    widget->m_wordChar->setText(QString::number(stats[0]));
    widget->m_commandChar->setText(QString::number(stats[1]));
    widget->m_whitespaceChar->setText(QString::number(stats[2]));
    widget->m_totalChar->setText(QString::number(stats[0] + stats[1] + stats[2]));

    widget->m_wordString->setText(QString::number(stats[3]));
    widget->m_commandString->setText(QString::number(stats[4]));
    widget->m_environmentString->setText(QString::number(stats[5]));
    widget->m_totalString->setText(QString::number(stats[3] + stats[4] + stats[5]));

    widget->updateColumns();
}

void StatisticsDialog::convertText(QString* text, bool forLaTeX) // the bool determines if we want plainText or LaTeXCode
{
    KileWidget::StatisticsWidget* widget = m_pagetowidget[currentPage()];
    QString name = m_pagetoname[currentPage()];
    QString charGroupName = widget->m_charactersGroup->title();
    QString stringGroupName = widget->m_stringsGroup->title();

    if (forLaTeX) {
        text->append(QStringLiteral("\\begin{tabular}{ll}\n"));
    }

    if (m_project && currentPage()) {
        text->append(i18n("Statistics for project %1, file %2", m_project->name(), name));
    }
    else {
        if (m_project) {
            text->append(i18n("Statistics for project %1", m_project->name()));
        }
        else {
            if (m_docinfo->getDoc()->url().isValid()) {
                text->append(i18n("Statistics for %1", m_docinfo->getDoc()->url().fileName()));
            }
            else {
                text->append(i18n("Statistics for Untitled"));
            }
        }
    }
    if(forLaTeX) {
        text->append(QStringLiteral(" & \\\\\\hline\n"));
    }
    else {
        text->append(QStringLiteral("\n\n"));
    }
    text->append(charGroupName + (forLaTeX ? QStringLiteral(" &  \\\\\n") : QStringLiteral("\n")));
    text->append(widget->m_wordCharText->text() + (forLaTeX ? QStringLiteral(" & ") : QStringLiteral("\t")) + widget->m_wordChar->text() + (forLaTeX ? QStringLiteral(" \\\\\n") : QStringLiteral("\n")));
    text->append(widget->m_commandCharText->text() + (forLaTeX ? QStringLiteral(" & ") : QStringLiteral("\t")) + widget->m_commandChar->text() + (forLaTeX ? QStringLiteral(" \\\\\n") : QStringLiteral("\n")));
    text->append(widget->m_whitespaceCharText->text() + (forLaTeX ? QStringLiteral(" & ") : QStringLiteral("\t")) + widget->m_whitespaceChar->text() + (forLaTeX ? QStringLiteral(" \\\\\n") : QStringLiteral("\n")));
    text->append(widget->m_totalCharText->text() + (forLaTeX ? QStringLiteral(" & ") : QStringLiteral("\t")) + widget->m_totalChar->text() + (forLaTeX ? QStringLiteral(" \\\\\n") : QStringLiteral("\n")));

    text->append((forLaTeX ? QStringLiteral(" & \\\\\n") : QStringLiteral("\n")));
    text->append(stringGroupName + (forLaTeX ? QStringLiteral(" &  \\\\\n") : QStringLiteral("\n")));
    text->append(widget->m_wordStringText->text() + (forLaTeX ? QStringLiteral(" & ") : QStringLiteral("\t")) + widget->m_wordString->text() + (forLaTeX ? QStringLiteral(" \\\\\n") : QStringLiteral("\n")));
    text->append(widget->m_commandStringText->text() + (forLaTeX ? QStringLiteral(" & ") : QStringLiteral("\t")) + widget->m_commandString->text() + (forLaTeX ? QStringLiteral(" \\\\\n") : QStringLiteral("\n")));
    text->append(widget->m_environmentStringText->text() + (forLaTeX ? QStringLiteral(" & ") : QStringLiteral("\t")) + widget->m_environmentString->text() + (forLaTeX ? QStringLiteral(" \\\\\n") : QStringLiteral("\n")));
    text->append(widget->m_totalStringText->text() + (forLaTeX ? QStringLiteral(" & ") : QStringLiteral("\t")) + widget->m_totalString->text() + (forLaTeX ? QStringLiteral(" \\\\\\hline\n") : QStringLiteral("\n")));

    if (forLaTeX) {
        text->append(QStringLiteral("\\end{tabular}\n"));
    }

    if (m_hasSelection) { // we can't have both cases
        text->append((forLaTeX ? QStringLiteral("\\par\\bigskip\n") : QStringLiteral("\n")) + widget->m_warning->text() + QLatin1Char('\n'));
    }
    else {
        if(m_notAllFilesOpenWarning) {
            text->append((forLaTeX ? QStringLiteral("\\par\\bigskip\n") : QStringLiteral("\n")) + widget->m_warning->text() + QLatin1Char('\n'));
        }
    }
}

}
