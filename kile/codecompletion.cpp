/***************************************************************************
date                 : Oct 05 2004
version              : 0.10.5
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

#include <qregexp.h>
#include <qfile.h>
#include <qtimer.h>

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

namespace KileDocument
{

	static QRegExp::QRegExp reRef("^\\\\(pageref|ref)\\{");
	static QRegExp::QRegExp reCite("^\\\\(c|C|noc)(ite|itep|itet|itealt|itealp|iteauthor|iteyear|iteyearpar|itetext)\\{");
	static QRegExp::QRegExp reRefExt("^\\\\(pageref|ref)\\{[^\\{\\}\\\\]+,$");
	static QRegExp::QRegExp reCiteExt("^\\\\(c|C|noc)(ite|itep|itet|itealt|itealp|iteauthor|iteyear|iteyearpar|itetext)\\{[^\\{\\}\\\\]+,$");
	
	CodeCompletion::CodeCompletion(KileInfo *info) : m_ki(info), m_view(0L)
	{
		m_firstconfig = true;
		m_inprogress = false;
		m_undo = false;

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
		return m_autocomplete;
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

	void CodeCompletion::readConfig(void)
	{
		kdDebug() << "=== CodeCompletion::readConfig ===================" << endl;

		// save normal parameter
		kdDebug() << "   read bool entries" << endl;
		m_isenabled = KileConfig::completeEnabled();
		m_setcursor = KileConfig::completeCursor();
		m_setbullets = KileConfig::completeBullets();
		m_closeenv = KileConfig::completeCloseEnv();
		m_autocomplete = KileConfig::completeAuto();

		// reading the wordlists is only necessary at the first start
		// and when the list of files changes
		if ( m_firstconfig || KileConfig::completeChangedLists() )
		{
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
		}

	}

	void CodeCompletion::setWordlist( const QStringList &files, const QString &dir,
	                                  QValueList<KTextEditor::CompletionEntry> *entrylist
	                                )
	{
		QStringList wordlist;
		for ( uint i = 0; i < files.count(); ++i )
		{
			// if checked, the wordlist has to be read
			if ( files[ i ].at( 0 ) == '1' )
			{
				readWordlist( wordlist, dir + "/" + files[ i ].right( files[ i ].length() - 2 ) + ".cwl" );
			}
		}

		// sort the wordlist
		// wordlist.sort();

		// build all completion entries
		setCompletionEntries( entrylist, wordlist );
	}


	//////////////////// completion box ////////////////////

	void CodeCompletion::completeWord(const QString &text, CodeCompletion::Mode mode)
	{
		kdDebug() << "==CodeCompletion::completeWord(" << text << ")=========" << endl;
		kdDebug() << "\tm_view = " << m_view << endl;
		if ( !m_view) return;
		kdDebug() << "ok" << endl;

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
				kdDebug() << "\tfound no backslash! s=" << s << endl;
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
        kdDebug() << "mode = cmLatex" << endl;
        list = m_texlist;
        appendNewCommands(list);
        break;
				case cmEnvironment:
        kdDebug() << "mode = cmEnvironment" << endl;
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
		}

		// is it necessary to show the complete dialog?
		QString entry, type;
		QString pattern = ( m_mode != cmEnvironment ) ? text : "\\begin{" + text;
		uint n = countEntries( pattern, &list, &entry, &type );
		
		kdDebug() << "entries = " << n << endl;

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

		kdDebug() << "completeFromList: " << list->count() << " items" << endl;
		m_labellist.clear();
		QStringList::ConstIterator it;
		QStringList::ConstIterator itend(list->end());
		for ( it = list->begin(); it != itend; ++it )
		{
			e.text = *it;
			m_labellist.append(  e );
		}

		completeWord("", cmLabel);
	}

	//////////////////// completion was done ////////////////////

	void CodeCompletion::CompletionDone(KTextEditor::CompletionEntry)
	{
		// is there a new cursor position?
		if ( m_setcursor && ( m_xoffset != 0 || m_yoffset != 0 ) && m_view )
		{
			int newx = m_xcursor, newy = m_ycursor;
			
			if ( m_yoffset == 0 )
				newx = m_xcursor + m_xoffset - m_textlen;
			else
				newy = m_ycursor + m_yoffset;

			m_view->setCursorPositionReal( newy, newx );
		}

		m_undo = false;
		m_inprogress = false;
	}

	void CodeCompletion::CompletionAborted()
	{
		if ( m_undo && m_view )
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
	}

	//////////////////// build the text for completion ////////////////////

	// parse an entry:
	// - delete arguments/parameters
	// - set cursor position
	// - insert bullets

	QString CodeCompletion::filterCompletionText( const QString &text, const QString &type )
	{
    static QRegExp::QRegExp reEnv = QRegExp("^\\\\(begin|end)[^a-zA-Z]+");
		kdDebug() << "   complete filter: " << text << " type " << type << endl;
		m_type = getType( text );    // remember current type
    
    if ( reEnv.search(text) != -1 ) m_mode = cmEnvironment;
    
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
		QString s;
		switch ( m_mode )
		{
				case cmLatex:
  			s = buildLatexText( text, m_yoffset, m_xoffset );
				break;
				case cmEnvironment:
				s = buildEnvironmentText( text, type, m_yoffset, m_xoffset );
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
	        uint &ypos, uint &xpos )
	{
    static QRegExp::QRegExp reEnv = QRegExp("^\\\\(begin|end)\\{([^\\}]*)\\}(.*)");
    
    if (reEnv.search(text) == -1) return text;
    
    QString parameter = stripParameter( reEnv.cap(3) );
    QString start = reEnv.cap(1);
    QString envname = reEnv.cap(2);
    
    QString s = "\\" + start + "{" + envname + "}" + parameter + "\n";
    
    bool item = (type == "list" );
    if ( item )
			s += "\\item ";
      
		if ( m_setbullets && !parameter.isEmpty() )
			s += s_bullet;
      
    if ( m_closeenv && start != "end" )
			s += "\n\\end{" + envname + "}\n";
    
    // place cursor
		if ( m_setcursor )
		{
			if ( parameter.isEmpty() )
			{
				ypos = 1;
				xpos = ( item ) ? 6 : 0;
			}
			else
			{
				ypos = 0;
				xpos = 9 + envname.length();
			}
		}
    
    return s;
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
					case '{':
					case '(':
					case '[':                    // insert character
					s += text[ i ];
					if ( xpos == 0 )
					{
						// remember position after first brace
						xpos = i + 1;
						// insert bullet, if this is no cursorposition
						if ( ( ! m_setcursor ) && m_setbullets )
							s += s_bullet;
					}
					// insert bullets after following braces
					else if ( m_setbullets )
						s += s_bullet;
					break;
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
							s += " " + s_bullet + " ";
						else
							s += " ";
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

	// astrip all names enclosed in braces

	QString CodeCompletion::stripParameter( const QString &text )
	{
		QString s = "";
		const QChar *ch = text.unicode();
		bool ignore = false;

		for ( uint i = 0; i < text.length(); ++i )
		{
			switch ( *ch )
			{
					case '{':
					case '(':
					case '[':
					s += *ch;
					ignore = true;
					break;
					case '}':
					case ')':
					case ']':
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
		// clear the whole list
		list->clear();

		KTextEditor::CompletionEntry e;

		// build new entries: if the last 5 chars of an environment are '\item',
		// it is a list environment, where the '\item' tag is also inserted
		for ( uint i = 0; i < wordlist.count(); ++i )
		{
			QString s = wordlist[ i ];
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
			// add new entry
			if ( list->findIndex(e) == -1 )
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
		kdDebug() << "==editCompleteList=============" << endl;
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
		CompletionDone(entry);

		if ( getMode() == cmLatex ) m_completeTimer->start( 0, false );
		
		if ( KileConfig::completeAuto() && ( (entry.text.find(reRef) != -1) || (entry.text.find(reCite) != -1) ) )
				completeWord("", cmLabel);
		else
			m_view = 0L;
	}

	void CodeCompletion::slotCompleteValueList()
	{
		m_completeTimer->stop();
		editCompleteList(getType());
	}

	void CodeCompletion::slotCompletionAborted()
	{
		CompletionAborted();
	}

	void CodeCompletion::slotFilterCompletion( KTextEditor::CompletionEntry* c, QString *s )
	{
		if ( inProgress() ) {                // dani 28.09.2004
			kdDebug() << "==CodeCompletion::slotFilterCompletion( KTextEditor::CompletionEntry* c, QString *s )==" << endl;
			kdDebug() << "\ts = " << *s << " type = " << c->type << " text = " << c->text << " prefix = " << c->prefix << " postfix = " << c->postfix << " comment = " << c->comment << endl;
			*s = filterCompletionText( c->text, c->type );
		}
	}

	void CodeCompletion::slotCharactersInserted(int, int, const QString& string )
	{
		if ( !isActive() || !autoComplete() )
			return ;

		//FIXME this is not very efficient
		m_view = info()->viewManager()->currentView();

		QString word;
		Type type;
		if ( getCompleteWord(true, word, type ) && word.at( 0 ) == '\\' )
		{
			kdDebug() << "   auto completion: word=" << word << endl;
			if ( string.at( 0 ).isLetter() )
			{
				completeWord(word, cmLatex);
			}
			else if ( string.at( 0 ) == '{' )
			{
				editCompleteList(type);
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

			if ( ch.isLetter() || ( latexmode && ( index + 1 == ( int ) col ) && ch == '{' ) )
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
