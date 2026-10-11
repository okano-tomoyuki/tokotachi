# 画面の定義ファイルの仕様(format_version 1)

デザイナーが読み書きする、ウィンドウ 1 つ分の定義ファイルの仕様。§10 の未決の論点は、実装しながら決める。
方針は [ADR 0039](../adr/0039-designer.md)。Bethany の DSL(Bethany の docs/designer/dsl-spec.md)を土台にし、配置の表し方を Tokotachi に合わせた。

構造は Zod のスキーマ([designer/packages/core/src/dsl/schema.ts](../../designer/packages/core/src/dsl/schema.ts))、意味の検証は
[validate.ts](../../designer/packages/core/src/dsl/validate.ts) を正とし、本書と食い違う場合は本書を直す。
見本は [designer/samples/](../../designer/samples/)(`main_window.ttui.json`・`settings_dialog.ttui.json`)。

## 1. 基本方針

- **Tokotachi の公開の API の語彙をそのまま使う。** クラス名・プロパティ名・ハンドラ名・列挙型の値は C++ と同じ
  (`Button`・`text`・`on_click`・`VERTICAL`)。ファイルの構造のキーも、API に合わせて snake_case にする。
- **1 ファイル = 1 ウィンドウ = 生成する 1 クラス**(`tokotachi::Window` か `tokotachi::Dialog` の派生。ADR 0039)。
- **配置は座標ではなくレイアウトで表す。** 子を持つ部品には、レイアウト(BoxLayout・GridLayout・PlaceLayout)を 1 つ書き、
  その項目として子を並べる(ADR 0017)。項目ごとの設定(伸びる割合・行と列・置き方)は、`add` の引数と同じく、子の部品ではなく項目に書く。
  Qt Designer の .ui と同じ考え方。
- **親で子の置き方が決まる部品**(TabWidget のタブ・Splitter の区画・ScrollArea の中身・ウィンドウのメニューバー等)は、それぞれ専用のキーで書く。
- **設定したプロパティ(既定値から変えたもの)だけを書き、ハンドラにはメンバ関数の名前を書く**(.dfm・Bethany と同じ)。
- JSON で書く。拡張が保存するときは決まった形で書き出す(§8)。

## 2. 例

```json
{
  "format_version": 1,
  "window": {
    "name": "MainWindow",
    "class": "Window",
    "properties": { "title": "メモ", "width": 640, "height": 480 },
    "events": { "on_close": "main_window_close" },
    "menu_bar": {
      "menus": [
        {
          "name": "menu_file",
          "title": "ファイル(&F)",
          "items": [{ "action": "action_open" }, { "separator": true }, { "action": "action_quit" }]
        }
      ]
    },
    "tool_bar": {
      "properties": { "button_style": "TEXT_BESIDE_ICON" },
      "items": [{ "action": "action_open" }]
    },
    "status_bar": {},
    "layout": {
      "class": "BoxLayout",
      "properties": { "margins": 12 },
      "items": [
        {
          "widget": {
            "name": "row_search",
            "class": "Widget",
            "layout": {
              "class": "BoxLayout",
              "properties": { "orientation": "HORIZONTAL" },
              "items": [
                { "widget": { "name": "line_edit_search", "class": "LineEdit", "properties": { "placeholder_text": "検索" } }, "stretch": 1 },
                { "widget": { "name": "button_search", "class": "Button", "properties": { "text": "検索" }, "events": { "on_click": "button_search_click" } } }
              ]
            }
          }
        },
        {
          "widget": {
            "name": "splitter_main",
            "class": "Splitter",
            "properties": { "sizes": [160, 360] },
            "panes": [
              { "properties": { "stretch": 0 }, "widget": { "name": "tree_view_folders", "class": "TreeView" } },
              { "widget": { "name": "list_view_files", "class": "ListView" } }
            ]
          },
          "stretch": 1
        }
      ]
    }
  },
  "actions": [
    {
      "name": "action_open",
      "properties": { "text": "開く(&O)...", "shortcut": "Ctrl+O" },
      "events": { "on_trigger": "action_open_trigger" }
    },
    { "name": "action_quit", "properties": { "text": "終了(&X)" }, "events": { "on_trigger": "action_quit_trigger" } }
  ],
  "objects": [
    { "name": "autosave_timer", "class": "Timer", "properties": { "interval": 60000 }, "events": { "on_timer": "autosave" } }
  ]
}
```

