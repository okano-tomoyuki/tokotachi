// C API の実装の共通部分(docs/adr/0065)。C と C++ の値の変換・ハンドル・失敗の報告・ハンドラ・名前からプロパティへの表。
#ifndef TOKOTACHI_C_COMMON_HPP
#define TOKOTACHI_C_COMMON_HPP

#define TOKOTACHI_C_BUILD
#include <tokotachi/tokotachi.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <vector>

#include <tokotachi/tokotachi.hpp>

// 値のハンドルの中身(C からは中を見せない)。
struct tokotachi_image
{
    tokotachi::Image image;
};

struct tokotachi_bitmap
{
    tokotachi::Bitmap bitmap;
};

struct tokotachi_path
{
    tokotachi::Path path;
};

struct tokotachi_text_layout
{
    tokotachi::TextLayout layout;
};

/// C の関数の中で C++ の例外を受け止め、失敗として報告して既定の値を返す。
#define TOKOTACHI_C_CATCH(...)                                                                                        \
    catch (const std::exception& e)                                                                                    \
    {                                                                                                                  \
        ::tokotachi::c::report(__func__, e.what());                                                                    \
        return __VA_ARGS__;                                                                                            \
    }                                                                                                                  \
    catch (...)                                                                                                        \
    {                                                                                                                  \
        ::tokotachi::c::report(__func__, "unknown error");                                                             \
        return __VA_ARGS__;                                                                                            \
    }

namespace tokotachi
{
namespace c
{

// ---- 失敗 ----

/// 失敗を記録し、失敗のハンドラを呼ぶ。
void report(const char* function, const std::string& message);

// ---- ハンドル ----

inline tokotachi_object* handle(Object* o) { return reinterpret_cast<tokotachi_object*>(o); }

inline Object* object_of(tokotachi_object* h) { return reinterpret_cast<Object*>(h); }

std::string describe(Object* o);

/// self として渡されたハンドルを T にする(NULL・型の違いは例外)。
template <typename T>
T* self_as(tokotachi_object* h, const char* function)
{
    Object* o = object_of(h);
    if (!o)
    {
        throw std::invalid_argument(std::string("the object is NULL (") + function + ")");
    }
    T* t = dynamic_cast<T*>(o);
    if (!t)
    {
        throw std::invalid_argument(describe(o) + " is not the expected class (" + function + ")");
    }
    return t;
}

/// 引数のハンドルを T にする(NULL は NULL のまま。型の違いは例外)。
template <typename T>
T* arg_as(tokotachi_object* h, const char* function)
{
    return h ? self_as<T>(h, function) : nullptr;
}

// ---- 文字列 ----

inline std::string str(const char* s) { return s ? std::string(s) : std::string(); }

/// malloc で写す(tokotachi_free で解放する)。
char* dup(const std::string& s);
char** dup_strv(const std::vector<std::string>& v);
std::vector<std::string> strv(const char* const* v, std::size_t count);
std::vector<FileFilter> file_filters_in(const char* const* v, std::size_t count);
char** file_filters_out(const std::vector<FileFilter>& filters);

// ---- 並び ----

template <typename T>
std::size_t fill_objects(const std::vector<T*>& v, tokotachi_object** out, std::size_t capacity)
{
    for (std::size_t i = 0; i < v.size() && i < capacity; ++i)
    {
        out[i] = handle(v[i]);
    }
    return v.size();
}

std::size_t fill_floats(const std::vector<float>& v, float* out, std::size_t capacity);
std::size_t fill_indexes(const std::vector<ModelIndex>& v, tokotachi_model_index* out, std::size_t capacity);
std::vector<float> floats(const float* v, std::size_t count);

// ---- 列挙 ----

template <typename E>
E enum_in(int value, int count)
{
    if (value < 0 || value >= count)
    {
        throw std::invalid_argument("enum value out of range: " + std::to_string(value));
    }
    return static_cast<E>(value);
}

// ---- 値の型 ----

inline Point from_c(const tokotachi_point& p) { return Point(p.x, p.y); }
inline tokotachi_point to_c(const Point& p) { return tokotachi_point{p.x, p.y}; }
inline Size from_c(const tokotachi_size& s) { return Size(s.width, s.height); }
inline tokotachi_size to_c(const Size& s) { return tokotachi_size{s.width, s.height}; }
inline Rect from_c(const tokotachi_rect& r) { return Rect(r.x, r.y, r.width, r.height); }
inline tokotachi_rect to_c(const Rect& r) { return tokotachi_rect{r.x, r.y, r.width, r.height}; }
inline Color from_c(const tokotachi_color& c) { return Color(c.r, c.g, c.b, c.a); }
inline tokotachi_color to_c(const Color& c) { return tokotachi_color{c.r, c.g, c.b, c.a}; }
inline Margins from_c(const tokotachi_margins& m) { return Margins(m.left, m.top, m.right, m.bottom); }
inline tokotachi_margins to_c(const Margins& m) { return tokotachi_margins{m.left, m.top, m.right, m.bottom}; }
inline Font from_c(const tokotachi_font& f) { return Font(f.size); }
inline tokotachi_font to_c(const Font& f) { return tokotachi_font{f.size}; }
Shortcut from_c(const tokotachi_shortcut& s);
tokotachi_shortcut to_c(const Shortcut& s);
ThemeColor from_c(const tokotachi_theme_color& c);
tokotachi_theme_color to_c(const ThemeColor& c);
ModelIndex from_c(const tokotachi_model_index& i);
tokotachi_model_index to_c(const ModelIndex& i);
Placement from_c(const tokotachi_placement& p);
tokotachi_placement to_c(const Placement& p);
Theme from_c(const tokotachi_theme* t);
tokotachi_theme to_c(const Theme& t);
Paint from_c(const tokotachi_paint* p);
StrokeStyle from_c(const tokotachi_stroke_style* s);

inline Image from_c(const tokotachi_image* i) { return i ? i->image : Image(); }
/// 空の画像なら NULL。
tokotachi_image* to_c_image(const Image& image);
tokotachi_bitmap* to_c_bitmap(Bitmap&& bitmap);

// ---- ハンドラ ----

/// 利用者のデータと解放の関数。最後の写しが無くなるときに destroy(user_data) を呼ぶ。
class UserData
{
public:
    UserData(void* user_data, tokotachi_destroy_fn destroy) : data(user_data), destroy_(destroy) {}
    ~UserData()
    {
        if (destroy_)
        {
            destroy_(data);
        }
    }
    UserData(const UserData&) = delete;
    /// destroy を呼ばないようにする(持ち主を移したとき)。
    void release() { destroy_ = nullptr; }
    UserData& operator=(const UserData&) = delete;

