// Remove a surrounding TeX command, when the cursor is placed inside the texgroup.
// Relative cursor position will not be changed.
//
// \texcommand{abc} --> abc

var range = document.texgroupRange(false);
//print( "range = " + range.toString() );
if ( range.isValid() ) {
	removeTexCommand(range);
}
else {
	kile.alert.sorry("No surrounding TeX group found.");
}

function removeTexCommand(r)
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
		document.removeText(r.end.line,r.end.column-1,r.end.line,r.end.column);
		document.removeText(cmdRange.start.line,cmdRange.start.column,cmdRange.end.line,cmdRange.end.column+1);
		c.column = c.column - (cmdRange.end.column-cmdRange.start.column) - 1;
	}
//	print( "c = " + c.toString() );
	view.setCursorPosition(c);
	document.editEnd();
}

