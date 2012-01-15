// Euro
//
// this script accepts selected text like
//
//  5Eur, 5 Eur, 5Euro, 5 Euro
//  5,4Eur, 5,4 Eur, 5,4Euro, 5,4 Euro
//  5,43Eur, 5,43 Eur, 5,43Euro, 5,43 Euro
//  5€, 5 €
//  5,4€, 5,4 €
//  5,43€, 5,43 €
//
// and converts the selection to \euro{...}
//
// needs LaTeX-package 'europs' and an additional LaTeX command:
// \newcommand{\euro}[1]{\ifmmode #1$\,\EURhv$\else$#1$\,\EURhv\fi{}}



var range = view.selectionRange();
if ( range.isValid() ) {
	var selection = view.selectedText();
	var re = /(\d+(,\d+)?)\s*(Euro?|€)/i;

	m = selection.match(re);
	if ( m ) {
		var s = selection.replace(re,"\\euro{$1}")
		document.replaceText(range,s);
	}
}
else {
	kile.alert.sorry("No selection found.");
}

