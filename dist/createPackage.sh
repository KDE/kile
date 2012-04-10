#!/bin/bash
#
# Extract an application from KDE svn, including documentation and translations.
#
# Authors: Michael Buesch <mbuesch@freenet.de>
#          Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>
#          Sebastian Trueg
#          Aurelien Gateau
#          Klas Kalass
#          Michel Ludwig <michel.ludwig@kdemail.net>
# License: GPL (http://www.gnu.org/)
#
#
# Directory lay-out of packages
# Application:
# + name-version
#        + admin
#        + src
#        + doc
#        + translations
#            + nl
#                + messages
#                + doc
#            + de (etc.)
#
# Translations:
#        + nl
#            + messages
#            + doc
#        + de (etc.)

COPYRIGHT="2009-2012 Michel Ludwig <michel.ludwig@kdemail.net>
          2005 Michael Buesch <mbuesch@freenet.de>
          2004-2005 Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>
          2003-2004 Sebastian Trueg
          2002 Aurelien Gateau
          2002-2004 Klas Kalass"
LICENSE="GNU General Public License v2 or later"

SCRIPTNAME="createPackage.sh"
SCRIPTVERSION='$Revision$'
SCRIPTVERSION="`echo $SCRIPTVERSION | cut -d ' ' -f2`"


function showVersion
{
    echo "$SCRIPTNAME revision $SCRIPTVERSION"
}

function showHelp
{
    showVersion
    echo -e "
Extract an application from KDE svn, including documentation and translations.

Copyright $COPYRIGHT
License: $LICENSE

usage:
    $0 -ab <PATH> -a <NAME> [OPTIONS]

These parameters are mandatory:

    -ab|--app-base <PATH>       Relative (to svnroot) path to the KDE or extragear module.
                                Example:
                                    trunk/extragear/utils
                                If you want to checkout from a branch, you might use:
                                    branches/foobar/1.2

    -a|--app <NAME>             Name of the application to checkout.
                                This is the subdir you want to checkout in --app-base.

The following options are available:

    -d|--svnroot <URL>          Base URL of the svn repository.
                                svn://anonsvn.kde.org/home/kde is the default.

    -ib|--i18n-base <PATH>      Path to the i18n translations base.
                                Default:
                                    trunk/l10n

    -is|--i18n-sub <PATH>       Path to the i18n files under the
                                \$i18n-base/\$LANGUAGE/messages
                                directories. Example:
                                    extragear-office

    -ids|--i18n-docsub <PATH>   Path to the i18n docbook files under the
                                \$i18n-base/\$LANGUAGE/docs
                                directories. Example:
                                    kdeextragear-2
                                Default:
                                    path of --i18n-sub

    --admin <PATH>              Path to the /admin/ directory.
                                Default:
                                    trunk/KDE/kde-common/admin

    -b|--builddir <DIRNAME>     Name of the temporary working directory where
                                all files will be put. Default is \"./build\"

    -av|--appversion <VERSION>  Version of the application (only for correct
                                naming of the tar archives).

    --nodoc                     Do not try to get any documentation.

    --noi18n                    Do not search for any translations.

    --split                     Make separate packages for the application and translations.

    --package [TYPE]            Create tarballs (.tar.gz and/or .tar.bz2).
                                You can specify an optional package type: \"gzip\" or \"bzip2\"
                                If no type is given, both types will be generated.

    --packagedir <TARBALLDIR>   Place a copy of the packages into TARBALLDIR.

    --postprocess <SCRIPT>      Runs SCRIPT on the package, after all data is gathered, but before
                                the tarball is generated. This allows packagers to give the
                                \"final touch\" to the package.

    --pofiles <POFILES>         Space seperated list of pofiles this application uses without the extension.
                                Defaults to the name of the application.

    -l|--languages <LANGUAGES>  Space seperated list of languages to search for
                                translations. If no languages are specified, all
                                available languages will be included.

    -el|--exclude-languages <LANGUAGES> Space seperated list of languages not to include
                                        in the package.

    --logfile <FILE>            Write all logging information to FILE.
                                If FILE is \"no\", no logfile will be created.

    -v|--version                Version of this script
    -h|--help                   This Help"
}

