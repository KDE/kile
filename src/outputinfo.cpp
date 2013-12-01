/************************************************************************************
  Copyright (C) 2003 by Jeroen Wijnhout (wijnhout@science.uva.nl)
                2008-2012 by Michel Ludwig (michel.ludwig@kdemail.net)
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


OutputInfo::OutputInfo(const QString& mainSourceFile, const QString& strSrcFile, int nSrcLine, int nOutputLine,
                       const QString& strError, int nErrorID /*=-1*/) :
    m_mainSourceFile(mainSourceFile),
    m_strSrcFile(strSrcFile),
    m_nSrcLine(nSrcLine),
    m_strError(strError),
    m_nOutputLine(nOutputLine),
    m_nErrorID(nErrorID)
{
}

void OutputInfo::clear()
{
	m_mainSourceFile.clear();
	m_strSrcFile.clear();
	m_nSrcLine = -1;
	m_nOutputLine = -1;
	m_strError.clear();
	m_nErrorID = -1;
}

bool OutputInfo::operator==(const OutputInfo& info) const
{
	return (m_mainSourceFile == info.m_mainSourceFile
	     && m_strSrcFile == info.m_strSrcFile
	     && m_nSrcLine == info.m_nSrcLine
	     && m_strError == info.m_strError
	     && m_nOutputLine == info.m_nOutputLine
	     && m_nErrorID == info.m_nErrorID);
}

bool OutputInfo::isValid() const
{
	return !(m_mainSourceFile.isEmpty() && m_strSrcFile.isEmpty() && m_nSrcLine == -1 && m_nOutputLine == -1
	                                    && m_strError.isEmpty() && m_nErrorID == -1);
}

LatexOutputInfo::LatexOutputInfo() : OutputInfo()
{
}


LatexOutputInfo::LatexOutputInfo(const QString& mainSourceFile, const QString& strSrcFile, int nSrcLine, int nOutputLine,
                                 const QString& strError, int nErrorID)
: OutputInfo(mainSourceFile, strSrcFile, nSrcLine, nOutputLine, strError, nErrorID)
{
}

/**
 * LatexOutputHandler
 */

LaTeXOutputHandler::LaTeXOutputHandler()
: m_nErrors(-1), m_nWarnings(-1), m_nBadBoxes(-1), m_currentError(-1)
{
}

LaTeXOutputHandler::~LaTeXOutputHandler()
{

}

void LaTeXOutputHandler::storeLaTeXOutputParserResult(int nErrors, int nWarnings, int nBadBoxes,
                                                                                  const LatexOutputInfoArray& outputList,
                                                                                  const QString& logFile)
{
	m_nErrors = nErrors;
	m_nWarnings = nWarnings;
	m_nBadBoxes = nBadBoxes;
	m_latexOutputInfoList = outputList;
	m_logFile = logFile;
	m_currentError = -1;
}

int LaTeXOutputHandler::numberOfWarnings() const
{
	return m_nWarnings;
}

int LaTeXOutputHandler::numberOfErrors() const
{
	return m_nErrors;
}

int LaTeXOutputHandler::numberOfBadBoxes() const
{
	return m_nBadBoxes;
}

LatexOutputInfoArray LaTeXOutputHandler::outputList() const
{
	return m_latexOutputInfoList;
}

QString LaTeXOutputHandler::logFile() const
{
	return m_logFile;
}

int LaTeXOutputHandler::currentError() const
{
	return m_currentError;
}

void LaTeXOutputHandler::setCurrentError(int i)
{
	m_currentError = i;
}

const KileTool::ToolConfigPair& LaTeXOutputHandler::bibliographyBackendToolUserOverride() const
{
	return m_userOverrideBibBackendToolConfigPair;
}

void LaTeXOutputHandler::setBibliographyBackendToolUserOverride(const KileTool::ToolConfigPair& p)
{
	m_userOverrideBibBackendToolConfigPair = p;
}

const KileTool::ToolConfigPair& LaTeXOutputHandler::bibliographyBackendToolAutoDetected() const
{
	return m_autodetectBibBackendToolConfigPair;
}

void LaTeXOutputHandler::setBibliographyBackendToolAutoDetected(const KileTool::ToolConfigPair& p)
{
	m_autodetectBibBackendToolConfigPair = p;
}

void LaTeXOutputHandler::readBibliographyBackendSettings(const KConfigGroup& group)
{
	const QString& bibBackendUserOverride = group.readEntry("bibliographyBackendUserOverride", QString());
	m_userOverrideBibBackendToolConfigPair = KileTool::ToolConfigPair::fromConfigStringRepresentation(bibBackendUserOverride);

	const QString& bibBackendAutoDetected = group.readEntry("bibliographyBackendAutoDetected", QString());
	m_autodetectBibBackendToolConfigPair = KileTool::ToolConfigPair::fromConfigStringRepresentation(bibBackendAutoDetected);
}

void LaTeXOutputHandler::writeBibliographyBackendSettings(KConfigGroup& group)
{
	group.writeEntry("bibliographyBackendUserOverride", m_userOverrideBibBackendToolConfigPair.configStringRepresentation());
	group.writeEntry("bibliographyBackendAutoDetected", m_autodetectBibBackendToolConfigPair.configStringRepresentation());
}
