SRC_DIR="../src"
DATA_DIR="../data"

SOURCES="${SRC_DIR}/Decoders.cpp ${SRC_DIR}/Models.cpp ${SRC_DIR}/Resources.cpp ${SRC_DIR}/main.cpp"

emcc --std=gnu++1z -o index.html ${SOURCES} -s USE_GLFW=3 --preload-file "${DATA_DIR}@/data/" -L../vendor/raylib-3.7.0_wasm/lib/ -lraylib -s ASYNCIFY
