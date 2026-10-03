# 0005. クロスプラットフォームの依存ライブラリはソースからビルドして静的にリンクし、OS の一部はシステムのものを使う

- 状態: 承認
- 日付: 2026-10-04

## 背景

最初は、依存ライブラリにシステムのパッケージ(MSYS2 の pacman、ディストリビューションの apt)を使い、
配布の段階で FetchContent に移る方針だった。しかし Ubuntu 24.04 LTS には SDL3 のパッケージが無く(ThorVG の 1.x も無い)、
Linux ではどのみちソースからビルドする必要があることが分かった。

## 検討した選択肢

- 選択肢A: システムのパッケージを使う。ビルドは速いが、ディストリビューションによってバージョンがばらつき、
  SDL3 が無い環境ではそもそもビルドできない。
- 選択肢B: FetchContent で本体のビルドに依存ライブラリを取り込む。本体を configure するたびに依存ライブラリも
  configure され、各ライブラリのオプション(`BUILD_SHARED_LIBS` など)が本体と干渉する。
- 選択肢C: 依存ライブラリだけを別にビルドする CMake(ExternalProject)を用意し、決まった場所にインストールして、
  本体からはそこを `find_package` する。

## 決定

**選択肢C を選ぶ。** Windows と Linux の両方で同じ仕組みを使う。

- [deps/CMakeLists.txt](../../deps/CMakeLists.txt) が依存ライブラリをソースからビルドし、静的ライブラリとして
  `deps/install/<プラットフォーム>/` にインストールする(`windows-x86_64`、`linux-x86_64`、`linux-aarch64` など)。
  バージョンは URL と SHA256 で固定し、アーカイブは `deps/download/` にキャッシュする。
- 本体は `find_package(... PATHS deps/install/<プラットフォーム> NO_DEFAULT_PATH)` で探し、
  システムに同じライブラリが入っていても混ざらないようにする。
- **ソースからビルドするのは、CMake でビルドできるクロスプラットフォームのライブラリに限る**(Meson や autotools が要るものは避ける。
  ソースが小さければ自前の CMake でビルドする)。
- **OS の一部はシステムのものを使う**: Linux の wayland-client、libxkbcommon、libdecor、X11、EGL、D-Bus・IBus、fontconfig。
  SDL3 はこれらを実行時に dlopen するので、ビルドに要るのは開発用パッケージ(ヘッダ)だけ。
- **Linux では、SDL3 が黙って機能を落とさないよう、configure で開発用パッケージの有無を確かめる**。SDL3 は D-Bus・IBus・libdecor が
  無くてもエラーにせず、その機能を無効にする(IME の使えないビルドができてしまう)。
- **Linux では、静的にリンクしたライブラリのシンボルを実行ファイル・共有ライブラリから公開しない**(`-Wl,--exclude-libs,ALL`)。
  公開すると、システムの libfontconfig がシステムの libfreetype ではなくこちらの FreeType を呼ぶ(シンボルの横取り。
  技術検証で 149 個の `FT_*` が公開されていたことを確かめた)。

現時点の依存ライブラリ:

| ライブラリ | バージョン | ライセンス |
|---|---|---|
| SDL3 | 3.4.18 | zlib |
| FreeType | 2.14.3 | FTL |
| HarfBuzz | 14.5.1 | MIT |
| plutovg | 1.3.3 | MIT |
| libunibreak | 8.0 | zlib(autotools のみなので自前の CMake でビルド。[ADR 0019](0019-line-breaking-and-text-navigation.md)) |
| SheenBidi | 3.0.0 | Apache-2.0([ADR 0020](0020-bidi-and-text-rendering-quality.md)) |

## 影響

- 初回のビルドに数分かかる(以後は本体だけをビルドする)。
- FreeType と HarfBuzz は互いに依存するので、FreeType は HarfBuzz なしでビルドし(`FT_DISABLE_HARFBUZZ`)、
  HarfBuzz の側から FreeType を使う。
- HarfBuzz の CMake は `CMAKE_INSTALL_INCLUDEDIR` を指定しないと、インストールした設定ファイルのインクルードのパスが空になる。
  依存ライブラリの共通の引数で `CMAKE_INSTALL_INCLUDEDIR`・`CMAKE_INSTALL_LIBDIR`・`CMAKE_INSTALL_BINDIR` を明示する。
- 配布の段階では、同じ依存ライブラリを FetchContent で取り込める形を別に用意する(利用者のビルドの手順を 1 つにするため)。
- 配布物には、各ライブラリのライセンスの表示を同梱する(FreeType の FTL は表示を求める)。
