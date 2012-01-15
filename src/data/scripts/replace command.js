// Replace a surrounding TeX command with another, when the cursor is placed inside the texgroup.
// Relative cursor position will not be changed.
//
// \texcommand{abc} --> \anothercommand{abc}

var range = document.texgroupRange(false);
//print( "range = " + range.toString() );
if ( range.isValid() ) {
	var cmd = kile.input.getLatexCommand("Choose","Choose new LaTeX command:");
//	print("cmd: "+ cmd);
	if ( cmd != "" ) {
		replaceTexCommand("\\"+cmd,range);
	}
}
else {
	kile.alert.sorry("No surrounding TeX group found.");
}

function replaceTexCommand(newcommand,r)
{
	var c = view.cursorPosition();
//	print( "c = " + c.toString() );

	document.editBegin();
	view.setCursorPosition(r.start);
	var cmd = document.latexCommand();
//	print( "cmd = " + cmd);

	var cmdRange = document.latexCommandRange();
//	print( "cmdRange = " + cmdRange.toString() );
	if ( cmdRange.isValid() ) {
		document.replaceText(cmdRange,newcommand);
		c.column = c.column - (cmd.length - newcommand.length);
	}
//	print( "c = " + c.toString() );
	view.setCursorPosition(c);
	document.editEnd();
}

