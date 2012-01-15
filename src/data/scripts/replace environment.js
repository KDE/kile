// Replace a surrounding LaTeX environment with another.
// Relative cursor position will not be changed.
//
// \begin{abc}...\end{abc} --> \begin{xyz}...\end{xyz}

var range = document.environmentRange(false);
//print( "r = " + range.toString() );
if ( range.isValid() ) {
	var envname = kile.input.getLatexCommand("Enter Environment","New environment name:");
	if ( envname != '' ) {
		replaceEnvCommand(envname,range);
	}
}
else {
	kile.alert.sorry("No surrounding LaTeX environment found.");
}

function replaceEnvCommand(newEnv,r)
{
	var c = view.cursorPosition();
//	print( "c = " + c.toString() );

	var envname = document.environmentName();
//	print( "env = " + envname );

	if ( envname != "" ) {
		var beginRange = new Range(r.start,new Cursor(r.start.line,r.start.column+8+envname.length));
		var endRange = new Range(new Cursor(r.end.line,r.end.column-6-envname.length),r.end);
//		print( "begin = " + beginRange.toString() );
//		print( "end = " + endRange.toString() );

		document.editBegin();
		document.replaceText(endRange,"\\end{"+newEnv+"}");
		document.replaceText(beginRange,"\\begin{"+newEnv+"}");
		document.editEnd();
	}
}

