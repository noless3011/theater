{
  "targets": [
    {
      "target_name": "native_addon",
      "sources": [
        "addon.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "../include"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "defines": [
        "NAPI_DISABLE_CPP_EXCEPTIONS"
      ],
      "conditions": [
        [
          "OS=='win'",
          {
            "libraries": [
              "..\libtheater.lib"
            ],
            "copies": [
              {
                "destination": "<(module_root_dir)/build/Release/",
                "files": [
                  "<(module_root_dir)\libtheater.dll"
                ]
              }
            ]
          }
        ],
        [
          "OS=='linux'",
          {
            "libraries": [
              "-L<(module_root_dir)/native",
              "-llibtheater"
            ],
            "ldflags": [
              "-Wl,-rpath,'$$ORIGIN'"
            ]
          }
        ],
        [
          "OS=='mac'",
          {
            "libraries": [
              "-L<(module_root_dir)/native",
              "-llibtheater"
            ],
            "xcode_settings": {
              "LD_RUNPATH_SEARCH_PATHS": [
                "@loader_path"
              ]
            }
          }
        ]
      ]
    }
  ]
}