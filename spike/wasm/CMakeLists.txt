# WebAssembly の試作(docs/adr/0039)。Emscripten でビルドしたときだけ作る:
#   emcmake cmake -S . -B build-wasm -G Ninja -DTOKOTACHI_SHARED=OFF
#   cmake --build build-wasm --target spike_wasm
#   node build-wasm/spike/wasm/spike_wasm.js out.png
add_executable(spike_wasm main.cpp)
target_link_libraries(spike_wasm PRIVATE tokotachi)
# Node.js で動かす。NODERAWFS で OS のファイル(フォント・出力の PNG)をそのままのパスで読み書きする。
target_link_options(spike_wasm PRIVATE -sENVIRONMENT=node -sNODERAWFS=1 -sALLOW_MEMORY_GROWTH=1 -sEXIT_RUNTIME=1
                                       "--pre-js=${CMAKE_CURRENT_LIST_DIR}/pre.js")
