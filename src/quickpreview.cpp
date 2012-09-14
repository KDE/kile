/**************************************************************************************************
   Copyright (C) 2005-2007 by Holger Danielsson (holger.danielsson@versanet.de)
                 2007-2009 by Michel Ludwig (michel.ludwig@kdemail.net)
 **************************************************************************************************/

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
#include "widgets/logwidget.h"

#include <QDir>
#include <QMap>
#include <QFile>
#include <QFileInfo>
#include <QTextCodec>
#include <QTextStream>

#include <KLocale>
#include <KStandardDirs>
#include <KTextEditor/Document>
#include <KTextEditor/View>

#include "errorhandler.h"
#include "kiledebug.h"


namespace KileTool
{

QuickPreview::QuickPreview(KileInfo *ki) : m_ki(ki), m_running(0), m_tempDir(NULL)
{
	m_taskList << i18n("LaTeX ---> DVI")
	           << i18n("LaTeX ---> DVI (Okular)")
	           << i18n("LaTeX ---> PS")
	           << i18n("LaTeX ---> PS (Okular)")
	           << i18n("PDFLaTeX ---> PDF")
	           << i18n("PDFLaTeX ---> PDF (Okular)")
	           << i18n("XeLaTeX ---> PDF")
	           << i18n("XeLaTeX ---> PDF (Okular)")
	           << i18n("LuaLaTeX ---> PDF")
	           << i18n("LuaLaTeX ---> PDF (Okular)")
	           ;
}

QuickPreview::~QuickPreview() 
{
	delete m_tempDir;
}

//////////////////// quick preview ////////////////////

// compile and view current selection (singlemode and mastermode)

void QuickPreview::previewSelection(KTextEditor::View *view, bool previewInWidgetConfig)
{
	if (view->selection()) {
		int startLine = view->selectionRange().start().line();
		KTextEditor::Document *doc = view->document();
		if ( previewInWidgetConfig && KileConfig::selPreviewInWidget() ) {
			m_ki->previewWidget()->showActivePreview(view->selectionText(), m_ki->getName(doc), startLine, KileTool::qpSelection);
		}
		else {
			run(view->selectionText(), m_ki->getName(doc), startLine);
			view->removeSelection();
		}
	} 
	else {
		showError( i18n("There is no selection to compile.") );
	}
}

// compile and view current environment (singlemode and mastermode)

void QuickPreview::previewEnvironment(KTextEditor::Document *doc)
{
	int row, col;
	QString envname;
	QString text = m_ki->editorExtension()->getEnvironmentText(row, col, envname);
	if (!text.isEmpty()) {
		if(m_ki->latexCommands()->isMathModeEnv(envname)) {
			text = '$' + text + '$';
		}
		else if (m_ki->latexCommands()->isDisplaymathModeEnv(envname)) {
			text = "\\[" + text + "\\]";
		}

		if(KileConfig::envPreviewInWidget()) {
			m_ki->previewWidget()->showActivePreview(text, m_ki->getName(doc), row, KileTool::qpEnvironment);
		}
		else {
			run(text, m_ki->getName(doc), row);
		}
	}
	else {
		showError(i18n("There is no surrounding environment."));
	}
}

// compile and view current subdocument (only mastermode)

void QuickPreview::previewSubdocument(KTextEditor::Document *doc)
{
	// this mode is only useful with a master document
	if(!m_ki->docManager()->activeProject() && m_ki->getSinglemode()) {
		showError(i18n("This job is only useful with a master document."));
		return;
	}

	// the current document should not be the master document
	QString filename = doc->url().toLocalFile();
	if(filename == m_ki->getCompileName()) {
		showError( i18n("This is not a subdocument, but the master document."));
		return;
	}

	run(doc->text(), m_ki->getName(doc), 0);
}

// compile and view current mathgroup (singlemode and mastermode)

void QuickPreview::previewMathgroup(KTextEditor::Document *doc)
{
	uint row,col;
	QString text = m_ki->editorExtension()->getMathgroupText(row, col);
	if (!text.isEmpty()) {
		if(KileConfig::mathgroupPreviewInWidget()) {
			m_ki->previewWidget()->showActivePreview(text, m_ki->getName(doc), row, KileTool::qpMathgroup);
		}
		else {
			run(text, m_ki->getName(doc), row);
		}
	}
	else {
		showError(i18n("There is no surrounding mathgroup."));
	}

}

//////////////////// run quick preview ////////////////////

void QuickPreview::getTaskList(QStringList &tasklist)
{
	tasklist.clear();
	tasklist << "Tool/ViewDVI/Embedded Viewer=" + m_taskList[0]
	         << "Tool/ViewDVI/Okular="     + m_taskList[1]
	         << "Tool/ViewPS/Embedded Viewer="  + m_taskList[2]
	         << "Tool/ViewPS/Okular="       + m_taskList[3]
	         << "Tool/ViewPDF/Embedded Viewer=" + m_taskList[4]
	         << "Tool/ViewPDF/Okular="      + m_taskList[5]
	         << "Tool/ViewPDF/Embedded Viewer=" + m_taskList[6]
	         << "Tool/ViewPDF/Okular="      + m_taskList[7]
	         << "Tool/ViewPDF/Embedded Viewer=" + m_taskList[8]
	         << "Tool/ViewPDF/Okular="      + m_taskList[9]
	         ;
}

bool QuickPreview::isRunning()
{
	return (m_running > 0);
}

bool QuickPreview::run(const QString &text,const QString &textfilename,int startrow) 
{
	// define possible tools
	QMap <QString,QString> map;
	map[m_taskList[0]] = "PreviewLaTeX,,,ViewDVI,Embedded Viewer,dvi"; 
	map[m_taskList[1]] = "PreviewLaTeX,,,ViewDVI,Okular,dvi"; 
	map[m_taskList[2]] = "PreviewLaTeX,DVItoPS,Default,ViewPS,Embedded Viewer,ps";
	map[m_taskList[3]] = "PreviewLaTeX,DVItoPS,Default,ViewPS,Okular,ps";
	map[m_taskList[4]] = "PreviewPDFLaTeX,,,ViewPDF,Embedded Viewer,pdf"; 
	map[m_taskList[5]] = "PreviewPDFLaTeX,,,ViewPDF,Okular,pdf";
	map[m_taskList[6]] = "PreviewXeLaTeX,,,ViewPDF,Embedded Viewer,pdf"; 
	map[m_taskList[7]] = "PreviewXeLaTeX,,,ViewPDF,Okular,pdf";
	map[m_taskList[8]] = "PreviewLuaLaTeX,,,ViewPDF,Embedded Viewer,pdf";
	map[m_taskList[9]] = "PreviewLuaLaTeX,,,ViewPDF,Okular,pdf";

	QString previewtask = KileConfig::previewTask();
	if(!map.contains(previewtask)) {
		showError(i18n("Could not run QuickPreview:\nunknown task '%1'",previewtask));
		return false;
	}

	return run (text, textfilename, startrow, map[previewtask]);
}

bool QuickPreview::run(const QString &text,const QString &textfilename,int startrow,const QString &spreviewlist) 
{
	KILE_DEBUG() << "==QuickPreview::run()=========================="  << endl;
	m_ki->errorHandler()->clearMessages();
	if(m_running > 0) {
		showError( i18n("There is already a preview running that has to be finished to run this one.") );
		return false;
	}
	
	// check if there is something to compile
	if(text.isEmpty()) {
		showError(i18n("There is nothing to compile and preview."));
		return false;
	}
	
	delete m_tempDir;
	m_tempDir = new KTempDir(KStandardDirs::locateLocal("tmp", "kile-preview"));
	m_tempDir->setAutoRemove(true);
	m_tempFile = QFileInfo(m_tempDir->name(), "preview.tex").absoluteFilePath();
	KILE_DEBUG() << "\tdefine tempfile: " << m_tempFile << endl;

	// create the temporary file with preamble and text
	int preamblelines = createTempfile(text);
	if(preamblelines == 0) {
		return false;
	}

	QStringList previewlist = spreviewlist.split(',', QString::KeepEmptyParts);
	
	// create preview tools
	KILE_DEBUG() << "\tcreate latex tool for QuickPreview: "  << previewlist[pvLatex] << endl;
	KileTool::PreviewLaTeX *latex = dynamic_cast<KileTool::PreviewLaTeX*>(m_ki->toolManager()->createTool(previewlist[pvLatex], QString(), false));
	if(!latex) {
		showError(i18n("Could not run '%1' for QuickPreview.", QString("LaTeX")));
		return false;
	}
	
	KileTool::Base *dvips = NULL;
	if(!previewlist[1].isEmpty()) {
		QString dvipstool = previewlist[pvDvips] + " (" + previewlist[pvDvipsCfg] + ')';
		KILE_DEBUG() << "\tcreate dvips tool for QuickPreview: "  << previewlist[pvDvips] << endl;
		dvips = m_ki->toolManager()->createTool(previewlist[pvDvips], previewlist[pvDvipsCfg]);
		if(!dvips) {
			showError(i18n("Could not run '%1' for QuickPreview.",dvipstool));
			return false;
		}
	} 

	KileTool::Base *viewer = NULL;
	if(!previewlist[pvViewer].isEmpty()) {
		QString viewertool = previewlist[pvViewer] + " (" + previewlist[pvViewerCfg] + ')';
		KILE_DEBUG() << "\tcreate viewer for QuickPreview: "  << viewertool << endl;
		viewer = m_ki->toolManager()->createTool(previewlist[pvViewer], previewlist[pvViewerCfg], false);
		if(!viewer) {
			showError(i18n("Could not run '%1' for QuickPreview.",viewertool));
			return false;
		}
	} 
	
	// set value of texinput path (only for QuickPreview tools)
	QString texinputpath = KileConfig::teXPaths();
	QString inputdir = QFileInfo(m_ki->getCompileName()).absolutePath();
	if(!texinputpath.isEmpty()) {
		inputdir += ':' + texinputpath;
	}
 	KileConfig::setPreviewTeXPaths(inputdir);
	KILE_DEBUG() << "\tQuickPreview: inputdir is '" << inputdir << "'" << endl;
	
	// prepare tools: previewlatex
	QString filepath = m_tempFile.left(m_tempFile.length() - 3);
	latex->setPreviewInfo(textfilename, startrow, preamblelines + 1);
	latex->setSource(m_tempFile);
	latex->prepareToRun();
	latex->setQuickie();
	connect(latex, SIGNAL(destroyed()), this, SLOT(toolDestroyed()));
	m_ki->toolManager()->run(latex);

	m_running++;

	// dvips
	if(dvips) {
		dvips->setSource( filepath + "dvi" );
		dvips->setQuickie();
		connect(dvips, SIGNAL(destroyed()), this, SLOT(toolDestroyed()));
		m_ki->toolManager()->run(dvips);

		m_running++;
	}

	// viewer
	if(viewer) {
		connect(viewer, SIGNAL(destroyed()), this, SLOT(toolDestroyed()));
		viewer->setSource( filepath + previewlist[pvExtension] );
		viewer->setQuickie();
		m_ki->toolManager()->run(viewer);
	}

	return true;
}

void QuickPreview::toolDestroyed()
{
	KILE_DEBUG() << "\tQuickPreview: tool destroyed" << endl;
	if(m_running > 0) {
		--m_running;
	}
}

QString QuickPreview::getPreviewFile(const QString &extension)
{
	if (m_tempFile.length () < 3) {
		return QString();
	}

	QString filepath = m_tempFile.left(m_tempFile.length () - 3);
	return filepath + extension;
} 

//////////////////// tempfile ////////////////////

int QuickPreview::createTempfile(const QString &text)
{
	// determine main document to read the preamble
	QString filename = m_ki->getCompileName();
	if(filename.isEmpty()) {
		showError(i18n("Could not determine the main document."));
		return 0;
	}
	
	// open to read
	QFile fin(filename);
	if(!fin.exists() || !fin.open(QIODevice::ReadOnly)) {
		showError(i18n("Could not read the preamble."));
		return 0;
	}
	KILE_DEBUG() << "\tcreate a temporary file: "  << m_tempFile << endl;
	
	// use a textstream
	QTextStream preamble(&fin);

	// create the temporary file
	QFile tempfile(m_tempFile);
	if(!tempfile.open(QIODevice::WriteOnly)) {
		showError(i18n("Could not create a temporary file."));
		return 0;
	}
	QTextStream stream(&tempfile);
	
	// set the encoding according to the original file (tbraun)
	if(m_ki->activeTextDocument()) {
		QTextCodec *codec = QTextCodec::codecForName(m_ki->activeTextDocument()->encoding().toAscii());
		if(codec) {
			stream.setCodec(codec);
		}
	}
	// write the whole preamble into this temporary file
	QString textline;
	int preamblelines = 0;
	bool begindocumentFound = false;
	while(!preamble.atEnd()) {
		textline = preamble.readLine();
		if (textline.indexOf("\\begin{document}") >= 0) {
			begindocumentFound = true;
			break;
		}
		stream << textline << "\n";
		preamblelines++;
	}

	// look if we found '\begin{document}' to finish the preamble
	if (!begindocumentFound) {
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

//////////////////// error messages ////////////////////

void QuickPreview::showError(const QString &text)
{
	m_ki->errorHandler()->printMessage(KileTool::Error, text, i18n("QuickPreview"));
}

}

#include "quickpreview.moc"
