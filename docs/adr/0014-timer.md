# 0014. タイマーは Object の派生クラスとし、enabled で動かし、利用者が寿命を管理する

- 状態: 承認(寿命の管理は一部置換→ [0015](0015-object-ownership.md))
- 日付: 2026-10-04

## 背景

これまでタイマーは `Application::start_timer(interval, fn, repeat)` と `stop_timer(id)` の関数の組だけで、
LineEdit のキャレットの点滅に使っていた。[ADR 0013](0013-api-style-properties-and-handlers.md) の規則(状態はプロパティ、
イベントは sender を受け取る単一のハンドラ)に合わせ、利用者が使うタイマーを用意する。

SDL3 のタイマー(`SDL_AddTimer`)は別のスレッドで呼ばれるので、そのままではウィジェットに触れない。

## 決定

- **`Timer` は `Object` の派生クラス**とし、プロパティ `interval`(ミリ秒、既定 1000)・`repeat`(既定 true)・`enabled`(既定 false)と、
  ハンドラ `on_timer`(`NotifyHandler`、sender はタイマー自身)を持つ。
  - **動かす・止めるのは `enabled` への代入だけ**とし、`start()`・`stop()` のメソッドは作らない(同じことの書き方を 1 つにする)。
  - 動いている間に `interval`・`repeat` を変えると、その時点から数え直す。
  - `repeat` が false のタイマーは、1 回呼んだ後に `enabled` が false になる。
  - 派生クラスのための入口は protected の仮想関数 `timeout()` で、基底の実装が `on_timer` を呼ぶ。
- **ハンドラはメインスレッドで呼ぶ**。SDL3 のタイマーのスレッドではイベントを積むだけにし、イベントループがハンドラを呼ぶ。
  タイマーを止めた後に、すでに積まれていたイベントが届いても呼ばない。
- **寿命は利用者が管理する**。タイマーはウィジェットのツリーに入らない(親を持たない)。スタック・メンバ変数などに置き、
  破棄すると止まる。ハンドラの中でタイマー自身を破棄してもよい。
- **`Application::start_timer`・`stop_timer` は公開をやめ、内部(`ApplicationImpl`)に移す**。利用者に見せるのは `Timer` だけにする。
  ライブラリの中でも `Timer` を使う(LineEdit のキャレットの点滅)。

## 影響

- `Object` に所有の仕組み(所有者が破棄されると一緒に破棄される)が無いため、`new Timer` で作ったタイマーは利用者が `delete` する。
  ウィジェットと同じように所有者に任せたい場合が多ければ、`Object` の所有の仕組み(ウィジェットの親子とは別の、所有者の一覧)を
  別に検討する。
- `Application` を作る前、または破棄した後に `enabled` を true にしても動かない(`enabled` は false のまま)。

## 後の変更

- ハンドラ `on_timer` は `on_timeout` に改めた([ADR 0059](0059-api-review-before-1-0.md))。