#
# Some simple helper functions.
#

function printlog
{
    if [ -n "$LOGFILE" ]; then
        echo -e "$SCRIPTNAME: $*" >> $LOGFILE
    fi
}

function printnlog
{
    if [ -n "$LOGFILE" ]; then
        echo -n -e "$SCRIPTNAME: $*" >> $LOGFILE
    fi
}

function print
{
    echo -e "$SCRIPTNAME: $*"
    printlog $*
}

function printn
{
    echo -n -e "$SCRIPTNAME: $*"
    printnlog $*
}

function runCommand
{
    stdoutlog="$LOGFILE"
    if [ -z "$stdoutlog" ]; then
        stdoutlog="/dev/null"
    fi

    eval "$1" >> $stdoutlog 2>&1
    LASTRESULT=$?
    print "         [$1 returned $LASTRESULT]"
}

function runCommandRedirect
{
    destinationfile=$1
    shift

    stdoutlog="$LOGFILE"
    if [ -z "$stdoutlog" ]; then
        stdoutlog="/dev/null"
    fi
    eval "$1" > $destinationfile 2>>$stdoutlog
    LASTRESULT=$?
    if [ $LASTRESULT -ne 0 ]; then
        rm $destinationfile
    fi
    print "         [$1 returned $LASTRESULT]"
}

function testParameter
{
    if [ -z "$2" ]; then
        echo "Please give a value to parameter $1"
        exit 1
    fi
}

function makeDir
{
    cd $BUILDDIR
    print "                  Creating directory $*"
    runCommand "mkdir $*"
}


#
# Removes the .svn directories
#
function removeVCDirs
{
    if ! [ -d "$BUILDDIR/$*" ]; then
        return
    fi
    print "                  Removing VCDirs $*"
    cd $BUILDDIR/$*
    find . -type d -a -name '.svn' | xargs rm -rf
}

function findRootWorkingDirectory
{
    DIR=$PWD
    while [ ! -d "$DIR/.git" ]; do
      DIR=$( dirname $DIR )
    done
    ROOTWORKINGDIRECTORY=$DIR
}

#
# This is our work-horse. getResource retrieves all
# data needed to assemble a working package.
# Available resources:
# admindir, source, documentation,
# languagelist, guitranslation, doctranslation
#
function getResource
{
    DESTINATION=$2
    SINGLEFILEHACK="no"
    DIR=$BUILDDIR
    case $1 in
        admindir)
#            COMMAND="svn export $SVN_CHECKOUT_OPTIONS $SVNROOT/$ADMINDIR $DESTINATION"
        ;;
        source)
            findRootWorkingDirectory
            DIR=$ROOTWORKINGDIRECTORY
            COMMAND="git $GIT_CHECKOUT_OPTIONS archive $TAGNAME | tar x -C $DESTINATION"
        ;;
        documentation)
            COMMAND="git $GIT_CHECKOUT_OPTIONS archive $TAGNAME doc | tar x -C $DESTINATION"
        ;;
        languagelist)
            SINGLEFILEHACK="yes"
            COMMAND="svn cat $SVN_CHECKOUT_OPTIONS $SVNROOT/$I18NBASE/subdirs"
        ;;
        guitranslation)
            DESTINATION=$3/$4.po
            SINGLEFILEHACK="yes"
            COMMAND="svn cat $SVN_CHECKOUT_OPTIONS $SVNROOT/$I18NBASE/$2/messages/$I18NSUB/$4.po"
        ;;
        doctranslation)
            DESTINATION=$3
            COMMAND="svn export $SVN_CHECKOUT_OPTIONS $SVNROOT/$I18NBASE/$2/docs/$I18NDOCSUB/$APPNAME $DESTINATION"
        ;;
        *)
            print "ERROR: Unknown resource $1"
            exit 1
        ;;
    esac

    print "         Retrieving resource: $1"
    printlog "         getResource $1: $COMMAND"

    CURRENTDIR=$PWD
    cd $DIR

    if [ $SINGLEFILEHACK = "yes" ]; then
       runCommandRedirect $DESTINATION "$COMMAND"
    else
       runCommand "$COMMAND"
    fi

    if [ $LASTRESULT != "0" ]; then
        print "Warning: Resource \"$1\" is not available."
    fi
    cd $CURRENTDIR
    return $LASTRESULT
}

