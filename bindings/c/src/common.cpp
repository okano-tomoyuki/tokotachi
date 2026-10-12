// C API の実装の共通部分(common.hpp)。
#include "common.hpp"

#include <cstdlib>
#include <cstring>
#include <mutex>

namespace tokotachi
{
namespace c
{

// ---- 失敗 ----

namespace
{
thread_local std::string last_error;
thread_local bool has_error = false;

std::mutex error_handler_mutex;
tokotachi_error_fn error_handler = nullptr;
void* error_handler_data = nullptr;
}  // namespace

void report(const char* function, const std::string& message)
{
    last_error = std::string(function) + ": " + message;
    has_error = true;
    tokotachi_error_fn handler;
    void* data;
    {
        std::lock_guard<std::mutex> lock(error_handler_mutex);
        handler = error_handler;
        data = error_handler_data;
    }
    if (handler)
    {
        handler(last_error.c_str(), data);
    }
}

std::string describe(Object* o) { return std::string(tokotachi_object_class_name(handle(o))); }

// ---- 文字列 ----

char* dup(const std::string& s)
{
    char* p = static_cast<char*>(std::malloc(s.size() + 1));
    if (!p)
    {
        throw std::bad_alloc();
    }
    std::memcpy(p, s.c_str(), s.size() + 1);
    return p;
}

char** dup_strv(const std::vector<std::string>& v)
{
    char** p = static_cast<char**>(std::calloc(v.size() + 1, sizeof(char*)));
    if (!p)
    {
        throw std::bad_alloc();
    }
    try
    {
        for (std::size_t i = 0; i < v.size(); ++i)
        {
            p[i] = dup(v[i]);
        }
    }
    catch (...)
    {
        tokotachi_strv_free(p);
        throw;
    }
    return p;
}

std::vector<std::string> strv(const char* const* v, std::size_t count)
{
    std::vector<std::string> out;
    out.reserve(count);
    for (std::size_t i = 0; i < count; ++i)
    {
        out.push_back(str(v ? v[i] : nullptr));
    }
    return out;
}

std::vector<FileFilter> file_filters_in(const char* const* v, std::size_t count)
{
    if (count % 2 != 0)
    {
        throw std::invalid_argument("filters must be pairs of a name and a pattern");
    }
    std::vector<FileFilter> out;
    for (std::size_t i = 0; i + 1 < count; i += 2)
    {
        out.push_back(FileFilter(str(v[i]), str(v[i + 1])));
    }
    return out;
}

char** file_filters_out(const std::vector<FileFilter>& filters)
{
    std::vector<std::string> v;
    for (std::size_t i = 0; i < filters.size(); ++i)
    {
        v.push_back(filters[i].name);
        v.push_back(filters[i].pattern);
    }
    return dup_strv(v);
}

// ---- 並び ----

std::size_t fill_floats(const std::vector<float>& v, float* out, std::size_t capacity)
{
    for (std::size_t i = 0; i < v.size() && i < capacity; ++i)
    {
        out[i] = v[i];
    }
    return v.size();
}

std::size_t fill_indexes(const std::vector<ModelIndex>& v, tokotachi_model_index* out, std::size_t capacity)
{
    for (std::size_t i = 0; i < v.size() && i < capacity; ++i)
    {
        out[i] = to_c(v[i]);
    }
    return v.size();
}

std::size_t fill_ranges(const std::vector<CellRange>& v, tokotachi_cell_range* out, std::size_t capacity)
{
    for (std::size_t i = 0; i < v.size() && i < capacity; ++i)
    {
        out[i] = to_c(v[i]);
    }
    return v.size();
}

std::vector<ModelIndex> indexes(const tokotachi_model_index* v, std::size_t count)
{
    std::vector<ModelIndex> out;
    for (std::size_t i = 0; v && i < count; ++i)
    {
        out.push_back(from_c(v[i]));
    }
    return out;
}

std::vector<float> floats(const float* v, std::size_t count)
{
    return v ? std::vector<float>(v, v + count) : std::vector<float>();
}

// ---- 値の型 ----

Shortcut from_c(const tokotachi_shortcut& s)
{
    return Shortcut(enum_in<Key>(s.key, static_cast<int>(Key::META) + 1), s.modifiers);
}

tokotachi_shortcut to_c(const Shortcut& s)
{
    return tokotachi_shortcut{static_cast<tokotachi_key>(s.key), s.modifiers};
}

ThemeColor from_c(const tokotachi_theme_color& c)
{
    if (c.is_role)
    {
        return ThemeColor(enum_in<ColorRole>(c.role, COLOR_ROLE_COUNT));
    }
    return ThemeColor(from_c(c.color));
}

tokotachi_theme_color to_c(const ThemeColor& c)
{
    tokotachi_theme_color out;
    out.is_role = c.is_role();
    out.role = static_cast<tokotachi_color_role>(c.role());
    out.color = to_c(c.is_role() ? Color(0, 0, 0, 0) : c.color());
    return out;
}

namespace
{
/// ModelIndex を作れるのは ItemModel だけなので、作るためだけのモデルを 1 つ持つ(破棄しない)。
class IndexMaker : public ItemModel
{
public:
    int row_count(const ModelIndex&) const override { return 0; }
    std::string text(const ModelIndex&, int) const override { return std::string(); }
    ModelIndex make(int row, void* id) const { return make_index(row, id); }
};

const IndexMaker& index_maker()
{
    static IndexMaker* maker = new IndexMaker();
    return *maker;
}
}  // namespace

ModelIndex from_c(const tokotachi_model_index& i)
{
    return i.row >= 0 ? index_maker().make(i.row, i.id) : ModelIndex();
}

tokotachi_model_index to_c(const ModelIndex& i)
{
    return tokotachi_model_index{i.row(), i.id()};
}

CellRange from_c(const tokotachi_cell_range& r)
{
    CellRange out;
    out.first_row = r.first_row;
    out.first_column = r.first_column;
    out.last_row = r.last_row;
    out.last_column = r.last_column;
    return out;
}

ControlState from_c(const tokotachi_control_state& s)
{
    ControlState out;
    out.enabled = s.enabled;
    out.hovered = s.hovered;
    out.pressed = s.pressed;
    out.focused = s.focused;
    out.checked = s.checked;
    return out;
}

ThemeOverride from_c(const tokotachi_theme_override* o)
{
    if (!o)
    {
        throw std::invalid_argument("the theme override is NULL");
    }
    ThemeOverride out;
    for (int i = 0; i < COLOR_ROLE_COUNT; ++i)
    {
        if (o->has[i])
        {
            out.set(static_cast<ColorRole>(i), from_c(o->light[i]), from_c(o->dark[i]));
        }
    }
    out.set_radius(o->radius);
    out.set_field_radius(o->field_radius);
    out.set_field_padding(o->field_padding);
    return out;
}

tokotachi_theme_override to_c(const ThemeOverride& o)
{
    tokotachi_theme_override out;
    for (int i = 0; i < COLOR_ROLE_COUNT; ++i)
    {
        const ColorRole role = static_cast<ColorRole>(i);
        out.has[i] = o.has(role);
        out.light[i] = to_c(o.has(role) ? o.light(role) : Color(0, 0, 0, 0));
        out.dark[i] = to_c(o.has(role) ? o.dark(role) : Color(0, 0, 0, 0));
    }
    out.radius = o.radius();
    out.field_radius = o.field_radius();
    out.field_padding = o.field_padding();
    return out;
}

Placement from_c(const tokotachi_placement& p)
{
    Placement out;
    out.x = p.x;
    out.y = p.y;
    out.relative_x = p.relative_x;
    out.relative_y = p.relative_y;
    out.width = p.width;
    out.height = p.height;
    out.relative_width = p.relative_width;
    out.relative_height = p.relative_height;
    out.anchor = enum_in<Anchor>(p.anchor, static_cast<int>(Anchor::BOTTOM_RIGHT) + 1);
    return out;
}

tokotachi_placement to_c(const Placement& p)
{
    tokotachi_placement out;
    out.x = p.x;
    out.y = p.y;
    out.relative_x = p.relative_x;
    out.relative_y = p.relative_y;
    out.width = p.width;
    out.height = p.height;
    out.relative_width = p.relative_width;
    out.relative_height = p.relative_height;
    out.anchor = static_cast<tokotachi_anchor>(p.anchor);
    return out;
}

Theme from_c(const tokotachi_theme* t)
{
    if (!t)
    {
        throw std::invalid_argument("the theme is NULL");
    }
    Theme out;
    for (int i = 0; i < COLOR_ROLE_COUNT; ++i)
    {
        out[static_cast<ColorRole>(i)] = from_c(t->colors[i]);
    }
    out.dark_colors = t->dark_colors;
    out.radius = t->radius;
    out.field_radius = t->field_radius;
    out.field_padding = t->field_padding;
    return out;
}

tokotachi_theme to_c(const Theme& t)
{
    tokotachi_theme out;
    for (int i = 0; i < COLOR_ROLE_COUNT; ++i)
    {
        out.colors[i] = to_c(t[static_cast<ColorRole>(i)]);
    }
    out.dark_colors = t.dark_colors;
    out.radius = t.radius;
    out.field_radius = t.field_radius;
    out.field_padding = t.field_padding;
    return out;
}

Paint from_c(const tokotachi_paint* p)
{
    if (!p)
    {
        throw std::invalid_argument("the paint is NULL");
    }
    std::vector<GradientStop> stops;
    for (std::size_t i = 0; i < p->stop_count; ++i)
    {
        stops.push_back(GradientStop(p->stops[i].offset, from_c(p->stops[i].color)));
    }
    switch (enum_in<Paint::Type>(p->type, 3))
    {
    case Paint::Type::LINEAR:
        return Paint::linear(from_c(p->p0), from_c(p->p1), stops);
    case Paint::Type::RADIAL:
        return Paint::radial(from_c(p->p0), p->radius, from_c(p->p1), stops);
    default:
        return Paint(from_c(p->color));
    }
}

StrokeStyle from_c(const tokotachi_stroke_style* s)
{
    if (!s)
    {
        throw std::invalid_argument("the stroke style is NULL");
    }
    StrokeStyle out(s->width, enum_in<LineCap>(s->cap, 3), enum_in<LineJoin>(s->join, 3));
    out.dashes = floats(s->dashes, s->dash_count);
    out.dash_offset = s->dash_offset;
    return out;
}

tokotachi_image* to_c_image(const Image& image)
{
    if (image.is_null())
    {
        return nullptr;
    }
    tokotachi_image* p = new tokotachi_image;
    p->image = image;
    return p;
}

tokotachi_bitmap* to_c_bitmap(Bitmap&& bitmap)
{
    if (bitmap.is_null())
    {
        return nullptr;
    }
    tokotachi_bitmap* p = new tokotachi_bitmap;
    p->bitmap = std::move(bitmap);
    return p;
}

// ---- ハンドラ ----

namespace
{
/// fn が NULL ならハンドラを外す(destroy はすぐに呼ぶ)。
template <typename Fn>
std::shared_ptr<UserData> hold(Fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    std::shared_ptr<UserData> data = std::make_shared<UserData>(user_data, destroy);
    if (!fn)
    {
        data.reset();
    }
    return data;
}

tokotachi_mouse_event to_c(const MouseEvent& e)
{
    tokotachi_mouse_event out;
    out.pos = c::to_c(e.pos);
    out.button = static_cast<tokotachi_mouse_button>(e.button);
    out.modifiers = e.modifiers;
    out.clicks = e.clicks;
    out.accepted = e.accepted;
    return out;
}

tokotachi_key_event to_c(const KeyEvent& e)
{
    tokotachi_key_event out;
    out.key = static_cast<tokotachi_key>(e.key);
    out.modifiers = e.modifiers;
    out.repeat = e.repeat;
    out.native_key = e.native_key;
    out.accepted = e.accepted;
    return out;
}

tokotachi_wheel_event to_c(const WheelEvent& e)
{
    tokotachi_wheel_event out;
    out.pos = c::to_c(e.pos);
    out.delta = c::to_c(e.delta);
    out.modifiers = e.modifiers;
    out.accepted = e.accepted;
    return out;
}
}  // namespace

NotifyHandler notify_handler(tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    std::shared_ptr<UserData> data = hold(fn, user_data, destroy);
    if (!data)
    {
        return NotifyHandler();
    }
    return [fn, data](Object* sender) { fn(handle(sender), data->data); };
}

MouseHandler mouse_handler(tokotachi_mouse_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    std::shared_ptr<UserData> data = hold(fn, user_data, destroy);
    if (!data)
    {
        return MouseHandler();
    }
    return [fn, data](Object* sender, MouseEvent& e)
    {
        tokotachi_mouse_event ce = to_c(e);
        fn(handle(sender), &ce, data->data);
        e.accepted = ce.accepted;
    };
}

KeyHandler key_handler(tokotachi_key_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    std::shared_ptr<UserData> data = hold(fn, user_data, destroy);
    if (!data)
    {
        return KeyHandler();
    }
    return [fn, data](Object* sender, KeyEvent& e)
    {
        tokotachi_key_event ce = to_c(e);
        fn(handle(sender), &ce, data->data);
        e.accepted = ce.accepted;
    };
}

WheelHandler wheel_handler(tokotachi_wheel_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    std::shared_ptr<UserData> data = hold(fn, user_data, destroy);
    if (!data)
    {
        return WheelHandler();
    }
    return [fn, data](Object* sender, WheelEvent& e)
    {
        tokotachi_wheel_event ce = to_c(e);
        fn(handle(sender), &ce, data->data);
        e.accepted = ce.accepted;
    };
}

PaintHandler paint_handler(tokotachi_paint_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    std::shared_ptr<UserData> data = hold(fn, user_data, destroy);
    if (!data)
    {
        return PaintHandler();
    }
    return [fn, data](Object* sender, Canvas& canvas)
    { fn(handle(sender), reinterpret_cast<tokotachi_canvas*>(&canvas), data->data); };
}

CloseHandler close_handler(tokotachi_close_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    std::shared_ptr<UserData> data = hold(fn, user_data, destroy);
    if (!data)
    {
        return CloseHandler();
    }
    return [fn, data](Object* sender, bool& can_close)
    {
        bool v = can_close;
        fn(handle(sender), &v, data->data);
        can_close = v;
    };
}

DragHandler drag_handler(tokotachi_drag_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    std::shared_ptr<UserData> data = hold(fn, user_data, destroy);
    if (!data)
    {
        return DragHandler();
    }
    return [fn, data](Object* sender, DragEvent& e)
    {
        tokotachi_drag_event ce;
        ce.pos = c::to_c(e.pos);
        ce.data = c::to_c(e.data);
        ce.modifiers = e.modifiers;
        ce.action = static_cast<tokotachi_drop_action>(e.action);
        ce.accepted = e.accepted;
        fn(handle(sender), &ce, data->data);
        e.action = enum_in<DropAction>(ce.action, 3);
        e.accepted = ce.accepted;
    };
}

DragFinishHandler drag_finish_handler(tokotachi_drag_finish_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    std::shared_ptr<UserData> data = hold(fn, user_data, destroy);
    if (!data)
    {
        return DragFinishHandler();
    }
    return [fn, data](Object* sender, DropAction action)
    { fn(handle(sender), static_cast<tokotachi_drop_action>(action), data->data); };
}

// ---- モデル ----

CallbackModel::CallbackModel(Object* owner, const tokotachi_model_callbacks& callbacks, void* user_data,
                             tokotachi_destroy_fn destroy)
    : ItemModel(owner), callbacks_(callbacks), user_data_(user_data, destroy)
{
}

CallbackModel::~CallbackModel()
{
    // この後で利用者のデータを解放するので、もう関数を呼ばない。
    callbacks_ = tokotachi_model_callbacks();
}

int CallbackModel::row_count(const ModelIndex& parent) const
{
    if (!callbacks_.row_count)
    {
        return 0;
    }
    const tokotachi_model_index p = to_c(parent);
    return callbacks_.row_count(&p, user_data_.data);
}

ModelIndex CallbackModel::index(int row, const ModelIndex& parent) const
{
    if (!callbacks_.index_id)
    {
        return ItemModel::index(row, parent);
    }
    if (row < 0 || row >= row_count(parent))
    {
        return ModelIndex();
    }
    const tokotachi_model_index p = to_c(parent);
    return make_index(row, callbacks_.index_id(row, &p, user_data_.data));
}

ModelIndex CallbackModel::parent_of(const ModelIndex& index) const
{
    if (!callbacks_.parent_of)
    {
        return ItemModel::parent_of(index);
    }
    const tokotachi_model_index i = to_c(index);
    tokotachi_model_index out = {-1, nullptr};
    return callbacks_.parent_of(&i, &out, user_data_.data) ? from_c(out) : ModelIndex();
}

std::string CallbackModel::text(const ModelIndex& index, int column) const
{
    if (!callbacks_.text)
    {
        return std::string();
    }
    const tokotachi_model_index i = to_c(index);
    return str(callbacks_.text(&i, column, user_data_.data));
}

Image CallbackModel::icon(const ModelIndex& index) const
{
    if (!callbacks_.icon)
    {
        return ItemModel::icon(index);
    }
    const tokotachi_model_index i = to_c(index);
    return from_c(callbacks_.icon(&i, user_data_.data));
}

bool CallbackModel::enabled(const ModelIndex& index) const
{
    if (!callbacks_.enabled)
    {
        return ItemModel::enabled(index);
    }
    const tokotachi_model_index i = to_c(index);
    return callbacks_.enabled(&i, user_data_.data);
}

bool CallbackModel::checkable(const ModelIndex& index) const
{
    if (!callbacks_.checkable)
    {
        return ItemModel::checkable(index);
    }
    const tokotachi_model_index i = to_c(index);
    return callbacks_.checkable(&i, user_data_.data);
}

bool CallbackModel::checked(const ModelIndex& index) const
{
    if (!callbacks_.checked)
    {
        return ItemModel::checked(index);
    }
    const tokotachi_model_index i = to_c(index);
    return callbacks_.checked(&i, user_data_.data);
}

void CallbackModel::set_checked(const ModelIndex& index, bool checked)
{
    if (!callbacks_.set_checked)
    {
        ItemModel::set_checked(index, checked);
        return;
    }
    const tokotachi_model_index i = to_c(index);
    callbacks_.set_checked(&i, checked, user_data_.data);
}

void CallbackModel::sort(int column, SortOrder order)
{
    if (!callbacks_.sort)
    {
        ItemModel::sort(column, order);
        return;
    }
    callbacks_.sort(column, static_cast<tokotachi_sort_order>(order), user_data_.data);
}

ModelIndex CallbackModel::locate(const ModelIndex& index) const
{
    if (!callbacks_.locate)
    {
        return ItemModel::locate(index);
    }
    const tokotachi_model_index i = to_c(index);
    tokotachi_model_index out = {-1, nullptr};
    return callbacks_.locate(&i, &out, user_data_.data) ? from_c(out) : ModelIndex();
}

ThemeColor CallbackModel::text_color(const ModelIndex& index, int column) const
{
    const tokotachi_model_index i = to_c(index);
    tokotachi_theme_color out = to_c(ThemeColor());
    if (callbacks_.text_color && callbacks_.text_color(&i, column, &out, user_data_.data))
    {
        return from_c(out);
    }
    return ItemModel::text_color(index, column);
}

Font CallbackModel::font(const ModelIndex& index, int column) const
{
    const tokotachi_model_index i = to_c(index);
    tokotachi_font out = to_c(Font());
    if (callbacks_.font && callbacks_.font(&i, column, &out, user_data_.data))
    {
        return from_c(out);
    }
    return ItemModel::font(index, column);
}

bool CallbackModel::editable(const ModelIndex& index, int column) const
{
    if (!callbacks_.editable)
    {
        return ItemModel::editable(index, column);
    }
    const tokotachi_model_index i = to_c(index);
    return callbacks_.editable(&i, column, user_data_.data);
}

void CallbackModel::set_text(const ModelIndex& index, int column, const std::string& text)
{
    if (!callbacks_.set_text)
    {
        ItemModel::set_text(index, column, text);
        return;
    }
    const tokotachi_model_index i = to_c(index);
    callbacks_.set_text(&i, column, text.c_str(), user_data_.data);
}

namespace
{
std::vector<tokotachi_model_index> to_c(const std::vector<ModelIndex>& v)
{
    std::vector<tokotachi_model_index> out;
    for (std::size_t i = 0; i < v.size(); ++i)
    {
        out.push_back(c::to_c(v[i]));
    }
    return out;
}
}  // namespace

bool CallbackModel::can_move_rows(const std::vector<ModelIndex>& indexes, const ModelIndex& parent, int row) const
{
    if (!callbacks_.can_move_rows)
    {
        return ItemModel::can_move_rows(indexes, parent, row);
    }
    const std::vector<tokotachi_model_index> v = to_c(indexes);
    const tokotachi_model_index p = c::to_c(parent);
    return callbacks_.can_move_rows(v.empty() ? nullptr : &v[0], v.size(), &p, row, user_data_.data);
}

bool CallbackModel::move_rows(const std::vector<ModelIndex>& indexes, const ModelIndex& parent, int row)
{
    if (!callbacks_.move_rows)
    {
        return ItemModel::move_rows(indexes, parent, row);
    }
    const std::vector<tokotachi_model_index> v = to_c(indexes);
    const tokotachi_model_index p = c::to_c(parent);
    return callbacks_.move_rows(v.empty() ? nullptr : &v[0], v.size(), &p, row, user_data_.data);
}

bool CallbackModel::can_drop(const DragData& data, const ModelIndex& parent, int row) const
{
    if (!callbacks_.can_drop)
    {
        return ItemModel::can_drop(data, parent, row);
    }
    const tokotachi_model_index p = c::to_c(parent);
    return callbacks_.can_drop(c::to_c(data), &p, row, user_data_.data);
}

bool CallbackModel::drop_data(const DragData& data, DropAction action, const ModelIndex& parent, int row)
{
    if (!callbacks_.drop_data)
    {
        return ItemModel::drop_data(data, action, parent, row);
    }
    const tokotachi_model_index p = c::to_c(parent);
    return callbacks_.drop_data(c::to_c(data), static_cast<tokotachi_drop_action>(action), &p, row, user_data_.data);
}

// ---- 名前からプロパティへの表 ----

namespace
{
std::vector<PropertyEntry>& entries()
{
    static std::vector<PropertyEntry>* list = new std::vector<PropertyEntry>();
    return *list;
}

void register_variables();
}  // namespace

void add_entry(const PropertyEntry& entry) { entries().push_back(entry); }

const PropertyEntry* find_entry(Object* o, const char* name, void** property)
{
    // 表はメインスレッドで最初に使うときに作る(Object はメインスレッドだけで扱う)。
    static bool registered = false;
    if (!registered)
    {
        registered = true;
        register_generated_properties();
        register_variables();
    }
    if (!o || !name)
    {
        return nullptr;
    }
    const std::vector<PropertyEntry>& list = entries();
    for (std::size_t i = 0; i < list.size(); ++i)
    {
        if (std::strcmp(list[i].name, name) == 0)
        {
            if (void* p = list[i].find(o))
            {
                *property = p;
                return &list[i];
            }
        }
    }
    return nullptr;
}

// ---- Variable ----

bool is_variable(Object* o)
{
    return dynamic_cast<Variable<bool>*>(o) || dynamic_cast<Variable<int>*>(o) || dynamic_cast<Variable<float>*>(o) ||
           dynamic_cast<Variable<double>*>(o) || dynamic_cast<Variable<std::string>*>(o);
}

namespace
{
void register_variables()
{
    TOKOTACHI_C_ADD_PROPERTY("value", &Variable<bool>::value);
    TOKOTACHI_C_ADD_PROPERTY("value", &Variable<int>::value);
    TOKOTACHI_C_ADD_PROPERTY("value", &Variable<float>::value);
    TOKOTACHI_C_ADD_PROPERTY("value", &Variable<double>::value);
    TOKOTACHI_C_ADD_PROPERTY("value", &Variable<std::string>::value);
}
}  // namespace

}  // namespace c
}  // namespace tokotachi

// ---- 失敗(C の関数) ----

extern "C" {

TOKOTACHI_C_API const char* tokotachi_last_error(void)
{
    return tokotachi::c::has_error ? tokotachi::c::last_error.c_str() : nullptr;
}

TOKOTACHI_C_API void tokotachi_clear_error(void)
{
    tokotachi::c::has_error = false;
    tokotachi::c::last_error.clear();
}

TOKOTACHI_C_API void tokotachi_set_error_handler(tokotachi_error_fn handler, void* user_data)
{
    std::lock_guard<std::mutex> lock(tokotachi::c::error_handler_mutex);
    tokotachi::c::error_handler = handler;
    tokotachi::c::error_handler_data = user_data;
}

}  // extern "C"
