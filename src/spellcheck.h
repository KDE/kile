/**************************************************************************
*   Copyright (C) 2008 by Michel Ludwig (michel.ludwig@kdemail.net)       *
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef SPELLCHECK_H
#define SPELLCHECK_H

#include <QList>
#include <QObject>
#include <QPair>
#include <QMutex>
#include <QString>
#include <QThread>
#include <QWaitCondition>

#include <KTextEditor/Document>
#include <sonnet/backgroundchecker.h>

class KileInfo;

namespace KileDocument {class TextInfo;};
namespace KileView {class Manager;};

namespace KileSpellCheck {

	class Manager;

	class OnTheFlyChecker : public QObject {
		Q_OBJECT

		friend class Manager;

		typedef QPair<int, QString> SpellCheckLine;
		typedef QPair<KTextEditor::Document*, SpellCheckLine> SpellCheckQueueItem;

		public:
			OnTheFlyChecker(QObject *parent = NULL);
			~OnTheFlyChecker();

		public Q_SLOTS:
			void spellCheckLine();
			void textInserted(KTextEditor::Document *document, const KTextEditor::Range &range);
			void textRemoved(KTextEditor::Document *document, const KTextEditor::Range &range);
			void freeDocument(KTextEditor::Document *document);

			void stop();

		protected:
			QList<SpellCheckQueueItem> m_spellCheckQueue;
			Sonnet::BackgroundChecker *m_backgroundChecker;
			SpellCheckQueueItem m_currentlyCheckedLine;
			static const SpellCheckQueueItem invalidSpellCheckQueueItem;
			bool m_stop;

		protected Q_SLOTS:
			void misspelling(const QString &word, int start);
			void spellCheckDone();
	};

	class Manager : public QThread {
		Q_OBJECT

		public:
			Manager(QObject *parent, KileView::Manager *viewManager);
			virtual ~Manager();

			void onTheFlyCheckDocument(KTextEditor::Document *document);
			void addOnTheFlySpellChecking(KileDocument::TextInfo *info);
			void removeOnTheFlySpellChecking(KTextEditor::Document *doc);

		Q_SIGNALS:
			void stop();
			void textInserted(KTextEditor::Document*, const KTextEditor::Range&);
			void textRemoved(KTextEditor::Document*, const KTextEditor::Range&);
			void freeDocument(KTextEditor::Document*);

		public Q_SLOTS:
			void setOnTheFlySpellCheckEnabled(bool b);

		protected:
			KileView::Manager* m_viewManager;
			QMutex m_onTheFlyCheckerMutex;
			bool m_onTheFlyCheckerSetup;
			QWaitCondition m_onTheFlyCheckerSetupWaitCondition;

			void startOnTheFlySpellCheckThread();
			void stopOnTheFlySpellCheckThread();
			void removeOnTheFlyHighlighting();

			virtual void run();

		protected Q_SLOTS:
			void onTheFlyCheckOpenDocuments();
	};
}

#endif
 
