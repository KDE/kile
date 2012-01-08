// texgroup tests
//
// Needs original 'scripting-test.tex' as current tex file opened in Kile.
// Kile should be started from the command line to view the results.

var arr = new Array(
	new Array(5,11, 5,12,5,19, "amsmath"),
	new Array(5,12, 5,12,5,19, "amsmath"),
	new Array(5,14, 5,12,5,19, "amsmath"),
	new Array(5,19, 5,12,5,19, "amsmath"),
	new Array(5,20, 5,12,5,19, "amsmath")
);


var len = arr.length;

print();
print( "Test: texgroup...");
for ( var i=0; i<len; ++i ) {
	texgroupTest(arr[i]);
}
print("finished");
print();


function texgroupTest(arr)
{
	var line = arr[0];
	var col = arr[1];
	view.setCursorPosition(line,col);

	var outside = new Range(arr[2],arr[3]-1,arr[4],arr[5]+1);
	var inside = new Range(arr[2],arr[3],arr[4],arr[5]);
	var expectedOutside = "{" + arr[6] + "}";
	var expectedInside = arr[6];

	var orange = document.texgroupRange(false);
	var irange = document.texgroupRange(true);
	var otext = document.texgroup(false);
	var itext = document.texgroup(true);

	view.clearSelection();
	view.selectTexgroup(false);
	var oseltext = view.selectedText();

	view.clearSelection();
	view.selectTexgroup(true);
	var iseltext = view.selectedText();

	view.clearSelection();

	view.setCursorPosition(line,col);
	document.gotoBeginTexgroup();
	var bcursor = view.cursorPosition();

	view.setCursorPosition(line,col);
	document.gotoEndTexgroup();
	var ecursor = view.cursorPosition();

	if ( !outside.equals(orange) ) {
		print ("outside: " + orange.toString() + "  expected: "+outside.toString());
	}
	if ( !inside.equals(irange) ) {
		print ("inside: " + irange.toString() + "  expected: "+inside.toString());
	}
	if ( otext != expectedOutside ) {
		print ("outside text: " + otext);
		print ("--> expected: " + expectedOutside);
	}
	if ( itext != expectedInside ) {
		print ("inside text:  " + itext);
		print ("--> expected: " + expectedInside);
	}

	if ( oseltext != expectedOutside ) {
		print ("outside sel text: " + oseltext);
		print ("--> expected:     " + expectedOutside);
	}
	if ( iseltext != expectedInside ) {
		print ("inside sel text:  " + iseltext);
		print ("--> expected:     " + expectedInside);
	}

	if ( !bcursor.equals(orange.start) ) {
		print("gotoBeginTexgroup: " + bcursor.toString() + "  expected: "+orange.start.toString());
	}

	if ( !ecursor.equals(orange.end) ) {
		print("gotoBeginTexgroup: " + ecursor.toString() + "  expected: "+orange.end.toString());
	}

}

