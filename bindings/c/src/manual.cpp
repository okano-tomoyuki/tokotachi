// C API の手で書く関数(api.py の F)。
#include "common.hpp"

#include <SDL3/SDL.h>

#include <cstdlib>
#include <cstring>

namespace c = tokotachi::c;
using tokotachi::Object;

namespace
{

tokotachi::Canvas& canvas_of(tokotachi_canvas* canvas)
{
    if (!canvas)
    {
        throw std::invalid_argument("the canvas is NULL");
    }
    return *reinterpret_cast<tokotachi::Canvas*>(canvas);
}

template <typename T>
T& checked(T* p, const char* what)
{
    if (!p)
    {
        throw std::invalid_argument(std::string("the ") + what + " is NULL");
    }
    return *p;
}

const tokotachi::Image& image_of(const tokotachi_image* image) { return checked(image, "image").image; }
tokotachi::Bitmap& bitmap_of(tokotachi_bitmap* bitmap) { return checked(bitmap, "bitmap").bitmap; }
const tokotachi::Bitmap& bitmap_of(const tokotachi_bitmap* bitmap) { return checked(bitmap, "bitmap").bitmap; }
tokotachi::Path& path_of(tokotachi_path* path) { return checked(path, "path").path; }
const tokotachi::Path& path_of(const tokotachi_path* path) { return checked(path, "path").path; }
const tokotachi::TextLayout& layout_of(const tokotachi_text_layout* layout)
{
    return checked(layout, "text layout").layout;
}

// ---- 利用者のデータ(tokotachi_object_set_user_data) ----

/// 対象の Object が所有する、見えない Object。対象と一緒に破棄されるときに destroy を呼ぶ。
class UserDataHolder : public Object
{
public:
    UserDataHolder(Object* target, void* data, tokotachi_destroy_fn destroy)
        : Object(target), data_(data), destroy_(destroy)
    {
    }

    ~UserDataHolder()
    {
        if (destroy_)
        {
            destroy_(data_);
        }
    }

    void* data() const { return data_; }

private:
    void* data_;
    tokotachi_destroy_fn destroy_;
};

/// target の UserDataHolder(登録の表。Object は所有しているものを外に見せないため)。
std::vector<std::pair<Object*, UserDataHolder*> >& holders()
{
    static std::vector<std::pair<Object*, UserDataHolder*> >* list =
        new std::vector<std::pair<Object*, UserDataHolder*> >();
    return *list;
}

UserDataHolder* holder_of(Object* target)
{
    std::vector<std::pair<Object*, UserDataHolder*> >& list = holders();
    for (std::size_t i = 0; i < list.size(); ++i)
    {
        if (list[i].first == target)
        {
            return list[i].second;
        }
    }
    return nullptr;
}

/// 表から外す UserDataHolder。
class TrackedHolder : public UserDataHolder
{
public:
    TrackedHolder(Object* target, void* data, tokotachi_destroy_fn destroy)
        : UserDataHolder(target, data, destroy), target_(target)
    {
        holders().push_back(std::make_pair(target, static_cast<UserDataHolder*>(this)));
    }

    ~TrackedHolder()
    {
        std::vector<std::pair<Object*, UserDataHolder*> >& list = holders();
        for (std::size_t i = 0; i < list.size(); ++i)
        {
            if (list[i].second == this)
            {
                list.erase(list.begin() + static_cast<std::ptrdiff_t>(i));
                break;
            }
        }
    }

private:
    Object* target_;
};

// ---- Variable ----

template <typename T>
tokotachi::Variable<T>* variable_as(tokotachi_object* h, const char* function)
{
    Object* o = c::self_as<Object>(h, function);
    tokotachi::Variable<T>* v = dynamic_cast<tokotachi::Variable<T>*>(o);
    if (!v)
    {
        throw std::invalid_argument("the variable has another type (" + std::string(function) + ")");
    }
    return v;
}

// ---- Task ----

}  // namespace

struct tokotachi_task_context
{
    tokotachi::TaskContext* context;
    bool failed;
    std::string message;
};

