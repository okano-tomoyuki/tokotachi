# pip(scikit-build-core)がビルドするときの入口。ライブラリを取り込んで C API を作り、パッケージの _lib に入れる。
cmake_minimum_required(VERSION 3.16)
project(tokotachi_python LANGUAGES C CXX)

set(TOKOTACHI_BUILD_C_API ON CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. tokotachi)

# 読み込むのはパスで指すので、版の付いた本体だけを入れる(wheel はシンボリックリンクを持てない)。
install(FILES $<TARGET_FILE:tokotachi_c> DESTINATION tokotachi/_lib)
