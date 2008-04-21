/************************************************************************************
    begin                : Die Sep 16 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (wijnhout@science.uva.nl)
                               2008 by Michel Ludwig (michel.ludwig@kdemail.net)
 ************************************************************************************/

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
	clear();
}


OutputInfo::OutputInfo(const QString& strSrcFile, int nSrcLine, int nOutputLine,
                       const QString& strError, int nErrorID /*=-1*/) :
    m_strSrcFile(strSrcFile),
    m_nSrcLine(nSrcLine),
    m_strError(strError),
    m_nOutputLine(nOutputLine),
    m_nErrorID(nErrorID)
{
}

void OutputInfo::clear()
{
	m_strSrcFile.clear();
	m_nSrcLine = -1;
	m_nOutputLine = -1;
	m_strError.clear();
	m_nErrorID = -1;
}

bool OutputInfo::operator==(const OutputInfo& info) const
{
	return (m_strSrcFile == info.m_strSrcFile
	     && m_nSrcLine == info.m_nSrcLine
	     && m_strError == info.m_strError
	     && m_nOutputLine == info.m_nOutputLine
	     && m_nErrorID == m_nErrorID);
}

bool OutputInfo::isValid() const
{
	return !(m_strSrcFile.isEmpty() && m_nSrcLine == -1 && m_nOutputLine == -1
	                                && m_strError.isEmpty() && m_nErrorID == -1);
}

LatexOutputInfo::LatexOutputInfo() : OutputInfo()
{
}


LatexOutputInfo::LatexOutputInfo(const QString& strSrcFile, int nSrcLine, int nOutputLine,
                                 const QString& strError, int nErrorID)
: OutputInfo(strSrcFile, nSrcLine, nOutputLine, strError, nErrorID)
{
}
