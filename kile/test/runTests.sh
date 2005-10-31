#!/bin/sh
echo "Kile System Check script..."

outfile=$1
basedir=$2

echo "outfile = $1, basedir = $2"

goAhead=ok

kileDCOP="dcop kile Kile"
openDoc="$kileDCOP openDocument"
closeDoc="$kileDCOP closeDocument"

declare -i totalnotests=17
declare -i current=0

echo "#script:$0" > $outfile
echo "#basedir:$basedir" >> $outfile

function setTool
{
	goAhead=ok
	echo "" >> $outfile
	echo "[$*]" >> $outfile
}

function setKey
{
	key=$1
	shift
	echo "$key=$*" >> $outfile
}

function performTest
{
	if [ $goAhead != ok ]; then return; fi

	test=$1
	shift
	$* >/dev/null 2>&1
	declare -i result=$?
	if [ $result -eq 0 ]; then goAhead=ok; else goAhead=notok; fi

	setKey $test $result
	current=$current+1
	echo $(( (100*$current)/$totalnotests));
}

function performKileTest
{
	if [ $goAhead != ok ]; then return; fi

	key=$1
	shift
	declare -i result=`$kileDCOP $*`
	setKey $key $result
	current=$current+1
	echo $(( (100*$current)/$totalnotests));
}

function getVersion
{
	echo `$* -v || $* -version || $* --version`
}

function getTeXVersion
{
	($* -v || $* -version || $* --version) | grep '^TeX' | sed 's/TeX\s*\(.*\)\s*([^\s]*)/\1/'
}

cd $basedir
echo "current dir $PWD"

testFile=test_plain.tex
echo "opening $basedir/$testFile"
$openDoc $basedir/$testFile

echo "starting test: TeX"
setTool TeX
tool="tex --interaction=nonstopmode"
setKey mustpass "where,basic,kile"
setKey executable tex
setKey where `which tex`
setKey version `getTeXVersion tex`
performTest basic "$tool test_plain.tex"
performKileTest kile "run TeX"

echo "starting test: PDFTeX"
setTool PDFTeX
tool="pdftex --interaction=nonstopmode"
setKey mustpass ""
setKey executable pdftex
setKey where `which pdftex`
performTest basic "$tool test_plain.tex"
performKileTest kile "run PDFTeX"
$closeDoc

testFileBase="test"
testFile=$testFileBase.tex
echo "opening $basedir/$testFile"
$openDoc $basedir/$testFile
echo "starting test: LaTeX"
setTool LaTeX
tool="latex --interaction=nonstopmode"
setKey mustpass "where,basic,kile"
setKey executable latex
setKey where `which latex`
performTest basic "$tool $testFile"
performKileTest kile "run LaTeX"
performTest src "$tool -src $testFile"
performTest srcpkg "$tool test_src.tex"

echo "starting test: PDFLaTeX"
setTool PDFLaTeX
setKey mustpass ""
setKey executable pdflatex
setKey where `which pdflatex`
performTest basic "pdflatex $testFile"
performKileTest kile "run PDFLaTeX"

echo "starting test: DVItoPS"
setTool DVItoPS
setKey mustpass ""
setKey executable dvips
setKey where `which dvips` 
if [ -r $testFileBase.dvi ]; then performKileTest kile "run DVItoPS"; fi

echo "starting test: DVItoPDF"
setTool DVItoPDF
setKey mustpass ""
setKey executable dvipdfm
setKey where `which dvipdfm`
if [ -r $testFileBase.dvi ]; then performKileTest kile "run DVItoPDF"; fi

echo "starting test: PStoPDF"
setTool PStoPDF
setKey mustpass ""
setKey executable ps2pdf
setKey where `which ps2pdf`
if [ -r $testFileBase.ps ]; then performKileTest kile "run PStoPDF"; fi
$closeDoc

echo "starting test: BibTeX"
setTool BibTeX
setKey mustpass ""
setKey executable bibtex
setKey where `which bibtex`
if [ -r "test.dvi" ] #LaTeX is working
then
	testFileBase=test_bib
	testFile=$testFileBase.tex
	$openDoc $basedir/$testFile
	latex --interaction=nonstopmode $testFile
	performTest basic "bibtex $testFileBase"
	performKileTest kile "run BibTeX"
	$closeDoc
fi

echo "starting test: MakeIndex"
setTool MakeIndex
setKey mustpass ""
setKey executable makeindex
setKey where `which makeindex`
if [ -r "test.dvi" ] #LaTeX is working
then
	testFileBase=test_index
	testFile=$testFileBase.tex
	$openDoc $basedir/$testFile
	latex --interaction=nonstopmode $testFile
	performTest basic "makeindex $testFileBase"
	performKileTest kile "run MakeIndex"
	$closeDoc
fi

echo "starting test: KDVI"
setTool KDVI
setKey mustpass "where"
setKey executable kdvi
setKey where `which kdvi`

echo "starting test: KGhostView"
setTool KGhostView
setKey mustpass ""
setKey executable kghostview
setKey where `which kghostview`

echo "starting test: KPDF"
setTool KPDF
setKey mustpass ""
setKey executable kpdf
setKey where `which kpdf`

echo "starting test: Gv"
setTool Gv
setKey mustpass ""
setKey executable gv
setKey where `which gv`

echo "starting test: Acroread"
setTool Acroread
setKey mustpass ""
setKey executable acroread
setKey where `which acroread`
