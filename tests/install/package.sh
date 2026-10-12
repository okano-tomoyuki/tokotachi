#!/usr/bin/env bash
# インストールしたものを、配布のパッケージにまとめる(docs/adr/0061)。
#   tests/install/package.sh <インストール先の親> <プラットフォーム> <出力先>
# <インストール先の親>/shared と /static を、tokotachi-<版>-<プラットフォーム>-<shared|static> の名前でまとめる
# (Windows は .zip、ほかは .tar.gz)。作った環境を BUILD_INFO.txt に書き、SHA256 を <出力先>/SHA256SUMS-<プラットフォーム>.txt に書く。
set -euo pipefail

installs=$(cd "$1" && pwd)
platform=$2
mkdir -p "$3"
out=$(cd "$3" && pwd)
here=$(cd "$(dirname "$0")" && pwd)
root=$(cd "$here/../.." && pwd)

version=$(sed -n 's/^#define TOKOTACHI_VERSION_STRING "\(.*\)"$/\1/p' "$root/include/tokotachi/core/version.hpp")
stage=$(mktemp -d)
trap 'rm -rf "$stage"' EXIT

for kind in shared static; do
  name="tokotachi-$version-$platform-$kind"
  cp -R "$installs/$kind" "$stage/$name"
  {
    echo "Tokotachi $version ($platform, $kind)"
    echo "commit: $(git -C "$root" rev-parse HEAD)"
    echo "compiler: $(${CXX:-g++} --version | head -n 1)"
    case "$platform" in
      linux-*) echo "glibc: $(ldd --version | head -n 1)" ;;
    esac
  } > "$stage/$name/BUILD_INFO.txt"
  case "$platform" in
    windows-*) (cd "$stage" && cmake -E tar cf "$out/$name.zip" --format=zip "$name") ;;
    *) (cd "$stage" && cmake -E tar czf "$out/$name.tar.gz" "$name") ;;
  esac
done

(cd "$out" && sha256sum tokotachi-"$version-$platform"-* > "SHA256SUMS-$platform.txt")
ls -l "$out"