#
# Creates the build dir, the dir where the packages
# are assembled.
#
function setupBuildDir
{
    print "Creating the build directory..."
    print "         ($BUILDDIR)"

    # clean up first
    if [ -d $BUILDDIR ]; then
        print "         Removing the old build directory"
        rm -rf $BUILDDIR
    fi

    mkdir $BUILDDIR
}

#
# Creates and initializes the tag dir, the dir under which the tagging
# takes place.
#
function setupTagDir
{
    print "Creating the tag directory..."
    print "         ($TAGDIR)"

    # clean up first
    if [ -d $TAGDIR ]; then
        print "         Removing the old tag directory"
        rm -rf $TAGDIR
    fi

    mkdir $TAGDIR
    runCommand "svn $SVN_CHECKOUT_OPTIONS co -N $SVNROOT/$TAGBASE/$APPNAME $TAGDIR"
}

#
# Gather all data in the build dir needed to build
# the application (without translations).
#
function assembleApplicationData
{
    print "Assembling the source code..."
    APPDIR=$APPNAME-$APPVERSION
    print "         ($APPDIR)"

    runCommand "git tag -a $TAGNAME -m \"$TAGMESSAGE\""

    mkdir -p $BUILDDIR/$APPDIR
    getResource "source" $BUILDDIR/$APPDIR
}

#
# Copies the GNU (README, TODO, Changelog, etc.) files.
#
function moveGNUFiles
{
    print "         Moving the standard GNU files"
    for file in $GNUFiles
    do
        if [ -f $BUILDDIR/$APPDIR/src/$file ]; then
            mv $BUILDDIR/$APPDIR/src/$file $BUILDDIR/$1
        fi
    done
}

#
# Cleanup the application or i18n directory by removing
# unneccessary files and directories.
#
function cleanupDirectory
{
    dir="$1"
    print "Removing unneccessary files and directories"
    for file in $CLEANUPFILES
    do
        path="$dir/$file"
        if [ -f "$path" ]; then
            rm -f $path
            printlog "         Removed file: $path"
        elif [ -d "$path" ]; then
            rm -Rf $path
            printlog "         Removed directory: $path"
        fi
    done
}

#
# After all data for the app is gathered, some files
# still need to be shuffled around.
#
function postProcessApplicationDir
{
    print "Post-processing the application directory..."

    if [ ! -z "$POSTPROCESSSCRIPT" ]; then
        print "Running post-processing script..."
        cd $BUILDDIR/$APPDIR
        $POSTPROCESSSCRIPT
        err="$?"
        if [ $err -ne 0 ]; then
            print "ERROR: Post-processing script failed (return status: $err)"
            exit $err
        fi
        print "Post-processing script finished."
    fi
}

#
# Determines what languages need to be retrieved.
# If LANGUAGES was not set using the -l switch,
# the kde-i18n/subdirs file is used.
#
function getLanguageList
{
    print "         Determining which languages to include..."
    if [ -z "$LANGUAGES" ]; then
        getResource "languagelist" languagelist
        if [ -e $BUILDDIR/languagelist ]; then
            LANGUAGES=`cat $BUILDDIR/languagelist`
            rm $BUILDDIR/languagelist
        else
            print "ERROR:"
            print "ERROR: Failed to automatically detect which languages to include."
            print "ERROR:"
            exit 1
        fi
    fi

    # Remove the excluded languages from the list
    LANGLIST=""
    for language in $LANGUAGES ; do
        EXCLUDE="false"
        for exclang in $EXCLUDELANGUAGES ; do
            if [ "$exclang" = "$language" ]; then
                EXCLUDE="true"
                break
            fi
        done
        if [ $EXCLUDE = "false" ]; then
            LANGLIST="$LANGLIST $language"
        fi
    done
    LANGUAGES=$LANGLIST
    print "Language list: $LANGUAGES"
}

