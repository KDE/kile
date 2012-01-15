// alert tests

kile.alert.information("Information alert.");
kile.alert.sorry("Sorry alert.");
kile.alert.error("Error alert.");

var result1 = kile.alert.question("Question alert.","Alert");
debug("Question alert: "+result1);

var result2 = kile.alert.warning("Warning alert.","Alert");
debug("Warning alert: "+result2);

