# VS Code の拡張とデザイナーの画面の設計

[ADR 0039](../adr/0039-designer.md) の進める順序の 5 番目。VS Code の拡張(カスタムエディタ)と、Webview のデザイナーの画面の設計。
Bethany の拡張・Webview の骨組み(Bethany の docs/designer/editor-design.md)を土台にし、キャンバスは WebAssembly の Tokotachi で描く
([ADR 0040](../adr/0040-designer-canvas-ops.md))。

## 1. 進め方

次の 3 段階で作り、段階ごとに動くものを確かめる(2026-10-10 に決めた)。

| 段階 | 作るもの | 状態 |
|---|---|---|
| 1. 編集の土台 | カスタムエディタ、キャンバスの表示と選択、構造の木、インスペクタ(名前・プロパティ・ハンドラ)、診断、コード生成のコマンドと保存したときの生成、ハンドラへの移動、新しいウィンドウ、JSON Schema | 実装済み |
| 2. 配置の編集 | パレットからの追加、ドラッグで入れる位置を変える(Qt Designer と同じく、BoxLayout は項目の間、GridLayout はセル)、削除、レイアウトの項目の設定(伸びる割合・行と列など)、レイアウトの切り替え | 実装済み |
| 3. 仕上げ | メニュー・ツールバー・Action の編集、ズーム、見本の名前の付け替え(種類を先頭に)、プロジェクトファイルとプロジェクトのビュー(新しいプロジェクトで CMakeLists.txt・main.cpp も作る。[project-spec.md](project-spec.md))、日本語の訳 | 実装済み |

## 2. パッケージ

```
webview   ─▶ core
extension ─▶ core, codegen, canvas
canvas    ─▶ core, codegen(操作の並び)
cli       ─▶ core, codegen
codegen   ─▶ core
```

extension と webview は互いを import せず、core の `protocol.ts` のメッセージでだけ通信する(Bethany と同じ)。
キャンバスの WebAssembly のモジュールは拡張の中(Node.js)で動かし、描いた画素を Webview に送る。

## 3. 編集のしくみ

Bethany と同じ。

- **TextDocument を唯一の正とする**(`CustomTextEditorProvider`)。Undo・Redo・保存・保存していない印は VS Code に任せる。
- 画面の操作は、core の**編集コマンド**([commands.ts](../../designer/packages/core/src/edit/commands.ts)。`applyCommand(doc, command)`)で表す。
  Webview はコマンドを拡張に送り、拡張は TextDocument の今の内容に適用して、決まった形で書き出し、変わった範囲だけを `WorkspaceEdit` で置き換える。
  変わった TextDocument は Webview に送り直す(1 回の操作 = 1 回の Undo)。
- ノードは**JSON 上のパス**で指す(レイアウト・タブ・区画には名前が無いことがあるため)。パスが指すものが無ければコマンドは失敗する。
- JSON として読めない・形が違うときは、画面を編集できない状態にして理由を出す(テキストで直してもらう)。意味の検証のエラーがあっても編集はでき、
  キャンバスは描かない(エラーが無くなったら描く)。

### 3.1 編集コマンド(第 1 段階)

| コマンド | 内容 |
|---|---|
| `setProperties` | プロパティを設定する。値が null なら書いたものを消す(既定値に戻す) |
| `setEvent` | ハンドラのメンバ関数の名前を設定する(null なら消す)。型が違う同じ名前のハンドラがあれば拒む |
| `rename` | 名前を変える。Action を参照するメニュー・ツールバーの項目・部品の actions、参照のプロパティも書き換える。識別子・予約の名前・重複を検査する |
| `batch` | 複数のコマンドを 1 回の変更として適用する(どれかが失敗したら何も変えない) |

どのコマンドの後も、検証のエラーを増やさないことをテストで確かめる。

### 3.2 編集コマンド(第 2 段階。[structure.ts](../../designer/packages/core/src/edit/structure.ts))

部品を入れる場所は**スロット**で表す: レイアウトの項目の位置(GridLayout は行と列、PlaceLayout は置く位置も)・自由な配置(位置)・
タブ・区画の位置・ScrollArea の中身・ツールバー・ステータスバーの位置。

