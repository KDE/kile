#!/usr/bin/perl

# tbraun 10/10/2007

print "# DELETE [Tools]DVItoPDF\n";

print <<EOT;

[Tool/DVItoPDF/Modern]
class=Convert
command=dvipdfmx
options='%S.dvi'
from=dvi
to=pdf
type=Process

[Tools]
DVItoPDF=Modern

EOT
