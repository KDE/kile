// Insert a LaTeX array with m lines and n columns

var n = kile.input.getPosInteger("Enter Value",
                                 "Please enter the number of columns:");
var m = kile.input.getPosInteger("Enter Value",
                                  "Please enter the number of lines:");

var cursor = view.cursorPosition();

document.editBegin();
document.insertText("\\begin{array}{");
for(var i = 0; i < n; i++) {
	document.insertText("c");
}
document.insertText("}\n");
for(var j = 0; j < m; j++) {
	for(var i = 0; i < n; i++) {
		document.insertBullet();
		if(i+1 != n) {
			document.insertText(" & ");
		}
		else {
			document.insertText("\\\\\n");
		}
	}
}
document.insertText("\\end{array}\n");

view.setCursorPosition(cursor);
document.nextBullet();
document.editEnd();
