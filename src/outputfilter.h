/***************************************************************************
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

#ifndef OUTPUTFILTER_H
#define OUTPUTFILTER_H

#include <QString>
#include <QWidget>

#include "widgets/logwidget.h"
#include "outputinfo.h"

/**An object of this class is used to parse the output messages
of any third-party tool.
 
@author Thorsten Lck
  *@author Jeroen Wijnhout
  */

class KTextEdit;

class OutputFilter : public QObject
{
    Q_OBJECT

public:
    OutputFilter();
    virtual ~OutputFilter();

protected:

public:
    virtual bool Run(const QString& logfile);

    //void setLog(const QString &log) { m_log = log; }
    const QString& log() const { return m_log; }

    void setSource(const QString &src);
    const QString& source() const  { return m_source; }
    const QString& path() const { return m_srcPath; }

Q_SIGNALS:
	void problem(int, const QString&, const OutputInfo& outputInfo = OutputInfo());
	void problems(const QList<KileWidget::LogWidget::ProblemInformation>& list);
	void output(const QString&);

protected:
    virtual short parseLine(const QString& strLine, short dwCookie);
    virtual bool OnTerminate();
    /**
    Returns the zero based index of the currently parsed line in the
    output file.
    */
    int GetCurrentOutputLine() const;

private:
    /** Number of current line in output file */
    unsigned int		m_nOutputLines;
    QString		m_log, m_source, m_srcPath;
};
#endif
