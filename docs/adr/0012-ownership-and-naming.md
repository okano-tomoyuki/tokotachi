# 0012. ウィジェットは親が子を所有し、公開の API はクラスを PascalCase・関数を snake_case にする

- 状態: 一部置換(→ [0013](0013-api-style-properties-and-handlers.md)。所有の決定は有効)
- 日付: 2026-10-04

## 背景

ライブラリの土台(Widget・Window・Application)を作るにあたり、ウィジェットの寿命の管理と API の命名を決める。
ライブラリの名前は未定で、名前空間・インクルードのパスは仮に `gui` とする(後に [ADR 0036](0036-library-name.md) で Tokotachi に決め、
`#include <tokotachi/tokotachi.hpp>`・`tokotachi::Button` になった)。

## 検討した選択肢

所有:

- 選択肢A: 親が子を所有する(Qt と同じ)。`new Button(parent)` で作り、親の破棄で子も破棄される。
- 選択肢B: `std::unique_ptr` で親に渡す(`parent->add(make_unique<Button>())`)。所有が型に現れるが、記述が長くなる。
- 選択肢C: bethany(C++Builder)に揃え、Owner と Parent を分ける。自前のライブラリで踏襲する必然性は薄い。

命名: Qt 風の camelCase(`setText`)、cpp_tk 風の snake_case(`set_text`)、C++Builder 風(`Text`・`SetText`)。

## 決定

- **所有は選択肢A**。ウィジェットのツリーと所有が一致し、デザイナーのコード生成とも相性が良い。
  - 子を先に `delete` してもよい(親の一覧から外れる)。親を持つウィジェットをスタックに置いてはいけない(二重に破棄される)。
  - `Window` は親を持たず、スタックに置いてよい。`Window` のデストラクタは、自分がまだ `Window` であるうちに子を破棄する
    (子のデストラクタがウィンドウのフォーカス・ホバーなどの参照を外すため)。
  - ウィジェットが破棄・非表示・無効にされたら、ウィンドウはフォーカス・ホバー・マウスの捕捉の参照を外す。
- **命名は cpp_tk と同じ**: クラス・列挙子は PascalCase、関数・変数は snake_case(`set_text`、`text_changed`)、
  非公開のメンバは末尾に `_`。
- **シグナル**は `Signal<Args...>` の公開のメンバとし、`connect` に関数を渡す(`button->clicked.connect(...)`)。
  呼び出しの途中で接続の変更やウィジェットの破棄があってもよいよう、接続の一覧を写してから呼ぶ。
  ウィジェットは、シグナルを状態の更新の最後に発する(シグナルの先で自分が破棄されてもよいように)。
- **イベント**は仮想関数(`mouse_press_event` など)で受け、受け取らなかったイベント(`ignore()`)は親へ送る(マウスのボタン・キー)。
- 公開のヘッダには SDL3・plutovg・FreeType・HarfBuzz の型を出さない(内部は `detail` 名前空間と `src/` に置く)。

## 影響

- 正式な名前が決まったら、名前空間・インクルードのパス・CMake のターゲットを一括で置き換える。
- 親の付け替え(`set_parent`)は未実装。必要になったら、フォーカス・ホバーの参照の扱いと合わせて加える。
