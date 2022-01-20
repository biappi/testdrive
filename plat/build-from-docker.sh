docker run -it --rm -v "$PWD/..":/src emscripten/emsdk  /bin/bash -c "cd plat; sh build-wasm.sh"

