/***************************************************************************
                               kxtconverter.h
                             -------------------
    begin                : fri may 14 18:09:41 CEST 2004
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

#include "kxtrcconverter.h"
#include "kileconfig.h"
#include <kconfig.h>
#include <qmap.h>

KxtRcConverter::KxtRcConverter(KConfig *toConvert, int newVersion)
{
	m_config = toConvert;
	m_newVersion = newVersion;
}

KxtRcConverter::~KxtRcConverter(void) {}

/**
 * Execution of the conversion utility.
 * @return true if all went OK, false otherwise.
 */
bool KxtRcConverter::Convert(void)
{
	if(NULL == m_config)
		return false;

	renameEntry("Structure", "Structure Level 1", "StructureLevel1");
	renameEntry("Structure", "Structure Level 2", "StructureLevel2");
	renameEntry("Structure", "Structure Level 3", "StructureLevel3");
	renameEntry("Structure", "Structure Level 4", "StructureLevel4");
	renameEntry("Structure", "Structure Level 5", "StructureLevel5");

	renameEntry("User", "Template Encoding", "TemplateEncoding");
	renameEntry("Editor Ext", "Complete Environment", "CompleteEnvironment");

	renameEntry("Complete", "enabled", "CompleteEnabled");
	renameEntry("Complete", "cursor", "CompleteCursor");
	renameEntry("Complete", "bullets", "CompleteBullets");
	renameEntry("Complete", "closeenv", "CompleteCloseEnv");
	renameEntry("Complete", "autocomplete", "CompleteAuto");
	renameEntry("Complete", "changedlists", "CompleteChangedLists");
	renameEntry("Complete", "tex", "CompleteTex");
	renameEntry("Complete", "dict", "CompleteDict");
	renameEntry("Complete", "abbrev", "CompleteAbbrev");

	renameEntry("Environments", "list", "EnvList");
	renameEntry("Environments", "math", "EnvMath");
	renameEntry("Environments", "tabular", "EnvTabular");

	renameEntry("Files", "Last Document", "LastDocument");
	renameEntry("Files", "Input Encoding", "InputEncoding");

	renameEntry("Quick", "Class", "QuickClass");
	renameEntry("Quick", "Typeface", "QuickTypeface");
	renameEntry("Quick", "Papersize", "QuickPapersize");
	renameEntry("Quick", "Document Classes", "QuickDocumentClasses");
	renameEntry("Quick", "Papersizes", "QuickPapersizes");
	renameEntry("Quick", "Encodings", "QuickEncodings");

	changeFileExtensionsFormat();
	changeAutosaveInterval();
	changeVersionNumber();
	changeUse();

	m_config->sync();
	KileConfig::self()->readConfig();

	return true;
}

/**
 * Renames an entry.
 * @param group The group where it is located.
 * @param o The old name.
 * @param n The new name.
 */
void KxtRcConverter::renameEntry(const QString& group, const QString& o, const QString& n)
{
	m_config->setGroup(group);
	if(m_config->hasKey(o)) {
		QString entry = m_config->readEntry(o);
		m_config->writeEntry(n, entry);
		m_config->deleteEntry(o);
	}
}

void KxtRcConverter::changeFileExtensionsFormat(void)
{
	m_config->setGroup("Files");
	if(m_config->hasKey("CleanUpFileExtensions")) {
		QStringList extList = m_config->readListEntry("CleanUpFileExtensions");
		QString newList = extList.join(" ");
		m_config->writeEntry("CleanUpFileExtensions", newList);
	}
}

/** Converts the autosave interval from milliseconds to minutes. */
void KxtRcConverter::changeAutosaveInterval(void)
{
	m_config->setGroup("Files");
	if(m_config->hasKey("AutosaveInterval")) {
		int milli = m_config->readNumEntry("AutosaveInterval");
		m_config->writeEntry("AutosaveInterval", milli / 60000);
	}
}

void KxtRcConverter::changeVersionNumber(void)
{
	m_config->setGroup("VersionInfo");
	m_config->writeEntry("RCVersion", m_newVersion);
}

/** Converts the value of the LaTeX contextual help source option. */
void KxtRcConverter::changeUse(void)
{
	m_config->setGroup("Help");
	if(m_config->hasKey("use")) {
		bool use = m_config->readBoolEntry("use");
		m_config->writeEntry("use", (use) ? 0 : 1);
	}
}
