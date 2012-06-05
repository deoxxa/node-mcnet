var mcnet = module.exports = require("./build/Release/mcnet");

var events = require("events");

for (var k in events.EventEmitter.prototype) {
  mcnet.Parser.prototype[k] = events.EventEmitter.prototype[k];
}

exports.Parser = mcnet.Parser;