#
# Creates the dir that holds the translations. This is either
# a "translations" dir in the application build dir, or, if the
# --split switch is used, the root dir of a separate i18n package.
#
function setupI18NDir
{

  I18NDIR=$APPDIR
  TRANSDIR="$I18NDIR/translations"
  makeDir $TRANSDIR
}

function createTranslationDirMakefile
{
    if [ $GETI18N = "no" ]; then
        return
    fi

    print "         Creating Makefile.am in $1."
    CONTENTS="SUBDIRS=$INCLUDED_LANGUAGES"
    echo $CONTENTS > $1/Makefile.am
}

function createTranslationMakefile
{
    print "         Creating Makefile.am in $1."
    CONTENTS="SUBDIRS="
    if [ -d "$1/messages" ]; then
        CONTENTS="$CONTENTS messages"
    fi
    if [ -d "$1/doc" ]; then
        CONTENTS="$CONTENTS doc";
    fi
    echo $CONTENTS > $1/Makefile.am
}

function createTranslationMakefiles
{
    if [ $GETI18N = "no" ]; then
        return
    fi

    for language in $INCLUDED_LANGUAGES ; do
        createTranslationMakefile $BUILDDIR/$TRANSDIR/$language
    done
}

function createGUITranslationMakefile
{
    print "         Creating Makefile.am in $2."
    echo "KDE_LANG = $1
SUBDIRS = \$(AUTODIRS)
POFILES = AUTO" > $2/Makefile.am
}

function createDocTranslationMakefile
{
    print "                  Creating Makefile.am in $2."
    echo "KDE_LANG = $1
KDE_DOCS=$APPNAME" > $2/Makefile.am
}

#
# Get the .po files and put them in xx/messages
# subdirectories, where xx is the language ISO code.
#
function retrieveGUITranslations
{
    if [ $GETI18N = "no" ]; then
        return
    fi

    print "Retrieving GUI translations..."
    setupI18NDir
    print "         ($TRANSDIR)"

    # determine which languages to get
    getLanguageList

    # then get them (its really simple actually)
    INCLUDED_LANGUAGES=""
    for language in $LANGUAGES
    do
        print "         Including language $language"
        makeDir $TRANSDIR/$language
        makeDir $TRANSDIR/$language/messages

        INCLUDE_THIS_LANG="yes"
        for pofile in $APP_POFILES; do
            getResource "guitranslation" $language $TRANSDIR/$language/messages $pofile
            if [ ! -e $BUILDDIR/$TRANSDIR/$language/messages/$pofile.po ]; then
                INCLUDE_THIS_LANG="no"
            fi
        done

        if [ $INCLUDE_THIS_LANG = "yes" ]; then
     #       createGUITranslationMakefile $language $BUILDDIR/$TRANSDIR/$language/messages
            INCLUDED_LANGUAGES="$INCLUDED_LANGUAGES $language"
        else
            rm -rf $TRANSDIR/$language
        fi
    done
}

function retrieveDocTranslations
{
    if [ $GETI18N = "no" ]; then
        return
    fi

    print "Retrieving documentation translations..."

    for language in $INCLUDED_LANGUAGES ; do
        print "         Including documentation for language $language"
        getResource "doctranslation" $language $TRANSDIR/$language/doc
        if [ ! -e $BUILDDIR/$TRANSDIR/$language/doc/index.docbook ]; then
            print "                  No translations for $language docs available."
            rm -rf $TRANSDIR/$language/doc
        fi
    done
}

