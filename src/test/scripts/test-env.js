// environment tests
//
// Needs original 'scripting-test.tex' as current tex file opened in Kile.
// Kile should be started from the command line to view the results.

var arr = new Array(
	new Array(22,4,  "center", 22,0,24,25, 22,14,24,13, "\\begin{center}\\begin{itshape}\n nested environments ...\n\\end{itshape}\\end{center}",  "\\begin{itshape}\n nested environments ...\n\\end{itshape}"),
	new Array(22,23, "itshape", 22,14,24,13, 22,29,24,0, "\\begin{itshape}\n nested environments ...\n\\end{itshape}",  "\n nested environments ...\n"),
	new Array(23,5, "itshape", 22,14,24,13, 22,29,24,0, "\\begin{itshape}\n nested environments ...\n\\end{itshape}",  "\n nested environments ...\n"),
	new Array(29,5, "align*", 28,0,31,12, 28,14,31,0, "\\begin{align*}\nc^2 &= a^2 + b^2 \\\\\nc &= \\sqrt{a^2 + b^2}\n\\end{align*}",  "\nc^2 &= a^2 + b^2 \\\\\nc &= \\sqrt{a^2 + b^2}\n")
);


var len = arr.length;

// ------ word wordAt ------
print();
debug( "Test: environment...");
for ( var i=0; i<len; ++i ) {
	envTest(arr[i]);
}
debug("finished");
print();


function envTest(arr)
{
	var line = arr[0];
	var col = arr[1];
	view.setCursorPosition(line,col);

	var outside = new Range(arr[3],arr[4],arr[5],arr[6]);
	var inside = new Range(arr[7],arr[8],arr[9],arr[10]);

	var envname = document.environmentName();
	var orange = document.environmentRange(false);
	var irange = document.environmentRange(true);
	var otext = document.environment(false);
	var itext = document.environment(true);

	var expectedEnvname = arr[2];
	var expectedOutside = arr[11];
	var expectedInside = arr[12];

	view.clearSelection();
	view.selectEnvironment(false);
	var oseltext = view.selectedText();

	view.clearSelection();
	view.selectEnvironment(true);
	var iseltext = view.selectedText();

	view.clearSelection();

	view.setCursorPosition(line,col);
	document.gotoBeginEnv();
	var bcursor = view.cursorPosition();

	view.setCursorPosition(line,col);
	document.gotoEndEnv();
	var ecursor = view.cursorPosition();

	print ("---> env: "+envname);

	if ( envname != expectedEnvname ) {
		print ("envname: " + envname + "  expected: "+expectedEnvname);
	}
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
		print("gotoBeginEnv: " + bcursor.toString() + "  expected: "+orange.start.toString());
	}

	if ( !ecursor.equals(orange.end) ) {
		print("gotoBeginEnv: " + ecursor.toString() + "  expected: "+orange.end.toString());
	}

}