| コマンド | 内容 |
|---|---|
| `addWidget` | 部品をスロットに足す。名前は種類を先頭に付けた空いているもの(`button_1`)。文字を持つ部品(Button・CheckBox・RadioButton・Label)は名前を文字にし、入れ物(Widget)には BoxLayout を付ける |
| `addObject` | 画面に出ないオブジェクト(Timer)を足す(`timer_1`) |
| `moveWidget` | 部品をスロットへ移す。同じ種類のレイアウトへ移すときは、項目の設定(伸びる割合等)を引き継ぐ。自分の中へは移せない。自由な配置でなくなったら位置と大きさを消す |
| `remove` | ノードを消す(複数。後ろのものから)。タブ・区画は中の部品ごと消す |
| `addSpacer` | BoxLayout に空き(伸びる空き・固定の空き)を足す |
| `setItem` | レイアウトの項目の設定を変える |
| `setLayout` | 部品(ウィンドウ)のレイアウトを変える。項目は今の並びのまま移す(BoxLayout → GridLayout は向きに合わせて 1 列か 1 行、GridLayout → BoxLayout は行・列の順)。PlaceLayout・自由な配置には、キャンバスに描いている位置で置く。余白・間隔は引き継げるものだけ引き継ぐ |

- 適用して**新しい検証のエラーが出るなら、コマンドは失敗する**(GridLayout のセルの重なり等)。
- 項目を外して空になった配列(`items`・`tabs` 等)は書き出さない。

### 3.3 編集コマンド(第 3 段階。[menus.ts](../../designer/packages/core/src/edit/menus.ts))

メニュー・ツールバーの項目は Action を名前で参照する(ADR 0027)。構造の木で「+ Item」を押すと、新しい Action(`action_1`)を作って
その項目から参照する(`addAction` と `addEntry` の batch)。項目を選ぶと、インスペクタで参照する Action を選び、その Action の
文字・ショートカット・ハンドラをそのまま編集できる。

| コマンド | 内容 |
|---|---|
| `addBar` | ウィンドウにメニューバー・ツールバー・ステータスバーを付ける |
| `addAction` | Action を足す(名前は送る側が決める。文字は名前) |
| `addMenu` | メニューバーにメニューを、メニューにサブメニューを足す |
| `addEntry` | メニュー・ツールバーに項目(Action の参照か区切り)を足す |
| `setEntryAction` | 項目が参照する Action を変える |
| `setMenuTitle` | メニューの見出しを変える |
| `moveSibling` | 同じ並びの中で前後に動かす(メニュー・項目・Action・レイアウトの項目・タブ・区画など。部品を指せば、それを入れている項目) |

- Action を消すと、それを参照するメニュー・ツールバーの項目と、部品の `actions` からも消す(参照が切れないように)。
- サブメニューを消すと、それを入れているメニューの項目ごと消す。

構造の木の行の右クリックのメニューに、そのノードに合わせた操作を出す([treeOps.ts](../../designer/packages/webview/src/treeOps.ts)):
ウィンドウはバーと Action・Variable を足す、メニューバーはメニューを足す、メニュー・ツールバーは項目・区切り(・サブメニュー)を足す、
項目はその後ろに足す、並びの中のものは上へ・下へ(Alt+↑・↓)、消せるものは削除(Delete)。

### 3.4 Variable の編集([variables.ts](../../designer/packages/core/src/edit/variables.ts))

| コマンド | 内容 |
|---|---|
| `addVariable` | Variable を足す(名前は送る側が決める。同じ batch の中で結びつけるため) |
| `setVariable` | Variable の型・初期値を変える(型を変えたら、合わなくなる初期値は消す) |

- プロパティを結びつける・外すのは `setProperties`(値を `{ "bind": "名前" }` にする・値に戻す)。型を変えて結びつけが合わなくなるときは、
  新しい検証のエラーが出るので失敗する。
- `rename` は結びつけも書き換え、`remove` は結びつけていたプロパティに Variable の初期値を書く。

### 3.5 一覧の列の編集

| コマンド | 内容 |
|---|---|
| `addColumn` | ListView・TreeView に列を足す(見出しの文字を `title` に書く) |

- 列の削除・並べ替えは `remove`・`moveSibling`、プロパティ(title・width・alignment)は `setProperties`。
- 構造の木では列を一覧の部品の子に出し、一覧の部品か列を選んでいるときに「+ 列」を出す。列を選ぶと、キャンバスには一覧の部品の枠を出す。

## 4. キャンバス

- 拡張は、TextDocument が変わるたびに(続けて変わったときは最後の内容だけを)、WebAssembly の Tokotachi で描き直し、
  画素(RGBA)と各部品の位置(変数の名前ごと)を Webview に送る(`canvas` のメッセージ)。1 回 10 ms ほど。
- Webview は画像を `<canvas>` に描き、選んだノードの枠を重ねる。ウィンドウの枠とタイトルバーは HTML で描く(Tokotachi が描くのは中身)。
- 配色は設定 `tokotachiDesigner.canvas.colorScheme`(`auto` は VS Code の配色テーマに合わせる)。
- 倍率は 50%〜200%(ツールバーの選択肢か、Ctrl+ホイール)。Webview の状態に保存し、開き直したときに戻す。
- クリック: 点の下のいちばん小さい部品を選ぶ。ダブルクリック: その部品の主なハンドラ(`on_click`・`on_trigger`・`on_change` 等。
  core の `defaultEventOf`)のコードへ移動する(未設定なら `<名前>_<ハンドラ>` の名前を設定してから。§6)。