## 3. ドキュメント

| キー | 必須 | 型 | 説明 |
|---|---|---|---|
| `format_version` | ○ | `1` | 形式の版。形式を変えたときに上げ、古い形式からの移し替えに使う |
| `window` | ○ | ウィンドウのノード | §4.1 |
| `actions` | | Action のノードの配列 | §4.6。メニュー・ツールバー・部品の右クリックから名前で参照する |
| `objects` | | 画面に出ないオブジェクトのノードの配列 | §4.7(Timer 等) |
| `variables` | | Variable のノードの配列 | §4.8。プロパティを結びつける(§5.2) |
| `$schema` | | 文字列 | エディタ向け。内容には影響しない |

## 4. ノード

部品・Action・オブジェクトのノードは `name` を持ち、生成するクラスのメンバ(ポインタ)の名前になる。
`name` はファイルの中で重複できず、C++ の識別子として正しく(ASCII のみ)、予約語・Window のメンバの名前(カタログ)・
生成したコードが使う名前と衝突してはならない。名前は snake_case を勧める(API に合わせる。検証では警告にとどめる)。

レイアウト・タブ・区画のノードの `name` は省略できる。省略すると、生成したコードではコンストラクタの中の一時的な変数になる。

### 4.1 ウィンドウのノード

| キー | 必須 | 型 | 説明 |
|---|---|---|---|
| `name` | ○ | 識別子 | **生成するクラスの名前**(PascalCase を勧める。`MainWindow`) |
| `class` | ○ | `"Window"`・`"Dialog"`・`"Widget"` | 生成するクラスの基底クラス。`Widget` は部品ファイル(§4.9) |
| `properties` | | 名前 → 値 | §5。ウィンドウ自身のプロパティ(`title`・`width`・`height`・`background` 等) |
| `events` | | 名前 → ハンドラ名 | §6 |
| `menu_bar` | | メニューバーのノード | §4.5。`Window::menu_bar` に設定する |
| `tool_bar` | | ツールバーのノード | §4.5。`Window::tool_bar` に設定する |
| `status_bar` | | ステータスバーのノード | §4.5。`Window::status_bar` に設定する |
| `layout` | | レイアウトのノード | §4.3。ウィンドウの中身 |
| `buttons` | | ボタンの配列 | `class` が `Dialog` のときだけ書ける。`{ "button": "OK" }`・`{ "button": "CANCEL", "text": "やめる" }`(`Dialog::add_button`) |

- `width`・`height` はウィンドウの中身(枠・タイトルバーを含まない)の大きさ。メニューバー・ツールバー・ステータスバーは、その中に含まれる。
- `Dialog` の生成するクラスは、コンストラクタで親のウィンドウを受け取る(`Dialog(Window* parent, const std::string& title)`)。
  Dialog は開くときに大きさを中身に合わせるので、大きさは `width`・`height` ではなく `minimum_size` で決める。

### 4.2 部品のノード

