/***************************************************************************
    date                 : Dec 06 2005
    version              : 0.24
    copyright            : (C) 2004-2005 by Holger Danielsson, 2004 Jeroen Wijnhout
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

#include "includegraphicsdialog.h"  

#include <qregexp.h>
#include <qfileinfo.h>
#include <qgroupbox.h>
#include <qvgroupbox.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qcheckbox.h>
#include <qlabel.h>

#include <klocale.h> 
#include <kfiledialog.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kpushbutton.h>
#include <kprocess.h>
#include <klineedit.h>

#include "kileconfig.h"
#include "kileinfo.h"
#include "kileedit.h"

namespace KileDialog
{

IncludeGraphics::IncludeGraphics(QWidget *parent, const QString &startdir, KileInfo *ki) :
	KDialogBase( Plain, i18n("Include Graphics"), Ok | Cancel, Ok, parent, 0, true, true),
	m_startdir(startdir),
	m_ki(ki),
	m_proc(0)
{
   // Layout
   QVBoxLayout *vbox = new QVBoxLayout(plainPage(), 6,6 );

   // first groupbox: choose picture
   QVGroupBox* group= new QVGroupBox(i18n("File"), plainPage());

   QWidget *widget = new QWidget(group);
   QGridLayout *grid = new QGridLayout( widget, 4,3, 6,6, "");
   grid->addRowSpacing( 0, fontMetrics().lineSpacing() );
   grid->addColSpacing( 0, fontMetrics().lineSpacing() );
   grid->setColStretch(1,1);

   // line 1: QLabel
   QLabel *label1 = new QLabel(i18n("Picture:"), widget);
   grid->addWidget( label1, 0,0 );

   // line 1: KLineEdit
   edit_file = new KLineEdit("",widget);
   edit_file->setMinimumWidth(300);
   grid->addWidget( edit_file, 0,1 );

   // line 1: Choose-Box
   KPushButton *pb_choose = new KPushButton("", widget, "filechooser_button" );
   pb_choose->setPixmap( SmallIcon("fileopen") );

   pb_choose->setFixedWidth(pb_choose->sizeHint().width());      // set width
   grid->addRowSpacing( 0, pb_choose->sizeHint().height()+5 );   // update height of line
   grid->addWidget(pb_choose,0,2);

   // line 2: some (more or less useful) information
   QLabel *label2 = new QLabel(i18n("Info:"), widget);
   infolabel = new QLabel("---", widget);

   grid->addWidget( label2, 1,0 );
   grid->addWidget( infolabel, 1,1 );

   // line 3: some output options
   QLabel *label3 = new QLabel(i18n("Output:"), widget);

   QWidget *cb_widget = new QWidget(widget);
   QGridLayout *cb_grid = new QGridLayout( cb_widget, 1,2, 0,0,"");
   cb_center = new QCheckBox(i18n("Center picture"),cb_widget);
   cb_pdftex = new QCheckBox(i18n("pdftex/pdflatex"),cb_widget);
   cb_grid->addWidget(cb_center,0,0);
   cb_grid->addWidget(cb_pdftex,0,1);

   grid->addWidget( label3, 2,0 );
   grid->addWidget( cb_widget, 2,1 );

   // line 4: graphics path
   QLabel *label5 = new QLabel(i18n("Path:"), widget);
   cb_graphicspath = new QCheckBox(i18n("Use \\graphicspath command of LaTeX"),widget);

   grid->addWidget( label5, 3,0 );
   grid->addWidget( cb_graphicspath, 3,1 );

   // second groupbox: options
   QVGroupBox* gb_opt= new QVGroupBox(i18n("Options"), plainPage());
   QWidget *widget_opt = new QWidget(gb_opt);
   QGridLayout *grid_opt = new QGridLayout( widget_opt, 2,4, 6,6, "");

   QLabel *label7 = new QLabel(i18n("Width:"), widget_opt);
   QLabel *label8 = new QLabel(i18n("Height:"),widget_opt);
   QLabel *label9 = new QLabel(i18n("Angle:"), widget_opt);
   QLabel *label10= new QLabel(i18n("Bounding box:"), widget_opt);
   edit_width = new KLineEdit("",widget_opt);
   edit_height = new KLineEdit("",widget_opt);
   edit_angle = new KLineEdit("",widget_opt);
   edit_bb = new KLineEdit("",widget_opt);

   grid_opt->addWidget( label7,     0,0, Qt::AlignRight );
   grid_opt->addWidget( edit_width, 0,1 );
   grid_opt->addWidget( label9,     0,2, Qt::AlignRight );
   grid_opt->addWidget( edit_angle, 0,3 );

   grid_opt->addWidget( label8,      1,0, Qt::AlignRight );
   grid_opt->addWidget( edit_height, 1,1 );
   grid_opt->addWidget( label10,     1,2, Qt::AlignRight );
   grid_opt->addWidget( edit_bb,     1,3 );

    // third groupbox: figure environment
   QGroupBox *gb_fig= new QGroupBox(2,Qt::Horizontal,i18n("Figure Environment"), plainPage());
   QWidget *widget_fig = new QWidget(gb_fig);
   QGridLayout *grid_fig = new QGridLayout( widget_fig, 3,2, 6,6, "");

   QLabel *label4 = new QLabel(i18n("Figure:"), widget_fig);
   lb_label = new QLabel(i18n("Label:"), widget_fig);
   lb_caption = new QLabel(i18n("Caption:"), widget_fig);
   cb_figure = new QCheckBox(i18n("Use figure environment"),widget_fig);
   edit_label = new KLineEdit("fig:",widget_fig);
   edit_caption = new KLineEdit("",widget_fig);

   grid_fig->addWidget( label4,0,0);
   grid_fig->addWidget( cb_figure, 0,1);
   grid_fig->addWidget( lb_label,1,0);
   grid_fig->addWidget( edit_label, 1,1);
   grid_fig->addWidget( lb_caption,2,0);
   grid_fig->addWidget( edit_caption, 2,1);

   // add to layout
   vbox->addWidget(group);
   vbox->addWidget(gb_opt);
   vbox->addWidget(gb_fig);
   vbox->addStretch();

   // read configuration
   readConfig();
   updateFigure();

   // connect
   connect( pb_choose, SIGNAL( clicked() ), this, SLOT( chooseFile() ) );
   connect( cb_figure, SIGNAL(clicked()), this, SLOT(updateFigure() ) );

   setFocusProxy( edit_file );
}

IncludeGraphics::~IncludeGraphics() 
{
   delete m_proc;
}

////////////////////////////// configuration data //////////////////////////////

void IncludeGraphics::readConfig()
{
	cb_center->setChecked( KileConfig::igCenter() );                             
	cb_pdftex->setChecked( KileConfig::igPdftex() ); 
	cb_graphicspath->setChecked( KileConfig::igGraphicspath() );                             
	cb_figure->setChecked( KileConfig::igFigure() );
	
	m_imagemagick = KileConfig::imagemagick();
	m_boundingbox = KileConfig::boundingbox();                  
	m_defaultresolution = KileConfig::resolution();
}

void IncludeGraphics::writeConfig()
{
	KileConfig::setIgCenter( cb_center->isChecked() );                             
	KileConfig::setIgPdftex( cb_pdftex->isChecked() );                             
	KileConfig::setIgGraphicspath( cb_graphicspath->isChecked() );                             
	KileConfig::setIgFigure( cb_figure->isChecked() );
}

////////////////////////////// update figure environment //////////////////////////////

void IncludeGraphics::updateFigure()
{
   bool state = cb_figure->isChecked();

   lb_label->setEnabled(state);
   lb_caption->setEnabled(state);
   edit_label->setEnabled(state);
   edit_caption->setEnabled(state);
}

////////////////////////////// determine the whole tag //////////////////////////////

QString IncludeGraphics::getTemplate()
{
	QString s;

	// state of figure and center checkbox
	bool figure = cb_figure->isChecked();
	bool center = cb_center->isChecked();
 	QString indent = ( figure || center ) ? m_ki->editorExtension()->autoIndentEnvironment() : QString::null;
	
	// add start of figure environment ?
	if ( figure )
		s += "\\begin{figure}\n";

	// add start of center environment ?
	if ( center )
	{
		if ( figure )
			s += indent + "\\centering\n";
		else
			s += "\\begin{center}\n";
	}

	// add includegraphics command
	s += indent + "\\includegraphics";
 
	// add some options
	QString options = getOptions();
	if ( !options.isEmpty() )
		s += '[' + options + ']';
 
	// add name of picture
	// either take the filename or try to take the relative part of the name
	QString filename = ( cb_graphicspath->isChecked() ) 
	                 ? QFileInfo(edit_file->text()).fileName() 
	                 : m_ki->relativePath(QFileInfo(m_ki->getCompileName()).dirPath(), edit_file->text());
	s += '{' + filename + "}\n";
 
	// add some comments (depending of given resolution, this may be wrong!)
	QString info = getInfo();
	if (info.length() > 0) 
		s += indent + info + '\n';

	// close center environment ? 
	if ( center && !figure )
		s += "\\end{center}\n";

	// close figure environment ?
	if ( figure )
	{
		QString caption = edit_caption->text().stripWhiteSpace();
		if ( !caption.isEmpty() )
			s +=  indent + "\\caption{" + caption + "}\n";
		QString label = edit_label->text().stripWhiteSpace();
		if ( !label.isEmpty() && label!="fig:" )
			s +=  indent + "\\label{" + label + "}\n";
		s += "\\end{figure}\n";
	}

	return s;
}

QString IncludeGraphics::getFilename()
{
	return edit_file->text();
}

////////////////////////////// some calculations //////////////////////////////

QString IncludeGraphics::getOptions()
{
   QString s = "";

   if ( ! edit_width->text().isEmpty() )
      s += ",width=" + edit_width->text();

   if ( ! edit_height->text().isEmpty() )
      s += ",height=" + edit_height->text();

   if ( ! edit_angle->text().isEmpty() )
      s += ",angle=" + edit_angle->text();

	// Only dvips needs the bounding box, not pdftex/pdflatex.
  // But it will be always inserted as a comment.
   if ( !edit_bb->text().isEmpty() && !cb_pdftex->isChecked() )
      s += ",bb=" + edit_bb->text();

   if ( s.left(1) == "," )
      return s.right(s.length()-1);
   else
      return s;
}

////////////////////////////// graphics info //////////////////////////////

QString IncludeGraphics::getInfo()
{
	QString wcm,hcm,dpi;
	int wpx,hpx;

	bool ok = getPictureSize(wpx,hpx,dpi,wcm,hcm);
	if ( ! ok )
		return "";
	else
	{
		QFileInfo fi( edit_file->text() );

		return "% " + fi.baseName() + '.' + fi.extension(true)
		            + QString(": %1x%2 pixel").arg(wpx).arg(hpx)
		            + ", " + dpi + "dpi"
		            + ", " + wcm + 'x' + hcm + " cm"
		            + ", bb=" + edit_bb->text();
	}
}

void IncludeGraphics::setInfo()
{
	QString text;
	QString wcm,hcm,dpi;
	int wpx,hpx;

	if ( !edit_file->text().isEmpty() && getPictureSize(wpx,hpx,dpi,wcm,hcm) ) 
	{
		text = QString("%1x%2 pixel").arg(wpx).arg(hpx)
			       + " / " + wcm + 'x' + hcm + " cm"
			       + "  (" + dpi + "dpi)";
	} 
	else
		text = "---";

// insert text
infolabel->setText(text);
}

bool IncludeGraphics::getPictureSize(int &wpx, int &hpx, QString &dpi, QString &wcm, QString &hcm)
{
	wpx = m_width;
	hpx = m_height;

	dpi = QString("%1").arg((int)(m_resolution+0.5));

	// convert from inch to cm
	float w = (float)m_width / m_resolution * 2.54;
	wcm = wcm.setNum(w,'f',2);

	float h = (float)m_height / m_resolution * 2.54;
	hcm = hcm.setNum(h,'f',2);
	return true;
}


void IncludeGraphics::chooseFile()
{
   QString filter = ( cb_pdftex->isChecked() )
                  ? i18n("*.png *.jpg *.pdf|Graphics\n")              // dani  31.7.2004
                          + "*.png|PNG Files\n"
                          + "*.jpg|JPG Files\n"
                          + "*.pdf|PDF Files\n"
                          + "*|All Files"
                  : i18n("*.png *.jpg *.eps.gz *.eps|Graphics\n")     // dani  31.7.2004
                          + "*.png|PNG Files\n"
                          + "*.jpg|JPG Files\n"
                          + "*.eps.gz|Zipped EPS Files\n"
                          + "*.eps|EPS Files\n"
                          + "*|All Files";

   QString fn = KFileDialog::getOpenFileName( m_startdir,filter,
                                              this,i18n("Select File") );
   QFileInfo fi(fn);
   // insert the chosen file
   edit_file->setText( fn );

   // could we accept the picture?
   if ( !fn.isEmpty() && fi.exists() && fi.isReadable() )
   {
      // execute the command and filter the result:
      // eps|eps.gz --> %%BoundingBox: 0 0 123 456
      // bitmaps    --> w=123 h=456 dpi=789
      QString grep = " | grep -m1 \"^%%BoundingBox:\"";
      QString ext = QFileInfo(fn).extension();
      if ( ext == "eps" )
         execute( "cat " + fn + grep);
      else if ( ext == "eps.gz" )
         execute( "gunzip -c " + fn + grep);
      else
         execute( "identify -format \"w=%w h=%h dpi=%x\" " + fn);
   } else {
      kdDebug() << "=== IncludeGraphics::error ====================" << endl;
      kdDebug() << "   filename: '" << fn << "'" << endl;
   }
}

void IncludeGraphics::execute(const QString &command)
{
   if ( !m_boundingbox || (!m_imagemagick && command.left(8)=="identify") )
      return;

   if(m_proc)
      delete m_proc;

   m_proc = new KShellProcess("/bin/sh");
   m_proc->clearArguments();
   (*m_proc) << QStringList::split(' ',command);

   connect(m_proc, SIGNAL(receivedStdout(KProcess*,char*,int)),
           this, SLOT(slotProcessOutput(KProcess*,char*,int)) );
   connect(m_proc, SIGNAL(receivedStderr(KProcess*,char*,int)),
           this, SLOT(slotProcessOutput(KProcess*,char*,int)) );
   connect(m_proc, SIGNAL(processExited(KProcess*)),
           this, SLOT(slotProcessExited(KProcess*)) );

   m_output = "";
   kdDebug() << "=== IncludeGraphics::execute ====================" << endl;
   kdDebug() << "   execute '" << command << "'" << endl;

   m_proc->start(KProcess::NotifyOnExit, KProcess::AllOutput);
}

// get all output of identify

void IncludeGraphics::slotProcessOutput(KProcess*,char* buffer,int buflen)
{
   m_output += QString::fromLocal8Bit(buffer,buflen);
}

// identify was called

void IncludeGraphics::slotProcessExited(KProcess* proc)
{
  if ( proc->normalExit() &&  !proc->exitStatus() ) {
      kdDebug() << "   result: " << m_output << endl;

      // set the default resolution
      m_resolution = m_defaultresolution;

      // analyze the result
      if ( m_output.left(14) == "%%BoundingBox:" )
      {
         edit_bb->setText( m_output.stripWhiteSpace().mid(15,m_output.length()-15) );

         // show information
         setInfo();
      }
      else if ( m_output.left(2) == "w=" )
      {
         // dani  31.7.2004
         // older version of imagemagick (pre 6.0):
         //  - doesn't care of PixelsPerCentimeter, but always works with PixelsPerInch
         //  - doesn't use floating numbers as resolution
	 // so the bounding box has to be calculated in a different way

         // this regexp will accept floating point numbers as resolution
         QRegExp reg("w=(\\d+)\\s+h=(\\d+)\\s+dpi=([0-9.]+) (.*)");
         if ( reg.search(m_output) == -1 )
             return;

         // get bounding box and resolution
	bool ok;
	m_width = (int)reg.cap(1).toInt( &ok);
	if (!ok) return;

	m_height = (int)reg.cap(2).toInt( &ok);
	if (!ok) return;

	float res = (float)reg.cap(3).toFloat( &ok);
	if (!ok) return;
	if ( res > 0.0 )
		m_resolution = res;

	// look, if resolution is in PixelsPerCentimeter
	if ( reg.cap(4).stripWhiteSpace() == "PixelsPerCentimeter" ) 
		m_resolution *= 2.54;

	// calc the bounding box
	int bbw = (int)( (float)m_width*72.0/m_resolution + 0.5 );
	int bbh = (int)( (float)m_height*72.0/m_resolution + 0.5 );

	// take width and height as parameters for the bounding box
	 edit_bb->setText( QString("0 0 ") + QString("%1").arg(bbw)
	                                   + ' '
	                                   + QString("%1").arg(bbh)
	                  );

         // show information
         setInfo();

      }
    }
}

void IncludeGraphics::slotOk()
{
	if ( checkParameter() )  {
		writeConfig();
		accept();
	}
}

bool IncludeGraphics::checkParameter()
{
	QString filename = edit_file->text().stripWhiteSpace();
	edit_file->setText(filename);
	 
	if ( filename.isEmpty() )
	{
		if ( KMessageBox::warningYesNo( this, i18n("No graphics file was given. Proceed any way?") ) == KMessageBox::No ) return false;
	}
	else
	{
		QFileInfo fi( filename );
		if ( ! fi.exists() )
		{
			if ( KMessageBox::warningYesNo( this, i18n("The graphics file does not exist. Proceed any way?") ) == KMessageBox::No )  return false;
		}
	}

	return true;
}

}

#include "includegraphicsdialog.moc"

