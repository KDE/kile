debug("Test kile.file object:");

debug("1: read(filename):");
var result = kile.file.read("/etc/resolv.conf");
debug("status = " + result.status);

if ( result.status == 0 )
	document.insertText(result.text);
else
	debug(result.message);
debug();

debug("2: read():");
var result = kile.file.read();
debug("status = " + result.status);

if ( result.status == 0 )
	document.insertText(result.text);
else
	debug(result.message);
debug();

debug("3: getOpenFileName():");
var s = kile.file.getOpenFileName();
debug("open file = " + s);
debug();

debug("4: writeFile(filename,text):");
var filename = kile.input.getText("Save file", "filename");
debug("write to file = " + filename);
var s = kile.file.write(filename,document.text());
debug("status = " + result.status);

if ( result.status > 0 )
	debug(result.message);
debug();

debug("5: writeFile(text):");
var s = kile.file.write(document.text());
debug("status = " + result.status);

if ( result.status > 0 )
	debug(result.message);
debug();
