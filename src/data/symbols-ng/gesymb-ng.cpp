/*
 *  description: converter from xml to png files for new file format
 *  date: 8 may 2009
 *
 *  Copyright (C) 2010  Thomas Braun <thomas.braun@virtuell-zuhause.de>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <QDomDocument>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QList>
#include <QTemporaryFile>
#include <QImage>

#include <iostream>
#include <stdlib.h>

#include "../../symbolviewclasses.h"

/* TODO
   - more error checking when parsing xml file
*/

void readImageComments(const QString &fileName)
{
    QImage image;
    QString output;

    if(image.load(fileName)) {
        qDebug() << QString("Image %1 has Command _%2_").arg(fileName).arg(image.text("Command"));
        qDebug() << QString("Image %1 has Comment _%2_").arg(fileName).arg(image.text("Comment"));
        qDebug() << QString("image %1 has Package _%2_").arg(fileName).arg(image.text("Packages"));
        qDebug() << QString("Image %1 has CommandUnicode _%2_").arg(fileName).arg(image.text("CommandUnicode"));
        qDebug() << QString("Image %1 has UnicodePackages _%2_").arg(fileName).arg(image.text("UnicodePackages"));
    }
    else {
        qDebug() << "===readComment=== ERROR " << fileName << " could not be loaded";
    }
}

QString convertUTF8toLatin1String(const QString &string) {

    QVector<uint> stringAsInt;
    QString stringAsLatin1;

    Q_FOREACH(uint i, string.toUcs4()) {
        stringAsLatin1 += QString("U+%1,").arg(i);
    }
    return stringAsLatin1;
}

QString pkgListToString(const QList<Package> &packages) {

    QString packagesArg, packagesName;

    for(int i=0; i < packages.count() ; i++) {
        packagesArg  += packages[i].arguments + ',';
        packagesName += packages[i].name + ',';
    }
    QString result = ( packagesArg.isEmpty() ? "" : '[' + packagesArg + ']' ) + ( packagesName.isEmpty() ? "" : '{' + packagesName + '}' );
    return result;
}

void writeImageComments(const Command &cmd, const QString &fileName)
{

    QImage image;
    QString unicodeCommandAsLatin1, commentAsLatin1;
    QString packagesarg, packages;

    if(!cmd.unicodeCommand.isEmpty()) {
        unicodeCommandAsLatin1 = convertUTF8toLatin1String(cmd.unicodeCommand);
    }
    if(!cmd.comment.isEmpty()) {
        commentAsLatin1 = convertUTF8toLatin1String(cmd.comment);
    }

    qDebug() << "fileName is " << fileName;
    qDebug() << "Command is " << cmd.latexCommand;
    qDebug() << "unicodeCommandAsLatin1 is " << unicodeCommandAsLatin1;
    qDebug() << "commentAsLatin1 is " << commentAsLatin1;
    qDebug() << "comment is " << cmd.comment;

    if(image.load(fileName)) {

        image.setText("Command",cmd.latexCommand);
        if( !unicodeCommandAsLatin1.isEmpty() ) {
            image.setText("CommandUnicode",unicodeCommandAsLatin1);
            image.setText("UnicodePackages",pkgListToString(cmd.unicodePackages));
        }
        if (!commentAsLatin1.isEmpty() ) {
            image.setText("Comment",commentAsLatin1);
        }

        image.setText("Packages",pkgListToString(cmd.packages));
        if(!image.save(fileName,"PNG")) {
            qDebug() << "Image " << fileName << " could not be saved";
            exit(1);
        }
    }
    else {
        qDebug() << "===writeComment=== ERROR " << fileName << "could not be loaded";
    }

}

QString generatePNG(QString latexFile, int index, QString symbolGroupName) {

    QString texfile, texfileWithoutSuffix,pngfile;
    int latexret, dvipngret;

    QTemporaryFile file("XXXXXX.tex");
    file.setAutoRemove(false);
    if (file.open()) {
        QTextStream t(&file);
        t.setCodec("UTF-8");
        t << latexFile;

        texfile = file.fileName();
        texfileWithoutSuffix = texfile.left(texfile.length() - 4);
        pngfile = QString("img%1%2.png").arg(index,3,10,QChar('0')).arg(symbolGroupName);
        qDebug() << texfile;
        qDebug() << texfileWithoutSuffix;
        qDebug() << pngfile;

        file.close();
    }

    QString texcommand=QString("latex %1").arg(texfile);
    QString dvipngcommand=QString("dvipng  --strict --picky --freetype -x 1440 -bg Transparent -O -1.2in,-1.2in -T bbox -z 6 -o %1 %2.dvi").arg(pngfile).arg(texfileWithoutSuffix);

    qDebug() << texcommand;
    qDebug() << dvipngcommand;

    latexret = system(texcommand.toLatin1());
    dvipngret= system(dvipngcommand.toLatin1());

    if (latexret) {
        qDebug() << "Error compiling the latex file";
        return QString();
    }

    if(dvipngret) {
        qDebug() << "Error producing the pngs";
        return QString();
    }

    return pngfile;
}


