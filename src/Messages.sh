#! /usr/bin/env bash

$EXTRACTRC `find . -name \*.rc -o -name \*.ui -o -name \*.kcfg \
            | grep -v ./data/kilestdtools.rc                   \
            | grep -v ./data/biblatexentries.rc                \
            | grep -v ./data/bibtexentries.rc                  \
            | grep -v ./data/kilestdtools-win.rc               ` >> rc.cpp

LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`

if test -n "$LIST"; then
   $XGETTEXT $LIST -o $podir/kile.pot;
fi

rm -f rc.cpp
