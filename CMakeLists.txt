cmake_minimum_required(VERSION 3.16)
project(tokotachi LANGUAGES C CXX)

# 依存ライブラリは deps/ でソースからビルドしたもの(deps/install/<プラットフォーム>/)だけを使う(docs/adr/0005)。
# MSYS2 などのシステムに同じライブラリが入っていても混ざらないよう、find_package は既定の探索先を使わない。
include(${CMAKE_CURRENT_LIST_DIR}/cmake/TokotachiPlatform.cmake)
set(TOKOTACHI_DEPS_PREFIX "${CMAKE_CURRENT_LIST_DIR}/deps/install/${TOKOTACHI_PLATFORM}" CACHE PATH
    "Install prefix of the dependencies built by deps/CMakeLists.txt")
if(NOT EXISTS "${TOKOTACHI_DEPS_PREFIX}/lib/cmake")
    message(FATAL_ERROR "Dependencies not found in ${TOKOTACHI_DEPS_PREFIX}. Build them first:\n"
                        "  cmake -S deps -B build-deps -G Ninja\n"
                        "  cmake --build build-deps")
endif()

find_package(Threads REQUIRED)
find_package(SDL3 REQUIRED CONFIG COMPONENTS SDL3-static PATHS "${TOKOTACHI_DEPS_PREFIX}" NO_DEFAULT_PATH)
find_package(freetype REQUIRED CONFIG PATHS "${TOKOTACHI_DEPS_PREFIX}" NO_DEFAULT_PATH)
find_package(harfbuzz REQUIRED CONFIG PATHS "${TOKOTACHI_DEPS_PREFIX}" NO_DEFAULT_PATH)
find_package(plutovg REQUIRED CONFIG PATHS "${TOKOTACHI_DEPS_PREFIX}" NO_DEFAULT_PATH)
find_package(unibreak REQUIRED CONFIG PATHS "${TOKOTACHI_DEPS_PREFIX}" NO_DEFAULT_PATH)
find_package(SheenBidi REQUIRED CONFIG PATHS "${TOKOTACHI_DEPS_PREFIX}" NO_DEFAULT_PATH)

option(TOKOTACHI_STATIC_RUNTIME "Link the MinGW runtime (libgcc, libstdc++, winpthread) statically into executables" ON)

add_library(tokotachi STATIC
    src/actions/action.cpp
    src/core/application.cpp
    src/core/binding.cpp
    src/core/object.cpp
    src/windows/popup.cpp
    src/actions/shortcut.cpp
    src/core/task.cpp
    src/core/clipboard.cpp
    src/graphics/theme.cpp
    src/core/timer.cpp
    src/widgets/widget.cpp
    src/windows/window.cpp
    src/graphics/bitmap.cpp
    src/graphics/canvas.cpp
    src/graphics/image.cpp
    src/graphics/path.cpp
    src/layout/box_layout.cpp
    src/layout/grid_layout.cpp
    src/layout/layout.cpp
    src/layout/place_layout.cpp
    src/views/item_model.cpp
    src/views/list_model.cpp
    src/views/tree_model.cpp
    src/graphics/raster_canvas.cpp
    src/graphics/font_cache.cpp
    src/graphics/text_layout.cpp
    src/widgets/button.cpp
    src/widgets/button_base.cpp
    src/widgets/check_box.cpp
    src/widgets/combo_box.cpp
    src/windows/dialog.cpp
    src/windows/file_dialog.cpp
    src/actions/menu.cpp
    src/windows/menu_bar.cpp
    src/windows/message_dialog.cpp
    src/widgets/paint_box.cpp
    src/widgets/popup_list.cpp
    src/widgets/progress_bar.cpp
    src/widgets/radio_button.cpp
    src/widgets/range_base.cpp
    src/widgets/scroll_area.cpp
    src/widgets/scroll_bar.cpp
    src/widgets/slider.cpp
    src/widgets/spin_box.cpp
    src/widgets/splitter.cpp
    src/windows/status_bar.cpp
    src/widgets/tab_widget.cpp
    src/views/tree_view.cpp
    src/widgets/text_edit.cpp
    src/widgets/text_input_base.cpp
    src/windows/tool_bar.cpp
    src/views/item_view.cpp
    src/widgets/label.cpp
    src/widgets/line_edit.cpp)
