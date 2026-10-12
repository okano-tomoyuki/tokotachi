cmake_minimum_required(VERSION 3.16)

# 版は include/tokotachi/core/version.hpp だけで管理する(docs/adr/0037)。
file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/include/tokotachi/core/version.hpp" _tokotachi_version_lines
     REGEX "^#define TOKOTACHI_VERSION_(MAJOR|MINOR|PATCH) ")
foreach(_line ${_tokotachi_version_lines})
    string(REGEX REPLACE "^#define TOKOTACHI_VERSION_([A-Z]+) ([0-9]+).*$" "\\1;\\2" _pair "${_line}")
    list(GET _pair 0 _key)
    list(GET _pair 1 _value)
    set(_tokotachi_version_${_key} ${_value})
endforeach()
project(tokotachi
        VERSION ${_tokotachi_version_MAJOR}.${_tokotachi_version_MINOR}.${_tokotachi_version_PATCH}
        DESCRIPTION "Cross-platform C++11 GUI library built on SDL3"
        LANGUAGES C CXX)

# 利用者のプロジェクトの中(add_subdirectory・FetchContent)で使われているときは、サンプル・テストを作らない。
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(TOKOTACHI_TOP_LEVEL ON)
else()
    set(TOKOTACHI_TOP_LEVEL OFF)
endif()

