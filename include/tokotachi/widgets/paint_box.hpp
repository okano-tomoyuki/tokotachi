/// @file
/// tokotachi::PaintBox の宣言。
#ifndef TOKOTACHI_WIDGETS_PAINT_BOX_HPP
#define TOKOTACHI_WIDGETS_PAINT_BOX_HPP

#include "../core/export.hpp"
#include "../graphics/bitmap.hpp"
#include "../graphics/image.hpp"
#include "widget.hpp"

namespace tokotachi
{

/// 自由に描くための部品(VCL の TPaintBox に当たる。docs/adr/0033)。on_paint の中で Canvas に描く。
///
/// @image html widget_paint_box.png
///
/// @code
/// tokotachi::PaintBox* paint_box_chart = new tokotachi::PaintBox(window_main);
/// paint_box_chart->minimum_size = tokotachi::Size(300, 200);
/// paint_box_chart->cached = true;                     // 重い描画: 描いた結果を覚えておく
/// paint_box_chart->on_paint = [=](tokotachi::Object*, tokotachi::Canvas& c) { ... 重い描画 ... };
/// paint_box_chart->refresh();                         // データが変わったら描き直す
/// @endcode
///
/// cached が true なら、描いた結果を Bitmap に覚えておき、refresh を呼ぶか、大きさ・倍率・テーマが変わるまで on_paint を
/// 呼ばない(画面の描き直しでは覚えた絵を貼るだけ)。on_paint の Canvas は、そのときは画面ではなく Bitmap に描く。
///
/// @ingroup widgets_drawing
class TOKOTACHI_API PaintBox : public Widget
{
public:
    explicit PaintBox(Widget* parent = nullptr);
    ~PaintBox() override;

    /// 描いた結果を覚えておくか(既定 false: 描き直すたびに on_paint を呼ぶ)。
    Property<bool> cached;

    /// 描き直す(cached なら覚えている絵を捨てる)。
    void refresh();

protected:
    void paint(Canvas& canvas) override;
    void resized() override;
    void scale_changed() override;
    void theme_changed() override;

private:
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
