/***************************************************************************
                           kileedit.cpp
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

#include "kileedit.h"

#include <kate/view.h>
#include <kate/document.h>
#include <ktexteditor/searchinterface.h>


KileEdit::KileEdit()
{
   m_reg.setPattern("\\\\(begin|end)\\s*\\{\\s*([A-Za-z]+\\*?)\\s*\\}");
}

KileEdit::~KileEdit()
{
}

//////////////////// goto environment tag (begin or end) ////////////////////

// goto the next non-nested environment tag

void KileEdit::gotoEnvironmentTag(Kate::View *view, bool backwards)
{
   if ( !view ) return;

   uint row,col;
   EnvData env;
   bool found;
   
   // get current position
   Kate::Document *doc = view->getDoc();
   view->cursorPositionReal(&row,&col);
   if ( backwards )
      found = findBeginEnvironment(doc,row,col,env);
   else
      found = findEndEnvironment(doc,row,col,env);
      
   if ( found )
      view->setCursorPositionReal(env.row,env.col);
}

// match the opposite environment tag

void KileEdit::matchEnvironmentTag(Kate::View *view)
{
   if ( !view ) return;

   uint row,col;
   EnvData env;

   // get current position
   Kate::Document *doc = view->getDoc();
   view->cursorPositionReal(&row,&col);
   if ( !isEnvironmentPosition(doc,row,col,env) )
      return;

   bool found = false;   
   if ( env.tag == EnvBegin ) {
      found = findEndEnvironment(doc,env.row,env.col+1,env);     
   } else {
      found = findBeginEnvironment(doc,row,col,env);        
   }
      
   if ( found )
      view->setCursorPositionReal(env.row,env.col);;
}

//////////////////// close an open environment  ////////////////////

void KileEdit::closeEnvironment(Kate::View *view)
{
   if ( !view ) return;

   uint row,col;
   EnvData env;

   Kate::Document *doc = view->getDoc();
   view->cursorPositionReal(&row,&col);
   if ( !isEnvironmentPosition(doc,row,col,env) && findBeginEnvironment(doc,row,col,env) ) {
      doc->insertText( row,col,"\\end{"+env.name+"}\n" );
      view->setCursorPositionReal(row+1,0);
   }     
}

//////////////////// get the type of an open environment ////////////////////

void KileEdit::insertEnvironmentNewline(Kate::View *view)
{
   if ( !view ) return;

   uint row,col;
   EnvData env;

   Kate::Document *doc = view->getDoc();
   view->cursorPositionReal(&row,&col);
   if ( !isEnvironmentPosition(doc,row,col,env) && findBeginEnvironment(doc,row,col,env) ) {
      if ( isListEnvironment(env.name) ) {      
         doc->insertText( row,col,"\n\\item " );
         view->setCursorPositionReal(row+1,6);
      } else if ( isTabEnvironment(env.name) ) {
         doc->insertText( row,col,"\\\\\n" );
         view->setCursorPositionReal(row+1,0);
      } 
   }
}

//////////////////// select an environment  ////////////////////

void KileEdit::selectEnvironment(Kate::View *view, bool inside)
{
   EnvData envbegin,envend;

   if ( getEnvironment(view,inside,envbegin,envend) ) {
      Kate::Document *doc = view->getDoc();
      doc->setSelection(envbegin.row,envbegin.col,envend.row,envend.col);
   }
}

void KileEdit::deleteEnvironment(Kate::View *view, bool inside)
{
   EnvData envbegin,envend;

   if ( getEnvironment(view,inside,envbegin,envend) ) {
      Kate::Document *doc = view->getDoc();
      if ( doc->hasSelection() )
         doc->clearSelection();

      doc->removeText(envbegin.row,envbegin.col,envend.row,envend.col);       
   }
}

bool KileEdit::getEnvironment(Kate::View *view, bool inside,
                              EnvData &envbegin, EnvData &envend)
{
   if ( !view ) return false;
   
    uint row,col;
      
   Kate::Document *doc = view->getDoc();
   view->cursorPositionReal(&row,&col);
   if ( !findBeginEnvironment(doc,row,col,envbegin) ) return false;
   if ( !findEndEnvironment(doc,row,col,envend) ) return false;

   if ( inside ) {
      // check first line
      envbegin.col += envbegin.len;
      if ( envbegin.col >= (uint)doc->lineLength(envbegin.row) ){
         envbegin.row++;
         envbegin.col = 0;
      }
      // check last line
      if ( envend.col == 0 ) {
        envend.row--;
        envend.col = doc->lineLength(envend.row);
      } 
   } else {
      envend.col += envend.len;
   }

   return true;
}

//////////////////// search for \begin{env}  ////////////////////

bool KileEdit::findBeginEnvironment(Kate::Document *doc, uint row, uint col,EnvData &env)
{
  if ( isEnvironmentPosition(doc,row,col,env) ) {
     // already found position?
     if ( env.tag == EnvBegin )
        return true;

     // go one position back
     row = env.row;
     col = env.col;
     if ( ! decreaseCursorPosition(doc,row,col) )
        return false;
  }

  // looking back for last environment
  return findEnvironment(doc,row,col,env,true);
}

//////////////////// search for \end{env}  ////////////////////

bool KileEdit::findEndEnvironment(Kate::Document *doc, uint row, uint col,EnvData &env)
{
  if ( isEnvironmentPosition(doc,row,col,env) ) {
     // already found position?
     if ( env.tag == EnvEnd )
        return true;

     // go one position forward
     row = env.row;
     col = env.col + 1;
  }

  // looking forward for next environment
  return findEnvironment(doc,row,col,env,false);
}

//////////////////// search for the last non-nested environment  ////////////////////

bool KileEdit::findEnvironment(Kate::Document *doc, uint row, uint col,
                           EnvData &env,bool backwards)
{   
  KTextEditor::SearchInterface *iface;
  iface = dynamic_cast<KTextEditor::SearchInterface *>(doc);
  QRegExp reg("\\\\(begin|end)\\s*\\{\\s*([A-Za-z]+\\*?)\\s*\\}");

  uint envcount = 0;
  QString wrong_env = ( backwards ) ? "end" : "begin";
  while ( iface->searchText(row,col,reg,&env.row,&env.col,&env.len,backwards) ) {
     if ( isValidBackslash(doc,env.row,env.col) )
     {
       if ( reg.cap(1) == wrong_env )
       {
             envcount++;
        }
       else
       {
          if ( envcount > 0 )
          {
             envcount--;
          }
          else
          {
            env.name = reg.cap(2);
            return true;
          }
       }
     }

     // new start position
     if ( !backwards ) {
        row = env.row;
        col = env.col + 1;
     } else {
       row = env.row;
       col = env.col;
       if ( ! decreaseCursorPosition(doc,row,col) )
          return false;
     }
   }

   return false;

  /*
  // B -->
  bool found = false;  
  uint brackets = 0;
  for ( uint line=row; line<doc->numLines() && !found; line++ ) {
     QString textline = doc->textLine(line);
     bool valid_char = true;
     uint start = ( line == row ) ? col : 0;
     for ( uint i=start; i<textline.length(); i++ ) {
        bool value = true;
        if ( textline[i] == '%' )
        {
           if ( valid_char )
              break;                          
        }
        else if ( textline[i] == '{' )
        {
          if ( valid_char )
             brackets++;
        }
        else if ( textline[i] == '}' )
        {
          if ( valid_char ) {
             if ( brackets > 0 )
                brackets--;
             else
                { found=true; row=line; col=i; break; }
          }
        }
        else if ( textline[i] == '\\' )
        {
           value = !valid_char;
        }

        valid_char = value;
     }      
   }
   */

   /*
  // B <-- 
  bool found = false;
  uint brackets = 0;
  for ( int line=row; line>0 && !found; line-- ) {
     QString textline = doc->textLine(line);
     bool valid_char = true;
     uint start = ( line == row ) ? col : textline.length() - 1;
     for ( int i=start; i>=0; i-- ) {
        bool value = true;
        if ( textline[i] == '{' )
        {
          if ( valid_char )
             brackets++;
        }
        else if ( textline[i] == '}' )
        {
          if ( valid_char ) {
             if ( brackets > 0 )
                brackets--;
             else
                { found=true; row=line; col=i; break; }
          }
        }
        else if ( textline[i] == '\\' )
        {
           value = !valid_char;
        }

        valid_char = value;
     }
   }

   if ( found ) {
       kdDebug() << "   found: " << row << "/" << col << endl;
       view->setCursorPositionReal(row,col);
   }
   */
   
}

