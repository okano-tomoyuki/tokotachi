# Tokotachi

SDL3 を土台にした C++11 のクロスプラットフォーム GUI ライブラリ(開発中)。
対象は Windows(MinGW-w64)、Linux(Wayland を主、X11 を従。x86_64 / aarch64)。

既存の GUI の層には頼らず、OS との境界には必要最小限のもの(SDL3)だけを使い、その上に高機能な部品を一から積み上げる。
名前は、日本の神話で国の土台そのものとして最初に現れる国之常立神(くにのとこたちのかみ)の「常立(とこたち)」から取った
(国の床が立ち現れる・永久に立ち続ける、の意)。

設計判断は [docs/](docs/README.md) の ADR を参照。名前空間は `tokotachi`、インクルードのパスは `<tokotachi/...>`。

## 構成

| 場所 | 内容 |
|---|---|
| [include/tokotachi/](include/tokotachi/) | 公開のヘッダ。役割ごとのディレクトリに分ける(下の表)。`#include <tokotachi/tokotachi.hpp>` ですべて、`<tokotachi/widgets.hpp>` などでグループごとに読み込める |
| [src/](src/) | 実装。公開のヘッダと同じディレクトリに分け、内部のヘッダもそこに置く |
| [examples/](examples/) | サンプル(`hello/` は全体、`gallery/` は部品ごと) |
| [tests/](tests/) | テスト(SDL の offscreen のビデオドライバで、画面なしで動く) |
| [spike/](spike/) | 技術検証のコード(ライブラリには引き継がない) |

| グループ | 内容 |
|---|---|
| `core/` | Object・プロパティ・結びつけ・変数・イベント・Application・タイマー・スレッド(post・Task)・クリップボード・基本の値(Point・Rect・Color) |
| `graphics/` | Canvas・Image・Bitmap・文字のレイアウト・テーマ |
| `layout/` | BoxLayout・GridLayout・PlaceLayout・そろえ方 |
| `widgets/` | Widget とボタン・入力欄・値の部品・スクロール・タブ・分割・PaintBox・ImageBox |
| `windows/` | Window・ポップアップ・ダイアログ・メニューバー・ツールバー・ステータスバー |
| `actions/` | Action・ショートカット・メニュー |
| `views/` | 一覧と木のモデルとビュー |

```cpp
#include <tokotachi/tokotachi.hpp>

int main(int argc, char** argv)
{
    tokotachi::Application app(argc, argv);
    tokotachi::Window window("hello", 400, 200);

    tokotachi::Button* button_close = new tokotachi::Button(&window, "クリック");  // 親(window)が所有する
    button_close->bounds = tokotachi::Rect(24, 24, 140, 36);                  // 状態はプロパティで読み書きする
    button_close->on_click = [&](tokotachi::Object* sender)                   // イベントは単一のハンドラ
    {
        window.close();
    };

    window.visible = true;
    return app.exec();
}
```

API の規則(プロパティ・ハンドラ・命名・書式)は [ADR 0013](docs/adr/0013-api-style-properties-and-handlers.md) を参照。

## 依存ライブラリ

クロスプラットフォームの依存ライブラリは [deps/](deps/CMakeLists.txt) でソースからビルドし、
静的ライブラリとして `deps/install/<プラットフォーム>/` にインストールする(バージョンは URL と SHA256 で固定)。

