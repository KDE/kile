/***************************************************************************
    date                 : Feb 15 2007
    version              : 0.34
    copyright            : (C) 2005-2007 by Holger Danielsson
    email                : holger.danielsson@versanet.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QUICKPREVIEW_H
#define QUICKPREVIEW_H

#include "kileinfo.h"
#include "kiletool.h"
#include "editorextension.h"
#include "widgets/previewwidget.h"

#include <QObject>
#include <QString>
#include <QStringList>

#include <QTemporaryDir>

namespace KileTool
{
enum { qpSelection=0, qpEnvironment, qpSubdocument, qpMathgroup };

class QuickPreview : public QObject
{
    Q_OBJECT

public:
    QuickPreview(KileInfo *ki);
    ~QuickPreview();

    bool run(const QString &text,const QString &textfilename,int startrow);
    bool isRunning();

    void previewEnvironment(KTextEditor::Document *doc);
    void previewSelection(KTextEditor::View *view, bool previewInWidgetConfig=true);
    void previewSubdocument(KTextEditor::Document *doc);
    void previewMathgroup(KTextEditor::Document *doc);

    /**
     * run (text, textfilename, startrow) works with the
     * default configuration for QuickPreview. This method
     * supports a forth parameter to choose the configuration as
     * comma - separated string as you can see them in run (text, textfilename, startrow)
     *
     * It is also possible not to specify a viewer, so the viewer is not
     * executed.
     *
     * @param text         Text to preview
     * @param textfilename Filename of the document
     * @param startrow     Position of preview text in the document
     * @param spreviewlist user-defined configuration, e.g. "PreviewLaTeX,DVItoPS,,,ps" (with no preview)
     * @return             true if method succeeds, else false
     */
    bool run (const QString &text, const QString &textfilename, int startrow, const QString &spreviewlist);
    void getTaskList(QStringList &tasklist);

    /**
     * QuickPreview uses temporary files for processing the output.
     * If you want to work with files from QuickPreview, you
     * can use this method. The method run returns true, and then
     * you can get the generated ps e.g. with getPreviewFile ("eps");
     * It works with all extensions which are generated while running
     * the corresponding tools (e.g. tex, dvi, ps, pdf, ...)
     *
     * @param  extension defines which file to use
     * @return           The temporary file with extension
     */
    QString getPreviewFile(const QString &extension);

private Q_SLOTS:
    void toolDestroyed();

private:
    enum { pvLatex=0, pvDvips=1, pvDvipsCfg=2, pvViewer=3, pvViewerCfg=4, pvExtension=5 };

    KileInfo *m_ki;
    QString m_tempFile;
    QStringList m_taskList;
    int m_running;
    QTemporaryDir *m_tempDir;

    int createTempfile(const QString &text);
    void showError(const QString &text);
};

}

#endif
