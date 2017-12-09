#!/usr/bin/perl

# tbraun 18.7.2007

print <<EOT;

[Tool/BibTeX/8bitVersion]
class=Compile
command=bibtex8
options='%S'
from=aux
to=bbl
type=Process

[Tool/Asymptote/Default]
class=Compile
command=asy
options='%S'
from=
to=
type=Process

[Tool/PDFLaTeX/Draft]
class=LaTeX
command=pdflatex
options=-draftmode-interaction=nonstopmode'%source'
from=
to=pdf
type=Process
checkForRoot=yes
jumpToFirstError=yes
autoRun=yes

[Tool/PDFTeX/Draft]
class=Compile
command=pdftex
options=-draftmode-interaction=nonstopmode'%source'
from=
to=pdf
type=Process

[Tools]
Asymptote=Default
EOT
