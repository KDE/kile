/* -------------------------- gnuMultiFile class --------------------------

   This class handles all operations related to the storage and retrieval of 
   multiple files and their options. These should be called from
   gnuInterface. 

   It is currently implemented with Qt's dictionary datastructure for keeping
   up with a list of plotFileOb's. 

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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   ------------------------------------------------------------------------*/

#include "gnuMultiFile.h"

gnuMultiFile::gnuMultiFile()
{
  // create new filelist
  fileList = new QDict<gnuPlotFile>(101,TRUE); // max 100 elements
  fileList->setAutoDelete(TRUE); // autodelete members when removed

  // create new iterator
  fileListIterator = new QDictIterator<gnuPlotFile>(*fileList);
}

void gnuMultiFile::insertMultiFileNew(QString filename)
{
  gnuPlotFile* thisFile = new gnuPlotFile; // create a new plotfile
    
  thisFile->setFilename(filename); // set filename of plotfile
  fileList->insert(filename,thisFile); // insert into list

}

void gnuMultiFile::removeMultiFile(QString filename)
{
  fileList->remove(filename);
}

void gnuMultiFile::setMultiFileDataSetStart(QString filename, QString start)
{
  tempFile = (*fileList)[filename];
  tempFile->setFileDataSetStart(start);
}

QString gnuMultiFile::getMultiFileDataSetStart(QString filename)
{
  tempFile = (*fileList)[filename];
  return tempFile->getFileDataSetStart();
}

void gnuMultiFile::setMultiFileDataSetEnd(QString filename,QString end)
{
  tempFile = (*fileList)[filename];
  tempFile->setFileDataSetEnd(end);
}

QString gnuMultiFile::getMultiFileDataSetEnd(QString filename)
{
  tempFile = (*fileList)[filename];
  return tempFile->getFileDataSetEnd();
}

void gnuMultiFile::setMultiFileDataSetIncrement(QString filename,QString inc)
{
  tempFile = (*fileList)[filename];
  tempFile->setFileDataSetIncrement(inc);
}

QString gnuMultiFile::getMultiFileDataSetIncrement(QString filename)
{
  tempFile = (*fileList)[filename];
  return tempFile->getFileDataSetIncrement();
}

void gnuMultiFile::setMultiFileSampPointInc(QString filename,QString pinc)
{
  tempFile = (*fileList)[filename];
  tempFile->setFileSampPointInc(pinc);
}

QString gnuMultiFile::getMultiFileSampPointInc(QString filename)
{
  tempFile = (*fileList)[filename];
  return tempFile->getFileSampPointInc();
}

void gnuMultiFile::setMultiFileSampLineInc(QString filename,QString linc)
{
  tempFile = (*fileList)[filename];
  tempFile->setFileSampLineInc(linc);
}

QString gnuMultiFile::getMultiFileSampLineInc(QString filename)
{
  tempFile = (*fileList)[filename];
  return tempFile->getFileSampLineInc();
}

void gnuMultiFile::setMultiFileSampStartPoint(QString filename,QString startp)
{
  tempFile = (*fileList)[filename];
  tempFile->setFileSampStartPoint(startp);
}

QString gnuMultiFile::getMultiFileSampStartPoint(QString filename)
{
  tempFile = (*fileList)[filename];
  return tempFile->getFileSampStartPoint();
}

void gnuMultiFile::setMultiFileSampStartLine(QString filename,QString startl)
{
  tempFile = (*fileList)[filename];
  tempFile->setFileSampStartLine(startl);
}

QString gnuMultiFile::getMultiFileSampStartLine(QString filename)
{
  tempFile = (*fileList)[filename];
  return tempFile->getFileSampStartLine();
}

void gnuMultiFile::setMultiFileSampEndPoint(QString filename,QString endp)
{
  tempFile = (*fileList)[filename];
  tempFile->setFileSampEndPoint(endp);
}

QString gnuMultiFile::getMultiFileSampEndPoint(QString filename)
{
  tempFile = (*fileList)[filename];
  return tempFile->getFileSampEndPoint();
}

void gnuMultiFile::setMultiFileSampEndLine(QString filename,QString endl)
{
  tempFile = (*fileList)[filename];
  tempFile->setFileSampEndLine(endl);
}

QString gnuMultiFile::getMultiFileSampEndLine(QString filename)
{
  tempFile = (*fileList)[filename];
  return tempFile->getFileSampEndLine();
}

void gnuMultiFile::setMultiFileSmoothType(QString filename,QString type)
{
  tempFile = (*fileList)[filename];
  tempFile->setFileSmoothType(type);
}

