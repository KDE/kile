#!/usr/bin/perl

# tbraun 9.2.2006

# first delete some groups and keys, if they exist
print "# DELETEGROUP [Tool/Archive/Default]\n";
print "# DELETEGROUP [Tool/LaTeXtoHTML/latex2html]\n";
print "# DELETEGROUP [Tool/LaTeXtoHTML/TeX4ht]\n";
print "# DELETEGROUP [Tool/LaTeXtoDocBook/Default]\n";
print "# DELETE [Tools]Archive\n";
print "# DELETE [Tools]LaTeXtoHTML\n";
print "# DELETE [Tools]LaTeXtoDocBook\n";
print "# DELETE [Tools]TeX4ht\n";
print "# DELETE [ToolsGUI]Archive\n";
print "# DELETE [ToolsGUI]LaTeXtoHTML\n";
print "# DELETE [ToolsGUI]LaTeXtoDocBook\n";
print "# DELETE [ToolsGUI]TeX\n";
print "# DELETE [ToolsGUI]PDFTeX\n";

# now filter the configuration file
print <<EOT;
[Tool/Archive/Tar]
class=Archive
command=tar
options=cvf '%S.tar' %AFL
from=kilepr
to=tar
type=Process

[Tool/Archive/Tar + gzip]
class=Archive
command=tar
options=zcvf '%S.tar.gz' %AFL
from=kilepr
to=tar.gz
type=Process

[Tool/Archive/Tar + bzip2]
class=Archive
command=tar
options=jcvf '%S.tar.bz2' %AFL
from=kilepr
to=tar.bz2
type=Process

[Tool/Archive/Zip]
class=Archive
command=zip
options='%S.zip' %AFL
from=kilepr
to=zip
type=Process

[Tool/DBLaTeX/Convert to DVI]
class=Compile
command=dblatex
options=-tdvi '%S.xml'
from=xml
to=dvi
type=Process

[Tool/DBLaTeX/Convert to PDF]
class=Compile
command=dblatex
from=xml
to=pdf
options=-tpdf '%S.xml'
type=Process

[Tool/DBLaTeX/Convert to LaTeX]
class=Compile
command=dblatex
from=xml
to=tex
options=-ttex '%S.xml'
type=Process

[Tool/DBLaTeX/Convert to Postscript]
class=Compile
command=dblatex
from=xml
to=ps
options=-tps '%S.xml'
type=Process

[Tool/DVItoPDF/Black and White]
command=dvipdfm
options=-c '%S.dvi'
from=dvi
to=pdf
type=Process

[Tool/DVItoPDF/Landscape]
command=dvipdfm
options=-l '%S.dvi'
from=dvi
to=pdf
type=Process

[Tool/LaTeX to Web/TeX4ht (LaTeX to HTML)]
class=Compile
command=mk4ht
from=tex
to=html
options=htlatex '%source'
type=Process

[Tool/LaTeX to Web/TeX4ht (LaTeX to MathML)]
class=Compile
command=mk4ht
from=tex
to=xml
options=xhmlatex '%source'
type=Process

[Tool/LaTeX to Web/TeX4ht (LaTeX to XML)]
class=Compile
command=mk4ht
from=tex
to=xml
options=xhlatex '%source'
type=Process

[Tool/LaTeX to Web/latex2html]
class=Compile
command=latex2html
options='%source'
from=tex
to=html
type=Process

[Tool/QuickBuild/LaTeX]
class=Sequence
type=Sequence
sequence=LaTeX

[Tool/ViewBib/JabRef]
type=Process
class=ViewBib
command=jabref
options='%source'
from=bib
to=bib

[Tool/ViewBib/gbib]
type=Process
class=ViewBib
command=gbib
options='%source'
from=bib
to=bib

[Tool/ViewBib/KBib]
class=ViewBib
command=kbib
options='%source'
from=bib
to=bib
type=Process

[Tool/Lilypond/PDF]
class=Compile
command=lilypond
options=--pdf '%source'
from=ly
to=pdf
type=Process

[Tool/Lilypond/DVI]
class=Compile
command=lilypond
options=--dvi '%source'
from=ly
to=dvi
type=Process

[Tool/Lilypond/TeX]
class=Compile
command=lilypond
options=--tex '%source'
from=ly
to=tex
type=Process

[Tool/Lilypond/PS]
class=Compile
command=lilypond
options=--ps '%source'
from=ly
to=ps
type=Process

[Tool/Lilypond/PNG]
class=Compile
command=lilypond
options=--png '%source'
from=ly
to=png
type=Process

[Tools]
Archive=Tar + gzip
DBLaTeX=Convert to LaTeX
LaTeX to Web=latex2html
Lilypond=PDF

[ToolsGUI]
Lilypond=Compile,lilypond
Archive=Archive,package
LaTeX to Web=Compile,l2h
DBLaTeX=Compile,dblatex
TeX=Compile,texcompiler
PDFTeX=Compile,pdftex
EOT
