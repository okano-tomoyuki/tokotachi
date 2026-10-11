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
| `format_version` | ○ | `1` |
| `codegen` | | コード生成の設定(§4) |
| `theme` | | アプリケーションのテーマ(§4.1) |
| `main_window` | | 起動したときに開くウィンドウ。`windows` のどれか(class が `Window` のもの) |
| `windows` | | プロジェクトに属するウィンドウの定義ファイル |

- パスは、**プロジェクトファイルのあるフォルダからの相対パス**で、区切りは `/`。`../` で上のフォルダも指せる。絶対パス・`\`・ドライブ名は使えない。

### 意味の検証

| 内容 | 重大度 |
|---|---|
| `windows` の各要素が `.ttui.json` で終わる相対パスであること | エラー |
| `windows` に同じウィンドウが 2 回ないこと | エラー |
| `main_window` が `windows` のどれかであること | エラー |
| `codegen.namespace` の `::` で区切った各部分が識別子であること | エラー |
| `codegen.include_guard_prefix` が英字・数字・`_` だけで、数字で始まらないこと | エラー |
| `codegen.header_dir`・`source_dir` が相対パスであること | エラー |
| `header_dir`・`source_dir` を使うとき、同じファイル名のウィンドウがないこと(出力先が重なる) | エラー |

書き出しの形: キーの順は `$schema`・`format_version`・`codegen`・`theme`・`main_window`・`windows`。一覧は 1 行に 1 つ。インデントは 2 文字、改行は LF、末尾に改行。

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
  メインのウィンドウを表示する(区間 `tokotachi_main`)。
- **CMakeLists.txt**: 最初に雛形を作る(`find_package(tokotachi)`・`add_executable`・`target_link_libraries`)。その後は**ソースの一覧の区間
  (`tokotachi_sources`。`TOKOTACHI_DESIGNER_SOURCES` と `TOKOTACHI_DESIGNER_INCLUDE_DIRS`)だけを置き換え**、利用者が書いた部分は変えない。
  ターゲットの名前はプロジェクトの名前(ファイルの名前。`my_app.ttproj.json` → `my_app`)。
- 生成した main.cpp・CMakeLists.txt でビルドできることを、`pnpm codegen:verify-project` で確かめる(見本の 2 つのウィンドウを `ui/` に置き、
  ヘッダを `include/`、ソースを `src/`、名前空間を `app` にしたプロジェクトを作り、インストールした Tokotachi でビルドする)。

## 6. デザイナーでの扱い

- **プロジェクトのビュー**(アクティビティバーの Tokotachi Designer): プロジェクトごとにウィンドウを並べ、メインのウィンドウに ★ を付ける。
  どのプロジェクトにも属さないウィンドウは「プロジェクトに属さないウィンドウ」にまとめる。ファイルを作る・消す・変えると描き直す。
  - プロジェクト: 開く、プロジェクトのコードを生成(すべてのウィンドウと起動部分)、アプリケーションの配色を選ぶ(`theme.color_scheme`)。
  - ウィンドウ: デザイナーで開く、コードを生成、メインのウィンドウに設定(プロジェクトのウィンドウ)、プロジェクトに追加(属さないウィンドウ)。
  - プロジェクトが無いときは「新しいプロジェクト」のボタンを出す。
- **新しいプロジェクト**: 名前(CMake のターゲットの名前にもなる。英字・数字・`_`・`-`)と作る場所を入力してもらい、
  `<名前>.ttproj.json`・`main_window.ttui.json` を作って、ウィンドウ・main.cpp・CMakeLists.txt を生成し、ウィンドウを開く。
  コメントの言語は VS Code の表示言語に合わせる。
- **新しいウィンドウ**: 同じフォルダか上のフォルダにプロジェクトがあれば、そのいちばん近いものに加える(最初のウィンドウはメインのウィンドウにもする)。
- **保存したときの生成**: ウィンドウはそのウィンドウのコードを、プロジェクトファイルは起動部分を生成する(`generate_on_save`)。
- プロジェクトファイルの編集はテキストで行う(JSON Schema で補完する)。検証の結果は問題パネルに出す。
