/***************************************************************************
                           includegraphicsdialog.h
----------------------------------------------------------------------------
    date                 : Jan 23 2004
    version              : 0.10.2
    copyright            : (C) 2004 by Holger Danielsson
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

#ifndef INCLUDEGRAPHICSDIALOG_H
#define INCLUDEGRAPHICSDIALOG_H

#include <qdialog.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qstring.h>
#include <kprocess.h>
#include <kconfig.h>

/**
  *@author dani
  */

class IncludegraphicsDialog : public QDialog  {
    Q_OBJECT
    
public:
   IncludegraphicsDialog(QWidget *parent,KConfig *config,const QString &startdir,bool pdflatex);
   ~IncludegraphicsDialog();

   QString getTemplate();
   
private slots:
   void chooseFile();
   void updateFigure();
   void checkParameter();
   
   void slotProcessOutput(KProcess* proc,char* buffer,int buflen);
   void slotProcessExited(KProcess* proc);

private:
   QString getOptions();
   QString getInfo();
   bool getPictureSize(int &wpx, int &hpx, QString &wcm, QString &hcm);
   void setInfo();

   QLabel *infolabel;
   QLineEdit *edit_file;
   QLineEdit *edit_label;
   QLineEdit *edit_caption;
   QLineEdit *edit_width;
   QLineEdit *edit_height;
   QLineEdit *edit_angle;
   QLineEdit *edit_bb;
   QCheckBox *cb_center, *cb_pdftex, *cb_figure;
   QLabel *lb_label, *lb_caption;
   
   QString m_startdir;
   QString m_output;

   // current picture
   bool m_pdflatex;
   float m_resolution;

   // default
   bool m_imagemagick;
   bool m_boundingbox;
   float m_defaultresolution;
   
   void execute(const QString &command);

};

#endif
