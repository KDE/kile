#!/usr/bin/perl

# dani 15.09.2006

# first delete some groups and keys, if they exist
print "# DELETEGROUP [Tool/Convert/eps2png]\n";
print "# DELETEGROUP [Tool/Convert/pdf2png]\n";
print "# DELETEGROUP [Tool/DVItoPNG/Default]\n";
print "# DELETEGROUP [Tool/DVItoPS/dvi2eps]\n";
print "# DELETE [Tools]Convert\n";
print "# DELETE [Tools]DVItoPNG\n";
print "# DELETE [ToolsGUI]Convert\n";
print "# DELETE [ToolsGUI]DVItoPNG\n";
print "# DELETE [ToolsGUI]ViewBib\n";

# now filter the configuration file
print <<EOT;
[Tool/Convert/eps2png]
class=Convert
command=convert
options=+adjoin -antialias -trim -transparent white -density %res '%S.eps' '%S.png'
ofrom=eps
to=png
type=Process

[Tool/Convert/pdf2png]
class=Convert
command=convert
options=+adjoin -antialias -trim -transparent white -density %res '%S.pdf' '%S.png'
from=pdf
to=png
type=Process

[Tool/DVItoPNG/Default]
class=Convert
command=dvipng
options=-T tight -D %res -o '%S.png' '%S.dvi'
from=dvi
to=png
type=Process

[Tool/DVItoPS/dvi2eps]
class=Convert
command=dvips
options=-o '%S.eps' '%S.dvi'
from=dvi
to=eps
type=Process

[Tools]
Convert=pdf2png
DVItoPNG=Default

[ToolsGUI]
Convert=Convert,convert
DVItoPNG=Convert,dvipng
ViewBib=View,viewbib
EOT
