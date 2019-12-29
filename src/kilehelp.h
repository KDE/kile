/***************************************************************************
    begin     : 2004
    copyright : (C) 2004-2012 by Holger Danielsson (holger.danielsson@versanet.de)
                (C) 2019 by Michel Ludwig (michel.ludwig@kdemail.net)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KILEHELP_H
#define KILEHELP_H

#include <QMap>

#include <KActionMenu>
#include <KTextEditor/View>

#include "editorextension.h"
#include "userhelp.h"

namespace KileDocument {
    class EditorExtension;
}

namespace KileHelp
{

enum HelpType
{
    HelpKileRefs,
    HelpLatex2eRefs,
    HelpLatexIndex,
    HelpLatexCommand,
    HelpLatexEnvironment
};

class Help : public QObject
{
    Q_OBJECT

public:
    Help(KileDocument::EditorExtension *edit, QWidget *mainWindow);
    ~Help();

    void setUserhelp(KileTool::Manager *manager, KActionMenu *userHelpActionMenu);
    void update();

    // calls for help
    void helpKeyword(KTextEditor::View *view);
    void noHelpAvailableFor(const QString &word);
    void userHelpDialog() {
        m_userhelp->userHelpDialog();
    }
    void enableUserhelpEntries(bool state);

public Q_SLOTS:
    inline void helpLatexIndex() {
        helpLatex(KileHelp::HelpLatexIndex);
    }
    inline void helpLatexCommand() {
        helpLatex(KileHelp::HelpLatexCommand);
    }
    inline void helpLatexEnvironment() {
        helpLatex(KileHelp::HelpLatexEnvironment);
    }
    void helpKeyword();
    void helpDocBrowser();

private:
    QWidget *m_mainWindow;
    KileTool::Manager *m_manager;
    KileDocument::EditorExtension *m_edit;
    UserHelp *m_userhelp;
    QString m_helpDir;

    QString m_latex2eReference;
    QString m_kileReference;

    HelpType m_contextHelpType;
    QMap<QString, QString> m_dictHelpTex;

    void initContextHelp();

    void readHelpList(const QString &filename);
    void showHelpFile(const QString &parameter);

    void helpLatex(HelpType type);
    QString getKeyword(KTextEditor::View *view);
    HelpType contextHelpType();

};
}

#endif
