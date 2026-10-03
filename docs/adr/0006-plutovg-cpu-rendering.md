# 0006. 2D の描画は Canvas の抽象の裏で plutovg(CPU)を使い、GPU の描画は後から別の実装として加える

- 状態: 承認
- 日付: 2026-10-04

## 背景

SDL3 の `SDL_Renderer` はアンチエイリアスの無い三角形と矩形しか描けず、角丸・曲線・グラデーションを含むウィジェットの描画には足りない。
2D のベクター描画のライブラリを選ぶ。

aarch64(Raspberry Pi 等)では GPU のドライバの事情がまちまちで、GPU に頼り切ると動かない環境が出る。
GUI の描画は、変化した領域だけを描き直せば CPU でも十分に速い([ADR 0010](0010-retained-mode-and-event-loop.md))。

## 検討した選択肢

- 選択肢A: ThorVG。SW・GL・WebGPU の描画の実装、SVG・Lottie の読み込みを持つ(MIT)。
  ただしビルドは Meson のみで、[ADR 0005](0005-build-dependencies-from-source.md) の方針(CMake でソースからビルドする)と合わない。
  Meson を ExternalProject から呼ぶと、Windows・Linux の両方に Python と Meson が要り、FetchContent で配るときに利用者の負担になる。
  自前の CMake でビルドすると、ThorVG を更新するたびにソースの一覧を追う必要がある。
- 選択肢B: plutovg。C99 で小さく(ソースは十数ファイル)、CMake でビルドでき、MIT。cairo に近い即時描画の API。
  SVG は同じ作者の lunasvg(CMake)で扱える。GPU の描画の実装は無い。
- 選択肢C: Skia。品質と性能は最上位だが、ビルドが非常に重く(GN、巨大なソース)、方針と合わない。
- 選択肢D: Blend2D。高速だが、ビルドに C++17 が要り、JIT の対応するアーキテクチャに制約がある。

## 決定

**選択肢B を選ぶ。** ただし plutovg を直接ウィジェットから使わず、`Canvas` の抽象(パス、塗り、線、クリップ、画像、テキスト)の
裏に置く。GPU の描画(SDL_GPU、OpenGL ES など)は、必要になったら `Canvas` の別の実装として加える。

描画の流れは次のとおり。

1. plutovg と自前のテキストの描画が、アルファ乗算済みの ARGB32 のバッファ(`0xAARRGGBB` の `uint32_t`)に描く。
2. バッファを `SDL_PIXELFORMAT_ARGB8888` のストリーミングのテクスチャに転送し、`SDL_Renderer` で表示する。

plutovg の画素の形式は SDL の `SDL_PIXELFORMAT_ARGB8888` と同じ並びなので、変換は要らない。

## 影響

- 技術検証(`spike/m1`)で、ThorVG から plutovg に置き換えても同じ描画結果(角丸、線形・放射グラデーション、ベジェ曲線)になることを確かめた。
- CPU の描画の速さは、大きいウィンドウ(4K で最大化など)と aarch64 で今後確かめる。変化した領域だけを描き直すことが前提になる。
- SVG のアイコンは、必要になったら lunasvg を依存ライブラリに加えて扱う。
