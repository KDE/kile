/***************************************************************************
                           kileedit.h
----------------------------------------------------------------------------
    date                 : Jan 24 2004
    version              : 0.10
    copyright            : (C) 2004 by Holger Danielsson
    email                : holger.danielsson@t-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KILEEDIT_H
#define KILEEDIT_H

#include <kate/document.h>
#include <qstring.h>
#include <qregexp.h>

/**
  *@author Holger Danielsson
  */

class KileEdit
{
public:
   KileEdit();
   ~KileEdit();

   enum EnvType {
      EnvNone,
      EnvList,
      EnvTab,
      EnvCrTab
   };
   
   QString getTextLineReal(Kate::Document *doc, uint row);

   void gotoBullet(Kate::View *view, const QString &bullet, bool backwards);
   void matchEnvironmentTag(Kate::View *view);
   void gotoEnvironmentTag(Kate::View *view, bool backwards);
   void closeEnvironment(Kate::View *view);
   void insertEnvironmentNewline(Kate::View *view);
   void selectEnvironment(Kate::View *view, bool inside);
   void deleteEnvironment(Kate::View *view, bool inside);
  
private:
   enum EnvTag {
      EnvBegin,
      EnvEnd
   };

   struct EnvData {
      uint row;
      uint col;
      QString name;
      uint len;
      EnvTag tag;
      EnvType type;
   };

   QRegExp m_reg;

    // change cursor position
   bool increaseCursorPosition(Kate::Document *doc, uint &row, uint &col);
   bool decreaseCursorPosition(Kate::Document *doc, uint &row, uint &col);

   // check position
   bool isValidBackslash(Kate::Document *doc, uint row, uint col);
   bool isCommentPosition(Kate::Document *doc, uint row, uint col);
   bool isEnvironmentPosition(Kate::Document *doc, uint row, uint col,EnvData &env);

   // find environment tags
   bool findBeginEnvironment(Kate::Document *doc, uint row, uint col,EnvData &env);
   bool findEndEnvironment(Kate::Document *doc, uint row, uint col,EnvData &env);
   bool findEnvironment(Kate::Document *doc, uint row, uint col,EnvData &env,
                        bool backwards=false);

   // check environment type
   bool isListEnvironment(const QString &name);
   bool isTabEnvironment(const QString &name);

   // get current environment
   bool getEnvironment(Kate::View *view, bool inside, EnvData &envbegin, EnvData &envend);

};


#endif
