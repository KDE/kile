#!/usr/bin/perl

# dani 19.02.2005

# first delete some groups and keys, if they exist
print "# DELETEGROUP [Tool/PreviewLaTeX/Default]\n";
print "# DELETEGROUP [Tool/PreviewPDFLaTeX/Default]\n";
print "# DELETE [Tools]PreviewLaTeX\n";
print "# DELETE [Tools]PreviewPDFLaTeX\n";
print "# DELETE [ToolsGUI]PreviewLaTeX\n";
print "# DELETE [ToolsGUI]PreviewPDFLaTeX\n";

# now filter the configuration file
print <<EOT;
[Tool/PreviewLaTeX/Default]
autoRun=no
checkForRoot=no
class=LaTeXpreview
command=latex
from=
jumpToFirstError=yes
options=-interaction=nonstopmode '%source'
to=dvi
type=Process

[Tool/PreviewPDFLaTeX/Default]
autoRun=no
checkForRoot=no
class=LaTeXpreview
command=pdflatex
from=
jumpToFirstError=yes
options=-interaction=nonstopmode '%source'
to=pdf
type=Process

[Tools]
PreviewLaTeX=Default
PreviewPDFLaTeX=Default

[ToolsGUI]
PreviewLaTeX=none,none
PreviewPDFLaTeX=none,none
EOT
