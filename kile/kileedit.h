/***************************************************************************
    date                 : Mar 05 2005
    version              : 0.11
    copyright            : (C) 2004-2005 by Holger Danielsson
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

#include <qobject.h>
#include <qregexp.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>

#include <kate/document.h>

#include "codecompletion.h"

/**
  *@author Holger Danielsson
  */

class KileInfo;
namespace KileAction { class TagData; }

namespace KileDocument
{
  
class EditorExtension : public QObject
{
	Q_OBJECT

public:
	EditorExtension(KileInfo *);
	~EditorExtension();

	enum EnvType { EnvNone, EnvList, EnvTab, EnvCrTab };

	enum SelectMode { smTex, smLetter, smWord, smNospace };

	void readConfig(void);

	void insertTag(const KileAction::TagData& data, Kate::View *view);

	QString getTextLineReal(Kate::Document *doc, uint row);
	void gotoBullet(bool backwards, Kate::View *view = 0L);

	void gotoEnvironment(bool backwards, Kate::View *view = 0L);
	void matchEnvironment(Kate::View *view = 0L);
	void closeEnvironment(Kate::View *view = 0L);
	void selectEnvironment(bool inside, Kate::View *view = 0L);
	void deleteEnvironment(bool inside, Kate::View *view = 0L);

	void gotoTexgroup(bool backwards, Kate::View *view = 0L);
	void selectTexgroup(bool inside, Kate::View *view = 0L);
	void deleteTexgroup(bool inside, Kate::View *view = 0L);

	// get current word
	bool getCurrentWord(Kate::Document *doc,uint row,uint col, SelectMode mode,QString &word,uint &x1,uint &x2);
	QString getEnvironmentText(int &row, int &col, QString &name, Kate::View *view = 0L);

	// complete environment
	bool eventInsertEnvironment(Kate::View *view);
	
public slots:
	void insertIntelligentNewline(Kate::View *view = 0L);

	void selectEnvInside() { selectEnvironment(true); }
	void selectEnvOutside() { selectEnvironment(false); }
	void deleteEnvInside() { deleteEnvironment(true); }
	void deleteEnvOutside() {deleteEnvironment(false); }
	void gotoBeginEnv() { gotoEnvironment(true); }
	void gotoEndEnv() { gotoEnvironment(false); }
	void matchEnv() { matchEnvironment(); }
	void closeEnv() {closeEnvironment(); }
	
	void selectTexgroupInside() { selectTexgroup(true); }
	void selectTexgroupOutside() { selectTexgroup(false); }
	void deleteTexgroupInside() { deleteTexgroup(true); }
	void deleteTexgroupOutside() { deleteTexgroup(false); }
	void gotoBeginTexgroup() { gotoTexgroup(true); }
	void gotoEndTexgroup() { gotoTexgroup(false); }
	void matchTexgroup(Kate::View *view = 0L);
	void closeTexgroup(Kate::View *view = 0L);

	void selectParagraph(Kate::View *view = 0L);
	void selectLine(Kate::View *view = 0L);
	void selectWord(SelectMode mode = smTex, Kate::View *view = 0L);
	void deleteParagraph(Kate::View *view = 0L);
	void deleteWord(SelectMode mode = smTex, Kate::View *view = 0L);

	void nextBullet();
	void prevBullet();

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
	bool findEnvironmentTag(Kate::Document *doc, uint row, uint col,EnvData &env, bool backwards=false);
	bool findOpenedEnvironment(uint &row,uint &col, QString &envname, Kate::View *view);

	// get current environment
	bool getEnvironment(bool inside, EnvData &envbegin, EnvData &envend,Kate::View *view);
	bool expandSelectionEnvironment(bool inside, Kate::View *view);

	// find brackets
	bool isBracketPosition(Kate::Document *doc, uint row, uint col, BracketData &bracket);
	bool findOpenBracket(Kate::Document *doc, uint row, uint col, BracketData &bracket);
	bool findCloseBracket(Kate::Document *doc, uint row, uint col, BracketData &bracket);
	bool findCloseBracketTag(Kate::Document *doc, uint row, uint col,BracketData &bracket);
	bool findOpenBracketTag(Kate::Document *doc, uint row, uint col, BracketData &bracket);

	// get current Texgroup
	bool getTexgroup(bool inside, BracketData &open, BracketData &close, Kate::View *view);

	// find current paragraph
	bool findCurrentTexParagraph(uint &startline, uint &endline, Kate::View *view);

	// environments
	QMap<QString,bool> m_dictListEnv;
	QMap<QString,bool> m_dictMathEnv;
	QMap<QString,bool> m_dictTabularEnv;
	void setEnvironment(const QStringList &list, QMap<QString,bool> &map);

	// check environment type
	bool isListEnvironment(const QString &name);
	bool isMathEnvironment(const QString &name);
	bool isTabEnvironment(const QString &name);

	// check environment type
	bool shouldCompleteEnv(const QString &envname, Kate::View *view);
	
	// complete environments
	QRegExp m_regexpEnter;
	
	// help
	void readHelpList(QString const &filename);

	Kate::View *determineView(Kate::View *);

	KileInfo	*m_ki;

//code completion
public slots:
	void completeWord();
	void completeEnvironment();
	void completeAbbreviation();

public:
	CodeCompletion* complete() const { return m_complete; }

private:
	CodeCompletion	*m_complete;
};

}

#endif