| ライブラリ | バージョン | ライセンス | 用途 |
|---|---|---|---|
| SDL3 | 3.4.18 | zlib | ウィンドウ、入力、IME、クリップボード、高 DPI |
| FreeType | 2.14.3 | FTL | グリフのラスタライズ |
| HarfBuzz | 14.5.1 | MIT | テキストのシェーピング |
| plutovg | 1.3.3 | MIT | 2D のベクター描画 |
| libunibreak | 8.0 | zlib | 改行・単語・書記素の境界(UAX #14・#29) |
| SheenBidi | 3.0.0 | Apache-2.0 | 双方向テキスト(UAX #9) |

OS の一部であるライブラリはシステムのものを使う。

- Windows: Win32 API のみ(exe は OS の DLL だけに依存する)。
- Linux: wayland-client、libxkbcommon、libdecor、X11、EGL、D-Bus・IBus(IME)、fontconfig。
  SDL3 はこれらを実行時に dlopen するので、ビルドには開発用パッケージ(ヘッダ)が要る。

## ビルド

依存ライブラリは、見つからなければ本体の configure のときに自動でビルドする(初回だけ。数分かかる)。
先に別にビルドしておくこともできる(依存ライブラリを更新するときなど):

```bash
cmake -S deps -B build-deps -G Ninja
cmake --build build-deps
```

本体(ライブラリ・サンプル・テスト・検証コード)のビルドとテスト:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/hello            # サンプル(--screenshot a.png で PNG に保存して終える)
```

CMake のオプション:

| オプション | 既定 | 内容 |
|---|---|---|
| `TOKOTACHI_SHARED` | ON | 共有ライブラリ(DLL / .so)を作る。OFF なら、依存ライブラリをまとめた静的ライブラリ(`libtokotachi.a`)を作る |
| `TOKOTACHI_BUILD_EXAMPLES` / `TOKOTACHI_BUILD_TESTS` | 単独のとき ON | サンプル・テスト(利用者のプロジェクトに組み込まれたときは OFF) |
| `TOKOTACHI_INSTALL` | 単独のとき ON | インストールの規則を作る |
| `TOKOTACHI_DEPS_PREFIX` | `deps/install/<プラットフォーム>` | 依存ライブラリの場所(組み込まれたときはビルドのフォルダの中) |
| `TOKOTACHI_BUILD_DEPS` | ON | 依存ライブラリが無ければ自動でビルドする |
| `TOKOTACHI_STATIC_RUNTIME` | ON | 静的ライブラリのとき、MinGW のランタイムも静的にリンクする(exe が OS の DLL だけに依存する) |

## 使う(インストール・組み込み)

インストールする(`<prefix>` は `/usr/local` や任意のフォルダ):

```bash
cmake --install build --prefix <prefix>
```

`include/tokotachi/`・ライブラリ・CMake のパッケージ(`lib/cmake/tokotachi/`)・pkg-config(`lib/pkgconfig/tokotachi.pc`)・
ライセンス(`share/licenses/tokotachi/`)が入る。

CMake から使う:

```cmake
find_package(tokotachi 0.1 REQUIRED)          # 見つからなければ -DCMAKE_PREFIX_PATH=<prefix>
target_link_libraries(app PRIVATE tokotachi::tokotachi)
```

pkg-config から使う(静的ライブラリなら `--static` を付ける):

```bash
g++ -std=c++11 main.cpp $(pkg-config --cflags --libs tokotachi)
```

ソースから一緒にビルドする(依存ライブラリも自動でビルドする):

```cmake
include(FetchContent)
FetchContent_Declare(tokotachi GIT_REPOSITORY <リポジトリ> GIT_TAG <タグ>)
FetchContent_MakeAvailable(tokotachi)        # add_subdirectory(<ソースの場所>) でもよい
target_link_libraries(app PRIVATE tokotachi::tokotachi)
```

```cpp
#include <tokotachi/tokotachi.hpp>         // すべて(<tokotachi/widgets.hpp> などでグループごとにも読める)
```

共有ライブラリの版を配るときは、exe と一緒に `libtokotachi.dll`(Windows。MinGW のランタイムの `libstdc++-6.dll`・
`libgcc_s_seh-1.dll`・`libwinpthread-1.dll` も)・`libtokotachi.so.<版>`(Linux)を配る。静的ライブラリの版なら exe 1 つで配れる。
`share/licenses/tokotachi/` の LICENSE と THIRD_PARTY_LICENSES.txt も一緒に配る。

### リファレンス

API リファレンス・使い方の手引き・サンプルの一覧([docs/guide/](docs/guide/))を Doxygen で作る(Doxygen が要る。Graphviz があれば図も描く):

```bash
cmake -S . -B build -G Ninja -DTOKOTACHI_BUILD_DOCS=ON
cmake --build build --target docs        # build/docs/html/index.html
```

master に push すると、GitHub Actions が作って GitHub Pages(https://okano-tomoyuki.github.io/tokotachi/)に出す(`.github/workflows/docs.yml`)。

### Windows(MSYS2 mingw64)

```bash
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja
```

MSYS2 の SDL3・HarfBuzz などのパッケージは使わない(入っていても、本体は `deps/install` のものだけを探す)。

### Linux(Ubuntu 24.04 / Debian)

```bash
sudo apt install build-essential cmake ninja-build pkg-config git \
  libwayland-dev libxkbcommon-dev libdecor-0-dev libegl-dev \
  libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev libxss-dev libxtst-dev \
  libdbus-1-dev libibus-1.0-dev libudev-dev libdrm-dev libgbm-dev libfontconfig-dev
```

足りないものがあると、`deps/` の configure がその名前を出して止まる
(SDL3 は D-Bus・IBus・libdecor が無くても黙って無効にするため、IME の使えないビルドを作らないように確かめている)。

WSL2 で Windows と同じソースのフォルダを使う場合は、ビルドのフォルダを分ける
(`deps/install/` はプラットフォームごとに分かれているので共有してよい)。
`/mnt/c` の上でのビルドは遅いので、ビルドのフォルダは WSL の側に置くとよい:

```bash
cmake -S deps -B ~/tokotachi-build/deps -G Ninja && cmake --build ~/tokotachi-build/deps
cmake -S . -B ~/tokotachi-build/main -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build ~/tokotachi-build/main
```

WSLg の Wayland で、ビルドと描画・`video: wayland` の確認はできる。WSL で気を付けること:

- systemd が有効だと `XDG_RUNTIME_DIR`(`/run/user/<uid>/`)に Wayland のソケットが無く、SDL3 が X11 にフォールバックする。
  ソケットは `/mnt/wslg/runtime-dir/` にあるので、リンクを張る(`~/.profile` 等に書いておく):
  ```bash
  ln -sf /mnt/wslg/runtime-dir/wayland-0 /mnt/wslg/runtime-dir/wayland-0.lock "$XDG_RUNTIME_DIR/"
  ```
- 日本語のフォントが入っていないことがある(`fc-list :lang=ja` が空)。`sudo apt install fonts-noto-cjk` で入れるか、
  `--font /mnt/c/Windows/Fonts/YuGothM.ttc` で Windows のフォントを使う。
- `libEGL warning: ...` は WSLg の GPU ドライバの警告で、描画には影響しない。
- WSLg のコンポジタは text-input-v3(SDL3 が使う Wayland の IME のプロトコル)も wp_fractional_scale_v1 も持たない。
  Windows の IME は届かないので、日本語入力は WSL の中で Fcitx5 を動かし、SDL3 から D-Bus で使う:
  ```bash
  sudo apt install fcitx5 fcitx5-mozc   # ~/.config/fcitx5/profile で mozc を追加する
  fcitx5 -d --replace --disable=wayland,waylandim
  XMODIFIERS=@im=fcitx ./spike_m1        # Ctrl+Space で切り替え
  ```
  Fcitx5 の Wayland の addon は WSLg の zwp_input_method_v1 を使おうとして拒否され、Fcitx5 ごと終了するので無効にする
  (候補窓は XWayland に出し、SDL3 とは D-Bus でやり取りする)。
  実機のデスクトップ(GNOME / KDE 等)では text-input-v3 の経路になり、WSL とは経路が違う。


IME と分数スケーリングは WSLg では確かめきれないので、実機の GNOME / KDE で確認する。

## ライセンス

MIT([LICENSE](LICENSE))。ライブラリの中に含める依存ライブラリのライセンスは [THIRD_PARTY_LICENSES.txt](THIRD_PARTY_LICENSES.txt)
(SDL3・libunibreak は zlib、HarfBuzz・plutovg は MIT 系、FreeType は FreeType License、SheenBidi は Apache-2.0)。

Portions of this software are copyright © The FreeType Project (https://freetype.org). All rights reserved.
