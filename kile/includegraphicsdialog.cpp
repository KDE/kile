/***************************************************************************
                           includegraphicsdialog.cpp
----------------------------------------------------------------------------
    date                 : Jul 31 2004
    version              : 0.10.3
    copyright            : (C) 2004 by Holger Danielsson, 2004 Jeroen Wijnhout
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

#include <klocale.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kpushbutton.h>

#include "kileconfig.h"

namespace KileDialog
{

IncludeGraphics::IncludeGraphics(QWidget *parent, const QString &startdir, bool pdflatex) :
	KDialogBase( Plain, i18n("Include Graphics"), Ok | Cancel, Ok, parent, 0, true, true),
	m_startdir(startdir),
	m_pdflatex(pdflatex)
{
   // Layout
   QVBoxLayout *vbox = new QVBoxLayout(plainPage(), 6,6 );
   
   // first groupbox: choose picture
   QVGroupBox* group= new QVGroupBox(i18n("File"), plainPage());

   QWidget *widget = new QWidget(group);
   QGridLayout *grid = new QGridLayout( widget, 3,3, 6,6, "");
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
   QGridLayout *cb_grid = new QGridLayout( cb_widget, 1,2, 5,5,"");
   cb_center = new QCheckBox(i18n("Center picture"),cb_widget);
   cb_pdftex = new QCheckBox(i18n("pdftex/pdflatex"),cb_widget);
   cb_center->setChecked(true);                             // default: always on
   cb_pdftex->setChecked(m_pdflatex);                       // default: on when using pdftex
   cb_grid->addWidget(cb_center,0,0);
   cb_grid->addWidget(cb_pdftex,0,1);

   grid->addWidget( label3, 2,0 );
   grid->addWidget( cb_widget, 2,1 );

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
   edit_label = new KLineEdit("Fig:",widget_fig);
   edit_caption = new KLineEdit("",widget_fig);

   grid_fig->addWidget( label4,0,0);
   grid_fig->addWidget( cb_figure, 0,1);
   grid_fig->addWidget( lb_label,1,0);
   grid_fig->addWidget( edit_label, 1,1);
   grid_fig->addWidget( lb_caption,2,0);
   grid_fig->addWidget( edit_caption, 2,1);

   // init
   cb_figure->setChecked(false);
   updateFigure();

   // add to layout
   vbox->addWidget(group);
   vbox->addWidget(gb_opt);
   vbox->addWidget(gb_fig);
   vbox->addStretch();

   // connect
   connect( pb_choose, SIGNAL( clicked() ), this, SLOT( chooseFile() ) );
   connect( cb_figure, SIGNAL(clicked()), this, SLOT(updateFigure() ) );

   // read configuration
   m_imagemagick = KileConfig::imagemagick();
   m_boundingbox = KileConfig::boundingbox();                     // dani 31.7.2004

   m_defaultresolution = KileConfig::resolution();

   setFocusProxy( edit_file );
}

IncludeGraphics::~IncludeGraphics()
{}

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
   QString s = "";

   // state of figure environment
   bool m_figure = cb_figure->isChecked();

   // add start of figure environment ?
   if ( m_figure )
      s += "\\begin{figure}\n";

   // add start of center environment ?
    if ( cb_center->isChecked() )
       if ( m_figure )
          s += "\\centering\n";
       else
          s += "\\begin{center}\n";

   // add inclucegraphics command
   s += "\\includegraphics";

   // add some options
   QString options = getOptions();
   if ( options != "" )
      s += "[" + options + "]";

   // add name of picture
   // (try to take the relative part of the name)
   QString filename = edit_file->text();
   if ( filename.find(m_startdir+"/",0) == 0 )
       filename = filename.remove(0,m_startdir.length()+1);
   s += "{" + filename + "}\n";

   // add some comments (depending of given resolution, this may be wrong!)
   s += getInfo() + "\n";

   // close center environment ?
   if ( cb_center->isChecked() && !m_figure )
      s += "\\end{center}\n";

   // close figure environment?
   if ( m_figure ) 
   {
      if ( ! edit_caption->text().isEmpty() )
         s +=  "\\caption{" + edit_caption->text() + "}\n";
	  if ( !edit_label->text().isEmpty() && edit_label->text()!="fig:" )
         s +=  "\\label{" + edit_label->text() + "}\n";

      s += "\\end{figure}\n";
   }

   return s;
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
   QString wcm,hcm;
   int wpx,hpx;

   bool ok = getPictureSize(wpx,hpx,wcm,hcm);
   if ( ! ok )
      return "";
   else
   {
      QFileInfo fi( edit_file->text() );

      return "% " + fi.baseName() + "." + fi.extension(true)
                  + ": " + QString("%1").arg(m_resolution) + "dpi"
                  + ", width=" + wcm + "cm"
                  + ", height=" + hcm + "cm"
                  + ", bb=" + edit_bb->text();
    }
}

void IncludeGraphics::setInfo()
{
   QString text;
   QString wcm,hcm,dpi;
   int wpx,hpx;

   if ( !edit_file->text().isEmpty() && getPictureSize(wpx,hpx,wcm,hcm) ) {
      if ( m_pdflatex ) {
         text = QString("%1x%2 pixel").arg(wpx).arg(hpx)
                   + " / " + wcm + "x" + hcm + " cm"
                   + "  (" + QString("%1").arg(m_resolution) + "dpi)";
      } else {
        text = "bb: " + edit_bb->text()
                      + " / " + wcm + "x" + hcm + " cm"
                      + "  (" + QString("%1").arg(m_resolution) + "dpi)";
      }
   }
   else
      text = "---";

   // insert text
   infolabel->setText(text);
}

bool IncludeGraphics::getPictureSize(int &wpx, int &hpx, QString &wcm, QString &hcm)
{
   float eps[4], width,height, w,h;
   bool ok;

   QRegExp reg("([-\\.0-9]+)\\s+([-\\.0-9]+)\\s+([-\\.0-9]+)\\s+([-\\.0-9]+)");
   QString boundingbox = edit_bb->text();
   if ( reg.search(boundingbox) == -1 )
      return false;

   for (uint i=0; i<4; i++) {
      eps[i] = reg.cap(i+1).toFloat( &ok );
      if (!ok) return false;
   }

   width = eps[2] - eps[0];
   height = eps[3] - eps[1];

   if ( width>0 && height>0 )
   {
      // use the extracted bounding box
      wpx = (int)width;
      hpx = (int)height;

      // die anderen Werte werden jetzt berechnet
      // dazu muss die Aufl�ung angegeben werden
      // Bitmap-Dateien in 300 dpi, EPS-Dateien in 72.27 dpi
      // da aber alle EPS-Dateien von Bitmaps abstammen,
      // wird immer 300dpi genommen

      // try to calculate the real size
      w = ((float)width  * 2.54) / (float)m_resolution;
      h = ((float)height * 2.54) / (float)m_resolution;

      // convert from inch to cm
      wcm = wcm.setNum(w,'f',2);
      hcm = hcm.setNum(h,'f',2);
      return true;
   }
   else
      return false;
}


void IncludeGraphics::chooseFile()
{
   QString filter = ( m_pdflatex )
                  ? QString("*.png *.jpg *.pdf|Graphics\n")              // dani  31.7.2004
                          + "*.png|PNG files\n"
                          + "*.jpg|JPG files\n"
                          + "*.pdf|PDF files\n"
                          + "*|All files"
                  : QString("*.png *.jpg *.eps.gz *.eps|Graphics\n")     // dani  31.7.2004
                          + "*.png|PNG files\n" 
                          + "*.jpg|JPG files\n"
                          + "*.eps.gz|zipped EPS files\n"
                          + "*.eps|EPS files\n"
                          + "*|All files";

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

   KShellProcess* proc = new KShellProcess("/bin/sh");
   proc->clearArguments();
   (*proc) << QStringList::split(' ',command);

   connect(proc, SIGNAL(receivedStdout(KProcess*,char*,int)),
           this, SLOT(slotProcessOutput(KProcess*,char*,int)) );
   connect(proc, SIGNAL(receivedStderr(KProcess*,char*,int)),
           this, SLOT(slotProcessOutput(KProcess*,char*,int)) );
   connect(proc, SIGNAL(processExited(KProcess*)),
           this, SLOT(slotProcessExited(KProcess*)) );

   m_output = "";
   kdDebug() << "=== IncludeGraphics::execute ====================" << endl;
   kdDebug() << "   execute '" << command << "'" << endl;

   proc->start(KProcess::NotifyOnExit, KProcess::AllOutput);
}

// get all output of identify

void IncludeGraphics::slotProcessOutput(KProcess*,char* buffer,int buflen)
{
   m_output += QCString(buffer,buflen+1);
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
	 int bbw = (int)reg.cap(1).toInt( &ok);
         if (!ok) return;
	 
	 int bbh = (int)reg.cap(2).toInt( &ok);
         if (!ok) return;
	 
         float res = (float)reg.cap(3).toFloat( &ok);
         if (!ok) return;

	 // look, if res is in PixelsPerCentimeter
	 if ( reg.cap(4).stripWhiteSpace() == "PixelsPerCentimeter" ) {
	    bbw = (int)( (float)bbw/2.54 + 0.5 ); 
	    bbh = (int)( (float)bbh/2.54 + 0.5 );
	    res *= 2.54;
	 }
	    
         // There is no resolution in jpeg files f.e., so if
         // the calculated resolution is acceptable, take it.
         // Otherwise use the default resolution;
         if ( res > 0.0 )
            m_resolution = res;

         // take width and height as parameters for the bounding box
	 edit_bb->setText( QString("0 0 ") + QString("%1").arg(bbw) 
	                                   + " " 
	                                   + QString("%1").arg(bbh)
                         );

         // show information
         setInfo();
      
      }
    }
}

void IncludeGraphics::slotOk()
{
	if ( checkParameter() ) accept();
}

bool IncludeGraphics::checkParameter()
{
	if ( edit_file->text().isEmpty() )
	{
		if ( KMessageBox::warningYesNo( this, i18n("No graphics file was given. Proceed any way?") ) == KMessageBox::No ) return false;
	}
	else
	{
		QFileInfo fi( edit_file->text() );
		if ( ! fi.exists() )
		{
			if ( KMessageBox::warningYesNo( this, i18n("The graphics file does not exist. Proceed any way?") ) == KMessageBox::No )  return false;
		}
	}

	return true;
}

}

#include "includegraphicsdialog.moc"

