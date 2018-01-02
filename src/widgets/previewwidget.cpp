/*****************************************************************************
*   Copyright (C) 2006 by Mathias Soeken (msoeken@informatik.uni-bremen.de)  *
*                        (orginal version of this preview)                   *
*             (C) 2011 by Michel Ludwig (michel.ludwig@kdemail.net)          *
******************************************************************************/

// dani/2006:
//  - signal/slot communication
//  - user-defined resolution of the png image
//  - add '%res' to the dictionary of KileTools
//  - install three possible conversion methods: dvipng, dvips/convert for documents
//    with postscript source and convert for source, which needs the pdftex driver
//  - preview configuration dialog

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "widgets/previewwidget.h"

#include <QImage>
#include <QLabel>
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QScrollArea>

#include <KLocalizedString>

#include "editorextension.h"
#include "errorhandler.h"
#include "kileconfig.h"
#include "kileinfo.h"
#include "kileviewmanager.h"
#include "kiletool.h"
#include "kiletool_enums.h"
#include "quickpreview.h"

namespace KileWidget
{

// We can't use signals/slots in this class as the moc doesn't parse it.
// Also, we better keep the declaration and implementation separate as
// we might have to move it back at some point.
class ImageDisplayWidget : public QWidget
{
public:
    ImageDisplayWidget(QWidget *parent);
    virtual ~ImageDisplayWidget();

    void clear();
    void setImageFile(const QString& fileName);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QImage *m_image;
};

ImageDisplayWidget::ImageDisplayWidget(QWidget *parent)
    : QWidget(parent),
      m_image(Q_NULLPTR)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

ImageDisplayWidget::~ImageDisplayWidget()
{
    delete m_image;
}

void ImageDisplayWidget::paintEvent(QPaintEvent * /* event */)
{
    QPainter p(this);
    // draw the background first
    p.fillRect(0, 0, width(), height(), KileConfig::previewPaneBackgroundColor());
    // and then the image
    if(m_image) {
        p.drawImage(3, 3, *m_image);
    }
}

void ImageDisplayWidget::clear()
{
    delete m_image;
    m_image = Q_NULLPTR;
    setMinimumSize(0, 0);
    repaint();
}

void ImageDisplayWidget::setImageFile(const QString& fileName)
{
    if(!m_image) {
        delete m_image;
    }

    m_image = new QImage(fileName);
    setMinimumSize(m_image->width() + 6, m_image->height() + 6);
    repaint();
}

PreviewWidget::PreviewWidget(KileInfo *info, QWidget *parent, const char *name)
    : QScrollArea(parent), m_info(info), m_running(false)
{
    setObjectName(name);
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setWidgetResizable(true);

    m_imageDisplayWidget = new ImageDisplayWidget(this);
    setWidget(m_imageDisplayWidget);
}


PreviewWidget::~PreviewWidget()
{
}

void PreviewWidget::showActivePreview(const QString &text,const QString &textfilename,int startrow,int previewtype)
{
    KILE_DEBUG_MAIN << "==PreviewWidget::showActivePreview()==========================";
    m_info->errorHandler()->clearMessages();
    if(m_running || m_info->quickPreview()->isRunning()) {
        showError( i18n("There is already a preview running that has to be finished to run this one.") );
        return;
    }

    // determine the type of conversion
    int conversiontype;
    switch(previewtype) {
    case KileTool::qpSelection:
        conversiontype = KileConfig::selPreviewTool();
        break;
    case KileTool::qpEnvironment:
        conversiontype = KileConfig::envPreviewTool();
        break;
    case KileTool::qpMathgroup:
        conversiontype = KileConfig::mathgroupPreviewTool();
        break;
    default: // should not happen
        conversiontype = pwDvipng;
        break;
    }


    // set parameter for these tools
    QString tasklist, tool, toolcfg, extension;
    if(conversiontype == pwConvert) {
        m_conversionTool = "convert";
        tasklist = "PreviewPDFLaTeX,,,,,png";
        tool = "Convert";
        toolcfg = "pdf2png";
        extension = "pdf";
    }
    else if(conversiontype == pwDvipsConvert) {
        m_conversionTool = "dvips/convert";
        tasklist = "PreviewLaTeX,DVItoPS,dvi2eps,,,png";
        tool = "Convert";
        toolcfg = "eps2png";
        extension = "eps";
    }
    else {
        m_conversionTool = "dvipng";
        tasklist = "PreviewLaTeX,,,,,png";
        tool = "DVItoPNG";
        toolcfg.clear();
        extension = "dvi";
    }

    if(!m_info->quickPreview()->run(text, textfilename, startrow, tasklist)) {
        return;
    }

    KileTool::Base *pngConverter = m_info->toolManager()->createTool(tool, toolcfg);
    if(!pngConverter) {
        showError(i18n("Could not run '%1' for QuickPreview.", tool));
        return;
    }
    pngConverter->setSource(m_info->quickPreview()->getPreviewFile(extension));

    // First, we have to disconnect the old done() signal, because this is
    // passed immediately to the toolmanager, whichs destroys the tool. This
    // means, that all connections, which are done later, will never been called.
    disconnect(pngConverter, SIGNAL(done(KileTool::Base*,int,bool)), m_info->toolManager(), SLOT(done(KileTool::Base*,int)));

    // Now we make some new connections, which are called in this sequence:
    // 1) when the tool is finished, the preview will be shown
    // 2) then the done() signal can be passed to the toolmanager,
    //    which destroys the tool
    connect(pngConverter, SIGNAL(done(KileTool::Base*,int,bool)), this, SLOT(drawImage()));
    connect(pngConverter, SIGNAL(done(KileTool::Base*,int,bool)), m_info->toolManager(), SLOT(done(KileTool::Base*,int)));

    // Finally we will send a signal, which will pass the focus from the log window
    // to the formula preview (dvipng --> toolmanager --> kile)
    //
    // Remark:
    // It's also possible to use only (!) the destroyed() signal. This must be sent
    // to the toolmanager, which passes it over to the kile object. This object can
    // call drawImage() and after it, we have to set the focus to the preview widget.
    // This can only be done from the kile object, which explains this indirect way.
    //
    // But i (dani) prefer the chosen way above, because
    //  - we can distinguish between drawImage() and focusPreview(), which may be
    //    important some time
    //  - it is more complicated
    connect(pngConverter, SIGNAL(destroyed()), m_info->toolManager(), SIGNAL(previewDone()));
    connect(pngConverter, SIGNAL(destroyed()), this, SLOT(toolDestroyed()));

    // Now we are ready to start the process...
    m_info->toolManager()->run(pngConverter);
    m_running = true;
}

void PreviewWidget::clear()
{
    m_imageDisplayWidget->clear();
}

void PreviewWidget::drawImage()
{
    KILE_DEBUG_MAIN << "\tconversion tool '" << m_conversionTool << "' done, processing file (by dani)";
    m_imageDisplayWidget->setImageFile(m_info->quickPreview()->getPreviewFile ("png"));
}

void PreviewWidget::toolDestroyed()
{
    KILE_DEBUG_MAIN << "\tQuickPreview: tool destroyed";
    m_running = false;
}

void PreviewWidget::showError(const QString &text)
{
    m_info->errorHandler()->printMessage(KileTool::Error, text, i18n("QuickPreview"));
}

}

