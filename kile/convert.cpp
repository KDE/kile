/***************************************************************************
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

#include <qregexp.h>
#include <qtextcodec.h>
#include <qfile.h>

#include <kmessagebox.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kate/document.h>

#include "convert.h"

QMap<QString, ConvertMap*> ConvertMap::g_maps;

bool ConvertMap::create(const QString & encoding)
{
	kdDebug() << "\tlooking for map for " << encoding << endl;
	ConvertMap * map = g_maps[encoding];

	if ( map == 0 )
	{
		kdDebug() << "\tcreating a map for " << encoding << endl;
		map = new ConvertMap(encoding); // FIXME This will never be deleted if load() succeeds...
		if ( map->load() )
			g_maps[encoding] = map;
        else {
			delete map;
			map = 0L;
        }

		map = g_maps[encoding];
	}

	return ( map != 0L );
}

QString ConvertMap::encodingNameFor(const QString & name)
{
	QString std;
	for ( uint i = 0; i < name.length(); ++i )
		if ( !name[i].isSpace() )
			std += name[i];

	std = std.lower();

	if ( std.startsWith("iso8859-") )
		return "latin" + std.right(1);

	if ( std.startsWith("cp") )
		return "cp" + std.right(4);

	return name;
}

QString ConvertMap::isoNameFor(const QString & name)
{
	QString std;
	for ( uint i = 0; i < name.length(); ++i )
		if ( !name[i].isSpace() )
			std += name[i];

	std = std.lower();

	if ( std.startsWith("latin") )
		return "ISO 8859-" + std.right(1);

	if ( std.startsWith("cp" ) )
		return "cp " + std.right(4);

	return name;
}

ConvertMap::ConvertMap(const QString & enc )
{
	m_aliases.append(encodingNameFor(enc));
	m_aliases.append(isoNameFor(enc));
}

void ConvertMap::addPair(QChar c, const QString & enc)
{
	m_toASCII[c] = commandIsTerminated(enc) ? enc : enc + "{}" ;
	m_toEncoding[enc] = c;
}

bool ConvertMap::commandIsTerminated(const QString & command)
{
	static QRegExp reCommandSequences("\\\\([a-zA-Z]+|\\\"|\\')$");

	return (reCommandSequences.search(command) == -1);
}

bool ConvertMap::load()
{
	static QRegExp reMap("^(.*):(.*)");

	//makeMap(encoding());

	//if map already exists, replace it
	QFile qf(KGlobal::dirs()->findResource("appdata","encodings/" + encoding() + ".enc"));

	if ( qf.open(IO_ReadOnly) )
	{
		QTextStream stream( &qf );
		QTextCodec *codec = QTextCodec::codecForName(isoName().ascii());
		if ( codec ) stream.setCodec(codec);

		while ( !stream.atEnd() ) 
		{
			//parse the line
			if ( stream.readLine().find(reMap) != -1)
				addPair(reMap.cap(1)[0], reMap.cap(2));
		}
		qf.close();

		return true;
	}

	return false;
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
	return (QString)m_io->currentLine()[i++];
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
		if ( ! m_io->done() ) m_io->text() += '\n';
	}
	while ( ! m_io->done() );

	m_io->writeText();
	return true;
}

bool ConvertBase::setMap()
{
	//create map (or use existing)
	if  (ConvertMap::create(m_encoding))
		m_map = ConvertMap::mapFor(m_encoding);
	else
		m_map = 0L;

	return ( m_map != 0L );
}
//END ConvertBase

//BEGIN ConvertEncToASCII
QString ConvertEncToASCII::mapNext(uint &i)
{
	return m_map->canDecode(m_io->currentLine()[i]) ? m_map->toASCII(m_io->currentLine()[i++]) : (QString)m_io->currentLine()[i++];
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
	static QRegExp reModifier("\\\\([cHkruv]|\"|\'|\\^|`|~|=|\\.)");
	return reModifier.exactMatch(seq);
}

QString ConvertASCIIToEnc::getSequence(uint &i)
{
	QString seq = nextSequence(i);
	static QRegExp reBraces("\\{([a-zA-Z]?)\\}");

	if ( isModifier(seq) )
	{
		kdDebug() << "\tisModifier true : " << seq << endl;
		if ( seq[seq.length() - 1].isLetter() ) seq += ' ';

		while ( m_io->currentLine()[i].isSpace() ) i++;

		if ( m_io->currentLine().mid(i,2) == "{}" ) i = i + 2;

		if ( m_io->currentLine()[i] == '\\' )
			seq += nextSequence(i);
		else
		{
			if ( reBraces.exactMatch(m_io->currentLine().mid(i,3)) )
			{
				kdDebug() << "\tbraces detected" << endl;
				i = i + 3;
				seq += reBraces.cap(1);
			}
			else
			{
				QChar nextChar = m_io->currentLine()[i++];
				if ( !nextChar.isSpace() ) seq += (QString)nextChar;
			}
		}
	}
	else if ( m_map->canEncode(seq) )
	{
		if ( m_io->currentLine().mid(i,2) == "{}" ) i = i + 2;
		else if ( m_io->currentLine()[i].isSpace() ) ++i;
	}

	return seq;
}

QString ConvertASCIIToEnc::mapNext(uint &i)
{
	if ( m_io->currentLine()[i] == '\\' )
	{ 
		QString seq = getSequence(i);
		kdDebug() << "'\tsequence: " << seq << endl;
		if ( m_map->canEncode(seq) )
		{
			kdDebug() << "\tcan encode this" << endl;
			//if ( m_io->currentLine().mid(i, 2) == "{}" ) i = i + 2;
			return m_map->toEncoding(seq);
		}
		else
			return seq;
	}

	return ConvertBase::mapNext(i);
}
//END ConvertASCIIToEnc
