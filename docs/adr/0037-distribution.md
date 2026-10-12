# 0037. 共有ライブラリを既定として配り、CMake のパッケージ・pkg-config・ソースからの組み込みに対応する

- 状態: 承認
- 日付: 2026-10-10

## 背景

Tokotachi を MIT で配布する。利用者が CMake でインストールして使う方法に加え、システムへのインストールも考える。
依存ライブラリ(SDL3・FreeType・HarfBuzz・plutovg・libunibreak・SheenBidi)はソースからビルドした静的ライブラリで
([ADR 0005](0005-build-dependencies-from-source.md))、これまでの `libtokotachi.a` には依存ライブラリが入っておらず、
利用者は依存ライブラリも自分でリンクする必要があった。決めること:

- 配る形(共有ライブラリか静的ライブラリか)と、依存ライブラリの扱い。
- 最初に用意する届け方。

## 検討した選択肢

- 形: 共有を既定にし静的も選べる(選択肢A)/静的だけ(選択肢B)/共有だけ(選択肢C)。
- 届け方: CMake のパッケージ・pkg-config・ソースからの組み込み(add_subdirectory・FetchContent)・ライセンスの文書を最初に用意し、
  Linux のディストリビューションのパッケージ・MSYS2 のパッケージは後にする。

## 決定

**形は選択肢A、届け方は上の 4 つを最初に用意する。**

### 共有ライブラリ(既定。`TOKOTACHI_SHARED=ON`)

- 依存ライブラリはライブラリの中にリンクし、**そのシンボルは公開しない**(`-Wl,--exclude-libs,ALL`)。利用者は tokotachi だけを
  リンクし、利用者が別の版の SDL3 などを使っても衝突しない。Linux では、システムの libfontconfig がこちらの FreeType を呼ぶこともない。
- このライブラリ自身のシンボルはすべて公開する(Linux は既定の可視性、MinGW は自動の公開)。公開する API に印(`TOKOTACHI_API`)を
  付けて絞るのは、1.0 の前に検討する。
- SONAME は 0.x の間は `libtokotachi.so.0.<minor>`(小さい版で ABI が変わりうる)、1.0 からは大きい版。
- MinGW のランタイムは、共有ライブラリの版では DLL(`libstdc++-6.dll` など)を使う(ライブラリと exe で C++ のランタイムを
  共有するため)。サンプルの exe の隣にはビルドのときにランタイムの DLL を写す。

### 静的ライブラリ(`TOKOTACHI_SHARED=OFF`)

- 依存ライブラリの静的ライブラリを GNU ar の MRI スクリプトで `libtokotachi.a` に**まとめる**。利用者は tokotachi と OS のライブラリだけを
  リンクする(CMake のパッケージ・pkg-config の `Libs.private` に OS のライブラリを書く)。MinGW では exe が OS の DLL だけに依存する。

### 届け方

- `cmake --install` で、ヘッダ・ライブラリ・CMake のパッケージ(`find_package(tokotachi)`・`tokotachi::tokotachi`。版の確かめは
  0.x の間は小さい版まで一致を求める)・pkg-config(`tokotachi.pc`。場所に依存しない `${pcfiledir}` からの相対)・ライセンスを入れる。
- 利用者のプロジェクトに組み込まれたとき(add_subdirectory・FetchContent)は、サンプル・テスト・インストールの規則を作らず、
  依存ライブラリが無ければ configure のときに自動でビルドする(ビルドのフォルダの中に入れる)。
- 版は `include/tokotachi/core/version.hpp` だけで管理し、CMake はそこから読む。`TOKOTACHI_VERSION_AT_LEAST`(コンパイル時)と
  `tokotachi::version()`(実行時に読み込まれたライブラリの版)を用意した。

### テスト

- テストは内部の状態を確かめ、SDL3 の関数も直接呼ぶ。共有ライブラリでは SDL3 が中に隠れるので、ソースを一度だけコンパイルした
  オブジェクト(OBJECT ライブラリ)から、テスト用に依存ライブラリごと静的にリンクした内部のライブラリも作って使う。

### ライセンス

- このライブラリは MIT(`LICENSE`)。中に含める依存ライブラリのライセンスの文書を `THIRD_PARTY_LICENSES.txt` にまとめ、インストールにも
  含める。FreeType は FTL と GPLv2 から FTL を選び、その求めに従って、文書(README・THIRD_PARTY_LICENSES)に FreeType の謝辞を書く。

### 確かめたこと(Windows・Linux)

- 共有ライブラリが SDL3・FreeType・HarfBuzz・plutovg のシンボルを公開しないこと。
- インストールしたものを、別のプロジェクトから find_package と pkg-config で使えること(共有・静的の両方)。静的の exe は OS の DLL だけに依存すること。
- add_subdirectory で組み込めること、依存ライブラリが無い状態から自動でビルドできること。

## 影響

- Linux のディストリビューションのパッケージ(依存ライブラリはシステムのものを使うのが普通)と、MSYS2 のパッケージは後で対応する。
  そのためには、システムの依存ライブラリを使うビルドの切り替えが要る。
- 依存ライブラリの版を上げたときは、THIRD_PARTY_LICENSES.txt も作り直す。

## 後の変更

- 公開するシンボルは `TOKOTACHI_API` を付けたものに絞った([ADR 0057](0057-compatibility-abi-and-c-layer.md))。
- 版の出し方(タグ・変更履歴・パッケージ)と、インストールしたものを利用者のプロジェクトから使えるかの確かめを CI に加えた
  ([ADR 0061](0061-library-release.md))。
