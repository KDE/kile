/***************************************************************************
    date                 : Feb 18 2005
    version              : 0.12
    copyright            : (C) 2005 by Holger Danielsson
    email                : holger.danielsson@t-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "quickpreview.h"
#include "kiletool.h"
#include "kiledocmanager.h"
#include "kilelauncher.h"

#include <qtextstream.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qmap.h>

#include <kdebug.h>
#include <ktempdir.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kconfig.h>
#include <kileconfig.h>

namespace KileTool
{

QuickPreview::QuickPreview(KileInfo *ki) : m_ki(ki), m_tempfile(QString::null)
{
	m_taskList << i18n("LaTeX ---> DVI")
	           << i18n("LaTeX ---> DVI (KDVI)")
	           << i18n("LaTeX ---> PS")
	           << i18n("LaTeX ---> PS (KGhostView)")
	           << i18n("PDFLaTeX ---> PDF")
	           << i18n("PDFLaTeX ---> PDF (KGhostView)")
	           << i18n("PDFLaTeX ---> PDF (KPDF)")
	           ;
}

QuickPreview::~QuickPreview() 
{
	removeTempFiles(true);
}

void QuickPreview::getTaskList(QStringList &tasklist)
{
	tasklist.clear();
	tasklist << "Tool/ViewDVI/Embedded Viewer=" + m_taskList[0]
	         << "Tool/ViewDVI/KDVI Unique="     + m_taskList[1]
	         << "Tool/ViewPS/Embedded Viewer="  + m_taskList[2]
	         << "Tool/ViewPS/KGhostView="       + m_taskList[3]
	         << "Tool/ViewPDF/Embedded Viewer=" + m_taskList[4]
	         << "Tool/ViewPDF/KGhostView="      + m_taskList[5]
	         << "Tool/ViewPDF/KPDF="            + m_taskList[6]
	         ;
}

void QuickPreview::run(const QString &text,const QString &textfilename,int startrow) 
{
	kdDebug() << "==QuickPreview::run()=========================="  << endl;
	//m_ki->docManager()->fileSaveAll();
	
	// check if there is something to compile
	if ( text.isEmpty() ) {
		showError(i18n("There is nothing to compile and preview."));
		return;
	}
	
	// create the name of a temporary file or delete already existing temporary files
	if ( m_tempfile.isEmpty() ) {
		m_tempfile =  KTempDir(QString::null).name() + "preview.tex";
		kdDebug() << "\tdefine tempfile: " << m_tempfile << endl;
	} else {
		removeTempFiles();
	}      
	
	// create the temporary file with preamble and text
	int preamblelines = createTempfile(text);
	if ( preamblelines == 0 )
		return;
	
	// define possible tools
	QMap <QString,QString> map;
	map[m_taskList[0]] = "PreviewLaTeX,,ViewDVI,Embedded Viewer,dvi"; 
	map[m_taskList[1]] = "PreviewLaTeX,,ViewDVI,KDVI Unique,dvi";
	map[m_taskList[2]] = "PreviewLaTeX,DVItoPS,ViewPS,Embedded Viewer,ps";
	map[m_taskList[3]] = "PreviewLaTeX,DVItoPS,ViewPS,KGhostView,ps";
	map[m_taskList[4]] = "PreviewPDFLaTeX,,ViewPDF,Embedded Viewer,pdf"; 
	map[m_taskList[5]] = "PreviewPDFLaTeX,,ViewPDF,KGhostView,pdf";
	map[m_taskList[6]] = "PreviewPDFLaTeX,,ViewPDF,KPDF,pdf";
	
	QString previewtask = KileConfig::previewTask();
	if ( ! map.contains(previewtask) ) {
		showError(QString(i18n("Could not run QuickPreview:\nunknown task '%1'").arg(previewtask)));
		return;
	}
	QStringList previewlist = QStringList::split(",",map[previewtask],true);
	
	// create preview tools 
	kdDebug() << "\tcreate latex tool for QuickPreview: "  << previewlist[pvLatex] << endl;
	KileTool::PreviewLaTeX *latex = (KileTool::PreviewLaTeX  *)m_ki->toolFactory()->create(previewlist[pvLatex],false);
	if ( !latex ) {
		showError(QString(i18n("Could not run '%1' for QuickPreview.").arg("LaTeX")));
		return;
	}
	
	KileTool::Base *dvips = 0L;
	if ( ! previewlist[1].isEmpty() ) {
		kdDebug() << "\tcreate dvips tool for QuickPreview: "  << previewlist[pvDvips] << endl;
		dvips = m_ki->toolFactory()->create(previewlist[pvDvips]);
		if ( !dvips ) {
			showError(QString(i18n("Could not run '%1' for QuickPreview.").arg("DVItoPS")));
			return;
		}
	}

	QString viewertool = previewlist[pvViewer] + " (" + previewlist[pvViewerCfg] + ")";
	kdDebug() << "\tcreate viewer for QuickPreview: "  << viewertool << endl;
	KileTool::Base *viewer = m_ki->toolFactory()->create(previewlist[pvViewer],false);
	if ( !viewer ) {
		showError(QString(i18n("Could not run '%1' for QuickPreview.").arg(viewertool)));
		return;
	}
	
	// update value of texinput path 
	QString texinputpath = KileConfig::teXPaths();
	QString inputdir = QFileInfo(m_ki->getCompileName()).dirPath(true);
	if ( ! texinputpath.isEmpty() )
		inputdir += ":" + texinputpath;
 	KileConfig::setTeXPaths(inputdir);
	
	// prepare tools
	QString filepath = m_tempfile.left( m_tempfile.length()-3 ); 
	latex->setPreviewInfo(textfilename,startrow,preamblelines+1);
	latex->setSource(m_tempfile);
	latex->prepareToRun();
	
	if ( dvips )
		dvips->setSource( filepath + "dvi" );
	viewer->setSource( filepath + previewlist[pvExtension] );
	
	// run tools
	m_ki->toolManager()->run(latex);
	if ( dvips ) 
		m_ki->toolManager()->run(dvips);
	m_ki->toolManager()->run(viewer,previewlist[pvViewerCfg]);
	
	// restore old texinputpath
	KileConfig::setTeXPaths(texinputpath);
}

int QuickPreview::createTempfile(const QString &text)
{
	// determine main document to read the preamble
	QString filename = m_ki->getCompileName();
	if ( filename.isEmpty() ) {
		showError(i18n("Could not determine the main document."));
		return 0;
	}
	
	// open to read
	QFile fin( filename );
	if ( !fin.exists() || !fin.open(IO_ReadOnly) ) {
		showError(i18n("Could not read the preamble."));
		return 0;
	}
	kdDebug() << "\tcreate a temporary file: "  << m_tempfile << endl;
	
	// use a textstream
	QTextStream preamble(&fin);

	// create the temporary file 
	QFile tempfile(m_tempfile);
	if ( ! tempfile.open( IO_WriteOnly ) ) {
		showError(i18n("Could not create a temporary file."));
		return 0;
	}
	QTextStream stream( &tempfile );
	
	// set the encoding of the preview file to the encoding of the original latex file and not to the system standard (tbraun)
	 stream.setCodec(preamble.codec());
	
	// write the whole preamble into this temporary file
	QString textline;
	int preamblelines = 0;
	bool begindocumentFound = false;
	while ( ! preamble.eof() ) {
		textline = preamble.readLine();
		if ( textline.find("\\begin{document}") >= 0 ) {
			begindocumentFound = true;
			break;
		}
		stream << textline << "\n";
		preamblelines++;
	}

	// look if we found '\begin{document}' to finish the preamble
	if ( ! begindocumentFound ) {
		tempfile.close();
		showError(i18n("Could not find a '\\begin{document}' command."));
		return 0;
	}

	// add the text to compile
	stream << "\\begin{document}\n";
	stream << text;                 
	stream << "\n\\end{document}\n";
	tempfile.close();
	
	return preamblelines;
}

void QuickPreview::removeTempFiles(bool rmdir)
{
	if ( m_tempfile.isEmpty() ) 
		return;
		
	QFileInfo fi(m_tempfile);
	QString tempdir = fi.dirPath(true) + '/';
	
	QDir dir = fi.dir(true); 
	if ( dir.exists() ) {                  
		QStringList list = dir.entryList(fi.baseName()+".*");
		for ( QStringList::Iterator it=list.begin(); it!=list.end(); ++it ) {
			QFile::remove( tempdir + (*it) );
			// kdDebug() << "\tremove temporary file: " << tempdir + (*it) << endl;
		}
		
		if ( rmdir )
			dir.rmdir(tempdir);
	}
}

void QuickPreview::showError(const QString &text)
{
	KMessageBox::error(0,text);
}


}

