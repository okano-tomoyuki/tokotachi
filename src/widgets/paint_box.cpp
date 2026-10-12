#include <tokotachi/widgets/paint_box.hpp>

#include <cmath>

#include <tokotachi/graphics/canvas.hpp>

#include "../core/impl.hpp"

namespace tokotachi
{

class PaintBox::Impl
{
    TOKOTACHI_IMPL(PaintBox);

    explicit Impl(PaintBox* object) : q(object) {}

    bool get_cached() const { return cached_; }
    void set_cached(const bool& v);

    bool cached_;
    bool valid_;  ///< cache_ が今の大きさ・倍率で描いたものか
    Image cache_;
};

PaintBox::PaintBox(Widget* parent_widget)
    : Widget(parent_widget), TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, bool, cached),
      d_(new Impl(this))
{
    d_->cached_ = false;
    d_->valid_ = false;
}

PaintBox::~PaintBox() { delete d_; }

void PaintBox::Impl::set_cached(const bool& v)
{
    cached_ = v;
    q->refresh();
}

void PaintBox::refresh()
{
    d_->valid_ = false;
    d_->cache_ = Image();
    invalidate();
}

void PaintBox::resized()
{
    d_->valid_ = false;
    Widget::resized();
}

void PaintBox::scale_changed()
{
    d_->valid_ = false;
    Widget::scale_changed();
}

void PaintBox::theme_changed()
{
    d_->valid_ = false;  // テーマの色で描いているかもしれない
    Widget::theme_changed();
}

void PaintBox::paint(Canvas& canvas)
{
    if (!d_->cached_)
    {
        Widget::paint(canvas);  // on_paint を呼ぶ
        return;
    }
    const Rect r = local_rect;
    if (!d_->valid_)
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
        d_->cache_ = bitmap.to_image();
        d_->valid_ = true;
    }
    if (!d_->cache_.is_null())
    {
        canvas.draw_image(d_->cache_, Rect(0, 0, d_->cache_.width() / canvas.scale(), d_->cache_.height() / canvas.scale()));
    }
}

}  // namespace tokotachi
