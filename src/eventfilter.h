/***********************************************************************************************
    Copyright (C) 2004 by Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>
                  2008-2019 by Michel Ludwig (michel.ludwig@kdemail.net)
 ***********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef EVENTFILTER_H
#define EVENTFILTER_H

#include "kiletoolmanager.h"
#include "kileviewmanager.h"
#include "livepreview.h"
#include <QEvent>
#include <QObject>

class KModifierKeyInfo;

namespace KTextEditor {
class View;
}
namespace KileDocument {
class EditorExtension;
}

/**
 * This class is capable of intercepting key-strokes from the editor. It can complete a \begin{env}
 * with a \end{env} when enter is pressed.
 **/
class LaTeXEventFilter : public QObject
{
    Q_OBJECT

public:
    LaTeXEventFilter(KTextEditor::View *view, 
                    KileDocument::EditorExtension *edit,
                    KileView::Manager *viewManager,
                    KileTool::LivePreviewManager *previewManager,
                    KileTool::Manager *toolManager);

public Q_SLOTS:
    void readConfig();

protected:
    bool eventFilter(QObject *o, QEvent *e) override;
    bool isCapsLockEnabled();

private:
    bool m_bCompleteEnvironment;
    KTextEditor::View *m_view;
    KileDocument::EditorExtension *m_edit;
    KileView::Manager *m_viewManager;
    KileTool::LivePreviewManager *m_previewManager;
    KileTool::Manager *m_toolManager;
    KModifierKeyInfo *m_modifierKeyInfo;

};

#endif
