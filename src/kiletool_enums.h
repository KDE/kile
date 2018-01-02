/************************************************************************************
    begin                : mon 3-11 20:40:00 CEST 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
 ************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KILETOOL_ENUMS_H
#define KILETOOL_ENUMS_H

#define OKULAR_LIBRARY_NAME "okularpart"

namespace KileTool
{
/**
 * KileTool can send several types of messages
 * Error
 * Warning
 * Info
 **/
enum { Error = 0, Warning, Info, ProblemError, ProblemWarning, ProblemBadBox };

enum { NeedTargetDirExec = 0x01, NeedTargetDirWrite = 0x02, NeedTargetExists = 0x04, NeedTargetRead = 0x08,
       NeedActiveDoc = 0x10, NeedMasterDoc = 0x20, NoUntitledDoc = 0x40, NeedSourceExists = 0x80, NeedSourceRead = 0x100, NeedSaveAll = 0x200
     };

enum { Running = 0, ConfigureFailed, NoLauncherInstalled, NoValidTarget, NoValidSource, TargetHasWrongPermissions, NoValidPrereqs, CouldNotLaunch, SelfCheckFailed};

enum { Success = 0, Failed = 1, AbnormalExit = 2, Aborted = 3, Silent = 4 };
}
#endif
