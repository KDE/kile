/***********************************************************************************************
    Copyright (C) 2004 by Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>
                  2008-2012 by Michel Ludwig (michel.ludwig@kdemail.net)
 ***********************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// 2007-03-17 dani
//  - select a single LaTeX command with CTRL+MouseDblClick-left
//    (such a double click on the middle part '\def' of '\abd\def\ghi'
//    will select only '\def\', not the whole text, as it does now)

#include "eventfilter.h"

#include <QMouseEvent>
#include <QKeyEvent>

#include <KModifierKeyInfo>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <qfileinfo.h>

#include "editorextension.h"
#include "kileconfig.h"
#include "kiledebug.h"
#include "kiletoolmanager.h"
#include "kileviewmanager.h"
#include "livepreview.h"

LaTeXEventFilter::LaTeXEventFilter(
    KTextEditor::View *view, KileDocument::EditorExtension *edit,
    KileView::Manager *viewManager,
    KileTool::LivePreviewManager *livePreviewManager,
    KileTool::Manager *toolManager)
    : QObject(view), m_view(view), m_edit(edit), m_viewManager(viewManager),
      m_previewManager(livePreviewManager), m_toolManager(toolManager)
{
    m_modifierKeyInfo = new KModifierKeyInfo(this);
    readConfig();
}

void LaTeXEventFilter::readConfig()
{
    m_bCompleteEnvironment = KileConfig::completeEnvironment();
}

// querying the caps lock state directly is currrently not supported by Qt
bool LaTeXEventFilter::isCapsLockEnabled()
{
    return m_modifierKeyInfo->isKeyLatched(Qt::Key_CapsLock)
           || m_modifierKeyInfo->isKeyLocked(Qt::Key_CapsLock);
}

//FIXME: there should be one central place to convert unicode chars to LaTeX;
//       also see 'EditorExtension::insertLatexFromUnicode'.
// KateViewInternal as a child of KTextEditor::View has the focus
// This was set with KTextEditor::View::setFocusProxy(viewInternal)
bool LaTeXEventFilter::eventFilter(QObject* /* o */, QEvent *e)
{
    // Handles input method events, i.e. multi-key combinations with international keyboard layouts
    if (e->type() == QEvent::InputMethod) {
        QInputMethodEvent *ime = static_cast<QInputMethodEvent*>(e);
        // Only single chars, please. ime->commitString() holds the non-latex unicode character string
        if (ime->commitString().size() == 1) {
            // Extract unicode representation:
            unsigned short rep = ime->commitString().at(0).unicode();
            KILE_DEBUG_MAIN << "string= "<< ime->commitString().at(0) << " dec= "<< rep;
            return m_edit->insertLatexFromUnicode(rep, m_view);
        }
    }

    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(e);
        switch(ke->key())
        {
        case Qt::Key_QuoteDbl:
            return m_edit->insertDoubleQuotes(m_view);
        case Qt::Key_exclamdown:
            return m_edit->insertSpecialCharacter(QStringLiteral("!`"), m_view);
        case Qt::Key_cent:
            return m_edit->insertSpecialCharacter(QStringLiteral("\\textcent"), m_view, QStringLiteral("textcomp"));
        case Qt::Key_sterling:
            return m_edit->insertSpecialCharacter(QStringLiteral("\\pounds"), m_view);
        case Qt::Key_currency:
            return m_edit->insertSpecialCharacter(QStringLiteral("\\textcurrency"), m_view, QStringLiteral("textcomp"));
        case Qt::Key_yen:
            return m_edit->insertSpecialCharacter(QStringLiteral("\\textyen"), m_view, QStringLiteral("textcomp"));
        case Qt::Key_copyright:
            return m_edit->insertSpecialCharacter(QStringLiteral("\\copyright"), m_view);
        case Qt::Key_ordfeminine:
            return m_edit->insertSpecialCharacter(QStringLiteral("\\textordfeminine"), m_view, QStringLiteral("textcomp"));
        case Qt::Key_guillemotleft:
            return m_edit->insertSpecialCharacter(QStringLiteral("\\guillemotleft"), m_view);
        case Qt::Key_notsign:
            return m_edit->insertSpecialCharacter(QStringLiteral("\\neg"), m_view);
        case Qt::Key_registered:
            return m_edit->insertSpecialCharacter(QStringLiteral("\\textregistered"), m_view, QStringLiteral("textcomp"));
        case Qt::Key_degree:
            return m_edit->insertSpecialCharacter(QStringLiteral("^\\circ"), m_view);
        case Qt::Key_plusminus:
            return m_edit->insertSpecialCharacter(QStringLiteral("\\pm"), m_view);
#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
        case Qt::Key_mu:
#else
        case Qt::Key_micro:
#endif
            return m_edit->insertSpecialCharacter(QStringLiteral("\\mu"), m_view);
        case Qt::Key_paragraph:
            return m_edit->insertSpecialCharacter(QStringLiteral("\\P"), m_view);
        case Qt::Key_guillemotright:
            return m_edit->insertSpecialCharacter(QStringLiteral("\\guillemotright"), m_view);
        case Qt::Key_onequarter:
            return m_edit->insertSpecialCharacter(QStringLiteral("\\textonequarter"), m_view, QStringLiteral("textcomp"));
        case Qt::Key_onehalf:
            return m_edit->insertSpecialCharacter(QStringLiteral("\\textonehalf"), m_view, QStringLiteral("textcomp"));
        case Qt::Key_threequarters:
            return m_edit->insertSpecialCharacter(QStringLiteral("\\textthreequarter"), m_view, QStringLiteral("textcomp"));
        case Qt::Key_questiondown:
            return m_edit->insertSpecialCharacter(QStringLiteral("?`"), m_view);
        case Qt::Key_multiply:
            return m_edit->insertSpecialCharacter(QStringLiteral("\\times"), m_view);
        case Qt::Key_ssharp:
            return m_edit->insertSpecialCharacter(QStringLiteral("\\ss{}"), m_view);
        case Qt::Key_Agrave:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\`A"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\`a"), m_view);
        case Qt::Key_Aacute:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\'A"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\'a"), m_view);
        case Qt::Key_Acircumflex:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\^A"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\^a"), m_view);
        case Qt::Key_Atilde:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\~A"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\~a"), m_view);
        case Qt::Key_Adiaeresis:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\\"A"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\\"a"), m_view);
        case Qt::Key_Aring:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\AA"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\aa"), m_view);
        case Qt::Key_AE:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\AE"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\ae"), m_view);
        case Qt::Key_Ccedilla:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\c{C}"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\c{c}"), m_view);
        case Qt::Key_Egrave:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\`E"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\`e"), m_view);
        case Qt::Key_Eacute:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\'E"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\'e"), m_view);
        case Qt::Key_Ecircumflex:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\^E"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\^e"), m_view);
        case Qt::Key_Ediaeresis:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\\"E"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\\"e"), m_view);
        case Qt::Key_Igrave:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\`I"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\`i"), m_view);
        case Qt::Key_Iacute:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\'I"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\'i"), m_view);
        case Qt::Key_Icircumflex:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\^I"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\^i"), m_view);
        case Qt::Key_Idiaeresis:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\\"I"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\\"i"), m_view);
        case Qt::Key_Ntilde:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\~N"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\~n"), m_view);
        case Qt::Key_Ograve:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\`O"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\`o"), m_view);
        case Qt::Key_Oacute:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\'O"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\'o"), m_view);
        case Qt::Key_Ocircumflex:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\^O"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\^o"), m_view);
        case Qt::Key_Otilde:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\~O"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\~o"), m_view);
        case Qt::Key_Odiaeresis:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\\"O"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\\"o"), m_view);
        case Qt::Key_Ugrave:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\`U"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\`u"), m_view);
        case Qt::Key_Uacute:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\'U"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\'u"), m_view);
        case Qt::Key_Ucircumflex:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\^U"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\^u"), m_view);
        case Qt::Key_Udiaeresis:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\\"U"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\\"u"), m_view);
        case Qt::Key_Yacute:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\'Y"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\'y"), m_view);
        case Qt::Key_ydiaeresis:
            if (ke->modifiers() == Qt::ShiftModifier || isCapsLockEnabled()) {
                return m_edit->insertSpecialCharacter(QStringLiteral("\\\"Y"), m_view);
            }
            else return m_edit->insertSpecialCharacter(QStringLiteral("\\\"y"), m_view);
        default:
            break;
        }

        if (m_bCompleteEnvironment && ke->key() == Qt::Key_Return && ke->modifiers() == 0) {
            return m_edit->eventInsertEnvironment(m_view);
        }
    }

    else if(e->type() == QEvent::MouseButtonDblClick) {
        QMouseEvent* me = static_cast<QMouseEvent*>(e);
        if(me->button() == Qt::LeftButton && (me->modifiers() & Qt::ControlModifier)) {
            m_edit->selectWord(KileDocument::EditorExtension::smTex, m_view);
            return true;
        }
    }

    else if(e->type() == QEvent::MouseButtonPress) {
        QMouseEvent* me = static_cast<QMouseEvent*>(e);
        if(me->button() == Qt::LeftButton && (me->modifiers() & KileConfig::envEventSelectModifier())) { // select from cursor
            me->setModifiers(Qt::ShiftModifier);
            return false; // Pass Shift-click on to KatePart / KTextEditor
        }
        else if(me->button() == Qt::LeftButton && (me->modifiers() & KileConfig::envEventSearchModifier())) { // forward search
            return doForwardSearch(me->globalPosition().toPoint());
        }
    }

    // pass this event on
    return false;
}

// Do a forward search in the viewer based upon editor position
bool LaTeXEventFilter::doForwardSearch(const QPoint& point)
{
    // change the cursor position first
    // use global coordinates to correctly handle possible icon borders, line
    // numbers, ...
    m_view->setCursorPosition(m_view->coordinatesToCursor(m_view->mapFromGlobal(point)));
    // live preview
    if(m_previewManager->isLivePreviewEnabledForCurrentDocument()) {
        m_previewManager->showCursorPositionInDocumentViewer();
        return true;
    }
    // generated PDF or DVI file
    KileTool::Base* forwardPdfTool = m_toolManager->createTool(QLatin1String("ForwardPDF"));
    if(!forwardPdfTool) { // just to be avoid crashing as this shouldn't
                           // happen
        return false;
    }
    forwardPdfTool->prepareToRun();
    QFile pdfFile(forwardPdfTool->targetDir() + QLatin1Char('/') + forwardPdfTool->target());

    KileTool::Base* forwardDviTool = m_toolManager->createTool(QLatin1String("ForwardDVI"));
    if(!forwardDviTool) { // just to be avoid crashing as this shouldn't
                           // happen
        return false;
    }
    forwardDviTool->prepareToRun();
    QFile dviFile(forwardDviTool->targetDir() + QLatin1Char('/') + forwardDviTool->target());

    if(QFileInfo(pdfFile).exists() && QFileInfo(dviFile).exists()) {
        if(QFileInfo(pdfFile).lastModified() > QFileInfo(dviFile).lastModified()) {
            delete forwardDviTool;
            m_toolManager->run(forwardPdfTool);
            return true;
        }
        else {
            delete forwardPdfTool;
            m_toolManager->run(forwardDviTool);
            return true;
        }
    }
    else if(QFileInfo(pdfFile).exists()) {
        delete forwardDviTool;
        m_toolManager->run(forwardPdfTool);
        return true;
    }
    else if(QFileInfo(dviFile).exists()) {
        delete forwardPdfTool;
        m_toolManager->run(forwardDviTool);
        return true;
    }
    return false;
}
