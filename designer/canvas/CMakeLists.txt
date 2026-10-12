# デザイナーのキャンバスの描画の部分(docs/adr/0039)。Emscripten で WebAssembly にする:
#   emcmake cmake -S designer/canvas -B designer/.cache/canvas -G Ninja -DCMAKE_BUILD_TYPE=Release
#   cmake --build designer/.cache/canvas
# ふつうは designer/tools/canvas/build.mts(pnpm canvas:build)が行い、packages/canvas/wasm/ に写す。
cmake_minimum_required(VERSION 3.16)
project(tokotachi_canvas CXX)

if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "tokotachi_canvas is built with Emscripten (emcmake cmake ...)")
endif()

get_filename_component(_root "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(TOKOTACHI_SHARED OFF CACHE BOOL "" FORCE)
set(TOKOTACHI_DEPS_PREFIX "${_root}/deps/install/emscripten-wasm32" CACHE PATH "")
add_subdirectory(${_root} tokotachi)

add_executable(tokotachi_canvas src/canvas.cpp src/json.cpp src/values.cpp src/generated/table.cpp)
target_link_libraries(tokotachi_canvas PRIVATE tokotachi::tokotachi)
set_target_properties(tokotachi_canvas PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF
                                                  SUFFIX ".mjs")
target_compile_options(tokotachi_canvas PRIVATE -Wall -Wextra -pedantic)
# 拡張(Node.js)から ES モジュールとして読む。NODERAWFS で OS のフォントをそのままのパスで読む
target_link_options(tokotachi_canvas PRIVATE
    -sENVIRONMENT=node -sNODERAWFS=1 -sMODULARIZE=1 -sEXPORT_ES6=1 -sALLOW_MEMORY_GROWTH=1
    -sEXPORTED_FUNCTIONS=_ttc_configure,_ttc_render,_ttc_defaults,_malloc,_free
    -sEXPORTED_RUNTIME_METHODS=ccall,HEAPU8)
