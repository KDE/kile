#!/bin/bash
# provide the path to the gesym-ng binary as first argument

GESYMBNG=$1
SYMBOLS="arrows cyrillic delimiters greek misc-math misc-text operators relation special"

echo "Deleting old files..."

for i in $SYMBOLS; do

  cd $i
  rm -f *.png
  cd ..

done

for i in $SYMBOLS; do

  echo "Generating image files in $i..."
  mkdir -p generate
  cd generate
  $GESYMBNG "../$i.xml" &> /dev/null
  mv *.png "../$i"
  cd ..
  rm -rf generate

done