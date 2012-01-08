// Range tests
//
// The results of this script are not automatically checked.
// Kile should be started from the command line to view the results.

var r1 = new Range(0,0,0,0);
var r2 = new Range(0,0,0,3);
var r3 = new Range(1,0,5,6);
var r4 = new Range(new Cursor(0,0),new Cursor(5,6));
var r5 = new Range();
var r6 = new Range().invalid();

rangetest(r1);
rangetest(r2);
rangetest(r3);
rangetest(r4);
rangetest(r5);
rangetest(r6);


function rangetest(range)
{
	print("Test:          " + range.toString() );
	print("isValid:       " + range.isValid() );
	print("columnWidth:   " + range.columnWidth() );
	print("numberOfLines: " + range.numberOfLines() );
	print("onSingleLine:  " + range.onSingleLine() );
	print("isEmpty:       " + range.isEmpty() );

	equals(range);
	clone(range);
	construct(range);
	print();
}

function equals(range)
{
	print("compareTo Range(0,0 - 0,0): " + range.equals(r1));
	print("compareTo Range(0,0 - 0,3): " + range.equals(r2));
	print("compareTo Range(0,0 - 5,6): " + range.equals(r3));
	print("compareTo Range(0,0 - 5,6): " + range.equals(r3));
	print("compareTo Range():          " + range.equals(r4));
	print("compareTo Range(i,i):       " + range.equals(r5));
}

function clone(range)
{
	var r = range.clone();
	print("clone:         " +  r.toString() );
	print("isValid:       " +  r.isValid() );
}

function construct(range)
{
	var r = new Range(range);
	print("constructor:   " + r.toString() );
	print("isValid:       " + r.isValid() );
}

