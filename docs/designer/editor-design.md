# VS Code の拡張とデザイナーの画面の設計

[ADR 0039](../adr/0039-designer.md) の進める順序の 5 番目。VS Code の拡張(カスタムエディタ)と、Webview のデザイナーの画面の設計。
Bethany の拡張・Webview の骨組み(Bethany の docs/designer/editor-design.md)を土台にし、キャンバスは WebAssembly の Tokotachi で描く
([ADR 0040](../adr/0040-designer-canvas-ops.md))。

## 1. 進め方

次の 3 段階で作り、段階ごとに動くものを確かめる(2026-10-10 に決めた)。

| 段階 | 作るもの | 状態 |
|---|---|---|
| 1. 編集の土台 | カスタムエディタ、キャンバスの表示と選択、構造の木、インスペクタ(名前・プロパティ・ハンドラ)、診断、コード生成のコマンドと保存したときの生成、ハンドラへの移動、新しいウィンドウ、JSON Schema | 実装済み |
| 2. 配置の編集 | パレットからの追加、ドラッグで入れる位置を変える(Qt Designer と同じく、BoxLayout は項目の間、GridLayout はセル)、削除、レイアウトの項目の設定(伸びる割合・行と列など)、レイアウトの切り替え | これから |
| 3. 仕上げ | メニュー・ツールバー・Action の編集、プロジェクトファイルとプロジェクトのビュー(新しいプロジェクトで CMakeLists.txt・main.cpp も作る)、日本語の訳、ズーム | これから |

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

## 4. キャンバス

- 拡張は、TextDocument が変わるたびに(続けて変わったときは最後の内容だけを)、WebAssembly の Tokotachi で描き直し、
  画素(RGBA)と各部品の位置(変数の名前ごと)を Webview に送る(`canvas` のメッセージ)。1 回 10 ms ほど。
- Webview は画像を `<canvas>` に描き、選んだノードの枠を重ねる。ウィンドウの枠とタイトルバーは HTML で描く(Tokotachi が描くのは中身)。
- 配色は設定 `tokotachiDesigner.canvas.colorScheme`(`auto` は VS Code の配色テーマに合わせる)。
- クリック: 点の下のいちばん小さい部品を選ぶ。ダブルクリック: その部品の主なハンドラ(`on_click`・`on_trigger`・`on_change` 等。
  core の `defaultEventOf`)のコードへ移動する(未設定なら `<名前>_<ハンドラ>` の名前を設定してから。§6)。
- レイアウト・タブ・区画を選んだときは、付いている部品・中の部品の枠を出す。

## 5. 構造の木とインスペクタ

- 構造の木([outline.ts](../../designer/packages/core/src/dsl/outline.ts)): ウィンドウ(メニューバー・ツールバー・ステータスバー・
  レイアウトと部品の入れ子・タブ・区画・空き)、Action、画面に出ないオブジェクト。キャンバスと選択を共有する。
- インスペクタ: 名前(名前を持つノード)と、プロパティ・ハンドラの 2 つのタブ。
  - 並びはカタログの順。書いた値は太字。既定値(カタログで測った値)は薄く出す。説明(ヘッダの `///`)はツールチップに出す。
  - 入力欄は型で決める: 真偽はチェック、列挙型は選択肢(先頭は既定値)、ThemeColor は役割の選択肢と色の選択、文字列の配列は複数行、
    数値の配列・Margins・Size はカンマ区切り、それ以外は文字の入力。
  - 文字の入力は Enter かフォーカスを外したときに確定し、Esc で戻す。空にすると既定値に戻す。読めない値は確定せず、欄にエラーを出す。
  - 位置と大きさは、ウィンドウの width・height と、自由な配置の子だけに出す。
  - ハンドラ: 名前の入力と「+」(既定の名前を設定してコードを開く)・「→」(コードを開く)。

## 6. コード生成とハンドラへの移動

Bethany と同じ。

- コマンド「コードを生成」(エディタのタイトルのボタン・画面のボタン・エクスプローラーの右クリック)。手で変えた区間があれば上書きしてよいか聞く。
- **保存したときの生成**(設定 `tokotachiDesigner.codegen.generateOnSave`、既定は有効)。ダイアログは出さず、手で変えた区間があれば上書きせずに知らせる。
- 生成の設定(コメントの言語・名前空間・ヘッダとソースのフォルダ)は、プロジェクトファイルを作るまでは拡張の設定(`tokotachiDesigner.codegen.*`)に持つ。
- ハンドラへの移動: コードを生成してから(足りない雛形を足して)、生成したソースのハンドラの本体にカーソルを置いて開く。

## 7. 拡張のその他

- **新しいウィンドウ**: エクスプローラーの右クリック・「新しいファイル」・コマンドパレットから。ファイルの名前(snake_case)を入力し、
  クラスの名前はそこから作る(`main_window` → `MainWindow`)。
- **診断**: TextDocument が変わるたびに検証して問題パネルに出す。JSON 上のパスは core の `locate` でファイル上の範囲にする。
- **JSON Schema**: Zod のスキーマから `schema/ttui.schema.json` を作ってコミットし(`check:schema` で食い違いを見つける)、`jsonValidation` に登録する。
- **梱包**: vsce(`--no-dependencies`、まとめた成果物とキャンバスのモジュールだけ)。拡張の ID は `tokotachi-designer`、約 1.2 MB。
- 表示の文字は今は英語(コマンド・設定の名前だけ日本語の訳がある)。日本語の訳は第 3 段階で用意する。

## 8. 開発

- `designer/` を VS Code で開き、F5(Run Extension)で開発用のホストを起動する(見本の写し `designer/.cache/playground` が開く)。
- または `pnpm build && pnpm package` で `.vsix` を作って入れる。キャンバスのモジュールは先に `pnpm canvas:build` で作っておく。
