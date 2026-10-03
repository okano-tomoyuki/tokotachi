# libunibreak(zlib のライセンス)を静的ライブラリとしてビルドする(docs/adr/0005、0007)。
# 本家は autotools でしかビルドできないので、deps/CMakeLists.txt がこのファイルをソースのフォルダに写して使う。
# 本家のソースのファイルの一覧が変わったら、ここも直す。
cmake_minimum_required(VERSION 3.16)
project(unibreak VERSION 8.0 LANGUAGES C)

# 本家の src/Makefile.am の libunibreak_la_SOURCES と同じ。*data.c の多く(linebreakdata.c 以外)は、
# 他の .c が #include して使うので、ここには入れない(入れると二重に定義される)。
add_library(unibreak STATIC
    src/eastasianwidthdef.c
    src/emojidef.c
    src/graphemebreak.c
    src/linebreak.c
    src/linebreakdata.c
    src/linebreakdef.c
    src/unibreakbase.c
    src/unibreakdef.c
    src/wordbreak.c)
target_include_directories(unibreak PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include/unibreak>)

include(GNUInstallDirs)
install(TARGETS unibreak EXPORT unibreakTargets ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES src/linebreak.h src/wordbreak.h src/graphemebreak.h src/unibreakbase.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/unibreak)
install(EXPORT unibreakTargets NAMESPACE unibreak:: FILE unibreakConfig.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/unibreak)