    void* const data;

private:
    tokotachi_destroy_fn destroy_;
};

NotifyHandler notify_handler(tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy);
MouseHandler mouse_handler(tokotachi_mouse_fn fn, void* user_data, tokotachi_destroy_fn destroy);
KeyHandler key_handler(tokotachi_key_fn fn, void* user_data, tokotachi_destroy_fn destroy);
WheelHandler wheel_handler(tokotachi_wheel_fn fn, void* user_data, tokotachi_destroy_fn destroy);
PaintHandler paint_handler(tokotachi_paint_fn fn, void* user_data, tokotachi_destroy_fn destroy);
CloseHandler close_handler(tokotachi_close_fn fn, void* user_data, tokotachi_destroy_fn destroy);

// ---- モデル ----

/// 関数で中身を答えるモデル(tokotachi_custom_model_new)。
class CallbackModel : public ItemModel
{
public:
    CallbackModel(Object* owner, const tokotachi_model_callbacks& callbacks, void* user_data,
                  tokotachi_destroy_fn destroy);
    ~CallbackModel();

    int row_count(const ModelIndex& parent = ModelIndex()) const override;
    ModelIndex index(int row, const ModelIndex& parent = ModelIndex()) const override;
    ModelIndex parent_of(const ModelIndex& index) const override;
    std::string text(const ModelIndex& index, int column) const override;
    Image icon(const ModelIndex& index) const override;
    bool enabled(const ModelIndex& index) const override;
    bool checkable(const ModelIndex& index) const override;
    bool checked(const ModelIndex& index) const override;
    void set_checked(const ModelIndex& index, bool checked) override;
    void sort(int column, SortOrder order) override;
    ModelIndex locate(const ModelIndex& index) const override;

