// Mathgroup tests
//
// Needs original 'scripting-test.tex' as current tex file opened in Kile.
// Kile should be started from the command line to view the results.

var arr = new Array(
	new Array(18,31, 18,31,18,53, "c = \\sqrt{a^2 + b^2}"),
	new Array(18,32, 18,31,18,53, "c = \\sqrt{a^2 + b^2}"),
	new Array(18,41, 18,31,18,53, "c = \\sqrt{a^2 + b^2}"),
	new Array(18,52, 18,31,18,53, "c = \\sqrt{a^2 + b^2}"),
	new Array(18,53, 18,31,18,53, "c = \\sqrt{a^2 + b^2}")
);


var len = arr.length;

print();
print( "Test: mathgroup...");
for ( var i=0; i<len; ++i ) {
	mathgroupTest(arr[i]);
}
print("finished");
print();


function mathgroupTest(arr)
{
	var line = arr[0];
	var col = arr[1];
	view.setCursorPosition(line,col);

	var expectedRange = new Range(arr[2],arr[3],arr[4],arr[5]);
	var expectedText = "$" + arr[6] + "$";

	var range = document.mathgroupRange();
	var text = document.mathgroup();

	view.clearSelection();
	view.selectMathgroup();
	var seltext = view.selectedText();

	view.clearSelection();

	if ( !range.equals(expectedRange) ) {
		print ("range:        " + range.toString() + "  expected: "+expectedRange.toString());
	}
	if ( text != expectedText ) {
		print ("text:         " + text);
		print ("--> expected: " + expectedText);
	}

	if ( seltext != expectedText ) {
		print ("selected text: " + seltext);
		print ("--> expected:  " + expectedText);
	}

}

