#!/usr/bin/perl

# tbraun 18.2.2007
# add metapost.cwl file

foreach (<>)
{
	$content=$_;
	$content .= ",0-metapost";
	print $content;
}
