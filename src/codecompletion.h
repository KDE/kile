/************************************************************************************************
    date                 : Mar 21 2007
    version              : 0.40
    copyright            : (C) 2004-2007 by Holger Danielsson (holger.danielsson@versanet.de)
                               2008 by Michel Ludwig (michel.ludwig@kdemail.net)
 ************************************************************************************************/

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

#include <QObject>
#include <QList>

#include <KTextEditor/CodeCompletionInterface>
#include <KTextEditor/CodeCompletionModel>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <kconfig.h>

#include "latexcmd.h"
#include "widgets/abbreviationview.h"

namespace KTextEditor {class CompletionEntry;}

//default bullet char (a cross)
static const QChar s_bullet_char = QChar(0xd7);
static const QString s_bullet = QString(&s_bullet_char, 1);

class QTimer;

class KileInfo;

namespace KileDocument
{
	class CodeCompletionModel : public KTextEditor::CodeCompletionModel {
		public:
			CodeCompletionModel(QObject *parent);
			virtual ~CodeCompletionModel();

			virtual QVariant data(const QModelIndex& index, int role) const;
			virtual void completionInvoked(KTextEditor::View *view, const KTextEditor::Range &range, InvocationType invocationType);
			virtual QModelIndex index(int row, int column, const QModelIndex& parent) const;
			virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
			void setCompletionList(const QStringList& list);

		protected:
			QStringList m_completionList;
	};

//FIXME fix the way the KTextEditor::View is passed, I'm not 100% confident m_view doesn't turn into a wild pointer
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
		cmLabel,
		cmDocumentWord
	};

	enum Type
	{
		ctNone,
		ctReference,
		ctCitation
	};

	enum KateConfigFlags
	{
		cfAutoIndent = 0x1,
		cfAutoBrackets = 0x40
	};

	bool isActive();
	bool inProgress();
	bool autoComplete();
	CodeCompletion::Mode getMode();
	CodeCompletion::Type getType();
	CodeCompletion::Type getType(const QString &text);

	KileInfo* info() const { return m_ki;}

	void readConfig(KConfig *config);
	void readKateConfigFlags(KConfig *config);
	void saveLocalChanges();

	void setAbbreviationListview(KileWidget::AbbreviationView *listview);

public Q_SLOTS:
	//in these two methods we should set m_view
	void textInsertedInView(KTextEditor::View *view, const KTextEditor::Cursor &position, const QString &text);
	void editComplete(KTextEditor::View *view, KileDocument::CodeCompletion::Mode mode);

	void slotCompletionDone(KTextEditor::CompletionEntry);
	void slotCompleteValueList();
	void slotCompletionAborted();

	void slotFilterCompletion(KTextEditor::CompletionEntry* c, QString *s);

	// a abbreviation was modified ind the abbreviation view (add, edit or delete)
	// so the abbreviation list was must also be updated
	void slotUpdateAbbrevList(const QString &ds, const QString &as);

private:
	void completeWord(KTextEditor::View* view, const KTextEditor::Range& range, CodeCompletion::Mode mode);
	QString filterCompletionText(KTextEditor::View *view, const QString &text, const QString &type);

	void CompletionDone(KTextEditor::CompletionEntry);
	void CompletionAborted();

	void completeFromList(KTextEditor::View* view, const QStringList *list, const KTextEditor::Cursor &position, const QString &pattern = QString());
	void editCompleteList(KTextEditor::View* view, KileDocument::CodeCompletion::Type type, const KTextEditor::Cursor &position, const QString &pattern = QString());
	KTextEditor::Range getCompleteWord(KTextEditor::View *view, bool latexmode, KileDocument::CodeCompletion::Type &type);
	KTextEditor::Range getReferenceWord(KTextEditor::View *view);
	bool oddBackslashes(const QString& text, int index);

	void appendNewCommands(QStringList& list);
	QStringList getDocumentWords(const QString &text);

	bool completeAutoAbbreviation(const QString &text);
	QString getAbbreviationWord(uint row, uint col);

	CodeCompletion::Type insideReference(QString &startpattern);

private:
	KileDocument::CodeCompletionModel *m_codeCompletionModel;

	// wordlists
	QStringList m_texlist;
	QStringList m_dictlist;
	QStringList m_abbrevlist;
	QStringList m_labellist;

	KileInfo *m_ki;
	QTimer *m_completeTimer;

	// some flags
	bool m_isenabled;
	bool m_setcursor;
	bool m_setbullets;
	bool m_closeenv;
	bool m_autocomplete;
	bool m_autocompletetext;
	bool m_autocompleteabbrev;
	bool m_citationMove;
	bool m_autoDollar;
	int  m_latexthreshold;
	int  m_textthreshold;

	// flags from Kate configuration
	bool m_autobrackets;
	bool m_autoindent;

	// state of complete: some flags
	bool m_firstconfig;
	bool m_inprogress;
	bool m_ref;
	bool m_kilecompletion;
	
	// undo text
	bool m_undo;

	// special types: ref, bib
	CodeCompletion::Type m_type;
	CodeCompletion::Type m_keylistType;

	// local abbreviation
	QString m_localAbbrevFile;
	KileWidget::AbbreviationView *m_abbrevListview;

	// internal parameter
	KTextEditor::View *m_view;                  // View
	QString m_text;                       // current pattern
	int m_textlen;                        // length of current pattern
	CodeCompletion::Mode m_mode;          // completion mode
	int m_ycursor, m_xcursor, m_xstart;   // current cursor position
	int m_yoffset, m_xoffset;             // offset of the new cursor position

	QString buildLatexText(const QString &text, int &ypos, int &xpos);
	QString buildEnvironmentText(const QString &text, const QString &type, const QString &prefix, int &ypos, int &xpos);
	QString getWhiteSpace(const QString &s);
	QString buildAbbreviationText(const QString &text);
	QString buildLabelText(const QString &text);

	QString parseText(const QString &text, int &ypos, int &xpos, bool checkgroup);
	QString stripParameter(const QString &text);

	QStringList buildWordList(const QStringList &files,const QString &dir);
	void readWordlist(QStringList &wordlist, const QString &filename, bool global);
	void addCommandsToTexlist(QStringList &wordlist);
	
	void setReferences();
	QString getCommandList(KileDocument::CmdAttribute attrtype);
	
	void setCompletionEntriesTexmode(QList<KTextEditor::CompletionEntry> *list, const QStringList &wordlist);

	int countEntries(const QString &pattern, const QStringList& list, QString *entry);

	void addAbbreviationEntry( const QString &entry );
	void deleteAbbreviationEntry( const QString &entry );
	QString findExpansion(const QString &abbrev);

	void autoInsertDollar();
};

}

#endif
