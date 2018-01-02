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

#ifndef CONVERT_H
#define CONVERT_H

#include <QString>
#include <QStringList>
#include <qmap.h>

#include <QUrl>

namespace KTextEditor {
class Document;
}

class ConvertMap
{
protected:
    ConvertMap(const QString & encoding);

public:
    const QString & encoding() const {
        return m_aliases[0];
    }
    const QString & isoName() const {
        return m_aliases[1];
    }

    QChar toEncoding(const QString & enc) {
        return m_toEncoding[enc];
    }
    QString toASCII(const QChar & c) {
        return m_toASCII[c];
    }

    void addPair(QChar c, const QString & enc);

    bool canDecode(const QChar & c) {
        return ( m_toASCII.contains(c));
    }
    bool canEncode(const QString & enc) {
        return ( m_toEncoding.contains(enc));
    }

    bool load();

private:
    bool commandIsTerminated(const QString &);

private:
    QStringList				m_aliases;
    QMap<QChar, QString>		m_toASCII;
    QMap<QString, QChar>		m_toEncoding;

//static members
public:
    static bool create(const QString & encoding);
    static QString encodingNameFor(const QString &);
    static QString isoNameFor(const QString &);
    static ConvertMap * mapFor(const QString & enc) {
        return g_maps[enc];
    }

private:
    static QMap<QString, ConvertMap*>	g_maps;
};

class ConvertIO
{
public:
    ConvertIO(KTextEditor::Document *doc);
    virtual ~ConvertIO() {}

    virtual void nextLine(); //read next line
    virtual QString& currentLine();
    virtual QString& text() {
        return m_text;
    }
    virtual void writeText();
    virtual int current(); //current line number
    virtual bool done();

protected:
    KTextEditor::Document	*m_doc;
    QString			m_text, m_line;
    int				m_nLine;
};

class ConvertIOFile : public ConvertIO
{
public:
    ConvertIOFile(KTextEditor::Document *doc, const QUrl &url);

    void writeText();

private:
    QUrl	m_url;
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

    virtual QString mapNext(int&);

    ConvertIO		*m_io;
    QString			m_encoding;
    ConvertMap		*m_map;
};

class ConvertEncToASCII : public ConvertBase
{
public:
    ConvertEncToASCII(const QString & encoding, ConvertIO * io) : ConvertBase(encoding, io) {}

protected:
    QString mapNext(int&);
};

class ConvertASCIIToEnc : public ConvertBase
{
public:
    ConvertASCIIToEnc(const QString & encoding, ConvertIO * io) : ConvertBase(encoding, io) {}

protected:
    QString getSequence(int&);
    QString nextSequence(int&);
    bool isModifier(const QString&);
    QString mapNext(int&);
};

#endif
