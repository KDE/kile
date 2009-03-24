#!/bin/bash
# some changes for KDE4

revision=`LANG=C svn info | grep "Last Changed Rev:" | tr -d " " | cut -f 2 -d":"`
logfile=createPackage-`date +%S-%M-%H--%d.%m.%y`.log

./createPackage.sh -ab trunk/extragear/office -a kile --i18n-base trunk/l10n-kde4 \
--i18n-sub extragear-office --notoplevel --package --logfile $logfile -av trunk-r$revision