add_library(tokotachi::tokotachi ALIAS tokotachi)
target_include_directories(tokotachi PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
# C++11(docs/adr/0003)。利用者が新しい規格でビルドしてもよいよう、公開のヘッダは C++20 でも通るように書く。
target_compile_features(tokotachi PUBLIC cxx_std_11)
set_target_properties(tokotachi PROPERTIES CXX_EXTENSIONS OFF)
target_compile_options(tokotachi PRIVATE -Wall -Wextra -pedantic)
target_link_libraries(tokotachi PUBLIC Threads::Threads)
target_link_libraries(tokotachi PRIVATE SDL3::SDL3-static plutovg::plutovg harfbuzz::harfbuzz Freetype::Freetype unibreak::unibreak SheenBidi::SheenBidi)

# MinGW のランタイムも静的にリンクし、exe が OS の DLL だけに依存するようにする(docs/adr/0004)。
if(MINGW AND TOKOTACHI_STATIC_RUNTIME)
    target_link_options(tokotachi INTERFACE -static)
endif()

if(UNIX AND NOT APPLE)
    # 静的にリンクした FreeType 等のシンボルを exe から公開しない。公開すると、システムの libfontconfig が
    # システムの libfreetype ではなくこちらの FreeType を呼ぶ(シンボルの横取り。docs/adr/0005)。
    target_link_options(tokotachi INTERFACE -Wl,--exclude-libs,ALL)
    # フォントの探索は fontconfig(OS の一部なのでシステムのもの)。
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FONTCONFIG REQUIRED IMPORTED_TARGET fontconfig)
    target_link_libraries(tokotachi PRIVATE PkgConfig::FONTCONFIG)
    target_compile_definitions(tokotachi PRIVATE TOKOTACHI_USE_FONTCONFIG)
endif()

# サンプル。
add_executable(hello examples/hello/main.cpp)
target_link_libraries(hello PRIVATE tokotachi::tokotachi)
target_compile_options(hello PRIVATE -Wall -Wextra -pedantic)
set_target_properties(hello PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

# テスト。画面なしで動くよう、SDL の offscreen のビデオドライバを使う。
enable_testing()
add_executable(tokotachi_tests tests/test_main.cpp)
# 内部の状態を確かめるテストのために、SDL3 のヘッダも読めるようにする(リンクは tokotachi が持つ)。
target_link_libraries(tokotachi_tests PRIVATE tokotachi::tokotachi SDL3::Headers)
target_compile_options(tokotachi_tests PRIVATE -Wall -Wextra -pedantic)
set_target_properties(tokotachi_tests PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
add_test(NAME tokotachi_tests COMMAND tokotachi_tests)
set_tests_properties(tokotachi_tests PROPERTIES ENVIRONMENT "SDL_VIDEO_DRIVER=offscreen")

# 公開のヘッダが C++20 でもコンパイルできることを確かめる(docs/adr/0003)。
add_library(tokotachi_header_check_cxx20 OBJECT tests/header_check.cpp)
target_include_directories(tokotachi_header_check_cxx20 PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_compile_options(tokotachi_header_check_cxx20 PRIVATE -Wall -Wextra -pedantic)
set_target_properties(tokotachi_header_check_cxx20 PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

# 公開のヘッダが、それぞれ単独で(他のヘッダを先に読まなくても)コンパイルできることを確かめる(docs/adr/0035)。
file(GLOB_RECURSE TOKOTACHI_PUBLIC_HEADERS RELATIVE ${CMAKE_CURRENT_LIST_DIR}/include ${CMAKE_CURRENT_LIST_DIR}/include/tokotachi/*.hpp)
set(TOKOTACHI_SELF_CONTAINED_SOURCES)
foreach(_header ${TOKOTACHI_PUBLIC_HEADERS})
    string(MAKE_C_IDENTIFIER ${_header} _name)
    set(_source ${CMAKE_CURRENT_BINARY_DIR}/header_check/${_name}.cpp)
    file(GENERATE OUTPUT ${_source} CONTENT "#include <${_header}>\n")
    list(APPEND TOKOTACHI_SELF_CONTAINED_SOURCES ${_source})
endforeach()
add_library(tokotachi_header_check_self_contained OBJECT ${TOKOTACHI_SELF_CONTAINED_SOURCES})
target_include_directories(tokotachi_header_check_self_contained PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_compile_options(tokotachi_header_check_self_contained PRIVATE -Wall -Wextra -pedantic)
set_target_properties(tokotachi_header_check_self_contained PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

# 技術検証(spike/)。ライブラリには引き継がない。
add_subdirectory(spike/m1)
