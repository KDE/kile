#!/usr/bin/perl

# tbraun 23.2.2006
# change names of cwl files

foreach (<>)
{
$content=$_;
$content=~ s/(0|1)-Common//g; # is now in latex-document
$content=~ s/(0|1)-Latex/\1-latex-document/g;
$content=~ s/(0|1)-Tex/\1-tex/g;
$content=~ s/(0|1)-MathSymbols/\1-latex-mathsymbols/g;

#clean up
$content=~ s/,,/,/g;
$content=~ s/(CompleteTex=),/\1/g;
$content=~ s/,$//g;

print $content;
}
