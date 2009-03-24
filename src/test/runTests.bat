@echo off
rem The shift operator in Windows doesn't affect %*.  Hence the annoying %2 %3 %4...
echo "Kile System Check script..."

set outfile=%1
set basedir=%2

echo outfile = %1, basedir = %2

set goAhead=ok

rem see man dbus-send for an explanation, also the qdbusviewer from the QT package serves very well
set kileDBUS=dbus-send --type=method_call --print-reply --dest=net.sourceforge.kile /main net.sourceforge.kile.main
set openDoc=%kileDBUS%.openDocument string:
set closeDoc=%kileDBUS%.closeDocument

set totalnotests=17
set current=0


echo #script:%0 > %outfile%
echo #basedir:%basedir% >> %outfile%

goto :afterfuncs

:setTool
	set goAhead=ok
	echo.  >> %outfile%
	echo [%*] >> %outfile%
	goto :eof

:setKey
	set val=%2 %3 %4 %5 %6 %7 %8
	rem strip " marks if found
	set val=%val:"=%
	set val=%val:\=\\%
	echo %1=%val% >> %outfile%
	goto :eof

:performTest
	if %goAhead% NEQ ok goto :eof

	%2 %3 %4 %5 %6 %7 %8 > junkfile 2>&1
	set result=%errorlevel%
	if %result% NEQ 0 (
          set goAhead=notok
          goto :eof 
        )

	call :setKey %1 %result%
	set /a current=%current%+1
	set /a percentdone = (( (100*%current%)/%totalnotests%))
	echo %percentdone%
	goto :eof

:performKileTest
	if %goAhead% NEQ ok goto :eof

	%kileDBUS%.%2 %3 %4 %5 %6 %7 %8
	set result=%errorlevel%
	call :setKey %1 %result%
	set /a current=%current%+1
	set /a percentdone = (( (100*%current%)/%totalnotests%))
	echo %percentdone%
	goto :eof

:findAndSetWhere
	set loc=
	for /f "tokens=*" %%i in ('where %1') do set loc=%%i 
	call :setKey where %loc%
	goto :eof

:afterfuncs

cd %basedir%

set testFile=test_plain.tex
echo opening %basedir%\%testFile%
%openDoc%%basedir%\%testFile%

echo starting test: TeX
call :setTool TeX
set tool=tex --interaction=nonstopmode
call :setKey mustpass "where,basic,kile"
call :setKey executable tex
call :performTest basic %tool% test_plain.tex
call :performKileTest kile runTool string:TeX
call :findAndSetWhere TeX

echo starting test: PDFTeX
call :setTool PDFTeX
set tool=pdftex --interaction=nonstopmode
call :setKey mustpass 
call :setKey executable pdftex
call :performTest basic %tool% test_plain.tex
call :performKileTest kile runTool string:PDFTeX
call :findAndSetWhere pdftex
%closeDoc%


set testFileBase=test
set testFile=%testFileBase%.tex
echo opening %basedir%\%testFile%
%openDoc%"%basedir%\%testFile%"
echo starting test: LaTeX
call :setTool LaTeX
set tool=latex --interaction=nonstopmode
call :setKey mustpass "where,basic,kile"
call :setKey executable latex
call :findAndSetWhere latex
call :performTest basic %tool% %testFile%
call :performKileTest kile runTool string:LaTeX
call :performTest src %tool% -src %testFile%


echo starting test: PDFLaTeX
call :setTool PDFLaTeX
call :setKey mustpass ""
call :setKey executable pdflatex
call :findAndSetWhere pdflatex
call :performTest basic pdflatex %testFile%
call :performKileTest kile runTool string:PDFLaTeX


echo starting test: DVItoPS
call :setTool DVItoPS
call :setKey mustpass ""
call :setKey executable dvips 
call :findAndSetWhere dvips
if exist %testFileBase%.dvi call :performKileTest kile runTool string:DVItoPS


echo starting test: DVItoPDF
call :setTool DVItoPDF
call :setKey mustpass ""
call :setKey executable dvipdfmx
call :findAndSetWhere dvipdfmx
if exist %testFileBase%.dvi call :performKileTest kile runTool string:DVItoPDF


echo starting test: PStoPDF
call :setTool PStoPDF
call :setKey mustpass ""
call :setKey executable ps2pdf
call :findAndSetWhere ps2pdf
if exist %testFileBase%.ps call :performKileTest kile runTool string:PStoPDF
%closeDoc%


echo starting test: BibTeX
call :setTool BibTeX
call :setKey mustpass ""
call :setKey executable bibtex
call :findAndSetWhere bibtex 

if exist %testFileBase%.dvi (
	set testFileBase=test_bib
	set testFile=%testFileBase%.tex
	%openDoc%"%basedir%\%testFile%"
	latex --interaction=nonstopmode %testFile%
	call :performTest basic bibtex %testFileBase%
	call :performKileTest kile runTool string:BibTeX
	%closeDoc%
)

echo starting test: MakeIndex
call :setTool MakeIndex
call :setKey mustpass ""
call :setKey executable makeindex
call :findAndSetWhere makeindex

if exist test.dvi (
	set testFileBase=test_index
	set testFile=%testFileBase%.tex
	%openDoc%"%basedir%\%testFile%"
	latex --interaction=nonstopmode %testFile%
	call :performTest basic makeindex %testFileBase%
	call :performKileTest kile runTest string:MakeIndex
	%closeDoc%
)

echo starting test: Okular
call :setTool Okular
call :setKey mustpass "where"
call :setKey executable okular
call :findAndSetWhere okular

echo starting test: Acroread
call :setTool Acroread
call :setKey mustpass ""
call :setKey executable acrord32
call :findAndSetWhere acrord32

echo starting test: DVItoPNG
call :setTool DVItoPNG
call :setKey mustpass ""
call :setKey executable dvipng
call :findAndSetWhere dvipng

echo starting test: Convert
call :setTool Convert
call :setKey mustpass ""
call :setKey executable convert
call :findAndSetWhere convert
