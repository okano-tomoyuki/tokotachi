# 0026. 一覧・木はモデルとビューに分け、手軽に使える項目のモデルも用意する

- 状態: 承認
- 日付: 2026-10-04

## 背景

項目の一覧(ListView)と木(TreeView)を作る。決めること:

- データの持ち方。数十万行のデータやデータベースの結果も表示したい一方で、数個の項目を並べるだけのときに、
  多くのコードを書かせたくない。
- 列(カラム)。
- 選び方。
- 1 行に何を描くか。

## 検討した選択肢

データの持ち方:

- 選択肢A: 抽象的なモデル(行の数と、各行の文字などを返す)を土台にする。さらに、項目をオブジェクトで持つモデルを既製品として用意する。
- 選択肢B: 項目のオブジェクトだけにする(Tab と同じ)。簡単だが、大量のデータでは項目ごとにオブジェクトが要る。
- 選択肢C: 抽象的なモデルだけにする。少しの項目でもモデルを書く必要がある。

1 行の中身:

- 選択肢A: 文字・画像・チェックボックスを標準で描く。行ごとにウィジェットは作らない。
- 選択肢B: 行ごとに任意のウィジェットを置く。自由だが、大量の行では重く、見えている行だけを描くこととの相性が悪い。

## 決定

**データは選択肢A(モデルと既製品の項目のモデル)、1 行の中身は選択肢A(文字・画像・チェック)を選ぶ。**
列は複数にでき、見出しを付けられる。選び方は NONE・SINGLE・MULTI から選ぶ。

### モデル(ItemModel)

- `ModelIndex` は項目の位置: 親の中での行の番号と、モデルが決める識別子(`void*`)。木のモデルは識別子で項目を区別する。
- 実装するのは `row_count(parent)` と `text(index, column)` だけでよい。木にするときは `index(row, parent)` と
  `parent_of(index)` も実装する。他に `icon`・`enabled`・`checkable`・`checked`・`set_checked`・`sort`・`locate` を上書きできる。
- 型ごとの仮想関数にする(Qt の `data(index, role)` のような何でも入る値は C++11 では型の安全さを失うため)。
- 変更の知らせは protected の関数で送る: `notify_rows_inserted`(加えた後)・`notify_rows_removing`(除く前)・
  `notify_data_changed`・`notify_layout_changed`(並びが変わった)・`notify_reset`。
  **除くときは前に 1 回だけ知らせる**(Qt の begin/end の対にしない)。ビューは知らせを受けたときに、除く項目とその子孫を
  選択などから外し、表示の行は後で(描く前に)作り直す。
- 1 つのモデルを複数のビューで表示できる。ビューはモデルを所有しない。モデルが破棄されると、ビューの `model` は nullptr になる。

### 既製品のモデル(ListModel・TreeModel)

- 項目は `Item`(TreeModel では `TreeItem`)で、`text`・`column_texts`(2 列目以降)・`icon`・`enabled`・`checkable`・`checked` を
  Property で持つ。値を変えると、表示しているビューが描き直す。結びつけ([ADR 0016](0016-property-binding.md))もできる。
- 項目はモデルが作り、`remove_item` で破棄する(利用者は delete しない)。識別子は項目のアドレス。
- 項目は Object の所有の木に入れない(Object の所有は子の一覧を線形に探して外すので、数万の項目を除くと遅いため)。
- `sort(column, order)` は文字のバイト列の順(UTF-8 なのでコードポイントの順)の安定な並べ替え。言語に合わせた順は対象外。

### ビュー(ItemView・ListView・TreeView)

- 共通の部分は `ItemView` にあり、`ListView` は一番上の行だけを、`TreeView` は開いた子も表示する。
- **見えている行だけを描き、そのときにモデルへ問い合わせる。** 行の高さはすべて同じにする(行の位置を計算で求めるため)。
  文字のレイアウトは文字列ごとに覚えておく(上限 2000 個)。
- 列: `add_column(title, width)` が `Column`(`title`・`width`・`alignment`)を返す。列が無ければ 1 列で、幅いっぱいに描く。
  最後の列は余りまで広げて描く。見出しの境目のドラッグで幅を変えられる。
- 並べ替え: `sorting_enabled` のとき、見出しを押すと `sort_column`・`sort_order` を変え、モデルの `sort` を呼ぶ。
  後から加えた項目は自動では並べ替えない。
- 選択: `selection_mode`・`current`(キーで動かす位置)・`selection`(表示の順)・`select`・`select_all`・`clear_selection`・
  `on_selection_change`・`on_activate`(ダブルクリック・Enter)。SINGLE では `current` を代入すると選ぶ。
  無効な項目は選べず、キーでは飛ばす。
- キー: ↑↓・Page Up/Down・Home・End(Shift で範囲、MULTI で Ctrl を押すと current だけ動く)、Ctrl+Space(MULTI で選ぶかを切り替え)、
  Space(チェックできる項目ならチェック、それ以外なら選ぶ)、Enter、Ctrl+A。TreeView では → で開く・最初の子へ、← で閉じる・親へ。
- 選択・current・開いている項目は、モデルの知らせで直す: 加えたら番号をずらし、除いたら忘れ、並べ替えたら `locate` で引き直す。
  **current の項目を除くと、current は無効になる**(隣へ移さない。必要なら利用者が代入する)。
- TreeView の開閉: `expand`・`collapse`・`is_expanded`・`expand_all`・`collapse_all`、`indentation`。閉じた中に current があれば
  閉じた項目へ移す。`scroll_to` は祖先を開く。

### 画像(Image)

- 行の画像のために `Image`(PNG・JPEG などを plutovg で読む値。コピーは安い)と `Canvas::draw_image(image, rect, opacity)` を加えた。
  画像は 16dp の四角に伸縮して描く。高い倍率のための大きい画像の切り替えは対象外。

## 影響

- 行の位置を float で計算するので、数百万行では位置が 1〜2dp ずれることがある。
- 行ごとに高さを変えること・行ごとのウィジェット・セルの編集・ドラッグでの並べ替えと移動・文字での検索(押した文字で始まる項目へ動く)・
  表(TableView のセル単位の選択)は対象外(必要になったら追加する)。
- 項目の文字の色・フォントを項目ごとに変えることも対象外(モデルの仮想関数を足せば追加できる)。
