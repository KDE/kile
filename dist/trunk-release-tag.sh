#!/bin/bash

if [ -z "$1" -o -z "$2" ]; then
    echo "Usage: $0 <short version> <full version>"
    echo "Example: $0 2.1b5 \"2.1 beta 5\""
    exit 1
fi

./createPackage.sh --svnroot "https://svn.kde.org/home/kde" -ab trunk/extragear/office -a kile \
--package --logfile $0.log --nodoc --noi18n -av $1 -tm "Tagging Kile $2."
