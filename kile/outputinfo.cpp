/***************************************************************************
                          outputinfo.cpp  -  description
                             -------------------
    begin                : Die Sep 16 2003
    copyright            : (C) 2003 by Jeroen Wijnhout
    email                : wijnhout@science.uva.nl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "outputinfo.h"

OutputInfo::OutputInfo()
{
    OutputInfo("",-1,-1);
}


OutputInfo::OutputInfo(const QString & strSrcFile, int nSrcLine, int nOutputLine,
const QString & strError , int nErrorID /*=-1*/) :
    m_strSrcFile(strSrcFile),
    m_nSrcLine(nSrcLine),
    m_strError(strError),
    m_nOutputLine(nOutputLine),
    m_nErrorID(nErrorID)
{
}

/** Clears all attributes. */
void OutputInfo::Clear()
{
    m_strSrcFile = "";
    m_nSrcLine = -1;
    m_nOutputLine = -1;
    m_strError = "";
    m_nErrorID = -1;
}