option(TOKOTACHI_SHARED "Build tokotachi as a shared library (DLL / .so). OFF builds a static library that contains the dependencies" ON)
option(TOKOTACHI_BUILD_EXAMPLES "Build the examples" ${TOKOTACHI_TOP_LEVEL})
option(TOKOTACHI_BUILD_TESTS "Build the tests" ${TOKOTACHI_TOP_LEVEL})
option(TOKOTACHI_INSTALL "Generate the install rules" ${TOKOTACHI_TOP_LEVEL})
option(TOKOTACHI_STATIC_RUNTIME "Static library build on MinGW: link the MinGW runtime statically into executables" ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# ---- 依存ライブラリ ----
# 依存ライブラリは deps/ でソースからビルドしたもの(<TOKOTACHI_DEPS_PREFIX>)だけを使う(docs/adr/0005)。
# MSYS2 などのシステムに同じライブラリが入っていても混ざらないよう、find_package は既定の探索先を使わない。
include(${CMAKE_CURRENT_LIST_DIR}/cmake/TokotachiPlatform.cmake)
if(TOKOTACHI_TOP_LEVEL)
    set(_tokotachi_default_deps "${CMAKE_CURRENT_LIST_DIR}/deps/install/${TOKOTACHI_PLATFORM}")
else()
    set(_tokotachi_default_deps "${CMAKE_CURRENT_BINARY_DIR}/tokotachi-deps/install")
endif()
set(TOKOTACHI_DEPS_PREFIX "${_tokotachi_default_deps}" CACHE PATH
    "Install prefix of the dependencies built by deps/CMakeLists.txt")
option(TOKOTACHI_BUILD_DEPS "Build the dependencies automatically when they are not found" ON)
if(NOT EXISTS "${TOKOTACHI_DEPS_PREFIX}/lib/cmake")
    if(NOT TOKOTACHI_BUILD_DEPS)
        message(FATAL_ERROR "Dependencies not found in ${TOKOTACHI_DEPS_PREFIX}. Build them first:\n"
                            "  cmake -S deps -B build-deps -G Ninja\n"
                            "  cmake --build build-deps")
    endif()
    # 初めての設定のときだけ、依存ライブラリをソースからビルドする(数分かかる)。
    message(STATUS "tokotachi: building the dependencies into ${TOKOTACHI_DEPS_PREFIX} (first time only)")
    set(_deps_build "${CMAKE_CURRENT_BINARY_DIR}/tokotachi-deps/build")
    set(_deps_args -S "${CMAKE_CURRENT_LIST_DIR}/deps" -B "${_deps_build}" -G "${CMAKE_GENERATOR}"
                   "-DTOKOTACHI_DEPS_PREFIX=${TOKOTACHI_DEPS_PREFIX}"
                   "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}" "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
    if(CMAKE_TOOLCHAIN_FILE)
        list(APPEND _deps_args "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} ${_deps_args} RESULT_VARIABLE _result)
    if(_result EQUAL 0)
        execute_process(COMMAND ${CMAKE_COMMAND} --build "${_deps_build}" RESULT_VARIABLE _result)
    endif()
    if(NOT _result EQUAL 0)
        message(FATAL_ERROR "tokotachi: building the dependencies failed (${_result})")
    endif()
endif()

if(UNIX AND NOT APPLE AND NOT EMSCRIPTEN)
    set(TOKOTACHI_LINUX ON)
else()
    set(TOKOTACHI_LINUX OFF)
endif()
# Emscripten(WebAssembly。デザイナーのキャンバス、docs/adr/0039)のツールチェーンは、find_package を自分の sysroot の中だけで探す。
if(EMSCRIPTEN)
    list(APPEND CMAKE_FIND_ROOT_PATH "${TOKOTACHI_DEPS_PREFIX}")
endif()

find_package(Threads REQUIRED)
find_package(SDL3 REQUIRED CONFIG COMPONENTS SDL3-static PATHS "${TOKOTACHI_DEPS_PREFIX}" NO_DEFAULT_PATH)
find_package(freetype REQUIRED CONFIG PATHS "${TOKOTACHI_DEPS_PREFIX}" NO_DEFAULT_PATH)
find_package(harfbuzz REQUIRED CONFIG PATHS "${TOKOTACHI_DEPS_PREFIX}" NO_DEFAULT_PATH)
find_package(plutovg REQUIRED CONFIG PATHS "${TOKOTACHI_DEPS_PREFIX}" NO_DEFAULT_PATH)
find_package(unibreak REQUIRED CONFIG PATHS "${TOKOTACHI_DEPS_PREFIX}" NO_DEFAULT_PATH)
find_package(SheenBidi REQUIRED CONFIG PATHS "${TOKOTACHI_DEPS_PREFIX}" NO_DEFAULT_PATH)
# 静的にまとめるときに使う、依存ライブラリの実体(静的ライブラリ)の target。
set(_tokotachi_dep_archives SDL3::SDL3-static plutovg::plutovg harfbuzz::harfbuzz freetype unibreak::unibreak
                            SheenBidi::SheenBidi)

# ---- ライブラリ ----
if(TOKOTACHI_SHARED)
    set(_tokotachi_type SHARED)
else()
    set(_tokotachi_type STATIC)
endif()
# ソースは一度だけコンパイルし、配布するライブラリ(tokotachi)と、テストのための内部のライブラリの両方に使う。
add_library(tokotachi_objects OBJECT
    src/actions/action.cpp
    src/actions/menu.cpp
    src/actions/shortcut.cpp
    src/core/application.cpp
    src/core/binding.cpp
    src/core/clipboard.cpp
    src/core/date.cpp
    src/core/locale.cpp
    src/core/object.cpp
    src/core/task.cpp
    src/core/timer.cpp
    src/core/version.cpp
    src/graphics/bitmap.cpp
    src/graphics/canvas.cpp
    src/graphics/font_cache.cpp
    src/graphics/icon.cpp
    src/graphics/icon_data.cpp
    src/graphics/image.cpp
    src/graphics/path.cpp
    src/graphics/raster_canvas.cpp
    src/graphics/text_layout.cpp
    src/graphics/theme.cpp
    src/layout/box_layout.cpp
    src/layout/grid_layout.cpp
    src/layout/layout.cpp
    src/layout/place_layout.cpp
    src/views/item_model.cpp
    src/views/item_view.cpp
    src/views/list_model.cpp
    src/views/table_view.cpp
    src/views/tree_model.cpp
    src/views/tree_view.cpp
    src/widgets/button.cpp
    src/widgets/button_base.cpp
    src/widgets/check_box.cpp
    src/widgets/calendar.cpp
    src/widgets/color_button.cpp
    src/widgets/combo_box.cpp
    src/widgets/date_edit.cpp
    src/widgets/group_box.cpp
    src/widgets/image_box.cpp
    src/widgets/label.cpp
    src/widgets/line_edit.cpp
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
    src/widgets/tab_widget.cpp
    src/widgets/text_edit.cpp
    src/widgets/text_input_base.cpp
    src/widgets/widget.cpp
    src/windows/color_dialog.cpp
    src/windows/dialog.cpp
    src/windows/file_dialog.cpp
    src/windows/menu_bar.cpp
    src/windows/message_dialog.cpp
    src/windows/popup.cpp
    src/windows/status_bar.cpp
    src/windows/tool_bar.cpp
    src/windows/window.cpp)
target_include_directories(tokotachi_objects PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
# C++11(docs/adr/0003)。利用者が新しい規格でビルドしてもよいよう、公開のヘッダは C++20 でも通るように書く。
target_compile_features(tokotachi_objects PUBLIC cxx_std_11)
set_target_properties(tokotachi_objects PROPERTIES CXX_EXTENSIONS OFF POSITION_INDEPENDENT_CODE ON)
target_compile_options(tokotachi_objects PRIVATE -Wall -Wextra -pedantic)
target_link_libraries(tokotachi_objects PRIVATE Threads::Threads SDL3::SDL3-static plutovg::plutovg harfbuzz::harfbuzz
                                                Freetype::Freetype unibreak::unibreak SheenBidi::SheenBidi)

add_library(tokotachi ${_tokotachi_type} $<TARGET_OBJECTS:tokotachi_objects>)
add_library(tokotachi::tokotachi ALIAS tokotachi)
target_include_directories(tokotachi PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
                                            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_features(tokotachi PUBLIC cxx_std_11)
# 0.x の間は、小さい版(minor)が変わると ABI も変わりうる。
if(PROJECT_VERSION_MAJOR EQUAL 0)
    set(_tokotachi_soversion ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
else()
    set(_tokotachi_soversion ${PROJECT_VERSION_MAJOR})
endif()
set_target_properties(tokotachi PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${_tokotachi_soversion})
target_link_libraries(tokotachi PUBLIC Threads::Threads)
# 依存ライブラリはライブラリの中に入れる(共有ならリンクして中に入れ、静的ならまとめる)。利用者には見せない。
target_link_libraries(tokotachi PRIVATE $<BUILD_INTERFACE:SDL3::SDL3-static> $<BUILD_INTERFACE:plutovg::plutovg>
                                        $<BUILD_INTERFACE:harfbuzz::harfbuzz> $<BUILD_INTERFACE:Freetype::Freetype>
                                        $<BUILD_INTERFACE:unibreak::unibreak> $<BUILD_INTERFACE:SheenBidi::SheenBidi>)

# pkg-config の .pc に書く、静的にリンクするときに要る OS のライブラリ。
set(TOKOTACHI_PC_REQUIRES_PRIVATE "")
set(TOKOTACHI_PC_LIBS_PRIVATE "")
if(WIN32)
    set(_tokotachi_system_libs kernel32 user32 gdi32 winmm imm32 ole32 oleaut32 version uuid advapi32 setupapi shell32
                               dinput8)
else()
    set(_tokotachi_system_libs m dl)
endif()

if(TOKOTACHI_LINUX)
    # フォントの探索は fontconfig(OS の一部なのでシステムのもの)。
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FONTCONFIG REQUIRED IMPORTED_TARGET fontconfig)
    target_link_libraries(tokotachi_objects PRIVATE PkgConfig::FONTCONFIG)
    target_link_libraries(tokotachi PRIVATE $<BUILD_INTERFACE:PkgConfig::FONTCONFIG>)
    target_compile_definitions(tokotachi_objects PRIVATE TOKOTACHI_USE_FONTCONFIG)
    set(TOKOTACHI_PC_REQUIRES_PRIVATE "fontconfig")
endif()

if(TOKOTACHI_SHARED)
    # 中に入れた静的ライブラリ(SDL3・FreeType など)のシンボルを公開しない。利用者が別の版の SDL3 を使っても衝突せず、
    # Linux ではシステムの libfontconfig がシステムの libfreetype ではなくこちらの FreeType を呼ぶこともない(docs/adr/0005)。
    # このライブラリ自身のシンボルは、Linux では既定の可視性で、MinGW では自動で、すべて公開する。
    target_link_options(tokotachi PRIVATE -Wl,--exclude-libs,ALL)
else()
    # 依存ライブラリの静的ライブラリを libtokotachi.a にまとめる。利用者は tokotachi と OS のライブラリだけをリンクする。
    set(_mri "CREATE $<TARGET_FILE_DIR:tokotachi>/tokotachi-merged.a\nADDLIB $<TARGET_FILE:tokotachi>\n")
    foreach(_dep ${_tokotachi_dep_archives})
        string(APPEND _mri "ADDLIB $<TARGET_FILE:${_dep}>\n")
    endforeach()
    string(APPEND _mri "SAVE\nEND\n")
    file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/tokotachi-merge-$<CONFIG>.mri" CONTENT "${_mri}")
    add_custom_command(TARGET tokotachi POST_BUILD
        COMMAND ${CMAKE_COMMAND} -DAR=${CMAKE_AR} -DSCRIPT=${CMAKE_CURRENT_BINARY_DIR}/tokotachi-merge-$<CONFIG>.mri
                -P ${CMAKE_CURRENT_LIST_DIR}/cmake/merge_static.cmake
        COMMAND ${CMAKE_COMMAND} -E rename $<TARGET_FILE_DIR:tokotachi>/tokotachi-merged.a $<TARGET_FILE:tokotachi>
        COMMENT "Merging the dependencies into libtokotachi.a")
    target_link_libraries(tokotachi INTERFACE $<INSTALL_INTERFACE:${_tokotachi_system_libs}>)
    if(TOKOTACHI_LINUX)
        target_link_libraries(tokotachi INTERFACE $<INSTALL_INTERFACE:fontconfig>)
        # 静的にリンクした FreeType 等のシンボルを exe から公開しない(上と同じ理由)。
        target_link_options(tokotachi INTERFACE -Wl,--exclude-libs,ALL)
    endif()
    if(MINGW AND TOKOTACHI_STATIC_RUNTIME)
        # MinGW のランタイムも静的にリンクし、exe が OS の DLL だけに依存するようにする(docs/adr/0004)。
        target_link_options(tokotachi INTERFACE -static)
    endif()
    foreach(_lib ${_tokotachi_system_libs})
        string(APPEND TOKOTACHI_PC_LIBS_PRIVATE " -l${_lib}")
    endforeach()
    if(TOKOTACHI_LINUX)
        string(APPEND TOKOTACHI_PC_LIBS_PRIVATE " -Wl,--exclude-libs,ALL")
    endif()
    string(STRIP "${TOKOTACHI_PC_LIBS_PRIVATE}" TOKOTACHI_PC_LIBS_PRIVATE)
endif()

# MinGW の共有ライブラリの版では、サンプル・テストの exe の隣に MinGW のランタイムの DLL を置く(MSYS2 の外からも動くように)。
function(tokotachi_copy_runtime target)
    if(MINGW AND TOKOTACHI_SHARED)
        get_filename_component(_bin "${CMAKE_CXX_COMPILER}" DIRECTORY)
        foreach(_dll libstdc++-6.dll libgcc_s_seh-1.dll libwinpthread-1.dll)
            if(EXISTS "${_bin}/${_dll}")
                add_custom_command(TARGET ${target} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_bin}/${_dll}" $<TARGET_FILE_DIR:${target}>)
            endif()
        endforeach()
    endif()
endfunction()

# ---- インストール(docs/adr/0037) ----
if(TOKOTACHI_INSTALL)
    install(TARGETS tokotachi EXPORT tokotachiTargets
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(DIRECTORY include/tokotachi DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(EXPORT tokotachiTargets NAMESPACE tokotachi:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tokotachi)
    configure_package_config_file(cmake/tokotachiConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/tokotachiConfig.cmake
                                  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tokotachi)
    # 0.x の間は、小さい版(minor)まで一致するものだけを互換とする。
    if(PROJECT_VERSION_MAJOR EQUAL 0)
        set(_tokotachi_compat SameMinorVersion)
    else()
        set(_tokotachi_compat SameMajorVersion)
    endif()
    write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/tokotachiConfigVersion.cmake
                                     COMPATIBILITY ${_tokotachi_compat})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tokotachiConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/tokotachiConfigVersion.cmake
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tokotachi)
    configure_file(cmake/tokotachi.pc.in ${CMAKE_CURRENT_BINARY_DIR}/tokotachi.pc @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tokotachi.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
    install(FILES LICENSE THIRD_PARTY_LICENSES.txt DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/tokotachi)
endif()

# ---- サンプル ----
if(TOKOTACHI_BUILD_EXAMPLES)
    add_executable(hello examples/hello/main.cpp)
    target_link_libraries(hello PRIVATE tokotachi::tokotachi)
    target_compile_options(hello PRIVATE -Wall -Wextra -pedantic)
    set_target_properties(hello PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
    tokotachi_copy_runtime(hello)

    # 部品ごとの小さなサンプル(リファレンスのサンプルの一覧。docs/guide/gallery.md)。
    set(TOKOTACHI_GALLERY buttons inputs pickers views table window dialog drawing drop images icons)
    foreach(_name ${TOKOTACHI_GALLERY})
        add_executable(gallery_${_name} examples/gallery/${_name}.cpp)
        target_link_libraries(gallery_${_name} PRIVATE tokotachi::tokotachi)
        target_compile_options(gallery_${_name} PRIVATE -Wall -Wextra -pedantic)
        set_target_properties(gallery_${_name} PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
        tokotachi_copy_runtime(gallery_${_name})
    endforeach()
endif()

# ---- リファレンス(GitHub Pages。docs/adr/0038) ----
option(TOKOTACHI_BUILD_DOCS "Add the 'docs' target that builds the reference with Doxygen" OFF)
if(TOKOTACHI_BUILD_DOCS)
    if(NOT TOKOTACHI_BUILD_EXAMPLES)
        message(FATAL_ERROR "TOKOTACHI_BUILD_DOCS needs TOKOTACHI_BUILD_EXAMPLES (the gallery screenshots)")
    endif()
    find_package(Doxygen REQUIRED OPTIONAL_COMPONENTS dot)
    # 見た目のテーマ(doxygen-awesome-css。MIT)。版と SHA256 を固定して取ってくる。
    set(TOKOTACHI_DOCS_THEME "${CMAKE_CURRENT_BINARY_DIR}/docs-theme")
    set(_theme_url "https://raw.githubusercontent.com/jothepro/doxygen-awesome-css/v2.5.0")
    foreach(_pair "doxygen-awesome.css;ea37516ec7a7a7780719b9d9a285fe3268a8d5411523d8b93af98d77e67cd1a1"
                  "doxygen-awesome-sidebar-only.css;909d17055552149555fc87c5e0cf171a41d09a9e358c389a51b25ddfafd462af")
        list(GET _pair 0 _file)
        list(GET _pair 1 _hash)
        if(NOT EXISTS "${TOKOTACHI_DOCS_THEME}/${_file}")
            file(DOWNLOAD "${_theme_url}/${_file}" "${TOKOTACHI_DOCS_THEME}/${_file}" EXPECTED_HASH SHA256=${_hash}
                 TLS_VERIFY ON)
        endif()
    endforeach()
    set(TOKOTACHI_DOCS_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/docs")
    set(TOKOTACHI_DOCS_IMAGES "${CMAKE_CURRENT_BINARY_DIR}/docs-images")
    if(DOXYGEN_DOT_FOUND)
        set(TOKOTACHI_DOCS_HAVE_DOT YES)
    else()
        set(TOKOTACHI_DOCS_HAVE_DOT NO)
    endif()
    configure_file(docs/Doxyfile.in "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile" @ONLY)
    # サンプルを画面なしで動かして、画面の画像を撮る。
    set(_shots)
    foreach(_name ${TOKOTACHI_GALLERY})
        set(_png "${TOKOTACHI_DOCS_IMAGES}/${_name}.png")
        add_custom_command(OUTPUT "${_png}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${TOKOTACHI_DOCS_IMAGES}"
            COMMAND ${CMAKE_COMMAND} -E env SDL_VIDEO_DRIVER=offscreen $<TARGET_FILE:gallery_${_name}> --screenshot "${_png}"
            DEPENDS gallery_${_name}
            COMMENT "Taking the screenshot of gallery_${_name}")
        list(APPEND _shots "${_png}")
    endforeach()
    # 色のダイアログ(gallery_pickers が開いて撮る)
    set(_png "${TOKOTACHI_DOCS_IMAGES}/color_dialog.png")
    add_custom_command(OUTPUT "${_png}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${TOKOTACHI_DOCS_IMAGES}"
        COMMAND ${CMAKE_COMMAND} -E env SDL_VIDEO_DRIVER=offscreen $<TARGET_FILE:gallery_pickers> --screenshot "${_png}" --color-dialog
        DEPENDS gallery_pickers
        COMMENT "Taking the screenshot of the color dialog")
    list(APPEND _shots "${_png}")
    file(GLOB_RECURSE _doc_inputs ${CMAKE_CURRENT_LIST_DIR}/docs/guide/*.md ${CMAKE_CURRENT_LIST_DIR}/include/tokotachi/*.hpp
         ${CMAKE_CURRENT_LIST_DIR}/examples/gallery/*.cpp)
    add_custom_target(docs
        COMMAND Doxygen::doxygen "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
        DEPENDS ${_shots} ${_doc_inputs} "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
        WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
        COMMENT "Building the reference into ${TOKOTACHI_DOCS_OUTPUT}/html")
endif()

# ---- テスト ----
if(TOKOTACHI_BUILD_TESTS)
    # 画面なしで動くよう、SDL の offscreen のビデオドライバを使う。
    enable_testing()
    # テストは内部の状態を確かめ、SDL3 の関数も直接呼ぶ。配布するライブラリ(共有なら SDL3 を中に隠す)ではなく、
    # 同じオブジェクトと依存ライブラリを静的にリンクした内部のライブラリを使う(ライブラリと同じ SDL3 を使うため)。
    add_library(tokotachi_internal STATIC $<TARGET_OBJECTS:tokotachi_objects>)
    target_include_directories(tokotachi_internal PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
    target_compile_features(tokotachi_internal PUBLIC cxx_std_11)
    target_link_libraries(tokotachi_internal PUBLIC Threads::Threads SDL3::SDL3-static plutovg::plutovg
                                                    harfbuzz::harfbuzz Freetype::Freetype unibreak::unibreak
                                                    SheenBidi::SheenBidi)
    if(TOKOTACHI_LINUX)
        target_link_libraries(tokotachi_internal PUBLIC PkgConfig::FONTCONFIG)
        target_link_options(tokotachi_internal INTERFACE -Wl,--exclude-libs,ALL)
    endif()
    if(MINGW AND TOKOTACHI_STATIC_RUNTIME)
        target_link_options(tokotachi_internal INTERFACE -static)
    endif()
    add_executable(tokotachi_tests tests/test_main.cpp)
    target_link_libraries(tokotachi_tests PRIVATE tokotachi_internal)
    target_compile_options(tokotachi_tests PRIVATE -Wall -Wextra -pedantic)
    set_target_properties(tokotachi_tests PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
    add_test(NAME tokotachi_tests COMMAND tokotachi_tests)
    set_tests_properties(tokotachi_tests PROPERTIES ENVIRONMENT "SDL_VIDEO_DRIVER=offscreen")

    # 公開のヘッダが C++20 でもコンパイルできることを確かめる(docs/adr/0003)。
    add_library(tokotachi_header_check_cxx20 OBJECT tests/header_check.cpp)
    target_include_directories(tokotachi_header_check_cxx20 PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
    target_compile_options(tokotachi_header_check_cxx20 PRIVATE -Wall -Wextra -pedantic)
    set_target_properties(tokotachi_header_check_cxx20 PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON
                                                                  CXX_EXTENSIONS OFF)

    # 公開のヘッダが、それぞれ単独で(他のヘッダを先に読まなくても)コンパイルできることを確かめる(docs/adr/0035)。
    file(GLOB_RECURSE TOKOTACHI_PUBLIC_HEADERS RELATIVE ${CMAKE_CURRENT_LIST_DIR}/include
         ${CMAKE_CURRENT_LIST_DIR}/include/tokotachi/*.hpp)
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
    set_target_properties(tokotachi_header_check_self_contained PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON
                                                                           CXX_EXTENSIONS OFF)
endif()

# 技術検証(spike/)。ライブラリには引き継がない。
if(TOKOTACHI_TOP_LEVEL)
    if(EMSCRIPTEN)
        add_subdirectory(spike/wasm)
    else()
        add_subdirectory(spike/m1)
    endif()
endif()
