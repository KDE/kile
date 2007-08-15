#!/bin/bash
# internal script for generating all symbols on one shot
# License: GPLv2 or later
# Author: Thomas Braun
#
names="relation arrows delimiters greek misc-math misc-text operators special cyrillic"

for i in $names
do
	rm -f ./$i/*.png
	./gesymb $i $i.tex && mv -f img*$i*png $i # && make install
done
