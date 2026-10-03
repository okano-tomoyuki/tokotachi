# 0024. 値の部品(Slider・SpinBox・ProgressBar)は double で値を持ち、操作できる部品は RangeBase を基底にする

- 状態: 承認
- 日付: 2026-10-04

## 背景

数値を選ぶ・見せる部品として、スライダー・数値の入力欄・進み具合の棒を作る。決めること:

- 値の型(int か double か、両方を用意するか)。結びつけ([ADR 0016](0016-property-binding.md))は型が一致する変数とだけなので、
  IntVariable と DoubleVariable のどちらに結べるかが決まる。
- 部品の間で共通にする部分。
- 利用者の操作で、値をどう丸めるか。ホイールをどう扱うか。

## 検討した選択肢

値の型:

- 選択肢A: すべて double。小数の範囲もそのまま扱え、SpinBox は小数の桁数(`decimals`)で表示を決める。
- 選択肢B: すべて int(Qt の QSlider・QProgressBar と同じ)。小数は利用者が換算する。
- 選択肢C: int と double の両方の部品を用意する(Qt の SpinBox・DoubleSpinBox)。どちらとも結べるが、部品の数が増える。

## 決定

**値の型は選択肢A(すべて double)を選ぶ。**

### RangeBase(Slider・SpinBox の基底)

- `minimum`・`maximum`(既定 0〜100)・`value`(既定 0)・`single_step`(既定 1)・`page_step`(既定 10)・`on_change`。
- `value` は範囲に丸める。`minimum` を `maximum` より大きくすると `maximum` も同じ値にする(逆も同様)。数でない値(NaN)は無視する。
- **代入した値は single_step に合わせず、利用者の操作(ドラッグ・キー・ホイール・ボタン)による値だけを `minimum + n × single_step` に合わせる。**
  プログラムが決めた値を勝手に変えないため。
- 派生クラスのために、`step_by(amount)`(動かして合わせる)・`snapped(v)`・`ratio()`・`range_changed()`(protected の仮想関数)を持つ。

### Slider

- `orientation`(既定は横)。縦のときは下が `minimum`。
- 溝を押すとその位置へ動き、そのままドラッグできる。つまみを押したときは、押した位置とつまみの中心のずれを保つ。
- ←↓・→↑(single_step)、Page Up/Down(page_step)、Home・End。目盛りは無し(必要になったら追加する)。

### SpinBox

- 中に入力欄(LineEdit の内部の派生)を持ち、枠と上下のボタンは SpinBox が描く。フォーカスは中の入力欄が受ける。
- `decimals`(既定 0。0〜10)で表示を丸め、打ち込んだ数値もこの桁に丸める。読み取り専用の `text` は入力欄の文字列。
- 打ち込んだ数値は、Enter・フォーカスが外れたとき(`commit()`)に `value` にする。読めなければ元の表示に戻す。
  打ち込みの途中では `value` を変えない(途中の「1」などで on_change が呼ばれないように)。
- ↑↓・Page Up/Down で増減する。ボタンは押し続けると、400 ミリ秒後から 60 ミリ秒ごとに繰り返す。

### ProgressBar

- 利用者が操作しないので RangeBase の派生にしない。`minimum`・`maximum`・`value`・`text_visible`(割合を棒の右に出す)。

### ホイール

- **Slider・SpinBox は、フォーカスがあるときだけホイールで値を変える。** フォームをスクロールしている途中で、ポインタの下の部品の値を
  うっかり変えないため。フォーカスが無いときは親(ScrollArea など)へ送る。

## 影響

- IntVariable とは直接結べない。整数で扱いたいときは、`decimals = 0`・`single_step = 1` にし、DoubleVariable と結ぶ。
- 進み具合の分からない処理のための、繰り返し動く表示(indeterminate)は対象外。
- SpinBox の接頭辞・接尾辞(単位)、Slider の目盛りは対象外(必要になったら追加する)。
