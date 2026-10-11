#include <tokotachi/widgets/image_box.hpp>

#include <algorithm>

#include <tokotachi/graphics/canvas.hpp>

#include "../core/property_impl.hpp"

namespace tokotachi
{

ImageBox::ImageBox(Widget* parent_widget, const Image& initial_image)
    : Widget(parent_widget),
      TOKOTACHI_PROPERTY_MEMBER(ImageBox, Image, image),
      TOKOTACHI_PROPERTY_MEMBER(ImageBox, ImageFit, fit),
      image_(initial_image),
      fit_(ImageFit::NONE)
{
}

void ImageBox::set_image(const Image& v)
{
    if (v == image_)
    {
        return;
    }
    const bool resized_image = v.width() != image_.width() || v.height() != image_.height();
    image_ = v;
    invalidate();
    if (resized_image)
    {
        request_layout();
    }
}

void ImageBox::set_fit(const ImageFit& v)
{
    fit_ = v;
    invalidate();
}

Size ImageBox::compute_preferred_size() const
{
    return image_.is_null() ? Size(0, 0) : Size(static_cast<float>(image_.width()), static_cast<float>(image_.height()));
}

void ImageBox::paint(Canvas& canvas)
{
    if (!image_.is_null() && image_.width() > 0 && image_.height() > 0)
    {
        const float w = width, h = height;
        float iw = static_cast<float>(image_.width()), ih = static_cast<float>(image_.height());
        if (fit_ == ImageFit::STRETCH)
        {
            iw = w;
            ih = h;
        }
        else if (fit_ == ImageFit::CONTAIN || fit_ == ImageFit::COVER)
        {
            const float sx = w / iw, sy = h / ih;
            const float s = fit_ == ImageFit::CONTAIN ? std::min(sx, sy) : std::max(sx, sy);
            iw *= s;
            ih *= s;
        }
        canvas.save();
        canvas.clip_rect(Rect(0, 0, w, h));
        canvas.draw_image(image_, Rect((w - iw) / 2, (h - ih) / 2, iw, ih), effectively_enabled ? 1.0f : 0.4f);
        canvas.restore();
    }
    Widget::paint(canvas);
}

}  // namespace tokotachi
