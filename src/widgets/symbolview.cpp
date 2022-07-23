/****************************************************************************************
    begin                : Fri Aug 1 2003
    copyright            : (C) 2003 by Jeroen Wijnhout (Jeroen.Wijnhout@kdemail.net)
                               2006 - 2009 by Thomas Braun
                               2012 - 2022 by Michel Ludwig (michel.ludwig@kdemail.net)
 ****************************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*
dani 2005-11-22
  - add some new symbols
  - rearranged source

tbraun 2006-07-01
   - added tooltips which show the keys, copied from kfileiconview
   - reorganized the hole thing, more flexible png loading, removing the old big code_array, more groups

tbraun 2007-06-04
    - Send a warning in the logwidget if needed packages are not included for the command
tbraun 2007-06-13
    - Added Most frequently used symbolview, including remembering icons upon restart, removing of least popular item and configurable max item count
*/


#include "symbolview.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPixmap>
#include <QPainter>
#include <QRegExp>
#include <QStringList>
#include <QTextDocument>

#include <KColorScheme>
#include <KConfig>
#include <KLocalizedString>

#include "kileconfig.h"
#include "kiledebug.h"
#include "kileinfo.h"
#include "../symbolviewclasses.h"
#include "utilities.h"

#define MFUS_GROUP "MostFrequentlyUsedSymbols"
#define MFUS_PREFIX "MFUS"


