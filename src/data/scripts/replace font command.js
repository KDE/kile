// Replace a surrounding LaTeX font command with another font command, when the cursor is placed inside the texgroup.
// Relative cursor position will not be changed.
//
// \textbf{abc} --> \textit{abc}

var fontCommands = new Array("\\textbf","\\textit","\\textsl","\\texttt","\\textsc","\\textrm","\\textsf","\\emph");

var range = document.texgroupRange(false);
//print( "range = " + range.toString() );
if ( range.isValid() ) {
	replaceFontCommand(range);
}
else {
	kile.alert.sorry("No surrounding TeX group found.");
}

function replaceFontCommand(r)
{
	var c = view.cursorPosition();
//	print( "c = " + c.toString() );

	document.editBegin();
	view.setCursorPosition(r.start);
	var cmd = document.latexCommand();
	var index = fontCommands.indexOf(cmd);
//	print( "cmd = " + cmd);
//	print( "index = " + index);
	if ( index >= 0 ) {
		var cmdRange = document.latexCommandRange();
//		print( "cmdRange = " + cmdRange.toString() );
		if ( cmdRange.isValid() ) {
			var newcommand = kile.input.getListboxItem("Choose","Choose font command:",buildCmdList(cmd));
			if ( newcommand != "" ) {
				document.replaceText(cmdRange,newcommand);
				c.column = c.column - (cmd.length - newcommand.length);
			}
		}
//		print( "c = " + c.toString() );
		view.setCursorPosition(c);
	}
	else {
		kile.alert.sorry("No surrounding font command found.");
	}
	document.editEnd();
}

function buildCmdList(current)
{
	var result = new Array();
	//var commands = new Array("\\textbf","\\textit","\\texttt","\\emph");
	for ( i=0; i<fontCommands.length; ++i ) {
		if ( fontCommands[i] != current ) {
			result.push(fontCommands[i]);
		}
	}
	return result;
}

