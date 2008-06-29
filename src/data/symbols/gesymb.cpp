/***************************************************************************
    begin                : Mon April 17 2006
    edit:		 : Sat March 29 2008
    copyright            : (C) 2006-2008 by Thomas Braun
    email                : braun@physik.fu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "gesymb.h"

using std::cout;
using std::endl;

int main( int argc, char ** argv )
{

int latexret=-1;
int dvipngret=-1;
QString texcommand, texfile,type,dvipngcommand,line;
QString pkgs,pkgsarg,savepkgs="",savepkgsarg="";
bool env=false;


if (argc < 2)
	usage();


texfile=argv[2];
type=argv[1];

QFile f( texfile );
if ( !f.open( QIODevice::ReadOnly ) )
{
	cout << "File " << qPrintable(texfile) << " is not readable" << endl;
	return 1;
}

texcommand="latex " + texfile;
dvipngcommand="dvipng  --picky -bg Transparent -x 518 -O -1.2in,-1.2in -T bbox -D 300 -o img%03d" + type + ".png " + texfile.left(texfile.length()-4);

cout << qPrintable(texcommand) << endl;
cout << qPrintable(dvipngcommand) << endl;

latexret = system(texcommand.toLatin1());
dvipngret= system(dvipngcommand.toLatin1());

if (latexret)
{
	cout << "Error compiling the latex file\n";
	return 1;
}
if(dvipngret)
{ 
	cout << "Error producing the pngs\n";
	return 1;
}

QTextStream t(&f);

QRegExp optarg("\\\\(?:math)?command\\[(.*)\\]\\{");
QRegExp arg("\\\\(?:math)?command\\{(.*)\\}");
QRegExp beginenvpkgs("^\\\\begin\\{neededpkgs\\}(?:\\[(.*)\\])?\\{(.*)\\}");
QRegExp endenvpkgs("^\\\\end\\{neededpkgs\\}");
QRegExp cmdpkgs("\\\\pkgs(?:\\[(.*)\\])?\\{(.*)\\}");
QRegExp	comment("^\\s*%");

int number=1;
beginenvpkgs.setMinimal(true);
cmdpkgs.setMinimal(true);
// optarg.setMinimal(true); in some cases necessary

while( (line = t.readLine()) != 0L)
{
	if( line.find(comment) != -1 )
		continue;
	if(env)
	{
		pkgs=savepkgs;
		pkgsarg=savepkgsarg;
	}
	else
	{
		pkgs="";
		pkgsarg="";
	}

	if ( line.indexOf(beginenvpkgs) != -1)
	{
		env=true;
		pkgs=beginenvpkgs.cap(2);
		pkgsarg=beginenvpkgs.cap(1);
		savepkgs=pkgs;
		savepkgsarg=pkgsarg;
	}
	else if( line.indexOf(cmdpkgs) != -1)
	{
		pkgs=cmdpkgs.cap(2);
		pkgsarg=cmdpkgs.cap(1);
	}
	else if( line.indexOf(endenvpkgs) != -1)
	{
		env=false;
		savepkgs="";
		savepkgsarg="";
		pkgs="";
		pkgsarg="";
	}

	cout << "line is " << qPrintable(line);
	cout << "; pkgs=" << qPrintable(pkgs) << " ,pkgsarg=" << qPrintable(pkgsarg) << " ,savepkgs=" << qPrintable(savepkgs) << " ,savepkgsarg=" << qPrintable(savepkgsarg) << endl;

	if ( line.indexOf(optarg) != -1)
		writeComment(optarg.cap(1),pkgs,pkgsarg,type,number++);
	else if(line.indexOf(arg) != -1)
		writeComment(arg.cap(1),pkgs,pkgsarg,type,number++);
}

f.close();
return 0;
}

void usage()
{
	cout << "Usage:\n";
	cout << "gesymb <type> <latex-file>\n";
	exit(1);
}

void writeComment(QString cmd, QString pkgs, QString pkgsarg, QString type, int number)
{

QImage image;
QString fname;
fname.sprintf("img%03d",number);
fname.append(type);
fname.append(".png");

cout << "fname is " << qPrintable(fname) << endl;

if(image.load(fname))
{
	image.setText("Command",0,cmd);
	image.setText("Packages",0, ( pkgsarg.isEmpty() ? "" : '[' + pkgsarg + ']' ) + ( pkgs.isEmpty() ? "" : '{' + pkgs + '}') );
	
	if(!image.save(fname,"PNG"))
	{
		cout << "Image " << qPrintable(fname) << " could not be saved" << endl;
		exit(1);
	}
	readComment(fname);
}
else
	cout << "===writeComment=== ERROR " << qPrintable(fname) << " could not be loaded" << endl;

}

void readComment(QString fname)
{
QImage image;
QString output;

if(image.load(fname))
{
	output = QString("image %1 has Command comment_%2_").arg(fname).arg(image.text("Command"));
	cout << qPrintable(output) << endl;

	output = QString("image %1 has Package comment_%2_").arg(fname).arg(image.text("Packages"));
	cout << qPrintable(output) << endl;
	
}
else
	cout << "===readComment=== ERROR " << qPrintable(fname) << " could not be loaded" << endl;
}


