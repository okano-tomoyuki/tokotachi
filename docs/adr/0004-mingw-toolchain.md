# 0004. Windows のツールチェーンは MinGW-w64(MSYS2 の mingw64)とし、実行ファイルは静的にリンクする

- 状態: 承認
- 日付: 2026-10-04

## 背景

bethany・cpp_tk は Windows では MinGW-w64 の g++ を使ってきた。Windows で使うコンパイラを決める。

## 決定

- **MinGW-w64 の g++ を使う**(MSYS2 の mingw64 環境。CMake + Ninja)。MSVC は当面対象外とし、動作も確かめない
  (C++11 の範囲で書くので、将来 MSVC に対応する余地は残る)。
- **実行ファイルは静的にリンクする**。依存ライブラリは静的ライブラリにし([ADR 0005](0005-build-dependencies-from-source.md))、
  MinGW のランタイム(libgcc・libstdc++・winpthread)も `-static` で含める。実行ファイルは OS の DLL だけに依存し、
  MSYS2 の PATH が無い環境でもそのまま動く(bethany の [ADR 0038](https://github.com/okano-tomoyuki/bethany/blob/main/docs/adr/0038-cpp-distribution.md) と同じ考え方)。

## 影響

- 技術検証(`spike/m1`)の exe は、ADVAPI32・GDI32・IMM32・KERNEL32・OLEAUT32・SETUPAPI・SHELL32・USER32・VERSION・WINMM・
  msvcrt・ole32 の DLL だけに依存することを確かめた。
- MSYS2 の SDL3・HarfBuzz などのパッケージは使わない(入っていても、本体は `deps/install` のものだけを探す)。
