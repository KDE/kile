// sectioning tests
//
// Needs original 'scripting-test.tex' as current tex file opened in Kile.
// Kile should be started from the command line to view the results.

var arr = new Array(
	new Array(8,  "\\section*{Test 1}"),
	new Array(12, "\\section*{Test 2}"),
	new Array(17, "\\section*{Test 3}"),
	new Array(21, "\\section*{Test 4}"),
	new Array(27, "\\section*{Test 5}")
);

var len = arr.length;
view.setCursorPosition(0,0);

print();
print( "Test: sectioning...");
for ( var i=0; i<len; ++i ) {
	sectioningTest(1,arr[i]);
}
print("part 1 finished");

view.setCursorPosition(document.lines()-1,0);
for ( var i=len-1; i>=0; --i ) {
	sectioningTest(-1,arr[i]);
}
print("part 2 finished");
print();

function sectioningTest(direction,arr)
{
	if ( direction == 1 )
		document.gotoNextSectioning();
	else
		document.gotoPrevSectioning();

	var cursor = view.cursorPosition();
	var line = cursor.line;
	var text = document.line( cursor.line );

	var lineExpected = arr[0];
	var textExpected = arr[1];

	if ( line != lineExpected ) {
		print ("line: " + line + "  expected: "+lineExpected);
	}
	if ( text != textExpected ) {
		print ("text: " + text + "  expected: "+textExpected);
	}
}

