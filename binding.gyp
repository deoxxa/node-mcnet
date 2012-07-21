{
  "targets": [
    {
      "target_name": "mcnet",
      "sources": [
        "src/mcnet.cc",
        "src/parser.cc",
      ],
      "include_dirs": [
        "<(module_root_dir)/deps/libmcnet/include",
      ],
      "libraries": [
        "<(module_root_dir)/deps/libmcnet/build/Release/mcnet.node",
      ],
    },
  ],
}
