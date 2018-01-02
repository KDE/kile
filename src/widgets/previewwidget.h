/*****************************************************************************
*   Copyright (C) 2006 by Mathias Soeken (msoeken@informatik.uni-bremen.de)  *
*             (C) 2011 by Michel Ludwig (michel.ludwig@kdemail.net)          *
******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QScrollArea>

class QImage;
class QLabel;

class KileInfo;

namespace KileTool {
class Base;
}

namespace KileWidget
{

// declared in the .cpp file
class ImageDisplayWidget;

/**
 * Widget which can display PNG images from Math LaTeX
 * Code
 *
 * This is used to be inserted in the bottom bar of the kile
 * main widget. When putting the cursor in a mathgroup, the LaTeX
 * source should be extracted and rendered in this widget.
 *
 * This widget uses one new Tool: DVItoPNG which converts
 * a dvi file to an png image, which is fitted to the size
 * of the formula).
 *
 * You could use this widget to implement a formula editor in Kile,
 * where the editor is the editor, but with this widget you can see
 * the result in a appropriate size just in time.
 *
 * @author Mathias Soeken <msoeken@informatik.uni-bremen.de>
 */

class PreviewWidget : public QScrollArea
{
    Q_OBJECT

public:
    explicit PreviewWidget(KileInfo *info, QWidget *parent = 0, const char *name = 0);
    ~PreviewWidget();

    /**
     * Tries to paint the current mathgroup of
     * the current document.
     *
     * If a document is open and the cursor is
     * inside a mathgroup, a PNG is generated
     * containing this mathgroup.
     *
     *  This PNG image is then displayed on the
     * widget.
     **/
    void showActivePreview(const QString &text, const QString &textfilename, int startrow, int previewtype);

public Q_SLOTS:
    void clear();

private:
    enum { pwDvipng=0, pwDvipsConvert, pwConvert };

    KileInfo *m_info;
    ImageDisplayWidget *m_imageDisplayWidget;
    bool m_running;
    QString m_conversionTool;

protected:
    void showError(const QString &text);

public Q_SLOTS:
    /**
     * Notify, if the DVItoPNG tool is done.
     *
     * Because the tool runs async. we
     * must wait, if the process is done.
     *
     * Then we try to generate a image of the
     * temporary PNG filename and display it on
     * the widget.
     *
     * The size of the widget is also adjusted
     * to the size of the widget.
     **/
    void drawImage();
    void toolDestroyed();
};

}

#endif
