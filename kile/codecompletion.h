/***************************************************************************
                           codecompletion.h 
----------------------------------------------------------------------------
    date                 : Jan 10 2004
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

#ifndef CODECOMPLETION_H
#define CODECOMPLETION_H

#include <kate/view.h>
#include <kate/document.h>

#include <ktexteditor/codecompletioninterface.h>

/**
  *@author Holger Danielsson
  */

class CodeCompletion 
{
public:        
  CodeCompletion();
  ~CodeCompletion();

  enum Mode
    {
      cmLatex,
      cmEnvironment,
      cmDictionary,
      cmAbbreviation
    };

  bool isActive();
  bool inProgress();
  bool autoComplete();
  
  void readConfig(KConfig *config);

  void completeWord(Kate::View *view, const QString &text, CodeCompletion::Mode mode);
  QString filterCompletionText(const QString &text,const QString &type);

  void CompletionDone();
  void CompletionAborted();

  const QString getBullet();
  
private:
   // wordlists
   QValueList<KTextEditor::CompletionEntry> m_texlist;
   QValueList<KTextEditor::CompletionEntry> m_dictlist;
   QValueList<KTextEditor::CompletionEntry> m_abbrevlist;
   
   // some flags
   bool m_isenabled;
   bool m_setcursor;
   bool m_setbullets;
   bool m_closeenv;
   bool m_autocomplete;
   
   // Flag zum Status von Complete
   bool m_firstconfig;
   bool m_inprogress;

   // Flag zum Wiederherstellen von Text
   bool m_undo;

   // das aktuelle Bullet-Zeichen
   QString m_bullet;
   
   // internal Parameter
   Kate::View *m_view;                  // View
   QString m_text;                      // current pattern
   uint m_textlen;                      // length of current pattern
   CodeCompletion::Mode m_mode;         // completion mode
   uint m_ycursor,m_xcursor,m_xstart;   // current cursor position
   uint m_yoffset,m_xoffset;            // offset of the new cursor position

   QString buildLatexText(const QString &text, uint &ypos, uint &xpos);
   QString buildEnvironmentText(const QString &text, const QString &type, uint &ypos, uint &xpos);
   QString buildAbbreviationText(const QString &text);
   
   QString parseText(const QString &text, uint &ypos, uint &xpos, bool checkgroup);
   QString stripParameter(const QString &text);

   void setWordlist(const QStringList &files,const QString &dir,
                    QValueList<KTextEditor::CompletionEntry> *entrylist);
   void readWordlist(QStringList &wordlist, const QString &filename);
   void setCompletionEntries(QValueList<KTextEditor::CompletionEntry> *list,
                             const QStringList &wordlist);
   
   uint countEntries(const QString &pattern,
                     QValueList<KTextEditor::CompletionEntry> *list,
                     QString *entry, QString *type);
};

#endif
