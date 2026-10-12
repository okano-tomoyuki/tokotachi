# プロジェクトファイルの仕様(format_version 1)

アプリケーションに属するウィンドウと、メインのウィンドウ、コード生成の設定を表すファイル `*.ttproj.json` の仕様。
Bethany の project-spec.md を土台にし、Tokotachi に要るものに絞った。
構造は Zod のスキーマ([designer/packages/core/src/project/schema.ts](../../designer/packages/core/src/project/schema.ts))、意味の検証は
[parse.ts](../../designer/packages/core/src/project/parse.ts) を正とし、本書と食い違う場合は本書を直す。

## 1. 基本方針

- **プロジェクトがウィンドウの一覧を持つ**(ウィンドウの定義ファイルはプロジェクトを知らない)。
- **生成の設定はプロジェクトに持つ。** プロジェクトに属するウィンドウは、プロジェクトの `codegen` で生成する。
  どのプロジェクトにも属さないウィンドウは、拡張の設定(`tokotachiDesigner.codegen.*`)・CLI の引数で生成する。
- **起動部分(main.cpp)と CMakeLists.txt も生成する**(§5)。新しいプロジェクトを作ったら、すぐにビルドして動かせるようにする
  (codegen-design.md §9 G1)。

## 2. 例

```json
{
  "format_version": 1,
  "codegen": {
    "comment_locale": "ja",
    "namespace": "app",
    "header_dir": "include",
    "source_dir": "src"
  },
  "main_window": "ui/main_window.ttui.json",
  "windows": [
    "ui/main_window.ttui.json",
    "ui/settings_dialog.ttui.json"
  ]
}
```

## 3. ドキュメント

| キー | 必須 | 内容 |
|---|---|---|
| `$schema` | | JSON Schema の場所(エディタの補完用) |
| `format_version` | ○ | `1`(形式の版。移し替えは README.md の「定義ファイルの形式」。ADR 0064) |
| `codegen` | | コード生成の設定(§4) |
| `theme` | | アプリケーションのテーマ(§4.1) |
| `main_window` | | 起動したときに開くウィンドウ。`windows` のどれか(class が `Window` のもの) |
| `windows` | | プロジェクトに属するウィンドウの定義ファイル。部品ファイル([ADR 0043](../adr/0043-designer-component-files.md))も入れ、同じ規則で生成して CMakeLists.txt のソースに加える |
| `custom_widgets` | | 自作部品のヘッダ([ADR 0044](../adr/0044-designer-custom-widgets.md)。catalog.md §4.1)。`.h`・`.hpp`・`.hh`・`.hxx` |