| キー | 必須 | 型 | 説明 |
|---|---|---|---|
| `name` | ○ | 識別子 | |
| `class` | ○ | クラス名 | カタログにある、利用者が作れる Widget の派生(`Button`・`Widget` 等) |
| `component` | | 相対パス | 部品ファイル(§4.9)を置く。`class` は `Widget` にし、`layout`・`children` 等の子は書けない |
| `properties` | | 名前 → 値 | §5 |
| `events` | | 名前 → ハンドラ名 | §6 |
| `actions` | | Action の名前の配列 | `Widget::add_action`(右クリックのメニュー等に出る Action) |
| `layout` | | レイアウトのノード | §4.3。子を持てる部品(カタログの `accepts_children`)だけが書ける |
| `children` | | 部品のノードの配列 | §4.4。レイアウトを付けない自由な配置。`layout` と一緒には書けない |
| `tabs` | | タブのノードの配列 | `TabWidget` だけ。`{ "name"?, "properties": { "title": "一般" }, "page": 部品のノード }`(`add_tab`) |
| `panes` | | 区画のノードの配列 | `Splitter` だけ。`{ "name"?, "properties": { "stretch": 0, "collapsible": true }, "widget": 部品のノード }`(`add_widget`) |
| `content` | | 部品のノード | `ScrollArea` だけ(`ScrollArea::content`) |
| `columns` | | 列のノードの配列 | `ListView`・`TreeView` だけ。`{ "name"?, "properties": { "title": "名前", "width": 200, "alignment": "END" } }`(`add_column`。§4.2.1) |

- 子を持つ部品を組むには、ふつう `class` を `Widget` にしてレイアウトを書く(上の例の `row_search`)。
- 列(`columns`)は子の部品ではないが、タブ・区画と同じく専用のキーで書く(§4.2.1)。
- 配列の順が生成の順になる。フォーカスを Tab キーで移す順にも影響する。

#### 4.2.1 一覧の列(`columns`)

ListView・TreeView の列(`ItemView::add_column`)。配列の順が列の順。`properties` は Column のプロパティ(`title`・`width`・`alignment`)で、
`title`・`width` は `add_column` の引数で渡し、それ以外は結果に設定する。列が無ければ見出しの無い 1 列の一覧になる。

```json
"columns": [
  { "properties": { "title": "名前", "width": 200 } },
  { "properties": { "title": "大きさ", "width": 80, "alignment": "END" } }
]
```

一覧の中身(モデルの項目)はふつう実行時にコードで作るので、定義ファイルには書かない(§10 Q5)。

### 4.3 レイアウトのノード

| キー | 必須 | 型 | 説明 |
|---|---|---|---|
| `name` | | 識別子 | 書くとメンバになる |
| `class` | ○ | `"BoxLayout"`・`"GridLayout"`・`"PlaceLayout"` | |
| `properties` | | 名前 → 値 | §5(`margins`・`orientation`・`spacing`・`row_spacing`・`column_spacing`) |
| `items` | | 項目の配列 | 下の表。配列の順が `add` の順になる |
| `rows`・`columns` | | 番号 → 行・列の設定 | `GridLayout` だけ。`{ "1": { "stretch": 1, "minimum": 80 } }`(`set_row_stretch`・`set_row_minimum` 等) |

項目の書き方はレイアウトで決まる(`add` の引数と同じ。省略すると `add` の既定値)。

| レイアウト | 項目 | 例 |
|---|---|---|
| `BoxLayout` | 部品: `widget`・`stretch`・`alignment`(並べる向きと直交する向きのそろえ方) | `{ "widget": {…}, "stretch": 1, "alignment": "CENTER" }` |
| | 伸びる空き(`add_stretch`) | `{ "stretch": 1 }` |
| | 固定の空き(`add_spacing`) | `{ "spacing": 12 }` |
| `GridLayout` | `widget`・`row`・`column`・`row_span`・`column_span`・`horizontal`・`vertical` | `{ "widget": {…}, "row": 1, "column": 0, "column_span": 2 }` |
| `PlaceLayout` | `widget`・`placement`(`Placement` のメンバ。一部だけ書ける) | `{ "widget": {…}, "placement": { "relative_x": 0.5, "anchor": "CENTER" } }` |

- GridLayout の同じセルに 2 つの部品を置くことはできない(エラー)。

### 4.4 自由な配置(`children`)

