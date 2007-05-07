/***************************************************************************
    begin                : Mar 12 2007
    copyright            : 2007 by Holger Danielsson
    email                : holger.danielsson@versanet.de
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
#include <qstringlist.h>
#include <qfileinfo.h>

#include <klocale.h>
#include <kdebug.h>

#include <kileextensions.h>

namespace KileDocument 
{

//////////////////// Extensions ////////////////////

Extensions::Extensions()
{
	m_documents = ".tex .ltx .latex .dtx .ins";
	m_packages = ".cls .sty";
	m_bibtex = ".bib";
	m_metapost = ".mp";
	m_script = ".js";
	m_project = ".kilepr";
	//m_images = ".eps .pdf .dvi .ps .fig .gif .jpg .jpeg .png";
	m_images = ".eps .jpg .jpeg .png .pdf .ps .fig .gif";

	m_documentDefault = ".tex";
	m_bibtexDefault = ".bib";
	m_metapostDefault = ".mp";
	m_scriptDefault = ".js";
	m_projectDefault = ".kilepr";
}

//////////////////// file filter ////////////////////

QString Extensions::fileFilter(uint type)
{
	QString ext,text;
	switch ( type )
	{ 
		case LATEX_EXT_DOC:
			ext = m_documents;
			text = i18n("(La)TeX Source Files");
			break;
		case LATEX_EXT_PKG:
			ext = m_packages;
			text = i18n("(La)TeX Packages");
			break;
		case LATEX_EXT_BIB:
			ext = m_bibtex;
			text = i18n("BibTeX Files");
			break;
		case LATEX_EXT_MP:
			ext = m_metapost;
			text = i18n("Metapost Files");
			break;
		case LATEX_EXT_JS:
			ext = m_script;
			text = i18n("Kile Script Files");
			break;
		case LATEX_EXT_PROJ:
			ext = m_project;
			text = i18n("Kile Project Files");
			break;
		default:
			return QString::null;
	}

	ext.replace(".","*.");
	return ext + "|" + text;
}

//////////////////// document type ////////////////////

bool Extensions::isTexFile(const KURL & url) const
{
	//TODO use mimetype
	QString ext = "." + QFileInfo(url.fileName()).extension(false);
	return isLatexDocument(ext) || isLatexPackage(ext);
}

bool Extensions::isBibFile(const KURL & url) const
{
	QString ext = "." + QFileInfo(url.fileName()).extension(false);
	return isBibtex(ext);
}

bool Extensions::isScriptFile(const KURL& url) const
{
	QString ext = "." + QFileInfo(url.fileName()).extension(false);
	return isScript(ext);
}

bool Extensions::isProjectFile(const KURL& url) const
{
	QString ext = "." + QFileInfo(url.fileName()).extension(false);
	return isProject(ext);
}

bool Extensions::validExtension(const QString &ext, const QString &extensions) const
{
	QStringList extlist = QStringList::split(" ",extensions);
	for ( QStringList::ConstIterator it=extlist.begin(); it!=extlist.end(); ++it )
	{
		if ( (*it) == ext ) 
			return true;
	}

	return false;
}


Type Extensions::determineDocumentType(const KURL& url) const
{
	if ( isTexFile(url) )
	{
		return KileDocument::LaTeX;
	}
	else if ( isBibFile(url) )
	{
		return KileDocument::BibTeX;
	}
	else if ( isScriptFile(url) )
	{
		return KileDocument::Script;
	}
	else // defaulting to a LaTeX file
	{
		return KileDocument::LaTeX;
	}
}

}