//////////////////// check for an environment position ////////////////////

// check if the current position belongs to an environment

bool KileEdit::isEnvironmentPosition(Kate::Document *doc, uint row, uint col, EnvData &env)
{
  // get real textline without comments, quoted comment signs and pairs of backslashes
  QString textline = getTextLineReal(doc,row);

  if ( col > textline.length() )
     return false;

  KTextEditor::SearchInterface *iface;
  iface = dynamic_cast<KTextEditor::SearchInterface *>(doc);
  QRegExp reg("\\\\(begin|end)\\s*\\{\\s*([A-Za-z]+\\*?)\\s*\\}");

  // check if there is a match in this line from the current position to the right
  if ( textline[col]=='\\' && col==(uint)textline.find(reg,col) ) {
     env.row = row;
     env.col = col;
     env.len = reg.matchedLength();
     env.tag = ( textline.at(env.col+1) == 'b' ) ? EnvBegin : EnvEnd;
     env.name = reg.cap(2);
     return true;
  }

  // no, then check if there is a match in this line from the current position to the left
  int pos = textline.findRev(reg,col);
  env.len = reg.matchedLength();
  if ( pos!=-1 && (uint)pos<=col && col<(uint)pos+env.len ) {
     env.row = row;
     env.col = pos;
     env.tag = ( textline.at(env.col+1) == 'b' ) ? EnvBegin : EnvEnd;
     env.name = reg.cap(2);
     return true;
  }
       
  return false;
}