QString gnuMultiFile::getMultiFileSmoothType(QString filename)
{
  tempFile = (*fileList)[filename];
  return tempFile->getFileSmoothType();
}

void gnuMultiFile::insertMultiFileXColumnOption(QString filename, QString xcol)
{
  tempFile = (*fileList)[filename];
  tempFile->setFileXColumn(xcol);
}

QString gnuMultiFile::getMultiFileXColumnOption(QString filename)
{
  tempFile = (*fileList)[filename];
  return tempFile->getFileXColumn();
}

void gnuMultiFile::insertMultiFileYColumnOption(QString filename, QString ycol)
{
  tempFile = (*fileList)[filename];
  tempFile->setFileYColumn(ycol);
}

QString gnuMultiFile::getMultiFileYColumnOption(QString filename)
{
  tempFile = (*fileList)[filename];
  return tempFile->getFileYColumn();
}

void gnuMultiFile::insertMultiFileZColumnOption(QString filename, QString zcol)
{
  tempFile = (*fileList)[filename];
  tempFile->setFileZColumn(zcol);
}

QString gnuMultiFile::getMultiFileZColumnOption(QString filename)
{
  tempFile = (*fileList)[filename];
  return tempFile->getFileZColumn();
}

void gnuMultiFile::insertMultiFileFormatOption(QString filename, QString format)
{
  tempFile = (*fileList)[filename];
  tempFile->setFileFormatString(format);
}

QString gnuMultiFile::getMultiFileFormatOption(QString filename)
{
  tempFile = (*fileList)[filename];
  return tempFile->getFileFormatString();
}

void gnuMultiFile::insertMultiFileRawFormatOption(QString filename, QString format)
{
  tempFile = (*fileList)[filename];
  tempFile->setRawFileFormatString(format);
}

QString gnuMultiFile::getMultiFileRawFormatOption(QString filename)
{
  tempFile = (*fileList)[filename];
  return tempFile->getRawFileFormatString();
}

void gnuMultiFile::setMultiFileStyleOption(QString filename, QString style)
{
  tempFile = (*fileList)[filename];
  tempFile->setFileStyleType(style);
}

QString gnuMultiFile::getMultiFileStyleOption(QString filename)
{
  tempFile = (*fileList)[filename];
  return tempFile->getFileStyleType();
}

void gnuMultiFile::setMultiFileFilter(QString filename, QString filter)
{
  tempFile = (*fileList)[filename];
  tempFile->setFileFilter(filter);
}

QString gnuMultiFile::getMultiFileFilter(QString filename)
{
  tempFile = (*fileList)[filename];
  return tempFile->getFileFilter();
}

void gnuMultiFile::setMultiFileFilterQuoteChar(QString filename, QString quote)
{
  tempFile = (*fileList)[filename];
  tempFile->setFileFilterQuoteChar(quote);
}

QString gnuMultiFile::getMultiFileFilterQuoteChar(QString filename)
{
  tempFile = (*fileList)[filename];
  return tempFile->getFileFilterQuoteChar();
}


QString gnuMultiFile::getMultiFileFirstFilename()
{
  // set iterator to first element
  tempFile = fileListIterator->toFirst();

  // check for error (empty list = null)
  if (tempFile == 0)
    return "END";
  else
  {
    // get and return filename
    return tempFile->getFilename();
  }
}

QString gnuMultiFile::getMultiFileFirstPlotCmd()
{
  // set iterator to first element
  tempFile = fileListIterator->toFirst();

  // check for error (empty list = null)
  if (tempFile == 0)
    return "END";
  else
  {
    // get and return plot command
    return tempFile->getPlotCmd();
  }
}

QString gnuMultiFile::getMultiFileNextFilename()
{
  // increment filelist iterator
  tempFile = ++(*fileListIterator);

  // check for error (end of list = null)
  if (tempFile == 0)
    return "END";
  else
  {
    // get and return filename
    return tempFile->getFilename();
  }
}

QString gnuMultiFile::getMultiFileNextPlotCmd()
{
  // increment filelist iterator
  tempFile = ++(*fileListIterator);

  // check for error (end of list = null)
  if (tempFile == 0)
    return "END";
  else
  {
    // get and return plot command
    return tempFile->getPlotCmd();
  }
}

void gnuMultiFile::setLegendTitle(QString file, QString title)
{
  tempFile = (*fileList)[file];
  tempFile->setLegendTitle(title);
}

QString gnuMultiFile::getLegendTitle(QString file)
{
  tempFile = (*fileList)[file];
  return tempFile->getLegendTitle();
}