- パスは、**プロジェクトファイルのあるフォルダからの相対パス**で、区切りは `/`。`../` で上のフォルダも指せる。絶対パス・`\`・ドライブ名は使えない。

### 意味の検証

| 内容 | 重大度 |
|---|---|
| `windows` の各要素が `.ttui.json` で終わる相対パスであること | エラー |
| `custom_widgets` の各要素が C++ のヘッダの相対パスで、重ならず、ファイルが有ること(ヘッダの中の読めないクラス・プロパティは警告) | エラー |
| `windows` に同じウィンドウが 2 回ないこと | エラー |
| `main_window` が `windows` のどれかであること | エラー |
| `codegen.namespace` の `::` で区切った各部分が識別子であること | エラー |
| `codegen.include_guard_prefix` が英字・数字・`_` だけで、数字で始まらないこと | エラー |
| `codegen.header_dir`・`source_dir` が相対パスであること | エラー |
| `header_dir`・`source_dir` を使うとき、同じファイル名のウィンドウがないこと(出力先が重なる) | エラー |

書き出しの形: キーの順は `$schema`・`format_version`・`codegen`・`theme`・`main_window`・`windows`・`custom_widgets`。一覧は 1 行に 1 つ。インデントは 2 文字、改行は LF、末尾に改行。

## 4. コード生成の設定(`codegen`)

| キー | 既定 | 内容 |
|---|---|---|
| `comment_locale` | `en` | 生成するコードのコメントの言語(`en`・`ja`) |
| `generate_on_save` | `true` | ウィンドウ・プロジェクトファイルを保存したときにコードを生成するか |
| `namespace` | なし | 生成するクラスの名前空間(`app::ui` は `namespace app { namespace ui {` と 1 つずつ書く) |
| `include_guard_prefix` | なし | インクルードガードのマクロの名前の先頭に付ける文字 |
| `header_dir`・`source_dir` | ウィンドウと同じフォルダ | ウィンドウのヘッダ・ソースを置くフォルダ(プロジェクトのフォルダからの相対パス) |

### 4.1 テーマ(`theme`)

```json
"theme": {
  "color_scheme": "DARK",
  "override": { "colors": { "ACCENT": "#7C3AED" }, "field_radius": 2 },
  "presets": {
    "danger": { "colors": { "ACCENT": "#DC2626", "FIELD": { "light": "#FEF2F2", "dark": "#450A0A" } } },
    "compact": { "radius": 0, "field_padding": 4 }
  }
}
```

| キー | 既定 | 内容 |
|---|---|---|
| `color_scheme` | `SYSTEM` | ライト・ダークの選び方(`SYSTEM`: OS の設定に従う・`LIGHT`・`DARK`) |
| `override` | なし | 標準のテーマ(ライト・ダーク)に重ねる上書き(部品の `theme_override` と同じ書き方。dsl-spec.md §5.3) |
| `presets` | なし | 名前を付けて登録したテーマ(名前 → 上書き)。部品の `theme_override` から `{ "preset": "名前" }` で参照する(§4.2) |

- 生成する main.cpp で、`app.color_scheme` を設定し、`Theme::light()`・`Theme::dark()` に上書きを重ねて `app.light_theme`・`app.dark_theme` にする。
- デザイナーのキャンバスは、プロジェクトに属するウィンドウをこのテーマで描く。`color_scheme` が `SYSTEM`(か、プロジェクトに属さない)なら、
  実行するまで OS の設定は分からないので、拡張の設定 `tokotachiDesigner.canvas.colorScheme`(既定は VS Code の配色テーマに合わせる)で決める。
- `override`・`presets` の役割・色・寸法の誤りはエラーにする(JSON Schema でも補完する)。`override`・`presets` には `preset` を書けない。

### 4.2 登録したテーマ(`presets`)

共通の自作テーマを、名前を付けてプロジェクトに登録し、部品から参照する(部品の種類によらず、入れ物に設定すれば中の部品すべてに効く)。

- 名前は識別子(生成する関数の名前 `<名前>_theme` にもなる)。
- プロジェクトの生成で、`header_dir`(無ければプロジェクトのフォルダ)に `tokotachi_themes.hpp` を作り、テーマごとに
  `inline tokotachi::ThemeOverride <名前>_theme()` を置く(名前空間は `codegen.namespace`)。テーマを登録していなければ作らない。
- 参照する部品は `<名前>_theme()` から始めて、部品に書いた色・寸法をつなげる(`danger_theme().set_radius(0)`)。
  そのウィンドウのソースは `tokotachi_themes.hpp` を include する。テーマの中身を変えても、ウィンドウのコードは変わらない。
- 参照できるのはプロジェクトに属するウィンドウだけ。登録していない名前・プロジェクトに属さないウィンドウでの参照はエラー
  (問題パネル。コードは生成しない)。キャンバスは、参照を登録した中身に展開して描く。

## 5. 起動部分と CMakeLists.txt

プロジェクトファイルと同じフォルダに生成する。実装は [designer/packages/codegen/src/project.ts](../../designer/packages/codegen/src/project.ts)。

- **main.cpp**: Tokotachi と、メインのウィンドウのヘッダを include し(区間 `tokotachi_includes`)、`main` の中で Application を作って
  メインのウィンドウを `new` で作り(Application が所有する)、`app.main_window` に代入して表示する(区間 `tokotachi_main`。
  メインのウィンドウを閉じると終える。docs/adr/0062)。
- **CMakeLists.txt**: 最初に雛形を作る(`find_package(tokotachi)`・`add_executable`・`target_link_libraries`)。その後は**ソースの一覧の区間
  (`tokotachi_sources`。`TOKOTACHI_DESIGNER_SOURCES` と `TOKOTACHI_DESIGNER_INCLUDE_DIRS`)だけを置き換え**、利用者が書いた部分は変えない。
  ターゲットの名前はプロジェクトの名前(ファイルの名前。`my_app.ttproj.json` → `my_app`)。
  自作部品のソース(`.cpp`)は区間の外に利用者が足す(`target_sources(my_app PRIVATE widgets/gauge.cpp)`)。
  画像を参照するウィンドウの画像のソース(`main_window_images.cpp`。codegen-design.md N8)は、ウィンドウのソースの次に一覧に入れる。
  `find_package` で求める版は、カタログを取り出したライブラリの版の major.minor(`find_package(tokotachi 0.2 REQUIRED)`。
  0.x の間は小さい版まで一致を求める。docs/adr/0061)。雛形は作り直さないので、ライブラリの小さい版が上がったら利用者が直す。
- 生成した main.cpp・CMakeLists.txt でビルドできることを、`pnpm codegen:verify-project` で確かめる(見本のウィンドウ・部品ファイルを `ui/` に、自作部品を `widgets/` に置き、
  ヘッダを `include/`、ソースを `src/`、名前空間を `app` にしたプロジェクトを作り、インストールした Tokotachi でビルドする)。

## 6. デザイナーでの扱い

- **プロジェクトのビュー**(アクティビティバーの Tokotachi Designer): プロジェクトごとにウィンドウを並べ、メインのウィンドウに ★ を付ける。
  どのプロジェクトにも属さないウィンドウは「プロジェクトに属さないウィンドウ」にまとめる。ファイルを作る・消す・変えると描き直す。
  - プロジェクト: 開く(設定画面)、プロジェクトのコードを生成(すべてのウィンドウと起動部分)、アプリケーションの配色を選ぶ(`theme.color_scheme`)。
  - ウィンドウ: デザイナーで開く、コードを生成、メインのウィンドウに設定(プロジェクトのウィンドウ)、プロジェクトに追加(属さないウィンドウ)。
  - 生成したコードへのリンク: プロジェクトの下に起動部分(`main.cpp`・`CMakeLists.txt`、テーマを登録していれば `tokotachi_themes.hpp`)、
    ウィンドウの下にヘッダ・ソースを出し、クリックで開く。出力先はコード生成と同じ規則で決め(プロジェクトに属さないウィンドウは拡張の設定)、
    まだ無いファイルには「未生成」と付ける。
  - プロジェクトが無いときは「新しいプロジェクト」のボタンを出す。
- **新しいプロジェクト**: 名前(CMake のターゲットの名前にもなる。英字・数字・`_`・`-`)と作る場所を入力してもらい、
  `<名前>.ttproj.json`・`main_window.ttui.json` を作って、ウィンドウ・main.cpp・CMakeLists.txt を生成し、ウィンドウを開く。
  コメントの言語は VS Code の表示言語に合わせる。
- **新しいウィンドウ**: 作る場所と加えるプロジェクトは、呼んだ場所で決める(最初のウィンドウはメインのウィンドウにもする)。
  - エクスプローラーのフォルダ: そのフォルダに作り、同じか上のフォルダのいちばん近いプロジェクトに加える。
  - プロジェクトのビューのプロジェクト(行の「+」): そのプロジェクトのフォルダに作って加える。
  - それ以外(ビューの上の「+」・コマンドパレット・「新しいファイル」): プロジェクトが 1 つならそこへ、複数あれば選んでもらう
    (「プロジェクトに属さない」ならワークスペースの直下に作る)。
- **保存したときの生成**: ウィンドウはそのウィンドウのコードを、プロジェクトファイルは起動部分を生成する(`generate_on_save`)。
- **プロジェクトの設定画面**([projectEditorProvider.ts](../../designer/packages/extension/src/projectEditorProvider.ts)・
  [ProjectApp.tsx](../../designer/packages/webview/src/project/ProjectApp.tsx)): `*.ttproj.json` は既定でこの画面で開く(「テキストで開く」で
  JSON を直接編集できる。JSON Schema で補完する)。デザイナーと同じく TextDocument を唯一の正とし、画面の編集は決まった形
  (`serializeProject`)で書き出して変わった範囲だけを置き換える(Undo が効き、保存は利用者が行う)。検証の結果は問題パネルと画面の欄の下に出す。
  - ウィンドウ: 一覧(クリックでデザイナーで開く)、メインのウィンドウの選択、プロジェクトから外す(ファイルは残す)、
    ワークスペースのまだ属さない定義ファイルから加える。
  - コード生成: `codegen` の各項目。空・既定の値は書かない。
  - テーマ: 配色(`color_scheme`)、プロジェクトのテーマ(`override`)と登録したテーマ(`presets`)を選んで編集する
    (色・寸法の入力欄はインスペクタの theme_override と同じ)。登録したテーマには参照している部品の数を出す。
    - 改名すると、プロジェクトのウィンドウの参照(`preset`)も同じ 1 回の編集で書き換える。消すときは、参照があれば確かめてから参照を外す
      (部品に書いた色・寸法は残す)。書き換える前に保存していたウィンドウは保存する。
    - 見本: 主な部品を並べた見本のウィンドウ([themePreview.ts](../../designer/packages/core/src/project/themePreview.ts))を、
      キャンバスと同じ WebAssembly の Tokotachi で、選んだテーマを当ててライト・ダークの両方で描く(プロジェクトのテーマはどちらにも重ねる)。
