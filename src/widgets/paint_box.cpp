#include <gui/paint_box.hpp>

#include <cmath>

#include <gui/canvas.hpp>

#include "../core/property_impl.hpp"

namespace gui
{

PaintBox::PaintBox(Widget* parent_widget)
    : Widget(parent_widget), GUI_PROPERTY_MEMBER(PaintBox, bool, cached), cached_(false), valid_(false)
{
}

void PaintBox::set_cached(const bool& v)
{
    cached_ = v;
    refresh();
}

void PaintBox::refresh()
{
    valid_ = false;
    cache_ = Image();
    invalidate();
}

void PaintBox::resized()
{
    valid_ = false;
    Widget::resized();
}

void PaintBox::scale_changed()
{
    valid_ = false;
    Widget::scale_changed();
}

void PaintBox::theme_changed()
{
    valid_ = false;  // テーマの色で描いているかもしれない
    Widget::theme_changed();
}

void PaintBox::paint(Canvas& canvas)
{
    if (!cached_)
    {
        Widget::paint(canvas);  // on_paint を呼ぶ
        return;
    }
    const Rect r = local_rect;
    if (!valid_)
    {
        // 今の大きさ・倍率の Bitmap に描いて覚える。
        const float scale = canvas.scale();
        Bitmap bitmap(static_cast<int>(std::ceil(r.width * scale)), static_cast<int>(std::ceil(r.height * scale)), scale);
        const PaintHandler handler = on_paint;
        if (handler && !bitmap.is_null())
        {
            Object* self = this;
            handler(self, bitmap.canvas());
        }
        cache_ = bitmap.to_image();
        valid_ = true;
    }
    if (!cache_.is_null())
    {
        canvas.draw_image(cache_, Rect(0, 0, cache_.width() / canvas.scale(), cache_.height() / canvas.scale()));
    }
}

}  // namespace gui
