mergeInto(LibraryManager.library, {
  js_write_file: function (pathPtr, contentPtr) {
    const path = UTF8ToString(pathPtr);
    const content = UTF8ToString(contentPtr);
    console.log("📄 [JS] Writing to:", path);
    console.log("📝 Content:", content);

    // ✅ Store files in a global Map (mock FS)
    if (!globalThis.mockFS) {
      globalThis.mockFS = new Map();
    }
    globalThis.mockFS.set(path, content);
  },

  js_read_file: function (pathPtr) {
    const path = UTF8ToString(pathPtr);
    console.log("📂 [JS] Reading from:", path);

    if (!globalThis.mockFS || !globalThis.mockFS.has(path)) {
      console.warn("⚠️ [JS] File not found:", path);
      return 0; // return NULL to wasm
    }

    const text = globalThis.mockFS.get(path);

    // ✅ Allocate memory inside Wasm for the returned string
    const lengthBytes = lengthBytesUTF8(text) + 1;
    const buffer = _malloc(lengthBytes);
    stringToUTF8(text, buffer, lengthBytes);
    return buffer;
  }
});
