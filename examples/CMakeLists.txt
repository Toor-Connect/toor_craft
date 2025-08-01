# ✅ Build the CLI executable
add_executable(toorcraft-cli main.cpp)

target_link_libraries(toorcraft-cli PRIVATE 
  ToorCraftRouterLib
  yaml-cpp
)

# ✅ Emscripten-specific settings (optional, only if you want WASM CLI in browser)
if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    message(STATUS "Compiling toorcraft-cli for WebAssembly")

    set_target_properties(toorcraft-cli PROPERTIES
        LINK_FLAGS "--js-library ${CMAKE_SOURCE_DIR}/wasm/glue.js \
                    -s MODULARIZE=1 \
                    -s EXPORT_ES6=1 \
                    -s ENVIRONMENT=web \
                    -s EXPORTED_RUNTIME_METHODS=['callMain'] \
                    -s INVOKE_RUN=0"
    )

    # ✅ Generate index.html at *configure time*, not build time
    set(INDEX_HTML_PATH ${CMAKE_CURRENT_BINARY_DIR}/index.html)

    file(WRITE ${INDEX_HTML_PATH} 
"<!DOCTYPE html>
<html>
<head>
  <meta charset='UTF-8'>
  <title>toorcraft-cli WASM</title>
</head>
<body>
  <h1>🚀 Running toorcraft-cli (WASM)</h1>
  <script type='module'>
    import Module from './toorcraft-cli.js';
    Module().then(mod => {
      console.log('✅ WASM Module loaded!');
      // Call CLI entry point (optional args can be passed)
      mod.callMain(['--schemas', '/virtual/schemas', '--data', '/virtual/data']);
    });
  </script>
</body>
</html>
")

    message(STATUS "✅ index.html generated at: ${INDEX_HTML_PATH}")
endif()
