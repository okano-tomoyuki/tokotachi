#include <tokotachi/widgets/image_box.hpp>

#include <algorithm>

#include <tokotachi/graphics/canvas.hpp>

#include "../core/impl.hpp"
#include "theme.hpp"

namespace tokotachi
{

class ImageBox::Impl
{
    TOKOTACHI_IMPL(ImageBox);

    explicit Impl(ImageBox* object) : q(object) {}

    void set_image(const Image& v);
    void set_fit(const ImageFit& v);

    Image image_;
    ImageFit fit_;
};

ImageBox::ImageBox(Widget* parent_widget, const Image& initial_image)
    : Widget(parent_widget),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, Image, image),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, ImageFit, fit),
      d_(new Impl(this))
{
    d_->image_ = initial_image;
    d_->fit_ = ImageFit::NONE;
}

ImageBox::~ImageBox() { delete d_; }

void ImageBox::Impl::set_image(const Image& v)
{
    if (v == image_)
    {
        return;
    }
    const bool resized_image = v.width() != image_.width() || v.height() != image_.height();
    image_ = v;
    q->invalidate();
    if (resized_image)
    {
        q->request_layout();
    }
}

void ImageBox::Impl::set_fit(const ImageFit& v)
{
    fit_ = v;
    q->invalidate();
}

Size ImageBox::compute_preferred_size() const
{
    return d_->image_.is_null() ? Size(0, 0) : Size(static_cast<float>(d_->image_.width()), static_cast<float>(d_->image_.height()));
}

void ImageBox::paint(Canvas& canvas)
{
    if (!d_->image_.is_null() && d_->image_.width() > 0 && d_->image_.height() > 0)
    {
        const float w = width, h = height;
        float iw = static_cast<float>(d_->image_.width()), ih = static_cast<float>(d_->image_.height());
        if (d_->fit_ == ImageFit::STRETCH)
        {
            iw = w;
            ih = h;
        }
        else if (d_->fit_ == ImageFit::CONTAIN || d_->fit_ == ImageFit::COVER)
        {
            const float sx = w / iw, sy = h / ih;
            const float s = d_->fit_ == ImageFit::CONTAIN ? std::min(sx, sy) : std::max(sx, sy);
            iw *= s;
            ih *= s;
        }
        canvas.save();
        canvas.clip_rect(Rect(0, 0, w, h));
        const bool enabled_now = effectively_enabled;
        canvas.draw_image(d_->image_, Rect((w - iw) / 2, (h - ih) / 2, iw, ih),
                          enabled_now ? theme::text() : theme::text_disabled(),
                          enabled_now || d_->image_.is_icon() ? 1.0f : 0.4f);
        canvas.restore();
    }
    Widget::paint(canvas);
}

}  // namespace tokotachi