extern "C" {

// ---- 全体 ----

TOKOTACHI_C_API const char* tokotachi_version(void) { return tokotachi::version(); }

TOKOTACHI_C_API void tokotachi_free(void* p) { std::free(p); }

TOKOTACHI_C_API void tokotachi_strv_free(char** strv)
{
    if (!strv)
    {
        return;
    }
    for (char** p = strv; *p; ++p)
    {
        std::free(*p);
    }
    std::free(strv);
}

TOKOTACHI_C_API size_t tokotachi_struct_size(const char* name)
{
    struct Entry
    {
        const char* name;
        size_t size;
    };
    static const Entry sizes[] = {
        {"tokotachi_point", sizeof(tokotachi_point)},
        {"tokotachi_size", sizeof(tokotachi_size)},
        {"tokotachi_rect", sizeof(tokotachi_rect)},
        {"tokotachi_color", sizeof(tokotachi_color)},
        {"tokotachi_margins", sizeof(tokotachi_margins)},
        {"tokotachi_font", sizeof(tokotachi_font)},
        {"tokotachi_shortcut", sizeof(tokotachi_shortcut)},
        {"tokotachi_theme_color", sizeof(tokotachi_theme_color)},
        {"tokotachi_model_index", sizeof(tokotachi_model_index)},
        {"tokotachi_placement", sizeof(tokotachi_placement)},
        {"tokotachi_theme", sizeof(tokotachi_theme)},
        {"tokotachi_mouse_event", sizeof(tokotachi_mouse_event)},
        {"tokotachi_key_event", sizeof(tokotachi_key_event)},
        {"tokotachi_wheel_event", sizeof(tokotachi_wheel_event)},
        {"tokotachi_gradient_stop", sizeof(tokotachi_gradient_stop)},
        {"tokotachi_paint", sizeof(tokotachi_paint)},
        {"tokotachi_stroke_style", sizeof(tokotachi_stroke_style)},
        {"tokotachi_model_callbacks", sizeof(tokotachi_model_callbacks)},
    };
    for (size_t i = 0; name && i < sizeof(sizes) / sizeof(sizes[0]); ++i)
    {
        if (std::strcmp(sizes[i].name, name) == 0)
        {
            return sizes[i].size;
        }
    }
    return 0;
}

// ---- Object ----

TOKOTACHI_C_API void tokotachi_object_delete(tokotachi_object* object)
{
    try
    {
        delete c::object_of(object);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_object_delete_later(tokotachi_object* object)
{
    try
    {
        c::self_as<Object>(object, __func__)->delete_later();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_object_set_user_data(tokotachi_object* object, void* data, tokotachi_destroy_fn destroy)
{
    try
    {
        Object* target = c::self_as<Object>(object, __func__);
        delete holder_of(target);
        new TrackedHolder(target, data, destroy);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void* tokotachi_object_get_user_data(tokotachi_object* object)
{
    try
    {
        UserDataHolder* h = holder_of(c::self_as<Object>(object, __func__));
        return h ? h->data() : nullptr;
    }
    TOKOTACHI_C_CATCH(nullptr)
}

// ---- 結びつけ ----

TOKOTACHI_C_API bool tokotachi_bind(tokotachi_object* target, const char* target_property, tokotachi_object* source,
                                    const char* source_property)
{
    try
    {
        void* tp = nullptr;
        void* sp = nullptr;
        const c::PropertyEntry* t = c::find_entry(c::self_as<Object>(target, __func__), target_property, &tp);
        const c::PropertyEntry* s = c::find_entry(c::self_as<Object>(source, __func__), source_property, &sp);
        if (!t || !s)
        {
            throw std::invalid_argument(std::string("no such property: ") + (!t ? c::str(target_property)
                                                                                 : c::str(source_property)));
        }
        if (t->read_only)
        {
            throw std::invalid_argument("the target property is read-only: " + c::str(target_property));
        }
        if (*t->type != *s->type)
        {
            throw std::invalid_argument("the properties have different types");
        }
        t->bind(tp, sp, s->read_only);
        return true;
    }
    TOKOTACHI_C_CATCH(false)
}

TOKOTACHI_C_API bool tokotachi_unbind(tokotachi_object* a, const char* a_property, tokotachi_object* b,
                                      const char* b_property)
{
    try
    {
        void* ap = nullptr;
        void* bp = nullptr;
        const c::PropertyEntry* ea = c::find_entry(c::self_as<Object>(a, __func__), a_property, &ap);
        const c::PropertyEntry* eb = c::find_entry(c::self_as<Object>(b, __func__), b_property, &bp);
        if (!ea || !eb)
        {
            throw std::invalid_argument("no such property");
        }
        tokotachi::detail::remove_binding_pair(ap, bp);
        return true;
    }
    TOKOTACHI_C_CATCH(false)
}

TOKOTACHI_C_API bool tokotachi_unbind_all(tokotachi_object* object, const char* property)
{
    try
    {
        void* p = nullptr;
        const c::PropertyEntry* e = c::find_entry(c::self_as<Object>(object, __func__), property, &p);
        if (!e)
        {
            throw std::invalid_argument("no such property: " + c::str(property));
        }
        e->unbind_all(p, e->read_only);
        return true;
    }
    TOKOTACHI_C_CATCH(false)
}

// ---- スレッド ----

TOKOTACHI_C_API bool tokotachi_is_main_thread(void) { return tokotachi::is_main_thread(); }

TOKOTACHI_C_API bool tokotachi_post(tokotachi_callback_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        std::shared_ptr<c::UserData> data = std::make_shared<c::UserData>(user_data, destroy);
        if (!fn)
        {
            return false;
        }
        return tokotachi::post([fn, data]() { fn(data->data); });
    }
    TOKOTACHI_C_CATCH(false)
}

// ---- クリップボード・ショートカット・テーマ ----

TOKOTACHI_C_API char* tokotachi_clipboard_text(void)
{
    try
    {
        return c::dup(tokotachi::clipboard::text());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_clipboard_set_text(const char* text)
{
    try
    {
        tokotachi::clipboard::set_text(c::str(text));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_clipboard_has_text(void)
{
    try
    {
        return tokotachi::clipboard::has_text();
    }
    TOKOTACHI_C_CATCH(false)
}

TOKOTACHI_C_API tokotachi_shortcut tokotachi_shortcut_parse(const char* text)
{
    try
    {
        return c::to_c(tokotachi::Shortcut(c::str(text)));
    }
    TOKOTACHI_C_CATCH(tokotachi_shortcut())
}

TOKOTACHI_C_API char* tokotachi_shortcut_to_string(tokotachi_shortcut shortcut)
{
    try
    {
        return c::dup(c::from_c(shortcut).to_string());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_theme_light(tokotachi_theme* out)
{
    try
    {
        checked(out, "output") = c::to_c(tokotachi::Theme::light());
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_theme_dark(tokotachi_theme* out)
{
    try
    {
        checked(out, "output") = c::to_c(tokotachi::Theme::dark());
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_color tokotachi_theme_color_resolve(tokotachi_theme_color color)
{
    try
    {
        return c::to_c(c::from_c(color).resolve());
    }
    TOKOTACHI_C_CATCH(tokotachi_color())
}

// ---- Image ----

TOKOTACHI_C_API tokotachi_image* tokotachi_image_load(const char* path)
{
    try
    {
        return c::to_c_image(tokotachi::Image::load(c::str(path)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_image* tokotachi_image_load_from_data(const void* data, size_t size)
{
    try
    {
        return c::to_c_image(tokotachi::Image::load_from_data(data, size));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_image* tokotachi_image_from_pixels(int width, int height, const tokotachi_color* pixels,
                                                             size_t count)
{
    try
    {
        std::vector<tokotachi::Color> v;
        v.reserve(count);
        for (size_t i = 0; i < count; ++i)
        {
            v.push_back(c::from_c(pixels[i]));
        }
        return c::to_c_image(tokotachi::Image::from_pixels(width, height, v));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_image* tokotachi_image_copy(const tokotachi_image* image)
{
    try
    {
        return image ? c::to_c_image(image->image) : nullptr;
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_image_free(tokotachi_image* image) { delete image; }

TOKOTACHI_C_API int tokotachi_image_width(const tokotachi_image* image)
{
    try
    {
        return image_of(image).width();
    }
    TOKOTACHI_C_CATCH(0)
}

TOKOTACHI_C_API int tokotachi_image_height(const tokotachi_image* image)
{
    try
    {
        return image_of(image).height();
    }
    TOKOTACHI_C_CATCH(0)
}

TOKOTACHI_C_API tokotachi_color tokotachi_image_pixel(const tokotachi_image* image, int x, int y)
{
    try
    {
        return c::to_c(image_of(image).pixel(x, y));
    }
    TOKOTACHI_C_CATCH(tokotachi_color())
}

TOKOTACHI_C_API bool tokotachi_image_equal(const tokotachi_image* a, const tokotachi_image* b)
{
    return c::from_c(a) == c::from_c(b);
}

// ---- Bitmap ----

TOKOTACHI_C_API tokotachi_bitmap* tokotachi_bitmap_new(int width, int height, float scale)
{
    try
    {
        if (width <= 0 || height <= 0 || !(scale > 0))
        {
            throw std::invalid_argument("invalid size or scale");
        }
        return c::to_c_bitmap(tokotachi::Bitmap(width, height, scale));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_bitmap* tokotachi_bitmap_from_image(const tokotachi_image* image, float scale)
{
    try
    {
        return c::to_c_bitmap(tokotachi::Bitmap::from_image(image_of(image), scale));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_bitmap_free(tokotachi_bitmap* bitmap) { delete bitmap; }

TOKOTACHI_C_API int tokotachi_bitmap_width(const tokotachi_bitmap* bitmap)
{
    try
    {
        return bitmap_of(bitmap).width();
    }
    TOKOTACHI_C_CATCH(0)
}

TOKOTACHI_C_API int tokotachi_bitmap_height(const tokotachi_bitmap* bitmap)
{
    try
    {
        return bitmap_of(bitmap).height();
    }
    TOKOTACHI_C_CATCH(0)
}

TOKOTACHI_C_API float tokotachi_bitmap_scale(const tokotachi_bitmap* bitmap)
{
    try
    {
        return bitmap_of(bitmap).scale();
    }
    TOKOTACHI_C_CATCH(0)
}

TOKOTACHI_C_API tokotachi_canvas* tokotachi_bitmap_canvas(tokotachi_bitmap* bitmap)
{
    try
    {
        return reinterpret_cast<tokotachi_canvas*>(&bitmap_of(bitmap).canvas());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_bitmap_clear(tokotachi_bitmap* bitmap, tokotachi_color color)
{
    try
    {
        bitmap_of(bitmap).clear(c::from_c(color));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_color tokotachi_bitmap_pixel(const tokotachi_bitmap* bitmap, int x, int y)
{
    try
    {
        return c::to_c(bitmap_of(bitmap).pixel(x, y));
    }
    TOKOTACHI_C_CATCH(tokotachi_color())
}

TOKOTACHI_C_API void tokotachi_bitmap_set_pixel(tokotachi_bitmap* bitmap, int x, int y, tokotachi_color color)
{
    try
    {
        bitmap_of(bitmap).set_pixel(x, y, c::from_c(color));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API uint32_t* tokotachi_bitmap_pixels(tokotachi_bitmap* bitmap)
{
    try
    {
        return bitmap_of(bitmap).pixels();
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_image* tokotachi_bitmap_to_image(const tokotachi_bitmap* bitmap)
{
    try
    {
        return c::to_c_image(bitmap_of(bitmap).to_image());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API bool tokotachi_bitmap_save_png(const tokotachi_bitmap* bitmap, const char* path)
{
    try
    {
        const tokotachi::Bitmap& b = bitmap_of(bitmap);
        const int w = b.width(), h = b.height();
        // アルファを乗算していない RGBA(バイトの順)にして SDL で保存する。
        std::vector<std::uint8_t> rgba(static_cast<std::size_t>(w) * h * 4);
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                const tokotachi::Color p = b.pixel(x, y);
                std::uint8_t* d = &rgba[(static_cast<std::size_t>(y) * w + x) * 4];
                d[0] = p.r;
                d[1] = p.g;
                d[2] = p.b;
                d[3] = p.a;
            }
        }
        SDL_Surface* s = SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_RGBA32, rgba.data(), w * 4);
        const bool ok = s && SDL_SavePNG(s, c::str(path).c_str());
        const std::string error = ok ? std::string() : SDL_GetError();
        SDL_DestroySurface(s);
        if (!ok)
        {
            throw std::runtime_error("cannot save the PNG: " + error);
        }
        return true;
    }
    TOKOTACHI_C_CATCH(false)
}

// ---- Path ----

TOKOTACHI_C_API tokotachi_path* tokotachi_path_new(void)
{
    try
    {
        return new tokotachi_path;
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_path_free(tokotachi_path* path) { delete path; }

TOKOTACHI_C_API void tokotachi_path_move_to(tokotachi_path* path, float x, float y)
{
    try
    {
        path_of(path).move_to(x, y);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_path_line_to(tokotachi_path* path, float x, float y)
{
    try
    {
        path_of(path).line_to(x, y);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_path_quad_to(tokotachi_path* path, float x1, float y1, float x2, float y2)
{
    try
    {
        path_of(path).quad_to(x1, y1, x2, y2);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_path_cubic_to(tokotachi_path* path, float x1, float y1, float x2, float y2, float x3,
                                             float y3)
{
    try
    {
        path_of(path).cubic_to(x1, y1, x2, y2, x3, y3);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_path_close(tokotachi_path* path)
{
    try
    {
        path_of(path).close();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_path_add_rect(tokotachi_path* path, tokotachi_rect rect)
{
    try
    {
        path_of(path).add_rect(c::from_c(rect));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_path_add_round_rect(tokotachi_path* path, tokotachi_rect rect, float radius)
{
    try
    {
        path_of(path).add_round_rect(c::from_c(rect), radius);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_path_add_ellipse(tokotachi_path* path, tokotachi_point center, float rx, float ry)
{
    try
    {
        path_of(path).add_ellipse(c::from_c(center), rx, ry);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_path_add_arc(tokotachi_path* path, tokotachi_point center, float rx, float ry,
                                            float start_angle, float sweep_angle, bool new_figure)
{
    try
    {
        path_of(path).add_arc(c::from_c(center), rx, ry, start_angle, sweep_angle, new_figure);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_path_add_polyline(tokotachi_path* path, const tokotachi_point* points, size_t count,
                                                 bool close)
{
    try
    {
        std::vector<tokotachi::Point> v;
        for (size_t i = 0; i < count; ++i)
        {
            v.push_back(c::from_c(points[i]));
        }
        path_of(path).add_polyline(v, close);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_path_empty(const tokotachi_path* path)
{
    try
    {
        return path_of(path).empty();
    }
    TOKOTACHI_C_CATCH(true)
}

// ---- TextLayout ----

TOKOTACHI_C_API tokotachi_text_layout* tokotachi_text_layout_new(const char* text, tokotachi_font font, float scale,
                                                                 float wrap_width, tokotachi_align alignment)
{
    try
    {
        tokotachi_text_layout* p = new tokotachi_text_layout;
        p->layout = tokotachi::TextLayout(c::str(text), c::from_c(font), scale, wrap_width,
                                          c::enum_in<tokotachi::Align>(alignment, 4));
        return p;
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_text_layout_free(tokotachi_text_layout* layout) { delete layout; }

#define TOKOTACHI_C_LAYOUT_GETTER(type, name)                                                                         \
    TOKOTACHI_C_API type tokotachi_text_layout_##name(const tokotachi_text_layout* layout)                             \
    {                                                                                                                  \
        try                                                                                                            \
        {                                                                                                              \
            return layout_of(layout).name();                                                                           \
        }                                                                                                              \
        TOKOTACHI_C_CATCH(0)                                                                                           \
    }

TOKOTACHI_C_LAYOUT_GETTER(float, width)
TOKOTACHI_C_LAYOUT_GETTER(float, height)
TOKOTACHI_C_LAYOUT_GETTER(float, ascent)
TOKOTACHI_C_LAYOUT_GETTER(float, descent)
TOKOTACHI_C_LAYOUT_GETTER(float, line_height)
TOKOTACHI_C_LAYOUT_GETTER(size_t, line_count)

TOKOTACHI_C_API tokotachi_rect tokotachi_text_layout_line_rect(const tokotachi_text_layout* layout, size_t line)
{
    try
    {
        return c::to_c(layout_of(layout).line_rect(line));
    }
    TOKOTACHI_C_CATCH(tokotachi_rect())
}

TOKOTACHI_C_API tokotachi_point tokotachi_text_layout_caret_position(const tokotachi_text_layout* layout,
                                                                     size_t offset)
{
    try
    {
        return c::to_c(layout_of(layout).caret_position(offset));
    }
    TOKOTACHI_C_CATCH(tokotachi_point())
}

TOKOTACHI_C_API size_t tokotachi_text_layout_offset_at(const tokotachi_text_layout* layout, tokotachi_point p)
{
    try
    {
        return layout_of(layout).offset_at(c::from_c(p));
    }
    TOKOTACHI_C_CATCH(0)
}

// ---- Canvas ----

TOKOTACHI_C_API float tokotachi_canvas_scale(tokotachi_canvas* canvas)
{
    try
    {
        return canvas_of(canvas).scale();
    }
    TOKOTACHI_C_CATCH(0)
}

TOKOTACHI_C_API void tokotachi_canvas_save(tokotachi_canvas* canvas)
{
    try
    {
        canvas_of(canvas).save();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_canvas_restore(tokotachi_canvas* canvas)
{
    try
    {
        canvas_of(canvas).restore();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_canvas_translate(tokotachi_canvas* canvas, float dx, float dy)
{
    try
    {
        canvas_of(canvas).translate(dx, dy);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_canvas_scale_by(tokotachi_canvas* canvas, float sx, float sy)
{
    try
    {
        canvas_of(canvas).scale_by(sx, sy);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_canvas_rotate(tokotachi_canvas* canvas, float radians)
{
    try
    {
        canvas_of(canvas).rotate(radians);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_canvas_clip_rect(tokotachi_canvas* canvas, tokotachi_rect rect)
{
    try
    {
        canvas_of(canvas).clip_rect(c::from_c(rect));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_canvas_clip_path(tokotachi_canvas* canvas, const tokotachi_path* path)
{
    try
    {
        canvas_of(canvas).clip_path(path_of(path));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_rect tokotachi_canvas_clip_bounds(tokotachi_canvas* canvas)
{
    try
    {
        return c::to_c(canvas_of(canvas).clip_bounds());
    }
    TOKOTACHI_C_CATCH(tokotachi_rect())
}

TOKOTACHI_C_API void tokotachi_canvas_begin_layer(tokotachi_canvas* canvas, float opacity)
{
    try
    {
        canvas_of(canvas).begin_layer(opacity);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_canvas_end_layer(tokotachi_canvas* canvas)
{
    try
    {
        canvas_of(canvas).end_layer();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_canvas_fill_rect(tokotachi_canvas* canvas, tokotachi_rect rect, tokotachi_color color)
{
    try
    {
        canvas_of(canvas).fill_rect(c::from_c(rect), c::from_c(color));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_canvas_fill_round_rect(tokotachi_canvas* canvas, tokotachi_rect rect, float radius,
                                                      const tokotachi_paint* paint)
{
    try
    {
        canvas_of(canvas).fill_round_rect(c::from_c(rect), radius, c::from_c(paint));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_canvas_stroke_round_rect(tokotachi_canvas* canvas, tokotachi_rect rect, float radius,
                                                        float width, tokotachi_color color)
{
    try
    {
        canvas_of(canvas).stroke_round_rect(c::from_c(rect), radius, width, c::from_c(color));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_canvas_stroke_rect(tokotachi_canvas* canvas, tokotachi_rect rect, float width,
                                                  tokotachi_color color)
{
    try
    {
        canvas_of(canvas).stroke_rect(c::from_c(rect), width, c::from_c(color));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_canvas_fill_path(tokotachi_canvas* canvas, const tokotachi_path* path,
                                                const tokotachi_paint* paint)
{
    try
    {
        canvas_of(canvas).fill_path(path_of(path), c::from_c(paint));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_canvas_stroke_path(tokotachi_canvas* canvas, const tokotachi_path* path,
                                                  const tokotachi_stroke_style* style, const tokotachi_paint* paint)
{
    try
    {
        canvas_of(canvas).stroke_path(path_of(path), c::from_c(style), c::from_c(paint));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_canvas_draw_line(tokotachi_canvas* canvas, tokotachi_point from, tokotachi_point to,
                                                const tokotachi_stroke_style* style, const tokotachi_paint* paint)
{
    try
    {
        canvas_of(canvas).draw_line(c::from_c(from), c::from_c(to), c::from_c(style), c::from_c(paint));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_canvas_fill_ellipse(tokotachi_canvas* canvas, tokotachi_rect rect,
                                                   const tokotachi_paint* paint)
{
    try
    {
        canvas_of(canvas).fill_ellipse(c::from_c(rect), c::from_c(paint));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_canvas_stroke_ellipse(tokotachi_canvas* canvas, tokotachi_rect rect,
                                                     const tokotachi_stroke_style* style, const tokotachi_paint* paint)
{
    try
    {
        canvas_of(canvas).stroke_ellipse(c::from_c(rect), c::from_c(style), c::from_c(paint));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_canvas_draw_text(tokotachi_canvas* canvas, const char* text, tokotachi_point pos,
                                                tokotachi_color color, tokotachi_font font)
{
    try
    {
        canvas_of(canvas).draw_text(c::str(text), c::from_c(pos), c::from_c(color), c::from_c(font));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_canvas_draw_text_layout(tokotachi_canvas* canvas, const tokotachi_text_layout* layout,
                                                       tokotachi_point pos, tokotachi_color color)
{
    try
    {
        canvas_of(canvas).draw_text(layout_of(layout), c::from_c(pos), c::from_c(color));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_canvas_draw_image(tokotachi_canvas* canvas, const tokotachi_image* image,
                                                 tokotachi_rect dst, float opacity)
{
    try
    {
        canvas_of(canvas).draw_image(image_of(image), c::from_c(dst), opacity);
    }
    TOKOTACHI_C_CATCH()
}

// ---- Application・MessageDialog ----

TOKOTACHI_C_API tokotachi_application* tokotachi_application_instance(void)
{
    return c::handle(tokotachi::Application::instance());
}

TOKOTACHI_C_API tokotachi_message_dialog* tokotachi_message_dialog_show(
    tokotachi_window* parent, const char* title, const char* text, const tokotachi_dialog_button* buttons,
    size_t button_count, tokotachi_notify_fn on_finish, void* user_data, tokotachi_destroy_fn destroy,
    tokotachi_message_type type)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(on_finish, user_data, destroy);
        std::vector<tokotachi::DialogButton> v;
        for (size_t i = 0; i < button_count; ++i)
        {
            v.push_back(c::enum_in<tokotachi::DialogButton>(buttons[i], 8));
        }
        return c::handle(tokotachi::MessageDialog::show(c::self_as<tokotachi::Window>(parent, __func__), c::str(title),
                                                        c::str(text), v, handler,
                                                        c::enum_in<tokotachi::MessageType>(type, 4)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

// ---- Variable ----

TOKOTACHI_C_API tokotachi_variable* tokotachi_variable_new(tokotachi_object* owner, tokotachi_variable_type type)
{
    try
    {
        Object* o = c::arg_as<Object>(owner, __func__);
        switch (c::enum_in<int>(type, 5))
        {
        case TOKOTACHI_VARIABLE_TYPE_BOOL:
            return c::handle(new tokotachi::Variable<bool>(o));
        case TOKOTACHI_VARIABLE_TYPE_INT:
            return c::handle(new tokotachi::Variable<int>(o));
        case TOKOTACHI_VARIABLE_TYPE_FLOAT:
            return c::handle(new tokotachi::Variable<float>(o));
        case TOKOTACHI_VARIABLE_TYPE_DOUBLE:
            return c::handle(new tokotachi::Variable<double>(o));
        default:
            return c::handle(new tokotachi::Variable<std::string>(o));
        }
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_variable_type tokotachi_variable_get_type(tokotachi_variable* variable)
{
    try
    {
        Object* o = c::self_as<Object>(variable, __func__);
        if (dynamic_cast<tokotachi::Variable<bool>*>(o))
        {
            return TOKOTACHI_VARIABLE_TYPE_BOOL;
        }
        if (dynamic_cast<tokotachi::Variable<int>*>(o))
        {
            return TOKOTACHI_VARIABLE_TYPE_INT;
        }
        if (dynamic_cast<tokotachi::Variable<float>*>(o))
        {
            return TOKOTACHI_VARIABLE_TYPE_FLOAT;
        }
        if (dynamic_cast<tokotachi::Variable<double>*>(o))
        {
            return TOKOTACHI_VARIABLE_TYPE_DOUBLE;
        }
        if (dynamic_cast<tokotachi::Variable<std::string>*>(o))
        {
            return TOKOTACHI_VARIABLE_TYPE_STRING;
        }
        throw std::invalid_argument(c::describe(o) + " is not a variable");
    }
    TOKOTACHI_C_CATCH(TOKOTACHI_VARIABLE_TYPE_BOOL)
}

#define TOKOTACHI_C_VARIABLE_ACCESS(suffix, ctype, cpptype, in, out, err)                                             \
    TOKOTACHI_C_API ctype tokotachi_variable_get_##suffix(tokotachi_variable* variable)                               \
    {                                                                                                                  \
        try                                                                                                            \
        {                                                                                                              \
            return out(variable_as<cpptype>(variable, __func__)->value.get());                                         \
        }                                                                                                              \
        TOKOTACHI_C_CATCH(err)                                                                                         \
    }                                                                                                                  \
    TOKOTACHI_C_API void tokotachi_variable_set_##suffix(tokotachi_variable* variable, in value)                       \
    {                                                                                                                  \
        try                                                                                                            \
        {                                                                                                              \
            variable_as<cpptype>(variable, __func__)->value = cpptype(value);                                          \
        }                                                                                                              \
        TOKOTACHI_C_CATCH()                                                                                            \
    }

#define TOKOTACHI_C_SAME(x) (x)
TOKOTACHI_C_VARIABLE_ACCESS(bool, bool, bool, bool, TOKOTACHI_C_SAME, false)
TOKOTACHI_C_VARIABLE_ACCESS(int, int, int, int, TOKOTACHI_C_SAME, 0)
TOKOTACHI_C_VARIABLE_ACCESS(float, float, float, float, TOKOTACHI_C_SAME, 0)
TOKOTACHI_C_VARIABLE_ACCESS(double, double, double, double, TOKOTACHI_C_SAME, 0)

TOKOTACHI_C_API char* tokotachi_variable_get_string(tokotachi_variable* variable)
{
    try
    {
        return c::dup(variable_as<std::string>(variable, __func__)->value.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_variable_set_string(tokotachi_variable* variable, const char* value)
{
    try
    {
        variable_as<std::string>(variable, __func__)->value = c::str(value);
    }
    TOKOTACHI_C_CATCH()
}

namespace
{
/// 型によらず、Variable の on_set・on_change を設定する。
void set_variable_handler(tokotachi_variable* variable, bool on_set, const tokotachi::NotifyHandler& h,
                          const char* function)
{
    Object* o = c::self_as<Object>(variable, function);
#define TOKOTACHI_C_SET_HANDLER(T)                                                                                    \
    if (tokotachi::Variable<T>* v = dynamic_cast<tokotachi::Variable<T>*>(o))                                          \
    {                                                                                                                  \
        if (on_set)                                                                                                    \
        {                                                                                                              \
            v->on_set = h;                                                                                             \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            v->on_change = h;                                                                                          \
        }                                                                                                              \
        return;                                                                                                        \
    }
    TOKOTACHI_C_SET_HANDLER(bool)
    TOKOTACHI_C_SET_HANDLER(int)
    TOKOTACHI_C_SET_HANDLER(float)
    TOKOTACHI_C_SET_HANDLER(double)
    TOKOTACHI_C_SET_HANDLER(std::string)
#undef TOKOTACHI_C_SET_HANDLER
    throw std::invalid_argument(c::describe(o) + " is not a variable");
}
}  // namespace

TOKOTACHI_C_API void tokotachi_variable_set_on_set(tokotachi_variable* variable, tokotachi_notify_fn fn,
                                                   void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler h = c::notify_handler(fn, user_data, destroy);
        set_variable_handler(variable, true, h, __func__);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_variable_set_on_change(tokotachi_variable* variable, tokotachi_notify_fn fn,
                                                      void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler h = c::notify_handler(fn, user_data, destroy);
        set_variable_handler(variable, false, h, __func__);
    }
    TOKOTACHI_C_CATCH()
}

// ---- Task ----

TOKOTACHI_C_API tokotachi_task* tokotachi_task_new(tokotachi_object* owner)
{
    try
    {
        return c::handle(new tokotachi::Task<void>(c::arg_as<Object>(owner, __func__)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API bool tokotachi_task_start(tokotachi_task* task, tokotachi_task_fn work, void* user_data,
                                          tokotachi_destroy_fn destroy)
{
    try
    {
        std::shared_ptr<c::UserData> data = std::make_shared<c::UserData>(user_data, destroy);
        tokotachi::Task<void>* t = c::self_as<tokotachi::Task<void> >(task, __func__);
        if (!work)
        {
            throw std::invalid_argument("the work is NULL");
        }
        return t->start(
            [work, data](tokotachi::TaskContext& context)
            {
                tokotachi_task_context ctx = {&context, false, std::string()};
                work(&ctx, data->data);
                if (ctx.failed)
                {
                    throw std::runtime_error(ctx.message);
                }
            });
    }
    TOKOTACHI_C_CATCH(false)
}

TOKOTACHI_C_API bool tokotachi_task_context_cancel_requested(tokotachi_task_context* context)
{
    return context && context->context->cancel_requested();
}

TOKOTACHI_C_API void tokotachi_task_context_report_progress(tokotachi_task_context* context, double value)
{
    if (context)
    {
        context->context->report_progress(value);
    }
}

TOKOTACHI_C_API void tokotachi_task_context_fail(tokotachi_task_context* context, const char* message)
{
    if (context)
    {
        context->failed = true;
        context->message = c::str(message);
    }
}

// ---- 独自のモデル ----

TOKOTACHI_C_API tokotachi_custom_model* tokotachi_custom_model_new(tokotachi_object* owner,
                                                                   const tokotachi_model_callbacks* callbacks,
                                                                   void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        c::UserData guard(user_data, destroy);  // 失敗したときに解放する
        if (!callbacks || !callbacks->row_count || !callbacks->text)
        {
            throw std::invalid_argument("row_count and text are required");
        }
        Object* o = c::arg_as<Object>(owner, __func__);
        c::CallbackModel* model = new c::CallbackModel(o, *callbacks, user_data, destroy);
        guard.release();
        return c::handle(model);
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_custom_model_notify_rows_inserted(tokotachi_custom_model* model,
                                                                 tokotachi_model_index parent, int first, int count)
{
    try
    {
        c::self_as<c::CallbackModel>(model, __func__)->notify_rows_inserted(c::from_c(parent), first, count);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_custom_model_notify_rows_removing(tokotachi_custom_model* model,
                                                                 tokotachi_model_index parent, int first, int count)
{
    try
    {
        c::self_as<c::CallbackModel>(model, __func__)->notify_rows_removing(c::from_c(parent), first, count);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_custom_model_notify_data_changed(tokotachi_custom_model* model,
                                                                tokotachi_model_index index)
{
    try
    {
        c::self_as<c::CallbackModel>(model, __func__)->notify_data_changed(c::from_c(index));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_custom_model_notify_layout_changed(tokotachi_custom_model* model)
{
    try
    {
        c::self_as<c::CallbackModel>(model, __func__)->notify_layout_changed();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_custom_model_notify_reset(tokotachi_custom_model* model)
{
    try
    {
        c::self_as<c::CallbackModel>(model, __func__)->notify_reset();
    }
    TOKOTACHI_C_CATCH()
}

}  // extern "C"
