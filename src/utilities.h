/**************************************************************************
*   Copyright (C) 2008-2019 by Michel Ludwig (michel.ludwig@kdemail.net)  *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef UTILITIES_H
#define UTILITIES_H

#include <QAction>
#include <QStandardPaths>
#include <KService>
#include <QUrl>

class ServiceRunAction : public QAction {
    Q_OBJECT

public:
    ServiceRunAction(const KService& service,
                     const QList<QUrl>& urls,
                     QWidget* window,
                     bool tempFiles = false,
                     const QString& suggestedFileName = QString(),
                     const QByteArray& asn = "",
                     QObject *parent = Q_NULLPTR);
    ~ServiceRunAction();

protected Q_SLOTS:
    void runService();

protected:
    const KService& m_service;
    QList<QUrl> m_urlList;
    QWidget* m_window;
    bool m_tempFiles;
    QString m_suggestedFileName;
    QByteArray m_asn;
};

namespace KileUtilities {

/**
 * @brief Finds the file with the most recent modification time from a list
 *
 * Checks last modification time for files and returns absolute (see @ref baseDir description) file name of
 * file with the latest modification time
 * @param files List of filenames, relative to @ref baseDir
 * @param baseDir Path to base directory. If empty, @ref files are used as they are
 * @return Absolute path of the file with the latest modification time or empty string if @ref files is empty
 **/
QString lastModifiedFile(const QStringList& files, const QString& baseDir = QString());


/**
 * Centers the given widget w.r.t. its parent. If it doesn't have a parent, the containing screen is used.
 **/
void centerWidgetRelativeToParent(QWidget *widget);

/**
 * Schedules the centering of the given widget w.r.t. its parent in the event loop.
 **/
void scheduleCenteringOfWidget(QWidget *widget);

/**
 * If 'url' is a local file, the canonical file path is returned if the file exists. Otherwise,
 * the cleaned file path is returned (see QDir::cleanPath).
 * If 'url' is not a local file, 'url' is returned.
 **/
QUrl canonicalUrl(const QUrl &url);


/**
 * Add our own versions of most QStandardPaths:: methods, which allow to locate 'DataLocation' or
 * 'AppDataLocation' items under <app binary dir>/../share/kile
 * This is necessary for being able to create an AppImage
 **/

QString findExecutable(const QString &executableName, const QStringList &paths = QStringList());

QString locate(QStandardPaths::StandardLocation type,
               const QString &fileName,
               QStandardPaths::LocateOptions options = QStandardPaths::LocateFile);

QStringList locateAll(QStandardPaths::StandardLocation type,
                      const QString &fileName,
                      QStandardPaths::LocateOptions options = QStandardPaths::LocateFile);

QStringList standardLocations(QStandardPaths::StandardLocation type);

QString writableLocation(QStandardPaths::StandardLocation type);
}

#endif
