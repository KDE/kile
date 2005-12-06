#!/bin/sh
#
# Author: Thomas Braun
# email: braun@physik.fu-berlin.de
# date: Mon Dec 5 2005
# License: GPL

suffix=special # this has to be changed if used for other stuff
TeXfile=$suffix-characters.tex
DVIfile=$suffix-characters.dvi

latex $TeXfile &&\
dvipng  --t1lib* -x 518 -bg Transparent -T tight -D 300 -o img%d$suffix-tmp.png $DVIfile
for i in `ls img*-tmp.png`
do
newname=`echo "$i" | sed -e 's/-tmp//'`
convert -fill white -opaque red "$i" "$newname" && rm "$i"
done
./create-key-codes.pl $TeXfile keys-stub-$suffix.h $suffix
ls img*$suffix.png | tr "\n" " " > Makefile-$suffix-stub.am