- レイアウト・タブ・区画を選んだときは、付いている部品・中の部品の枠を出す。
- 構造の木で、表示していないタブの中の部品を選ぶと、そのタブを表示する(キャンバスの表示だけ。定義ファイルの `current_index` は変えない)。
- 中身が空の入れ物(部品の無い Widget・TabWidget・Splitter・ScrollArea)は、レイアウトの中で大きさが 0 になって見えず、
  部品を入れる場所にもならない。そこで、デザイナーの中でだけ最小の大きさ(80×48)を与えて描く(`minimum_size` を書いていないものだけ)。

### 4.1 部品を入れる(第 2 段階)

- 左のペイン([Sidebar.tsx](../../designer/packages/webview/src/components/Sidebar.tsx))は、上にパレット、下に構造の木を置く。
  間の仕切りで高さの割合を、右の端で幅を変えられ、ツールバーの ◧ で隠せる(キャンバスを広く使う)。配置は Webview の状態に保存する。
  VS Code のサイドバーのビューからは Webview のキャンバスへドラッグして入れられない(Webview は Shift を押している間しか外からのドロップを
  受けない)ので、パレットと構造の木は Webview の中に置く。
- パレットは「部品」と「非ビジュアル」の 2 つのタブに分け、足す場所を 1 か所にまとめる。名前で絞り込め、分類の見出しで畳める
  (開いているタブと畳んだ分類は覚えておく)。
  - 部品: カタログの分類ごとの画面に出る部品と、空き(Stretch・Spacing)を、アイコンを付けて並べる。
  - 非ビジュアル: Action・画面に出ないオブジェクト(Timer など)・Variable。キャンバスには置かないので、行の「+」ボタン
    (か行のダブルクリック)で足す。
- パレットからキャンバスへドラッグすると足し、キャンバスの部品をドラッグすると移す(4px 動かしたらドラッグ。Esc でやめる)。
  パレットの項目をクリックすると、選んでいる部品の後ろ(BoxLayout)か、選んでいる入れ物の終わりに足す。
- 入れる場所は、点の下のいちばん内側の入れ物で決める([dropTarget.ts](../../designer/packages/webview/src/dropTarget.ts)):

  | 入れ物 | 場所 | 印 |
  |---|---|---|
  | BoxLayout | 並べる向きで、点に近い項目の間 | 線 |
  | GridLayout | 点の下のセル(最後の行・列の外なら新しい行・列)。使われているセルには入れられない | 枠(入れられなければ赤) |
  | PlaceLayout・レイアウトの無い入れ物 | その点(入れ物の左上から) | 点 |
  | TabWidget | 新しいタブ | 入れ物の枠 |
  | Splitter | 並ぶ向きで、点に近い区画の間 | 線 |
  | ScrollArea | 中身(空のときだけ) | 入れ物の枠 |

  空きは BoxLayout にだけ入れられる。位置はキャンバスの描画の結果(部品の位置と大きさ)から求め、実際の描画でテストする。
- 削除: ツールバーの「Delete」、Delete キー、構造の木の右クリックのメニュー。
- インスペクタの「Layout」の欄: 入れ物(Widget・ウィンドウ)はレイアウトの種類、レイアウトの中の部品は項目の設定
  (BoxLayout は stretch・alignment、GridLayout は row・column・row_span・column_span・horizontal・vertical、PlaceLayout は placement のメンバ)、
  空きは stretch か spacing。

## 5. 構造の木とインスペクタ

- 構造の木([outline.ts](../../designer/packages/core/src/dsl/outline.ts)): ウィンドウ(メニューバー・ツールバー・ステータスバー・
  レイアウトと部品の入れ子・タブ・区画・空き)、Action、画面に出ないオブジェクト、Variable。キャンバスと選択を共有する。
  キャンバスで選んだノードは、見えるところまでスクロールする。
  - Action・オブジェクト・Variable は、ウィンドウの木の下に分けた欄に、空でも見出しを常に出す(ウィンドウの木が長くても隠れない)。
    ウィンドウの木との間の仕切りで、欄の高さの割合を変えられる(Webview の状態に保存する)。
- アイコン([ClassIcon.tsx](../../designer/packages/webview/src/components/ClassIcon.tsx)): パレットと構造の木の各行に、クラス(クラスを持たない
  ノードは種類)のアイコンを出す。16×16 の線画の SVG(Bethany のものを土台に描いた)。線は文字の色、要所だけテーマの強調色。
  カタログのすべてのクラスと見本の木のすべてのノードにアイコンがあることを、テストで確かめる。
