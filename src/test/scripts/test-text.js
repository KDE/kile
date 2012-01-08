// Text tests
//
// Needs original 'scripting-test.tex' as current tex file opened in Kile.
// Kile should be started from the command line to view the results.

var arr = new Array(
	new Array(2,4, 2,24, "package[utf8x]{input"),
	new Array(2,4, 3,20, "package[utf8x]{inputenc}\n\\usepackage[T1]{font"),
	new Array(2,0, 3,24, "\\usepackage[utf8x]{inputenc}\n\\usepackage[T1]{fontenc}")
);


var len = arr.length;view

print();
print( "Test: text...");
for ( var i=0; i<len; ++i ) {
	textTest(arr[i]);
}
print("finished");
print();


function textTest(arr)
{
	var line1 = arr[0];
	var col1 = arr[1];
	var line2 = arr[2];
	var col2 = arr[3];
	var expectedText = arr[4];

	var cursor1 = new Cursor(line1,col1);
	var cursor2 = new Cursor(line2,col2);
	var range = new Range(line1,col1,line2,col2);

	var text1 = document.text(line1,col1,line2,col2);
	var text2 = document.text(cursor1,cursor2);
	var text3 = document.text(range);

	view.clearSelection();
	view.setSelection(range);
	var text4 = view.selectedText();

	view.clearSelection();

	if ( text1 != expectedText ) {
		print ("text1:  " + text1 + "  expected: "+expectedText);
	}

	if ( text2 != expectedText ) {
		print ("text2:  " + text2 + "  expected: "+expectedText);
	}

	if ( text3 != expectedText ) {
		print ("text3:  " + text3 + "  expected: "+expectedText);
	}

	if ( text1 != expectedText ) {
		print ("text4:  " + text4 + "  expected: "+expectedText);
	}

}

