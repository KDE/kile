/**************************************************************************
*   Copyright (C) 2008 by Michel Ludwig (michel.ludwig@kdemail.net)       *
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

#include <KAction>
#include <KService>
#include <KUrl>

class ServiceRunAction : public KAction {
	Q_OBJECT

	public:
		ServiceRunAction(const KService& service,
		                 const KUrl::List& urls,
		                 QWidget* window,
		                 bool tempFiles = false,
		                 const QString& suggestedFileName = QString(),
		                 const QByteArray& asn = "",
		                 QObject *parent = NULL);
		~ServiceRunAction();

	protected Q_SLOTS:
		void runService();

	protected:
		const KService& m_service;
		KUrl::List m_urlList;
		QWidget* m_window;
		bool m_tempFiles;
		QString m_suggestedFileName;
		QByteArray m_asn;
};

namespace KileUtilities {

	/**
	 * @brief Finds the file whith the most recent modification time from a list
	 *
	 * Checks last modification time for files and returns absolute (see @ref baseDir description) file name of
	 * file with the latest modification time
	 * @param files List of filenames, relative to @ref baseDir
	 * @param baseDir Path to base directory. If empty, @ref files are used as they are
	 * @return Absolute path of the file with the latest modification time or empty string if @ref files is empty
	 **/
	QString lastModifiedFile(const QStringList& files, const QString& baseDir = QString());
}

#endif
