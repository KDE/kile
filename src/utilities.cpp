/**************************************************************************
*   Copyright (C) 2008-2018 by Michel Ludwig (michel.ludwig@kdemail.net)  *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "utilities.h"

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QStyle>
#include <QTimer>
#include <QWidget>

#include <KIO/ApplicationLauncherJob>
#include <KIO/JobUiDelegateFactory>
#include <KJobUiDelegate>

#include "kiledebug.h"

ServiceRunAction::ServiceRunAction(const KService& service,
                                   const QList<QUrl>& urls,
                                   QWidget* window,
                                   bool tempFiles,
                                   const QString& suggestedFileName,
                                   const QByteArray& asn,
                                   QObject *parent)
    : QAction(QIcon::fromTheme(service.icon()), service.genericName(), parent),
      m_service(service),
      m_urlList(urls),
      m_window(window),
      m_tempFiles(tempFiles),
      m_suggestedFileName(suggestedFileName),
      m_asn(asn)
{
    connect(this, SIGNAL(triggered()), this, SLOT(runService()));
}

ServiceRunAction::~ServiceRunAction()
{
}

void ServiceRunAction::runService()
{
    KService kservice(m_service);
    KService::Ptr servicePointer = KService::Ptr(&kservice);
    auto *job = new KIO::ApplicationLauncherJob(servicePointer);
    job->setUrls(m_urlList);
    job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, m_window));
    if (m_tempFiles) {
        job->setRunFlags(KIO::ApplicationLauncherJob::DeleteTemporaryFiles);
    }
    job->setSuggestedFileName(m_suggestedFileName);
    job->setStartupId(m_asn);
    job->start();
}

QString KileUtilities::lastModifiedFile(const QStringList& files, const QString& baseDir)
{
    KILE_DEBUG_MAIN << "==KileUtilities::lastModifiedFile()=====" << files << "baseDir:" << baseDir;

    if(files.empty()) {
        return QString();
    }

    QStringList absoluteFileNames;
    if(baseDir.isEmpty()) {
        absoluteFileNames = files;
    }
    else {
        QDir basePath(baseDir);
        for(const QString& file : files) {
            absoluteFileNames.append(basePath.absoluteFilePath(file));
        }
    }

    QDateTime lastModifiedTime;
    const QString* lastModifiedFile = Q_NULLPTR;

    for(const QString& file : std::as_const(absoluteFileNames)) {
        QFileInfo fileInfo(file);
        if(fileInfo.exists()) {
            QDateTime modificationTime = fileInfo.lastModified();
            if(!lastModifiedTime.isValid() || modificationTime > lastModifiedTime) {
                lastModifiedFile = &file;
                lastModifiedTime = modificationTime;
            }
        }
        else {
            KILE_DEBUG_MAIN << "file does not exist:" << file << "files:" << files;
        }
    }

    if(lastModifiedFile) {
        return *lastModifiedFile;
    }
    return QString();
}

void centerWidgetRelativeToParentRect(QWidget *widget, const QRect& parentRect)
{
    QRect alignedRect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, widget->size(), parentRect);
    widget->move(alignedRect.x(), alignedRect.y());
}

void KileUtilities::centerWidgetRelativeToParent(QWidget *widget)
{
    QWidget *parentWidget = widget->parentWidget();
    if(parentWidget) {
        QRect parentRect(parentWidget->mapToGlobal(QPoint(0, 0)), parentWidget->size());
        centerWidgetRelativeToParentRect(widget, parentRect);
    }
}

void KileUtilities::scheduleCenteringOfWidget(QWidget *widget)
{
    QTimer::singleShot(0, widget, [=] () {
        centerWidgetRelativeToParent(widget);
    });
}

QUrl KileUtilities::canonicalUrl(const QUrl &url)
{
    if(!url.isLocalFile()) {
        return url;
    }

    QFileInfo fileInfo(url.toLocalFile());

    if(fileInfo.exists()) {
        const QString canonicalFileName  = fileInfo.canonicalFilePath();
        Q_ASSERT_X(!canonicalFileName.isEmpty(), "canonicalUrl", "empty although file exists!");

        return QUrl::fromLocalFile(canonicalFileName);
    }
    else {
        return QUrl::fromLocalFile(QDir::cleanPath(url.toLocalFile()));
    }
}

static inline QString getRelativeSharePath()
{
    return QCoreApplication::applicationDirPath() + QLatin1String("/../share/kile/");
}

QString KileUtilities::findExecutable(const QString &executableName, const QStringList &paths)
{
    return QStandardPaths::findExecutable(executableName, paths);
}

QString KileUtilities::locate(QStandardPaths::StandardLocation type, const QString &fileName,
                                                      QStandardPaths::LocateOptions options)
{
    if(type == QStandardPaths::AppDataLocation || type == QStandardPaths::AppLocalDataLocation) {
	const QString candidate = getRelativeSharePath() + fileName;
	if((options == QStandardPaths::LocateFile) && QFileInfo::exists(candidate)) {
            return candidate;
	}
        else if((options == QStandardPaths::LocateDirectory) && QDir(candidate).exists()) {
            return candidate;
        }
    }

    return QStandardPaths::locate(type, fileName, options);
}

QStringList KileUtilities::locateAll(QStandardPaths::StandardLocation type, const QString &fileName,
                                                             QStandardPaths::LocateOptions options)
{
    QStringList toReturn;
    if(type == QStandardPaths::AppDataLocation || type == QStandardPaths::AppLocalDataLocation) {
	const QString candidate = getRelativeSharePath() + fileName;
	if((options == QStandardPaths::LocateFile) && QFileInfo::exists(candidate)) {
            toReturn << candidate;
	}
        else if((options == QStandardPaths::LocateDirectory) && QDir(candidate).exists()) {
            toReturn << candidate;
        }
    }
    toReturn << QStandardPaths::locateAll(type, fileName, options);

    return toReturn;
}

QStringList KileUtilities::standardLocations(QStandardPaths::StandardLocation type)
{
    QStringList toReturn;
    if(type == QStandardPaths::AppDataLocation || type == QStandardPaths::AppLocalDataLocation) {
        toReturn << getRelativeSharePath();
    }
    toReturn << QStandardPaths::standardLocations(type);

    return toReturn;
}

QString KileUtilities::writableLocation(QStandardPaths::StandardLocation type)
{
    return QStandardPaths::writableLocation(type);
}

// kate: indent-width 4; replace-tabs: true;
