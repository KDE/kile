/**
 * texfilter.cpp
 *
 * Copyright (C)  2004  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 */
#include "texfilter.h"
#if KDE_VERSION >= KDE_MAKE_VERSION(3,2,90)
#include <kdebug.h>
#include <qstring.h>

using namespace KSpell2;

Word TeXFilter::nextWord() const
{
	Word word = Filter::nextWord();

	kdDebug()<<"Char = (0) '" << m_buffer[ word.start  ]    << "'" << endl;
	kdDebug()<<"Char = (-1) '"<< m_buffer[ word.start - 1 ] << "'" << endl;
	kdDebug()<<"Char = (-2) '"<< m_buffer[ word.start - 2 ] << "'" << endl;

	while ( !word.end && m_buffer[ word.start - 1 ] == '\\' )
	{
		word = Filter::nextWord();
	}

	return word;
}

Word TeXFilter::previousWord() const
{
	Word word = Filter::previousWord();

	while ( !word.end && m_buffer[ word.start - 1] == '\\' )
	{
		word = Filter::nextWord();
	}

	return word;
}

Word TeXFilter::wordAtPosition( unsigned int pos ) const
{
	Word word = Filter::wordAtPosition( pos );

	if ( !word.end && m_buffer[ word.start - 1 ] == '\\'   )
	{
		return Filter::end();
	}

	return word;
}

#endif
