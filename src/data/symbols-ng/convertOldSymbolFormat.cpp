/***************************************************************************
    begin                : Sat Mai 9 2009
    copyright            : (C) 2009 by Thomas Braun
    email                : thomas.braun@virtuell-zuhause.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "convertOldSymbolFormat.h"

using std::cout;
using std::cerr;
using std::endl;

int main( int argc, char ** argv )
{

    bool mathMode;
    QList<Package> PackagesList;
    QString  texfile, type, greedyOptArgString, line;
    QString pkgs, pkgsarg, savepkgs, savepkgsarg;
    bool env=false, greedyOptArg=true;

    if (argc < 1) {
        usage();
    }

    texfile=argv[1];
    greedyOptArgString=argv[2];

    if( greedyOptArgString.isEmpty() || greedyOptArgString == QString("true") ) {
        greedyOptArg = true;
    }
    else if( greedyOptArgString == QString("false") ) {
        greedyOptArg = false;
    }
    else {
        usage();
    }

    QFile file(texfile);
    if (  !file.open( QIODevice::ReadOnly ) ) {
        cerr << "File " << qPrintable(texfile) << " is not readable" << Qt::endl;
        return 1;
    }

    type = texfile.left(texfile.length() - 4);
    cout << qPrintable("<symbolGroupName>" + type + "</symbolGroupName>") << Qt::endl;

    QTextStream t(&file);

    QRegExp optarg("\\\\(math)?command\\[(.*)\\]\\{(.*)\\}");
    QRegExp arg("\\\\(math)?command\\{(.*)\\}");
    QRegExp beginenvpkgs("^\\\\begin\\{neededpkgs\\}(?:\\[(.*)\\])?\\{(.*)\\}");
    QRegExp endenvpkgs("^\\\\end\\{neededpkgs\\}");
    QRegExp cmdpkgs("\\\\pkgs(?:\\[(.*)\\])?\\{(.*)\\}");
    QRegExp	comment("^\\s*%");

    beginenvpkgs.setMinimal(true);
    cmdpkgs.setMinimal(true);
    if(!greedyOptArg) {
        optarg.setMinimal(true);
    }

    while( !t.atEnd() ) {

        line = t.readLine();
// 		cout << "<!-- line is " << qPrintable(line) << " -->";

        if( line.contains(comment) ) {
            continue;
        }

        if(env) {
            pkgs=savepkgs;
            pkgsarg=savepkgsarg;
        }
        else {
            pkgs.clear();
            pkgsarg.clear();
        }

        if ( line.indexOf(beginenvpkgs) != -1) {
            env=true;
            pkgs=beginenvpkgs.cap(2);
            pkgsarg=beginenvpkgs.cap(1);
            savepkgs=pkgs;
            savepkgsarg=pkgsarg;
        }
        else if( line.indexOf(cmdpkgs) != -1) {
            pkgs=cmdpkgs.cap(2);
            pkgsarg=cmdpkgs.cap(1);
        }
        else if( line.indexOf(endenvpkgs) != -1) {
            env=false;
            savepkgs.clear();
            savepkgsarg.clear();
            pkgs.clear();
            pkgsarg.clear();
        }

        cout << "<!-- pkgs=" << qPrintable(pkgs) << " ,pkgsarg=" << qPrintable(pkgsarg) << " ,savepkgs=" << qPrintable(savepkgs) << " ,savepkgsarg=" << qPrintable(savepkgsarg) << "-->" << Qt::endl;


        QString packageString = QString("{%1}").arg(pkgs);
        mathMode = false;

        if(!pkgsarg.isEmpty()) {
            packageString.prepend(QString("[%1]").arg(pkgsarg));
        }

        if(line.indexOf(optarg) != -1) {
            cout << "<!-- optarg " << qPrintable(optarg.cap(1) + ' ' + optarg.cap(2) + ' ' + optarg.cap(3) + " -->") << Qt::endl;
            if(optarg.cap(1) == QString("math") ) {
                mathMode = true;
            }
            extractPackageString(packageString,PackagesList);
            outputXML(optarg.cap(2),optarg.cap(3),PackagesList,mathMode);
        }
        else if(line.indexOf(arg) != -1) {
            cout << "<!-- arg " << qPrintable(arg.cap(1) + ' ' + arg.cap(2) + ' ' + arg.cap(3)+ " -->") << Qt::endl;

            if(arg.cap(1) == QString("math") ) {
                mathMode = true;
            }
            extractPackageString(packageString,PackagesList);
            outputXML(arg.cap(2),arg.cap(2),PackagesList,mathMode);
        }
    }
    cout << "</symbols>" << Qt::endl;
    return 0;
}

void usage()
{
    cerr << "Usage:\n";
    cerr << "convertOldSymbolFormat <latex-file> <greadyOptArg>\n";
    cerr << "latex-file, a file which can be compiled with LaTeX, has to end with .tex\n";
    cerr << "greedyOptArg, can be true or false, defaults to true\n";
    cerr << "Setting it to true will match as much of the square brackets from \\command[.*], setting it to false to as few as possible.\n";
    exit(1);
}

void outputXML(const QString latexCommand, const QString imageCommand, QList< Package >& packages, bool mathMode)
{
    Package pkg;
    QString output;
    output = "<commandDefinition>\n";
    if(imageCommand == latexCommand) {
        output += "   <latexCommand>" + Qt::escape(latexCommand) + "</latexCommand>\n";
    }
    else {
        output += "   <latexCommand>" + Qt::escape(latexCommand) + "</latexCommand>\n";
        output += "   <imageCommand>" + Qt::escape(imageCommand) + "</imageCommand>\n";
    }

    if(mathMode) {
        output += "   <mathMode>true</mathMode>\n";
    }

    foreach(pkg, packages) {
        if(pkg.name.isEmpty()) {
            continue;
        }
        output += "   <package>\n\
      <name>" + Qt::escape(pkg.name) + "</name>\n";
        if(!pkg.arguments.isEmpty()) {
            output += "      <arguments>" + Qt::escape(pkg.arguments) + "</arguments>\n";
        }
        output += "   </package>\n";
    }

    output += "</commandDefinition>\n";

    cout << qPrintable(output) << Qt::endl;
}

void extractPackageString(const QString&string, QList<Package> &packages) {

    QRegExp rePkgs("(?:\\[(.*)\\])?\\{(.*)\\}");
    QStringList args,pkgs;
    Package pkg;

    if(string.isEmpty()) {
        return;
    }

    packages.clear();

    if ( rePkgs.exactMatch(string) ) {
        args = rePkgs.cap(1).split(',');
        pkgs = rePkgs.cap(2).split(',');
    }
    else {
        return;
    }

    for(int i = 0 ; i <  pkgs.count() && i < args.count() ; i++) {
        pkg.name = pkgs.at(i);
        pkg.arguments = args.at(i);
        packages.append(pkg);
    }

}
