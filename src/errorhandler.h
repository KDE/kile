/***************************************************************************
   Copyright (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                 2011-2016 by Michel Ludwig (michel.ludwig@kdemail.net)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <QObject>
#include <QPointer>

#include "outputinfo.h"

class QLabel;
class QTabWidget;
class QToolBar;
class QToolButton;

class QAction;
class KActionCollection;
class KileInfo;
class KileProject;
class LaTeXOutputHandler;
class OutputInfo;

namespace KileTool {
class Base;
}
namespace KileWidget {
class LogWidget;
}

class KileErrorHandler : public QObject
{
    Q_OBJECT

public:
    KileErrorHandler(QObject *parent, KileInfo *info, KActionCollection *ac);

    ~KileErrorHandler();

    QLabel* compilationResultLabel();

    QWidget* outputWidget();

    void setErrorHandlerToolBar(QToolBar *toolBar);

    void setMostRecentLogInformation(const QString& logFile, const LatexOutputInfoArray& outputInfoList);


    enum ProblemType { AllProblems = 0, OnlyErrors, OnlyWarnings, OnlyBadBoxes };

    void displayProblemsInLogWidget(KileWidget::LogWidget *logWidget, const LatexOutputInfoArray& infoList, ProblemType problemType = AllProblems);

    bool areMessagesShown() const;
    void addEmptyLineToMessages();

    void startToolLogOutput();
    void endToolLogOutput();

Q_SIGNALS:
    void showingErrorMessage(QWidget *w);
    void currentLaTeXOutputHandlerChanged(LaTeXOutputHandler *outputHandler);

public Q_SLOTS:
    void handleSpawnedChildTool(KileTool::Base *parent, KileTool::Base *child);
    void handleLaTeXToolDone(KileTool::Base*, int, bool childToolSpawned);


    void printMessage(const QString& message);
    void printMessage(int type, const QString& message, const QString &tool = "Kile",
                      const OutputInfo& outputInfo = OutputInfo(), bool allowSelection = false,
                      bool scroll = true);
    void printProblem(int type, const QString& problem, const OutputInfo& outputInfo = OutputInfo());
    void clearMessages();

    void jumpToFirstError();
    void jumpToProblem(const OutputInfo& info);

    int currentOutputTabIndex();
    void setCurrentOutputTab(int i);

    void showMessagesOutput();
    void showErrorsOutput();
    void showWarningsOutput();
    void showBadBoxesOutput();

    void clearErrorOutput();

    /* log view, error handling */
private Q_SLOTS:
    void ViewLog();
    void NextError();
    void PreviousError();
    void NextWarning();
    void PreviousWarning();
    void NextBadBox();
    void PreviousBadBox();

    void setOutputActionsEnabled(bool b);

    void handleProjectOpened(KileProject *project);
    void updateCurrentLaTeXOutputHandler();
    void updateForCompilationResult();

private:
    KileInfo				*m_ki;
    QToolBar				*m_errorHanderToolBar;
    int					m_currentOutputType;
    LaTeXOutputHandler			*m_currentLaTeXOutputHandler;
    QPointer<QTabWidget>			m_outputTabWidget;
    QPointer<QLabel>			m_compilationResultLabel;
    QPointer<KileWidget::LogWidget>		m_mainLogWidget;
    QPointer<KileWidget::LogWidget>		m_errorLogWidget, m_warningLogWidget, m_badBoxLogWidget;
    QPointer<QAction>			m_viewLogAction;
    QPointer<QAction>			m_previousErrorAction, m_nextErrorAction;
    QPointer<QAction>			m_previousWarningAction, m_nextWarningAction;
    QPointer<QAction>			m_previousBadBoxAction, m_nextBadBoxAction;

    void createActions(KActionCollection *ac);
    void jumpToProblem(int type, bool);
    void displayProblemsInMainLogWidget(const LatexOutputInfoArray& infoList);
    void printNoInformationAvailable();
};

#endif
