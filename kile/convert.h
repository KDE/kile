/***************************************************************************
                          convert.h -  description
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

#ifndef CONVERT_H
#define CONVERT_H

#include <qmap.h>

namespace Kate { class Document; }

class ConvertMap
{
public:
	ConvertMap() {}

	unsigned char fromEncoding(const QString & enc) { return m_fromEncoding[enc]; }
	QString fromASCII(const unsigned char c) { return m_fromASCII[c]; }

	void addPair(unsigned char c, QString enc);

	bool contains(const unsigned char c) { return ( m_fromASCII.contains(c) > 0 ); }
	bool contains(const QString & enc) { return ( m_fromEncoding.contains(enc) > 0 ); }

private:
	bool commandIsTerminated(const QString &);

private:
	QMap<unsigned char, QString>	m_fromASCII;
	QMap<QString, unsigned char>	m_fromEncoding;
};

typedef QMap<QString, ConvertMap*> ConvertMetaMap;

class ConvertIO
{
public:
	ConvertIO(Kate::Document *doc);
	virtual ~ConvertIO() {}

	virtual void nextLine(); //read next line
	virtual QString & currentLine();
	virtual QString & text() { return m_text; }
	virtual void writeText();
	virtual uint current(); //current line number
	virtual bool done();

protected:
	Kate::Document	*m_doc;
	QString			m_text, m_line;
	uint				m_nLine;
};

class ConvertIOFile : public ConvertIO
{
public:
	ConvertIOFile(Kate::Document *doc, const KURL & url);

	void writeText();

private:
	KURL	m_url;
};

class ConvertBase
{
public:
	ConvertBase(const QString & encoding, ConvertIO * io);
	virtual ~ConvertBase() {};

public:
	virtual bool convert();

protected:
	virtual bool setMap();
	virtual bool loadMap(const QString &);

	virtual QString mapNext(uint &);

	ConvertIO		*m_io;
	QString			m_encoding;
	ConvertMap		*m_map;
};

class ConvertEncToASCII : public ConvertBase
{
public:
	ConvertEncToASCII(const QString & encoding, ConvertIO * io) : ConvertBase(encoding, io) {}

protected:
	QString mapNext(uint &);
};

class ConvertASCIIToEnc : public ConvertBase
{
public:
	ConvertASCIIToEnc(const QString & encoding, ConvertIO * io) : ConvertBase(encoding, io) {}

protected:
	QString getSequence(uint &);
	QString nextSequence(uint &);
	bool isModifier(const QString &);
	QString mapNext(uint &);
};

#endif
