/**
 * texfilter.h
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
#ifndef TEXFILTER_H
#define TEXFILTER_H

#include <kdeversion.h>

#if KDE_VERSION >= KDE_MAKE_VERSION(3,2,90)

#include <kspell2/filter.h>

class TeXFilter : public KSpell2::Filter
{
public:
	virtual KSpell2::Word nextWord() const;
	virtual KSpell2::Word previousWord() const;
	virtual KSpell2::Word wordAtPosition( unsigned int pos ) const;
};

#endif

#endif
