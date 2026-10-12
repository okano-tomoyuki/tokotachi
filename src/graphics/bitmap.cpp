#include <tokotachi/graphics/bitmap.hpp>

#include <plutovg.h>

#include <algorithm>
#include <cstring>
#include <memory>
#include <vector>

#include "image_data.hpp"
#include "raster_canvas.hpp"

namespace tokotachi
{

namespace
{

std::uint32_t premultiply(const Color& c)
{
    const std::uint32_t a = c.a;
    return (a << 24) | ((c.r * a + 127) / 255 << 16) | ((c.g * a + 127) / 255 << 8) | (c.b * a + 127) / 255;
}

Color unpremultiply(std::uint32_t p)
{
    const std::uint32_t a = p >> 24;
    if (a == 0)
    {
        return Color(0, 0, 0, 0);
    }
    return Color(static_cast<std::uint8_t>((((p >> 16) & 0xff) * 255 + a / 2) / a),
                 static_cast<std::uint8_t>((((p >> 8) & 0xff) * 255 + a / 2) / a),
                 static_cast<std::uint8_t>(((p & 0xff) * 255 + a / 2) / a), static_cast<std::uint8_t>(a));
}

}  // namespace

struct Bitmap::Impl
{
    int width, height;
    float scale;
    std::vector<std::uint32_t> pixels;
    detail::RasterCanvas canvas;

    Impl(int w, int h, float s) : width(w), height(h), scale(s > 0 ? s : 1), pixels(static_cast<std::size_t>(w) * h, 0)
    {
        canvas.set_target(&pixels[0], w, h);
        canvas.set_scale(scale);
        const detail::PixelRect all = {0, 0, w, h};
        canvas.begin(all);
    }
};

Bitmap::Bitmap() : d_(nullptr) {}

Bitmap::Bitmap(int width_value, int height_value, float scale_value) : d_(nullptr)
{
    if (width_value > 0 && height_value > 0)
    {
        d_ = new Impl(width_value, height_value, scale_value);
    }
}

Bitmap::~Bitmap() { delete d_; }

Bitmap::Bitmap(Bitmap&& other) : d_(other.d_) { other.d_ = nullptr; }

Bitmap& Bitmap::operator=(Bitmap&& other)
{
    if (this != &other)
    {
        delete d_;
        d_ = other.d_;
        other.d_ = nullptr;
    }
    return *this;
}

Bitmap Bitmap::from_image(const Image& image, float scale_value)
{
    Bitmap b(image.width(), image.height(), scale_value);
    if (b.d_)
    {
        const plutovg_surface_t* s = image.data()->surface;
        const unsigned char* src = plutovg_surface_get_data(s);
        const int stride = plutovg_surface_get_stride(s);
        for (int y = 0; y < b.d_->height; ++y)
        {
            std::memcpy(&b.d_->pixels[static_cast<std::size_t>(y) * b.d_->width], src + y * stride,
                        static_cast<std::size_t>(b.d_->width) * 4);
        }
    }
    return b;
}

int Bitmap::width() const { return d_ ? d_->width : 0; }

int Bitmap::height() const { return d_ ? d_->height : 0; }

float Bitmap::scale() const { return d_ ? d_->scale : 1; }

Size Bitmap::size() const { return d_ ? Size(d_->width / d_->scale, d_->height / d_->scale) : Size(); }

Canvas& Bitmap::canvas()
{
    if (!d_)
    {
        // 空の画像の Canvas: 1 × 1 の捨てる先に描く(呼び出し側が is_null を確かめなくても落ちないように)。
        static Bitmap sink(1, 1);
        return sink.d_->canvas.api();
    }
    return d_->canvas.api();
}

void Bitmap::clear(const Color& color)
{
    if (d_)
    {
        std::fill(d_->pixels.begin(), d_->pixels.end(), premultiply(color));
    }
}

Color Bitmap::pixel(int x, int y) const
{
    if (!d_ || x < 0 || y < 0 || x >= d_->width || y >= d_->height)
    {
        return Color(0, 0, 0, 0);
    }
    return unpremultiply(d_->pixels[static_cast<std::size_t>(y) * d_->width + x]);
}

void Bitmap::set_pixel(int x, int y, const Color& color)
{
    if (d_ && x >= 0 && y >= 0 && x < d_->width && y < d_->height)
    {
        d_->pixels[static_cast<std::size_t>(y) * d_->width + x] = premultiply(color);
    }
}

std::uint32_t* Bitmap::pixels() { return d_ ? &d_->pixels[0] : nullptr; }

const std::uint32_t* Bitmap::pixels() const { return d_ ? &d_->pixels[0] : nullptr; }

Image Bitmap::to_image() const
{
    if (!d_)
    {
        return Image();
    }
    plutovg_surface_t* s = plutovg_surface_create(d_->width, d_->height);
    if (!s)
    {
        return Image();
    }
    unsigned char* dst = plutovg_surface_get_data(s);
    const int stride = plutovg_surface_get_stride(s);
    for (int y = 0; y < d_->height; ++y)
    {
        std::memcpy(dst + y * stride, &d_->pixels[static_cast<std::size_t>(y) * d_->width],
                    static_cast<std::size_t>(d_->width) * 4);
    }
    return Image(std::make_shared<detail::ImageData>(s));
}

}  // namespace tokotachi
