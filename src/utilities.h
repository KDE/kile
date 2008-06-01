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


#endif
