/* -------------------------- gnuMultiFile class --------------------------

   This class handles all operations related to the storage and retrieval of 
   multiple files and their options. These should be called from
   gnuInterface. This class allows you to implement the storage in any way
   you choose. 

   This file is part of Xgfe: X Windows GUI front end to Gnuplot
   Copyright (C) 1998 David Ishee

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   ------------------------------------------------------------------------*/
using namespace std;

#ifndef gnuMultiFile_included
#define gnuMultiFile_included

#include <qstring.h>
#include "gnuPlotFile.h"
#include <qdict.h> // Qt's dictionary data structure

class gnuMultiFile
{
public:
  gnuMultiFile();
  // Constructor function

  void insertMultiFileNew(QString filename);
  /* Incoming arguments:
       QString filename: new filename
     Outgoing arguments:
       none
     Description:
       Inserts a new file into multiple file list */

  void removeMultiFile(QString filename);
  /* Incoming arguments:
       QString filename: file to remove
     Outgoing arguments:
       none
     Description:
       Removes file from multiple file list */

  void setMultiFileDataSetStart(QString filename, QString start);
  QString getMultiFileDataSetStart(QString filename);
  void setMultiFileDataSetEnd(QString filename,QString end);
  QString getMultiFileDataSetEnd(QString filename);
  void setMultiFileDataSetIncrement(QString filename,QString inc);
  QString getMultiFileDataSetIncrement(QString filename);
  void setMultiFileSampPointInc(QString filename,QString pinc);
  QString getMultiFileSampPointInc(QString filename);
  void setMultiFileSampLineInc(QString filename,QString linc);
  QString getMultiFileSampLineInc(QString filename);
  void setMultiFileSampStartPoint(QString filename,QString startp);
  QString getMultiFileSampStartPoint(QString filename);
  void setMultiFileSampStartLine(QString filename,QString startl);
  QString getMultiFileSampStartLine(QString filename);
  void setMultiFileSampEndPoint(QString filename,QString endp);
  QString getMultiFileSampEndPoint(QString filename);
  void setMultiFileSampEndLine(QString filename,QString endl);
  QString getMultiFileSampEndLine(QString filename);
  void setMultiFileSmoothType(QString filename,QString type);
  QString getMultiFileSmoothType(QString filename);

  void insertMultiFileXColumnOption(QString filename, QString xcol);
  /* Incoming arguments:
       QString filename: file to operate on
       QString xcol: X column to plot
     Outgoing arguments:
       none
     Description:
       Sets x column for a given file in the multiple file list */

  QString getMultiFileXColumnOption(QString filename);
  /* Incoming arguments:
       QString filename: file to operate on
     Outgoing arguments:
       QString: x column for given file
     Description:
       Return x column for a given file in the multiple file list */

  void insertMultiFileYColumnOption(QString filename, QString ycol);
  /* Incoming arguments:
       QString filename: file to operate on
       QString ycol: Y column to plot
     Outgoing arguments:
       none
     Description:
       Sets y column for a given file in the multiple file list */

  QString getMultiFileYColumnOption(QString filename);
  /* Incoming arguments:
       QString filename: file to operate on
     Outgoing arguments:
       QString: y column for a given file
     Description:
       Return y column for a given file in the multiple file list */

  void insertMultiFileZColumnOption(QString filename, QString zcol);
  /* Incoming arguments:
       QString filename: file to operate on
       QString zcol: Z column to plot
     Outgoing arguments:
       none
     Description:
       Sets z column for a given file in the multiple file list */

  QString getMultiFileZColumnOption(QString filename);
  /* Incoming arguments:
       QString filename: file to operate on
     Outgoing arguments:
       QString: z column for the given file
     Description:
       Return z column for a given file in the multiple file list */

  void insertMultiFileFormatOption(QString filename, QString format);
  /* Incoming arguments:
       QString filename: file to operate on
       QString format: format for plotting
     Outgoing arguments:
       none
     Description:
       Set format for the given file in the multiple file list */

  QString getMultiFileFormatOption(QString filename);
  /* Incoming arguments:
       QString filename: file to operate on
     Outgoing arguments:
       QString: format for given file
     Description:
       Returns format for the given file in the multiple file list */

  void insertMultiFileRawFormatOption(QString filename, QString format);
  /* Incoming arguments:
       QString filename: file to operate on
       QString format: raw format QString for plotting
     Outgoing arguments:
       none
     Description:
       Set raw format for the given file in the multiple file list */

  QString getMultiFileRawFormatOption(QString filename);
  /* Incoming arguments:
       QString filename: file to operate on
     Outgoing arguments:
       QString: raw format for given file
     Description:
       Returns raw format for the given file in the multiple file list */

  void setMultiFileStyleOption(QString filename, QString style);
  /* Incoming arguments:
       QString filename: file to operate on
       QString format: style QString for plotting
     Outgoing arguments:
       none
     Description:
       Set style for the given file in the multiple file list */

  QString getMultiFileStyleOption(QString filename);
  /* Incoming arguments:
       QString filename: file to operate on
     Outgoing arguments:
       QString: style for given file
     Description:
       Returns style for the given file in the multiple file list */

  void setMultiFileFilter(QString filename, QString filter);
  /* Incoming arguments:
       QString filename: filename to operate on
       QString filter: filter command
     Outgoing arguments:
       none
     Description:
       Sets filter command for the file */

  QString getMultiFileFilter(QString filename);
  /* Incoming arguments:
       QString: filename to operate on
     Outgoing arguments:
       QString: filter command
     Description:
       Gets filter command for the file */

  void setMultiFileFilterQuoteChar(QString filename, QString quote);
  /* Incoming arguments:
       QString filename: filename to operate on
       QString quote: quoting char
     Outgoing arguments:
       none
     Description:
       Sets quoting character for the file's filter cmd */

  QString getMultiFileFilterQuoteChar(QString filename);
  /* Incoming arguments:
       QString filename: filename to operate on
     Outgoing arguments:
       QString: quoting character
     Description:
       Gets quoting character for the file's filter cmd */

  QString getMultiFileFirstFilename();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: filename for the first file
     Description:
       Returns filename for the first file in the multiple file list
       Returns END if empty list */

  QString getMultiFileFirstPlotCmd();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: command for plotting the first file in the list
     Description:
       Returns plotting command for the first file in the list */

  QString getMultiFileNextFilename();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: filename for the next file
     Description:
       Returns filename for the next file in the multiple file list
       Returns END if at the end of the list */

  QString getMultiFileNextPlotCmd();
  /* Incoming arguments:
       none
     Outgoing arguments:
       QString: command for plotting the next file in the list
     Description:
       Returns plotting command for the next file in the list
       Each time this is called, it auto-increments to next element */

  void setLegendTitle(QString file, QString title);
  /* Incoming arguments:
       file: file to operate on
       title: title for legend
     Outgoing arguments:
       none
     Description:
       Sets title to be used in legend */

  QString getLegendTitle(QString file);
  /* Incoming arguments:
       file: file to operate on
     Outgoing arguments:
       QString: title for current file
     Description:
       Gets title to be used in legend for the current file */

private:
  /* Implement a multiFileList with Qt's Dictionary data structure. 
     If you are porting to another GUI toolkit, re-implement this class */
  QDict<gnuPlotFile>* fileList;
  QDictIterator<gnuPlotFile>* fileListIterator;
  gnuPlotFile* tempFile;
};

#endif // gnuMultiFile_included
