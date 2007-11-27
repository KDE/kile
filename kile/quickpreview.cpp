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

#include "quickpreview.h"
#include "kiletool_enums.h"
#include "kiledocmanager.h"
#include "kilelogwidget.h"

#include <q3textstream.h>
#include <qtextcodec.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qmap.h>

#include "kiledebug.h"
#include <ktempdir.h>
#include <klocale.h>
#include <kconfig.h>
#include <kileconfig.h>
#include <kate/document.h>

namespace KileTool
{

QuickPreview::QuickPreview(KileInfo *ki) : m_ki(ki), m_tempfile(QString::null), m_running(0)
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

//////////////////// quick preview ////////////////////

// compile and view current selection (singlemode and mastermode)

void QuickPreview::previewSelection(Kate::Document *doc, bool previewInWidgetConfig)
{
	if ( doc->hasSelection() ) 
	{
		if ( previewInWidgetConfig && KileConfig::selPreviewInWidget() )
		{
			m_ki->previewWidget()->showActivePreview( doc->selection(),m_ki->getName(doc),doc->selStartLine(),KileTool::qpSelection );
		}
		else
		{	
			run( doc->selection(),m_ki->getName(doc),doc->selStartLine() );
			doc->clearSelection();
		}
	} 
	else 
	{
		showError( i18n("There is no selection to compile.") );
	}
}

// compile and view current environment (singlemode and mastermode)

void QuickPreview::previewEnvironment(Kate::Document *doc)
{
	uint row,col;
	QString envname;
	QString text = m_ki->editorExtension()->getEnvironmentText(row,col,envname);
	if ( text != QString::null )
	{
		if ( m_ki->latexCommands()->isMathModeEnv(envname)  )
			text = '$' + text + '$';
		else if ( m_ki->latexCommands()->isDisplaymathModeEnv(envname) )
			text = "\\[" + text + "\\]";

		if ( KileConfig::envPreviewInWidget() )
			m_ki->previewWidget()->showActivePreview( text,m_ki->getName(doc),row,KileTool::qpEnvironment );
		else
			run( text,m_ki->getName(doc),row );
	}
	else
	{
		showError( i18n("There is no surrounding environment.") );
	}
}

// compile and view current subdocument (only mastermode)

void QuickPreview::previewSubdocument(Kate::Document *doc)
{
	// this mode is only useful with a master document
	if ( !m_ki->docManager()->activeProject() && m_ki->getSinglemode() ) 
	{
		showError( i18n("This job is only useful with a master document.") );
		return;
	}

	// the current document should not be the master document
	QString filename = doc->url().path();
	if ( filename == m_ki->getCompileName() ) 
	{
		showError( i18n("This is not a subdocument, but the master document.") );
		return;
	}

	run( doc->text(),m_ki->getName(doc),0 );
}

// compile and view current mathgroup (singlemode and mastermode)

void QuickPreview::previewMathgroup(Kate::Document *doc)
{
	uint row,col;
	QString text = m_ki->editorExtension()->getMathgroupText(row,col);
	if ( text != QString::null )
	{
		if ( KileConfig::mathgroupPreviewInWidget() )
			m_ki->previewWidget()->showActivePreview( text,m_ki->getName(doc),row,KileTool::qpMathgroup );
		else
			run( text,m_ki->getName(doc),row );
	}
	else
	{
		showError( i18n("There is no surrounding mathgroup.") );
	}

}

//////////////////// run quick preview ////////////////////

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

bool QuickPreview::isRunning()
{
	return ( m_running > 0 );
}

bool QuickPreview::run(const QString &text,const QString &textfilename,int startrow) 
{
	// define possible tools
	QMap <QString,QString> map;
	map[m_taskList[0]] = "PreviewLaTeX,,,ViewDVI,Embedded Viewer,dvi"; 
	map[m_taskList[1]] = "PreviewLaTeX,,,ViewDVI,KDVI Unique,dvi";
	map[m_taskList[2]] = "PreviewLaTeX,DVItoPS,Default,ViewPS,Embedded Viewer,ps";
	map[m_taskList[3]] = "PreviewLaTeX,DVItoPS,Default,ViewPS,KGhostView,ps";
	map[m_taskList[4]] = "PreviewPDFLaTeX,,,ViewPDF,KPDF (embedded),pdf"; 
	map[m_taskList[5]] = "PreviewPDFLaTeX,,,ViewPDF,KGhostView,pdf";
	map[m_taskList[6]] = "PreviewPDFLaTeX,,,ViewPDF,KPDF,pdf";

	QString previewtask = KileConfig::previewTask();
	if ( ! map.contains(previewtask) ) 
	{
		showError(i18n("Could not run QuickPreview:\nunknown task '%1'",previewtask));
		return false;
	}

	return run (text, textfilename, startrow, map[previewtask]);
}

bool QuickPreview::run(const QString &text,const QString &textfilename,int startrow,const QString &spreviewlist) 
{
	KILE_DEBUG() << "==QuickPreview::run()=========================="  << endl;
	m_ki->logWidget()->clear();
	if ( m_running > 0 )
	{
		showError( i18n("There is already a preview running, which you have to finish to run this one.") );
		return false;
	}
	
	// check if there is something to compile
	if ( text.isEmpty() ) 
	{
		showError(i18n("There is nothing to compile and preview."));
		return false;
	}
	
	// create the name of a temporary file or delete already existing temporary files
	if ( m_tempfile.isEmpty() ) 
	{
		m_tempfile =  KTempDir(QString::null).name() + "preview.tex";
		KILE_DEBUG() << "\tdefine tempfile: " << m_tempfile << endl;
	} 
	else 
	{
		removeTempFiles();
	}
	
	// create the temporary file with preamble and text
	int preamblelines = createTempfile(text);
	if ( preamblelines == 0 )
		return false;
	
	QStringList previewlist = spreviewlist.split(",", QString::KeepEmptyParts);
	
	// create preview tools 
	KILE_DEBUG() << "\tcreate latex tool for QuickPreview: "  << previewlist[pvLatex] << endl;
	KileTool::PreviewLaTeX *latex = (KileTool::PreviewLaTeX  *)m_ki->toolFactory()->create(previewlist[pvLatex],false);
	if ( !latex ) 
	{
		showError(i18n("Could not run '%1' for QuickPreview.",QString("LaTeX")));
		return false;
	}
	
	KileTool::Base *dvips = 0L;
	if ( ! previewlist[1].isEmpty() ) 
	{
		QString dvipstool = previewlist[pvDvips] + " (" + previewlist[pvDvipsCfg] + ')';
		KILE_DEBUG() << "\tcreate dvips tool for QuickPreview: "  << previewlist[pvDvips] << endl;
		dvips = m_ki->toolFactory()->create(previewlist[pvDvips]);
		if ( !dvips ) 
		{
			showError(i18n("Could not run '%1' for QuickPreview.",dvipstool));
			return false;
		}
	} 

	KileTool::Base *viewer = 0L;
	if ( !previewlist[pvViewer].isEmpty() ) 
	{
		QString viewertool = previewlist[pvViewer] + " (" + previewlist[pvViewerCfg] + ')';
		KILE_DEBUG() << "\tcreate viewer for QuickPreview: "  << viewertool << endl;
		viewer = m_ki->toolFactory()->create(previewlist[pvViewer],false);
		if ( !viewer ) 
		{
			showError(i18n("Could not run '%1' for QuickPreview.",viewertool));
			return false;
		}
	} 
	
	// set value of texinput path (only for QuickPreview tools)
	QString texinputpath = KileConfig::teXPaths();
	QString inputdir = QFileInfo(m_ki->getCompileName()).absolutePath();
	if ( ! texinputpath.isEmpty() )
		inputdir += ':' + texinputpath;
 	KileConfig::setPreviewTeXPaths(inputdir);
	KILE_DEBUG() << "\tQuickPreview: inputdir is '" << inputdir << "'" << endl;
	
	// prepare tools: previewlatex
	QString filepath = m_tempfile.left( m_tempfile.length()-3 ); 
	latex->setPreviewInfo(textfilename,startrow,preamblelines+1);
	latex->setSource(m_tempfile);
	latex->prepareToRun();
	latex->setQuickie();
	if ( m_ki->toolManager()->run(latex) != KileTool::Running )
		return false;

	connect(latex, SIGNAL(destroyed()), this, SLOT(toolDestroyed()));
	m_running++;

	// dvips
	if ( dvips )
	{
		dvips->setSource( filepath + "dvi" );
		dvips->setQuickie();
		if ( m_ki->toolManager()->run(dvips,previewlist[pvDvipsCfg])  != KileTool::Running )
			return false;

		connect(dvips, SIGNAL(destroyed()), this, SLOT(toolDestroyed()));
		m_running++;
	}

	// viewer
	if ( viewer )
	{
		connect(viewer, SIGNAL(destroyed()), this, SLOT(toolDestroyed()));
		viewer->setSource( filepath + previewlist[pvExtension] );
		viewer->setQuickie();
		if ( m_ki->toolManager()->run(viewer,previewlist[pvViewerCfg]) != KileTool::Running )
			return false;
	}

	return true;	
}

void QuickPreview::toolDestroyed()
{
	KILE_DEBUG() << "\tQuickPreview: tool destroyed" << endl;
	if ( m_running > 0 )
		m_running--;
}

QString QuickPreview::getPreviewFile(const QString &extension) 
{
	if (m_tempfile.length () < 3) 
		return QString::null;

	QString filepath = m_tempfile.left(m_tempfile.length () - 3); 
	return filepath + extension;
} 

//////////////////// tempfile ////////////////////

int QuickPreview::createTempfile(const QString &text)
{
	// determine main document to read the preamble
	QString filename = m_ki->getCompileName();
	if ( filename.isEmpty() ) 
	{
		showError(i18n("Could not determine the main document."));
		return 0;
	}
	
	// open to read
	QFile fin( filename );
	if ( !fin.exists() || !fin.open(QIODevice::ReadOnly) ) 
	{
		showError(i18n("Could not read the preamble."));
		return 0;
	}
	KILE_DEBUG() << "\tcreate a temporary file: "  << m_tempfile << endl;
	
	// use a textstream
	Q3TextStream preamble(&fin);

	// create the temporary file 
	QFile tempfile(m_tempfile);
	if ( ! tempfile.open( QIODevice::WriteOnly ) ) 
	{
		showError(i18n("Could not create a temporary file."));
		return 0;
	}
	Q3TextStream stream( &tempfile );
	
	// set the encoding according to the original file (tbraun)
	if(m_ki->activeTextDocument())
	{
		QTextCodec *codec = QTextCodec::codecForName(m_ki->activeTextDocument()->encoding().ascii());
	 if ( codec )
		stream.setCodec(codec); 
	}
	// write the whole preamble into this temporary file
	QString textline;
	int preamblelines = 0;
	bool begindocumentFound = false;
	while ( ! preamble.eof() ) 
	{
		textline = preamble.readLine();
		if ( textline.indexOf("\\begin{document}") >= 0 ) 
		{
			begindocumentFound = true;
			break;
		}
		stream << textline << "\n";
		preamblelines++;
	}

	// look if we found '\begin{document}' to finish the preamble
	if ( ! begindocumentFound ) 
	{
		tempfile.close();
		showError(i18n("Could not find a '\\begin{document}' command."));
		return 0;
	}

	// add the text to compile
	stream << "\\pagestyle{empty}\n";
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
	QString tempdir = fi.absolutePath() + '/';
	
	QDir dir = fi.dir(true); 
	if ( dir.exists() ) 
	{
		QStringList list = dir.entryList(fi.baseName()+".*");
		for ( QStringList::Iterator it=list.begin(); it!=list.end(); ++it ) 
		{
			QFile::remove( tempdir + (*it) );
			// KILE_DEBUG() << "\tremove temporary file: " << tempdir + (*it) << endl;
		}
		
		if ( rmdir )
			dir.rmdir(tempdir);
	}
}

//////////////////// error messages ////////////////////

void QuickPreview::showError(const QString &text)
{
	m_ki->logWidget()->printMsg( KileTool::Error, text, i18n("QuickPreview") );
}

}

#include "quickpreview.moc"
