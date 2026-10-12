# C API(docs/adr/0041)。ライブラリの C++ のオブジェクトと依存ライブラリを中に入れた、単独の共有ライブラリ tokotachi_c を作る。
# 公開するのは tokotachi_* の C の関数だけ(C++ の ABI に依らないので、Python などのバインディングから読み込める)。
#
# ヘッダ(include/tokotachi/tokotachi.h)と src/generated.cpp は api.py から generate.py で作る(手で書き換えない)。

add_library(tokotachi_c SHARED
    src/common.cpp
    src/generated.cpp
    src/manual.cpp
    $<TARGET_OBJECTS:tokotachi_objects>)
add_library(tokotachi::tokotachi_c ALIAS tokotachi_c)
target_include_directories(tokotachi_c PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
                                              $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_include_directories(tokotachi_c PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_compile_features(tokotachi_c PRIVATE cxx_std_11)
target_compile_options(tokotachi_c PRIVATE -Wall -Wextra -pedantic)
# C API の実装の関数は TOKOTACHI_C_API の付いたものだけを公開する。
set_target_properties(tokotachi_c PROPERTIES CXX_EXTENSIONS OFF CXX_VISIBILITY_PRESET hidden
                                             VISIBILITY_INLINES_HIDDEN ON
                                             VERSION ${PROJECT_VERSION} SOVERSION ${_tokotachi_soversion})
target_link_libraries(tokotachi_c PRIVATE Threads::Threads SDL3::SDL3-static plutovg::plutovg harfbuzz::harfbuzz
                                          Freetype::Freetype unibreak::unibreak SheenBidi::SheenBidi)
if(TOKOTACHI_LINUX)
    target_link_libraries(tokotachi_c PRIVATE PkgConfig::FONTCONFIG)
    # 中に入れた C++ のライブラリ(tokotachi_objects は既定の可視性)と依存ライブラリのシンボルを隠す。
    target_link_options(tokotachi_c PRIVATE "-Wl,--version-script=${CMAKE_CURRENT_LIST_DIR}/tokotachi_c.map"
                                            -Wl,--no-undefined)
    set_property(TARGET tokotachi_c APPEND PROPERTY LINK_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/tokotachi_c.map")
endif()
if(MINGW)
    # Python などから読み込む DLL が、MinGW のランタイムの DLL に依存しないようにする。
    target_link_options(tokotachi_c PRIVATE -static-libgcc -static-libstdc++ -Wl,-Bstatic -lwinpthread -Wl,-Bdynamic)
    target_link_libraries(tokotachi_c PRIVATE ${_tokotachi_system_libs})
endif()

if(TOKOTACHI_INSTALL)
    install(TARGETS tokotachi_c EXPORT tokotachi_cTargets
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(FILES include/tokotachi/tokotachi.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tokotachi)
    install(EXPORT tokotachi_cTargets NAMESPACE tokotachi:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tokotachi)
    configure_file(tokotachi_c.pc.in ${CMAKE_CURRENT_BINARY_DIR}/tokotachi_c.pc @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tokotachi_c.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()

if(TOKOTACHI_BUILD_TESTS)
    # ヘッダが C99 として通り、C から使えることを確かめる。
    add_executable(tokotachi_c_tests tests/test_c.c)
    target_link_libraries(tokotachi_c_tests PRIVATE tokotachi_c)
    target_compile_options(tokotachi_c_tests PRIVATE -Wall -Wextra -pedantic)
    set_target_properties(tokotachi_c_tests PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)
    add_test(NAME tokotachi_c_tests COMMAND tokotachi_c_tests)
    set_tests_properties(tokotachi_c_tests PROPERTIES ENVIRONMENT "SDL_VIDEO_DRIVER=offscreen")

    # Python のバインディングのテスト(Python 3.9 以上があれば)。
    find_package(Python3 3.9 COMPONENTS Interpreter)
    if(Python3_FOUND)
        add_test(NAME tokotachi_python_tests
                 COMMAND ${Python3_EXECUTABLE} -m unittest discover -s ${PROJECT_SOURCE_DIR}/bindings/python/tests -v)
        set_tests_properties(tokotachi_python_tests PROPERTIES ENVIRONMENT
            "SDL_VIDEO_DRIVER=offscreen;TOKOTACHI_C_LIBRARY=$<TARGET_FILE:tokotachi_c>;PYTHONPATH=${PROJECT_SOURCE_DIR}/bindings/python/src")
    endif()
endif()
