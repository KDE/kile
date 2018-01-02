/**************************************************************************
*   Copyright (C) 2010 by Michel Ludwig (michel.ludwig@kdemail.net)       *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "editorcommands.h"

#include <QTimer>

#include "kiledocmanager.h"
#include "kileinfo.h"

EditorCommands::EditorCommands(KileInfo *info)
    : KTextEditor::Command(QStringList() << "w" << "wa" << "wq" << "q" << "wqa")
    , m_ki(info)
{
}

EditorCommands::~EditorCommands()
{
}

bool EditorCommands::exec(KTextEditor::View *view, const QString &cmd, QString &msg,
                          const KTextEditor::Range &range)
{
    Q_UNUSED(range)

    if(cmd == "w" || cmd == "wa") {
        if(cmd == "wa") {
            bool result = m_ki->docManager()->fileSaveAll();
            msg = result ? i18n("All documents saved to disk.")
                  : i18n("Saving of all documents failed.");
            return result;
        }
        else {
            bool result = m_ki->docManager()->fileSave(view);
            msg = result ? i18n("Document saved to disk.")
                  : i18n("Saving document failed.");
            return result;
        }
    }
    else if(cmd == "q" || cmd == "wq" || cmd == "wqa") {
        if(cmd == "wq" || cmd == "wqa") {
            bool result = true;
            if(cmd == "wq") {
                result = m_ki->docManager()->fileSave(view);
            }
            else {
                result = m_ki->docManager()->fileSaveAll();
            }
            if(!result) {
                msg = i18n("Saving failed and quitting canceled.");
                return false;
            }
        }
        QTimer::singleShot(0, m_ki->mainWindow(), SLOT(close()));
        return true;
    }

    return false;
}

bool EditorCommands::help(KTextEditor::View *view, const QString &cmd, QString &msg)
{
    Q_UNUSED(view);

    if(cmd == "w" || cmd == "wa") {
        msg = "<p><b>w/wa</b>: Save document(s) to disk.</p>"
              "<p><b>w</b> only saves the current document, whereas "
              "<b>wa</b> saves all the documents.</p>";
        return true;
    }
    else if(cmd == "q" || cmd == "wq" || cmd == "wqa") {
        msg = "<p><b>q/wq/wqa</b>: Quit Kile</p>"
              "<p><b>wq</b> additionally saves the current document to disk "
              "before quitting, whereas <b>wqa</b> saves all the documents "
              "before exiting.</p>";
        return true;
    }

    return false;
}
