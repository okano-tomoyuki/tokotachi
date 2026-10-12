# 見本から生成した C++ を Tokotachi と一緒にビルドする(tools/codegen/verify-cpp.mts が作業フォルダに生成してから使う)。
#   -DTOKOTACHI_ROOT=<リポジトリ>  -DGENERATED_DIR=<生成したファイルのフォルダ>  -DTOKOTACHI_DEPS_PREFIX=<deps/install/<プラットフォーム>>
cmake_minimum_required(VERSION 3.16)
project(tokotachi_designer_verify CXX)

set(TOKOTACHI_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory(${TOKOTACHI_ROOT} tokotachi)

# 画像を参照するウィンドウは、画像のソース(*_images.cpp)も生成する
file(GLOB_RECURSE generated_images ${GENERATED_DIR}/*_images.cpp)
add_executable(verify_cpp ${CMAKE_CURRENT_LIST_DIR}/main.cpp ${GENERATED_DIR}/main_window.cpp
                          ${GENERATED_DIR}/settings_dialog.cpp ${GENERATED_DIR}/parts/labeled_edit.cpp
                          ${GENERATED_DIR}/widgets/level_bar.cpp ${generated_images})
target_include_directories(verify_cpp PRIVATE ${GENERATED_DIR})
target_link_libraries(verify_cpp PRIVATE tokotachi::tokotachi)
set_target_properties(verify_cpp PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
# 生成したコードは警告なしでコンパイルできること(ハンドラの雛形の使っていない引数を除く)
target_compile_options(verify_cpp PRIVATE -Wall -Wextra -pedantic -Werror -Wno-unused-parameter)
