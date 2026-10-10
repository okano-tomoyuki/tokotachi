# 依存ライブラリをソースからビルドして、静的ライブラリとして deps/install/<プラットフォーム>/ にインストールする。
# 本体とは別に、初回と依存ライブラリの更新時だけ実行する:
#
#   cmake -S deps -B build-deps -G Ninja
#   cmake --build build-deps
#
# ソースのアーカイブは deps/download/ にキャッシュする(バージョンは URL と SHA256 で固定)。
# OS の一部である Linux のライブラリ(wayland-client、libxkbcommon、libdecor、X11、fontconfig、D-Bus)は
# システムのものを使う(SDL3 は実行時に dlopen するので、ビルドに必要なのはヘッダだけ)。
cmake_minimum_required(VERSION 3.16)
project(tokotachi_deps C CXX)

include(ExternalProject)
include(${CMAKE_CURRENT_LIST_DIR}/../cmake/TokotachiPlatform.cmake)

set(TOKOTACHI_DEPS_PREFIX "${CMAKE_CURRENT_LIST_DIR}/install/${TOKOTACHI_PLATFORM}" CACHE PATH "Install prefix of the dependencies")
set(TOKOTACHI_DEPS_DOWNLOAD_DIR "${CMAKE_CURRENT_LIST_DIR}/download")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# すべての依存ライブラリに共通の設定。コンパイラはこのプロジェクトと同じものを使う。
# CMAKE_PREFIX_PATH で、先にインストールした依存ライブラリ(HarfBuzz から見た FreeType)を MSYS2 などのシステムのものより優先する。
set(TOKOTACHI_DEPS_COMMON_ARGS
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${TOKOTACHI_DEPS_PREFIX}
    -DCMAKE_PREFIX_PATH=${TOKOTACHI_DEPS_PREFIX}
    -DCMAKE_INSTALL_LIBDIR=lib
    -DCMAKE_INSTALL_INCLUDEDIR=include  # HarfBuzz の CMake は未指定だと設定ファイルのインクルードのパスが空になる
    -DCMAKE_INSTALL_BINDIR=bin
    -DBUILD_SHARED_LIBS=OFF
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON)
if(CMAKE_TOOLCHAIN_FILE)
    list(APPEND TOKOTACHI_DEPS_COMMON_ARGS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})
endif()
# Emscripten のツールチェーンは、find_package を自分の sysroot の中だけで探すように制限する。インストール先も探す対象に加える。
if(EMSCRIPTEN)
    list(APPEND TOKOTACHI_DEPS_COMMON_ARGS -DCMAKE_FIND_ROOT_PATH=${TOKOTACHI_DEPS_PREFIX})
endif()

# Linux: SDL3 は D-Bus・IBus(IME)や libdecor(Wayland のウィンドウの装飾)が無くてもエラーにせず、黙って無効にする。
# IME が使えないビルドを作らないよう、ここで確かめる。
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(PkgConfig REQUIRED)
    set(_gui_missing)
    foreach(_mod wayland-client wayland-egl xkbcommon libdecor-0 dbus-1 ibus-1.0 x11 xext xrandr xcursor xfixes xi xscrnsaver xtst egl fontconfig)
        pkg_check_modules(_gui_chk_${_mod} QUIET ${_mod})
        if(NOT _gui_chk_${_mod}_FOUND)
            list(APPEND _gui_missing ${_mod})
        endif()
    endforeach()
    if(_gui_missing)
        message(FATAL_ERROR "Missing development packages (pkg-config): ${_gui_missing}\n"
            "On Debian/Ubuntu:\n"
            "  sudo apt install build-essential cmake ninja-build pkg-config git \\\n"
            "    libwayland-dev libxkbcommon-dev libdecor-0-dev libegl-dev \\\n"
            "    libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev libxss-dev libxtst-dev \\\n"
            "    libdbus-1-dev libibus-1.0-dev libudev-dev libdrm-dev libgbm-dev libfontconfig-dev")
    endif()
endif()

ExternalProject_Add(sdl3
    URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.18/SDL3-3.4.18.tar.gz
    URL_HASH SHA256=9c75cf16330322c217dedd2e0609f1124f1b54b8633e763467b4684d0f4334a3
    DOWNLOAD_DIR ${TOKOTACHI_DEPS_DOWNLOAD_DIR}
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    CMAKE_ARGS ${TOKOTACHI_DEPS_COMMON_ARGS}
        -DSDL_SHARED=OFF
        -DSDL_STATIC=ON
        -DSDL_TEST_LIBRARY=OFF
        -DSDL_TESTS=OFF
        -DSDL_EXAMPLES=OFF)

