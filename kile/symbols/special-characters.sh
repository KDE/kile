#!/bin/sh
#
# Author: Thomas Braun
# email: braun@physik.fu-berlin.de
# date: Mon Dec 5 2005
# last edit: Sat Feb 18 2006
# License: GPL

suffix=special # this has to be changed if used for other stuff
TeXfile=$suffix-characters.tex
DVIfile=$suffix-characters.dvi

latex $TeXfile &&\
dvipng  -bg Transparent -x 518 -O -1.2in,-1.2in -T bbox -D 300 -o img%d$suffix.png $DVIfile
./create-key-codes.pl $TeXfile keys-stub-$suffix.h $suffix
ls img*$suffix.png | tr "\n" " " > Makefile-$suffix-stub.am
