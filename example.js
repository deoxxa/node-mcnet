#!/usr/bin/env node

var mcnet = require("./mcnet");

var parser = new mcnet.Parser(mcnet.Parser.prototype.TYPE_CLIENT);

parser.on("packet", function(packet) {
  console.log(packet);
});

parser.on("error", function(err) {
  console.log(["error", err]);
});

var buf = new Buffer([
//  0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x00, 0x61, 0x00, 0x73, 0x00, 0x64, 0x00, 0x66, 0x02, 0x03, 0x04, 0x05, 0x06,
//  0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x00, 0x61, 0x00, 0x73, 0x00, 0x64, 0x00, 0x66, 0x02, 0x03, 0x04, 0x05, 0x06,
//  0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x00, 0x61, 0x00, 0x73, 0x00, 0x64, 0x00, 0x66, 0x02, 0x03, 0x04, 0x05, 0x06,
//  0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x00, 0x61, 0x00, 0x73, 0x00, 0x64, 0x00, 0x66, 0x02, 0x03, 0x04, 0x05, 0x06,
  0x01,
  0x01,
  0x01,
  0x01,
  0xFF, 0x00, 0x04, 0x00, 0x61, 0x00, 0x73, 0x00, 0x64, 0x00, 0x66,
  0x33, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x01, 0x00, 0x03, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x01, 0x02, 0x03, 0x04,

  // Entity metadata packet - tests metadata parsing
  0x28,
  0x00, 0x00, 0x00, 0x01,
    0x01, 0x01,
    0x22, 0x00, 0x02,
    0x43, 0x00, 0x00, 0x00, 0x03,
    0x64, 0x00, 0x00, 0x00, 0x04,
    0x85, 0x00, 0x02, 0x00, 0x61, 0x00, 0x62,
    0xA6, 0x00, 0x01, 0x02, 0x00, 0x03,
    0xC7, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03,
    0x7F,

  // Set window items packet - tests multiple slot parsing
  0x68,
  0x01,
  0x00, 0x04,
    0x00, 0x01, 0x02, 0x00, 0x03,
    0x00, 0x05, 0x06, 0x00, 0x07,
    0xFF, 0xFF,
    0x00, 0x0A, 0x0B, 0x00, 0x0C,
]);

var nparsed;
while (nparsed = parser.execute(buf)) {
  if (nparsed == parser.EAGAIN) {
    break;
  } else if (nparsed == parser.EINVALID) {
    break;
  } else {
    if (buf.length <= nparsed) {
      buf = new Buffer(0);
    } else {
      buf = buf.slice(nparsed);
    }
  }
}
