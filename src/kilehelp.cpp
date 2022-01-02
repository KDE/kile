/**********************************************************************************************
    Copyright (C) 2004-2012 by Holger Danielsson (holger.danielsson@versanet.de)
              (C) 2019 by Michel Ludwig (michel.ludwig@kdemail.net)
 **********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kilehelp.h"
#include <QDir>
#include <QFileInfo>
#include <QTextStream>

#include "editorextension.h"
#include "errorhandler.h"
#include "kiledebug.h"
#include "kiletool_enums.h"
#include "kiletoolmanager.h"
#include "kileviewmanager.h"
#include "kileinfo.h"
#include "dialogs/texdocumentationdialog.h"
#include "kileconfig.h"
#include "utilities.h"

namespace KileHelp
{

Help::Help(KileDocument::EditorExtension *edit, QWidget *mainWindow) : m_mainWindow(mainWindow), m_edit(edit), m_userhelp(Q_NULLPTR)
{
    m_helpDir = KileUtilities::locate(QStandardPaths::AppDataLocation, QLatin1String("help/"), QStandardPaths::LocateDirectory); // this must end in '/'
    KILE_DEBUG_MAIN << "help dir: " << m_helpDir;

    m_kileReference = m_helpDir + "latexhelp.html";
    m_latex2eReference =  m_helpDir + QLatin1String("unofficial-latex2e-reference-manual/");

    m_contextHelpType = contextHelpType();
    initContextHelp();
}

Help::~Help()
{
    delete m_userhelp;
}

void Help::initContextHelp()
{
    // read a list with keywords for context help
    if(m_contextHelpType == HelpKileRefs) {
        readHelpList("latex-kile.lst");
    }
    else if(m_contextHelpType == HelpLatex2eRefs) {
        readHelpList("unofficial-latex2e-reference-manual.index");
    }
}

////////////////////// update paths and context help of TeX documentation  //////////////////////

void Help::update()
{
    HelpType contextHelp = contextHelpType();
    if(m_contextHelpType != contextHelp) {
        m_contextHelpType = contextHelp;
        initContextHelp();
    }
}

////////////////////// set parameter/initialize user help //////////////////////

void Help::setUserhelp(KileTool::Manager *manager, KActionMenu *userHelpActionMenu)
{
    m_manager = manager;
    m_userhelp = new UserHelp(manager, userHelpActionMenu, m_mainWindow);
}

void Help::enableUserhelpEntries(bool state)
{
    if(m_userhelp) {
        m_userhelp->enableUserHelpEntries(state);
    }
}
////////////////////// show help //////////////////////

void Help::showHelpFile(const QString &parameter)
{
    KILE_DEBUG_MAIN << "--------------------------------------------> help file: " << parameter;
    KileTool::Base *tool = m_manager->createTool("ViewHTML", QString(), false);
    if(!tool) {
        return;
    }
    tool->setFlags(KileTool::NeedSourceExists | KileTool::NeedSourceRead);
    //FIXME strip the #label part of the source (not the target),
    //somehow this is already done somewhere (by accident),
    //bad to rely on it
    tool->setMsg(KileTool::NeedSourceExists, ki18n("Could not find the LaTeX documentation at %1; please set the correct path in Settings->Configure Kile->Help."));
    tool->setSource(parameter);
    tool->setTargetPath(parameter);
    tool->prepareToRun();
    m_manager->run(tool);
}

void Help::helpKeyword()
{
    //FIXME: we should have a better way to access the current view
    helpKeyword(m_manager->info()->viewManager()->currentTextView());
}

////////////////////// Help: TexDoc //////////////////////

void Help::helpDocBrowser()
{
    KileDialog::TexDocDialog *dlg = new KileDialog::TexDocDialog();
    dlg->exec();
    delete dlg;
}

////////////////////// Help: LaTeX //////////////////////

void Help::helpLatex(HelpType type)
{
    switch(type) {
        case HelpLatexIndex:
            showHelpFile(m_latex2eReference + QLatin1String("index.html"));
            break;
        case HelpLatexCommand:
            showHelpFile(m_latex2eReference + QLatin1String("IndexDocument.html#Index_cp_symbol-8"));
            break;
        case HelpLatexEnvironment:
            showHelpFile(m_latex2eReference + QLatin1String("Environments.html#Environments"));
            break;
        default:
            return;
    }
}

////////////////////// Help: Keyword //////////////////////

// Context help: user either current TexLive's Latex2e help, TexLive's older tex-refs help or Kile LaTeX help
void Help::helpKeyword(KTextEditor::View *view)
{
    QString word = getKeyword(view);
    KILE_DEBUG_MAIN << "keyword: " << word;

    if(!m_helpDir.isEmpty() && !word.isEmpty() && m_dictHelpTex.contains(word)) {
        KILE_DEBUG_MAIN << "about to show help for '" << word << "' (section " << m_dictHelpTex[word] << " )";

        if(m_contextHelpType == HelpLatex2eRefs) {
            showHelpFile(m_latex2eReference + m_dictHelpTex[word]);
        }
        else if ( m_contextHelpType == HelpKileRefs ) {
            showHelpFile(m_kileReference + '#' + m_dictHelpTex[word]);
        }
    }
    else {
        noHelpAvailableFor(word);
    }
}

void Help::noHelpAvailableFor(const QString &word)
{
    m_manager->info()->errorHandler()->printMessage(KileTool::Error, i18n("No help available for %1.", word), i18n("Help"));
}

QString Help::getKeyword(KTextEditor::View *view)
{
    if(!view) {
        return QString();
    }

    // get current position
    int row, col, col1, col2;
    QString word;
    KTextEditor::Document *doc = view->document();
    KTextEditor::Cursor cursor = view->cursorPosition();
    row = cursor.line();
    col = cursor.column();

    if (m_edit->getCurrentWord(doc, row, col, KileDocument::EditorExtension::smTex, word, col1, col2)) {
        // There is no starred keyword in the references. So if     // dani 04.08.2004
        // we find one, we better try the unstarred keyword.
        if(word.right(1) == "*") {
            return word.left(word.length() - 1);
        }
        else {
            return word;
        }
    }
    else {
        return QString();
    }
}

HelpType Help::contextHelpType()
{
    if ( KileConfig::latex2erefs() ) {
        return HelpLatex2eRefs;
    }
    else {
        return HelpKileRefs;
    }
}

//////////////////// read help lists ////////////////////

void Help::readHelpList(const QString &filename)
{
    // clear old map
    m_dictHelpTex.clear();

    QString file = m_helpDir + filename;
    if(file.isEmpty()) {
        KILE_DEBUG_MAIN << "   file not found: " << filename << Qt::endl;
        return;
    }

    KILE_DEBUG_MAIN << "read keyword file: " << file;
    QRegExp reg("\\s*(\\S+)\\s*\\t\\s*(\\S+)");

    QFile f(file);
    if(f.open(QIODevice::ReadOnly)) { // file opened successfully
        QTextStream t(&f);         // use a text stream
        while(!t.atEnd()) { // until end of file...
            QString s = t.readLine().trimmed();       // line of text excluding '\n'
            if(!(s.isEmpty() || s.at(0)=='#')) {
                int pos = reg.indexIn(s);
                if(pos != -1) {
                    m_dictHelpTex[reg.cap(1)] = reg.cap(2);
                }
            }
        }
        f.close();
    }
}

}