レイアウトを付けず、子の位置と大きさを `x`・`y`・`width`・`height` のプロパティで決める。
親の大きさが変わっても置き直さないので、ふつうはレイアウトを使う(親に合わせて置き直したいなら PlaceLayout)。

### 4.5 メニューバー・ツールバー・ステータスバー

| ノード | キー | 説明 |
|---|---|---|
| メニューバー | `name`?・`menus` | `menus` はメニューのノード(`{ "name"?, "title": "ファイル(&F)", "items": [...] }`)の配列(`add_menu`) |
| メニュー | `items` の各項目 | `{ "action": "Action の名前" }`・`{ "separator": true }`・`{ "menu": メニューのノード }`(サブメニュー) |
| ツールバー | `name`?・`properties`・`items` | 項目は `{ "action": … }`・`{ "separator": true }`・`{ "widget": 部品のノード }`(`add_widget`) |
| ステータスバー | `name`?・`properties`・`widgets` | `widgets` は右側に並べる部品のノードの配列(`add_widget`) |

- メニュー・ツールバーの項目は、`actions`(§4.6)の Action を名前で参照する。同じ Action をメニューとツールバーに出せる([ADR 0027](../adr/0027-actions-shortcuts-and-menu-bar.md))。
- `name` を省略したメニューバー等は、`Window::menu_bar` 等で参照する(メンバを作らない)。

### 4.6 Action のノード(`actions`)

| キー | 必須 | 型 | 説明 |
|---|---|---|---|
| `name` | ○ | 識別子 | |
| `properties` | | 名前 → 値 | `text`・`shortcut`・`tooltip`・`enabled`・`checkable`・`checked`・`icon`(§5.1) |
| `events` | | 名前 → ハンドラ名 | `on_trigger` |

所有者はウィンドウ(`new tokotachi::Action(this)`)。

### 4.7 画面に出ないオブジェクトのノード(`objects`)

| キー | 必須 | 型 | 説明 |
|---|---|---|---|
| `name` | ○ | 識別子 | |
| `class` | ○ | クラス名 | カタログにある、Widget ではない Object の派生(`Timer` 等) |
| `properties` | | 名前 → 値 | §5 |
| `events` | | 名前 → ハンドラ名 | §6 |

所有者はウィンドウ。モデル(ListModel・TreeModel)は §10 Q5。

### 4.8 Variable のノード(`variables`)

値を 1 つ持つオブジェクト(`tokotachi::Variable<T>`。[ADR 0016](../adr/0016-property-binding.md))。プロパティを結びつけて(§5.2)、
複数の部品で値を共有したり、ハンドラ(`on_change`)で値の変化を受け取ったりする。

| キー | 必須 | 型 | 説明 |
|---|---|---|---|
| `name` | ○ | 識別子 | |
| `type` | ○ | `"bool"`・`"int"`・`"float"`・`"double"`・`"string"` | 値の型(`Variable<T>` の T) |
| `value` | | 型に合う値 | 初期値。省略すると `T()`(`""`・`0`・`false`) |
| `events` | | 名前 → ハンドラ名 | `on_set`(設定されるたび)・`on_change`(変わったとき) |

```json
"variables": [
  { "name": "file_name", "type": "string", "value": "メモ.txt", "events": { "on_change": "file_name_change" } }
]
```

所有者はウィンドウ(`new tokotachi::Variable<std::string>(this, "メモ.txt")`)。

### 4.9 部品ファイル

部品の組み合わせ(検索欄とボタンなど)を別の定義ファイルにして、ほかのウィンドウから置く([ADR 0043](../adr/0043-designer-component-files.md))。

- 部品ファイルは、ウィンドウのノードの `class` を `Widget` にした定義ファイル(拡張子は同じ `.ttui.json`)。`name` は生成するクラスの名前。
  メニューバー・ツールバー・ステータスバー・`buttons` は書けない。`width`・`height` はデザイナーで描く大きさで、生成するコードには書かない
  (置いた側のレイアウトが大きさを決める)。
