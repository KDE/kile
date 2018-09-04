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

#ifndef EDITORCOMMANDS_H
#define EDITORCOMMANDS_H

#include <QStringList>

#include <KTextEditor/Command>
#include <KTextEditor/Editor>
#include <KTextEditor/View>

class KileInfo;

/**
 * Implements bindings for commands like 'w', 'q', etc. for the VI input mode of KatePart.
 **/
class EditorCommands : public KTextEditor::Command {
public:
    explicit EditorCommands(KileInfo *info);
    virtual ~EditorCommands();

    virtual bool exec(KTextEditor::View *view, const QString &cmd, QString &msg,
                      const KTextEditor::Range &range = KTextEditor::Range::invalid()) Q_DECL_OVERRIDE;
    virtual bool help(KTextEditor::View *view, const QString &cmd, QString &msg) Q_DECL_OVERRIDE;

private:
    KileInfo *m_ki;
    QStringList m_commandList;
};

#endif
