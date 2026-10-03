# マイルストーン1の検証: SDL3 のウィンドウ + plutovg の描画 + FreeType/HarfBuzz の日本語テキスト + IME 入力。
# 依存ライブラリはルートの CMakeLists.txt で見つけたもの(deps/ でソースからビルドした静的ライブラリ)を使う。

add_executable(spike_m1 main.cpp font.cpp)
set_target_properties(spike_m1 PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)
target_compile_options(spike_m1 PRIVATE -Wall -Wextra -pedantic)
target_link_libraries(spike_m1 PRIVATE
    SDL3::SDL3-static
    plutovg::plutovg
    harfbuzz::harfbuzz
    Freetype::Freetype)

# MinGW のランタイム(libgcc・libstdc++・winpthread)も静的にリンクし、exe 単体で動くようにする。
if(MINGW)
    target_link_options(spike_m1 PRIVATE -static)
endif()

# Linux ではシステムのフォントを fontconfig で探す(OS の一部なのでシステムのものを使う)。
# Windows は Fonts フォルダの既知のファイルを使う。
if(UNIX AND NOT APPLE)
    # 静的にリンクした FreeType 等のシンボルを exe から公開しない。公開すると、システムの libfontconfig が
    # システムの libfreetype ではなくこちらの FreeType を呼ぶ(シンボルの横取り)。
    target_link_options(spike_m1 PRIVATE -Wl,--exclude-libs,ALL)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FONTCONFIG REQUIRED IMPORTED_TARGET fontconfig)
    target_link_libraries(spike_m1 PRIVATE PkgConfig::FONTCONFIG)
    target_compile_definitions(spike_m1 PRIVATE SPIKE_USE_FONTCONFIG)
endif()
