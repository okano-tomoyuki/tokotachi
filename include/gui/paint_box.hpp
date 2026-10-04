// 自由に描くための部品(VCL の TPaintBox に当たる。docs/adr/0033)。on_paint の中で Canvas に描く。
//
//   gui::PaintBox* chart = new gui::PaintBox(&window);
//   chart->minimum_size = gui::Size(300, 200);
//   chart->cached = true;                     // 重い描画: 描いた結果を覚えておく
//   chart->on_paint = [=](gui::Object*, gui::Canvas& c) { ... 重い描画 ... };
//   chart->refresh();                         // データが変わったら描き直す
//
// cached が true なら、描いた結果を Bitmap に覚えておき、refresh を呼ぶか、大きさ・倍率・テーマが変わるまで on_paint を
// 呼ばない(画面の描き直しでは覚えた絵を貼るだけ)。on_paint の Canvas は、そのときは画面ではなく Bitmap に描く。
#ifndef GUI_PAINT_BOX_HPP
#define GUI_PAINT_BOX_HPP

#include "bitmap.hpp"
#include "image.hpp"
#include "widget.hpp"

namespace gui
{

class PaintBox : public Widget
{
public:
    explicit PaintBox(Widget* parent = nullptr);

    // 描いた結果を覚えておくか(既定 false: 描き直すたびに on_paint を呼ぶ)。
    Property<bool> cached;

    // 描き直す(cached なら覚えている絵を捨てる)。
    void refresh();

protected:
    void paint(Canvas& canvas) override;
    void resized() override;
    void scale_changed() override;
    void theme_changed() override;

private:
    bool get_cached() const { return cached_; }
    void set_cached(const bool& v);

    bool cached_;
    bool valid_;  // cache_ が今の大きさ・倍率で描いたものか
    Image cache_;
};

}  // namespace gui

#endif
