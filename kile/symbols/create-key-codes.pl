#! /usr/bin/perl 

# Author: Thomas Braun
# email: braun@physik.fu-berlin.de
# date: Mon Dec 5 2005
# License: GPL

use strict;
use warnings ;

my $eindat=$ARGV[0];
my $ausdat=$ARGV[1];
my $suffix=$ARGV[2];

my ($number,$content,$found);

unless(defined($eindat) && defined($ausdat) && defined($suffix))
{
print "Missing args\n";
exit 1;
}


open(READ,"< $eindat");
open(WRITE,"> $ausdat");

 while(<READ>)
{
	$content=$_;
	if ($content =~ m/^\\command\[(.*)\]/ || $content =~ m/^\\command{(.*)}/)
	{		
		$number++;
		$found =$1;
		$found =~ s/\\/\\\\/g; # escape backslash
		$found =~ s/\"/\\\"/g; # "
		$found =~ s/\'/\\\'/g; # '
		
		print WRITE "\"$found\", // img" . $number . $suffix . ".png\n";
	}
}	
exit 0;
