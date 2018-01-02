/*****************************************************************************
*   Copyright (C) 2007 by Holger Danielsson (holger.danielsson@versanet.de)  *
*             (C) 2016 by Michel Ludwig (michel.ludwig@kdemail.net)          *
******************************************************************************/

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

#include <QLinkedList>
#include <QString>
#include <QUrl>

namespace KileDocument
{

class Extensions
{
public:
    Extensions();
    ~Extensions() {}

    enum ExtensionType { TEX=1,  PACKAGES=2,  BIB=4, IMG=8, METAPOST=16, JS=32, KILE_PROJECT=64 };

    QString latexDocuments() const {
        return m_documents;
    }
    QString latexPackages() const {
        return m_packages;
    }
    QString bibtex() const {
        return m_bibtex;
    }
    QString images() const {
        return m_images;
    }
    QString metapost() const {
        return m_metapost;
    }

    QString latexDocumentDefault() const {
        return m_latexDefault;
    }
    QString bibtexDefault() const {
        return m_bibtexDefault;
    }
    QString metapostDefault() const {
        return m_metapostDefault;
    }

    // we need two methods as KEncodingFileDialog has no Qt-equivalent yet
    QString fileFilterKDEStyle(bool includeAllFiles, const QLinkedList<ExtensionType>& extensions) const;
    QString fileFilterQtStyle(bool includeAllFiles, const QLinkedList<ExtensionType>& extensions) const;

    bool isTexFile(const QString &fileName) const;
    bool isTexFile(const QUrl &url) const {
        return isTexFile(url.fileName());
    }
    bool isBibFile(const QString &fileName) const;
    bool isBibFile(const QUrl &url) const {
        return isBibFile(url.fileName());
    }
    bool isScriptFile(const QString &fileName) const;
    bool isScriptFile(const QUrl &url) const {
        return isScriptFile(url.fileName());
    }
    bool isProjectFile(const QString &fileName) const;
    bool isProjectFile(const QUrl &url) const {
        return isProjectFile(url.fileName());
    }

    bool isLatexDocument(const QString &ext) const {
        return validExtension(ext,m_documents);
    }
    bool isLatexPackage(const QString &ext) const {
        return validExtension(ext,m_packages);
    }
    bool isImage(const QString &ext) const {
        return validExtension(ext,m_images);
    }

    KileDocument::Type determineDocumentType(const QUrl &url) const;
    QString defaultExtensionForDocumentType(KileDocument::Type type) const;

private:
    QString m_documents, m_packages;
    QString m_bibtex, m_metapost;
    QString m_images, m_script;
    QString m_project;

    QString m_latexDefault, m_bibtexDefault;
    QString m_metapostDefault, m_scriptDefault;
    QString m_projectDefault;

    bool isBibtex(const QString &ext) const {
        return validExtension(ext,m_bibtex);
    }
    bool isMetapost(const QString &ext) const {
        return validExtension(ext,m_metapost);
    }
    bool isScript(const QString &ext) const {
        return validExtension(ext,m_script);
    }
    bool isProject(const QString &ext) const {
        return validExtension(ext,m_project);
    }
    bool validExtension(const QString &ext, const QString &extensions) const;

    void fileFilterRaw(ExtensionType type, QString& ext, QString& text) const;
    QString fileFilterKDEStyle(ExtensionType type) const;
    QString fileFilterQtStyle(ExtensionType type) const;
};


}

#endif
