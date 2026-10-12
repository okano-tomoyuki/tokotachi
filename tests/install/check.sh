#!/usr/bin/env bash
# インストールしたライブラリを、利用者のプロジェクトとして使えるか確かめる(docs/adr/0061)。
#   tests/install/check.sh <インストール先> <作業用のフォルダ>
# CMake のパッケージ(find_package)と pkg-config の両方でビルドし、画面なしで動かす。
# Linux と Windows(MSYS2 の bash)で動く。
set -euo pipefail

prefix=$(cd "$1" && pwd)
work=$2
here=$(cd "$(dirname "$0")" && pwd)
root=$(cd "$here/../.." && pwd)

version=$(sed -n 's/^#define TOKOTACHI_VERSION_STRING "\(.*\)"$/\1/p' "$root/include/tokotachi/core/version.hpp")
export SDL_VIDEO_DRIVER=offscreen
# 共有ライブラリを見つけられるように(Windows は exe の隣か PATH、Linux は RUNPATH が無いので LD_LIBRARY_PATH)
export PATH="$prefix/bin:$PATH"
export LD_LIBRARY_PATH="$prefix/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

echo "== CMake のパッケージ(find_package(tokotachi $version))"
cmake -S "$here" -B "$work/cmake" -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$prefix" -DTOKOTACHI_EXPECTED_VERSION="$version"
cmake --build "$work/cmake"
"$work/cmake/install_check"

echo "== pkg-config"
export PKG_CONFIG_PATH="$prefix/lib/pkgconfig"
pc_version=$(pkg-config --modversion tokotachi)
if [ "$pc_version" != "$version" ]; then
  echo "pkg-config の版 $pc_version がヘッダの版 $version と違う" >&2
  exit 1
fi
shared=$(sed -n 's/^set(tokotachi_SHARED \(.*\))$/\1/p' "$prefix/lib/cmake/tokotachi/tokotachiConfig.cmake")
static_flag=
case "$shared" in
  ON|TRUE|1) ;;
  *) static_flag=--static ;;
esac
mkdir -p "$work/pkg-config"
# shellcheck disable=SC2046
${CXX:-g++} -std=c++11 -O1 -o "$work/pkg-config/install_check" "$here/main.cpp" \
  $(pkg-config $static_flag --cflags --libs tokotachi)
"$work/pkg-config/install_check"
