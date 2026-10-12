# インストールしたライブラリを find_package で使う利用者のプロジェクト(docs/adr/0061)。check.sh から使う。
cmake_minimum_required(VERSION 3.16)
project(tokotachi_install_check LANGUAGES CXX)

find_package(tokotachi ${TOKOTACHI_EXPECTED_VERSION} EXACT REQUIRED)

add_executable(install_check main.cpp)
target_compile_features(install_check PRIVATE cxx_std_11)
target_link_libraries(install_check PRIVATE tokotachi::tokotachi)
