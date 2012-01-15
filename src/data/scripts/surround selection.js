// Surround selected text with a TeX command
// Relative cursor position will not be changed.
//
// abc --> \texcommand{abc}

var range = view.selectionRange();
//print( "range = " + range.toString() );

if ( range.isValid() ) {
	var cmd = kile.input.getLatexCommand("Choose","Choose surrounding LaTeX command:");
//	print("cmd: "+ cmd);
	if ( cmd != "" ) {
		surroundTexCommand("\\"+cmd,range);
	}
}
else {
	kile.alert.sorry("No selection found.");
}

function surroundTexCommand(cmd,r)
{
	var c = view.cursorPosition();

	document.editBegin();
	view.clearSelection();
	document.insertText(r.end,"}");
	document.insertText(r.start,cmd+"{");

	c.column = c.column + cmd.length + 2;
	view.setCursorPosition(c);
	document.editEnd();
}
