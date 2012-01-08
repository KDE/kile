// nextColumn/prevColumn tests
//
// Needs original 'scripting-test.tex' as current tex file opened in Kile.
// Kile should be started from the command line to view the results.

view.setCursorPosition(0,0);

print();
print( "Test: non space column...");

nextColumnTest(11,6,7);
nextColumnTest(11,7,-1);
nextColumnTest(11,8,-1);
nextColumnTest(18,27,28);
nextColumnTest(18,28,31);

prevColumnTest(11,11,7);
prevColumnTest(11,9,7);
prevColumnTest(11,8,7);
prevColumnTest(11,0,-1);
prevColumnTest(18,32,31);
prevColumnTest(18,31,28);
prevColumnTest(18,0,-1);

print("finished");
print();

function nextColumnTest(line,col,expected)
{
	var nonspace = document.nextNonSpaceColumn(line,col);
	if ( nonspace != expected ) {
		print ("pos: "+ line + "/"+col + " nonspace: " + nonspace + "  expected: "+expected);
	}
}

function prevColumnTest(line,col,expected)
{
	var nonspace = document.prevNonSpaceColumn(line,col);
	if ( nonspace != expected ) {
		print ("pos: "+ line + "/"+col + " nonspace: " + nonspace + "  expected: "+expected);
	}
}



