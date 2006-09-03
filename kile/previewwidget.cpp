//
// C++ Implementation: previewwidget
//
// Description: 
//
//
// Author: Mathias Soeken <msoeken@informatik.uni-bremen.de>, (C) 2006
//
// dani/2006:
//  - signal/slot communication
//  - user defined resolution of the png image
//  - add '%res' to the dictionary of KileTools
//  - preview configuration dialog
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <qimage.h>
#include <qpainter.h>
#include <qpalette.h>

#include <klocale.h>
#include <kmessagebox.h>

#include <kate/view.h>
#include <kate/document.h>

#include "kileconfig.h"
#include "kileedit.h"
#include "kileinfo.h"
#include "kileviewmanager.h"
#include "kiletool.h"
#include "quickpreview.h"
#include "previewwidget.h"

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
	if ( m_previewImage != NULL )
		p.drawImage(0, 0, *m_previewImage);
}

void PreviewWidget::showActivePreview() 
{
	Kate::View *view = m_info->viewManager()->currentView();
	if (!view) return;

	kdDebug() << "==PreviewWidget::showActivePreview()=========================="  << endl;
	if ( m_running )
	{
		KMessageBox::error( 0,i18n("There is already a preview running, which you have to finish to run this one.") );
		return;
	}

	Kate::Document *doc = view->getDoc();
	if ( doc ) 
	{
		uint row, col;
		QString text = m_info->editorExtension()->getMathgroupText(row,col,view);
		if (text != QString::null) 
		{
			m_running = true;
			if ( m_info->quickPreview()->run(text, m_info->getName(doc), row, "PreviewLaTeX,,,,png")) 
			{
				KileTool::Base *dvipng = m_info->toolFactory()->create("DVItoPNG");
				if ( ! dvipng ) 
				{
					KMessageBox::error(this, QString(i18n("Could not run '%1' for QuickPreview.").arg("DVItoPNG")));
					return;
				}
				dvipng->setSource(m_info->quickPreview()->getPreviewFile("dvi"));
	
				// First, we have to disconnect the old done() signal, because this is 
				// passed immediately to the toolmanager, whichs destroys the tool. This
				// means, that all connections, which are done later, will never been called.
				disconnect(dvipng, SIGNAL(done(Base*,int)), m_info->toolManager(), SLOT(done(Base*,int)));
			
				// Now we make some new connections, which are called in this sequence:
				// 1) when the tool is finished, the preview will be shown
				// 2) then the done() signal can be passed to the toolmanager,
				//    which destroys the tool
				connect(dvipng, SIGNAL(done (Base*,int)), this, SLOT(drawImage()));
				connect(dvipng, SIGNAL(done(Base*,int)), m_info->toolManager(), SLOT(done(Base*,int)));

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
				connect(dvipng, SIGNAL(destroyed()), m_info->toolManager(), SIGNAL(previewDone()));

				// Now we are ready to start the process...
				m_info->toolManager()->run(dvipng);
			}
			else
				m_running = false;
		}
	}
}

void PreviewWidget::drawImage() 
{
	kdDebug() << "\tDVItoPNG tool done, processing file (by dani)" << endl;
	if ( ! m_previewImage )
		delete m_previewImage;

	m_previewImage = new QImage (m_info->quickPreview()->getPreviewFile ("png"));
	setFixedSize( m_previewImage->size() );

	repaint ();
	m_running = false;
}

}

#include "previewwidget.moc"
