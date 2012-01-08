// Cursor tests
//
// The results of this script are not automatically checked.
// Kile should be started from the command line to view the results.

var c1 = new Cursor(0,0);
var c2 = new Cursor(1,0);
var c3 = new Cursor(2,1);
var c4 = new Cursor();
var c5 = new Cursor().invalid();

cursortest(1,c1);
cursortest(2,c2);
cursortest(3,c3);
cursortest(4,c4);
cursortest(5,c5);

var c6 = view.cursorPosition();
cursortest(6,c6);

view.setCursorPosition(0,0);
var c7 = view.cursorPosition();
cursortest(7,c7);

view.setCursorPosition(13,13);
view.cursorLeft();
var c8 = view.cursorPosition();

view.setCursorPosition(13,13);
print("cursor pos: Cursor(13,13) ---> ");
move(0);
move(1);
move(2);
move(3);
print();

view.setCursorPosition(12,0);
print("cursor pos: Cursor(12,0) ---> ");
move(0);
move(1);
move(2);
move(3);
print();

function cursortest(nr,cursor)
{
	print("Test " + nr + ":      "+ cursor.toString() );
	print("isValid:     " + cursor.isValid() );
	compareTo(cursor);
	equals(cursor);
	clone(cursor);
	construct(cursor);
	print();
}

function compareTo(cursor)
{
	print("compareTo Cursor(0,0): " + cursor.compareTo(c1));
	print("compareTo Cursor(1,0): " + cursor.compareTo(c2));
	print("compareTo Cursor(2,1): " + cursor.compareTo(c3));
	print("compareTo Cursor():    " + cursor.compareTo(c4));
	print("compareTo Cursor(i,i): " + cursor.compareTo(c5));
}

function equals(cursor)
{
	print("equals Cursor(0,0): " + cursor.equals(c1));
	print("equals Cursor(1,0): " + cursor.equals(c2));
	print("equals Cursor(2,1): " + cursor.equals(c3));
	print("equals Cursor():    " + cursor.equals(c4));
	print("equals Cursor(i,i): " + cursor.equals(c5));
}

function clone(cursor)
{
	var c = cursor.clone();
	print("clone:       " +  c.toString() );
	print("isValid:     " +  c.isValid() );
}

function construct(cursor)
{
	var c = new Cursor(cursor);
	print("constructor: " + c.toString() );
	print("isValid:     " + c.isValid() );
}

function move(direction)
{
	if ( direction == 0 )
		view.cursorLeft();
	else if ( direction == 1 )
		view.cursorRight();
	else if ( direction == 2 )
		view.cursorUp();
	else
		view.cursorDown();

	var cc = view.cursorPosition();
	print("cursor pos: " + cc.toString());
}
