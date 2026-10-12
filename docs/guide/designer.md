# デザイナーで画面を作る {#designer_guide}

[TOC]

VS Code の拡張 **Tokotachi Designer** で、画面を見ながらウィンドウ・ダイアログを組み、C++ のクラスを生成できます。
キャンバスは Tokotachi 自身(WebAssembly にしたもの)が描くので、ビルドしたアプリと同じ見た目になります。
拡張の入れ方・画面の使い方は、拡張の README(リポジトリの `designer/packages/extension/README.ja.md`)を参照してください。

## 流れ

1. VS Code でフォルダを開き、**Tokotachi Designer: New Project...** でプロジェクトを作ります。プロジェクトファイル(`*.ttproj.json`)・
   メインのウィンドウの定義ファイル(`main_window.ttui.json`)ができ、デザイナーで開きます。
2. パレットの部品をキャンバスに置き、インスペクタでプロパティ・ハンドラ・変数との結びつけを決めます。
3. 保存すると、ウィンドウごとの C++ のクラスと、`main.cpp`・`CMakeLists.txt` を生成します。
4. 生成したクラスにハンドラの中身を書き、CMake でビルドします(@ref getting_started)。

```bash
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH=<Tokotachi のインストール先>
cmake --build build
```

## 生成するコード

定義ファイル `main_window.ttui.json` から、`main_window.hpp`・`main_window.cpp` を生成します。

```cpp
class MainWindow : public tokotachi::Window
{
public:
    explicit MainWindow(tokotachi::Window* owner = nullptr);   // 所有者のウィンドウ(任意)

    // <tokotachi-designer:begin id="declarations">
    tokotachi::LineEdit* line_edit_search;        // 名前を付けた部品は、メンバとして使える
    tokotachi::Button* button_search;
    void search(tokotachi::Object* sender);       // インスペクタでつないだハンドラ
    // <tokotachi-designer:end id="declarations" hash="...">

private:
    void tokotachi_setup_ui();                    // 部品を作り、プロパティを決める(生成する)
};
```

- **印(`tokotachi-designer:begin`・`end`)で囲んだ区間だけ**を、生成し直すたびに置き換えます。区間の外に書いたもの
  (ハンドラの中身・メンバ・include)はそのまま残ります。区間の中を手で書き換えると、生成の前に上書きしてよいかを尋ねます
  (保存のときは生成せずに知らせます)。
- ハンドラの雛形は、ソース(.cpp)の `tokotachi-designer:handler-stubs` の印の下に足します。中身を書けば、後で生成し直しても残ります。
- 部品は `tokotachi_setup_ui()` の中で `new` で作り、ウィンドウが所有します(@ref basics)。
- 画像を埋め込んだウィンドウは、画像のデータを別のソース(`main_window_images.cpp`)に書きます。

```cpp
void MainWindow::search(tokotachi::Object* sender)
{
    std::string word = line_edit_search->text;   // 部品はメンバで使う
    ...
}
```

## ほかのウィンドウ・ダイアログを開く

ダイアログ(`Dialog` の定義ファイル)のクラスは、親のウィンドウを受け取ります。

```cpp
void MainWindow::button_settings_click(tokotachi::Object* sender)
{
    SettingsDialog* dialog_settings = new SettingsDialog(this);   // this(親のウィンドウ)が所有する
    dialog_settings->destroy_on_close = true;                      // 閉じたら破棄する
    dialog_settings->on_finish = [=](tokotachi::Object*)
    {
        if (dialog_settings->result == tokotachi::DialogButton::OK) { ... }
    };
    dialog_settings->open();
}
```

`Window` の定義ファイルのクラスは、所有者のウィンドウを任意で受け取ります。渡さなければ `Application` が所有し、渡せば
そのウィンドウの上に出て、一緒に隠れ・破棄されます(ツールのウィンドウなど)。

```cpp
ToolWindow* window_tool = new ToolWindow(this);   // this(このウィンドウ)が所有する
window_tool->visible = true;
```

生成する `main.cpp` は、プロジェクトの
メインのウィンドウを作り、`app.main_window` に決めて表示します(閉じたら終わります。@ref windows_guide)。

## プロジェクト

- プロジェクトファイルに、ウィンドウの一覧・メインのウィンドウ・コード生成の設定(名前空間・ヘッダとソースのフォルダ・コメントの言語)・
  テーマを書きます。デザイナーのプロジェクトの設定画面で編集できます。
- `CMakeLists.txt` は最初に雛形を作り、その後は**ソースの一覧の区間だけ**を置き換えます。区間の外(ライブラリのリンク・自分のソース)は
  自由に書き足せます。
- `CMakeLists.txt` は、デザイナーを作ったときのライブラリの版を `find_package(tokotachi 0.2 REQUIRED)` のように求めます。ライブラリの
  小さい版を上げたら(0.x の間)、この行を直します(雛形は作り直しません。@ref compatibility)。

## 使い回す・自作の部品

- 部品の組み合わせを**部品ファイル**(New Component File)にすると、ほかのウィンドウに置けます。部品ファイルからは `Widget` の派生クラスを
  生成します。
- C++ で書いた**自作の部品**(@ref custom_widgets_guide)は、プロジェクトにヘッダを登録するとパレットに出ます。キャンバスでは基底の部品として
  描きます。自作の部品のソース(.cpp)は `CMakeLists.txt` の区間の外に足します(`target_sources(my_app PRIVATE widgets/gauge.cpp)`)。
