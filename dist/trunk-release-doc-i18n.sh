#!/bin/bash
# some changes for KDE4

date=`date +%S-%M-%H--%d.%m.%y`
logfile=createPackage-$date.log

if [ -e ".svn" ]; then
	svn up # get newest revision info
	version=r`LANG=C svn info | grep "Last Changed Rev:" | tr -d " " | cut -f 2 -d":"`
else
	version=$date
fi

./createPackage.sh -ab trunk/extragear/office -a kile --i18n-base trunk/l10n-kde4 \
--i18n-sub extragear-office --notoplevel --package --logfile $logfile -av trunk-$version
