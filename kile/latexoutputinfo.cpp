/***************************************************************************
                          latexoutputinfo.cpp  -  description
                             -------------------
    begin                : Don Sep 18 2003
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

#include "latexoutputinfo.h"

LatexOutputInfo::LatexOutputInfo()
{
}


LatexOutputInfo::LatexOutputInfo(const QString & strSrcFile, int nSrcLine, int nOutputLine,
const QString & strError, int nErrorID)
{
    setSource(strSrcFile);
    setSourceLine(nSrcLine);
    setOutputLine(nOutputLine);
    setMessage(strError);
    setType(nErrorID);
}


/** No descriptions */
LatexOutputInfo LatexOutputInfo::operator=(const LatexOutputInfo &a)
{
    setSource(a.source());
    setSourceLine(a.sourceLine());
    setOutputLine(a.outputLine());
    setMessage(a.message());
    setType(a.type());
    return  *this;
}
