// input tests
//
// Kile should be started from the command line to view the results.

var fontCommands = new Array("\\textbf","\\textit","\\texttt","\\emph");

var result1 = kile.input.getListboxItem("Choose","Choose font command:",fontCommands);
print("getListboxItem:  "+ result1);

var result2 = kile.input.getComboboxItem("Choose","Choose font command:",fontCommands);
print("getComboboxItem: "+ result2);

var result3 = kile.input.getText("Choose","Label for text input:");
print("getText:         "+ result3);

var result4 = kile.input.getInteger("Choose","Label for integer input:");
print("getInteger:      "+ result4);

var result5 = kile.input.getPosInteger("Choose","Label for positive integer input:");
print("getPosInteger:   "+ result5);