#
# Create the configure script and Makefile.in's.
#
function packageApplication
{
    cd $BUILDDIR

#    print "Creating configure script and Makefile.in files"
#    (cd $APPDIR; runCommand make -f admin/Makefile.common cvs)

    cleanupDirectory $APPDIR

    TARNAME="$APPNAME-$APPVERSION.tar"
    if [ $PACKAGE = "gzip" ] || [ $PACKAGE = "yes" ]; then
        print "Packaging application ($TARNAME.gz)..."
        $GZIP $TARNAME.gz $APPDIR
        if [ $BUILDDIR != $TARBALLDIR ]; then
            cp $TARNAME.gz $TARBALLDIR
        fi
    fi
    if [ $PACKAGE = "bzip2" ] || [ $PACKAGE = "yes" ]; then
        print "Packaging application ($TARNAME.bz2)..."
        $BZIP2 $TARNAME.bz2 $APPDIR
        if [ $BUILDDIR != $TARBALLDIR ]; then
            cp $TARNAME.bz2 $TARBALLDIR
        fi
    fi
}

#
# Create the configure script and Makefile.in's.
#
function packageTranslations
{
    if [ $SPLIT != "yes" ] || [ $GETI18N != "yes" ]; then
        return
    fi

    cd $BUILDDIR

#    print "Creating translations configure script and Makefile.in files"
#    (cd $I18NDIR; runCommand make -f admin/Makefile.common cvs)

    cleanupDirectory $I18NDIR

    TARNAME="$APPNAME-i18n-$APPVERSION.tar"
    if [ $PACKAGE = "gzip" ] || [ $PACKAGE = "yes" ]; then
        print "Packaging translations ($TARNAME.gz)..."
        $GZIP $TARNAME.gz $I18NDIR
        if [ $BUILDDIR != $TARBALLDIR ]; then
            cp $TARNAME.gz $TARBALLDIR
        fi
    fi
    if [ $PACKAGE = "bzip2" ] || [ $PACKAGE = "yes" ]; then
        print "Packaging translations ($TARNAME.bz2)..."
        $BZIP2 $TARNAME.bz2 $I18NDIR
        if [ $BUILDDIR != $TARBALLDIR ]; then
            cp $TARNAME.bz2 $TARBALLDIR
        fi
    fi
}

#
# Force the script to quit
#
function cancelScript
{
    errorcode="$1"
    if [ -z "$errorcode" ]
    then
        errorcode="1"
    fi
    print "Script canceled ($errorcode)"
    exit $errorcode
}

#
# Do basic parameter checks, print some basic stuff
# and initialize basic things.
#
function initBasic
{
    trap "cancelScript" SIGINT SIGTERM

    if [ -n "$LOGFILE" ]; then
        echo "$SCRIPTNAME revision $SCRIPTVERSION  `date`" > $LOGFILE
    fi

    if [ -t "$APPNAME" ]; then
        print "You need to specify an application name (--app). See $SCRIPTNAME --help for more info."
        exit 1
    fi

    if [ "$GETI18N" = "yes" ] && [ -z "$I18NSUB" ]; then
        print "You need to specify the i18n subdir or disable i18n checkout (--i18n-sub or --noi18n). See $SCRIPTNAME --help for more info."
        exit 1
    fi
    if [ -z "$I18NDOCSUB" ]; then
        I18NDOCSUB="$I18NSUB"
    fi

    if [ -z "$APP_POFILES" ]; then
        APP_POFILES="$APPNAME"
    fi

    TAGNAME="v$APPVERSION"

    print "Using svn-root: $SVNROOT"
}

function initVars
{
    LOGFILE="$PWD/$SCRIPTNAME.log"

    APPNAME=""
    SVNROOT="svn://anonsvn.kde.org/home/kde"
    I18NBASE="trunk/l10n"
    I18NSUB=""
    I18NDOCSUB=""
    APP_POFILES=""
#    ADMINDIR="trunk/KDE/kde-common/admin"

    LASTRESULT="0"
    BUILDDIR="$PWD/build"
    TARBALLDIR=$BUILDDIR
    APPVERSION="`date +%d.%m.%y`"
    GETDOC="yes"
    GETI18N="yes"
    SPLIT="no"
    PLAINCONFIGURE="no"
    LISTTAGS="no"
    PACKAGE="no"
    LANGUAGES=""
    EXCLUDELANGUAGES="xx"
    GNUFiles="AUTHORS COPYING ChangeLog INSTALL TODO README README.MacOSX README.cwl kile-remote-control.txt Building-with-cmake.txt"
    CLEANUPFILES="autom4te.cache Makefile.cvs dist"
    POSTPROCESSSCRIPT=""

    SVN_CHECKOUT_OPTIONS=""

    BZIP2="tar jcf"
    GZIP="tar zcf"
    TAGDIR="$PWD/tag"
    TAGBASE="tags"
}

