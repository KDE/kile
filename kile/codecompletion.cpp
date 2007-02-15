/***************************************************************************
    date                 : Feb 15 2007
    version              : 0.32
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
 
#include <qregexp.h>
#include <qfile.h>
#include <qtimer.h>
#include <qdict.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <ktexteditor/codecompletioninterface.h>

#include "kileinfo.h"
#include "kileviewmanager.h"
#include "codecompletion.h"
#include "kileconfig.h"
#include "kileedit.h"

namespace KileDocument
{

	//static QRegExp::QRegExp reRef("^\\\\(pageref|ref)\\{");
	//static QRegExp::QRegExp reCite("^\\\\(c|C|noc)(ite|itep|itet|itealt|itealp|iteauthor|iteyear|iteyearpar|itetext)\\{");
	//static QRegExp::QRegExp reRefExt("^\\\\(pageref|ref)\\{[^\\{\\}\\\\]+,$");
	//static QRegExp::QRegExp reCiteExt("^\\\\(c|C|noc)(ite|itep|itet|itealt|itealp|iteauthor|iteyear|iteyearpar|itetext)\\{[^\\{\\}\\\\]+,$");
	
	static QRegExp::QRegExp reRef;
	static QRegExp::QRegExp reRefExt;
	static QRegExp::QRegExp reCite;
	static QRegExp::QRegExp reCiteExt;
	static QRegExp::QRegExp reNotRefChars("[^a-zA-z0-9_@\\.\\+\\-\\*\\:]");
	
	CodeCompletion::CodeCompletion(KileInfo *info) : m_ki(info), m_view(0L)
	{
		m_firstconfig = true;
		m_inprogress = false;
		m_undo = false;
		m_ref = false;
		m_kilecompletion = false;

		//reRef.setPattern("^\\\\(pageref|ref|xyz)\\{");
		m_completeTimer = new QTimer( this );
		connect(m_completeTimer, SIGNAL( timeout() ), this, SLOT( slotCompleteValueList() ) );
	}

	CodeCompletion::~CodeCompletion() {}

	bool CodeCompletion::isActive()
	{
		return m_isenabled;
	}

	bool CodeCompletion::inProgress()
	{
		return m_inprogress;
	}

	bool CodeCompletion::autoComplete()
	{
		return m_autocomplete || m_autocompletetext;
	}

	CodeCompletion::Type CodeCompletion::getType()
	{
		return m_type;
	}

	CodeCompletion::Type CodeCompletion::getType( const QString &text )
	{
		if ( text.find( reRef ) != -1 )
			return CodeCompletion::ctReference;
		else if ( text.find( reCite ) != -1 )
			return CodeCompletion::ctCitation;
		else
			return CodeCompletion::ctNone;
	}

	CodeCompletion::Mode CodeCompletion::getMode()
	{
		return m_mode;
	}

	//////////////////// configuration ////////////////////

	void CodeCompletion::readConfig(KConfig *config)
	{
		kdDebug() << "=== CodeCompletion::readConfig ===================" << endl;

		// save normal parameter
		//kdDebug() << "   read bool entries" << endl;
		m_isenabled = KileConfig::completeEnabled();
		m_setcursor = KileConfig::completeCursor();
		m_setbullets = KileConfig::completeBullets();
		m_closeenv = KileConfig::completeCloseEnv();
		m_autocomplete = KileConfig::completeAuto();
		m_autocompletetext = KileConfig::completeAutoText();
		m_latexthreshold = KileConfig::completeAutoThreshold();
		m_textthreshold = KileConfig::completeAutoTextThreshold();

		// we need to read some of Kate's config flags
		readKateConfigFlags(config);

		// reading the wordlists is only necessary at the first start
		// and when the list of files changes
		if ( m_firstconfig || KileConfig::completeChangedLists()  || KileConfig::completeChangedCommands() )
		{
			kdDebug() << "   set regexp for references..." << endl;
			setReferences();
			
			kdDebug() << "   read wordlists..." << endl;
			// wordlists for Tex/Latex mode
			QStringList files = KileConfig::completeTex();
			setWordlist( files, "tex", &m_texlist );
		
			// wordlist for dictionary mode
			files = KileConfig::completeDict();
			setWordlist( files, "dictionary", &m_dictlist );

			// wordlist for abbreviation mode
			files = KileConfig::completeAbbrev();
			setWordlist( files, "abbreviation", &m_abbrevlist );

			// remember changed lists
			m_firstconfig = false;
			KileConfig::setCompleteChangedLists(false);
			KileConfig::setCompleteChangedCommands(false);
		}
	}

	void CodeCompletion::readKateConfigFlags(KConfig *config)
	{
		config->setGroup("Kate Document Defaults");
		m_autobrackets = ( config->readNumEntry("Basic Config Flags",0) & cfAutoBrackets );
		m_autoindent   = ( config->readNumEntry("Indentation Mode",0) > 0 );
	}
	
	//////////////////// references and citations ////////////////////
	
	void CodeCompletion::setReferences()
	{
		// build list of references
		QString references = getCommandList(KileDocument::CmdAttrReference);
		references.replace("*","\\*");
		reRef.setPattern("^\\\\(" + references + ")\\{");
		reRefExt.setPattern("^\\\\(" + references + ")\\{[^\\{\\}\\\\]+,$");
		
		// build list of citations
		QString citations = getCommandList(KileDocument::CmdAttrCitations);
		citations.replace("*","\\*");
		reCite.setPattern("^\\\\(((c|C|noc)(ite|itep|itet|itealt|itealp|iteauthor|iteyear|iteyearpar|itetext))" + citations +  ")\\{");
		reCiteExt.setPattern("^\\\\(((c|C|noc)(ite|itep|itet|itealt|itealp|iteauthor|iteyear|iteyearpar|itetext))" + citations + ")\\{[^\\{\\}\\\\]+,$");
	}
		
	QString CodeCompletion::getCommandList(KileDocument::CmdAttribute attrtype)
	{
		QStringList cmdlist;
		QStringList::ConstIterator it;

		// get info about user defined references
		KileDocument::LatexCommands *cmd = m_ki->latexCommands();
		cmd->commandList(cmdlist,attrtype,false);
	
		// build list of references
		QString commands = QString::null;
		for ( it=cmdlist.begin(); it != cmdlist.end(); ++it ) 
		{
			if ( cmd->isStarredEnv(*it) )
				commands += '|' + (*it).mid(1) + '*';
			commands += '|' + (*it).mid(1);
		}
		return commands;
	}
	
	//////////////////// wordlists ////////////////////
	
	void CodeCompletion::setWordlist( const QStringList &files, const QString &dir,
	                                  QValueList<KTextEditor::CompletionEntry> *entrylist
	                                )
	{
		
		// read wordlists from files
		QStringList wordlist;
		for ( uint i = 0; i < files.count(); ++i )
		{
			// if checked, the wordlist has to be read
			if ( files[ i ].at( 0 ) == '1' )
			{
				readWordlist( wordlist, dir + '/' + files[ i ].right( files[ i ].length() - 2 ) + ".cwl" );
			}
		}

		// add user defined commands and environments
		if ( dir == "tex" )
		{
			addCommandsToTexlist(wordlist);
			setCompletionEntriesTexmode( entrylist, wordlist );
		}
		else
		{
			wordlist.sort();
			setCompletionEntries( entrylist, wordlist );
		}
	}

	void CodeCompletion::addCommandsToTexlist(QStringList &wordlist)
	{
		QStringList cmdlist;
		QStringList::ConstIterator it;
		KileDocument::LatexCmdAttributes attr;

		// get info about user defined commands and environments
		KileDocument::LatexCommands *cmd = m_ki->latexCommands();
		cmd->commandList(cmdlist,KileDocument::CmdAttrNone,true);
	
		// add entries to wordlist
		for ( it=cmdlist.begin(); it != cmdlist.end(); ++it ) 
		{
			if ( cmd->commandAttributes(*it,attr) ) 
			{
				QString command,eos;
				QStringList entrylist;
				if ( attr.type < KileDocument::CmdAttrLabel )          // environment
				{
					command = "\\begin{" + (*it);
					eos = "}";
				}
				else                                                   // command
				{
					command = (*it);
					// eos = QString::null;
				}
				
				// get all possibilities into a stringlist
				entrylist.append( command + eos );
				if ( ! attr.option.isEmpty() )
					entrylist.append( command + eos + "[option]" );
				if ( attr.starred )
				{
					entrylist.append( command + '*' + eos );
					if ( ! attr.option.isEmpty() )
						entrylist.append( command + '*' + eos + "[option]" );
				}

				// finally append entries to wordlist
				QStringList::ConstIterator itentry;
				for ( itentry=entrylist.begin(); itentry != entrylist.end(); ++itentry ) 
				{
					QString entry = (*itentry);
					if ( ! attr.parameter.isEmpty()  )
						entry += "{param}";
					if ( attr.type == KileDocument::CmdAttrList )
						entry += "\\item";
					wordlist.append( entry ); 
				}
			}
		}
	}

	//////////////////// completion box ////////////////////

	void CodeCompletion::completeWord(const QString &text, CodeCompletion::Mode mode)
	{
		kdDebug() << "==CodeCompletion::completeWord(" << text << ")=========" << endl;
		//kdDebug() << "\tm_view = " << m_view << endl;
		if ( !m_view) return;
		//kdDebug() << "ok" << endl;

		// remember all parameters (view, pattern, length of pattern, mode)
		m_text = text;
		m_textlen = text.length();
		m_mode = mode;

		// and the current cursor position
		m_view->cursorPositionReal( &m_ycursor, &m_xcursor );
		m_xstart = m_xcursor - m_textlen;

		// and the current document
		Kate::Document *doc = m_view->getDoc();

		// switch to cmLatex mode, if cmLabel is chosen without any entries
		if ( mode==cmLabel && m_labellist.count()==0 ) {
			QString s = doc->textLine(m_ycursor);
			int pos = s.findRev("\\",m_xcursor);
			if (pos < 0) {
				//kdDebug() << "\tfound no backslash! s=" << s << endl;
				return;
			}
			m_xstart = pos;
			m_text = doc->text(m_ycursor,m_xstart,m_ycursor,m_xcursor);
			m_textlen = m_text.length();
			m_mode = cmLatex;
		}

		// determine the current list
		QValueList<KTextEditor::CompletionEntry> list;
		switch ( m_mode )
		{
				case cmLatex:
        //kdDebug() << "mode = cmLatex" << endl;
        list = m_texlist;
        appendNewCommands(list);
        break;
				case cmEnvironment:
        //kdDebug() << "mode = cmEnvironment" << endl;
				list = m_texlist;				
				break;
				case cmDictionary:
				list = m_dictlist;
				break;
				case cmAbbreviation:
				list = m_abbrevlist;
				break;
				case cmLabel:
				list = m_labellist;
				break;
				case cmDocumentWord:
				getDocumentWords(text,list);
				break;
		}

		// is it necessary to show the complete dialog?
		QString entry, type;
		QString pattern = ( m_mode != cmEnvironment ) ? text : "\\begin{" + text;
		uint n = countEntries( pattern, &list, &entry, &type );
		
		//kdDebug() << "entries = " << n << endl;

		// nothing to do
		if ( n == 0 )
			return ;


		// Add a prefix ('\\begin{', length=7) in cmEnvironment mode,
		// because KateCompletion reads from the current line, This also
		// means that the original text has to be restored, if the user
		// aborts the completion dialog
		if ( m_mode == cmEnvironment )
		{
			doc->removeText( m_ycursor, m_xstart, m_ycursor, m_xcursor );
			doc->insertText( m_ycursor, m_xstart, "\\begin{" + m_text );

			// set the cursor to the new position
			m_textlen += 7;
			m_xcursor += 7;
			m_view->setCursorPositionReal( m_ycursor, m_xcursor );

			// set restore mode
			m_undo = true;
		}

		//  set restore mode
		if ( m_mode == cmAbbreviation ) m_undo = true;

		// show the completion dialog
		m_inprogress = true;

		KTextEditor::CodeCompletionInterface *iface;
		iface = dynamic_cast<KTextEditor::CodeCompletionInterface *>( m_view );
		iface->showCompletionBox( list, m_textlen );
	}

	void CodeCompletion::appendNewCommands(QValueList<KTextEditor::CompletionEntry> & list)
	{
		KTextEditor::CompletionEntry e;
		const QStringList *ncommands = m_ki->allNewCommands();
		QStringList::ConstIterator it;
		QStringList::ConstIterator itend(ncommands->end());
		for ( it = ncommands->begin(); it != itend; ++it )
		{
			e.text = *it;
			list.prepend(e);
		}
	}

	void CodeCompletion::completeFromList(const QStringList *list )
	{
		KTextEditor::CompletionEntry e;

		//kdDebug() << "completeFromList: " << list->count() << " items" << endl;
		QStringList sortedlist( *list );
		sortedlist.sort();

		m_labellist.clear();
		QStringList::ConstIterator it;
		QStringList::ConstIterator itend(sortedlist.end());
		for ( it = sortedlist.begin(); it != itend; ++it )
		{
			e.text = *it;
			m_labellist.append(  e );
		}

		completeWord("", cmLabel);
	}

	//////////////////// completion was done ////////////////////

	void CodeCompletion::CompletionDone(KTextEditor::CompletionEntry)
	{
		// kile completion: is there a new cursor position?
		if ( m_kilecompletion && m_setcursor && ( m_xoffset != 0 || m_yoffset != 0 ) && m_view )
		{
			int newx = ( m_xoffset != 0 ) ? m_xcursor + m_xoffset - m_textlen : m_xcursor;
			int newy = ( m_yoffset != 0 ) ? m_ycursor + m_yoffset : m_ycursor;

			m_view->setCursorPositionReal( newy, newx );
		}

		m_undo = false;
		m_inprogress = false;
		m_ref = false;
	}

	void CodeCompletion::CompletionAborted()
	{
		// aborted: undo if kile completion is active
		if ( m_inprogress && m_undo && m_view )
		{
			uint row, col;
			m_view->cursorPositionReal( &row, &col );

			Kate::Document *doc = m_view->getDoc();
			doc->removeText( m_ycursor, m_xstart, m_ycursor, col );
			doc->insertText( m_ycursor, m_xstart, m_text );

			m_view->setCursorPositionReal( m_ycursor, m_xstart + m_text.length() );
		}

		m_undo = false;
		m_inprogress = false;
		m_ref = false;
	}

	//////////////////// build the text for completion ////////////////////

	// parse an entry for kile completion modes:
	// - delete arguments/parameters
	// - set cursor position
	// - insert bullets

	QString CodeCompletion::filterCompletionText( const QString &text, const QString &type )
	{
		static QRegExp::QRegExp reEnv = QRegExp("^\\\\(begin|end)[^a-zA-Z]+");
		//kdDebug() << "   complete filter: " << text << " type " << type << endl;
		m_type = getType( text );    // remember current type
    
		if ( text!="\\begin{}" && reEnv.search(text)!=-1 )
			m_mode = cmEnvironment;
    
		// check the cursor position, because the user may have
		// typed some characters or the backspace key. This also
		// changes the length of the current pattern.
		uint row, col;
		m_view->cursorPositionReal( &row, &col );
		if ( m_xcursor != col )
		{
			m_textlen += ( col - m_xcursor );
			m_xcursor = col;
		}

		// initialize offset for the new cursorposition
		m_xoffset = m_yoffset = 0;

		// build the text
		QString s,prefix;
		Kate::Document *doc = m_view->getDoc();
		QString textline = doc->textLine(row);
		switch ( m_mode )
		{
				case cmLatex:
				s = buildLatexText( text, m_yoffset, m_xoffset );
				if ( m_autobrackets && textline.at(col)=='}' && m_text.find('{')>=0 )
				{
					doc->removeText(row,col,row,col+1);
				}
				break;
				case cmEnvironment:
				prefix = QString::null;
				if ( m_autoindent )
				{
					if ( col-m_textlen>0 ) 
					{
						prefix = textline.left(col-m_textlen);
						if ( prefix.right(7) == "\\begin{" )
							prefix.truncate(prefix.length()-7);
						else if ( prefix.right(5) == "\\end{" )
							prefix.truncate(prefix.length()-5);
					}
				}
				s = buildEnvironmentText( text, type, prefix, m_yoffset, m_xoffset );
				if ( m_autobrackets && textline.at(col)=='}' && (textline[m_xstart]!='\\' || m_text.find('{')>=0 ) )
				{
					doc->removeText(row,col,row,col+1);
				}
				if ( m_xstart>=7 && textline.mid(m_xstart-7,7) == "\\begin{" ) 
				{
					m_textlen += 7;
				} 
				else if ( m_xstart>=5 && textline.mid(m_xstart-5,5) == "\\end{" ) 
				{
					m_textlen += 5;
				} 
				break;
				case cmDictionary:
				s = text;
				break;
				case cmAbbreviation:
				s = buildAbbreviationText( text );
				break;
				case cmLabel:
				s = buildLabelText( text );
				break;
				case cmDocumentWord:
				s = text;
				break;
        default : s = text; break;
		}
		
		if ( s.length() > m_textlen )
			return s.right( s.length() - m_textlen );
		else
			return "";
	}

	//////////////////// text in cmLatex mode ////////////////////

	QString CodeCompletion::buildLatexText( const QString &text, uint &ypos, uint &xpos )
	{
		return parseText( stripParameter( text ), ypos, xpos, true );
	}

	////////////////////  text in cmEnvironment mode ////////////////////

	QString CodeCompletion::buildEnvironmentText( const QString &text, const QString &type,
	                                              const QString &prefix, uint &ypos, uint &xpos )
	{
		static QRegExp::QRegExp reEnv = QRegExp("^\\\\(begin|end)\\{([^\\}]*)\\}(.*)");

		if (reEnv.search(text) == -1) return text;

		QString parameter = stripParameter( reEnv.cap(3) );
		QString start = reEnv.cap(1);
		QString envname = reEnv.cap(2);
		QString whitespace = getWhiteSpace(prefix);
		QString envIndent = m_ki->editorExtension()->autoIndentEnvironment();

		QString s = "\\" + start + "{" + envname + "}" + parameter + "\n";

		s += whitespace;
		if ( start != "end" )
			s += envIndent;

		bool item = (type == "list" );
		if ( item )
			s += "\\item ";

		if ( m_setbullets && !parameter.isEmpty() )
			s += s_bullet;

		if ( m_closeenv && start != "end" )
			s += '\n' + whitespace + "\\end{" + envname + "}\n";

		// place cursor
		if ( m_setcursor )
		{
			if ( parameter.isEmpty() )
			{
				ypos = 1;
				xpos = whitespace.length() + envIndent.length() + (( item ) ? 6 : 0);
			}
			else
			{
				ypos = 0;
				if( parameter.left(2) == "[<" )
					xpos = 10 + envname.length();
				else
					xpos = 9 + envname.length();
			}
		}

		return s;
	}

	QString CodeCompletion::getWhiteSpace(const QString &s)
	{
		QString whitespace = s;
		for ( uint i=0; i<whitespace.length(); ++i )
		{
			if ( ! whitespace[i].isSpace() ) 
				whitespace[i] = ' ';
		}
		return whitespace;
	}

	//////////////////// text in  cmAbbreviation mode ////////////////////

	QString CodeCompletion::buildAbbreviationText( const QString &text )
	{
		QString s;

		int index = text.find( '=' );
		if ( index >= 0 )
		{
			// determine text to insert
			s = text.right( text.length() - index - 1 );

			// delete abbreviation
			Kate::Document *doc = m_view->getDoc();
			doc->removeText( m_ycursor, m_xstart, m_ycursor, m_xcursor );
			m_view->setCursorPositionReal( m_ycursor, m_xstart );
			m_xcursor = m_xstart;

			m_textlen = 0;
		}
		else
			s = "";

		return s;
	}

	//////////////////// text in cmLabel mode ////////////////////

	QString CodeCompletion::buildLabelText( const QString &text )
	{
		if ( text.at( 0 ) == ' ' )
		{
			// delete space
			Kate::Document * doc = m_view->getDoc();
			doc->removeText( m_ycursor, m_xstart, m_ycursor, m_xstart + 1 );
			m_view->setCursorPositionReal( m_ycursor, m_xstart );
			m_xcursor = m_xstart;

			m_textlen = 0;
			return text.right( text.length() - 1 );
		}
		else
			return text;
	}


	//////////////////// some functions ////////////////////

	QString CodeCompletion::parseText( const QString &text, uint &ypos, uint &xpos, bool checkgroup )
	{
		bool foundgroup = false;
		QString s = "";

		xpos = ypos = 0;
		for ( uint i = 0; i < text.length(); ++i )
		{
			switch ( text[ i ] )
			{
					case '<':
					case '{':
					case '(':
					case '[':		// insert character
					s += text[ i ];
					if ( xpos == 0 )
					{
						// remember position after first brace
						if(text[i] == '[' && (i+1) < text.length() &&  text[i+1] == '<'){
							xpos = i + 2;
							s += text[i+1];
							i++;
						}// special handling for '[<'
						else 
							xpos = i + 1;
						// insert bullet, if this is no cursorposition
						if ( ( ! m_setcursor ) && m_setbullets && !( text[i] == '[' && (i+1) < text.length() &&  text[i+1] == '<' ))
							s += s_bullet;
					}
					// insert bullets after following braces
					else if ( m_setbullets && !( text[i] == '[' && (i+1) < text.length() &&  text[i+1] == '<' ) )
						s += s_bullet;
					break;
					case '>':
					case '}':
					case ')':
					case ']':                    // insert character
					s += text[ i ];
					break;
					case ',':                    // insert character
					s += text[ i ];
					// insert bullet?
					if ( m_setbullets )
						s += s_bullet;
					break;
					case '.':      // if the last character is a point of a range operator,
					// it will be replaced by a space or a bullet surrounded by spaces
					if ( checkgroup && ( s.right( 1 ) == "." ) )
					{
						foundgroup = true;
						s.truncate( s.length() - 1 );
						if ( m_setbullets )
							s += ' ' + s_bullet + ' ';
						else
							s += ' ';
					}
					else
						s += text[ i ];
					break;
					default:                      // insert all other characters
					s += text[ i ];
					break;
			}
		}

		// some more work with groups and bullets
		if ( checkgroup && foundgroup && ( m_setbullets | m_setcursor ) )
		{
			int pos = 0;

			// search for braces, brackets and parens
			switch ( QChar( s[ 1 ] ) )
			{
					case 'l' :
					if ( s.left( 6 ) == "\\left " )
						pos = 5;
					break;
					case 'b' :
					if ( s.left( 6 ) == "\\bigl " )
						pos = 5;
					else if ( s.left( 7 ) == "\\biggl " )
						pos = 6;
					break;
					case 'B' :
					if ( s.left( 6 ) == "\\Bigl " )
						pos = 5;
					else if ( s.left( 7 ) == "\\Biggl " )
						pos = 6;
					break;
			}

			// update cursorposition and set bullet
			if ( pos > 0 )
			{
				if ( m_setcursor )
					xpos = pos;
				if ( m_setbullets )
				{
					if ( ! m_setcursor )
						s.insert( pos, s_bullet );
					s.append( s_bullet );
				}
			}
		}

		// Ergebnis
		return s;
	}

	// strip all names enclosed in braces
	// consider also beamer like stuff [<...>] and <...>

	QString CodeCompletion::stripParameter( const QString &text )
	{
		QString s = "";
		const QChar *ch = text.unicode();
		bool ignore = false;


		for ( uint i = 0; i < text.length(); ++i )
		{
			switch ( *ch )
			{
					case '[':
					case '{':
					case '(':
					case '<':
					s += *ch;
					ignore = true;
					break;
					case ']':
					case '}':
					case ')':
					case '>':
					s += *ch;
					ignore = false;
					break;
					case ',':
					s += *ch;
					break;
					default:
					if ( ! ignore )
						s += *ch;
					break;
			}
			++ch;
		}
		return s;
	}

	//////////////////// read wordlists  ////////////////////

	void CodeCompletion::readWordlist( QStringList &wordlist, const QString &filename )
	{
		QString file = KGlobal::dirs() ->findResource( "appdata", "complete/" + filename );
		if ( file.isEmpty() ) return;

		QFile f( file );
		if ( f.open( IO_ReadOnly ) )
		{     // file opened successfully
			QTextStream t( &f );         // use a text stream
			while ( ! t.eof() )
			{        // until end of file...
				QString s = t.readLine().stripWhiteSpace();       // line of text excluding '\n'
				if ( ! ( s.isEmpty() || s.at( 0 ) == '#' ) )
				{
					wordlist.append( s );
				}
			}
			f.close();
		}
	}

	void CodeCompletion::setCompletionEntries( QValueList<KTextEditor::CompletionEntry> *list,
	                                           const QStringList &wordlist )
	{
		// clear the list of completion entries
		list->clear();

		KTextEditor::CompletionEntry e;
		QStringList::ConstIterator it;
		
		// build new entries
		for ( it=wordlist.begin(); it != wordlist.end(); ++it ) 
		{
			// set CompletionEntry
			e.text = *it;
			e.type = "";
			
			// add new entry
			if ( list->findIndex(e) == -1 )
				list->append(e);
		}
	}

	void CodeCompletion::setCompletionEntriesTexmode( QValueList<KTextEditor::CompletionEntry> *list,
	        const QStringList &wordlist )
	{
		// clear the list of completion entries
		list->clear();

		// create a QMap for a user defined sort
		// order: \abc, \abc[], \abc{}, \abc*, \abc*[], \abc*{}, \abcd, \abcD
		QStringList keylist;
		QMap<QString,QString> map;
		
		for ( uint i=0; i< wordlist.count(); ++i )
		{
			QString s = wordlist[i];
			for ( uint j=0; j<s.length(); ++j ) 
			{
				QChar ch = s[j];
				if ( ch>='A' && ch<='Z' )
					s[j] = (int)ch + 32 ;
				else if ( ch>='a' && ch<='z' )
					s[j] = (int)ch - 32 ;
				else if ( ch == '}' )
					s[j] = 48;
				else if ( ch == '{' )
					s[j] = 49;
				else if ( ch == '[' )
					s[j] = 50;
				else if ( ch == '*' )
					s[j] = 51;
				else if ( ch == ']' )
					s[j] = 52;
			}
			// don't allow duplicate entries
			if ( ! map.contains(s) )
			{
				map[s] = wordlist[i];
				keylist.append(s);
			}
		}
		
		// sort mapped keys
		keylist.sort();
		
		// build new entries: get the sorted keys and insert
		// the real entries, which are saved in the map. 
		// if the last 5 chars of an environment are '\item', it is a 
		// list environment, where the '\item' tag is also inserted
		KTextEditor::CompletionEntry e;
		QStringList::ConstIterator it;
		
		for ( it=keylist.begin(); it != keylist.end(); ++it ) 
		{
			// get real entry
			QString s = map[*it];
			if ( s.left( 7 ) == "\\begin{" && s.right( 5 ) == "\\item" )
			{
				e.text = s.left( s.length() - 5 );     // list environment entry
				e.type = "list";
			}
			else
			{
				e.text = s;                        // normal entry
				e.type = "";
			}
			// add new entry (duplicates are impossible)
			list->append(e);
		}
	}

	//////////////////// determine number of entries ////////////////////

	// Count the number of entries. Stop, wenn there are 2 entries,
	// because special functions are only called, when there are 0
	// or 1 entries.

	uint CodeCompletion::countEntries( const QString &pattern,
	                                   QValueList<KTextEditor::CompletionEntry> *list,
	                                   QString *entry, QString *type )
	{
		QValueList<KTextEditor::CompletionEntry>::Iterator it;
		uint n = 0;

		for ( it = list->begin(); it != list->end() && n < 2; ++it )
		{
			if ( ( *it ).text.startsWith( pattern ) )
			{
				*entry = ( *it ).text;
				*type = ( *it ).type;
				++n;
			}
		}

		return n;
	}

	void CodeCompletion::editComplete(Kate::View *view, Mode mode)
	{
		m_view = view;

		if ( !m_view || !isActive() || inProgress() )
			return ;

		QString word;
		Type type;
		if ( getCompleteWord(( mode == cmLatex ) ? true : false, word, type ) )
		{
			if ( mode == cmLatex && word.at( 0 ) != '\\' )
			{
				mode = cmDictionary;
			}

			if ( type == ctNone )
				completeWord(word, mode);
			else
				editCompleteList(type);
		}
		//little hack to make multiple insertions like \cite{test1,test2} possible (only when
		//completion is invoke explicitly using ctrl+space.
		else if ( m_view->getDoc() )
		{
			QString currentline = m_view->getDoc()->textLine(m_view->cursorLine()).left(m_view->cursorColumnReal() + 1);
			if ( currentline.find(reCiteExt) != -1 )
				editCompleteList(ctCitation);
			else if ( currentline.find(reRefExt) != -1 )
				editCompleteList(ctReference);
		}
	}

	void CodeCompletion::editCompleteList(Type type )
	{
		//kdDebug() << "==editCompleteList=============" << endl;
		if ( type == ctReference )
			completeFromList(info()->allLabels());
		else if ( type == ctCitation )
			completeFromList(info()->allBibItems());
    else
      kdWarning() << "unsupported type in CodeCompletion::editCompleteList" << endl;
	}

	//////////////////// slots for code completion ////////////////////

	void CodeCompletion::slotCompletionDone(KTextEditor::CompletionEntry entry)
	{
		//kdDebug() << "==slotCompletionDone (" << m_kilecompletion << "," << m_inprogress << ")=============" << endl;
		CompletionDone(entry);

		// if kile completion was active, look if we need to show an additional list
		if ( m_kilecompletion )
		{
			m_kilecompletion = false;
			if ( getMode() == cmLatex )
			{
				m_type = getType(entry.text);
				if ( (m_type==CodeCompletion::ctReference && info()->allLabels()->count()>0)  ||
					  (m_type==CodeCompletion::ctCitation  && info()->allBibItems()->count()>0) )
				{
					m_ref = true;
		 			m_completeTimer->start(20,true);
				}
			}
		}
	}

	void CodeCompletion::slotCompleteValueList()
	{
		//kdDebug() << "==slotCompleteValueList (" << m_kilecompletion << "," << m_inprogress << ")=============" << endl;
		m_completeTimer->stop();
		editCompleteList(getType());
	}

	void CodeCompletion::slotCompletionAborted()
	{
		//kdDebug() << "==slotCompletionAborted (" << m_kilecompletion << "," << m_inprogress << ")=============" << endl;
		CompletionAborted();
	}

	void CodeCompletion::slotFilterCompletion( KTextEditor::CompletionEntry* c, QString *s )
	{
		//kdDebug() << "==slotFilterCompletion (" << m_kilecompletion << "," << m_inprogress << ")=============" << endl;
		if ( inProgress() )                 // dani 28.09.2004
		{
			//kdDebug() << "\tin progress: s=" << *s << endl;
			*s = filterCompletionText( c->text, c->type );
			//kdDebug() << "\tfilter --->" << *s << endl;
			m_inprogress = false;
			m_kilecompletion = true;
		}
	}

	void CodeCompletion::slotCharactersInserted(int, int, const QString& string )
	{
		// only work, if autocomplete mode of Kile is active
		if ( !isActive() || !autoComplete() )
			return ;

		//kdDebug() << "==slotCharactersInserted (" << m_kilecompletion << "," << m_inprogress << ", " << string << ")=============" << endl;
		//FIXME this is not very efficient
		m_view = info()->viewManager()->currentTextView();
		
		QString word;
		Type type;
		bool found = ( m_ref ) ? getReferenceWord(word) : getCompleteWord(true,word,type ); 
		if ( found ) 
		{ 
			int wordlen = word.length();
			//kdDebug() << "   auto completion: word=" << word << " mode=" << m_mode << " inprogress=" << inProgress() << endl;
			if ( inProgress() )               // continue a running mode?
			{
				//kdDebug() << "   auto completion: continue current mode" << endl;
				completeWord(word, m_mode);     
			}
			else if ( word.at( 0 )=='\\' && m_autocomplete && wordlen>=m_latexthreshold)
			{
				//kdDebug() << "   auto completion: latex mode" << endl;
				if ( string.at( 0 ).isLetter() )
				{
					completeWord(word, cmLatex);
				}
				else if ( string.at( 0 ) == '{' )
				{
					editCompleteList(type);
				}
			} 
			else if ( word.at(0).isLetter() && m_autocompletetext && wordlen>=m_textthreshold) 
			{
				//kdDebug() << "   auto completion: document mode" << endl;
				completeWord(word,cmDocumentWord);
			}
		}
	}

	//////////////////// testing characters (dani) ////////////////////

	static bool isBackslash ( QChar ch )
	{
		return ( ch == '\\' );
	}

	bool CodeCompletion::getCompleteWord(bool latexmode, QString &text, Type &type )
	{
		if ( !m_view ) return false;

		uint row, col;
		QChar ch;

		// get current position
		m_view->cursorPositionReal( &row, &col );

		// there must be et least one sign
		if ( col < 1 )
			return "";

		// get current text line
		QString textline = m_view->getDoc()->textLine( row );

		//
		int n = 0;                           // number of characters
		int index = col;                     // go back from here
		while ( --index >= 0 )
		{
			// get current character
			ch = textline.at( index );

			if ( ch.isLetter() || ch=='.' || ch == '_' || ch.isDigit() || ( latexmode && ( index + 1 == ( int ) col ) && ch == '{' ) )
				++n;                           // accept letters and '{' as first character in latexmode
			else
			{
				if ( latexmode && isBackslash( ch ) && oddBackslashes( textline, index ) )         // backslash?
					++n;
				break;                         // stop when a backslash was found
			}
		}

		// select pattern and set type of match
		text = textline.mid( col - n, n );
		type = getType( text );

		return !text.isEmpty();
	}

	bool CodeCompletion::getReferenceWord(QString &text)
	{
		if ( !m_view ) return false;

		uint row, col;
		QChar ch;

		// get current position
		m_view->cursorPositionReal( &row, &col );
		// there must be et least one sign
		if ( col < 1 )
			return false;

		// get current text line
		QString textline = m_view->getDoc()->textLine( row );

		// search the current reference string 
		int pos = textline.findRev(reNotRefChars,col-1);
		if ( pos < 0 )
			pos = 0;
			
		// select pattern
		text = textline.mid(pos+1,col-1-pos);
		return ( (uint)pos < col-1 );
	}
	
	void CodeCompletion::getDocumentWords(const QString &text,
	                                      QValueList<KTextEditor::CompletionEntry> &list)
	{
		//kdDebug() << "getDocumentWords: " << endl;
		list.clear();
		
		QRegExp reg("(\\\\?\\b" + QString(text[0]) + "[^\\W\\d_]+)\\b");          
		Kate::Document *doc = m_view->getDoc();
		
		QString s;
		KTextEditor::CompletionEntry e;
		QDict<bool> seen; 
		bool alreadyseen = true;
		               
		for (uint i=0; i<doc->numLines(); ++i) {
			s = doc->textLine(i);
			int pos = 0;
			while ( pos >= 0 ) {
				pos = reg.search(s,pos);
				if ( pos >= 0 ) {
					if ( reg.cap(1).at(0)!='\\' && text!=reg.cap(1) && !seen.find(reg.cap(1)) ) {
						e.text = reg.cap(1);                        // normal entry
						e.type = "";
						list.append( e );
						seen.insert(reg.cap(1),&alreadyseen);
					}
					pos += reg.matchedLength();
				}
			}
		}
	}

	//////////////////// counting backslashes (dani) ////////////////////

	bool CodeCompletion::oddBackslashes( const QString& text, int index )
	{
		uint n = 0;
		while ( index >= 0 && isBackslash( text.at( index ) ) )
		{
			++n;
			--index;
		}
		return ( n % 2 ) ? true : false;
	}
}

#include "codecompletion.moc"
