# 0001. OS との境界に SDL3 を使い、ウィジェットはすべて自前で描く

- 状態: 承認
- 日付: 2026-10-04

## 背景

これまで、既存の GUI ツールキットに薄いラッパーを被せる形で 2 つの GUI ライブラリを作ってきた。

- [bethany](https://github.com/okano-tomoyuki/bethany): Lazarus の LCL を C++/Python から使う。
- [cpp_tk](https://github.com/okano-tomoyuki/cpp_tk): Tcl/Tk を C++ から使う。

薄いラッパーは作るのが速い一方で、できることが土台のツールキットに縛られる(見た目、テキストの品質、高 DPI、
Wayland への対応など)。このライブラリでは、もう一段低い層から作り、Qt に匹敵する GUI ライブラリを目指す。

対象は Windows と Linux(x86_64、aarch64)で、Linux は Wayland を主に置く([ADR 0002](0002-target-platforms-and-wayland-first.md))。

## 検討した選択肢

- 選択肢A: OS のネイティブ API を直接使う(Win32、Wayland のクライアントと各プロトコル、X11)。
  依存は最小になるが、ウィンドウ・入力・IME・クリップボード・D&D・高 DPI を OS ごとに書くことになり、コードの量が極端に増える。
  Wayland だけでも xdg-shell、xdg-decoration、libdecor、text-input-v3、fractional-scale、xdg-activation などを個別に扱う必要がある。
- 選択肢B: GLFW を使う。軽量だが、IME(変換中の文字列、候補窓の位置)・ポップアップのウィンドウ・ファイルダイアログが無く、
  GUI ライブラリの土台としては足りない。
- 選択肢C: SDL3 を使う。ウィンドウ、入力、IME(`SDL_StartTextInput`、`SDL_SetTextInputArea`、変換中の文字列のイベント)、
  MIME 付きのクリップボード、D&D、高 DPI(分数スケーリングを含む)、ポップアップのウィンドウ(`SDL_CreatePopupWindow`。
  Wayland では xdg_popup)、モーダル、ファイルダイアログ(Linux は xdg-desktop-portal)、トレイを OS ごとに実装済み。
  ライセンスは zlib。

## 決定

**選択肢C を選ぶ。** SDL3 は OS との境界(ウィンドウ、イベント、IME、クリップボード、ダイアログ)だけに使い、
ウィジェットはすべて自前で描く(SDL3 の外に描画・テキスト・ウィジェットの層を作る)。

SDL3 が持たないものは、別のライブラリか自前で補う。

| 領域 | 補う手段 |
|---|---|
| 2D のベクター描画(アンチエイリアス、角丸、グラデーション) | plutovg([ADR 0006](0006-plutovg-cpu-rendering.md)) |
| テキストのシェーピング・ラスタライズ | HarfBuzz + FreeType([ADR 0007](0007-text-stack.md)) |
| 改行・双方向テキスト | libunibreak・SheenBidi(予定、[ADR 0007](0007-text-stack.md)) |
| システムのフォントの探索 | Linux は fontconfig、Windows は Fonts フォルダ(予定、[ADR 0007](0007-text-stack.md)) |
| ウィジェット、レイアウト、テーマ | 自前 |
| アクセシビリティ | 将来 AccessKit を検討する |

## 影響

- SDL3 のアプリケーションに描画を任せる形のため、ウィジェットの見た目は OS の標準と一致しない(テーマで寄せる)。
- SDL3 は 2025 年に出たばかりで、ディストリビューションのパッケージが古い・無いことがある。依存ライブラリはソースからビルドする
  ([ADR 0005](0005-build-dependencies-from-source.md))。
- 技術検証(`spike/m1`)で、Windows と Linux(WSLg の Wayland)でのウィンドウ表示、描画、日本語の表示、
  IME の変換中の文字列の表示と確定を確認した。
