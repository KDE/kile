#!/usr/bin/perl
use POSIX qw(floor);
foreach (<>) 
{
	if(/^CleanUpFileExtensions(.*)/)
	{
		$values=$1;
		$values =~ s/,/ /g;
		print "CleanUpFileExtensions$values\n";
	}
	elsif (/^AutosaveInterval\s*=\s*(.*)\s*/)
	{
		$interval=floor($1/60000);
		if ( $interval < 1 ) { $interval=1; }
		print "AutosaveInterval=$interval\n";
	}
	elsif (/^use\s*=\s*(.*)\s*/)
	{
		$boolres=$1;
		$intres=0;
		if ( $boolres eq "true" ) { $intres = 1; }
		elsif ( $boolres eq "false" ) { $intres = 0; }
		print "use=$intres\n";
	}
	else
	{
		print $_;
	}
}
