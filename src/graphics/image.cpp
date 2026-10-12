#include <tokotachi/graphics/image.hpp>

#include <plutovg.h>

#include <tokotachi/graphics/bitmap.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "icon_data.hpp"
#include "image_data.hpp"

namespace tokotachi
{

namespace detail
{

ImageData::~ImageData()
{
    plutovg_surface_destroy(surface);
    for (std::size_t i = 0; i < scaled_.size(); ++i)
    {
        plutovg_surface_destroy(scaled_[i].surface);
    }
}

namespace
{

const std::size_t SCALED_CACHE = 4;  // 覚えておく大きさの数

// 1 次元の写し方: 出力の画素ごとの、入力の画素とその重み(和は 1)。
struct Tap
{
    int index;
    float weight;
};

std::vector<std::vector<Tap>> taps(int from, int to)
{
    std::vector<std::vector<Tap>> out(static_cast<std::size_t>(to));
    const float ratio = static_cast<float>(from) / to;
    for (int i = 0; i < to; ++i)
    {
        std::vector<Tap>& t = out[static_cast<std::size_t>(i)];
        if (to < from)
        {
            // 縮める: 出力の画素が覆う入力の範囲 [a, b) と、入力の画素ごとの重なり
            const float a = i * ratio, b = (i + 1) * ratio;
            for (int k = static_cast<int>(std::floor(a)); k < static_cast<int>(std::ceil(b)); ++k)
            {
                const float w = std::min(b, k + 1.0f) - std::max(a, static_cast<float>(k));
                if (w > 0)
                {
                    const Tap tap = {std::min(k, from - 1), w / ratio};
                    t.push_back(tap);
                }
            }
        }
        else
        {
            // 広げる: 画素の中心どうしの線形補間(端の外は端の画素)
            const float c = std::max(0.0f, std::min(from - 1.0f, (i + 0.5f) * ratio - 0.5f));
            const int k = static_cast<int>(std::floor(c));
            const float f = c - k;
            const Tap t0 = {k, 1 - f};
            t.push_back(t0);
            if (f > 0 && k + 1 < from)
            {
                const Tap t1 = {k + 1, f};
                t.push_back(t1);
            }
        }
    }
    return out;
}

// アルファ乗算済みの ARGB32 を width × height へ写す(横、縦の順に分けて)。
plutovg_surface_t* resample(plutovg_surface_t* src, int width, int height)
{
    const int sw = plutovg_surface_get_width(src), sh = plutovg_surface_get_height(src);
    plutovg_surface_t* dst = plutovg_surface_create(width, height);
    if (!dst)
    {
        return nullptr;
    }
    const std::vector<std::vector<Tap>> tx = taps(sw, width), ty = taps(sh, height);
    const unsigned char* sbytes = plutovg_surface_get_data(src);
    const int sstride = plutovg_surface_get_stride(src);
    // 横に写した中間(sh 行 × width 列。成分ごと)
    std::vector<float> mid(static_cast<std::size_t>(sh) * width * 4);
    for (int y = 0; y < sh; ++y)
    {
        const std::uint32_t* row = reinterpret_cast<const std::uint32_t*>(sbytes + y * sstride);
        for (int x = 0; x < width; ++x)
        {
            float* m = &mid[(static_cast<std::size_t>(y) * width + x) * 4];
            const std::vector<Tap>& t = tx[static_cast<std::size_t>(x)];
            for (std::size_t k = 0; k < t.size(); ++k)
            {
                const std::uint32_t p = row[t[k].index];
                for (int c = 0; c < 4; ++c)
                {
                    m[c] += static_cast<float>((p >> (c * 8)) & 0xff) * t[k].weight;
                }
            }
        }
    }
    unsigned char* dbytes = plutovg_surface_get_data(dst);
    const int dstride = plutovg_surface_get_stride(dst);
    for (int y = 0; y < height; ++y)
    {
        std::uint32_t* row = reinterpret_cast<std::uint32_t*>(dbytes + y * dstride);
        const std::vector<Tap>& t = ty[static_cast<std::size_t>(y)];
        for (int x = 0; x < width; ++x)
        {
            float acc[4] = {0, 0, 0, 0};
            for (std::size_t k = 0; k < t.size(); ++k)
            {
                const float* m = &mid[(static_cast<std::size_t>(t[k].index) * width + x) * 4];
                for (int c = 0; c < 4; ++c)
                {
                    acc[c] += m[c] * t[k].weight;
                }
            }
            // アルファ乗算済みなので、色の成分はアルファを超えない
            const float alpha = std::min(255.0f, std::max(0.0f, acc[3]));
            std::uint32_t p = static_cast<std::uint32_t>(alpha + 0.5f) << 24;
            for (int c = 0; c < 3; ++c)
            {
                p |= static_cast<std::uint32_t>(std::min(alpha, std::max(0.0f, acc[c])) + 0.5f) << (c * 8);
            }
            row[x] = p;
        }
    }
    return dst;
}

}  // namespace

plutovg_surface* ImageData::scaled(int width, int height) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (std::size_t i = 0; i < scaled_.size(); ++i)
    {
        if (scaled_[i].width == width && scaled_[i].height == height)
        {
            // 見つけたものを先頭へ
            const Scaled found = scaled_[i];
            scaled_.erase(scaled_.begin() + static_cast<std::ptrdiff_t>(i));
            scaled_.insert(scaled_.begin(), found);
            return plutovg_surface_reference(found.surface);
        }
    }
    plutovg_surface* s = resample(surface, width, height);
    if (!s)
    {
        return nullptr;
    }
    if (scaled_.size() >= SCALED_CACHE)
    {
        plutovg_surface_destroy(scaled_.back().surface);
        scaled_.pop_back();
    }
    const Scaled entry = {width, height, s};
    scaled_.insert(scaled_.begin(), entry);
    return plutovg_surface_reference(s);
}

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

// アイコンを作る(24 × 24 に描いた画像も持つ。画素を読む・線画で描けないときに使う)。
Image Image::make_icon(Icon icon, bool has_color, const Color& color)
{
    const int index = static_cast<int>(icon);
    if (index < 0 || index >= detail::ICON_COUNT)
    {
        return Image();
    }
    const int size = static_cast<int>(detail::ICON_GRID);
    Bitmap bitmap(size, size);
    detail::draw_icon(bitmap.canvas(), index, Rect(0, 0, detail::ICON_GRID, detail::ICON_GRID),
                      has_color ? color : Color(0, 0, 0));
    plutovg_surface_t* s = plutovg_surface_reference(const_cast<plutovg_surface_t*>(bitmap.to_image().data()->surface));
    std::shared_ptr<detail::ImageData> d = std::make_shared<detail::ImageData>(s);
    d->icon = index;
    d->has_color = has_color;
    d->color = color;
    return Image(d);
}

Image Image::icon(Icon icon) { return make_icon(icon, false, Color(0, 0, 0)); }

Image Image::icon(Icon icon, const Color& color) { return make_icon(icon, true, color); }

bool Image::is_icon() const { return d_ && d_->icon >= 0; }

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