####################################################################
##                                                                ##
##                       Here we start:                           ##
##                                                                ##
####################################################################

# Set the default values for options.
initVars

# First check if the script is run within a Git working copy
git status 2>&1 > /dev/null

if [ $? -ne 0 ]; then
  echo "Please run this script from within a Git working copy."
  exit -1
fi

# Process the options given on the command-line.
while [ "$#" -gt 0 ]
do
    case $1 in
        -a|--app)
            testParameter $1 $2
            APPNAME="$2"
            shift
        ;;
        -d|--svnroot)
            testParameter $1 $2
            SVNROOT="$2"
            shift
        ;;
        -ib|--i18n-base)
            testParameter $1 $2
            I18NBASE="$2"
            shift
        ;;
        -is|--i18n-sub)
            testParameter $1 $2
            I18NSUB="$2"
            shift
        ;;
        -ids|--i18n-docsub)
            testParameter $1 $2
            I18NDOCSUB="$2"
            shift
        ;;
        -tm|--tag-message)
            testParameter $1 $2
            TAGMESSAGE="$2"
            shift
        ;;
        --admin)
            testParameter $1 $2
            ADMINDIR="$2"
            shift
        ;;
        -b|--builddir)
            testParameter $1 $2
            BUILDDIR="$2"
            shift
        ;;
        -av|--appversion)
            testParameter $1 $2
            APPVERSION="$2"
            shift
        ;;
        --pofiles)
            testParameter $1 $2
            APP_POFILES="$APP_POFILES $2"
            shift
        ;;
        --nodoc)
            GETDOC="no"
        ;;
        --noi18n)
            GETI18N="no"
        ;;
        --split)
            SPLIT="yes"
        ;;
        --package)
            PKGTYPE="$2"
            if [ "$PKGTYPE" = "gzip" ] || [ "$PKGTYPE" = "bzip2" ]
            then
                shift
                PACKAGE="$PKGTYPE"
            else
                PACKAGE="yes"
            fi
        ;;
        --packagedir)
            testParameter $1 $2
            TARBALLDIR=$2
            shift
        ;;
        --postprocess)
            testParameter $1 $2
            POSTPROCESSSCRIPT=$2
            shift
        ;;
        -l|--languages)
            testParameter $1 $2
            LANGUAGES="$LANGUAGES $2"
            shift
        ;;
        -el|--exclude-languages)
            testParameter $1 $2
            EXCLUDELANGUAGES="$EXCLUDELANGUAGES $2"
            shift
        ;;
        --logfile)
            testParameter $1 $2
            if [ "$2" = "no" ]
            then
                LOGFILE=""
            else
                if [ "`echo $2 | cut -c1`" = "/" ]
                then
                    LOGFILE=$2
                else
                    LOGFILE=$PWD/$2
                fi
            fi
            shift
        ;;
        -h|--help)
            showHelp
            exit 0
        ;;
        -v|--version)
            showVersion
            exit 0
        ;;
        *)
            echo "Invalid parameter $1"
            exit 1
        ;;
    esac
    # process the next parameter
    shift
done

initBasic

# Create the builddir, the place where the package will be assembled.
# If no builddir is specified is specified "./$APPNAME-build" is used.
setupBuildDir

assembleApplicationData

retrieveGUITranslations
retrieveDocTranslations
# #createTranslationMakefiles
# #createTranslationDirMakefile $BUILDDIR/$TRANSDIR

postProcessApplicationDir

packageApplication
# packageTranslations

# vim: set et ts=4 sts=4:
