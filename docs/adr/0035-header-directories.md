# 0035. 公開のヘッダと実装を役割ごとの 7 つのディレクトリに分ける

- 状態: 承認
- 日付: 2026-10-04

## 背景

公開のヘッダが `include/gui/` に 58 個、平らに並び、探しにくくなった。実装(`src/`)は core・gfx・text・layout・models・widgets に
分かれていたが、公開のヘッダの分け方とは対応しておらず、ウィンドウ・ダイアログが core と widgets に散らばっていた。

## 検討した選択肢

- 分け方: 役割ごとに 7 つ(選択肢A)/core・graphics・ui の 3 つ(選択肢B。ui が 40 個近くになる)/公開のヘッダは平らのまま、
  src/ だけ整理する(選択肢C)。
- 名前空間: `gui::` のまま(選択肢A)/ディレクトリに合わせて分ける(選択肢B。`gui::widgets::Button`)。
- まとめのヘッダ: 全体とグループごと(選択肢A)/全体だけ(選択肢B)。
- 互換: 古い場所のヘッダを残さない(選択肢A。まだ公開前)/転送するヘッダを残す(選択肢B)。

## 決定

**すべて選択肢A を選ぶ。**

| ディレクトリ | 内容 |
|---|---|
| `core/` | Object・プロパティ・結びつけ・変数・イベント・Application・タイマー・スレッド(post・Task)・クリップボード・基本の値(geometry) |
| `graphics/` | Canvas・Image・Bitmap・文字のレイアウト・テーマ |
| `layout/` | Layout・BoxLayout・GridLayout・PlaceLayout・そろえ方 |
| `widgets/` | Widget と部品 |
| `windows/` | Window・Popup・Dialog・MessageDialog・FileDialog・MenuBar・ToolBar・StatusBar |
| `actions/` | Action・Shortcut・Menu |
| `views/` | Item とモデル・ItemView・ListView・TreeView |

- `#include <gui/widgets/button.hpp>` のように読み込む。`<gui/gui.hpp>` はすべて、`<gui/core.hpp>`・`<gui/graphics.hpp>`・
  `<gui/layouts.hpp>`・`<gui/widgets.hpp>`・`<gui/windows.hpp>`・`<gui/actions.hpp>`・`<gui/views.hpp>` はグループごとに読み込む。
  layout だけは、Layout クラスのヘッダ(`<gui/layout/layout.hpp>`)と紛れないよう、まとめを `layouts.hpp` とした。
- geometry(Point・Rect・Color)は、イベントなど core のヘッダが使う土台なので core に置く。
- 公開のヘッダどうしは、同じディレクトリなら `"button_base.hpp"`、別のディレクトリなら `"../core/property.hpp"` のように相対で読む。
- `src/` も同じ 7 つに分け、内部のヘッダ(`*_impl.hpp`・`*_access.hpp` など)は、それを主に使う実装と同じディレクトリに置く。
  フォントと文字のレイアウトの実装(以前の `src/text/`)は graphics に、モデルの実装(以前の `src/models/`)は views に入れた。
- 名前空間は `gui::` のまま。ディレクトリはファイルの整理だけで、書き方(`gui::Button`)は変えない。
- **公開のヘッダが、それぞれ単独でコンパイルできることをビルドで確かめる**(CMake がヘッダごとに 1 行の .cpp を作ってコンパイルする)。
  ディレクトリに分けると、他のヘッダに頼った読み込みの漏れが表に出やすいため。

## 影響

- 依存の向きは厳密な層にはしていない(core の Application はテーマ(graphics)を使う)。
- 移動は `git mv` で行い、ファイルの履歴は保つ。
