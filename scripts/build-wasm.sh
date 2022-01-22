SRC_DIR="../src"
DATA_DIR="../data"

RAY_DIR="../vendor/raylib-4.0.0_webassembly/"
RAY_INCLUDE="${RAY_DIR}/include"
RAY_LIB="${RAY_DIR}/lib"

SOURCES="${SRC_DIR}/main.cpp"
EMCCFLAGS="-s USE_GLFW=3 -s ASYNCIFY -s ALLOW_MEMORY_GROWTH=1"
emcc --std=gnu++1z -o index.html -I"${RAY_INCLUDE}" ${SOURCES} --preload-file "${DATA_DIR}@/data/" -L"${RAY_LIB}" -lraylib ${EMCCFLAGS}
