//
// C++ Implementation: previewwidget
//
// Description: 
//
// Author: Mathias Soeken <msoeken@informatik.uni-bremen.de>, (C) 2006
//         (orginal version of this preview)
//
// dani/2006:
//  - signal/slot communication
//  - user defined resolution of the png image
//  - add '%res' to the dictionary of KileTools
//  - install three possible conversion methods: dvipng, dvips/convert for documents
//    with postscript source and convert for source, which needs the pdftex driver
//  - preview configuration dialog
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "previewwidget.h"

#include <qimage.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qwidget.h>
//Added by qt3to4:
#include <QPaintEvent>

#include <klocale.h>
#include <kate/view.h>
#include <kate/document.h>

#include "kileconfig.h"
#include "kileedit.h"
#include "kileinfo.h"
#include "kileviewmanager.h"
#include "kilelogwidget.h"
#include "kiletool.h"
#include "kiletool_enums.h"
#include "quickpreview.h"

namespace KileWidget 
{

PreviewWidget::PreviewWidget(KileInfo *info, QWidget *parent, const char *name) 
	: QWidget(parent,name), m_info(info), m_previewImage(0L), m_running(false) 
{
	setPalette( QPalette(QColor(0xff,0xff,0xff)) );
}


PreviewWidget::~PreviewWidget() 
{
	delete m_previewImage;
}

void PreviewWidget::paintEvent(QPaintEvent*) 
{
	QPainter p(this);
	if ( m_previewImage )
		p.drawImage(3,3,*m_previewImage);
}

void PreviewWidget::showActivePreview(const QString &text,const QString &textfilename,int startrow,int previewtype)
{
	KILE_DEBUG() << "==PreviewWidget::showActivePreview()=========================="  << endl;
	m_info->logWidget()->clear();
	if ( m_running || m_info->quickPreview()->isRunning() )
	{
		showError( i18n("There is already a preview running, which you have to finish to run this one.") );
		return;
	}

	// determine the type of conversion
	int conversiontype;
	switch ( previewtype )
	{
		case KileTool::qpSelection:   conversiontype = KileConfig::selPreviewTool(); break;
		case KileTool::qpEnvironment: conversiontype = KileConfig::envPreviewTool(); break;
		default:                      conversiontype = pwDvipng;                     break;	
	}


	// set parameter for these tools
	QString tasklist,tool,toolcfg,extension;
	if (conversiontype == pwConvert )
	{
		m_conversionTool = "convert";
		tasklist = "PreviewPDFLaTeX,,,,,png";
		tool = "Convert";
		toolcfg = "pdf2png";
		extension = "pdf";
	}
	else if (conversiontype == pwDvipsConvert )
	{
		m_conversionTool = "dvips/convert";
		tasklist = "PreviewLaTeX,DVItoPS,dvi2eps,,,png";
		tool = "Convert";
		toolcfg = "eps2png";
		extension = "eps";
	}
	else
	{
		m_conversionTool = "dvipng";
		tasklist = "PreviewLaTeX,,,,,png";
		tool = "DVItoPNG";
		toolcfg = QString::null;
		extension = "dvi";
	}

	if ( ! m_info->quickPreview()->run(text, textfilename, startrow, tasklist) )
		return;
 
	KileTool::Base *pngConverter = m_info->toolFactory()->create(tool);
	if ( ! pngConverter ) 
	{
		showError( i18n("Could not run '%1' for QuickPreview.",tool) );
		return;
	}
	pngConverter->setSource(m_info->quickPreview()->getPreviewFile(extension));

	// First, we have to disconnect the old done() signal, because this is 
	// passed immediately to the toolmanager, whichs destroys the tool. This
	// means, that all connections, which are done later, will never been called.
	disconnect(pngConverter, SIGNAL(done(Base*,int)), m_info->toolManager(), SLOT(done(Base*,int)));

	// Now we make some new connections, which are called in this sequence:
	// 1) when the tool is finished, the preview will be shown
	// 2) then the done() signal can be passed to the toolmanager,
	//    which destroys the tool
	connect(pngConverter, SIGNAL(done (Base*,int)), this, SLOT(drawImage()));
	connect(pngConverter, SIGNAL(done(Base*,int)), m_info->toolManager(), SLOT(done(Base*,int)));

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
	if ( m_info->toolManager()->run(pngConverter,toolcfg) == KileTool::Running )
		m_running = true;
}

void PreviewWidget::drawImage() 
{
	KILE_DEBUG() << "\tconversion tool '" << m_conversionTool << "' done, processing file (by dani)" << endl;
	if ( ! m_previewImage )
		delete m_previewImage;

	m_previewImage = new QImage (m_info->quickPreview()->getPreviewFile ("png"));
	setFixedSize( m_previewImage->width()+6,m_previewImage->height()+6 );

	repaint ();
}

void PreviewWidget::toolDestroyed() 
{
	KILE_DEBUG() << "\tQuickPreview: tool destroyed" << endl;
	m_running = false;
}

void PreviewWidget::showError(const QString &text)
{
	m_info->logWidget()->printMsg( KileTool::Error, text, i18n("QuickPreview") );
}

}

#include "previewwidget.moc"
