# 0009. IME の変換中の文字列はライブラリが入力欄の中に描き、候補窓は OS・IME に任せる

- 状態: 承認
- 日付: 2026-10-04

## 背景

日本語の入力を、Qt と同じ水準で扱いたい。変換中の文字列(プリエディット)の表示の仕方には 2 通りある。

- OS・IME が自分の窓に描く(オーバーザスポット)。アプリケーションは何もしなくてよいが、入力欄の文字と表示がずれる。
- アプリケーションが入力欄の中に描く(インライン)。Qt・ブラウザと同じ。変換中の文字列・変換対象の文節・
  変換中のキャレットの位置をアプリケーションが受け取って描く。

Linux では、SDL3 が IME とつなぐ経路が環境で変わる。

1. コンポジタが text-input-v3 を持つ(GNOME・KDE・wlroots 系): コンポジタ経由で IME とつなぐ。
2. 持たない: D-Bus で Fcitx(`SDL_IM_MODULE=fcitx` または `XMODIFIERS=@im=fcitx` のとき)か IBus に直接つなぐ。

## 決定

- **変換中の文字列はライブラリが描く**。`SDL_HINT_IME_IMPLEMENTED_UI` を `composition` にし、
  `SDL_EVENT_TEXT_EDITING`(文字列、変換対象の文節の開始と長さ)を受けて入力欄の中に下線付きで描く。変換対象の文節は太い下線にする。
- **候補窓は OS・IME に任せる**(`candidates` は指定しない)。ライブラリは `SDL_SetTextInputArea` で入力欄の矩形と
  キャレットの位置(window の座標。[ADR 0008](0008-coordinate-spaces.md))を伝え、候補窓をキャレットの近くに出させる。
- **文字の入力を受け付けるのはフォーカスのある入力欄だけ**。フォーカスを得たら `SDL_StartTextInput`、失ったら `SDL_StopTextInput` を呼ぶ。
- **変換中はキーの操作を IME に任せる**。変換中の文字列があるあいだ、Backspace・矢印キーなどを入力欄の編集に使わない。

## 影響

- 技術検証(`spike/m1`)で、WSLg + Fcitx5(Mozc)の D-Bus の経路で、変換中の文字列の表示と確定を確かめた。
  WSLg のコンポジタは text-input-v1 しか持たず、Fcitx5 は Wayland の addon を無効にしないと終了する
  (`fcitx5 -d --replace --disable=wayland,waylandim`)。
- text-input-v3 の経路(実機の GNOME・KDE)と、Windows の IME(IMM32)の経路は、まだ確かめていない。
- 変換中の文字列のうち、変換対象の文節以外の区切り(文節ごとの下線)は SDL3 のイベントでは分からない。表示は「全体に細い下線、
  変換対象の文節に太い下線」にとどめる。
