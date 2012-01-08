// word/latexCommand tests
//
// Needs original 'scripting-test.tex' as current tex file opened in Kile.
// Kile should be started from the command line to view the results.

var arr1 = new Array(
	5,0,  "",
	5,1,  "usepackage",
	5,7,  "usepackage",
	5,11, "usepackage",
	5,12, "amsmath",
	5,15, "amsmath" ,
	5,19, "amsmath" ,
	5,20, "",
	8,0,  "",
	8,1,  "section",
	8,8,  "section",
	8,10, "Test",
	8,14, "Test"
);

var arr2 = new Array(
	5,0,  "\\usepackage",
	5,1,  "\\usepackage",
	5,7,  "\\usepackage",
	5,11, "\\usepackage",
	5,12, "amsmath",
	5,15, "amsmath" ,
	5,19, "amsmath" ,
	5,20, "",
	8,0,  "\\section*",
	8,1,  "\\section*",
	8,8,  "\\section*",
	8,10, "Test",
	8,14, "Test"
);

var len = arr1.length / 3;

// ------ word wordAt ------
print();
print( "Test: wordTest1...");
for ( var i=0; i<len; ++i ) {
	wordTest1(arr1[3*i],arr1[3*i+1],arr1[3*i+2]);
}
print("finished");
print();

print( "Test: wordTest2...");
for ( var i=0; i<len; ++i ) {
	wordTest2(arr1[3*i],arr1[3*i+1],arr1[3*i+2]);
}
print("finished");
print();

print( "Test: wordAt...");
for ( var i=0; i<len; ++i ) {
	wordAtTest(arr1[3*i],arr1[3*i+1],arr1[3*i+2]);
}
print("finished");
print();


// ------ latexCommand latexCommandAt ------

print( "Test: latexCommand1...");
for ( var i=0; i<len; ++i ) {
	latexCommandTest1(arr2[3*i],arr2[3*i+1],arr2[3*i+2]);
}
print("finished");
print();

print( "Test: latexCommand2...");
for ( var i=0; i<len; ++i ) {
	latexCommandTest2(arr2[3*i],arr2[3*i+1],arr2[3*i+2]);
}
print("finished");
print();

print( "Test: latexCommandAt...");
for ( var i=0; i<len; ++i ) {
	latexCommandAtTest(arr2[3*i],arr2[3*i+1],arr2[3*i+2]);
}
print("finished");
print();


function wordTest1(line,col,expected)
{
	view.setCursorPosition(line,col);
	var x = document.word();
	if ( x != expected ) {
		print ("wordAt " + line + "/" + col + ": " + x + "  expected: "+expected);
	}
}

function wordTest2(line,col,expected)
{
	var cursor = new Cursor(line,col);
	view.setCursorPosition(cursor);
	var x = document.word();
	if ( x != expected ) {
		print ("wordAt " + cursor.toString() + ": " + x + "  expected: "+expected);
	}
}

function wordAtTest(line,col,expected)
{
	var x = document.wordAt(line,col);
	if ( x != expected ) {
		print ("wordAt: " + x + "  expected: "+expected);
	}
}

function latexCommandTest1(line,col,expected)
{
	view.setCursorPosition(line,col);
	var x = document.latexCommand();
	if ( x != expected ) {
		print ("latexCommand " + line + "/" + col + ": " + x + "  expected: "+expected);
	}
}

function latexCommandTest2(line,col,expected)
{
	var cursor = new Cursor(line,col);
	view.setCursorPosition(cursor);
	var x = document.latexCommand();
	if ( x != expected ) {
		print ("latexCommand " + cursor.toString() + ": " + x + "  expected: "+expected);
	}
}

function latexCommandAtTest(line,col,expected)
{
	var x = document.latexCommandAt(line,col);
	if ( x != expected ) {
		print ("latexCommandAt: " + x + "  expected: "+expected);
	}
}
