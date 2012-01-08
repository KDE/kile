// Test for firstColumn/lastColumn
//
// Needs original 'scripting-test.tex' as current tex file opened in Kile.
// Kile should be started from the command line to view the results.

view.setCursorPosition(0,0);

print();
print( "Test: first/last column...");

firstColumnTest(22,0);
firstColumnTest(23,1);

lastColumnTest(10,-1);
lastColumnTest(11,7);

print("finished");
print();

function firstColumnTest(line,expected)
{
	var col = document.firstColumn(line);
	if ( col != expected ) {
		print ("line "+ line + " column: " + col + "  expected: "+expected);
	}
}

function lastColumnTest(line,expected)
{
	var col = document.lastColumn(line);
	if ( col != expected ) {
		print ("line "+line+" column: " + col + "  expected: "+expected);
	}
}



