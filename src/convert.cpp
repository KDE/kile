/*************************************************************************************
    begin                : Sun Feb 29 2004
    copyright            : (C) 2004 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
 *************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "convert.h"

#include <QFile>
#include <QRegExp>
#include <QTextCodec>
#include <QTextStream>

#include <KGlobal>
#include <KMessageBox>
#include <KStandardDirs>
#include <KTextEditor/Document>

#include "kiledebug.h"

QMap<QString, ConvertMap*> ConvertMap::g_maps;

bool ConvertMap::create(const QString & encoding)
{
	KILE_DEBUG() << "\tlooking for map for " << encoding;
	ConvertMap * map = g_maps[encoding];

	if(!map) {
		KILE_DEBUG() << "\tcreating a map for " << encoding;
		map = new ConvertMap(encoding); // FIXME This will never be deleted if load() succeeds...
		if(map->load()) {
			g_maps[encoding] = map;
		}
		else {
				delete map;
				map = NULL;
		}
		map = g_maps[encoding];
	}

	return (map != NULL);
}

QString ConvertMap::encodingNameFor(const QString & name)
{
	QString std;
	for(int i = 0; i < name.length(); ++i) {
		if(!name[i].isSpace()) {
			std += name[i];
		}
	}

	std = std.toLower();

	if(std.startsWith("iso8859-")) {
		return "latin" + std.right(1);
	}
	
	if(std.startsWith("cp")) {
		return "cp" + std.right(4);
	}
	
	return name;
}

QString ConvertMap::isoNameFor(const QString & name)
{
	QString std;
	for(int i = 0; i < name.length(); ++i) {
		if(!name[i].isSpace()) {
			std += name[i];
		}
	}

	std = std.toLower();

	if(std.startsWith("latin")) {
		return "ISO 8859-" + std.right(1);
	}
	
	if(std.startsWith("cp")) {
		return "cp " + std.right(4);
	}
	
	return name;
}

ConvertMap::ConvertMap(const QString& enc)
{
	m_aliases.append(encodingNameFor(enc));
	m_aliases.append(isoNameFor(enc));
}

void ConvertMap::addPair(QChar c, const QString& enc)
{
	m_toASCII[c] = commandIsTerminated(enc) ? enc : enc + "{}" ;
	m_toEncoding[enc] = c;
}

bool ConvertMap::commandIsTerminated(const QString & command)
{
	static QRegExp reCommandSequences("\\\\([a-zA-Z]+|\\\"|\\')$");

	return (reCommandSequences.indexIn(command) == -1);
}

bool ConvertMap::load()
{
	static QRegExp reMap("^(.*):(.*)");

	//makeMap(encoding());

	//if map already exists, replace it
	QFile qf(KGlobal::dirs()->findResource("appdata", "encodings/" + encoding() + ".enc"));

	if(qf.open(QIODevice::ReadOnly)) {
		QTextStream stream(&qf);
		QTextCodec *codec = QTextCodec::codecForName(isoName().toAscii());
		if(codec) {
			stream.setCodec(codec);
		}

		while(!stream.atEnd()) {
			//parse the line
			if(stream.readLine().indexOf(reMap) != -1) {
				addPair(reMap.cap(1)[0], reMap.cap(2));
			}
		}
		qf.close();

		return true;
	}

	return false;
}

//BEGIN ConvertIO classes
ConvertIO::ConvertIO(KTextEditor::Document *doc) :
	m_doc(doc),
	m_text(QString()),
	m_line(QString()),
	m_nLine(0)
{
}

QString & ConvertIO::currentLine()
{
	return m_line;
}

void ConvertIO::nextLine()
{
	m_line = m_doc->line(m_nLine++);
}

void ConvertIO::writeText()
{
	m_doc->setText(m_text);
}

int ConvertIO::current()
{
	return m_nLine;
}

bool ConvertIO::done()
{
	return current() == m_doc->lines();
}

ConvertIOFile::ConvertIOFile(KTextEditor::Document *doc, const KUrl & url) : ConvertIO(doc), m_url(url)
{
}

void ConvertIOFile::writeText()
{
	QFile qf(m_url.toLocalFile());
	if(qf.open(QIODevice::WriteOnly)) {
		//read the file
		QTextStream stream(&qf);
		stream << m_text;
		qf.close();
	}
	else {
		kWarning() << "Could not open " << m_url.toLocalFile();
	}
}

ConvertBase::ConvertBase(const QString & encoding, ConvertIO * io) :
	m_io(io),
	m_encoding(encoding),
	m_map(NULL)
{
}

//END ConvertIO classes

//BEGIN ConvertBase
QString ConvertBase::mapNext(int &i)
{
	return (QString)m_io->currentLine()[i++];
}

bool ConvertBase::convert()
{
	if(!setMap()) {
		return false;
	}

	m_io->text().clear();
	do {
		m_io->nextLine();
		int i = 0;
		while(i < m_io->currentLine().length()) {
			m_io->text() += mapNext(i);
		}
		if(!m_io->done()) {
			m_io->text() += '\n';
		}
	}
	while(!m_io->done());

	m_io->writeText();
	return true;
}

bool ConvertBase::setMap()
{
	//create map (or use existing)
	if(ConvertMap::create(m_encoding)) {
		m_map = ConvertMap::mapFor(m_encoding);
	}
	else {
		m_map = NULL;
	}

	return (m_map != NULL);
}
//END ConvertBase

//BEGIN ConvertEncToASCII
QString ConvertEncToASCII::mapNext(int &i)
{
	return m_map->canDecode(m_io->currentLine()[i]) ? m_map->toASCII(m_io->currentLine()[i++]) : (QString)m_io->currentLine()[i++];
}
//END ConvertEncToASCII

//BEGIN ConvertASCIIToEnc

//i is the position of the '\'
QString ConvertASCIIToEnc::nextSequence(int &i)
{
	//get first two characters
	QString seq = (QString)m_io->currentLine()[i++];

	if(m_io->currentLine()[i].isLetter()) {
		while(m_io->currentLine()[i].isLetter()) {
			seq += (QString)m_io->currentLine()[i++];
		}
	}
	else {
		return seq + (QString)m_io->currentLine()[i++];
	}

	return seq;
}

bool ConvertASCIIToEnc::isModifier(const QString& seq)
{
	static QRegExp reModifier("\\\\([cHkruv]|\"|\'|\\^|`|~|=|\\.)");
	return reModifier.exactMatch(seq);
}

QString ConvertASCIIToEnc::getSequence(int &i)
{
	QString seq = nextSequence(i);
	static QRegExp reBraces("\\{([a-zA-Z]?)\\}");

	if(isModifier(seq)) {
		KILE_DEBUG() << "\tisModifier true : " << seq;
		if(seq[seq.length() - 1].isLetter()) {
			seq += ' ';
		}

		while(m_io->currentLine()[i].isSpace()) {
			++i;
		}

		if(m_io->currentLine().mid(i, 2) == "{}") {
			i = i + 2;
		}

		if(m_io->currentLine()[i] == '\\') {
			seq += nextSequence(i);
		}
		else {
			if(reBraces.exactMatch(m_io->currentLine().mid(i, 3))) {
				KILE_DEBUG() << "\tbraces detected";
				i = i + 3;
				seq += reBraces.cap(1);
			}
			else {
				QChar nextChar = m_io->currentLine()[i++];
				if(!nextChar.isSpace()) {
					seq += (QString)nextChar;
				}
			}
		}
	}
	else if(m_map->canEncode(seq)) {
		if(m_io->currentLine().mid(i, 2) == "{}") {
			i = i + 2;
		}
		else if(m_io->currentLine()[i].isSpace()) {
			++i;
		}
	}

	return seq;
}

QString ConvertASCIIToEnc::mapNext(int &i)
{
	if(m_io->currentLine()[i] == '\\') { 
		QString seq = getSequence(i);
		KILE_DEBUG() << "'\tsequence: " << seq;
		if(m_map->canEncode(seq)) {
			KILE_DEBUG() << "\tcan encode this";
			//if ( m_io->currentLine().mid(i, 2) == "{}" ) i = i + 2;
			return m_map->toEncoding(seq);
		}
		else {
			return seq;
		}
	}

	return ConvertBase::mapNext(i);
}
//END ConvertASCIIToEnc
