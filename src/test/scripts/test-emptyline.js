// Test for empty lines
//
// Needs original 'scripting-test.tex' as current tex file opened in Kile.
// Kile should be started from the command line to view the results.

view.setCursorPosition(0,0);

print();
print( "Test: empty lines...");

emptyLineTest(0,2);
emptyLineTest(2,0);

emptyLineTest(31,35);
emptyLineTest(35,31);

print("finished");
print();

function emptyLineTest(startline,expected)
{
	var line;
	if ( expected > startline )
		line = document.nextNonEmptyLine(startline);
	else
		line = document.prevNonEmptyLine(startline);

	if ( line != expected ) {
		print ("line: " + line + "  expected: "+expected);
	}
}

