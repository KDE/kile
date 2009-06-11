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

#ifndef KILEEXTENSIONS_H
#define KILEEXTENSIONS_H

#include "kileconstants.h"

#include <QString>
#include <QStringList>

#include <kurl.h>

namespace KileDocument 
{

class Extensions
{
public:
	Extensions();
	~Extensions() {}

	enum { LATEX_EXT_DOC=1,  LATEX_EXT_PKG=2,  LATEX_EXT_BIB=4, LATEX_EXT_IMG=8,  LATEX_EXT_MP=16, LATEX_EXT_JS=32, LATEX_EXT_PROJ=64 };

	QString latexDocuments() { return m_documents; }
	QString latexPackages() { return m_packages; }
	QString bibtex() { return m_bibtex; }
	QString images() { return m_images; }
	QString metapost() { return m_metapost; }

	QString latexDocumentDefault() { return m_latexDefault; }
	QString bibtexDefault() { return m_bibtexDefault; }
	QString metapostDefault() { return m_metapostDefault; }

	QString latexDocumentFileFilter() { return fileFilter(LATEX_EXT_DOC); }
	QString latexPackageFileFilter() { return fileFilter(LATEX_EXT_PKG); }
	QString bibtexFileFilter() { return fileFilter(LATEX_EXT_BIB); }
	QString imageFileFilter() { return fileFilter(LATEX_EXT_IMG); }
	QString metapostFileFilter() { return fileFilter(LATEX_EXT_MP); }
	QString scriptFileFilter() { return fileFilter(LATEX_EXT_JS); }
	QString projectFileFilter() { return fileFilter(LATEX_EXT_PROJ); }
	
	bool isTexFile(const QString &fileName) const;
	bool isTexFile(const KUrl &url) const { return isTexFile(url.fileName()); }
	bool isBibFile(const QString &fileName) const;	
	bool isBibFile(const KUrl &url) const { return isBibFile(url.fileName()); }
	bool isScriptFile(const QString &fileName) const;
	bool isScriptFile(const KUrl & url) const { return isScriptFile(url.fileName()); }
	bool isProjectFile(const QString &fileName) const;
	bool isProjectFile(const KUrl &url) const { return isProjectFile(url.fileName()); }
	
	bool isLatexDocument(const QString &ext) const { return validExtension(ext,m_documents); }
	bool isLatexPackage(const QString &ext) const { return validExtension(ext,m_packages); }
	bool isImage(const QString &ext) const { return validExtension(ext,m_images); }

	KileDocument::Type determineDocumentType(const KUrl &url) const;
	QString defaultExtensionForDocumentType(KileDocument::Type type) const;

private:
	QString m_documents, m_packages;
	QString m_bibtex, m_metapost;
	QString m_images, m_script;
	QString m_project;

	QString m_latexDefault, m_bibtexDefault;
	QString m_metapostDefault, m_scriptDefault;
	QString m_projectDefault;

	bool isBibtex(const QString &ext) const { return validExtension(ext,m_bibtex); }
	bool isMetapost(const QString &ext) const { return validExtension(ext,m_metapost); }
	bool isScript(const QString &ext) const { return validExtension(ext,m_script); }
	bool isProject(const QString &ext) const { return validExtension(ext,m_project); }
	bool validExtension(const QString &ext, const QString &extensions) const;

	QString fileFilter(uint type);
};

}

#endif
