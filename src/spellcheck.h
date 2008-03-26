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

#include <KTextEditor/Document>
#include <sonnet/backgroundchecker.h>

class KileInfo;

namespace KileView {class Manager;};

namespace KileSpellCheck {

	class OnTheFlyChecker : public QThread {
		Q_OBJECT

		typedef QPair<int, QString> SpellCheckLine;
		typedef QPair<KTextEditor::Document*, SpellCheckLine> SpellCheckQueueItem;

		public:
			OnTheFlyChecker(QObject *parent = NULL);
			virtual ~OnTheFlyChecker();

			void textAdded(KTextEditor::Document *document, const KTextEditor::Range &range);
			void textRemoved(KTextEditor::Document *document, const KTextEditor::Range &range);
			void documentDestroyed(QObject *object);

			virtual void run();

		protected:
			QMutex m_spellCheckQueueMutex;
			QMutex m_backgroundCheckerMutex;
			QList<SpellCheckQueueItem> m_spellCheckQueue;
			Sonnet::BackgroundChecker *m_backgroundChecker;
			SpellCheckQueueItem m_currentlyCheckedLine;
			static const SpellCheckQueueItem invalidSpellCheckQueueItem;

		protected Q_SLOTS:
			void misspelling(const QString &word, int start);
			void spellCheckDone();
			void spellCheckLine();
	};

	class Manager : public QObject {
		Q_OBJECT

		public:
			Manager(QObject *parent, KileView::Manager *viewManager);
			virtual ~Manager();

			void onTheFlyCheckDocument(KTextEditor::Document *document);

		public Q_SLOTS:
			void textChanged(KTextEditor::Document *document,
			                        const KTextEditor::Range &oldRange,
			                        const KTextEditor::Range &newRange);
			void textInserted(KTextEditor::Document *document,
			                  const KTextEditor::Range &range);
			void textRemoved(KTextEditor::Document *document,
			                 const KTextEditor::Range &range);
			void documentDestroyed(QObject *object);

			void setOnTheFlySpellCheckEnabled(bool b);

		protected:
			KileView::Manager* m_viewManager;
			OnTheFlyChecker *m_onTheFlyChecker;

			void startOnTheFlySpellCheckThread();
			void stopOnTheFlySpellCheckThread();
			void removeOnTheFlyHighlighting();
	};
}

#endif
 
