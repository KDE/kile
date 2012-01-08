// alert tests

kile.alert.information("Alert","Information alert.");
kile.alert.sorry("Alert","Sorry alert.");
kile.alert.error("Alert","Error alert.");

var result1 = kile.alert.question("Alert","Question alert.");
print("Question alert: "+result1);

var result2 = kile.alert.warning("Alert","Warning alert.");
print("Warning alert: "+result2);

