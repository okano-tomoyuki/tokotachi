# 0002. 対象は Windows と Linux(x86_64・aarch64)とし、Linux は Wayland を主、X11 を従にする

- 状態: 承認
- 日付: 2026-10-04

## 背景

対象のプラットフォームを決める。Linux のデスクトップは Wayland への移行が進んでいる(GNOME・KDE は Wayland が既定。
Raspberry Pi OS も Wayland に移った)。一方で X11 の環境もまだ残る。

Wayland は X11 と考え方が違い、X11 や Windows の感覚で設計すると後で破綻する。

- トップレベルのウィンドウの絶対座標を取れない・指定できない(中央寄せや位置の復元ができないことがある)。
- メニュー・ドロップダウン・ツールチップは、親のウィンドウに相対的に置く xdg_popup でしか作れない([ADR 0011](0011-popups-as-child-windows.md))。
- ポップアップの grab は入力のイベントを起点にしないと得られない。
- ウィンドウの装飾をクライアントが描く環境がある(GNOME。SDL3 は libdecor を使う)。
- 分数スケーリング(125%、150% など)が普通に使われる([ADR 0008](0008-coordinate-spaces.md))。

## 決定

- **対象**: Windows x64、Linux x86_64、Linux aarch64。macOS は当面対象外とする(SDL3 自体は対応しているので、後から加える余地は残す)。
- **Linux は Wayland を主にする**。ライブラリは SDL3 のビデオドライバの既定を `wayland,x11` にし(`SDL_HINT_VIDEO_DRIVER`)、
  Wayland が使えなければ X11 に落ちる。環境変数 `SDL_VIDEO_DRIVER` の指定があればそちらを優先する。
- **API は Wayland の制約に合わせて設計する**。X11 や Windows でしかできないこと(トップレベルのウィンドウの絶対位置の指定など)は、
  「できないことがある」前提の API にする(要求として出し、結果を保証しない)。

## 影響

- 動作確認は Wayland を先に行う。Windows と X11 では、Wayland の制約に合わせた API がそのまま動くことを確かめる。
- WSLg(WSL2)の Wayland は text-input-v3・wp_fractional_scale_v1 を持たず、IME と分数スケーリングの確認には使えない。
  これらは実機の GNOME・KDE で確認する([ADR 0009](0009-ime-preedit-drawn-by-app.md))。
- aarch64 は、x86_64 と同じソースをそのまま(あるいはクロスコンパイルで)ビルドする。実機(Raspberry Pi 等)での確認は今後行う。