    using ItemModel::make_index;
    using ItemModel::notify_data_changed;
    using ItemModel::notify_layout_changed;
    using ItemModel::notify_reset;
    using ItemModel::notify_rows_inserted;
    using ItemModel::notify_rows_removing;

private:
    tokotachi_model_callbacks callbacks_;
    UserData user_data_;
};

// ---- Variable ----

bool is_variable(Object* o);

// ---- 名前からプロパティへの表(tokotachi_bind) ----

struct PropertyEntry
{
    const char* name;
    const std::type_info* type;
    bool read_only;
    /// o がこのプロパティを持つクラスなら、プロパティ(Property<T>・ReadOnlyProperty<T>)のアドレス。違えば nullptr。
    void* (*find)(Object* o);
    /// target(Property<T>)と source を結ぶ。
    void (*bind)(void* target, void* source, bool source_read_only);
    void (*unbind_all)(void* property, bool read_only);
};

void add_entry(const PropertyEntry& entry);

template <typename C, typename T, Property<T> C::*M>
void* find_property(Object* o)
{
    C* c = dynamic_cast<C*>(o);
    return c ? &(c->*M) : nullptr;
}

template <typename C, typename T, ReadOnlyProperty<T> C::*M>
void* find_read_only(Object* o)
{
    C* c = dynamic_cast<C*>(o);
    return c ? &(c->*M) : nullptr;
}

template <typename T>
void bind_typed(void* target, void* source, bool source_read_only)
{
    Property<T>& t = *static_cast<Property<T>*>(target);
    if (source_read_only)
    {
        tokotachi::bind(t, *static_cast<ReadOnlyProperty<T>*>(source));
    }
    else
    {
        tokotachi::bind(t, *static_cast<Property<T>*>(source));
    }
}

template <typename T>
void unbind_all_typed(void* property, bool read_only)
{
    if (read_only)
    {
        tokotachi::unbind(*static_cast<ReadOnlyProperty<T>*>(property));
    }
    else
    {
        tokotachi::unbind(*static_cast<Property<T>*>(property));
    }
}

/// 表に加える(メンバへのポインタは型ごとに 1 つの関数を作るため、テンプレートの引数で受ける)。
template <typename C, typename T, Property<T> C::*M>
void add_property_entry(const char* name)
{
    PropertyEntry e = {name, &typeid(T), false, &find_property<C, T, M>, &bind_typed<T>, &unbind_all_typed<T>};
    add_entry(e);
}

template <typename C, typename T, ReadOnlyProperty<T> C::*M>
void add_read_only_entry(const char* name)
{
    PropertyEntry e = {name, &typeid(T), true, &find_read_only<C, T, M>, &bind_typed<T>, &unbind_all_typed<T>};
    add_entry(e);
}

/// generated.cpp の TOKOTACHI_C_ADD_PROPERTY("text", &Label::text) から、上のどちらかを呼ぶ。
#define TOKOTACHI_C_ADD_PROPERTY(name, member)                                                                        \
    ::tokotachi::c::add_member<decltype(member), member>(name)

template <typename P, P M>
struct MemberTraits;

template <typename C, typename T, Property<T> C::*M>
struct MemberTraits<Property<T> C::*, M>
{
    static void add(const char* name) { add_property_entry<C, T, M>(name); }
};

template <typename C, typename T, ReadOnlyProperty<T> C::*M>
struct MemberTraits<ReadOnlyProperty<T> C::*, M>
{
    static void add(const char* name) { add_read_only_entry<C, T, M>(name); }
};

template <typename P, P M>
void add_member(const char* name)
{
    MemberTraits<P, M>::add(name);
}

/// generated.cpp が作る(api.py のプロパティを表に加える)。
void register_generated_properties();

/// 名前からプロパティを探す(無ければ nullptr)。
const PropertyEntry* find_entry(Object* o, const char* name, void** property);

}  // namespace c
}  // namespace tokotachi

#endif
