// Test for matchesAt/startsWith/endsWith
//
// Needs original 'scripting-test.tex' as current tex file opened in Kile.
// Kile should be started from the command line to view the results.

print();
print( "Test: match...");

matchesTest(9,0,"Hi",true);
matchesTest(9,0,"His",false);
matchesTest(9,4,"this",true);
matchesTest(9,4,"thiss",false);

startsWithTest(9,"Hi",true,true);
startsWithTest(9,"Hi",false,true);
startsWithTest(9,"His",true,false);
startsWithTest(9,"His",false,false);
startsWithTest(23," nested",true,false);
startsWithTest(23," nested",false,true);
startsWithTest(23," nestedd",true,false);
startsWithTest(23," nestedd",false,false);
startsWithTest(23,"nested",true,true);
startsWithTest(23,"nested",false,false);
startsWithTest(23,"nestedd",false,false);
startsWithTest(23,"nestedd",false,false);

endsWithTest(11,"newpage",true,true);
endsWithTest(11,"newpage",false,false);
endsWithTest(11,"newpag",true,false);
endsWithTest(11,"newpag",false,false);
endsWithTest(23,"...",true,true);
endsWithTest(23,"...",false,true);

print("finished");
print();

function matchesTest(line,col,text,expected)
{
	var match = document.matchesAt(line,col,text);
	if ( match != expected ) {
		print ("pos: "+ line + "/"+col + " match '"+ text + "': " + match + "  expected: "+expected);
	}
}

function startsWithTest(line,text,skipspaces,expected)
{
	var match = document.startsWith(line,text,skipspaces);
	if ( match != expected ) {
		print ("line: "+ line + " startswith '"+ text + "' (skip="+skipspaces+"): " + match + "  expected: "+expected);
	}
}

function endsWithTest(line,text,skipspaces,expected)
{
	var match = document.endsWith(line,text,skipspaces);
	if ( match != expected ) {
		print ("line: "+ line + " endswith '"+ text + "' (skip="+skipspaces+"): " + match + "  expected: "+expected);
	}
}




