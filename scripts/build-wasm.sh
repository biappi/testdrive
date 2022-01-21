SRC_DIR="../src"
DATA_DIR="../data"

RAY_DIR="../vendor/raylib-4.0.0_webassembly/"
RAY_INCLUDE="${RAY_DIR}/include"
RAY_LIB="${RAY_DIR}/lib"

SOURCES="${SRC_DIR}/Decoders.cpp ${SRC_DIR}/Models.cpp ${SRC_DIR}/Resources.cpp ${SRC_DIR}/main.cpp"

emcc --std=gnu++1z -o index.html -I"${RAY_INCLUDE}" ${SOURCES} -s USE_GLFW=3 --preload-file "${DATA_DIR}@/data/" -L"${RAY_LIB}" -lraylib -s ASYNCIFY