- 置く側は、部品のノードに `component`(定義ファイルのフォルダからの相対パス)を書く。設定できるのは Widget のプロパティ・ハンドラ・`actions` だけ。

```json
{ "name": "search_bar_1", "class": "Widget", "component": "parts/search_bar.ttui.json", "properties": { "tooltip": "検索" } }
```

- 部品ファイルの中の部品・ハンドラは、部品ファイルのクラスのもの(置いた側から `search_bar_1->button_search` で触れる)。

## 5. プロパティ

`properties` のキーは Tokotachi のプロパティの名前、値の書き方はプロパティの型(カタログ)で決まる。
読み取り専用のプロパティ(ReadOnlyProperty)と、カタログでデザイン時に設定できないとしたもの(`parent`・`bounds`・`focused` 等)は書けない。

| 型 | 書き方 | 例 |
|---|---|---|
| `int`・`float`・`double` | 数値 | `75`、`0.5` |
| `bool` | 真偽 | `true` |
| `std::string` | 文字列 | `"開く(&O)..."` |
| 列挙型 | 値の名前の文字列(型の名前は書かない) | `"VERTICAL"` |
| `Margins` | 数値(4 辺とも同じ)か、`left`・`top`・`right`・`bottom` のオブジェクト | `12`、`{ "left": 12, "right": 12 }` |
| `Size`・`Point`・`Rect` | メンバの名前のオブジェクト | `{ "width": 200, "height": 0 }` |
| `Font` | メンバの名前のオブジェクト | `{ "size": 18 }` |
| `ThemeColor` | 役割の名前、または `"#RRGGBB"`・`"#RRGGBBAA"` | `"ACCENT"`、`"#FF8000"`(§10 Q7) |
| `Shortcut` | 表記の文字列(`Shortcut(const char*)` と同じ) | `"Ctrl+S"` |
| `ThemeOverride` | `colors`(役割の名前 → 色か `{ "light", "dark" }`)と寸法(`radius`・`field_radius`・`field_padding`)のオブジェクト(§5.3) | `{ "colors": { "ACCENT": "#DC2626" }, "radius": 0 }` |
| `std::vector<std::string>` | 文字列の配列 | `["小", "中", "大"]` |
| `std::vector<float>` | 数値の配列 | `[160, 360]` |
| 部品・Action・メニューへのポインタ | 同じファイルのノードの `name` | `"action_open"` |
| `Image` | 画像のファイルの、定義ファイルのあるフォルダからの相対パス(§5.1) | `"icons/open.png"` |
| (結びつけ) | `{ "bind": "Variable の名前" }`(§5.2) | `{ "bind": "file_name" }` |

- **位置と大きさは書かない**(レイアウトが決める)。自由な配置(§4.4)の子と、ウィンドウの `width`・`height` だけが例外。
- 最小・最大の大きさ(`minimum_size`・`maximum_size`)は書ける(レイアウトの計算に使われる)。

### 5.1 画像

画像のプロパティ(今は Action の `icon`。ツールバーのボタンに出る)には、画像のファイルの**定義ファイルのあるフォルダからの相対パス**を書く。

