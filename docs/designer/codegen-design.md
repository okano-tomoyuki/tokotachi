# コード生成の設計

画面の定義ファイル([dsl-spec.md](dsl-spec.md))から、Tokotachi の C++ のコードを生成する方法。
生成したコードと利用者のコードの共存には、Bethany と同じく、マーカーで囲んだ区間だけを置き換える方式を使う([ADR 0039](../adr/0039-designer.md))。

## 1. 生成の流れ

```
定義ファイル (*.ttui.json)
  └─ 検証(Zod のスキーマ + カタログとの照合 + 意味の検証)
       └─ 操作の並び(作る・プロパティを設定する・呼び出す・置く・ハンドラをつなぐ。model.ts の buildSetup)
            ├─ C++ の書き出し → 区間ごとのコード片
            │    └─ 合わせる: ファイルが無ければ雛形を作り、あればマーカーの区間だけを置き換える
            └─ キャンバス(WebAssembly の Tokotachi)が同じ並びを実行して描く(ADR 0040)
```

生成の単位は 1 つの定義ファイル = 1 ウィンドウ = 1 クラス。生成するクラスは `tokotachi::Window`(または `tokotachi::Dialog`)を継承する。

| 生成するもの | 決まり方(定義ファイルが `main_window.ttui.json`、`window.name` が `MainWindow` の場合) |
|---|---|
| クラス名 | `window.name`(`MainWindow`) |
| ヘッダ・ソース | 定義ファイルと同じフォルダの `main_window.hpp`・`main_window.cpp`(定義ファイルの名前から。拡張子とフォルダはプロジェクトの設定で変えられる。§8) |

## 2. マーカーの区間

| 区間 | 内容 | 置き場所 |
|---|---|---|
| `tokotachi_header_begin` | インクルードガードの始まりと `#include <tokotachi/tokotachi.hpp>` | ヘッダの先頭 |
| `tokotachi_source_begin` | 自分のヘッダの include | ソースの先頭 |
| `tokotachi_namespace_begin`・`tokotachi_namespace_end` | 名前空間の始まりと終わり(プロジェクトの設定。使わないときはマーカーの行だけ残す) | ヘッダ・ソース |
| `declarations` | 部品・Action・オブジェクトのメンバ(ポインタ)と、**ハンドラのメンバ関数の宣言** | ヘッダ(クラスの定義の中、public) |
| `tokotachi_setup_ui` | 部品を作り、プロパティを設定し、レイアウトに入れ、ハンドラをつなぐ関数の定義 | ソース |
| `tokotachi_header_end` | インクルードガードの終わり | ヘッダの末尾 |

利用者の include は `tokotachi_header_begin`・`tokotachi_source_begin` の後、名前空間の区間の前に書く。
コンストラクタ・デストラクタと、ハンドラの中身は区間の外にあり、生成し直しても変わらない。

### マーカーの書式

```cpp
// <tokotachi-designer:begin id="tokotachi_setup_ui">
...
// <tokotachi-designer:end id="tokotachi_setup_ui" hash="3f9a1c...">
```

`id` で区間を見分け、`hash` は区間の中身のハッシュ(手で変えたことに気づくため。§6 M2)。
ハンドラの雛形は `// <tokotachi-designer:handler-stubs>` の行の後に並べる。生成し直して足りない雛形を足すときは、新しく作ったときと同じ位置(生成する順で後ろに来る既存の雛形の前。無ければ雛形の並びの終わり)に、同じ空行で足す。

## 3. 生成例

