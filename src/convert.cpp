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

#include <KMessageBox>
#include <KTextEditor/Document>

#include "kiledebug.h"
#include "utilities.h"

QMap<QString, ConvertMap*> ConvertMap::g_maps;

bool ConvertMap::create(const QString & encoding)
{
    KILE_DEBUG_MAIN << "\tlooking for map for " << encoding;
    ConvertMap * map = g_maps[encoding];

    if(!map) {
        KILE_DEBUG_MAIN << "\tcreating a map for " << encoding;
        map = new ConvertMap(encoding); // FIXME This will never be deleted if load() succeeds...
        if(map->load()) {
            g_maps[encoding] = map;
        }
        else {
            delete map;
            map = Q_NULLPTR;
        }
        map = g_maps[encoding];
    }

    return (map != Q_NULLPTR);
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

    if(std.startsWith(QLatin1String("iso8859-"))) {
        return QLatin1String("latin") + std.right(1);
    }

    if(std.startsWith(QLatin1String("cp"))) {
        return QLatin1String("cp") + std.right(4);
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

    if(std.startsWith(QLatin1String("latin"))) {
        return QLatin1String("ISO 8859-") + std.right(1);
    }

    if(std.startsWith(QLatin1String("cp"))) {
        return QLatin1String("cp ") + std.right(4);
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
    m_toASCII[c] = commandIsTerminated(enc) ? enc : enc + QLatin1String("{}");
    m_toEncoding[enc] = c;
}

bool ConvertMap::commandIsTerminated(const QString & command)
{
    static QRegularExpression reCommandSequences(QLatin1String("\\\\([a-zA-Z]+|\\\"|\\')$"));

    return reCommandSequences.match(command).hasMatch();
}

bool ConvertMap::load()
{
    static QRegularExpression reMap(QLatin1String("^(.*):(.*)"));

    //makeMap(encoding());

    //if map already exists, replace it
    QFile qf(KileUtilities::locate(QStandardPaths::AppDataLocation, QLatin1String("encodings/") + encoding() + QLatin1String(".enc")));

    if(qf.open(QIODevice::ReadOnly)) {
        QTextStream stream(&qf);
        auto encoding = QStringConverter::encodingForName(isoName().toLatin1().constData());
        if(encoding) {
            stream.setEncoding(*encoding);
        }

        while(!stream.atEnd()) {
            //parse the line
            auto match = reMap.match(stream.readLine());
            if(match.hasMatch()) {
                addPair(match.captured(1)[0], match.captured(2));
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

ConvertIOFile::ConvertIOFile(KTextEditor::Document *doc, const QUrl &url) : ConvertIO(doc), m_url(url)
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
        qWarning() << "Could not open " << m_url.toLocalFile();
    }
}

ConvertBase::ConvertBase(const QString & encoding, ConvertIO * io) :
    m_io(io),
    m_encoding(encoding),
    m_map(Q_NULLPTR)
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
            m_io->text() += QLatin1Char('\n');
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
        m_map = Q_NULLPTR;
    }

    return (m_map != Q_NULLPTR);
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
    static QRegularExpression reModifier(QLatin1String("^\\\\([cHkruv]|\"|\'|\\^|`|~|=|\\.)$"));
    return reModifier.match(seq).hasMatch();
}

QString ConvertASCIIToEnc::getSequence(int &i)
{
    QString seq = nextSequence(i);
    static QRegExp reBraces(QLatin1String("\\{([a-zA-Z]?)\\}"));

    if(isModifier(seq)) {
        KILE_DEBUG_MAIN << "\tisModifier true : " << seq;
        if(seq[seq.length() - 1].isLetter()) {
            seq += QLatin1Char(' ');
        }

        while(m_io->currentLine()[i].isSpace()) {
            ++i;
        }

        if(m_io->currentLine().mid(i, 2) == QLatin1String("{}")) {
            i = i + 2;
        }

        if(m_io->currentLine()[i] == QLatin1Char('\\')) {
            seq += nextSequence(i);
        }
        else {
            if(reBraces.exactMatch(m_io->currentLine().mid(i, 3))) {
                KILE_DEBUG_MAIN << "\tbraces detected";
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
        if(m_io->currentLine().mid(i, 2) == QLatin1String("{}")) {
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
    if(m_io->currentLine()[i] == QLatin1Char('\\')) {
        QString seq = getSequence(i);
        KILE_DEBUG_MAIN << "'\tsequence: " << seq;
        if(m_map->canEncode(seq)) {
            KILE_DEBUG_MAIN << "\tcan encode this";
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
