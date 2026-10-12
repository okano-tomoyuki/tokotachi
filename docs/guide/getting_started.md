# はじめに {#getting_started}

[TOC]

## 必要なもの

- CMake 3.16 以上、Ninja(推奨)、C++11 のコンパイラ
- Windows: MSYS2 の mingw64(`pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja`)
- Linux(Ubuntu・Debian): 次の開発用パッケージ(SDL3 が実行時に使う OS のライブラリのヘッダ)

```bash
sudo apt install build-essential cmake ninja-build pkg-config git \
  libwayland-dev libxkbcommon-dev libdecor-0-dev libegl-dev \
  libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev libxss-dev libxtst-dev \
  libdbus-1-dev libibus-1.0-dev libudev-dev libdrm-dev libgbm-dev libfontconfig-dev
```

SDL3・FreeType・HarfBuzz などの依存ライブラリは、Tokotachi がソースからビルドしてライブラリの中に入れます(初回の configure で自動。数分かかります)。

## 自分のプロジェクトから使う

### ソースから一緒にビルドする

```cmake
include(FetchContent)
FetchContent_Declare(tokotachi GIT_REPOSITORY https://github.com/okano-tomoyuki/tokotachi.git GIT_TAG v0.3.0)
FetchContent_MakeAvailable(tokotachi)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE tokotachi::tokotachi)
```

### ビルドしたパッケージを使う

[GitHub の Release](https://github.com/okano-tomoyuki/tokotachi/releases) に、Linux(x86_64・aarch64)と Windows(MSYS2 の MINGW64)の
パッケージがあります(`-shared` が共有ライブラリ、`-static` が静的ライブラリ)。展開したフォルダを `<prefix>` として、下の
`find_package` か pkg-config で使います。ビルドした環境(コンパイラ・glibc)はパッケージの `BUILD_INFO.txt` にあります。C++ のライブラリなので、
同じ系統のコンパイラで使ってください(Windows は MSYS2 の MINGW64 の GCC)。

### インストールして使う

```bash
cmake -S tokotachi -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build --prefix <prefix>
```

```cmake
find_package(tokotachi 0.3 REQUIRED)          # -DCMAKE_PREFIX_PATH=<prefix>
target_link_libraries(app PRIVATE tokotachi::tokotachi)
```

CMake を使わないときは pkg-config(`pkg-config --cflags --libs tokotachi`)。

### 共有ライブラリと静的ライブラリ

既定は共有ライブラリ(`libtokotachi.dll`・`libtokotachi.so`)です。依存ライブラリは中に入っていて、外からは見えません。
exe 1 つで配りたいときは `-DTOKOTACHI_SHARED=OFF` で静的ライブラリにします(依存ライブラリもまとめて入ります)。

| 配るもの | 共有ライブラリ | 静的ライブラリ |
|---|---|---|
| Windows | exe・`libtokotachi.dll`・MinGW のランタイム(`libstdc++-6.dll`・`libgcc_s_seh-1.dll`・`libwinpthread-1.dll`) | exe だけ |
| Linux | 実行ファイル・`libtokotachi.so.<版>` | 実行ファイルだけ |

どちらも、インストール先の `share/licenses/tokotachi/` にある LICENSE と THIRD_PARTY_LICENSES.txt を一緒に配ってください。

## 最初のウィンドウ

```cpp
#include <tokotachi/tokotachi.hpp>

int main(int argc, char** argv)
{
    tokotachi::Application app(argc, argv);          // 最初に 1 つ作る(これだけスタックに置く)
    tokotachi::Window* window_main = new tokotachi::Window("最初のウィンドウ", 400, 200);   // app が所有する
    new tokotachi::Label(window_main, "こんにちは");     // window_main が所有する(delete しない)
    app.main_window = window_main;                   // このウィンドウを閉じると exec() から戻る
    window_main->visible = true;
    return app.exec();
}
```

- `Application` を最初に作り、`exec()` でイベントループを回します。`Application` は main の中でスタックに置きます。
- ウィンドウ・部品は `new` で作ります。ウィンドウは `Application` が、部品は親が所有し、所有者が破棄されると一緒に破棄されるので、
  `delete` は書きません(@ref basics)。
- `main_window` を決めると、そのウィンドウを閉じたときに `exec()` から戻ります。決めなければ、最後のウィンドウを閉じたときに戻ります。
- 大きさと位置の単位は dp(倍率 1 のときのピクセル)です。高 DPI の画面では自動で拡大されます。

## 版を調べる

```cpp
#if TOKOTACHI_VERSION_AT_LEAST(0, 2, 0)    // コンパイルに使ったヘッダの版
#endif
const char* v = tokotachi::version();      // 実行時に読み込まれたライブラリの版
```
