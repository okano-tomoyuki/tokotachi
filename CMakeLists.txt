cmake_minimum_required(VERSION 3.16)
project(gui LANGUAGES C CXX)

# 依存ライブラリは deps/ でソースからビルドしたもの(deps/install/<プラットフォーム>/)だけを使う(docs/adr/0005)。
# MSYS2 などのシステムに同じライブラリが入っていても混ざらないよう、find_package は既定の探索先を使わない。
include(${CMAKE_CURRENT_LIST_DIR}/cmake/GuiPlatform.cmake)
set(GUI_DEPS_PREFIX "${CMAKE_CURRENT_LIST_DIR}/deps/install/${GUI_PLATFORM}" CACHE PATH
    "Install prefix of the dependencies built by deps/CMakeLists.txt")
if(NOT EXISTS "${GUI_DEPS_PREFIX}/lib/cmake")
    message(FATAL_ERROR "Dependencies not found in ${GUI_DEPS_PREFIX}. Build them first:\n"
                        "  cmake -S deps -B build-deps -G Ninja\n"
                        "  cmake --build build-deps")
endif()

find_package(Threads REQUIRED)
find_package(SDL3 REQUIRED CONFIG COMPONENTS SDL3-static PATHS "${GUI_DEPS_PREFIX}" NO_DEFAULT_PATH)
find_package(freetype REQUIRED CONFIG PATHS "${GUI_DEPS_PREFIX}" NO_DEFAULT_PATH)
find_package(harfbuzz REQUIRED CONFIG PATHS "${GUI_DEPS_PREFIX}" NO_DEFAULT_PATH)
find_package(plutovg REQUIRED CONFIG PATHS "${GUI_DEPS_PREFIX}" NO_DEFAULT_PATH)
find_package(unibreak REQUIRED CONFIG PATHS "${GUI_DEPS_PREFIX}" NO_DEFAULT_PATH)
find_package(SheenBidi REQUIRED CONFIG PATHS "${GUI_DEPS_PREFIX}" NO_DEFAULT_PATH)

option(GUI_STATIC_RUNTIME "Link the MinGW runtime (libgcc, libstdc++, winpthread) statically into executables" ON)

add_library(gui STATIC
    src/core/action.cpp
    src/core/application.cpp
    src/core/binding.cpp
    src/core/object.cpp
    src/core/popup.cpp
    src/core/shortcut.cpp
    src/core/clipboard.cpp
    src/core/theme.cpp
    src/core/timer.cpp
    src/core/widget.cpp
    src/core/window.cpp
    src/gfx/image.cpp
    src/gfx/path.cpp
    src/layout/box_layout.cpp
    src/layout/grid_layout.cpp
    src/layout/layout.cpp
    src/layout/place_layout.cpp
    src/models/item_model.cpp
    src/models/list_model.cpp
    src/models/tree_model.cpp
    src/gfx/raster_canvas.cpp
    src/text/font_cache.cpp
    src/text/text_layout.cpp
    src/widgets/button.cpp
    src/widgets/button_base.cpp
    src/widgets/check_box.cpp
    src/widgets/combo_box.cpp
    src/widgets/menu.cpp
    src/widgets/menu_bar.cpp
    src/widgets/popup_list.cpp
    src/widgets/progress_bar.cpp
    src/widgets/radio_button.cpp
    src/widgets/range_base.cpp
    src/widgets/scroll_area.cpp
    src/widgets/scroll_bar.cpp
    src/widgets/slider.cpp
    src/widgets/spin_box.cpp
    src/widgets/tab_widget.cpp
    src/widgets/tree_view.cpp
    src/widgets/text_edit.cpp
    src/widgets/text_input_base.cpp
    src/widgets/item_view.cpp
    src/widgets/label.cpp
    src/widgets/line_edit.cpp)
add_library(gui::gui ALIAS gui)
target_include_directories(gui PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
# C++11(docs/adr/0003)。利用者が新しい規格でビルドしてもよいよう、公開のヘッダは C++20 でも通るように書く。
target_compile_features(gui PUBLIC cxx_std_11)
set_target_properties(gui PROPERTIES CXX_EXTENSIONS OFF)
target_compile_options(gui PRIVATE -Wall -Wextra -pedantic)
target_link_libraries(gui PRIVATE SDL3::SDL3-static plutovg::plutovg harfbuzz::harfbuzz Freetype::Freetype unibreak::unibreak SheenBidi::SheenBidi)

# MinGW のランタイムも静的にリンクし、exe が OS の DLL だけに依存するようにする(docs/adr/0004)。
if(MINGW AND GUI_STATIC_RUNTIME)
    target_link_options(gui INTERFACE -static)
endif()

if(UNIX AND NOT APPLE)
    # 静的にリンクした FreeType 等のシンボルを exe から公開しない。公開すると、システムの libfontconfig が
    # システムの libfreetype ではなくこちらの FreeType を呼ぶ(シンボルの横取り。docs/adr/0005)。
    target_link_options(gui INTERFACE -Wl,--exclude-libs,ALL)
    # フォントの探索は fontconfig(OS の一部なのでシステムのもの)。
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FONTCONFIG REQUIRED IMPORTED_TARGET fontconfig)
    target_link_libraries(gui PRIVATE PkgConfig::FONTCONFIG)
    target_compile_definitions(gui PRIVATE GUI_USE_FONTCONFIG)
endif()

# サンプル。
add_executable(hello examples/hello/main.cpp)
target_link_libraries(hello PRIVATE gui::gui)
target_compile_options(hello PRIVATE -Wall -Wextra -pedantic)
set_target_properties(hello PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

# テスト。画面なしで動くよう、SDL の offscreen のビデオドライバを使う。
enable_testing()
add_executable(gui_tests tests/test_main.cpp)
# 内部の状態を確かめるテストのために、SDL3 のヘッダも読めるようにする(リンクは gui が持つ)。
target_link_libraries(gui_tests PRIVATE gui::gui SDL3::Headers)
target_compile_options(gui_tests PRIVATE -Wall -Wextra -pedantic)
set_target_properties(gui_tests PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
add_test(NAME gui_tests COMMAND gui_tests)
set_tests_properties(gui_tests PROPERTIES ENVIRONMENT "SDL_VIDEO_DRIVER=offscreen")

# 公開のヘッダが C++20 でもコンパイルできることを確かめる(docs/adr/0003)。
add_library(gui_header_check_cxx20 OBJECT tests/header_check.cpp)
target_include_directories(gui_header_check_cxx20 PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_compile_options(gui_header_check_cxx20 PRIVATE -Wall -Wextra -pedantic)
set_target_properties(gui_header_check_cxx20 PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

# 技術検証(spike/)。ライブラリには引き継がない。
add_subdirectory(spike/m1)
