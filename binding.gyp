{
  "targets": [
    {
      "target_name": "mcnet",
      "sources": [
        "src/mcnet.cc",
        "src/parser.cc"
      ],
      "conditions": [
        ['OS=="linux"', {
          "include_dirs": ["deps/libmcnet/include"],
          "libraries": ["<(module_root_dir)/deps/libmcnet/libmcnet.so"],
        }]
      ]
    }
  ]
}