# HarfBuzz と相互に依存するので、FreeType は HarfBuzz なしでビルドする(HarfBuzz の側から FreeType を使う)。
# 圧縮されたフォントの展開には FreeType 内蔵の zlib を使い、外部のライブラリには依存しない。
ExternalProject_Add(freetype
    URL https://github.com/freetype/freetype/archive/refs/tags/VER-2-14-3.tar.gz
    URL_HASH SHA256=dc49de6b01a266eef4876a4dd34d9842c475d3e28ff2eff63bd2fb760ab56261
    DOWNLOAD_NAME freetype-VER-2-14-3.tar.gz
    DOWNLOAD_DIR ${TOKOTACHI_DEPS_DOWNLOAD_DIR}
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    CMAKE_ARGS ${TOKOTACHI_DEPS_COMMON_ARGS}
        -DFT_DISABLE_HARFBUZZ=ON
        -DFT_DISABLE_ZLIB=ON
        -DFT_DISABLE_BZIP2=ON
        -DFT_DISABLE_PNG=ON
        -DFT_DISABLE_BROTLI=ON)

ExternalProject_Add(harfbuzz
    DEPENDS freetype
    URL https://github.com/harfbuzz/harfbuzz/releases/download/14.5.1/harfbuzz-14.5.1.tar.xz
    URL_HASH SHA256=7e2fa4e8c7c98e8d8140671f5772542afaaa6acccfbd746506886b6d85f7f8d6
    DOWNLOAD_DIR ${TOKOTACHI_DEPS_DOWNLOAD_DIR}
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    CMAKE_ARGS ${TOKOTACHI_DEPS_COMMON_ARGS}
        -DHB_HAVE_FREETYPE=ON
        -DHB_HAVE_GLIB=OFF
        -DHB_HAVE_ICU=OFF
        -DHB_HAVE_GOBJECT=OFF
        -DHB_BUILD_UTILS=OFF
        -DHB_BUILD_SUBSET=OFF
        -DHB_BUILD_RASTER=OFF
        -DHB_BUILD_VECTOR=OFF
        -DHB_BUILD_GPU=OFF
        -DHB_BUILD_TESTS=OFF)

ExternalProject_Add(plutovg
    URL https://github.com/sammycage/plutovg/archive/refs/tags/v1.3.3.tar.gz
    URL_HASH SHA256=2b0d17a6e016f47b86f9c00e2cb82600041b1ea1f7d2a00c2d46ae542cbfed3c
    DOWNLOAD_NAME plutovg-1.3.3.tar.gz
    DOWNLOAD_DIR ${TOKOTACHI_DEPS_DOWNLOAD_DIR}
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    CMAKE_ARGS ${TOKOTACHI_DEPS_COMMON_ARGS}
        -DPLUTOVG_BUILD_EXAMPLES=OFF)

# 改行・単語・書記素の境界(UAX #14・#29)。autotools でしかビルドできないので、自前の CMakeLists.txt を写して使う。
ExternalProject_Add(libunibreak
    URL https://github.com/adah1972/libunibreak/releases/download/libunibreak_8_0/libunibreak-8.0.tar.gz
    URL_HASH SHA256=9c4fad6e517338a098373acc9f35579ae2c325e6446666fb9ac2666ba15ceba4
    DOWNLOAD_DIR ${TOKOTACHI_DEPS_DOWNLOAD_DIR}
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/libunibreak/CMakeLists.txt <SOURCE_DIR>/CMakeLists.txt
    CMAKE_ARGS ${TOKOTACHI_DEPS_COMMON_ARGS})

# 双方向テキスト(UAX #9)。内部のビルドには C++14 が要るが、公開の API は C。
ExternalProject_Add(sheenbidi
    URL https://github.com/Tehreer/SheenBidi/archive/refs/tags/v3.0.0.tar.gz
    URL_HASH SHA256=86c56014034739ba39a24c23eb00323b0bf6f737354f665786015fca842af786
    DOWNLOAD_NAME SheenBidi-v3.0.0.tar.gz
    DOWNLOAD_DIR ${TOKOTACHI_DEPS_DOWNLOAD_DIR}
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    CMAKE_ARGS ${TOKOTACHI_DEPS_COMMON_ARGS}
        -DSB_CONFIG_UNITY=ON
        -DBUILD_TESTING=OFF)

message(STATUS "Dependencies will be installed to ${TOKOTACHI_DEPS_PREFIX}")
