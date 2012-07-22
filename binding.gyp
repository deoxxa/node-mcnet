{
  "targets": [
    {
      "target_name": "mcnet",
      "sources": [
        "src/mcnet.cc",
        "src/parser.cc",
      ],
      "dependencies": [
        "./deps/libmcnet/binding.gyp:libmcnet",
      ],
    },
  ],
}
