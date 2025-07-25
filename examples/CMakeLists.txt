add_executable(example_app main.cpp)

target_link_libraries(example_app PRIVATE 
    EntitySchemaLib 
    EntityLib 
    FieldSchemaLib 
    FieldSchemaFactoryLib 
    FieldValueFactoryLib
    EntityManagerLib
    CommandLib
)

# ✅ Emscripten-specific settings
if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    message(STATUS "Compiling example_app for WebAssembly")

    set_target_properties(example_app PROPERTIES
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
  <title>example_app WASM Test</title>
</head>
<body>
  <h1>🚀 Running example_app (WASM)</h1>
  <script type='module'>
    import Module from './example_app.js';
    Module().then(mod => {
      console.log('✅ WASM Module loaded!');
      mod.callMain(['/virtual/lua/dir']);
    });
  </script>
</body>
</html>
")

    message(STATUS "✅ index.html generated at: ${INDEX_HTML_PATH}")
endif()
