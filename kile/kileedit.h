/***************************************************************************
                           kileedit.h
----------------------------------------------------------------------------
    date                 : Feb 09 2004
    version              : 0.10.2
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
#include <kconfig.h>

#include <qregexp.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>

/**
  *@author Holger Danielsson
  */

class KileEdit
{
public:
	KileEdit();
	~KileEdit() {}

	enum EnvType { EnvNone, EnvList, EnvTab, EnvCrTab };

	enum SelectMode { smTex, smLetter, smWord, smNospace };

	void readConfig(KConfig *config);

	QString getTextLineReal(Kate::Document *doc, uint row);
	void gotoBullet(Kate::View *view, const QString &bullet, bool backwards);

	void gotoEnvironment(Kate::View *view, bool backwards);
	void matchEnvironment(Kate::View *view);
	void closeEnvironment(Kate::View *view);
	void selectEnvironment(Kate::View *view, bool inside);
	void deleteEnvironment(Kate::View *view, bool inside);

	void gotoTexgroup(Kate::View *view, bool backwards);
	void matchTexgroup(Kate::View *view);
	void closeTexgroup(Kate::View *view);
	void selectTexgroup(Kate::View *view, bool inside);
	void deleteTexgroup(Kate::View *view, bool inside);

	void commentSelection(Kate::View *view, bool insert);
	void spaceSelection(Kate::View *view, bool insert);
	void tabSelection(Kate::View *view, bool insert);
	void stringSelection(Kate::View *view, bool insert);

	void selectParagraph(Kate::View *view);
	void deleteParagraph(Kate::View *view);
	void selectLine(Kate::View *view);
	void selectWord(Kate::View *view,KileEdit::SelectMode mode);
	void deleteWord(Kate::View *view,KileEdit::SelectMode mode);

	void insertIntelligentNewline(Kate::View *view);

	// get current word
	bool getCurrentWord(Kate::Document *doc,uint row,uint col,KileEdit::SelectMode mode,QString &word,uint &x1,uint &x2);

	private:

	enum EnvTag { EnvBegin, EnvEnd };

	enum EnvPos { EnvLeft, EnvInside, EnvRight };

	struct EnvData 
	{
		 uint row;
		uint col;
		QString name;
		uint len;
		EnvPos cpos;
		EnvTag tag;
		EnvType type;
	};

	struct BracketData
	{
		uint row;
		uint col;
		bool open;
	};

	QRegExp m_reg;
	bool m_overwritemode;

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
	bool findEnvironmentTag(Kate::Document *doc, uint row, uint col,EnvData &env,
				bool backwards=false);
	bool findOpenedEnvironment(Kate::View *view,uint &row,uint &col, QString &envname);

	// get current environment
	bool getEnvironment(Kate::View *view, bool inside, EnvData &envbegin, EnvData &envend);

	// find brackets
	bool isBracketPosition(Kate::Document *doc, uint row, uint col, BracketData &bracket);
	bool findOpenBracket(Kate::Document *doc, uint row, uint col, BracketData &bracket);
	bool findCloseBracket(Kate::Document *doc, uint row, uint col, BracketData &bracket);
	bool findCloseBracketTag(Kate::Document *doc, uint row, uint col,BracketData &bracket);
	bool findOpenBracketTag(Kate::Document *doc, uint row, uint col, BracketData &bracket);

	// get current Texgroup
	bool getTexgroup(Kate::View *view, bool inside, BracketData &open, BracketData &close);

	// insert/remove selection
	void moveSelection(Kate::View *view, const QString &prefix,bool insertmode);

	// find current paragraph
	bool findCurrentTexParagraph(Kate::View *view,uint &startline, uint &endline);

	// environments
	QStringList listenv, mathenv,tabularenv;
	QMap<QString,bool> m_dictListEnv;
	QMap<QString,bool> m_dictMathEnv;
	QMap<QString,bool> m_dictTabularEnv;
	void setEnvironment(const QStringList &list, QMap<QString,bool> &map);

	// check environment type
	bool isListEnvironment(const QString &name);
	bool isMathEnvironment(const QString &name);
	bool isTabEnvironment(const QString &name);

	// help
	void readHelpList(QString const &filename);
};

#endif
