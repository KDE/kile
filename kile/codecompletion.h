/***************************************************************************
                           codecompletion.h 
----------------------------------------------------------------------------
    date                 : Jan 24 2004
    version              : 0.10.3
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

#include <qobject.h>

#include <kate/view.h>
#include <kate/document.h>
#include <ktexteditor/codecompletioninterface.h>

#define BULLET QString("×")

/**
  *@author Holger Danielsson
  */

class QTimer;

class KileInfo;

namespace KileDocument
{

//FIXME fix the way the Kate::View is passed, I'm not 100% confident m_view doesn't turn into a wild pointer
//FIXME refactor the complete class, it's pretty ugly, there are too many methods with similar names suggesting that the code could be more efficient
class CodeCompletion : public QObject
{
	Q_OBJECT

public:        
	CodeCompletion(KileInfo *ki);
	~CodeCompletion();

	enum Mode
	{
		cmLatex,
		cmEnvironment,
		cmDictionary,
		cmAbbreviation,
		cmLabel
	};

	enum Type
	{
		ctNone,
		ctReference,
		ctCitation
	};

	bool isActive();
	bool inProgress();
	bool autoComplete();
	CodeCompletion::Mode getMode();
	CodeCompletion::Type getType();
	CodeCompletion::Type getType(const QString &text);

	KileInfo* info() const { return m_ki;}

	void readConfig(void);

public slots:
	const QString getBullet();

	//in these two methods we should set m_view
	void slotCharactersInserted(int, int, const QString&);
	void editComplete(Kate::View *view, KileDocument::CodeCompletion::Mode mode);

	void slotCompletionDone( );
	void slotCompleteValueList();
	void slotCompletionAborted();

	void slotFilterCompletion(KTextEditor::CompletionEntry* c,QString *s);

private:
	void completeWord(const QString &text, CodeCompletion::Mode mode);
	QString filterCompletionText(const QString &text, const QString &type);

	void CompletionDone();
	void CompletionAborted();

	void completeFromList(const QStringList *list);
	void editCompleteList(KileDocument::CodeCompletion::Type type);
	bool getCompleteWord(bool latexmode, QString &text, KileDocument::CodeCompletion::Type &type);
	bool oddBackslashes(const QString& text, int index);

	void appendNewCommands(QValueList<KTextEditor::CompletionEntry> & list);

private:
	// wordlists
	QValueList<KTextEditor::CompletionEntry> m_texlist;
	QValueList<KTextEditor::CompletionEntry> m_dictlist;
	QValueList<KTextEditor::CompletionEntry> m_abbrevlist;
	QValueList<KTextEditor::CompletionEntry> m_labellist;

	KileInfo *m_ki;
	QTimer *m_completeTimer;

	// some flags
	bool m_isenabled;
	bool m_setcursor;
	bool m_setbullets;
	bool m_closeenv;
	bool m_autocomplete;

	// state of complete: some flags
	bool m_firstconfig;
	bool m_inprogress;

	// undo text
	bool m_undo;

	// character which is used as bullet
	QString m_bullet;

	// special types: ref, bib
	CodeCompletion::Type m_type;

	// internal parameter
	Kate::View *m_view;                  // View
	QString m_text;                      // current pattern
	uint m_textlen;                      // length of current pattern
	CodeCompletion::Mode m_mode;         // completion mode
	uint m_ycursor,m_xcursor,m_xstart;   // current cursor position
	uint m_yoffset,m_xoffset;            // offset of the new cursor position

	QString buildLatexText(const QString &text, uint &ypos, uint &xpos);
	QString buildEnvironmentText(const QString &text, const QString &type, uint &ypos, uint &xpos);
	QString buildAbbreviationText(const QString &text);
	QString buildLabelText(const QString &text);

	QString parseText(const QString &text, uint &ypos, uint &xpos, bool checkgroup);
	QString stripParameter(const QString &text);

	void setWordlist(const QStringList &files,const QString &dir, QValueList<KTextEditor::CompletionEntry> *entrylist);
	void readWordlist(QStringList &wordlist, const QString &filename);
	void setCompletionEntries(QValueList<KTextEditor::CompletionEntry> *list, const QStringList &wordlist);

	uint countEntries(const QString &pattern, QValueList<KTextEditor::CompletionEntry> *list, QString *entry, QString *type);
};

}

#endif