- 区切りは `/`。`../` で上のフォルダも指せる。絶対パス・`\`・ドライブ名は使えない。拡張子は `png`・`jpg`・`jpeg`・`bmp`・`gif`(大文字も可)。
- 生成するコードには、**画像のファイルの中身を埋め込む**(codegen-design.md N8)。実行するときに画像のファイルは要らず、
  作業フォルダ・実行ファイルの場所に依らない。画像のファイルを変えたら、コードを生成し直す。
- ファイルが無いと、コードを生成できない(拡張は問題パネルにエラーを出す)。キャンバスは、拡張が読んだ中身で描く(無ければ画像なしで描く)。

### 5.2 Variable との結びつけ

プロパティの値に `{ "bind": "Variable の名前" }` と書くと、値を設定する代わりに、その Variable(§4.8)に結びつける
(生成するコードは `tokotachi::bind(line_edit_name->text, file_name->value);`)。結んだ時点でプロパティは Variable の値になり、
その後はどちらを変えても、もう一方に伝わる。同じ Variable に複数のプロパティを結びつけられる。

- 結びつけられるのは、型が `bool`・`int`・`float`・`double`・`string` のプロパティで、Variable の型と同じもの(`bind` は同じ型どうしだけ)。
- 位置と大きさ(`x`・`y`・`width`・`height`)は結びつけられない(レイアウト・デザイナーのつまみで決めるため)。
- Variable を消すと、結びつけていたプロパティには Variable の初期値を書く(見た目は変わらない)。名前を変えると、結びつけも書き換える。

### 5.3 テーマの上書き(`theme_override`)

部品の `theme_override`([ADR 0042](../adr/0042-widget-theme-override.md))は、その部品と子孫に効くテーマの一部の上書き。

```json
"theme_override": {
  "colors": {
    "ACCENT": { "light": "#059669", "dark": "#34D399" },
    "ON_ACCENT": "#FFFFFF"
  },
  "radius": 0
}
```

- `colors` は役割(`ColorRole` の値)から色へ。色は `"#RRGGBB"`・`"#RRGGBBAA"`(ライト・ダークで同じ)か、`{ "light": ..., "dark": ... }`。
- 寸法は 0 以上の数(dp)。書いていない役割・寸法は親から継ぐ。
- 生成するコードは 1 つの式にする(`tokotachi::ThemeOverride().set(...).set_radius(0)`)。キャンバスも同じ値を設定して描く。
- `"preset": "名前"` で、プロジェクトに登録したテーマ(project-spec.md §4.2)を参照できる。`colors`・寸法を一緒に書くと、テーマの上に重なる
  (`{ "preset": "danger", "radius": 0 }`)。プロジェクトに属するウィンドウだけが使える。
- プロジェクトのテーマの上書き(project-spec.md §4.1)も同じ書き方(`preset` は書けない)。

## 6. ハンドラ

`events` のキーは Tokotachi のハンドラのプロパティの名前(`on_click` 等)、値は生成するクラスのメンバ関数の名前(識別子)。

- メンバ関数の引数は、ハンドラの型(カタログ)で決まる。`NotifyHandler` なら `void button_search_click(tokotachi::Object* sender);`、
  `CloseHandler` なら `void main_window_close(tokotachi::Object* sender, bool& can_close);`。
- 同じメンバ関数を複数のハンドラに書ける。ただし、ハンドラの型が同じでなければならない。
- メンバ関数の名前は、ノードの `name` と衝突してはならない。

## 7. 意味の検証

構造(形・型)は Zod のスキーマで、次の意味の検証は TypeScript の検証関数で行う(Bethany と同じ 2 段階)。

- `name`・メンバ関数の名前の重複と、識別子・予約語・基底クラスのメンバとの衝突
- クラス・プロパティ・ハンドラがカタログにあること、値が型に合うこと
- 参照(Action・部品・メニュー)の解決と型の一致
- 親子の制約(`layout` と `children` は同時に書けない、`tabs`・`panes`・`content`・`columns` は対応するクラスだけ、`buttons` は Dialog だけ、
  部品ファイルの根はバーを持たない、部品ファイルを置いた部品は子を持たない)
- 部品ファイルの参照の書き方(`.ttui.json` で終わる相対パス・区切りは `/`)。ファイルが有るか・根が Widget か・循環していないかは拡張が調べる
- レイアウトの項目の制約(GridLayout のセルの重なり、BoxLayout の項目のキー)
- 同じメンバ関数を書いたハンドラの型が一致すること
- 生成したコードの一時的な変数の名前(codegen-design.md N3)とノードの `name` が重ならないこと
- 名前の書き方(クラス名は PascalCase、メンバは snake_case)。合わなければ警告にとどめる

診断は JSON 上の位置(パス)を持ち、拡張がファイル上の範囲に変えて問題パネルに出す。

## 8. 決まった形での書き出し

Bethany と同じ。

- 実装は [serialize.ts](../../designer/packages/core/src/dsl/serialize.ts)。`*.ttui.json` は Prettier の対象から外している。
- キーの順: ドキュメントは `$schema`・`format_version`・`window`・`actions`・`objects`・`variables`。Variable は `name`・`type`・`value`・`events`。レイアウトは `name`・`class`・`properties`・`rows`・`columns`・`items`。ノードは `name`・`class`・`properties`・`events`・
  そのあとに子を表すキー(`menu_bar`・`tool_bar`・`status_bar`・`layout`・`children`・`tabs`・`panes`・`content`・`columns`・`buttons`・`items` 等)。
  レイアウトの項目は `widget` を最後に置く(設定が先に読める)。`properties`・`events` の中はカタログの順。
- `properties` とノードは 1 行に 1 つのキー。値が単純なもの(文字列・数値・真偽とその配列)だけのオブジェクト・配列は、
  1 行(100 文字)に収まれば 1 行で書く。
- インデントは 2 文字、改行は LF、末尾に改行。

## 9. コード生成の設定

定義ファイルは画面の設計だけを持つ。生成先のフォルダ・拡張子・コメントの言語は、プロジェクトファイルに持つ(Bethany の project-spec.md と同じ考え方)。
詳しくはコード生成の設計(これから書く codegen-design.md)で決める。

## 10. 未決の論点

| # | 論点 | 案 |
|---|---|---|
| Q1 | ファイルの拡張子 | **決定(2026-10-10)**: `*.ttui.json`(Tokotachi の UI。`tk` は cpp_tk(Tcl/Tk のラッパー)と紛らわしいので `tt` にする。JSON であることが分かり、`jsonValidation` で関連付けられる)。プロジェクトファイルも同じく `*.ttproj.json` |
| Q2 | 生成するファイルの名前 | 定義ファイルの名前から(`main_window.ttui.json` → `main_window.hpp`・`main_window.cpp`)。クラス名は `window.name` |
| Q3 | Variable と結びつけ(`bind`) | **決定(2026-10-10)**: 案のとおり、`variables`(名前・型・初期値・ハンドラ。§4.8)をドキュメントに置き、プロパティに `{ "bind": "変数の名前" }` と書く(§5.2)。部品どうしを直接結ぶこと(読み取り専用のプロパティからの一方向の結びつけ)は扱わない(Variable を介せば同じことができる) |
| Q4 | 画像(Action・ボタンの `icon` 等) | **決定(2026-10-10)**: 定義ファイルからの相対パスを書き、生成するコードには中身を埋め込む(§5.1)。実行時に読むと、作業フォルダ・インストール先に依って見つからなくなるため。アイコンは小さいので、ソースが大きくなるのは許す |
| Q5 | 一覧の列・モデル(ListView の `add_column`、ListModel・TreeModel の項目) | **決定(2026-10-10)**: 列は部品の `columns` に書く(§4.2.1)。モデル(ListModel・TreeModel)とその中身はふつうコードで作るので、デザイナーでは扱わない |
| Q6 | 再利用できる部品の組み合わせ(C++Builder のフレーム) | ルートを `Widget` にしたファイルを認め、別のファイルから部品として置く。MVP の後 |
| Q7 | 色の書き方 | ThemeColor は役割(テーマに従う)と具体的な色の両方を持つので、役割の名前と `#RRGGBB(AA)` の両方を書けるようにする |
| Q8 | ハンドラのメンバ関数を生成するか、ラムダで書くか | メンバ関数(C++Builder と同じ。利用者がソースの中身を書く場所が決まる)。ラムダの本体を定義ファイルに書くことはしない |
