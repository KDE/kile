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

	class AbbreviationCompletionModel : public KTextEditor::CodeCompletionModel, public KTextEditor::CodeCompletionModelControllerInterface {
		Q_OBJECT
		Q_INTERFACES(KTextEditor::CodeCompletionModelControllerInterface)
		
		public:
			AbbreviationCompletionModel(QObject *parent, KileAbbreviation::Manager *manager);
			virtual ~AbbreviationCompletionModel();

			virtual QModelIndex index(int row, int column, const QModelIndex &parent=QModelIndex()) const;
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
			KileAbbreviation::Manager *m_abbreviationManager;
			QStringList m_completionList;
			KTextEditor::View *m_currentView;

			void buildModel(KTextEditor::View *view, const KTextEditor::Range &r);
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

			QStringList readCWLFile(const QString &filename, bool fullPathGiven = false);
			QStringList readCWLFiles(const QStringList &files, const QString &dir);

		public Q_SLOTS:
			void startLaTeXCompletion(KTextEditor::View *view = NULL);
			void startLaTeXEnvironment(KTextEditor::View *view = NULL);
			void startAbbreviationCompletion(KTextEditor::View *view = NULL);

		protected:
			KileInfo* m_ki;
			QStringList m_texWordList, m_dictWordList, m_abbrevWordList;
			bool m_firstConfig;
			QRegExp m_referencesRegExp;
			QRegExp m_referencesExtRegExp;
			QRegExp m_citeRegExp;
			QRegExp m_citeExtRegExp;
	
			void addUserDefinedLaTeXCommands(QStringList &wordlist);
			void buildReferenceCitationRegularExpressions();
			QString getCommandsString(KileDocument::CmdAttribute attrtype);
};

}
#endif
