/***************************************************************************
    date                 : Feb 12 2007
    version              : 0.30
    copyright            : (C) 2004-2007 by Holger Danielsson
    email                : holger.danielsson@versanet.de
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

#include <qobject.h>
#include <qmap.h>
#include <qstring.h>

#include <kconfig.h>
#include <kmenubar.h>
#include <kate/view.h>
#include "kileedit.h"

#include "userhelp.h"
#include "texdocdialog.h"
#include "usermenudialog.h"
#include "kiletool.h"
#include "kiletoolmanager.h"

/**
  *@author Holger Danielsson
  */

namespace KileDocument { class EditorExtension; }

namespace KileHelp
{

	enum Type 
	{
		HelpKileRefs,
		HelpTexRefs,
		HelpLatexIndex,
		HelpLatexCommand,
		HelpLatexSubject,
		HelpLatexEnvironment
	};

	enum TexVersion
	{
		TETEX2,
		TETEX3,
		TEXLIVE2005
	};

	class Help : public QObject
	{
		Q_OBJECT

	public:
		Help(KileDocument::EditorExtension *edit);
		~Help();
		
		void setUserhelp(KileTool::Manager *manager, KMenuBar *menubar); 
		void update();

		// calls for help
		void helpKeyword(Kate::View *view);
		void noHelpAvailableFor(const QString &word);
		void userHelpDialog() { m_userhelp->userHelpDialog(); }
		void enableUserhelpEntries(bool state);
	public slots:
		void helpTexGuide();
		void helpLatexIndex() { helpLatex(KileHelp::HelpLatexIndex); }
		void helpLatexCommand() { helpLatex(KileHelp::HelpLatexCommand); }
		void helpLatexSubject() { helpLatex(KileHelp::HelpLatexSubject); }
		void helpLatexEnvironment() { helpLatex(KileHelp::HelpLatexEnvironment); }
		void helpKeyword();
		void helpDocBrowser(); 
		
	private:
		KileTool::Manager *m_manager;
		KileDocument::EditorExtension *m_edit;
		UserHelp *m_userhelp;
		
		TexVersion m_texVersion;
		QString m_texReference;
		QString m_texdocPath;

		QMap<QString,QString> m_dictHelpKile;
		QMap<QString,QString> m_dictHelpTex;

		void initTexDocumentation();
		void readHelpList(const QString &filename,QMap<QString,QString> &map);
		void showHelpFile(const QString &parameter);

		void helpLatex(KileHelp::Type type);
		void helpTexRefsKeyword(Kate::View *view);
		void helpKileRefsKeyword(Kate::View *view);
		QString getKeyword(Kate::View *view);
	};
}

#endif
