# Toor Craft – Build Instructions

This project can be compiled in two different modes:

1. ✅ **Native (Linux / macOS / Windows)** – for development, debugging, and running on your machine.
2. 🌐 **WebAssembly (WASM via Emscripten)** – to run in a browser or JS environment.

---

## 📦 **Requirements**

### ✅ **For Native builds**
- **CMake** (>= 3.15)
- A C++20 compiler (e.g. `g++`, `clang++`, or MSVC)
- **Make** or **Ninja**

### 🌐 **For WebAssembly builds**
- [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) (latest version recommended)

After installing Emscripten:
```bash
source /path/to/emsdk/emsdk_env.sh
```
This adds `emcmake`, `emmake`, and `em++` to your PATH.

---

## 🚀 **Build Instructions**

### 🔹 **1️⃣ Native Build**

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

➡️ This will generate:
- A native executable in `examples/example_app` (runs in your terminal).

Run it:
```bash
./examples/example_app
```

---

### 🔹 **2️⃣ WebAssembly Build**

```bash
mkdir build-wasm
cd build-wasm
emcmake cmake ..
emmake make -j$(nproc)
```

➡️ This will generate:
- `examples/example_app.js`
- `examples/example_app.wasm`
- `examples/index.html` (auto-created for testing)

---

## 🌐 **Running in the Browser**

1. Start a **local web server** in the `build-wasm/examples/` directory (to avoid CORS issues):

```bash
cd build-wasm/examples
python3 -m http.server 8000
```

2. Open your browser and navigate to:

```
http://localhost:8000/index.html
```

✅ You should see the WASM module load and logs appear in the **browser console**.

---

## ⚙️ **Optional: Enable Asyncify for Sleeping**

If you plan to use `emscripten_sleep()` (non-blocking sleep in WASM), add the **Asyncify flag**:

Edit `CMakeLists.txt` (Emscripten section):
```cmake
set_target_properties(example_app PROPERTIES
    LINK_FLAGS "--js-library ${CMAKE_SOURCE_DIR}/wasm/glue.js -s MODULARIZE=1 -s EXPORT_ES6=1 -s ENVIRONMENT=web -sEXPORTED_RUNTIME_METHODS=['callMain'] -s ASYNCIFY"
)
```

Rebuild with:
```bash
rm -rf build-wasm
mkdir build-wasm
cd build-wasm
emcmake cmake ..
emmake make -j$(nproc)
```

This allows using:
```cpp
#include <emscripten.h>
emscripten_sleep(1000);
```

Without freezing the browser.

---

## 📜 **Summary**

✅ **Native**:  
```bash
cmake .. && make
./examples/example_app
```

🌐 **WebAssembly**:  
```bash
emcmake cmake ..
emmake make
python3 -m http.server
```

Now open `http://localhost:8000`.