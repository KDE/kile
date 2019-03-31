/*************************************************************************************
  Copyright (C) 2004 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                2012-2019 by Michel Ludwig (michel.ludwig@kdemail.net)
 *************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONFIGCHECKERDIALOG_H
#define CONFIGCHECKERDIALOG_H

#include <KAssistantDialog>

#include <QCheckBox>
#include <QLabel>
#include <QList>
#include <QListWidget>
#include <QProgressBar>

#include "configtester.h"

class KileInfo;

namespace KileDialog
{
class ResultItem : public QListWidgetItem
{
public:
    ResultItem(QListWidget *listWidget, const QString &toolGroup, int status, bool isCritical, const QList<ConfigTest*> &tests);
};

class ConfigChecker : public KAssistantDialog
{
    Q_OBJECT

public:
    explicit ConfigChecker(KileInfo *kileInfo, QWidget* parent = Q_NULLPTR);
    ~ConfigChecker();

public Q_SLOTS:
    void run();
    void started();
    void finished(bool);
    void setPercentageDone(int);
    void slotCancel();

    void next() override;

protected Q_SLOTS:
    void assistantFinished();

private:
    KileInfo *m_ki;
    Tester    *m_tester;
    QProgressBar *m_progressBar;
    QListWidget *m_listWidget;
    QLabel *m_overallResultLabel;
    KPageWidgetItem *m_introPageWidgetItem, *m_runningTestsPageWidgetItem, *m_testResultsPageWidgetItem;
    QCheckBox *m_useEmbeddedViewerCheckBox, *m_useModernConfigurationForLaTeXCheckBox, *m_useModernConfigurationForPDFLaTeX;
};
}
#endif
