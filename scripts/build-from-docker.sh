docker run -it --rm -v "$PWD/..":/src emscripten/emsdk  /bin/bash -c "cd scripts; sh build-wasm.sh"

