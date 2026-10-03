# 0008. 座標は dp・window・pixel の 3 つを区別し、ウィジェットは dp で扱う

- 状態: 承認
- 日付: 2026-10-04

## 背景

高 DPI と分数スケーリング(125%、150% など)に最初から対応する。SDL3 の座標は OS によって意味が違う。

- Windows: ウィンドウの座標(マウスのイベント、ウィンドウの大きさ)は物理ピクセルで、`SDL_GetWindowPixelDensity` は 1。
  倍率は `SDL_GetWindowDisplayScale` に出る。
- Wayland: ウィンドウの座標は論理ピクセルで、`SDL_GetWindowPixelDensity` と `SDL_GetWindowDisplayScale` の両方に倍率が出る
  (wp_fractional_scale_v1 を持つコンポジタでは 1.25 などの分数になる)。

どちらの場合も、`SDL_GetWindowDisplayScale` が「UI をどれだけ大きく描くべきか」を表し、
`SDL_GetWindowPixelDensity` が「ウィンドウの座標 1 つが何ピクセルか」を表す。

## 決定

座標を 3 つに分け、型・名前で区別する。

| 名前 | 意味 | 使う場所 |
|---|---|---|
| dp | UI の設計の単位。倍率 100% のときの 1 ピクセル | ウィジェットの位置・大きさ、レイアウト、フォントの大きさ |
| window | SDL3 のウィンドウの座標 | マウスのイベント、`SDL_SetTextInputArea`、ウィンドウの大きさの指定 |
| pixel | 描画のバッファのピクセル | Canvas の実装、グリフのラスタライズ |

変換は次の 2 つの値だけで行う。

```
pixel  = dp * displayScale       (displayScale = SDL_GetWindowDisplayScale)
window = pixel / pixelDensity    (pixelDensity = SDL_GetWindowPixelDensity)
```

- ウィンドウは `SDL_WINDOW_HIGH_PIXEL_DENSITY` で作る。
- 倍率は `SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED`・`SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED` で取り直し、フォントの大きさ(pixel)と
  描画のバッファを作り直す(倍率の違うモニタへの移動、実行中の設定の変更に追従する)。
- 細い線や矩形の端は pixel の整数に揃えて描く(線の中心を半ピクセルずらす)。分数の倍率でもにじまないようにするため。
- ウィンドウの大きさを dp で指定する API は、内部で `dp * displayScale / pixelDensity` に直して SDL3 に渡す
  (Windows では倍率の分だけ大きくなり、Wayland では倍率に関係なく同じ値になる)。

## 影響

- ウィジェットを書く人は dp だけを意識すればよい。pixel を扱うのは Canvas とテキストの層だけになる。
- 分数の倍率では、dp の整数の位置が pixel の整数にならない。ウィジェットの境界のにじみを避けるには、
  レイアウトの結果を pixel に揃える処理(スナップ)が要る。どの層で行うかはレイアウトの設計で決める。
- 分数スケーリングの動作は、実機の GNOME・KDE と Windows で確かめる(WSLg は wp_fractional_scale_v1 を持たない)。
