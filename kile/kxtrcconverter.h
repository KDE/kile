/***************************************************************************
                               kxtconverter.h
                             -------------------
    begin                : fri mai 14 18:04:26 CEST 2004
    copyright            : (C) 2004 by Simon MARTIN
    email                : simartin@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

class KConfig;
class QString;

/**
 * A class to reformat Kile's old resource files (<4) to use KConfigXT.
 * @author Simon MARTIN
 */
class KxtRcConverter
{
public:
	KxtRcConverter(KConfig*, int);
	~KxtRcConverter(void);

	bool Convert(void);

private:
	void renameEntry(const QString&, const QString&, const QString&);
	void changeFileExtensionsFormat(void);
	void changeAutosaveInterval(void);
	void changeVersionNumber(void);
	void changeUse(void);

private:
	KConfig *m_config;
	int m_newVersion;
};