[dsl-spec.md §2](dsl-spec.md#2-例) の例から生成するコード(抜粋)。コメントの言語はプロジェクトの設定で決める(既定は英語。ここでは日本語)。

`main_window.hpp`

```cpp
// <tokotachi-designer:begin id="tokotachi_header_begin">
#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <tokotachi/tokotachi.hpp>
// <tokotachi-designer:end id="tokotachi_header_begin" hash="...">

// <tokotachi-designer:begin id="tokotachi_namespace_begin">
// <tokotachi-designer:end id="tokotachi_namespace_begin" hash="...">

/// デザイナーで作ったウィンドウ(main_window.ttui.json)。マーカーで囲んだ区間は、生成し直すと上書きされる。
class MainWindow : public tokotachi::Window
{
public:
    explicit MainWindow(tokotachi::Window* owner = nullptr);

    // <tokotachi-designer:begin id="declarations">
    tokotachi::Widget* row_search;
    tokotachi::LineEdit* line_edit_search;
    tokotachi::Button* button_search;
    tokotachi::Splitter* splitter_main;
    tokotachi::TreeView* tree_view_folders;
    tokotachi::ListView* list_view_files;
    tokotachi::Menu* menu_file;
    tokotachi::Action* action_open;
    tokotachi::Action* action_quit;
    tokotachi::Timer* autosave_timer;

    void main_window_close(tokotachi::Object* sender, bool& can_close);
    void button_search_click(tokotachi::Object* sender);
    void action_open_trigger(tokotachi::Object* sender);
    void action_quit_trigger(tokotachi::Object* sender);
    void autosave(tokotachi::Object* sender);
    // <tokotachi-designer:end id="declarations" hash="...">

private:
    void tokotachi_setup_ui();
};

// <tokotachi-designer:begin id="tokotachi_namespace_end">
// <tokotachi-designer:end id="tokotachi_namespace_end" hash="...">

// <tokotachi-designer:begin id="tokotachi_header_end">
#endif  // MAIN_WINDOW_HPP
// <tokotachi-designer:end id="tokotachi_header_end" hash="...">
```

`main_window.cpp`

```cpp
// <tokotachi-designer:begin id="tokotachi_source_begin">
#include "main_window.hpp"
// <tokotachi-designer:end id="tokotachi_source_begin" hash="...">

// <tokotachi-designer:begin id="tokotachi_namespace_begin">
// <tokotachi-designer:end id="tokotachi_namespace_begin" hash="...">

MainWindow::MainWindow(tokotachi::Window* owner)
    : tokotachi::Window(owner, std::string())
{
    tokotachi_setup_ui();
}

// <tokotachi-designer:begin id="tokotachi_setup_ui">
// 部品を作り、プロパティを設定し、レイアウトに入れ、ハンドラをつなぐ(生成したコード)。
void MainWindow::tokotachi_setup_ui()
{
    action_open = new tokotachi::Action(this);
    action_open->text = "開く(&O)...";
    action_open->shortcut = tokotachi::Shortcut("Ctrl+O");
    action_open->on_trigger = [this](tokotachi::Object* sender) { action_open_trigger(sender); };

    action_quit = new tokotachi::Action(this);
    action_quit->text = "終了(&X)";
    action_quit->on_trigger = [this](tokotachi::Object* sender) { action_quit_trigger(sender); };

    autosave_timer = new tokotachi::Timer(this);
    autosave_timer->interval = 60000;
    autosave_timer->on_timeout = [this](tokotachi::Object* sender) { autosave(sender); };

    title = "メモ";
    width = 640;
    height = 480;
    on_close = [this](tokotachi::Object* sender, bool& can_close) { main_window_close(sender, can_close); };

    tokotachi::MenuBar* window_menu_bar = new tokotachi::MenuBar();
    menu_file = window_menu_bar->add_menu("ファイル(&F)");
    menu_file->add_action(action_open);
    menu_file->add_separator();
    menu_file->add_action(action_quit);
    menu_bar = window_menu_bar;

    tokotachi::ToolBar* window_tool_bar = new tokotachi::ToolBar();
    window_tool_bar->button_style = tokotachi::ToolButtonStyle::TEXT_BESIDE_ICON;
    window_tool_bar->add_action(action_open);
    tool_bar = window_tool_bar;

    status_bar = new tokotachi::StatusBar();

    tokotachi::BoxLayout* window_layout = new tokotachi::BoxLayout(this);
    window_layout->margins = tokotachi::Margins(12);

    row_search = new tokotachi::Widget(this);
    tokotachi::BoxLayout* row_search_layout = new tokotachi::BoxLayout(row_search);
    row_search_layout->orientation = tokotachi::Orientation::HORIZONTAL;

    line_edit_search = new tokotachi::LineEdit(row_search);
    line_edit_search->placeholder_text = "検索";
    row_search_layout->add(line_edit_search, 1);

    button_search = new tokotachi::Button(row_search);
    button_search->text = "検索";
    button_search->on_click = [this](tokotachi::Object* sender) { button_search_click(sender); };
    row_search_layout->add(button_search);

    window_layout->add(row_search);

    splitter_main = new tokotachi::Splitter(this);

    tree_view_folders = new tokotachi::TreeView(splitter_main);
    splitter_main->add_widget(tree_view_folders, 0);

    list_view_files = new tokotachi::ListView(splitter_main);
    splitter_main->add_widget(list_view_files);

    splitter_main->sizes = std::vector<float>{160, 360};
    window_layout->add(splitter_main, 1);
}
// <tokotachi-designer:end id="tokotachi_setup_ui" hash="...">

// <tokotachi-designer:handler-stubs>
void MainWindow::main_window_close(tokotachi::Object* sender, bool& can_close)
{
}

void MainWindow::button_search_click(tokotachi::Object* sender)
{
}
// ...

// <tokotachi-designer:begin id="tokotachi_namespace_end">
// <tokotachi-designer:end id="tokotachi_namespace_end" hash="...">
```

`Window` のときは、コンストラクタが所有者のウィンドウを任意で受け取る(既定 nullptr。渡さなければ Application が所有し、渡せば
そのウィンドウの上に出て一緒に破棄される。docs/adr/0062)。`Dialog` のときは、コンストラクタが親のウィンドウを受け取る
(`explicit SettingsDialog(tokotachi::Window* parent);`、`: tokotachi::Dialog(parent, std::string())`)。
コンストラクタは区間の外にあるので、この形は新しく生成するファイルにだけ効く(既に生成したファイルは利用者が直す)。

利用側(プロジェクトの起動部分。§8):

```cpp
#include <tokotachi/tokotachi.hpp>
#include "main_window.hpp"

int main(int argc, char** argv)
{
    tokotachi::Application app(argc, argv);
    MainWindow* window_main = new MainWindow();   // Application が所有する(docs/adr/0062)
    app.main_window = window_main;
    window_main->visible = true;
    return app.exec();
}
```

## 4. `tokotachi_setup_ui` の中の順序

1. **Action と画面に出ないオブジェクトと Variable**(定義ファイルの順)。メニュー・ツールバー・部品から参照・結びつけされるので先に作る。
   Variable は `new tokotachi::Variable<T>(this, 初期値)`(初期値を書かなければ `(this)`)。
2. **ウィンドウ自身のプロパティとハンドラ。**
3. **メニューバー・ツールバー・ステータスバー**(作って中身を入れてから、ウィンドウに設定する)。
4. **中身の部品を木の順(親が先)に。** 部品ごとに次の順で書く。
   1. 作る(`new tokotachi::Button(親)`。親を渡して、作った時点から親が所有する)。
   2. プロパティ(カタログの順。宣言の順なので、範囲を持つ部品の `value` は `minimum`・`maximum` の後、ComboBox の `current_index` は `items` の後になる)。
      Variable に結びつけたプロパティ(dsl-spec.md §5.2)は、同じ位置で `tokotachi::bind(部品->プロパティ, 変数->value);` にする
      (結んだ時点で値が設定されるので、順序の意味は値の設定と同じ)。
      ただし、カタログで `after_children` の印が付いたもの(`TabWidget::current_index`・`Splitter::sizes`。タブ・区画が無いと効かない)は除き、5 の後に設定する。
   3. ハンドラをつなぐ。
   4. `actions`(`add_action`)と、一覧の列(`columns`。`add_column(title, width)`。width が既定の 120 なら省き、alignment 等は結果に設定する)。
   5. 自分のレイアウトと子(`layout`・`children`・`tabs`・`panes`・`content`)。ここで再帰する。
   6. `after_children` の印が付いたプロパティ。
   7. 親のレイアウトに入れる(`add`)。子の設定がすべて済んでから入れる。
5. **部品への参照**(ほかの部品を指すプロパティ)。相手がすべてできてから、最後にまとめて設定する。

この順で生成したウィンドウの配置が、キャンバス(同じ定義ファイルを WebAssembly の Tokotachi で組んだもの)と同じになることを、生成したコードの実行で確かめる(§7)。

### 4.1 部品ファイル([ADR 0043](../adr/0043-designer-component-files.md))

- 部品ファイル(根が `Widget`)は `class LabeledEdit : public tokotachi::Widget` にし、コンストラクタは
  `explicit LabeledEdit(tokotachi::Widget* parent)`(`: tokotachi::Widget(parent)`)。根の `width`・`height` は書かない。
- 部品ファイルを置いた部品は `panel_user = new LabeledEdit(this);` とし、メンバの型は `LabeledEdit*`。
  ヘッダで部品ファイルのヘッダを include する(ヘッダのフォルダからの相対パス。部品ファイルの出力先は、部品ファイルが属するプロジェクトの設定で決まる)。
- 部品ファイルの中身(子の部品・ハンドラ)は部品ファイルのクラスのコンストラクタが組むので、置いた側の順は部品の 1 と同じ位置で、
  その後に置いた側のプロパティを設定する。
- キャンバスでは、部品ファイルのクラスが WebAssembly の Tokotachi に無いので、置いた部品を Widget として作り、部品ファイルの操作の並びを
  変数の名前に `置いた部品の名前.` を付けて続ける(ハンドラはつながない。画像のパスは置いた側のフォルダからにする)。
  順は生成したコードと同じなので、描いた画面は同じになる(見本の settings_dialog が置く parts/labeled_edit で確かめている)。

### 4.2 自作部品([ADR 0044](../adr/0044-designer-custom-widgets.md))

- 自作部品は名前空間を付けた名前で作る(`level_bar_zoom = new samples::LevelBar(area_free);`。メンバの型も `samples::LevelBar*`)。
  プロパティ・ハンドラはライブラリの部品と同じ書き方(ヘッダから読んだ型で値を書く)。
- ヘッダから自作部品のヘッダを include する(ヘッダのフォルダからの相対パス。登録したヘッダはプロジェクトのフォルダからのパスなので、
  `CppOptions.projectDir` でたどる)。プロジェクトに属さないウィンドウで使えばエラーにする。
- 見本(`designer/samples` はプロジェクト `samples.ttproj.json`)の settings_dialog が `widgets/level_bar.hpp` の LevelBar を置き、
  `pnpm codegen:verify-cpp`・`codegen:verify-project` でビルド・実行し、`canvas:verify` でキャンバス(基底の ProgressBar で描く)と画素まで同じことを確かめる。

## 5. 書き出しの決まり

| # | 論点 | 案 |
|---|---|---|
| N1 | ハンドラのつなぎ方 | `[this](引数...) { メンバ関数(引数...); }` のラムダ。引数はカタログのハンドラの型から作る |
| N2 | 名前空間 | 生成したコードは `tokotachi::` を省かずに書く(ヘッダにもソースにも `using namespace` を書かない。利用者の名前と衝突させないため) |
| N3 | 名前の無いレイアウト・メニューバー等 | 一時的な変数にする。名前は `<親の名前>_layout`(ウィンドウは `window_layout`)、`window_menu_bar`・`window_tool_bar`・`window_status_bar`、名前の無いメニューは `<親>_menu_<番号>`、プロパティを設定するタブ・区画・列は `<部品の名前>_tab_<番号>`・`_pane_<番号>`・`_column_<番号>`(番号は 1 から)。これらの名前はノードの `name` に使えない(検証でエラーにする)。実装は [tree.ts](../../designer/packages/core/src/dsl/tree.ts) の `temporaryNames` |
| N4 | 値の書き出し | 文字列は UTF-8 のまま C++ の文字列リテラルに(`"`・`\`・制御文字はエスケープ)。`float` の小数は `0.5f`。列挙型は `tokotachi::Orientation::HORIZONTAL`。`Margins` は `tokotachi::Margins(12)` か 4 つの値、`Size` 等は値をそろえたコンストラクタ、`ThemeColor` の役割は `tokotachi::ColorRole::ACCENT`・色は `tokotachi::Color(0xFF, 0x80, 0x00)`、`Shortcut` は `tokotachi::Shortcut("Ctrl+S")`、配列は `std::vector<std::string>{...}`。`Placement` は一時的な変数にメンバを設定して `add` に渡す |
| N5 | 既定値 | 定義ファイルに書いたプロパティだけを書き出す |
| N6 | レイアウトの `add` の引数 | 既定値と同じ後ろの引数は省く(`add(w)`、`add(w, 1)`)。途中の引数を書くときは、その前の引数も書く |
| N7 | 予約の名前 | 生成したコードが使うメンバ関数・区間には接頭辞 `tokotachi_` を付ける(`tokotachi_setup_ui`)。ノードの `name`・ハンドラの名前に `tokotachi_` で始まる名前は使えない |
| N8 | 画像 | 定義ファイルが参照する画像のファイル(dsl-spec.md §5.1)の中身を、別のソース `<定義ファイルの名前>_images.cpp`(ソースと同じフォルダ。ファイル全体を区間 `tokotachi_images` にして、生成し直すと上書きする)に、ウィンドウと同じ名前空間の中の `tokotachi_images_<クラス名>` の名前空間の `const unsigned char tokotachi_image_<パス>[]` と大きさ `tokotachi_image_<パス>_size` として書く(同じファイルは 1 つ。1 行に 16 バイト。別の翻訳単位から使うので、先に `extern` で宣言する)。ウィンドウのソースは区間 `tokotachi_setup_ui` の先頭で同じものを `extern` で宣言し、`tokotachi::Image::load_from_data(配列, 大きさ)` で読む。画像を参照しないウィンドウは画像のソースを書かない(前に書いたものが残っていれば、使われていないと知らせる)。プロジェクトに属すれば CMakeLists.txt のソースの一覧に入れ、属さなければ作ったときに、ビルドに加えるよう知らせる。読めない画像があれば、何も書かずにエラーにする。実装は [cpp/images.ts](../../designer/packages/codegen/src/cpp/images.ts)([ADR 0053](../adr/0053-designer-image-source-file.md)) |
| N9 | Variable | メンバは `tokotachi::Variable<std::string>* file_name;`(`StringVariable` 等の typedef は float に無いので使わず、型の引数で書く)。ハンドラ(`on_set`・`on_change`)はほかのオブジェクトと同じくつなぐ |

## 6. Bethany から引き継ぐ決まり

| # | 論点 | 決定 |
|---|---|---|
| M1 | ハンドラの宣言と中身の置き場所 | 宣言は `declarations` の区間に入れる。中身の雛形は `handler-stubs` の行の後に(生成する順に)足すだけで、消さない(定義ファイルから消えたハンドラの中身は、宣言が無くなってコンパイルエラーになるので気づける) |
| M2 | 区間の中の手での変更 | 終わりのマーカーに区間の中身のハッシュを持たせ、生成し直すときに合わなければ、上書きする前に警告して確かめる |
| M3 | 整形の道具との共存 | ハッシュは空白(改行・字下げを含む)を取り除いてから求める(clang-format で整形しても手での変更とみなさない) |
| M4 | 壊れたマーカー | マーカーが欠けている・重なっている・入れ子になっている・対応していないときは、何も書かずにエラーにする |
| M7 | 名前を変えたとき | 警告だけにする。区間の外の参照は書き換えない(無くなった名前が区間の外で使われていれば警告する) |
| M8 | VS Code での書き込み | WorkspaceEdit で書き込む。書き込む前に保存していない変更が無かったファイルは、書き込んだ後に保存する |

## 7. 確かめ方

- **期待する出力との比較**: 定義ファイル → 生成したコードを、codegen のテストで期待するファイル(`packages/codegen/src/__golden__/`。
  `pnpm codegen:golden` で作り直す)と比べる。区間の置き換え・利用者のコードの保存・手での変更の検出・雛形の追加・壊れたマーカー・
  消えた名前の警告もテストする。
- **ビルドと実行**: 見本の定義ファイル(`designer/samples/`)から生成した C++ を、本リポジトリの Tokotachi と一緒に CMake でビルドして実行し、次を確かめる。
  1. 表示した後の各部品の位置と大きさが、キャンバス(WebAssembly で同じ定義ファイルから組んだもの)と同じであること。
  2. ハンドラがつながっていること(ボタン・Action・Timer のハンドラが呼ばれる)。
  3. 参照(メニュー・ツールバーの Action)・値の型(Margins・Font・色・Shortcut 等)・配列が設定されていること。
- 生成したコードは `-Wall -Wextra -pedantic -Werror`(使っていない引数を除く)でコンパイルする。
- 実装は [designer/tools/codegen/verify-cpp.mts](../../designer/tools/codegen/verify-cpp.mts)(`pnpm codegen:verify-cpp`)。
  ハンドラの雛形の中身を「呼ばれたことを記録する処理」に書き換えてビルドし、画面なし(offscreen)で開いて 39 の項目を確かめる
  (埋め込んだ画像・Variable との結びつけ・一覧の列の確認を 2026-10-10 に加えた)。
  2026-10-10 に Windows(MinGW)と Linux(WSL。生成したファイルを CMake で直接ビルド)で、すべて通った。

## 8. プロジェクトと設定

定義ファイルは画面の設計だけを持ち、生成の設定はプロジェクトファイル(`*.ttproj.json`)に持つ。
どのプロジェクトにも属さないウィンドウは、拡張の設定か CLI(`ttd generate`)の引数で渡す(`--comment-locale`・`--namespace`・`--header-dir`・`--source-dir`)。
仕様は [project-spec.md](project-spec.md)。持つ設定は次のとおり。

| 設定 | 既定 |
|---|---|
| 名前空間(`app::ui` なら `namespace app { namespace ui {` と 1 つずつ書く。C++11 でも通るように) | なし |
| インクルードガードの接頭辞 | なし |
| ヘッダ・ソースの拡張子とフォルダ | `.hpp`・`.cpp`、定義ファイルと同じフォルダ |
| コメントの言語 | 英語(日本語も選べる) |
| メインのウィンドウと、起動部分(`main.cpp`)を生成するか | — |

## 9. 未決の論点

| # | 論点 | 案 |
|---|---|---|
| G1 | 新しいプロジェクトで CMakeLists.txt も作るか | **決定(2026-10-10)**: 作る(`find_package(tokotachi)` と生成したファイルを並べた最小のもの)。すぐにビルドして動かせるように。一度作ったら上書きしない |
| G2 | ハンドラをメンバ関数にせず、ラムダの中身を書けるようにするか | しない(dsl-spec.md §10 Q8) |
| G3 | 生成したメンバの初期化 | コンストラクタの中で `tokotachi_setup_ui` が必ず設定するので、宣言では初期化しない。`-Weffc++` 等で警告が出るなら `= nullptr` を付ける |