//////////////////// get real text ////////////////////

// get current textline and remove
//  - all paurs of backslashes
//  - all quoted comment signs
//  - all comment

QString KileEdit::getTextLineReal(Kate::Document *doc, uint row)
{
   QString textline = doc->textLine(row);
   textline.replace("\\\\","&&");
   textline.replace("\\%","&&");
   int pos = textline.find('%');
   return (pos == -1 ) ? textline : textline.left(pos);   
}

//////////////////// check for a comment ////////////////////

// check if the current position is within a comment

bool KileEdit::isCommentPosition(Kate::Document *doc, uint row, uint col)
{
   QString textline = doc->textLine(row);
   
   bool backslash = false;
   for ( uint i=0; i<col; i++ ) {
      if ( textline[i] == '%' ) {
         if ( !backslash )
            return true;                  // found a comment sign
         else
            backslash = false;
      } else if ( textline[i] == '\\' )   // count number of backslashes
         backslash = !backslash;
      else
         backslash = false;               // no backslash
   }

   return false;
}

// check if the character at text[col] is a valid backslash:
//  - there is no comment sign in this line before
//  - there is not a odd number of backslashes directly before

bool KileEdit::isValidBackslash(Kate::Document *doc, uint row, uint col)
{
   QString textline = doc->textLine(row);
   
   bool backslash = false;
   for ( uint i=0; i<col; i++ ) {
      if ( textline[i] == '%' ) {
         if ( !backslash )
            return false;                 // found a comment sign
         else
            backslash = false;
      } else if ( textline[i] == '\\' )   // count number of backslashes
         backslash = !backslash;
      else
         backslash = false;               // no backslash
   }

   return !backslash;
}

//////////////////// goto next bullet ////////////////////

void KileEdit::gotoBullet(Kate::View *view, const QString &bullet, bool backwards)
{
   if ( !view ) return;

   uint row,col,ypos,xpos,len;

   // get current position
   Kate::Document *doc = view->getDoc();
   view->cursorPositionReal(&row,&col);

   // change the start position or we will stay at this place
   if ( backwards )
   {
      if ( ! decreaseCursorPosition(doc,row,col) )
         return;
   }
   else
   {
      if ( ! increaseCursorPosition(doc,row,col) )
         return;
   }

   if ( doc->searchText(row,col,bullet,&ypos,&xpos,&len,true,backwards) ) {
      doc->setSelection(ypos,xpos,ypos,xpos+1);
      view->setCursorPositionReal(ypos,xpos);
   }
}

//////////////////// increase/decrease cursor position ////////////////////

bool KileEdit::increaseCursorPosition(Kate::Document *doc, uint &row, uint &col)
{
   bool ok = true;
   
   if ( (int)col < doc->lineLength(row)-1 )
      col++;
   else if ( row < doc->numLines() - 1 ) {
      row++;
      col=0;
   } else
      ok = false;

   return ok;
}

bool KileEdit::decreaseCursorPosition(Kate::Document *doc, uint &row, uint &col)
{
   bool ok = true;

   if (col > 0)
      col--;
   else if ( row > 0 ) {
      row--;
      col = doc->lineLength(row) - 1;
   } else
      ok = false;

   return ok;
}

//////////////////// check the current type of environment ////////////////////

bool KileEdit::isListEnvironment(const QString &name)
{
   return (name=="description" || name=="enumerate" || name=="itemize" ) ? true : false;
}

bool KileEdit::isTabEnvironment(const QString &name)
{
   return (name=="tabular" || name=="tabular*" || name=="tabbing" || name=="array" ) ? true : false;
}


