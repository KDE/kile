/************************************************************************************************
    date                 : Mar 21 2007
    version              : 0.40
    copyright            : (C) 2004-2007 by Holger Danielsson (holger.danielsson@versanet.de)
                               2008-2009 by Michel Ludwig (michel.ludwig@kdemail.net)
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
#include <KTextEditor/CodeCompletionModelControllerInterface>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <kconfig.h>

#include "latexcmd.h"
#include "widgets/abbreviationview.h"

//default bullet char (a cross)
static const QChar s_bullet_char = QChar(0xd7);
static const QString s_bullet = QString(&s_bullet_char, 1);

class QTimer;

class KileInfo;

namespace KileDocument { class EditorExtension; }

namespace KileCodeCompletion
{
	class Manager;

	class LaTeXCompletionModel : public KTextEditor::CodeCompletionModel, public KTextEditor::CodeCompletionModelControllerInterface {
		Q_OBJECT
		Q_INTERFACES(KTextEditor::CodeCompletionModelControllerInterface)
		
		public:
			LaTeXCompletionModel(QObject *parent, KileCodeCompletion::Manager *manager,
			                                      KileDocument::EditorExtension *editorExtension);
			virtual ~LaTeXCompletionModel();

			virtual QModelIndex index (int row, int column, const QModelIndex &parent=QModelIndex()) const;
			virtual QVariant data(const QModelIndex& index, int role) const;
			virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;

			virtual bool shouldAbortCompletion(KTextEditor::View *view, const KTextEditor::SmartRange &range,
			                                                            const QString &currentCompletion);
			virtual void completionInvoked(KTextEditor::View *view, const KTextEditor::Range &range,
			                                                        InvocationType invocationType);
			virtual void updateCompletionRange(KTextEditor::View *view, KTextEditor::SmartRange &range);
			virtual KTextEditor::Range completionRange(KTextEditor::View *view,
			                                           const KTextEditor::Cursor &position);
			virtual QString filterString(KTextEditor::View *view,
			                             const KTextEditor::SmartRange &range,
			                             const KTextEditor::Cursor &position);

			virtual void executeCompletionItem(KTextEditor::Document *document, const KTextEditor::Range& word,
			                                                                    int row) const;

		protected:
			KileCodeCompletion::Manager *m_codeCompletionManager;
			KileDocument::EditorExtension *m_editorExtension;
			QStringList m_completionList;
			KTextEditor::View *m_currentView;

			void buildModel(KTextEditor::View *view, const KTextEditor::Range &r);
			void filterModel(const QString& text);
			
			QString stripParameters(const QString &text) const;
			QString buildRegularCompletedText(const QString &text, int &cursorYPos, int &cursorXPos,
			                                                                        bool checkGroup) const;
			QString buildEnvironmentCompletedText(const QString &text, const QString &prefix,
                                                              int &ypos, int &xpos) const;
			QString buildWhiteSpaceString(const QString &s) const;
			KTextEditor::Cursor determineLaTeXCommandStart(KTextEditor::Document *doc,
			                                               const KTextEditor::Cursor& position) const;
			bool isWithinLaTeXCommand(KTextEditor::Document *doc, const KTextEditor::Cursor& commandStart,
			                                                      const KTextEditor::Cursor& cursorPosition) const;
};

	class Manager : public QObject {
		Q_OBJECT
		friend class LaTeXCompletionModel;

		public:
			Manager(KileInfo *info, QObject *parent);
			virtual ~Manager();

			QStringList getLaTeXCommands() const;
			QStringList getLocallyDefinedLaTeXCommands(KTextEditor::View *view) const;

			void readConfig(KConfig *config);

		public Q_SLOTS:
			void startLaTeXCompletion(KTextEditor::View *view = NULL);
			void startLaTeXEnvironment(KTextEditor::View *view = NULL);

		protected:
			KileInfo* m_ki;
			QStringList m_texWordList, m_dictWordList, m_abbrevWordList;
			bool m_firstConfig;
			QRegExp m_referencesRegExp;
			QRegExp m_referencesExtRegExp;
			QRegExp m_citeRegExp;
			QRegExp m_citeExtRegExp;
	
			void addUserDefinedLaTeXCommands(QStringList &wordlist);
			void readCWLFile(QStringList &wordlist, const QString &filename);
			QStringList readCWLFiles(const QStringList &files, const QString &dir);
			void buildReferenceCitationRegularExpressions();
			QString getCommandsString(KileDocument::CmdAttribute attrtype);
}	;

}

namespace KileDocument
{
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

	bool inProgress(KTextEditor::View *view);
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

// 	void slotCompletionDone(KTextEditor::CompletionEntry);
	void slotCompleteValueList();
	void slotCompletionAborted();

// 	void slotFilterCompletion(KTextEditor::CompletionEntry* c, QString *s);

	// a abbreviation was modified ind the abbreviation view (add, edit or delete)
	// so the abbreviation list was must also be updated
	void slotUpdateAbbrevList(const QString &ds, const QString &as);

private:
	void completeWord(KTextEditor::View* view, const KTextEditor::Range& range, CodeCompletion::Mode mode);
	QString filterCompletionText(KTextEditor::View *view, const QString &text, const QString &type);

// 	void CompletionDone(KTextEditor::CompletionEntry);
	void CompletionAborted();

	void completeFromList(KTextEditor::View* view, const QStringList *list, const KTextEditor::Cursor &position, const QString &pattern = QString());
	void editCompleteList(KTextEditor::View* view, KileDocument::CodeCompletion::Type type, const KTextEditor::Cursor &position, const QString &pattern = QString());
	KTextEditor::Range getCompleteWord(KTextEditor::View *view, bool latexmode, KileDocument::CodeCompletion::Type &type);
	KTextEditor::Range getReferenceWord(KTextEditor::View *view);
	bool oddBackslashes(const QString& text, int index);

	void appendNewCommands(QStringList& list);
	QStringList getDocumentWords(KTextEditor::View *view, const QString &text);

	bool completeAutoAbbreviation(KTextEditor::View *view, const QString &text);
	QString getAbbreviationWord(KTextEditor::View *view, int row, int col);

	CodeCompletion::Type insideReference(KTextEditor::View *view, QString &startpattern);

private:
	// wordlists
	QStringList m_texlist;
	QStringList m_dictlist;
	QStringList m_abbrevlist;
	QStringList m_labellist;

	KileInfo *m_ki;
	QTimer *m_completeTimer;

	// some flags
	bool m_setcursor;
	bool m_setbullets;
	bool m_closeenv;
	bool m_autocomplete;
	bool m_autocompleteabbrev;
	bool m_citationMove;
	bool m_autoDollar;
	int  m_latexthreshold;

	// flags from Kate configuration
	bool m_autobrackets;
	bool m_autoindent;

	// state of complete: some flags
	bool m_firstconfig;
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
	QString m_text;                       // current pattern
	int m_textlen;                        // length of current pattern
	CodeCompletion::Mode m_mode;          // completion mode
	int m_ycursor, m_xcursor, m_xstart;   // current cursor position
	int m_yoffset, m_xoffset;             // offset of the new cursor position

	QString buildLatexText(const QString &text, int &ypos, int &xpos);
	QString buildEnvironmentText(const QString &text, const QString &type, const QString &prefix, int &ypos, int &xpos);
	QString getWhiteSpace(const QString &s);
	QString buildAbbreviationText(KTextEditor::View *view, const QString &text);
	QString buildLabelText(KTextEditor::View *view, const QString &text);

	QString parseText(const QString &text, int &ypos, int &xpos, bool checkgroup);
	QString stripParameter(const QString &text);

	void setReferences();
	QString getCommandList(KileDocument::CmdAttribute attrtype);

	int countEntries(const QString &pattern, const QStringList& list, QString *entry);

	void addAbbreviationEntry(const QString &entry);
	void deleteAbbreviationEntry(const QString &entry);
	QString findExpansion(const QString &abbrev);

	void autoInsertDollar();
};

}

#endif
