/***************************************************************************
                          outputinfo.h  -  description
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

#ifndef OUTPUTINFO_H
#define OUTPUTINFO_H

#include <qvaluelist.h>
#include <qstring.h>

/**Class for output-information of third program (e.g. Latex-Output, C-Compiler output)

@author Thorsten Lück
  *@author Jeroen Wijnhout
  */

using namespace std;

class OutputInfo
{
    public:
        OutputInfo();
        OutputInfo(QString strSrcFile, int nSrcLine, int nOutputLine, QString strError="", int nErrorID=-1);

	public:
		/** Source file where error occured. */
		QString source() const { return m_strSrcFile; }
		/** Source file where error occured. */
		void setSource(QString src) { m_strSrcFile = src; }

		/** Line number in source file of the current message */
		int sourceLine() const { return m_nSrcLine; }
		/** Line number in source file of the current message */
		void setSourceLine(int line) { m_nSrcLine =  line; }

		/** Error message */
		QString message() const { return m_strError; }
		/** Error message */
		void setMessage(QString message) { m_strError = message; }

		/** Error code */
		int type() const { return m_nErrorID; }
		/** Error code */
		void setType(int type) { m_nErrorID = type; }

		/** Line number in the output, where error was reported. */
		int outputLine() const { return m_nOutputLine; }
		/** Line number in the output, where error was reported. */
		void setOutputLine(int line) { m_nOutputLine = line; }

        /** Clears all attributes. */
        void Clear();

	private:
        QString m_strSrcFile, m_file;
        int m_nSrcLine;
        QString m_strError;
        int m_nErrorID;
        int m_nOutputLine;
};

/**Array of OutputInfo

@author Thorsten Lück
*/
typedef QValueList<OutputInfo> OutputInfoArray;
#endif
