/***************************************************************************
                          convert.cpp -  description
                             -------------------
    begin                : Sun Feb 29 2004
    copyright            : (C) 2004 by Jeroen Wijnhout
    email                : Jeroen.Wijnhout@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qstring.h>
#include <qregexp.h>
#include <qfile.h>

#include <kglobal.h>
#include <kstddirs.h>
#include <kdebug.h>
#include <kate/document.h>

#include "convert.h"

static ConvertMetaMap g_maps = ConvertMetaMap();

void ConvertMap::addPair(unsigned char c, QString enc)
{
	m_fromASCII[c] = commandIsTerminated(enc) ? enc : enc + "{}" ;
	m_fromEncoding[enc] = c;

	kdDebug() << "added pair " << m_fromASCII[c] << " ~ " << m_fromEncoding[enc] << endl;
}

bool ConvertMap::commandIsTerminated(const QString & command)
{
	static QRegExp reCommandSequences("\\\\[a-zA-Z]+$");

	return (reCommandSequences.search(command) == -1);
}

//BEGIN ConvertIO classes
ConvertIO::ConvertIO(Kate::Document *doc) :
	m_doc(doc),
	m_text(QString::null),
	m_line(QString::null),
	m_nLine(0)
{
}

QString & ConvertIO::currentLine()
{
	return m_line;
}

void ConvertIO::nextLine()
{
	m_line = m_doc->textLine(m_nLine++);
}

void ConvertIO::writeText()
{
	m_doc->setText(m_text);
}

uint ConvertIO::current()
{
	return m_nLine;
}

bool ConvertIO::done()
{
	return current() == m_doc->numLines();
}

ConvertIOFile::ConvertIOFile(Kate::Document *doc, const KURL & url) : ConvertIO(doc), m_url(url)
{
}

void ConvertIOFile::writeText()
{
	QFile qf(m_url.path());
	if ( qf.open(IO_WriteOnly) )
	{
		//read the file
		QTextStream stream( &qf );
		stream << m_text;
		qf.close();
	}
	else
		kdWarning() << "Could not open " << m_url.path() << endl;
}

ConvertBase::ConvertBase(const QString & encoding, ConvertIO * io) :
	m_io(io),
	m_encoding(encoding),
	m_map(0L)
{
}

//END ConvertIO classes

//BEGIN ConvertBase
QString ConvertBase::mapNext(uint &i)
{
	return (QString) m_io->currentLine()[i++];
}

bool ConvertBase::convert()
{
	if ( ! setMap() ) return false;

	m_io->text() = QString::null;
	do
	{
		m_io->nextLine();
		uint i = 0;
		while ( i < m_io->currentLine().length() )
		{
			m_io->text() += mapNext(i);
		}
		if ( ! m_io->done() ) m_io->text() += "\n";
	}
	while ( ! m_io->done() );

	m_io->writeText();
	return true;
}

bool ConvertBase::loadMap(const QString & encoding)
{
	static QRegExp reMap("^([0-9]+):(.*)");

	//if map already exists, replace it
	QFile qf(KGlobal::dirs()->findResource("appdata","encodings/" + encoding + ".enc"));
	if ( qf.open(IO_ReadOnly) )
	{
		ConvertMap * map = new ConvertMap();

		//read the file
		QTextStream stream( &qf );
		while ( !stream.atEnd() ) 
		{
			//parse the line
			if ( stream.readLine().find(reMap) != -1)
				map->addPair((unsigned char)reMap.cap(1).toUInt(), reMap.cap(2));
		}
		qf.close();

		//add the map
		g_maps[encoding] = map;
		return true;
	}

	return false;
}

bool ConvertBase::setMap()
{
	m_map = g_maps[m_encoding];

	if ( (m_map == 0) && loadMap(m_encoding))
	{
		m_map = g_maps[m_encoding];
	}

	return ( m_map != 0 );
}
//END ConvertBase

//BEGIN ConvertEncToASCII
QString ConvertEncToASCII::mapNext(uint &i)
{
	return m_map->contains(m_io->currentLine()[i].latin1()) ? m_map->fromASCII(m_io->currentLine()[i++].latin1()) : (QString)m_io->currentLine()[i++];
}
//END ConvertEncToASCII

//BEGIN ConvertASCIIToEnc

//i is the position of the '\'
QString ConvertASCIIToEnc::nextSequence(uint &i)
{
	//get first two characters
	QString seq = (QString)m_io->currentLine()[i++];

	if ( m_io->currentLine()[i].isLetter() )
	{
		while ( m_io->currentLine()[i].isLetter() )
			seq += (QString)m_io->currentLine()[i++];
	}
	else
		return seq + (QString)m_io->currentLine()[i++];

	return seq;
}

bool ConvertASCIIToEnc::isModifier(const QString & seq)
{
	static QRegExp reModifier("\\\\(r|c|\\\"|\\'|\\^|\\`|\\~)");
	return reModifier.exactMatch(seq);
}

QString ConvertASCIIToEnc::getSequence(uint &i)
{
	QString seq = nextSequence(i);

	if ( isModifier(seq) )
	{
		if ( seq[seq.length() - 1].isLetter() ) seq += " ";
		while ( m_io->currentLine()[i].isSpace() ) i++;
		if ( m_io->currentLine()[i] == '\\' )
			seq += nextSequence(i);
		else
			seq += (QString)m_io->currentLine()[i++];
	}

	return seq;
}

QString ConvertASCIIToEnc::mapNext(uint &i)
{
	if ( m_io->currentLine()[i] == '\\' )
	{ 
		QString seq = getSequence(i);
		kdDebug() << "'\tsequence: " << seq << endl;
		if ( m_map->contains(seq) )
		{
			if ( m_io->currentLine().mid(i, 2) == "{}" ) i = i + 2;
			return (QString)(QChar)m_map->fromEncoding(seq);
		}
		else
			return seq;
	}

	return ConvertBase::mapNext(i);
}
//END ConvertASCIIToEnc
