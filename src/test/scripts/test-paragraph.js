// paragraph tests
//
// Needs original 'scripting-test.tex' as current tex file opened in Kile.
// Kile should be started from the command line to view the results.

var arr = new Array( 0,2,5,7,11,16,20,26,35 );

var len = arr.length;
view.setCursorPosition(0,0);

print();
print( "Test: paragraph...");
for ( var i=0; i<len-1; ++i ) {
	paragraphTest(1,arr[i+1]);
}
print("part 1 finished");

view.setCursorPosition(document.lines(),0);
for ( var i=len-1; i>=1; --i ) {
	paragraphTest(-1,arr[i-1]);
}
print("part 2 finished");
print();

function paragraphTest(direction,expected)
{
	if ( direction == 1 )
		document.gotoNextParagraph();
	else
		document.gotoPrevParagraph();

	var cursor = view.cursorPosition();
	var line = cursor.line;
	var text = document.line( cursor.line );

	if ( line != expected ) {
		print ("line: " + line + "  expected: "+expected);
	}
}

