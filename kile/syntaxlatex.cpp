/***************************************************************************
                          syntaxlatex.cpp  -  description
                             -------------------
    begin                : Sun Dec 30 2001
    copyright            : (C) 2003 by Jeroen Wijnhout
    email                : Jeroen.Wijnhout@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "syntaxlatex.h"
#include <qstring.h>
#include <qstringlist.h>
#include <qapplication.h>
#include <qregexp.h>
#include <qmap.h>
#include <paragdata.h>
#include <qcolor.h>
#include <qfont.h>
#include <qtextedit.h>

#include "kdebug.h"

const char * const SyntaxLatex::Keywords[] = {
    "section",
    "subsection",
    "subsubsection",
    "chapter",
    "part",
    "paragraph",
    "subparagraph",
    "section*",
    "subsection*",
    "subsubsection*",
    "chapter*",
    "part*",
    "paragraph*",
    "subparagraph*",
    "label",
    "includegraphics",
    "include",
    "input",
    0
};
const char * const SyntaxLatex::Environment[] = {
    "begin",
    "end",
    0
};

SyntaxLatex::SyntaxLatex(QTextEdit *textEdit, ListColors col, QFont &efont) : QSyntaxHighlighter(textEdit)
{
  changeSettings(col, efont);
}

SyntaxLatex::~SyntaxLatex()
{

}

void SyntaxLatex::changeSettings(ListColors col, QFont &efont)
{
		for ( int i=0; Keywords[i] != 0; i++)
    {
			mapCommands[Keywords[i]]=stKeywordDetected;
		}

		mapCommands[Environment[0]]=stEnvironmentDetected;
		mapCommands[Environment[1]]=stEndDetected;

    LaTeXFormat *fmt;

    fmt = new LaTeXFormat;
    fmt->color=col[1];
    fmt->font=efont;
    dictFormats.insert(fmtStandard,fmt);

    fmt = new LaTeXFormat;
    fmt->color=col[2];
    fmt->font=efont;
    fmt->font.setItalic(true);
    dictFormats.insert(fmtComment,fmt);

    fmt = new LaTeXFormat;
    fmt->color=col[3];
    fmt->font=efont;
    dictFormats.insert(fmtMath,fmt);

    fmt = new LaTeXFormat;
    fmt->color=col[4];
    fmt->font=efont;
    dictFormats.insert(fmtControlSequence,fmt);

    fmt = new LaTeXFormat;
    fmt->color=col[5];
    fmt->font=efont;
    dictFormats.insert(fmtKeyword,fmt);

    fmt = new LaTeXFormat;
    fmt->color=col[6];
    fmt->font=efont;
    dictFormats.insert(fmtEnvironment,fmt);

    listVerbChars.clear();
}

void SyntaxLatex::setLaTeXFormat(int start, int count, int format)
{
	LaTeXFormat *fmt = dictFormats.find(format);
	setFormat(start,count, fmt->font, fmt->color);
}

int SyntaxLatex::highlightParagraph ( const QString & text, int endStateOfLastPara )
{
	Formats fmt;
	int state = endStateOfLastPara, len = text.length(),
		cmmnd_start=0, env_start=0, key=0,ind=0;
  QChar ch, verbatimDelimiter;
  QString cmmnd,env;

  //kdDebug() << "new paragraph: " << text << " state " << state << endl;

  //begin of file, set to standard
  if ( state == -2) { state = stStandard;}

  //there is extra information stored in state : the verbatimdelimiter
  if (state > 31) {
		key= state>>5;
		verbatimDelimiter = listVerbChars[key - 1];
		//kdDebug() << "verbatimdelim index " << key<< endl;
		state = (state - 32 * key);
		//kdDebug() << "continuing with delimiter: " << static_cast<char>(verbatimDelimiter.latin1()) << endl;
		//kdDebug() << "real state " << state << endl;
	}



	for (int i=0; i<len; i++)
	{
		ch = text.at(i);
		//kdDebug() << "considering " << static_cast<int>(ch.latin1()) << endl;

		//determine the state change
		switch (state)
    {
				case stStandard :
				case stMathEnded :
				case stKeywordDetected :
						switch (ch)
					  {
							case TEX_CAT0		: state = stControlSequence;break;
							case TEX_CAT3		: state = stMath;break;
							case TEX_CAT14	: state = stComment;break;
							default : state = stStandard; break;
						};
						//kdDebug() << "changing from Standard/MathEnded/KeywordDetected to " << state << endl;
				break; //standard

	      case stComment :
	      		//done with line, reset to standard
	      		if (i == len-1) state = stStandard;
	      break; //comment

	      case stMath :
	      		switch (ch)
	        	{
						 	case TEX_CAT0	: state = stMathControlSequence; break;
						  case TEX_CAT3	: state = stMathEnded; break;
						  default : break;
						}
	      break; //math

	      case stMathControlSequence :
	      		//we need this because we should not let \$ inside $$ end the math mode
	      		state = stMath;
	      break; //mathcontrolsequence

	      case stControlSequence :
			      //determine whether to expect a symbol or a command
			      if ( ch.isLetter() )
			      {
							state = stCommand;
							cmmnd_start=i;
							//kdDebug() << "starting command at : " << cmmnd_start << endl;
						}
			      else state = stControlSymbol;
			  break; //controlsequence

			  case stControlSymbol :
			    	switch (ch)
						{
							case TEX_CAT0		: state = stControlSequence; break;
							case TEX_CAT3		: state = stMath; break;
							case TEX_CAT14	: state = stComment; break;
							default : state = stStandard; break;
						};
			  break;

	      case stCommand :
	      			//no letter signals the ending of a command
	      			if ( !ch.isLetter())
	         		{
							 	//character that ends the command also changes the state
							 	switch (ch)
							  {
									case TEX_CAT0		: state = stControlSequence; break;
									case TEX_CAT3		: state = stMath; break;
									case TEX_CAT14	: state = stComment; break;
									default : state = stStandard; break;
								};

								cmmnd = text.mid(cmmnd_start,i-cmmnd_start);

							  //kdDebug() << "command detected: " << cmmnd << " reason " << ch.latin1() << endl;
							  //determine what to do next
							  switch ( *(mapCommands.find(cmmnd)) )
							  {
									case stKeywordDetected :
										//kdDebug() << "keyword detected: " << cmmnd << " start " << cmmnd_start-1 << " count " << i-cmmnd_start+1 << endl;
										//quick and dirty: reformat the command
										setLaTeXFormat(cmmnd_start-1,i-cmmnd_start+1,fmtKeyword);
									break;

									case stEnvironmentDetected :
										setLaTeXFormat(cmmnd_start-1,i-cmmnd_start+1,fmtEnvironment);
										if ( ch.isSpace() || ( ch == TEX_CAT1 ) ) state=stEnvironmentDetected;
									break;

									case stEndDetected :
										setLaTeXFormat(cmmnd_start-1,i-cmmnd_start+1,fmtEnvironment);
									break;

									default:
										//kdDebug() << "not a keyword or environment" << endl;
										if ( cmmnd == "verb" )
										{
											//FIXME: there seems to be a space on the end
											//of every line, this is bothering me quite a lot
											//since that would mean that we always detect a
											//space as the verbatim delimiter
											if (ch != ' ')
											{
												verbatimDelimiter=ch;
												ind= listVerbChars.findIndex(ch);
												if ( ind < 0)
												{
													listVerbChars.append(ch);
													key=listVerbChars.size();
												}
												else
												{
													key=ind+1;
												}
                        state=stVerbatimDetected;
                        //kdDebug() << "going verbatim with " << ch.latin1() << " at " << listVerbChars.size() << endl;
                        for (QValueListIterator<QChar> i = listVerbChars.begin(); i != listVerbChars.end(); i++ )
                        {
													//kdDebug() << static_cast<char>(*i) << endl;
												}
											}
										}

									break;
								} //switch mapCommands
							} //if not letter
				break; //command

				case stVerbatimDetected :
						if ( ch == verbatimDelimiter )
						{
							state = stStandard;
							key=0;
						}
				break;

				case stEnvironmentDetected :
						if ( ch.isLetter() )
            {
							state = stDeterminingEnv; env_start=i;
						}
						else if ( !ch.isSpace() )
						{
							//kdDebug() << "eaten all space" << ch.latin1() << endl;
							switch (ch)
							{
								case TEX_CAT0		: state = stControlSequence; break;
								case TEX_CAT1		: state = stDeterminingEnv; env_start=i+1; break;
								case TEX_CAT3		: state = stMath; break;
								case TEX_CAT14	: state = stComment; break;
								default : state = stStandard; break;
							};
						}

				break;

				case stDeterminingEnv :
						if ( ch == TEX_CAT2 )
						{
								env=text.mid(env_start,i-env_start);
								//kdDebug() << "environment : " << env << endl;
								state=stStandard;
								if ( env == "verbatim" )
								{
									state = stVerbatimEnv;
									//kdDebug() << "verbatim environment" << endl;
								}
						}
				break;

				case stVerbatimEnv :
						if ( ch == TEX_CAT0 )
						{
							//kdDebug() << "possible ending: " << text.mid(i+1,13) << " distance " << len-i << endl;
							if ( ((len-i) > 13) && text.mid(i+1,13) == VERBATIM_END )
							{
								state=stControlSequence;
								//kdDebug() << "verbatim environment ending" << endl;
							}
						}
				break;

				default :
					//kdDebug() << "WARNING WARNING state " << state << " not handled" << endl;
				break;

		} //switch state, change state

      //set the format corresponding to the new state
    switch (state)
    {
				case stStandard: fmt = fmtStandard; break;
				case stComment : fmt = fmtComment;  break;

				case stControlSequence :
				case stControlSymbol :
				case stCommand : fmt = fmtControlSequence; break;

				case stMath :
				case stMathEnded :
				case stMathControlSequence : fmt = fmtMath; break;

				case stKeywordDetected : fmt = fmtKeyword; break;

				default : fmt= fmtStandard; break;
		} //switch state, determine format

		setLaTeXFormat(i,1,fmt);
		//kdDebug() << "processed: " << ch.latin1() << " state: " << state << endl;
	}// end main for loop

	//if end state is verbatimdetected, encode the verbatimdelimiter in the state
	//kdDebug() << "saving key " << key << " into the endstate " << endl;
  if (key > 0)
	{
			state += (key << 5) ;
	}

  //kdDebug() << "endstate: " << state << endl;
	return state;
}
