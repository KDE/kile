/***************************************************************************
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

#ifndef LATEXOUTPUTINFO_H
#define LATEXOUTPUTINFO_H

#include "outputinfo.h"

#include <QList>

/**A class to keep output info from LaTeX.
 *@author Thorsten Lck
 */

class LatexOutputInfo : public OutputInfo
{
    public:
        LatexOutputInfo();
        LatexOutputInfo(const QString & strSrcFile, int nSrcLine, int nOutputLine, const QString & strError="", int nErrorID=-1);
        /** No descriptions */
        LatexOutputInfo operator=(const LatexOutputInfo &a) ;

    public:
        /**
        These constants are describing, which item types is currently
        parsed. (to be set as error code)
        */
        enum tagCookies
        {
            itmNone = 0,
            itmError,
            itmWarning,
            itmBadBox
        };
};

/**Array of OutputInfo

@author Thorsten Lck
*/
typedef QList<LatexOutputInfo> LatexOutputInfoArray;
#endif