namespace KileWidget {

SymbolView::SymbolView(KileInfo *kileInfo, QWidget *parent, int type, const char *name)
    : QListWidget(parent), m_ki(kileInfo)
{
    setObjectName(name);
    setViewMode(IconMode);
    setGridSize(QSize(36, 36));
    setSpacing(5);
    setWordWrap(false);
    setResizeMode(Adjust);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setMovement(Static);
    setSortingEnabled(false);
    setFlow(LeftToRight);
    setDragDropMode(NoDragDrop);
    m_brush = KStatefulBrush(KColorScheme::View, KColorScheme::NormalText);
    initPage(type);
}

SymbolView::~SymbolView()
{
}

/* key format
from old symbols with package info
1%\textonequarter%%%{textcomp}%%/home/kdedev/.kde4/share/apps/kile/mathsymbols/misc-text/img072misc-text.png
from old symbols without package info
1%\oldstylenums{9}%%%%%/home/kdedev/.kde4/share/apps/kile/mathsymbols/misc-text/img070misc-text.png
new symbol
1%\neq%≠%[utf8x,,]{inputenc,ucs,}%[fleqn,]{amsmath,}%This command gives nice weather!%/home/kdedev/.kde4/share/apps/kile/mathsymbols/user/img002math.png
*/

void SymbolView::extract(const QString& key, int& refCnt)
{
    if (!key.isEmpty()) {
        refCnt = key.section('%', 0, 0).toInt();
    }
}

void SymbolView::extractPackageString(const QString&string, QList<Package> &packages)
{
    QRegExp rePkgs("(?:\\[(.*)\\])?\\{(.*)\\}");
    QStringList args,pkgs;
    Package pkg;

    if(string.isEmpty()) {
        return;
    }

    packages.clear();

    if(rePkgs.exactMatch(string)) {
        args = rePkgs.cap(1).split(',');
        pkgs = rePkgs.cap(2).split(',');
    }
    else {
        return;
    }

    for(int i = 0 ; i < pkgs.count() && i < args.count() ; i++) {
        const QString packageName = pkgs.at(i);
        if(packageName.isEmpty()) {
            continue;
        }
        pkg.name = packageName;
        pkg.arguments = args.at(i);
        packages.append(pkg);
    }

}

void SymbolView::extract(const QString& key, Command &cmd)
{
    if (key.isEmpty()) {
        return;
    }
    QStringList contents = key.split('%');
    QString packages;

    cmd.referenceCount = contents.at(0).toInt();
    cmd.latexCommand = contents.at(1);
    cmd.unicodeCommand = contents.at(2);

    extractPackageString(contents.at(3), cmd.unicodePackages);
    extractPackageString(contents.at(4), cmd.packages);
    cmd.comment = contents.at(5);
    cmd.path = contents.at(6);
}

void SymbolView::initPage(int page)
{
    switch(page) {
    case MFUS:
        fillWidget(MFUS_PREFIX);
        break;

    case Relation:
        fillWidget("relation");
        break;

    case Operator:
        fillWidget("operators");
        break;

    case Arrow:
        fillWidget("arrows");
        break;

    case MiscMath:
        fillWidget("misc-math");
        break;

    case MiscText:
        fillWidget("misc-text");
        break;

    case Delimiters:
        fillWidget("delimiters");
        break;

    case Greek:
        fillWidget("greek");
        break;

    case Special:
        fillWidget("special");
        break;

    case Cyrillic:
        fillWidget("cyrillic");
        break;

    case User:
        fillWidget("user");
        break;

    default:
        qWarning() << "wrong argument in initPage()";
        break;
    }
}

QString SymbolView::getToolTip(const QString &key)
{
    Command cmd;
    extract(key, cmd);

    QString label = "<p style='white-space:pre'>";
    label += "<b>" + i18n("Command: %1", cmd.latexCommand.toHtmlEscaped()) + "</b>";
    if(!cmd.unicodeCommand.isEmpty()) {
        label += i18n("<br/>Unicode: %1", cmd.unicodeCommand.toHtmlEscaped());
    }

    if(cmd.packages.count() > 0) {
        QString packageString;

        if(cmd.packages.count() == 1) {
            Package pkg = cmd.packages.at(0);
            if(!pkg.arguments.isEmpty()) {
                packageString += '[' + pkg.arguments + ']' + pkg.name;
            }
            else {
                packageString += pkg.name;
            }
        }
        else {
            packageString = "<ul>";
            for (int i = 0; i < cmd.packages.count() ; ++i) {
                Package pkg = cmd.packages.at(i);
                if(!pkg.arguments.isEmpty()) {
                    packageString += "<li>[" + pkg.arguments + ']' + pkg.name + "</li>";
                }
                else {
                    packageString += "<li>" + pkg.name + "</li>";
                }
            }
            packageString += "</ul>";
        }
        label += "<br/>" + i18np("Required Package: %2", "Required Packages: %2", cmd.packages.count(), packageString);
    }

    if(!cmd.comment.isEmpty()) {
        label += "<br/><i>" + i18n("Comment: %1", cmd.comment.toHtmlEscaped())  + "</i>";
    }

    label += "</p>";
    return label;
}

void SymbolView::mousePressEvent(QMouseEvent *event)
{
    Command cmd;
    QString code_symbol;
    QList<Package> packages;
    QListWidgetItem *item = Q_NULLPTR;
    bool math = false, bracket = false;

    if(event->button() == Qt::LeftButton && (item = itemAt(event->pos()))) {
        bracket = event->modifiers() & Qt::ControlModifier;
        math = event->modifiers() & Qt::ShiftModifier;

        extract(item->data(Qt::UserRole).toString(), cmd);
        if(KileConfig::symbolViewUTF8()) {
            code_symbol = cmd.unicodeCommand;
            if(code_symbol.isEmpty()) {
                code_symbol = cmd.latexCommand;
            }
            packages = cmd.unicodePackages;
        }
        else {
            code_symbol = cmd.latexCommand;
            packages = cmd.packages;
        }

        if(math != bracket) {
            if(math) {
                code_symbol = '$' + code_symbol + '$';
            }
            else if(bracket) {
                code_symbol = '{' + code_symbol + '}';
            }
        }
        emit(insertText(code_symbol, packages));
        emit(addToList(item));
        m_ki->focusEditor();
    }

    KILE_DEBUG_MAIN << "math is " << math << ", bracket is " << bracket << " and item->data(Qt::UserRole).toString() is " << (item ? item->data(Qt::UserRole).toString() : "");
}

QString convertLatin1StringtoUTF8(const QString &string)
{
    if(string.isEmpty()) {
        return QString();
    }

    QVector<uint> stringAsIntVector;
    QStringList stringList = string.split(',', Qt::SkipEmptyParts);

    QStringList::const_iterator it;
    QString str;
    bool ok;
    int stringAsInt;
    for(it = stringList.constBegin(); it != stringList.constEnd(); it++) {
        str = *it;
        str.remove("U+");
        stringAsInt = str.toInt(&ok);
        if(!ok) {
            return QString();
        }
        stringAsIntVector.append(stringAsInt);
    }

    return QString::fromUcs4(stringAsIntVector.data(),stringAsIntVector.count());
}

void SymbolView::fillWidget(const QString& prefix)
{
    KILE_DEBUG_MAIN << "===SymbolView::fillWidget(const QString& " << prefix <<  " )===";
    QImage image;
    QListWidgetItem* item;
    QStringList refCnts, paths, unicodeValues;
    QString key;

    // find paths
    if (prefix == MFUS_PREFIX) { // case: most frequently used symbols
        KConfigGroup config = KSharedConfig::openConfig()->group(MFUS_GROUP);
        QString configPaths = config.readEntry("paths");
        QString configrefCnts = config.readEntry("counts");
        paths = configPaths.split(',', Qt::SkipEmptyParts);
        refCnts = configrefCnts.split(',', Qt::SkipEmptyParts);
        KILE_DEBUG_MAIN << "Read " << paths.count() << " paths and " << refCnts.count() << " refCnts";
        if(paths.count() != refCnts.count()) {
            KILE_DEBUG_MAIN << "error in saved LRU list";
            paths.clear();
            refCnts.clear();
        }
    }
    else { // case: any other group of math symbols
        const QStringList dirs = KileUtilities::locateAll(QStandardPaths::AppDataLocation,
                                                          QLatin1String("mathsymbols/") + prefix,
                                                          QStandardPaths::LocateDirectory);
        for(const QString &dir : dirs) {
            const QStringList fileNames = QDir(dir).entryList(QStringList() << QStringLiteral("*.png"));
            for(const QString &file : fileNames) {
                const QString path = dir + '/' + file;
                if (!paths.contains(path)) {
                    paths.append(path);
                }
            }
        }
        paths.sort();
        for (int i = 0; i < paths.count(); i++) {
            refCnts.append("1");
        }
    }

    // render symbols
    for (int i = 0; i < paths.count(); i++) {
        if (image.load(paths[i])) {
            item = new QListWidgetItem(this);

            key = refCnts[i] + '%' + image.text("Command");
            key += '%' + convertLatin1StringtoUTF8(image.text("CommandUnicode"));
            key += '%' + image.text("UnicodePackages");
            key += '%' + image.text("Packages");
            key += '%' + convertLatin1StringtoUTF8(image.text("Comment"));
            key += '%' + paths[i];

            item->setData(Qt::UserRole, key);
            item->setToolTip(getToolTip(key));

            if (prefix != QLatin1String("user")) {
                if (image.format() != QImage::Format_ARGB32_Premultiplied && image.format() != QImage::Format_ARGB32) {
                    image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
                }

                QPainter p;
                p.begin(&image);
                p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
                p.fillRect(image.rect(), m_brush.brush(QPalette::Active));
                p.end();
            }
            item->setIcon(QPixmap::fromImage(image));
        }
        else {
            KILE_DEBUG_MAIN << "Loading file " << paths[i] << " failed";
        }
    }
}

void SymbolView::writeConfig()
{
    QListWidgetItem *item;
    QStringList paths;
    QList<int> refCnts;
    Command cmd;

    KConfigGroup grp = KSharedConfig::openConfig()->group(MFUS_GROUP);

    if (KileConfig::clearMFUS()) {
        grp.deleteEntry("paths");
        grp.deleteEntry("counts");
    }
    else {
        for(int i = 0; i < count(); ++i) {
            item = this->item(i);
            extract(item->data(Qt::UserRole).toString(),cmd);
            refCnts.append(cmd.referenceCount);
            paths.append(cmd.path);
            KILE_DEBUG_MAIN << "path=" << paths.last() << ", count is " << refCnts.last();
        }
        grp.writeEntry("paths", paths);
        grp.writeEntry("counts", refCnts);
    }
}

void SymbolView::slotAddToList(const QListWidgetItem *item)
{
    if(!item || item->icon().isNull()) {
        return;
    }

    QListWidgetItem *tmpItem = Q_NULLPTR;
    bool found = false;
    const QRegExp reCnt("^\\d+");

    KILE_DEBUG_MAIN << "===void SymbolView::slotAddToList(const QIconViewItem *" << item << " )===";

    for(int i = 0; i < count(); ++i) {
        tmpItem = this->item(i);
        if (item->data(Qt::UserRole).toString().section('%', 1) == tmpItem->data(Qt::UserRole).toString().section('%', 1)) {
            found = true;
            break;
        }
    }

    if(!found
            && static_cast<unsigned int>(this->count() + 1) > KileConfig::numSymbolsMFUS()) {   // we check before adding the symbol
        int refCnt, minRefCnt = 10000;
        QListWidgetItem *unpopularItem = Q_NULLPTR;

        KILE_DEBUG_MAIN << "Removing most unpopular item";

        for (int i = 0; i < count(); ++i) {
            tmpItem = this->item(i);
            extract(tmpItem->data(Qt::UserRole).toString(), refCnt);

            if (refCnt < minRefCnt) {
                refCnt = minRefCnt;
                unpopularItem = tmpItem;
            }
        }
        KILE_DEBUG_MAIN << " minRefCnt is " << minRefCnt;
        delete unpopularItem;
    }

    if(found) {
        KILE_DEBUG_MAIN << "item is already in the iconview";

        int refCnt;
        extract(tmpItem->data(Qt::UserRole).toString(), refCnt);

        QString key = tmpItem->data(Qt::UserRole).toString();
        key.replace(reCnt, QString::number(refCnt + 1));
        tmpItem->setData(Qt::UserRole, key);
        tmpItem->setToolTip(getToolTip(key));
    }
    else {
        tmpItem = new QListWidgetItem(this);
        tmpItem->setIcon(item->icon());
        QString key = item->data(Qt::UserRole).toString();
        tmpItem->setData(Qt::UserRole, key);
        tmpItem->setToolTip(getToolTip(key));
    }
}

}