- インスペクタ: 名前(名前を持つノード)と、プロパティ・ハンドラの 2 つのタブ。
  - 並びはカタログの順。書いた値は太字。既定値(カタログで測った値)は薄く出す。説明(ヘッダの `///`)はツールチップに出す。
  - 入力欄は型で決める: 真偽はチェック、列挙型は選択肢(先頭は既定値)、ThemeColor は役割の選択肢と色の選択、文字列の配列は複数行、
    数値の配列・Margins・Size はカンマ区切り、テーマの上書きはプロジェクトに登録したテーマの選択(登録していれば)と、役割ごとのライト・ダークの色(「+ 色を上書きする」で足し、× で継ぐ)と寸法の数値、画像はパスの入力と「…」(ファイルを選んで、定義ファイルからの相対パスを設定する。`pickImage` のメッセージ)、
    それ以外は文字の入力。
  - 文字の入力は Enter かフォーカスを外したときに確定し、Esc で戻す。空にすると既定値に戻す。読めない値は確定せず、欄にエラーを出す。
  - 位置と大きさは、ウィンドウの width・height と、自由な配置の子だけに出す。
  - ハンドラ: 名前の入力と「+」(既定の名前を設定してコードを開く)・「→」(コードを開く)。
  - 結びつけ(dsl-spec.md §5.2): 結びつけられるプロパティの右に、同じ型の Variable・「新しい Variable」(`<部品の名前>_<プロパティ>`。
    今の値を初期値にする)・「結びつけを外す」の選択肢を出す。結びつけたプロパティの欄には、Variable の名前と初期値を出す。
  - Variable: 名前・型(選択肢)・初期値・ハンドラ(`on_set`・`on_change`)。

## 6. コード生成とハンドラへの移動

Bethany と同じ。

- コマンド「コードを生成」(エディタのタイトルのボタン・画面のボタン・エクスプローラーの右クリック)。手で変えた区間があれば上書きしてよいか聞く。
- **保存したときの生成**(設定 `tokotachiDesigner.codegen.generateOnSave`、既定は有効)。ダイアログは出さず、手で変えた区間があれば上書きせずに知らせる。
- 生成の設定(コメントの言語・名前空間・ヘッダとソースのフォルダ)は、ウィンドウが属するプロジェクトの `codegen` から決める([project-spec.md](project-spec.md))。どのプロジェクトにも属さないウィンドウは、拡張の設定(`tokotachiDesigner.codegen.*`)で生成する。
- ハンドラへの移動: コードを生成してから(足りない雛形を足して)、生成したソースのハンドラの本体にカーソルを置いて開く。

## 7. 拡張のその他

- **新しいウィンドウ**: エクスプローラーの右クリック・「新しいファイル」・コマンドパレットから。ファイルの名前(snake_case)を入力し、
  クラスの名前はそこから作る(`main_window` → `MainWindow`)。
- **診断**: TextDocument が変わるたびに検証して問題パネルに出す。JSON 上のパスは core の `locate` でファイル上の範囲にする。
  参照している画像のファイルが無ければ、拡張が調べてエラーを加える(`missing-image`。core はファイルを読まない)。
- **画像**(dsl-spec.md §5.1): キャンバスを描くたびに、参照している画像のファイルを読んで渡す。画像のファイルが変わったら描き直す。
- **JSON Schema**: Zod のスキーマから `schema/ttui.schema.json` を作ってコミットし(`check:schema` で食い違いを見つける)、`jsonValidation` に登録する。
- **梱包**: vsce(`--no-dependencies`、まとめた成果物とキャンバスのモジュールだけ)。拡張の ID は `tokotachi-designer`、約 1.2 MB。
  配る .vsix と Marketplace への公開は CI で行う([ci.md](ci.md))。
- **l10n**: 元の文字は英語で、`l10n.t('...')`(拡張は `vscode.l10n.t`)で訳す。訳は `packages/extension/l10n/bundle.l10n.ja.json`、
  コマンド・設定の名前は `package.nls.ja.json`。Webview には拡張が HTML に埋め込んで渡し、CLI も同じ訳を使う(Bethany と同じ。Bethany と同じ文字は同じ訳)。
  `pnpm l10n:check`(`pnpm check` に含む)で、訳の漏れ・使われていない訳・`{0}` の食い違い・コードに日本語を直接書いていないことを調べる
  (生成するコードのコメントの定型文など、意図したものは `l10n-ignore` を付ける)。

## 8. 開発

- `designer/` を VS Code で開き、F5(Run Extension)で開発用のホストを起動する(見本の写し `designer/.cache/playground` が開く)。
- または `pnpm build && pnpm package` で `.vsix` を作って入れる。キャンバスのモジュールは先に `pnpm canvas:build` で作っておく。
