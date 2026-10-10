#include <tokotachi/graphics/image.hpp>

#include <plutovg.h>

#include <cstdint>

#include "image_data.hpp"

namespace tokotachi
{

namespace detail
{

ImageData::~ImageData() { plutovg_surface_destroy(surface); }

}  // namespace detail

Image::Image() {}

Image Image::load(const std::string& path)
{
    plutovg_surface_t* s = plutovg_surface_load_from_image_file(path.c_str());
    return s ? Image(std::make_shared<detail::ImageData>(s)) : Image();
}

Image Image::load_from_data(const void* data, std::size_t size)
{
    if (!data || size == 0)
    {
        return Image();
    }
    plutovg_surface_t* s = plutovg_surface_load_from_image_data(data, static_cast<int>(size));
    return s ? Image(std::make_shared<detail::ImageData>(s)) : Image();
}

Image Image::from_pixels(int width, int height, const std::vector<Color>& pixels)
{
    if (width <= 0 || height <= 0 || pixels.size() != static_cast<std::size_t>(width) * static_cast<std::size_t>(height))
    {
        return Image();
    }
    plutovg_surface_t* s = plutovg_surface_create(width, height);
    if (!s)
    {
        return Image();
    }
    unsigned char* bytes = plutovg_surface_get_data(s);
    const int stride = plutovg_surface_get_stride(s);
    for (int y = 0; y < height; ++y)
    {
        std::uint32_t* row = reinterpret_cast<std::uint32_t*>(bytes + y * stride);
        for (int x = 0; x < width; ++x)
        {
            const Color& c = pixels[static_cast<std::size_t>(y) * width + x];
            const std::uint32_t a = c.a;
            row[x] = (a << 24) | ((c.r * a + 127) / 255 << 16) | ((c.g * a + 127) / 255 << 8) | (c.b * a + 127) / 255;
        }
    }
    return Image(std::make_shared<detail::ImageData>(s));
}

int Image::width() const { return d_ ? plutovg_surface_get_width(d_->surface) : 0; }

int Image::height() const { return d_ ? plutovg_surface_get_height(d_->surface) : 0; }

Color Image::pixel(int x, int y) const
{
    if (!d_ || x < 0 || y < 0 || x >= width() || y >= height())
    {
        return Color(0, 0, 0, 0);
    }
    const unsigned char* bytes = plutovg_surface_get_data(d_->surface);
    const std::uint32_t p =
        reinterpret_cast<const std::uint32_t*>(bytes + y * plutovg_surface_get_stride(d_->surface))[x];
    const std::uint32_t a = p >> 24;
    if (a == 0)
    {
        return Color(0, 0, 0, 0);
    }
    // アルファ乗算を戻す。
    Color c;
    c.r = static_cast<std::uint8_t>((((p >> 16) & 0xff) * 255 + a / 2) / a);
    c.g = static_cast<std::uint8_t>((((p >> 8) & 0xff) * 255 + a / 2) / a);
    c.b = static_cast<std::uint8_t>(((p & 0xff) * 255 + a / 2) / a);
    c.a = static_cast<std::uint8_t>(a);
    return c;
}

}  // namespace tokotachi
