#!/usr/bin/env node

var mcnet = require("./build/Release/mcnet");

var parser = new mcnet.Parser();

var res = parser.execute();

if (res == parser.eagain) {
  console.log("need more dataz");
} else if (res == parser.einvalid) {
  console.log("wrong dataz");
} else {
  console.log("parsed " + res + " bytes");
}
