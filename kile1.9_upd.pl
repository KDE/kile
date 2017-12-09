#!/usr/bin/perl

# tbraun 14.12.2005

# add Kbibtex as ViewBib tool
print <<EOT;
[Tool/ViewBib/KBibTex]
type=Process
class=ViewBib
command=kbibtex
options='%source'
from=bib
to=bib

[Tool/ViewBib/KBibTex (embedded)]
type=Part
class=ViewBib
libName=libkbibtexpart
className=KBibTeXPart
liboptions='%source'
state=Viewer
from=bib
to=bib
EOT
