# 描画 {#drawing_guide}

[TOC]

![描画](drawing.png)

## PaintBox と Canvas

自由に描くには `PaintBox`(または任意の部品の `on_paint`)の中で、渡される `Canvas` に描きます。

```cpp
tokotachi::PaintBox* chart = new tokotachi::PaintBox(&window);
chart->on_paint = [](tokotachi::Object*, tokotachi::Canvas& c)
{
    c.fill_rect(tokotachi::Rect(0, 0, 100, 50), tokotachi::Color::rgb(0xf3f4f6));
    c.draw_line(tokotachi::Point(0, 0), tokotachi::Point(100, 50), 2, tokotachi::Color::rgb(0x2563eb));
    c.draw_text("こんにちは", tokotachi::Point(8, 8), tokotachi::Color::rgb(0x111827));
};
chart->refresh();      // 描き直す(データが変わったとき)
```

Canvas の決まり:

- 描画の呼び出しの間だけ有効です。参照を持ち続けず、描き直したいときは `invalidate`・`refresh` で頼みます。
- メインスレッドからだけ使います(別のスレッドでは Bitmap に描きます)。
- 座標は dp。描いたものは裏のバッファーに描いてから画面に出すので、ちらつきません。
- `clip_bounds()` の外は見えないので、重い描画は範囲の外を飛ばします。

## 描けるもの

| 種類 | 関数 |
|---|---|
| 塗り | `fill_rect`・`fill_round_rect`・`fill_ellipse`・`fill_path` |
| 線 | `draw_line`・`stroke_rect`・`stroke_ellipse`・`stroke_path`(`StrokeStyle` で太さ・端・角・破線) |
| 文字・画像 | `draw_text`・`draw_image` |
| パス | `Path` の `move_to`・`line_to`・`quad_to`・`cubic_to`・`add_arc`・`add_polyline` など |
| 塗り方 | 色(アルファで半透明)・`Paint::linear`・`Paint::radial` |
| 変換・クリップ | `translate`・`scale_by`・`rotate`・`clip_rect`・`clip_path`(`save`・`restore` で戻す) |
| グループの透過 | `begin_layer(opacity)`〜`end_layer()`(重なった所も同じ濃さ) |

## 重い描画

- `PaintBox::cached` を true にすると、描いた結果を覚え、`refresh()`・大きさ・倍率・テーマが変わるまで `on_paint` を呼びません。
- `Bitmap` は書き換えられる画像です。画面と同じ Canvas で描け、画素も読み書きできます。前もって描いておき、`to_image()` で貼ります。
- Bitmap には**別のスレッドでも描けます**(文字も)。`Task` の中で描いて Image を返し、画面に貼ります(@ref threads_guide)。

```cpp
tokotachi::Task<tokotachi::Image>* render = new tokotachi::Task<tokotachi::Image>(&window);
render->on_finish = [=](tokotachi::Object*) { image = render->result; chart->refresh(); };
render->start([](tokotachi::TaskContext&) -> tokotachi::Image
{
    tokotachi::Bitmap b(800, 600);
    b.canvas().fill_rect(tokotachi::Rect(0, 0, 800, 600), tokotachi::Color::rgb(0xffffff));
    ...
    return b.to_image();
});
```
