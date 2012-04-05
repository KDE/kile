/***************************************************************************
    begin     : 2004
    copyright : (C) 2004-2012 by Holger Danielsson (holger.danielsson@versanet.de)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KILEHELP_H
#define KILEHELP_H

#include <QMap>

#include <KActionMenu>
#include <KTextEditor/View>

#include "editorextension.h"
#include "userhelp.h"

namespace KileDocument { class EditorExtension; }

namespace KileHelp
{

	enum HelpType
	{
		HelpKileRefs,
		HelpTexRefs,
		HelpLatex2eRefs,
		HelpLatexIndex,
		HelpLatexCommand,
		HelpLatexSubject,
		HelpLatexEnvironment
	};

	enum TexVersion
	{
		TEXLIVE_201x_TUG,
		TEXLIVE2009,
		TEXLIVE2005,
		TETEX3,
		TEX_UNKNOWN
	};

	class Help : public QObject
	{
		Q_OBJECT

	public:
		Help(KileDocument::EditorExtension *edit, QWidget *mainWindow);
		~Help();

		void setUserhelp(KileTool::Manager *manager, KActionMenu *userHelpActionMenu);
		void update();

		// calls for help
		void helpKeyword(KTextEditor::View *view);
		void noHelpAvailableFor(const QString &word);
		void userHelpDialog() { m_userhelp->userHelpDialog(); }
		void enableUserhelpEntries(bool state);

	public Q_SLOTS:
		void helpTexGuide();
		void helpLatexIndex() { helpLatex(KileHelp::HelpLatexIndex); }
		void helpLatexCommand() { helpLatex(KileHelp::HelpLatexCommand); }
		void helpLatexSubject() { helpLatex(KileHelp::HelpLatexSubject); }
		void helpLatexEnvironment() { helpLatex(KileHelp::HelpLatexEnvironment); }
		void helpKeyword();
		void helpDocBrowser();

	private:
		QWidget *m_mainWindow;
		KileTool::Manager *m_manager;
		KileDocument::EditorExtension *m_edit;
		UserHelp *m_userhelp;
		QString m_helpDir;

		TexVersion m_texVersion;
		QString m_texVersionText;
		QString m_texlivePath;
		QString m_texdocPath;

		QString m_latex2eReference;
		QString m_texrefsReference;
		QString m_kileReference;

		HelpType m_contextHelpType;
		QMap<QString, QString> m_dictHelpTex;

		void initTexDocumentation();
		void initContextHelp();
		QString locateTexLivePath(const QStringList &paths);
		QString locateTexLive201x();

		void readHelpList(const QString &filename);
		void showHelpFile(const QString &parameter);

		void helpLatex(HelpType type);
		QString getKeyword(KTextEditor::View *view);
		HelpType contextHelpType();

	};
}

#endif
