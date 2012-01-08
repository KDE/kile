// insert/remove tests
//
// Needs original 'scripting-test.tex' as current tex file opened in Kile.
// Kile should be started from the command line to view the results.

print();
print( "Test: insert/remove...");

var line = 9;
var col = 12;
var add = "(ABC) ";
var originalText = "Hi, this is some text...";
var changedText = "Hi, this is " + add + "some text...";

insertTest(line,col);

print("finished");
print();


function insertTest(line,col)
{
	var c1 = new Cursor(line,col);
	var c2 = new Cursor(line,col+6);
	var r = new Range(line,col,line,col+6);

	document.insertText(line,col,add);
	var text1 = document.line(line);
	if ( text1 != changedText ) {
		print ("text added 1:  " + text1 + "  expected: "+changedText);
	}

	document.removeText(r);
	var text2 = document.line(line);
	if ( text2 != originalText ) {
		print ("text removed 1:  " + text2 + "  expected: "+originalText);
	}

	document.insertText(c1,add);
	var text3 = document.line(line);
	if ( text3 != changedText ) {
		print ("text added 2:  " + text3 + "  expected: "+changedText);
	}

	document.removeText(c1,c2);
	var text4 = document.line(line);
	if ( text4 != originalText ) {
		print ("text removed 2:  " + text4 + "  expected: "+originalText);
	}

}