QString generateLatexFile(const Preamble &preamble, const Command &cmd)
{
    QString output;
    Package pkg;
    QString cmdString;

    output += QString("\\documentclass[%1]{%2}\n").arg(preamble.classArguments).arg(preamble.className);
    output += '\n';
    output += preamble.additional;
    output += '\n';

    for(int i=0; i < cmd.packages.count(); i++) {
        pkg = cmd.packages[i];
        if(pkg.arguments.isEmpty()) {
            output += QString("\\usepackage{%1}\n").arg(pkg.name);
        }
        else {
            output += QString("\\usepackage[%1]{%2}\n").arg(pkg.arguments).arg(pkg.name);
        }
    }

    output += "\\begin{document}\n";
    output += '\n';
    cmdString = !cmd.ImageCommand.isEmpty() ? cmd.ImageCommand : cmd.latexCommand;
    output += cmd.mathMode ? QString("\\ensuremath{%1}\n").arg(cmdString) : QString("%1\n").arg(cmdString);
    output += '\n';
    output += "\\end{document}\n";

    return output;
}

QList<Package> getAllPackages(const QDomElement &e) {

    QList<Package> packages;
    Package pkg;
    QDomElement element;

    if(e.isNull()) {
        return packages;
    }

    QDomNodeList cmdNodes = e.childNodes();

    for(int i=0; i < cmdNodes.count(); i++) {
        element = cmdNodes.item(i).toElement();
        if( element.tagName()== "package") {
            pkg.name = element.firstChildElement("name").text();
            pkg.arguments =  element.firstChildElement("arguments").text();
            packages.append(pkg);
            qDebug() << "pkg.name is " << pkg.name;
            qDebug() << "pkg.arguments is " << pkg.arguments;
        }
    }
    return packages;
}

Command getCommandDefinition(const QDomElement &e, QList<Package> unicodePackages)
{
    if(e.isNull()) {
        return Command();
    }

    Package pkg;
    Command cmd;

    cmd.unicodePackages = unicodePackages;
    cmd.packages = getAllPackages(e);
    cmd.mathMode = e.firstChildElement("mathMode").text() == "true" ? true : false;
    cmd.comment = e.firstChildElement("comment").text();
    cmd.latexCommand = e.firstChildElement("latexCommand").text();
    cmd.unicodeCommand = e.firstChildElement("unicodeCommand").text();
    cmd.ImageCommand = e.firstChildElement("imageCommand").text();

    qDebug() << QString("cmd: latexCommand=%1, unicodeCommand=%2, imageCommand=%3, comment=%4, mathmode=%5").arg(cmd.latexCommand).arg(cmd.unicodeCommand).arg(cmd.ImageCommand).arg(cmd.comment).arg(cmd.mathMode);

    if(cmd.packages.count() > 0 ) {
        qDebug() << "packages are";
        for(int i=0; i < cmd.packages.count(); i++) {
            qDebug() << QString("name=%1, arguments=%2").arg(cmd.packages[i].name).arg(cmd.packages[i].arguments);
        }
    }
    else {
        qDebug() << "no packages to include";
    }

    return cmd;
}

void usage() {

    qDebug() << QString("usage: gesymb-ng mySymbols.xml");
    exit(1);
}

int main(int argc, char** argv)
{
    Preamble preamble;
    Version version;
    QList<Command> commands;
    QString symbolGroupName;
    QList<Package> unicodePkgList;

    if(argc < 2) {
        usage();
    }
    QFile file( argv[1] );

    if( !file.open( QIODevice::ReadOnly ) ) {
        qDebug() << "could not open file";
        return -1;
    }

    QString errorMsg;
    int errorLine,errorColumn;
    QDomDocument doc( "KileSymbolSources" );

    if( !doc.setContent( &file,false, &errorMsg, &errorLine, &errorColumn) )
    {
        qDebug() << "could not find xml content";
        qDebug() << errorMsg;
        qDebug() << "line is " << errorLine;
        qDebug() << "column is " << errorColumn;
        file.close();
        return -2;
    }
    file.close();

    // check root element
    QDomElement root = doc.documentElement();
    if( root.tagName() != "symbols" ) {
        qDebug() << "wrong format";
        return -3;
    }

    QDomNode n = root.firstChild();
    while( !n.isNull() ) {
        QDomElement e = n.toElement();

        if( e.isNull() ) {
            n = n.nextSibling();
            continue;
        }
        qDebug() << "element name is " << e.tagName();
        QString tagName = e.tagName();

        if( tagName == "formatVersion" ) {
            version.major = e.attribute("major");
            version.minor = e.attribute("minor");
        }
        else if( tagName == "symbolGroupName" ) {
            symbolGroupName = e.text();
        }
        else if(tagName == "preamble") {
            preamble.className = e.firstChildElement("class").text();
            preamble.classArguments = e.firstChildElement("arguments").text();
            preamble.additional= e.firstChildElement("additional").text();

            qDebug() << "class is " << preamble.className;
            qDebug() << "arguments is " << preamble.classArguments;
            qDebug() << "additional is " << preamble.additional;

        }
        else if( tagName == "unicodeCommandPackages") {
            unicodePkgList = getAllPackages(e);
        }
        else if( tagName == "commandDefinition" ) {
            commands.append(getCommandDefinition(e,unicodePkgList));
        }
        else {
            qDebug() << "unexpected node: " << tagName;
        }
        n = n.nextSibling();
    }

    QString content,pngfile;
    for(int i=0; i < commands.count(); i++) {
        content = generateLatexFile(preamble,commands[i]);
        qDebug() << content;
        pngfile = generatePNG(content,i+1,symbolGroupName);
        writeImageComments(commands[i],pngfile);
        readImageComments(pngfile);
    }

}
