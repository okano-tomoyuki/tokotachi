// api.py から generate.py で作ったファイル。手で書き換えない。
#include "common.hpp"

namespace c = tokotachi::c;

extern "C" {

TOKOTACHI_C_API tokotachi_object* tokotachi_object_get_owner(tokotachi_object* self)
{
    try
    {
        return c::handle(c::self_as<tokotachi::Object>(self, __func__)->owner.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_application* tokotachi_application_new(int argc, char** argv)
{
    try
    {
        return c::handle(new tokotachi::Application(argc, argv));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_color_scheme tokotachi_application_get_color_scheme(tokotachi_application* self)
{
    try
    {
        return static_cast<tokotachi_color_scheme>(c::self_as<tokotachi::Application>(self, __func__)->color_scheme.get());
    }
    TOKOTACHI_C_CATCH(static_cast<tokotachi_color_scheme>(0))
}

TOKOTACHI_C_API void tokotachi_application_set_color_scheme(tokotachi_application* self, tokotachi_color_scheme value)
{
    try
    {
        c::self_as<tokotachi::Application>(self, __func__)->color_scheme = c::enum_in<tokotachi::ColorScheme>(value, 3);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_application_get_light_theme(tokotachi_application* self, tokotachi_theme* out)
{
    try
    {
        *out = c::to_c(c::self_as<tokotachi::Application>(self, __func__)->light_theme.get());
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_application_set_light_theme(tokotachi_application* self, const tokotachi_theme* value)
{
    try
    {
        c::self_as<tokotachi::Application>(self, __func__)->light_theme = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_application_get_dark_theme(tokotachi_application* self, tokotachi_theme* out)
{
    try
    {
        *out = c::to_c(c::self_as<tokotachi::Application>(self, __func__)->dark_theme.get());
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_application_set_dark_theme(tokotachi_application* self, const tokotachi_theme* value)
{
    try
    {
        c::self_as<tokotachi::Application>(self, __func__)->dark_theme = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_application_get_dark(tokotachi_application* self)
{
    try
    {
        return c::self_as<tokotachi::Application>(self, __func__)->dark.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_application_get_theme(tokotachi_application* self, tokotachi_theme* out)
{
    try
    {
        *out = c::to_c(c::self_as<tokotachi::Application>(self, __func__)->theme.get());
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API char* tokotachi_application_get_language(tokotachi_application* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::Application>(self, __func__)->language.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_application_set_language(tokotachi_application* self, const char* value)
{
    try
    {
        c::self_as<tokotachi::Application>(self, __func__)->language = c::str(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_window* tokotachi_application_get_main_window(tokotachi_application* self)
{
    try
    {
        return c::handle(c::self_as<tokotachi::Application>(self, __func__)->main_window.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_application_set_main_window(tokotachi_application* self, tokotachi_window* value)
{
    try
    {
        c::self_as<tokotachi::Application>(self, __func__)->main_window = c::arg_as<tokotachi::Window>(value, __func__);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_application_get_auto_quit_enabled(tokotachi_application* self)
{
    try
    {
        return c::self_as<tokotachi::Application>(self, __func__)->auto_quit_enabled.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_application_set_auto_quit_enabled(tokotachi_application* self, bool value)
{
    try
    {
        c::self_as<tokotachi::Application>(self, __func__)->auto_quit_enabled = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_application_set_on_theme_change(tokotachi_application* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::Application>(self, __func__)->on_theme_change = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API int tokotachi_application_exec(tokotachi_application* self)
{
    try
    {
        return c::self_as<tokotachi::Application>(self, __func__)->exec();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_application_quit(tokotachi_application* self, int code)
{
    try
    {
        c::self_as<tokotachi::Application>(self, __func__)->quit(code);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_timer* tokotachi_timer_new(tokotachi_object* owner)
{
    try
    {
        return c::handle(new tokotachi::Timer(c::arg_as<tokotachi::Object>(owner, __func__)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API int tokotachi_timer_get_interval(tokotachi_timer* self)
{
    try
    {
        return c::self_as<tokotachi::Timer>(self, __func__)->interval.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_timer_set_interval(tokotachi_timer* self, int value)
{
    try
    {
        c::self_as<tokotachi::Timer>(self, __func__)->interval = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_timer_get_repeat(tokotachi_timer* self)
{
    try
    {
        return c::self_as<tokotachi::Timer>(self, __func__)->repeat.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_timer_set_repeat(tokotachi_timer* self, bool value)
{
    try
    {
        c::self_as<tokotachi::Timer>(self, __func__)->repeat = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_timer_get_enabled(tokotachi_timer* self)
{
    try
    {
        return c::self_as<tokotachi::Timer>(self, __func__)->enabled.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_timer_set_enabled(tokotachi_timer* self, bool value)
{
    try
    {
        c::self_as<tokotachi::Timer>(self, __func__)->enabled = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_timer_set_on_timeout(tokotachi_timer* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::Timer>(self, __func__)->on_timeout = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_widget* tokotachi_widget_new(tokotachi_widget* parent)
{
    try
    {
        return c::handle(new tokotachi::Widget(c::arg_as<tokotachi::Widget>(parent, __func__)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_widget* tokotachi_widget_get_parent(tokotachi_widget* self)
{
    try
    {
        return c::handle(c::self_as<tokotachi::Widget>(self, __func__)->parent.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_widget_set_parent(tokotachi_widget* self, tokotachi_widget* value)
{
    try
    {
        c::self_as<tokotachi::Widget>(self, __func__)->parent = c::arg_as<tokotachi::Widget>(value, __func__);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API size_t tokotachi_widget_get_children(tokotachi_widget* self, tokotachi_widget** out, size_t capacity)
{
    try
    {
        return c::fill_objects(c::self_as<tokotachi::Widget>(self, __func__)->children.get(), out, capacity);
    }
    TOKOTACHI_C_CATCH(0)
}

TOKOTACHI_C_API tokotachi_window* tokotachi_widget_get_window(tokotachi_widget* self)
{
    try
    {
        return c::handle(c::self_as<tokotachi::Widget>(self, __func__)->window.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API float tokotachi_widget_get_x(tokotachi_widget* self)
{
    try
    {
        return c::self_as<tokotachi::Widget>(self, __func__)->x.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_widget_set_x(tokotachi_widget* self, float value)
{
    try
    {
        c::self_as<tokotachi::Widget>(self, __func__)->x = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API float tokotachi_widget_get_y(tokotachi_widget* self)
{
    try
    {
        return c::self_as<tokotachi::Widget>(self, __func__)->y.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_widget_set_y(tokotachi_widget* self, float value)
{
    try
    {
        c::self_as<tokotachi::Widget>(self, __func__)->y = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API float tokotachi_widget_get_width(tokotachi_widget* self)
{
    try
    {
        return c::self_as<tokotachi::Widget>(self, __func__)->width.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_widget_set_width(tokotachi_widget* self, float value)
{
    try
    {
        c::self_as<tokotachi::Widget>(self, __func__)->width = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API float tokotachi_widget_get_height(tokotachi_widget* self)
{
    try
    {
        return c::self_as<tokotachi::Widget>(self, __func__)->height.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_widget_set_height(tokotachi_widget* self, float value)
{
    try
    {
        c::self_as<tokotachi::Widget>(self, __func__)->height = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_rect tokotachi_widget_get_bounds(tokotachi_widget* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::Widget>(self, __func__)->bounds.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_rect())
}

TOKOTACHI_C_API void tokotachi_widget_set_bounds(tokotachi_widget* self, tokotachi_rect value)
{
    try
    {
        c::self_as<tokotachi::Widget>(self, __func__)->bounds = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_rect tokotachi_widget_get_local_rect(tokotachi_widget* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::Widget>(self, __func__)->local_rect.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_rect())
}

TOKOTACHI_C_API tokotachi_size tokotachi_widget_get_preferred_size(tokotachi_widget* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::Widget>(self, __func__)->preferred_size.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_size())
}

TOKOTACHI_C_API tokotachi_size tokotachi_widget_get_minimum_size(tokotachi_widget* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::Widget>(self, __func__)->minimum_size.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_size())
}

TOKOTACHI_C_API void tokotachi_widget_set_minimum_size(tokotachi_widget* self, tokotachi_size value)
{
    try
    {
        c::self_as<tokotachi::Widget>(self, __func__)->minimum_size = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_size tokotachi_widget_get_maximum_size(tokotachi_widget* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::Widget>(self, __func__)->maximum_size.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_size())
}

TOKOTACHI_C_API void tokotachi_widget_set_maximum_size(tokotachi_widget* self, tokotachi_size value)
{
    try
    {
        c::self_as<tokotachi::Widget>(self, __func__)->maximum_size = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_size tokotachi_widget_get_minimum_hint(tokotachi_widget* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::Widget>(self, __func__)->minimum_hint.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_size())
}

TOKOTACHI_C_API tokotachi_layout* tokotachi_widget_get_layout(tokotachi_widget* self)
{
    try
    {
        return c::handle(c::self_as<tokotachi::Widget>(self, __func__)->layout.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API bool tokotachi_widget_get_visible(tokotachi_widget* self)
{
    try
    {
        return c::self_as<tokotachi::Widget>(self, __func__)->visible.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_widget_set_visible(tokotachi_widget* self, bool value)
{
    try
    {
        c::self_as<tokotachi::Widget>(self, __func__)->visible = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_widget_get_showing(tokotachi_widget* self)
{
    try
    {
        return c::self_as<tokotachi::Widget>(self, __func__)->showing.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API bool tokotachi_widget_get_enabled(tokotachi_widget* self)
{
    try
    {
        return c::self_as<tokotachi::Widget>(self, __func__)->enabled.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_widget_set_enabled(tokotachi_widget* self, bool value)
{
    try
    {
        c::self_as<tokotachi::Widget>(self, __func__)->enabled = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_widget_get_effectively_enabled(tokotachi_widget* self)
{
    try
    {
        return c::self_as<tokotachi::Widget>(self, __func__)->effectively_enabled.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API tokotachi_focus_policy tokotachi_widget_get_focus_policy(tokotachi_widget* self)
{
    try
    {
        return static_cast<tokotachi_focus_policy>(c::self_as<tokotachi::Widget>(self, __func__)->focus_policy.get());
    }
    TOKOTACHI_C_CATCH(static_cast<tokotachi_focus_policy>(0))
}

TOKOTACHI_C_API void tokotachi_widget_set_focus_policy(tokotachi_widget* self, tokotachi_focus_policy value)
{
    try
    {
        c::self_as<tokotachi::Widget>(self, __func__)->focus_policy = c::enum_in<tokotachi::FocusPolicy>(value, 4);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_widget_get_focused(tokotachi_widget* self)
{
    try
    {
        return c::self_as<tokotachi::Widget>(self, __func__)->focused.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_widget_set_focused(tokotachi_widget* self, bool value)
{
    try
    {
        c::self_as<tokotachi::Widget>(self, __func__)->focused = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_mouse_cursor tokotachi_widget_get_cursor(tokotachi_widget* self)
{
    try
    {
        return static_cast<tokotachi_mouse_cursor>(c::self_as<tokotachi::Widget>(self, __func__)->cursor.get());
    }
    TOKOTACHI_C_CATCH(static_cast<tokotachi_mouse_cursor>(0))
}

TOKOTACHI_C_API void tokotachi_widget_set_cursor(tokotachi_widget* self, tokotachi_mouse_cursor value)
{
    try
    {
        c::self_as<tokotachi::Widget>(self, __func__)->cursor = c::enum_in<tokotachi::MouseCursor>(value, 7);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_widget_get_accepts_text_input(tokotachi_widget* self)
{
    try
    {
        return c::self_as<tokotachi::Widget>(self, __func__)->accepts_text_input.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_widget_set_accepts_text_input(tokotachi_widget* self, bool value)
{
    try
    {
        c::self_as<tokotachi::Widget>(self, __func__)->accepts_text_input = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API float tokotachi_widget_get_display_scale(tokotachi_widget* self)
{
    try
    {
        return c::self_as<tokotachi::Widget>(self, __func__)->display_scale.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API char* tokotachi_widget_get_tooltip(tokotachi_widget* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::Widget>(self, __func__)->tooltip.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_widget_set_tooltip(tokotachi_widget* self, const char* value)
{
    try
    {
        c::self_as<tokotachi::Widget>(self, __func__)->tooltip = c::str(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API size_t tokotachi_widget_get_actions(tokotachi_widget* self, tokotachi_action** out, size_t capacity)
{
    try
    {
        return c::fill_objects(c::self_as<tokotachi::Widget>(self, __func__)->actions.get(), out, capacity);
    }
    TOKOTACHI_C_CATCH(0)
}

TOKOTACHI_C_API void tokotachi_widget_get_theme_override(tokotachi_widget* self, tokotachi_theme_override* out)
{
    try
    {
        *out = c::to_c(c::self_as<tokotachi::Widget>(self, __func__)->theme_override.get());
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_widget_set_theme_override(tokotachi_widget* self, const tokotachi_theme_override* value)
{
    try
    {
        c::self_as<tokotachi::Widget>(self, __func__)->theme_override = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_widget_get_theme(tokotachi_widget* self, tokotachi_theme* out)
{
    try
    {
        *out = c::to_c(c::self_as<tokotachi::Widget>(self, __func__)->theme.get());
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_widget_get_accepts_drops(tokotachi_widget* self)
{
    try
    {
        return c::self_as<tokotachi::Widget>(self, __func__)->accepts_drops.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_widget_set_accepts_drops(tokotachi_widget* self, bool value)
{
    try
    {
        c::self_as<tokotachi::Widget>(self, __func__)->accepts_drops = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_widget_add_action(tokotachi_widget* self, tokotachi_action* action)
{
    try
    {
        c::self_as<tokotachi::Widget>(self, __func__)->add_action(c::arg_as<tokotachi::Action>(action, __func__));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_widget_remove_action(tokotachi_widget* self, tokotachi_action* action)
{
    try
    {
        c::self_as<tokotachi::Widget>(self, __func__)->remove_action(c::arg_as<tokotachi::Action>(action, __func__));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_widget_invalidate(tokotachi_widget* self)
{
    try
    {
        c::self_as<tokotachi::Widget>(self, __func__)->invalidate();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_widget_invalidate_rect(tokotachi_widget* self, tokotachi_rect rect)
{
    try
    {
        c::self_as<tokotachi::Widget>(self, __func__)->invalidate(c::from_c(rect));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_widget_request_layout(tokotachi_widget* self)
{
    try
    {
        c::self_as<tokotachi::Widget>(self, __func__)->request_layout();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_widget_apply_layout(tokotachi_widget* self)
{
    try
    {
        c::self_as<tokotachi::Widget>(self, __func__)->apply_layout();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_point tokotachi_widget_map_to_window(tokotachi_widget* self, tokotachi_point p)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::Widget>(self, __func__)->map_to_window(c::from_c(p)));
    }
    TOKOTACHI_C_CATCH(tokotachi_point())
}

TOKOTACHI_C_API tokotachi_point tokotachi_widget_map_from_window(tokotachi_widget* self, tokotachi_point p)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::Widget>(self, __func__)->map_from_window(c::from_c(p)));
    }
    TOKOTACHI_C_CATCH(tokotachi_point())
}

TOKOTACHI_C_API tokotachi_widget* tokotachi_widget_child_at(tokotachi_widget* self, tokotachi_point p)
{
    try
    {
        return c::handle(c::self_as<tokotachi::Widget>(self, __func__)->child_at(c::from_c(p)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API bool tokotachi_widget_start_drag(tokotachi_widget* self, const tokotachi_drag_data* data, const char* label)
{
    try
    {
        return c::self_as<tokotachi::Widget>(self, __func__)->start_drag(c::drag_data_of(data), c::str(label));
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API bool tokotachi_widget_focus_next_after(tokotachi_widget* self)
{
    try
    {
        return c::self_as<tokotachi::Widget>(self, __func__)->focus_next_after();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API bool tokotachi_widget_focus_first_within(tokotachi_widget* self)
{
    try
    {
        return c::self_as<tokotachi::Widget>(self, __func__)->focus_first_within();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_widget_set_on_paint(tokotachi_widget* self, tokotachi_paint_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::PaintHandler handler = c::paint_handler(fn, user_data, destroy);
        c::self_as<tokotachi::Widget>(self, __func__)->on_paint = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_widget_set_on_resize(tokotachi_widget* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::Widget>(self, __func__)->on_resize = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_widget_set_on_mouse_down(tokotachi_widget* self, tokotachi_mouse_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::MouseHandler handler = c::mouse_handler(fn, user_data, destroy);
        c::self_as<tokotachi::Widget>(self, __func__)->on_mouse_down = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_widget_set_on_mouse_up(tokotachi_widget* self, tokotachi_mouse_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::MouseHandler handler = c::mouse_handler(fn, user_data, destroy);
        c::self_as<tokotachi::Widget>(self, __func__)->on_mouse_up = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_widget_set_on_mouse_move(tokotachi_widget* self, tokotachi_mouse_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::MouseHandler handler = c::mouse_handler(fn, user_data, destroy);
        c::self_as<tokotachi::Widget>(self, __func__)->on_mouse_move = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_widget_set_on_mouse_enter(tokotachi_widget* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::Widget>(self, __func__)->on_mouse_enter = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_widget_set_on_mouse_leave(tokotachi_widget* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::Widget>(self, __func__)->on_mouse_leave = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_widget_set_on_mouse_wheel(tokotachi_widget* self, tokotachi_wheel_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::WheelHandler handler = c::wheel_handler(fn, user_data, destroy);
        c::self_as<tokotachi::Widget>(self, __func__)->on_mouse_wheel = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_widget_set_on_key_down(tokotachi_widget* self, tokotachi_key_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::KeyHandler handler = c::key_handler(fn, user_data, destroy);
        c::self_as<tokotachi::Widget>(self, __func__)->on_key_down = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_widget_set_on_key_up(tokotachi_widget* self, tokotachi_key_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::KeyHandler handler = c::key_handler(fn, user_data, destroy);
        c::self_as<tokotachi::Widget>(self, __func__)->on_key_up = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_widget_set_on_focus_in(tokotachi_widget* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::Widget>(self, __func__)->on_focus_in = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_widget_set_on_focus_out(tokotachi_widget* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::Widget>(self, __func__)->on_focus_out = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_widget_set_on_drag_move(tokotachi_widget* self, tokotachi_drag_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::DragHandler handler = c::drag_handler(fn, user_data, destroy);
        c::self_as<tokotachi::Widget>(self, __func__)->on_drag_move = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_widget_set_on_drag_leave(tokotachi_widget* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::Widget>(self, __func__)->on_drag_leave = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_widget_set_on_drop(tokotachi_widget* self, tokotachi_drag_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::DragHandler handler = c::drag_handler(fn, user_data, destroy);
        c::self_as<tokotachi::Widget>(self, __func__)->on_drop = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_widget_set_on_drag_finish(tokotachi_widget* self, tokotachi_drag_finish_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::DragFinishHandler handler = c::drag_finish_handler(fn, user_data, destroy);
        c::self_as<tokotachi::Widget>(self, __func__)->on_drag_finish = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API char* tokotachi_button_base_get_text(tokotachi_button_base* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::ButtonBase>(self, __func__)->text.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_button_base_set_text(tokotachi_button_base* self, const char* value)
{
    try
    {
        c::self_as<tokotachi::ButtonBase>(self, __func__)->text = c::str(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_font tokotachi_button_base_get_font(tokotachi_button_base* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::ButtonBase>(self, __func__)->font.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_font())
}

TOKOTACHI_C_API void tokotachi_button_base_set_font(tokotachi_button_base* self, tokotachi_font value)
{
    try
    {
        c::self_as<tokotachi::ButtonBase>(self, __func__)->font = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_button_base_set_on_click(tokotachi_button_base* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::ButtonBase>(self, __func__)->on_click = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_button* tokotachi_button_new(tokotachi_widget* parent, const char* text)
{
    try
    {
        return c::handle(new tokotachi::Button(c::arg_as<tokotachi::Widget>(parent, __func__), c::str(text)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_button_style tokotachi_button_get_style(tokotachi_button* self)
{
    try
    {
        return static_cast<tokotachi_button_style>(c::self_as<tokotachi::Button>(self, __func__)->style.get());
    }
    TOKOTACHI_C_CATCH(static_cast<tokotachi_button_style>(0))
}

TOKOTACHI_C_API void tokotachi_button_set_style(tokotachi_button* self, tokotachi_button_style value)
{
    try
    {
        c::self_as<tokotachi::Button>(self, __func__)->style = c::enum_in<tokotachi::ButtonStyle>(value, 2);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_image* tokotachi_button_get_icon(tokotachi_button* self)
{
    try
    {
        return c::to_c_image(c::self_as<tokotachi::Button>(self, __func__)->icon.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_button_set_icon(tokotachi_button* self, const tokotachi_image* value)
{
    try
    {
        c::self_as<tokotachi::Button>(self, __func__)->icon = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API float tokotachi_button_get_icon_size(tokotachi_button* self)
{
    try
    {
        return c::self_as<tokotachi::Button>(self, __func__)->icon_size.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_button_set_icon_size(tokotachi_button* self, float value)
{
    try
    {
        c::self_as<tokotachi::Button>(self, __func__)->icon_size = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_check_box* tokotachi_check_box_new(tokotachi_widget* parent, const char* text)
{
    try
    {
        return c::handle(new tokotachi::CheckBox(c::arg_as<tokotachi::Widget>(parent, __func__), c::str(text)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API bool tokotachi_check_box_get_checked(tokotachi_check_box* self)
{
    try
    {
        return c::self_as<tokotachi::CheckBox>(self, __func__)->checked.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_check_box_set_checked(tokotachi_check_box* self, bool value)
{
    try
    {
        c::self_as<tokotachi::CheckBox>(self, __func__)->checked = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_check_box_set_on_change(tokotachi_check_box* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::CheckBox>(self, __func__)->on_change = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_radio_button* tokotachi_radio_button_new(tokotachi_widget* parent, const char* text, int value)
{
    try
    {
        return c::handle(new tokotachi::RadioButton(c::arg_as<tokotachi::Widget>(parent, __func__), c::str(text), value));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API int tokotachi_radio_button_get_value(tokotachi_radio_button* self)
{
    try
    {
        return c::self_as<tokotachi::RadioButton>(self, __func__)->value.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_radio_button_set_value(tokotachi_radio_button* self, int value)
{
    try
    {
        c::self_as<tokotachi::RadioButton>(self, __func__)->value = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API int tokotachi_radio_button_get_group_value(tokotachi_radio_button* self)
{
    try
    {
        return c::self_as<tokotachi::RadioButton>(self, __func__)->group_value.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_radio_button_set_group_value(tokotachi_radio_button* self, int value)
{
    try
    {
        c::self_as<tokotachi::RadioButton>(self, __func__)->group_value = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_radio_button_get_checked(tokotachi_radio_button* self)
{
    try
    {
        return c::self_as<tokotachi::RadioButton>(self, __func__)->checked.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_radio_button_set_on_change(tokotachi_radio_button* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::RadioButton>(self, __func__)->on_change = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_combo_box* tokotachi_combo_box_new(tokotachi_widget* parent)
{
    try
    {
        return c::handle(new tokotachi::ComboBox(c::arg_as<tokotachi::Widget>(parent, __func__)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API char** tokotachi_combo_box_get_items(tokotachi_combo_box* self)
{
    try
    {
        return c::dup_strv(c::self_as<tokotachi::ComboBox>(self, __func__)->items.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_combo_box_set_items(tokotachi_combo_box* self, const char* const* value, size_t value_count)
{
    try
    {
        c::self_as<tokotachi::ComboBox>(self, __func__)->items = c::strv(value, value_count);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API int tokotachi_combo_box_get_current_index(tokotachi_combo_box* self)
{
    try
    {
        return c::self_as<tokotachi::ComboBox>(self, __func__)->current_index.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_combo_box_set_current_index(tokotachi_combo_box* self, int value)
{
    try
    {
        c::self_as<tokotachi::ComboBox>(self, __func__)->current_index = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API char* tokotachi_combo_box_get_current_text(tokotachi_combo_box* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::ComboBox>(self, __func__)->current_text.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API char* tokotachi_combo_box_get_placeholder_text(tokotachi_combo_box* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::ComboBox>(self, __func__)->placeholder_text.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_combo_box_set_placeholder_text(tokotachi_combo_box* self, const char* value)
{
    try
    {
        c::self_as<tokotachi::ComboBox>(self, __func__)->placeholder_text = c::str(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_font tokotachi_combo_box_get_font(tokotachi_combo_box* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::ComboBox>(self, __func__)->font.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_font())
}

TOKOTACHI_C_API void tokotachi_combo_box_set_font(tokotachi_combo_box* self, tokotachi_font value)
{
    try
    {
        c::self_as<tokotachi::ComboBox>(self, __func__)->font = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_combo_box_set_on_change(tokotachi_combo_box* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::ComboBox>(self, __func__)->on_change = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_combo_box_open_popup(tokotachi_combo_box* self)
{
    try
    {
        c::self_as<tokotachi::ComboBox>(self, __func__)->open_popup();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_combo_box_close_popup(tokotachi_combo_box* self)
{
    try
    {
        c::self_as<tokotachi::ComboBox>(self, __func__)->close_popup();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_combo_box_get_popup_open(tokotachi_combo_box* self)
{
    try
    {
        return c::self_as<tokotachi::ComboBox>(self, __func__)->popup_open.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API tokotachi_label* tokotachi_label_new(tokotachi_widget* parent, const char* text)
{
    try
    {
        return c::handle(new tokotachi::Label(c::arg_as<tokotachi::Widget>(parent, __func__), c::str(text)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API char* tokotachi_label_get_text(tokotachi_label* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::Label>(self, __func__)->text.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_label_set_text(tokotachi_label* self, const char* value)
{
    try
    {
        c::self_as<tokotachi::Label>(self, __func__)->text = c::str(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_font tokotachi_label_get_font(tokotachi_label* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::Label>(self, __func__)->font.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_font())
}

TOKOTACHI_C_API void tokotachi_label_set_font(tokotachi_label* self, tokotachi_font value)
{
    try
    {
        c::self_as<tokotachi::Label>(self, __func__)->font = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_theme_color tokotachi_label_get_text_color(tokotachi_label* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::Label>(self, __func__)->text_color.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_theme_color())
}

TOKOTACHI_C_API void tokotachi_label_set_text_color(tokotachi_label* self, tokotachi_theme_color value)
{
    try
    {
        c::self_as<tokotachi::Label>(self, __func__)->text_color = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_align tokotachi_label_get_alignment(tokotachi_label* self)
{
    try
    {
        return static_cast<tokotachi_align>(c::self_as<tokotachi::Label>(self, __func__)->alignment.get());
    }
    TOKOTACHI_C_CATCH(static_cast<tokotachi_align>(0))
}

TOKOTACHI_C_API void tokotachi_label_set_alignment(tokotachi_label* self, tokotachi_align value)
{
    try
    {
        c::self_as<tokotachi::Label>(self, __func__)->alignment = c::enum_in<tokotachi::Align>(value, 4);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_label_get_word_wrap(tokotachi_label* self)
{
    try
    {
        return c::self_as<tokotachi::Label>(self, __func__)->word_wrap.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_label_set_word_wrap(tokotachi_label* self, bool value)
{
    try
    {
        c::self_as<tokotachi::Label>(self, __func__)->word_wrap = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_image* tokotachi_label_get_icon(tokotachi_label* self)
{
    try
    {
        return c::to_c_image(c::self_as<tokotachi::Label>(self, __func__)->icon.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_label_set_icon(tokotachi_label* self, const tokotachi_image* value)
{
    try
    {
        c::self_as<tokotachi::Label>(self, __func__)->icon = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API float tokotachi_label_get_icon_size(tokotachi_label* self)
{
    try
    {
        return c::self_as<tokotachi::Label>(self, __func__)->icon_size.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_label_set_icon_size(tokotachi_label* self, float value)
{
    try
    {
        c::self_as<tokotachi::Label>(self, __func__)->icon_size = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_widget* tokotachi_label_get_buddy(tokotachi_label* self)
{
    try
    {
        return c::handle(c::self_as<tokotachi::Label>(self, __func__)->buddy.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_label_set_buddy(tokotachi_label* self, tokotachi_widget* value)
{
    try
    {
        c::self_as<tokotachi::Label>(self, __func__)->buddy = c::arg_as<tokotachi::Widget>(value, __func__);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API char* tokotachi_text_input_base_get_text(tokotachi_text_input_base* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::TextInputBase>(self, __func__)->text.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_text_input_base_set_text(tokotachi_text_input_base* self, const char* value)
{
    try
    {
        c::self_as<tokotachi::TextInputBase>(self, __func__)->text = c::str(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API char* tokotachi_text_input_base_get_placeholder_text(tokotachi_text_input_base* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::TextInputBase>(self, __func__)->placeholder_text.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_text_input_base_set_placeholder_text(tokotachi_text_input_base* self, const char* value)
{
    try
    {
        c::self_as<tokotachi::TextInputBase>(self, __func__)->placeholder_text = c::str(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_font tokotachi_text_input_base_get_font(tokotachi_text_input_base* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::TextInputBase>(self, __func__)->font.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_font())
}

TOKOTACHI_C_API void tokotachi_text_input_base_set_font(tokotachi_text_input_base* self, tokotachi_font value)
{
    try
    {
        c::self_as<tokotachi::TextInputBase>(self, __func__)->font = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API size_t tokotachi_text_input_base_get_cursor_position(tokotachi_text_input_base* self)
{
    try
    {
        return c::self_as<tokotachi::TextInputBase>(self, __func__)->cursor_position.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_text_input_base_set_cursor_position(tokotachi_text_input_base* self, size_t value)
{
    try
    {
        c::self_as<tokotachi::TextInputBase>(self, __func__)->cursor_position = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_text_input_base_get_has_selection(tokotachi_text_input_base* self)
{
    try
    {
        return c::self_as<tokotachi::TextInputBase>(self, __func__)->has_selection.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API char* tokotachi_text_input_base_get_selected_text(tokotachi_text_input_base* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::TextInputBase>(self, __func__)->selected_text.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API bool tokotachi_text_input_base_get_can_undo(tokotachi_text_input_base* self)
{
    try
    {
        return c::self_as<tokotachi::TextInputBase>(self, __func__)->can_undo.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API bool tokotachi_text_input_base_get_can_redo(tokotachi_text_input_base* self)
{
    try
    {
        return c::self_as<tokotachi::TextInputBase>(self, __func__)->can_redo.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_text_input_base_set_on_change(tokotachi_text_input_base* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::TextInputBase>(self, __func__)->on_change = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_text_input_base_select_all(tokotachi_text_input_base* self)
{
    try
    {
        c::self_as<tokotachi::TextInputBase>(self, __func__)->select_all();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_text_input_base_clear_selection(tokotachi_text_input_base* self)
{
    try
    {
        c::self_as<tokotachi::TextInputBase>(self, __func__)->clear_selection();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_text_input_base_cut(tokotachi_text_input_base* self)
{
    try
    {
        c::self_as<tokotachi::TextInputBase>(self, __func__)->cut();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_text_input_base_copy(tokotachi_text_input_base* self)
{
    try
    {
        c::self_as<tokotachi::TextInputBase>(self, __func__)->copy();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_text_input_base_paste(tokotachi_text_input_base* self)
{
    try
    {
        c::self_as<tokotachi::TextInputBase>(self, __func__)->paste();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_text_input_base_insert(tokotachi_text_input_base* self, const char* s)
{
    try
    {
        c::self_as<tokotachi::TextInputBase>(self, __func__)->insert(c::str(s));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_text_input_base_undo(tokotachi_text_input_base* self)
{
    try
    {
        c::self_as<tokotachi::TextInputBase>(self, __func__)->undo();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_text_input_base_redo(tokotachi_text_input_base* self)
{
    try
    {
        c::self_as<tokotachi::TextInputBase>(self, __func__)->redo();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_text_input_base_clear_undo_history(tokotachi_text_input_base* self)
{
    try
    {
        c::self_as<tokotachi::TextInputBase>(self, __func__)->clear_undo_history();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_line_edit* tokotachi_line_edit_new(tokotachi_widget* parent, const char* text)
{
    try
    {
        return c::handle(new tokotachi::LineEdit(c::arg_as<tokotachi::Widget>(parent, __func__), c::str(text)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_line_edit_set_on_submit(tokotachi_line_edit* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::LineEdit>(self, __func__)->on_submit = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_text_edit* tokotachi_text_edit_new(tokotachi_widget* parent, const char* text)
{
    try
    {
        return c::handle(new tokotachi::TextEdit(c::arg_as<tokotachi::Widget>(parent, __func__), c::str(text)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API bool tokotachi_text_edit_get_word_wrap(tokotachi_text_edit* self)
{
    try
    {
        return c::self_as<tokotachi::TextEdit>(self, __func__)->word_wrap.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_text_edit_set_word_wrap(tokotachi_text_edit* self, bool value)
{
    try
    {
        c::self_as<tokotachi::TextEdit>(self, __func__)->word_wrap = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_paint_box* tokotachi_paint_box_new(tokotachi_widget* parent)
{
    try
    {
        return c::handle(new tokotachi::PaintBox(c::arg_as<tokotachi::Widget>(parent, __func__)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API bool tokotachi_paint_box_get_cached(tokotachi_paint_box* self)
{
    try
    {
        return c::self_as<tokotachi::PaintBox>(self, __func__)->cached.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_paint_box_set_cached(tokotachi_paint_box* self, bool value)
{
    try
    {
        c::self_as<tokotachi::PaintBox>(self, __func__)->cached = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_paint_box_refresh(tokotachi_paint_box* self)
{
    try
    {
        c::self_as<tokotachi::PaintBox>(self, __func__)->refresh();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_progress_bar* tokotachi_progress_bar_new(tokotachi_widget* parent)
{
    try
    {
        return c::handle(new tokotachi::ProgressBar(c::arg_as<tokotachi::Widget>(parent, __func__)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API double tokotachi_progress_bar_get_minimum(tokotachi_progress_bar* self)
{
    try
    {
        return c::self_as<tokotachi::ProgressBar>(self, __func__)->minimum.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_progress_bar_set_minimum(tokotachi_progress_bar* self, double value)
{
    try
    {
        c::self_as<tokotachi::ProgressBar>(self, __func__)->minimum = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API double tokotachi_progress_bar_get_maximum(tokotachi_progress_bar* self)
{
    try
    {
        return c::self_as<tokotachi::ProgressBar>(self, __func__)->maximum.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_progress_bar_set_maximum(tokotachi_progress_bar* self, double value)
{
    try
    {
        c::self_as<tokotachi::ProgressBar>(self, __func__)->maximum = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API double tokotachi_progress_bar_get_value(tokotachi_progress_bar* self)
{
    try
    {
        return c::self_as<tokotachi::ProgressBar>(self, __func__)->value.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_progress_bar_set_value(tokotachi_progress_bar* self, double value)
{
    try
    {
        c::self_as<tokotachi::ProgressBar>(self, __func__)->value = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_progress_bar_get_text_visible(tokotachi_progress_bar* self)
{
    try
    {
        return c::self_as<tokotachi::ProgressBar>(self, __func__)->text_visible.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_progress_bar_set_text_visible(tokotachi_progress_bar* self, bool value)
{
    try
    {
        c::self_as<tokotachi::ProgressBar>(self, __func__)->text_visible = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_progress_bar_get_indeterminate(tokotachi_progress_bar* self)
{
    try
    {
        return c::self_as<tokotachi::ProgressBar>(self, __func__)->indeterminate.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_progress_bar_set_indeterminate(tokotachi_progress_bar* self, bool value)
{
    try
    {
        c::self_as<tokotachi::ProgressBar>(self, __func__)->indeterminate = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API double tokotachi_range_base_get_minimum(tokotachi_range_base* self)
{
    try
    {
        return c::self_as<tokotachi::RangeBase>(self, __func__)->minimum.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_range_base_set_minimum(tokotachi_range_base* self, double value)
{
    try
    {
        c::self_as<tokotachi::RangeBase>(self, __func__)->minimum = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API double tokotachi_range_base_get_maximum(tokotachi_range_base* self)
{
    try
    {
        return c::self_as<tokotachi::RangeBase>(self, __func__)->maximum.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_range_base_set_maximum(tokotachi_range_base* self, double value)
{
    try
    {
        c::self_as<tokotachi::RangeBase>(self, __func__)->maximum = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API double tokotachi_range_base_get_value(tokotachi_range_base* self)
{
    try
    {
        return c::self_as<tokotachi::RangeBase>(self, __func__)->value.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_range_base_set_value(tokotachi_range_base* self, double value)
{
    try
    {
        c::self_as<tokotachi::RangeBase>(self, __func__)->value = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API double tokotachi_range_base_get_single_step(tokotachi_range_base* self)
{
    try
    {
        return c::self_as<tokotachi::RangeBase>(self, __func__)->single_step.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_range_base_set_single_step(tokotachi_range_base* self, double value)
{
    try
    {
        c::self_as<tokotachi::RangeBase>(self, __func__)->single_step = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API double tokotachi_range_base_get_page_step(tokotachi_range_base* self)
{
    try
    {
        return c::self_as<tokotachi::RangeBase>(self, __func__)->page_step.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_range_base_set_page_step(tokotachi_range_base* self, double value)
{
    try
    {
        c::self_as<tokotachi::RangeBase>(self, __func__)->page_step = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_range_base_set_on_change(tokotachi_range_base* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::RangeBase>(self, __func__)->on_change = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_slider* tokotachi_slider_new(tokotachi_widget* parent, tokotachi_orientation orientation)
{
    try
    {
        return c::handle(new tokotachi::Slider(c::arg_as<tokotachi::Widget>(parent, __func__), c::enum_in<tokotachi::Orientation>(orientation, 2)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_orientation tokotachi_slider_get_orientation(tokotachi_slider* self)
{
    try
    {
        return static_cast<tokotachi_orientation>(c::self_as<tokotachi::Slider>(self, __func__)->orientation.get());
    }
    TOKOTACHI_C_CATCH(static_cast<tokotachi_orientation>(0))
}

TOKOTACHI_C_API void tokotachi_slider_set_orientation(tokotachi_slider* self, tokotachi_orientation value)
{
    try
    {
        c::self_as<tokotachi::Slider>(self, __func__)->orientation = c::enum_in<tokotachi::Orientation>(value, 2);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API double tokotachi_slider_get_tick_interval(tokotachi_slider* self)
{
    try
    {
        return c::self_as<tokotachi::Slider>(self, __func__)->tick_interval.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_slider_set_tick_interval(tokotachi_slider* self, double value)
{
    try
    {
        c::self_as<tokotachi::Slider>(self, __func__)->tick_interval = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_spin_box* tokotachi_spin_box_new(tokotachi_widget* parent)
{
    try
    {
        return c::handle(new tokotachi::SpinBox(c::arg_as<tokotachi::Widget>(parent, __func__)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API int tokotachi_spin_box_get_decimals(tokotachi_spin_box* self)
{
    try
    {
        return c::self_as<tokotachi::SpinBox>(self, __func__)->decimals.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_spin_box_set_decimals(tokotachi_spin_box* self, int value)
{
    try
    {
        c::self_as<tokotachi::SpinBox>(self, __func__)->decimals = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API char* tokotachi_spin_box_get_prefix(tokotachi_spin_box* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::SpinBox>(self, __func__)->prefix.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_spin_box_set_prefix(tokotachi_spin_box* self, const char* value)
{
    try
    {
        c::self_as<tokotachi::SpinBox>(self, __func__)->prefix = c::str(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API char* tokotachi_spin_box_get_suffix(tokotachi_spin_box* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::SpinBox>(self, __func__)->suffix.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_spin_box_set_suffix(tokotachi_spin_box* self, const char* value)
{
    try
    {
        c::self_as<tokotachi::SpinBox>(self, __func__)->suffix = c::str(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API char* tokotachi_spin_box_get_text(tokotachi_spin_box* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::SpinBox>(self, __func__)->text.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_spin_box_commit(tokotachi_spin_box* self)
{
    try
    {
        c::self_as<tokotachi::SpinBox>(self, __func__)->commit();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_scroll_area* tokotachi_scroll_area_new(tokotachi_widget* parent)
{
    try
    {
        return c::handle(new tokotachi::ScrollArea(c::arg_as<tokotachi::Widget>(parent, __func__)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_widget* tokotachi_scroll_area_get_content(tokotachi_scroll_area* self)
{
    try
    {
        return c::handle(c::self_as<tokotachi::ScrollArea>(self, __func__)->content.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_scroll_area_set_content(tokotachi_scroll_area* self, tokotachi_widget* value)
{
    try
    {
        c::self_as<tokotachi::ScrollArea>(self, __func__)->content = c::arg_as<tokotachi::Widget>(value, __func__);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_scroll_area_get_fit_width(tokotachi_scroll_area* self)
{
    try
    {
        return c::self_as<tokotachi::ScrollArea>(self, __func__)->fit_width.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_scroll_area_set_fit_width(tokotachi_scroll_area* self, bool value)
{
    try
    {
        c::self_as<tokotachi::ScrollArea>(self, __func__)->fit_width = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_point tokotachi_scroll_area_get_scroll_position(tokotachi_scroll_area* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::ScrollArea>(self, __func__)->scroll_position.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_point())
}

TOKOTACHI_C_API void tokotachi_scroll_area_set_scroll_position(tokotachi_scroll_area* self, tokotachi_point value)
{
    try
    {
        c::self_as<tokotachi::ScrollArea>(self, __func__)->scroll_position = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_scroll_area_scroll_to_visible(tokotachi_scroll_area* self, tokotachi_rect rect)
{
    try
    {
        c::self_as<tokotachi::ScrollArea>(self, __func__)->scroll_to_visible(c::from_c(rect));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_scroll_bar* tokotachi_scroll_bar_new(tokotachi_widget* parent, tokotachi_orientation orientation)
{
    try
    {
        return c::handle(new tokotachi::ScrollBar(c::arg_as<tokotachi::Widget>(parent, __func__), c::enum_in<tokotachi::Orientation>(orientation, 2)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_orientation tokotachi_scroll_bar_get_orientation(tokotachi_scroll_bar* self)
{
    try
    {
        return static_cast<tokotachi_orientation>(c::self_as<tokotachi::ScrollBar>(self, __func__)->orientation.get());
    }
    TOKOTACHI_C_CATCH(static_cast<tokotachi_orientation>(0))
}

TOKOTACHI_C_API void tokotachi_scroll_bar_set_orientation(tokotachi_scroll_bar* self, tokotachi_orientation value)
{
    try
    {
        c::self_as<tokotachi::ScrollBar>(self, __func__)->orientation = c::enum_in<tokotachi::Orientation>(value, 2);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API float tokotachi_scroll_bar_get_value(tokotachi_scroll_bar* self)
{
    try
    {
        return c::self_as<tokotachi::ScrollBar>(self, __func__)->value.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_scroll_bar_set_value(tokotachi_scroll_bar* self, float value)
{
    try
    {
        c::self_as<tokotachi::ScrollBar>(self, __func__)->value = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API float tokotachi_scroll_bar_get_maximum(tokotachi_scroll_bar* self)
{
    try
    {
        return c::self_as<tokotachi::ScrollBar>(self, __func__)->maximum.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_scroll_bar_set_maximum(tokotachi_scroll_bar* self, float value)
{
    try
    {
        c::self_as<tokotachi::ScrollBar>(self, __func__)->maximum = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API float tokotachi_scroll_bar_get_page_step(tokotachi_scroll_bar* self)
{
    try
    {
        return c::self_as<tokotachi::ScrollBar>(self, __func__)->page_step.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_scroll_bar_set_page_step(tokotachi_scroll_bar* self, float value)
{
    try
    {
        c::self_as<tokotachi::ScrollBar>(self, __func__)->page_step = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API float tokotachi_scroll_bar_get_single_step(tokotachi_scroll_bar* self)
{
    try
    {
        return c::self_as<tokotachi::ScrollBar>(self, __func__)->single_step.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_scroll_bar_set_single_step(tokotachi_scroll_bar* self, float value)
{
    try
    {
        c::self_as<tokotachi::ScrollBar>(self, __func__)->single_step = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_scroll_bar_set_on_change(tokotachi_scroll_bar* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::ScrollBar>(self, __func__)->on_change = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API int tokotachi_splitter_pane_get_stretch(tokotachi_splitter_pane* self)
{
    try
    {
        return c::self_as<tokotachi::SplitterPane>(self, __func__)->stretch.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_splitter_pane_set_stretch(tokotachi_splitter_pane* self, int value)
{
    try
    {
        c::self_as<tokotachi::SplitterPane>(self, __func__)->stretch = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_splitter_pane_get_collapsible(tokotachi_splitter_pane* self)
{
    try
    {
        return c::self_as<tokotachi::SplitterPane>(self, __func__)->collapsible.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_splitter_pane_set_collapsible(tokotachi_splitter_pane* self, bool value)
{
    try
    {
        c::self_as<tokotachi::SplitterPane>(self, __func__)->collapsible = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_splitter_pane_get_collapsed(tokotachi_splitter_pane* self)
{
    try
    {
        return c::self_as<tokotachi::SplitterPane>(self, __func__)->collapsed.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API tokotachi_widget* tokotachi_splitter_pane_get_widget(tokotachi_splitter_pane* self)
{
    try
    {
        return c::handle(c::self_as<tokotachi::SplitterPane>(self, __func__)->widget.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_splitter* tokotachi_splitter_new(tokotachi_widget* parent, tokotachi_orientation orientation)
{
    try
    {
        return c::handle(new tokotachi::Splitter(c::arg_as<tokotachi::Widget>(parent, __func__), c::enum_in<tokotachi::Orientation>(orientation, 2)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_orientation tokotachi_splitter_get_orientation(tokotachi_splitter* self)
{
    try
    {
        return static_cast<tokotachi_orientation>(c::self_as<tokotachi::Splitter>(self, __func__)->orientation.get());
    }
    TOKOTACHI_C_CATCH(static_cast<tokotachi_orientation>(0))
}

TOKOTACHI_C_API void tokotachi_splitter_set_orientation(tokotachi_splitter* self, tokotachi_orientation value)
{
    try
    {
        c::self_as<tokotachi::Splitter>(self, __func__)->orientation = c::enum_in<tokotachi::Orientation>(value, 2);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API size_t tokotachi_splitter_get_panes(tokotachi_splitter* self, tokotachi_splitter_pane** out, size_t capacity)
{
    try
    {
        return c::fill_objects(c::self_as<tokotachi::Splitter>(self, __func__)->panes.get(), out, capacity);
    }
    TOKOTACHI_C_CATCH(0)
}

TOKOTACHI_C_API size_t tokotachi_splitter_get_sizes(tokotachi_splitter* self, float* out, size_t capacity)
{
    try
    {
        return c::fill_floats(c::self_as<tokotachi::Splitter>(self, __func__)->sizes.get(), out, capacity);
    }
    TOKOTACHI_C_CATCH(0)
}

TOKOTACHI_C_API void tokotachi_splitter_set_sizes(tokotachi_splitter* self, const float* value, size_t value_count)
{
    try
    {
        c::self_as<tokotachi::Splitter>(self, __func__)->sizes = c::floats(value, value_count);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_splitter_set_on_change(tokotachi_splitter* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::Splitter>(self, __func__)->on_change = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_splitter_pane* tokotachi_splitter_add_widget(tokotachi_splitter* self, tokotachi_widget* widget, int stretch)
{
    try
    {
        return c::handle(c::self_as<tokotachi::Splitter>(self, __func__)->add_widget(c::arg_as<tokotachi::Widget>(widget, __func__), stretch));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_splitter_pane* tokotachi_splitter_insert_widget(tokotachi_splitter* self, int index, tokotachi_widget* widget, int stretch)
{
    try
    {
        return c::handle(c::self_as<tokotachi::Splitter>(self, __func__)->insert_widget(index, c::arg_as<tokotachi::Widget>(widget, __func__), stretch));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API char* tokotachi_tab_get_title(tokotachi_tab* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::Tab>(self, __func__)->title.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_tab_set_title(tokotachi_tab* self, const char* value)
{
    try
    {
        c::self_as<tokotachi::Tab>(self, __func__)->title = c::str(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_tab_get_enabled(tokotachi_tab* self)
{
    try
    {
        return c::self_as<tokotachi::Tab>(self, __func__)->enabled.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_tab_set_enabled(tokotachi_tab* self, bool value)
{
    try
    {
        c::self_as<tokotachi::Tab>(self, __func__)->enabled = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_widget* tokotachi_tab_get_page(tokotachi_tab* self)
{
    try
    {
        return c::handle(c::self_as<tokotachi::Tab>(self, __func__)->page.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_tab_widget* tokotachi_tab_widget_new(tokotachi_widget* parent)
{
    try
    {
        return c::handle(new tokotachi::TabWidget(c::arg_as<tokotachi::Widget>(parent, __func__)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API size_t tokotachi_tab_widget_get_tabs(tokotachi_tab_widget* self, tokotachi_tab** out, size_t capacity)
{
    try
    {
        return c::fill_objects(c::self_as<tokotachi::TabWidget>(self, __func__)->tabs.get(), out, capacity);
    }
    TOKOTACHI_C_CATCH(0)
}

TOKOTACHI_C_API int tokotachi_tab_widget_get_current_index(tokotachi_tab_widget* self)
{
    try
    {
        return c::self_as<tokotachi::TabWidget>(self, __func__)->current_index.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_tab_widget_set_current_index(tokotachi_tab_widget* self, int value)
{
    try
    {
        c::self_as<tokotachi::TabWidget>(self, __func__)->current_index = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_tab* tokotachi_tab_widget_get_current_tab(tokotachi_tab_widget* self)
{
    try
    {
        return c::handle(c::self_as<tokotachi::TabWidget>(self, __func__)->current_tab.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_tab_widget_set_on_change(tokotachi_tab_widget* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::TabWidget>(self, __func__)->on_change = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_tab* tokotachi_tab_widget_add_tab(tokotachi_tab_widget* self, tokotachi_widget* page, const char* title)
{
    try
    {
        return c::handle(c::self_as<tokotachi::TabWidget>(self, __func__)->add_tab(c::arg_as<tokotachi::Widget>(page, __func__), c::str(title)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_tab* tokotachi_tab_widget_insert_tab(tokotachi_tab_widget* self, int index, tokotachi_widget* page, const char* title)
{
    try
    {
        return c::handle(c::self_as<tokotachi::TabWidget>(self, __func__)->insert_tab(index, c::arg_as<tokotachi::Widget>(page, __func__), c::str(title)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_widget* tokotachi_tab_widget_remove_tab(tokotachi_tab_widget* self, tokotachi_tab* tab)
{
    try
    {
        return c::handle(c::self_as<tokotachi::TabWidget>(self, __func__)->remove_tab(c::arg_as<tokotachi::Tab>(tab, __func__)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_group_box* tokotachi_group_box_new(tokotachi_widget* parent, const char* title)
{
    try
    {
        return c::handle(new tokotachi::GroupBox(c::arg_as<tokotachi::Widget>(parent, __func__), c::str(title)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API char* tokotachi_group_box_get_title(tokotachi_group_box* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::GroupBox>(self, __func__)->title.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_group_box_set_title(tokotachi_group_box* self, const char* value)
{
    try
    {
        c::self_as<tokotachi::GroupBox>(self, __func__)->title = c::str(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_rect tokotachi_group_box_get_content_rect(tokotachi_group_box* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::GroupBox>(self, __func__)->content_rect.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_rect())
}

TOKOTACHI_C_API tokotachi_image_box* tokotachi_image_box_new(tokotachi_widget* parent, const tokotachi_image* image)
{
    try
    {
        return c::handle(new tokotachi::ImageBox(c::arg_as<tokotachi::Widget>(parent, __func__), c::from_c(image)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_image* tokotachi_image_box_get_image(tokotachi_image_box* self)
{
    try
    {
        return c::to_c_image(c::self_as<tokotachi::ImageBox>(self, __func__)->image.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_image_box_set_image(tokotachi_image_box* self, const tokotachi_image* value)
{
    try
    {
        c::self_as<tokotachi::ImageBox>(self, __func__)->image = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_image_fit tokotachi_image_box_get_fit(tokotachi_image_box* self)
{
    try
    {
        return static_cast<tokotachi_image_fit>(c::self_as<tokotachi::ImageBox>(self, __func__)->fit.get());
    }
    TOKOTACHI_C_CATCH(static_cast<tokotachi_image_fit>(0))
}

TOKOTACHI_C_API void tokotachi_image_box_set_fit(tokotachi_image_box* self, tokotachi_image_fit value)
{
    try
    {
        c::self_as<tokotachi::ImageBox>(self, __func__)->fit = c::enum_in<tokotachi::ImageFit>(value, 4);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_calendar* tokotachi_calendar_new(tokotachi_widget* parent)
{
    try
    {
        return c::handle(new tokotachi::Calendar(c::arg_as<tokotachi::Widget>(parent, __func__)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_date tokotachi_calendar_get_date(tokotachi_calendar* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::Calendar>(self, __func__)->date.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_date())
}

TOKOTACHI_C_API void tokotachi_calendar_set_date(tokotachi_calendar* self, tokotachi_date value)
{
    try
    {
        c::self_as<tokotachi::Calendar>(self, __func__)->date = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_date tokotachi_calendar_get_minimum(tokotachi_calendar* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::Calendar>(self, __func__)->minimum.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_date())
}

TOKOTACHI_C_API void tokotachi_calendar_set_minimum(tokotachi_calendar* self, tokotachi_date value)
{
    try
    {
        c::self_as<tokotachi::Calendar>(self, __func__)->minimum = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_date tokotachi_calendar_get_maximum(tokotachi_calendar* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::Calendar>(self, __func__)->maximum.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_date())
}

TOKOTACHI_C_API void tokotachi_calendar_set_maximum(tokotachi_calendar* self, tokotachi_date value)
{
    try
    {
        c::self_as<tokotachi::Calendar>(self, __func__)->maximum = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_day_of_week tokotachi_calendar_get_first_day_of_week(tokotachi_calendar* self)
{
    try
    {
        return static_cast<tokotachi_day_of_week>(c::self_as<tokotachi::Calendar>(self, __func__)->first_day_of_week.get());
    }
    TOKOTACHI_C_CATCH(static_cast<tokotachi_day_of_week>(0))
}

TOKOTACHI_C_API void tokotachi_calendar_set_first_day_of_week(tokotachi_calendar* self, tokotachi_day_of_week value)
{
    try
    {
        c::self_as<tokotachi::Calendar>(self, __func__)->first_day_of_week = c::enum_in<tokotachi::DayOfWeek>(value, 7);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API int tokotachi_calendar_get_shown_year(tokotachi_calendar* self)
{
    try
    {
        return c::self_as<tokotachi::Calendar>(self, __func__)->shown_year.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API int tokotachi_calendar_get_shown_month(tokotachi_calendar* self)
{
    try
    {
        return c::self_as<tokotachi::Calendar>(self, __func__)->shown_month.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_calendar_set_on_change(tokotachi_calendar* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::Calendar>(self, __func__)->on_change = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_calendar_set_on_activate(tokotachi_calendar* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::Calendar>(self, __func__)->on_activate = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_calendar_show_month(tokotachi_calendar* self, int year, int month)
{
    try
    {
        c::self_as<tokotachi::Calendar>(self, __func__)->show_month(year, month);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_date_edit* tokotachi_date_edit_new(tokotachi_widget* parent)
{
    try
    {
        return c::handle(new tokotachi::DateEdit(c::arg_as<tokotachi::Widget>(parent, __func__)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_date tokotachi_date_edit_get_date(tokotachi_date_edit* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::DateEdit>(self, __func__)->date.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_date())
}

TOKOTACHI_C_API void tokotachi_date_edit_set_date(tokotachi_date_edit* self, tokotachi_date value)
{
    try
    {
        c::self_as<tokotachi::DateEdit>(self, __func__)->date = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_date tokotachi_date_edit_get_minimum(tokotachi_date_edit* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::DateEdit>(self, __func__)->minimum.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_date())
}

TOKOTACHI_C_API void tokotachi_date_edit_set_minimum(tokotachi_date_edit* self, tokotachi_date value)
{
    try
    {
        c::self_as<tokotachi::DateEdit>(self, __func__)->minimum = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_date tokotachi_date_edit_get_maximum(tokotachi_date_edit* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::DateEdit>(self, __func__)->maximum.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_date())
}

TOKOTACHI_C_API void tokotachi_date_edit_set_maximum(tokotachi_date_edit* self, tokotachi_date value)
{
    try
    {
        c::self_as<tokotachi::DateEdit>(self, __func__)->maximum = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API char* tokotachi_date_edit_get_format(tokotachi_date_edit* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::DateEdit>(self, __func__)->format.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_date_edit_set_format(tokotachi_date_edit* self, const char* value)
{
    try
    {
        c::self_as<tokotachi::DateEdit>(self, __func__)->format = c::str(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_day_of_week tokotachi_date_edit_get_first_day_of_week(tokotachi_date_edit* self)
{
    try
    {
        return static_cast<tokotachi_day_of_week>(c::self_as<tokotachi::DateEdit>(self, __func__)->first_day_of_week.get());
    }
    TOKOTACHI_C_CATCH(static_cast<tokotachi_day_of_week>(0))
}

TOKOTACHI_C_API void tokotachi_date_edit_set_first_day_of_week(tokotachi_date_edit* self, tokotachi_day_of_week value)
{
    try
    {
        c::self_as<tokotachi::DateEdit>(self, __func__)->first_day_of_week = c::enum_in<tokotachi::DayOfWeek>(value, 7);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API char* tokotachi_date_edit_get_placeholder_text(tokotachi_date_edit* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::DateEdit>(self, __func__)->placeholder_text.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_date_edit_set_placeholder_text(tokotachi_date_edit* self, const char* value)
{
    try
    {
        c::self_as<tokotachi::DateEdit>(self, __func__)->placeholder_text = c::str(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API char* tokotachi_date_edit_get_text(tokotachi_date_edit* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::DateEdit>(self, __func__)->text.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API bool tokotachi_date_edit_get_calendar_open(tokotachi_date_edit* self)
{
    try
    {
        return c::self_as<tokotachi::DateEdit>(self, __func__)->calendar_open.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_date_edit_set_on_change(tokotachi_date_edit* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::DateEdit>(self, __func__)->on_change = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_date_edit_commit(tokotachi_date_edit* self)
{
    try
    {
        c::self_as<tokotachi::DateEdit>(self, __func__)->commit();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_date_edit_open_calendar(tokotachi_date_edit* self)
{
    try
    {
        c::self_as<tokotachi::DateEdit>(self, __func__)->open_calendar();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_date_edit_close_calendar(tokotachi_date_edit* self)
{
    try
    {
        c::self_as<tokotachi::DateEdit>(self, __func__)->close_calendar();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_color_button* tokotachi_color_button_new(tokotachi_widget* parent, tokotachi_color color)
{
    try
    {
        return c::handle(new tokotachi::ColorButton(c::arg_as<tokotachi::Widget>(parent, __func__), c::from_c(color)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_color tokotachi_color_button_get_color(tokotachi_color_button* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::ColorButton>(self, __func__)->color.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_color())
}

TOKOTACHI_C_API void tokotachi_color_button_set_color(tokotachi_color_button* self, tokotachi_color value)
{
    try
    {
        c::self_as<tokotachi::ColorButton>(self, __func__)->color = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_color_button_get_alpha_enabled(tokotachi_color_button* self)
{
    try
    {
        return c::self_as<tokotachi::ColorButton>(self, __func__)->alpha_enabled.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_color_button_set_alpha_enabled(tokotachi_color_button* self, bool value)
{
    try
    {
        c::self_as<tokotachi::ColorButton>(self, __func__)->alpha_enabled = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API char* tokotachi_color_button_get_dialog_title(tokotachi_color_button* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::ColorButton>(self, __func__)->dialog_title.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_color_button_set_dialog_title(tokotachi_color_button* self, const char* value)
{
    try
    {
        c::self_as<tokotachi::ColorButton>(self, __func__)->dialog_title = c::str(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_color_button_set_on_change(tokotachi_color_button* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::ColorButton>(self, __func__)->on_change = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_color_button_open_dialog(tokotachi_color_button* self)
{
    try
    {
        c::self_as<tokotachi::ColorButton>(self, __func__)->open_dialog();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_window* tokotachi_window_new(const char* title, float width, float height)
{
    try
    {
        return c::handle(new tokotachi::Window(c::str(title), width, height));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API char* tokotachi_window_get_title(tokotachi_window* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::Window>(self, __func__)->title.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_window_set_title(tokotachi_window* self, const char* value)
{
    try
    {
        c::self_as<tokotachi::Window>(self, __func__)->title = c::str(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_theme_color tokotachi_window_get_background(tokotachi_window* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::Window>(self, __func__)->background.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_theme_color())
}

TOKOTACHI_C_API void tokotachi_window_set_background(tokotachi_window* self, tokotachi_theme_color value)
{
    try
    {
        c::self_as<tokotachi::Window>(self, __func__)->background = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_widget* tokotachi_window_get_focused_widget(tokotachi_window* self)
{
    try
    {
        return c::handle(c::self_as<tokotachi::Window>(self, __func__)->focused_widget.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_menu_bar* tokotachi_window_get_menu_bar(tokotachi_window* self)
{
    try
    {
        return c::handle(c::self_as<tokotachi::Window>(self, __func__)->menu_bar.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_window_set_menu_bar(tokotachi_window* self, tokotachi_menu_bar* value)
{
    try
    {
        c::self_as<tokotachi::Window>(self, __func__)->menu_bar = c::arg_as<tokotachi::MenuBar>(value, __func__);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_tool_bar* tokotachi_window_get_tool_bar(tokotachi_window* self)
{
    try
    {
        return c::handle(c::self_as<tokotachi::Window>(self, __func__)->tool_bar.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_window_set_tool_bar(tokotachi_window* self, tokotachi_tool_bar* value)
{
    try
    {
        c::self_as<tokotachi::Window>(self, __func__)->tool_bar = c::arg_as<tokotachi::ToolBar>(value, __func__);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_status_bar* tokotachi_window_get_status_bar(tokotachi_window* self)
{
    try
    {
        return c::handle(c::self_as<tokotachi::Window>(self, __func__)->status_bar.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_window_set_status_bar(tokotachi_window* self, tokotachi_status_bar* value)
{
    try
    {
        c::self_as<tokotachi::Window>(self, __func__)->status_bar = c::arg_as<tokotachi::StatusBar>(value, __func__);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_window_get_destroy_on_close(tokotachi_window* self)
{
    try
    {
        return c::self_as<tokotachi::Window>(self, __func__)->destroy_on_close.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_window_set_destroy_on_close(tokotachi_window* self, bool value)
{
    try
    {
        c::self_as<tokotachi::Window>(self, __func__)->destroy_on_close = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_window_set_on_close(tokotachi_window* self, tokotachi_close_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::CloseHandler handler = c::close_handler(fn, user_data, destroy);
        c::self_as<tokotachi::Window>(self, __func__)->on_close = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_window_close(tokotachi_window* self)
{
    try
    {
        c::self_as<tokotachi::Window>(self, __func__)->close();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_window_save_png(tokotachi_window* self, const char* path)
{
    try
    {
        return c::self_as<tokotachi::Window>(self, __func__)->save_png(c::str(path));
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API tokotachi_bitmap* tokotachi_window_capture(tokotachi_window* self)
{
    try
    {
        return c::to_c_bitmap(c::self_as<tokotachi::Window>(self, __func__)->capture());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_dialog* tokotachi_dialog_new(tokotachi_window* parent, const char* title)
{
    try
    {
        return c::handle(new tokotachi::Dialog(c::arg_as<tokotachi::Window>(parent, __func__), c::str(title)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_dialog_button tokotachi_dialog_get_result(tokotachi_dialog* self)
{
    try
    {
        return static_cast<tokotachi_dialog_button>(c::self_as<tokotachi::Dialog>(self, __func__)->result.get());
    }
    TOKOTACHI_C_CATCH(static_cast<tokotachi_dialog_button>(0))
}

TOKOTACHI_C_API void tokotachi_dialog_set_on_finish(tokotachi_dialog* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::Dialog>(self, __func__)->on_finish = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_button* tokotachi_dialog_add_button(tokotachi_dialog* self, tokotachi_dialog_button button, const char* text)
{
    try
    {
        return c::handle(c::self_as<tokotachi::Dialog>(self, __func__)->add_button(c::enum_in<tokotachi::DialogButton>(button, 8), c::str(text)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_dialog_open(tokotachi_dialog* self)
{
    try
    {
        c::self_as<tokotachi::Dialog>(self, __func__)->open();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_dialog_finish(tokotachi_dialog* self, tokotachi_dialog_button result)
{
    try
    {
        c::self_as<tokotachi::Dialog>(self, __func__)->finish(c::enum_in<tokotachi::DialogButton>(result, 8));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_message_dialog* tokotachi_message_dialog_new(tokotachi_window* parent, const char* title, const char* text, tokotachi_message_type type)
{
    try
    {
        return c::handle(new tokotachi::MessageDialog(c::arg_as<tokotachi::Window>(parent, __func__), c::str(title), c::str(text), c::enum_in<tokotachi::MessageType>(type, 4)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API char* tokotachi_message_dialog_get_text(tokotachi_message_dialog* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::MessageDialog>(self, __func__)->text.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_message_dialog_set_text(tokotachi_message_dialog* self, const char* value)
{
    try
    {
        c::self_as<tokotachi::MessageDialog>(self, __func__)->text = c::str(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_message_type tokotachi_message_dialog_get_type(tokotachi_message_dialog* self)
{
    try
    {
        return static_cast<tokotachi_message_type>(c::self_as<tokotachi::MessageDialog>(self, __func__)->type.get());
    }
    TOKOTACHI_C_CATCH(static_cast<tokotachi_message_type>(0))
}

TOKOTACHI_C_API void tokotachi_message_dialog_set_type(tokotachi_message_dialog* self, tokotachi_message_type value)
{
    try
    {
        c::self_as<tokotachi::MessageDialog>(self, __func__)->type = c::enum_in<tokotachi::MessageType>(value, 4);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_color_dialog* tokotachi_color_dialog_new(tokotachi_window* parent, const char* title)
{
    try
    {
        return c::handle(new tokotachi::ColorDialog(c::arg_as<tokotachi::Window>(parent, __func__), c::str(title)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_color tokotachi_color_dialog_get_color(tokotachi_color_dialog* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::ColorDialog>(self, __func__)->color.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_color())
}

TOKOTACHI_C_API void tokotachi_color_dialog_set_color(tokotachi_color_dialog* self, tokotachi_color value)
{
    try
    {
        c::self_as<tokotachi::ColorDialog>(self, __func__)->color = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_color_dialog_get_alpha_enabled(tokotachi_color_dialog* self)
{
    try
    {
        return c::self_as<tokotachi::ColorDialog>(self, __func__)->alpha_enabled.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_color_dialog_set_alpha_enabled(tokotachi_color_dialog* self, bool value)
{
    try
    {
        c::self_as<tokotachi::ColorDialog>(self, __func__)->alpha_enabled = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_file_dialog* tokotachi_file_dialog_new(tokotachi_window* parent, tokotachi_file_dialog_mode mode)
{
    try
    {
        return c::handle(new tokotachi::FileDialog(c::arg_as<tokotachi::Window>(parent, __func__), c::enum_in<tokotachi::FileDialogMode>(mode, 3)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_file_dialog_mode tokotachi_file_dialog_get_mode(tokotachi_file_dialog* self)
{
    try
    {
        return static_cast<tokotachi_file_dialog_mode>(c::self_as<tokotachi::FileDialog>(self, __func__)->mode.get());
    }
    TOKOTACHI_C_CATCH(static_cast<tokotachi_file_dialog_mode>(0))
}

TOKOTACHI_C_API void tokotachi_file_dialog_set_mode(tokotachi_file_dialog* self, tokotachi_file_dialog_mode value)
{
    try
    {
        c::self_as<tokotachi::FileDialog>(self, __func__)->mode = c::enum_in<tokotachi::FileDialogMode>(value, 3);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API char* tokotachi_file_dialog_get_title(tokotachi_file_dialog* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::FileDialog>(self, __func__)->title.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_file_dialog_set_title(tokotachi_file_dialog* self, const char* value)
{
    try
    {
        c::self_as<tokotachi::FileDialog>(self, __func__)->title = c::str(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API char** tokotachi_file_dialog_get_filters(tokotachi_file_dialog* self)
{
    try
    {
        return c::file_filters_out(c::self_as<tokotachi::FileDialog>(self, __func__)->filters.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_file_dialog_set_filters(tokotachi_file_dialog* self, const char* const* value, size_t value_count)
{
    try
    {
        c::self_as<tokotachi::FileDialog>(self, __func__)->filters = c::file_filters_in(value, value_count);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API char* tokotachi_file_dialog_get_location(tokotachi_file_dialog* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::FileDialog>(self, __func__)->location.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_file_dialog_set_location(tokotachi_file_dialog* self, const char* value)
{
    try
    {
        c::self_as<tokotachi::FileDialog>(self, __func__)->location = c::str(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_file_dialog_get_multi_enabled(tokotachi_file_dialog* self)
{
    try
    {
        return c::self_as<tokotachi::FileDialog>(self, __func__)->multi_enabled.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_file_dialog_set_multi_enabled(tokotachi_file_dialog* self, bool value)
{
    try
    {
        c::self_as<tokotachi::FileDialog>(self, __func__)->multi_enabled = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API char** tokotachi_file_dialog_get_paths(tokotachi_file_dialog* self)
{
    try
    {
        return c::dup_strv(c::self_as<tokotachi::FileDialog>(self, __func__)->paths.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API char* tokotachi_file_dialog_get_error(tokotachi_file_dialog* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::FileDialog>(self, __func__)->error.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API int tokotachi_file_dialog_get_selected_filter(tokotachi_file_dialog* self)
{
    try
    {
        return c::self_as<tokotachi::FileDialog>(self, __func__)->selected_filter.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API bool tokotachi_file_dialog_get_running(tokotachi_file_dialog* self)
{
    try
    {
        return c::self_as<tokotachi::FileDialog>(self, __func__)->running.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_file_dialog_set_on_finish(tokotachi_file_dialog* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::FileDialog>(self, __func__)->on_finish = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_file_dialog_open(tokotachi_file_dialog* self)
{
    try
    {
        c::self_as<tokotachi::FileDialog>(self, __func__)->open();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_menu_bar* tokotachi_menu_bar_new(tokotachi_widget* parent)
{
    try
    {
        return c::handle(new tokotachi::MenuBar(c::arg_as<tokotachi::Widget>(parent, __func__)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API size_t tokotachi_menu_bar_get_menus(tokotachi_menu_bar* self, tokotachi_menu** out, size_t capacity)
{
    try
    {
        return c::fill_objects(c::self_as<tokotachi::MenuBar>(self, __func__)->menus.get(), out, capacity);
    }
    TOKOTACHI_C_CATCH(0)
}

TOKOTACHI_C_API tokotachi_menu* tokotachi_menu_bar_add_menu(tokotachi_menu_bar* self, const char* title)
{
    try
    {
        return c::handle(c::self_as<tokotachi::MenuBar>(self, __func__)->add_menu(c::str(title)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_menu_bar_remove_menu(tokotachi_menu_bar* self, tokotachi_menu* menu)
{
    try
    {
        c::self_as<tokotachi::MenuBar>(self, __func__)->remove_menu(c::arg_as<tokotachi::Menu>(menu, __func__));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_popup* tokotachi_popup_new(tokotachi_widget* owner_widget, tokotachi_popup_kind kind, tokotachi_object* owner_object)
{
    try
    {
        return c::handle(new tokotachi::Popup(c::arg_as<tokotachi::Widget>(owner_widget, __func__), c::enum_in<tokotachi::PopupKind>(kind, 2), c::arg_as<tokotachi::Object>(owner_object, __func__)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_widget* tokotachi_popup_get_owner_widget(tokotachi_popup* self)
{
    try
    {
        return c::handle(c::self_as<tokotachi::Popup>(self, __func__)->owner_widget.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_popup_kind tokotachi_popup_get_kind(tokotachi_popup* self)
{
    try
    {
        return static_cast<tokotachi_popup_kind>(c::self_as<tokotachi::Popup>(self, __func__)->kind.get());
    }
    TOKOTACHI_C_CATCH(static_cast<tokotachi_popup_kind>(0))
}

TOKOTACHI_C_API void tokotachi_popup_set_on_closed(tokotachi_popup* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::Popup>(self, __func__)->on_closed = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_popup_open(tokotachi_popup* self, tokotachi_rect anchor, tokotachi_size size, tokotachi_popup_placement placement)
{
    try
    {
        c::self_as<tokotachi::Popup>(self, __func__)->open(c::from_c(anchor), c::from_c(size), c::enum_in<tokotachi::PopupPlacement>(placement, 3));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_status_bar* tokotachi_status_bar_new(tokotachi_widget* parent)
{
    try
    {
        return c::handle(new tokotachi::StatusBar(c::arg_as<tokotachi::Widget>(parent, __func__)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API char* tokotachi_status_bar_get_text(tokotachi_status_bar* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::StatusBar>(self, __func__)->text.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_status_bar_set_text(tokotachi_status_bar* self, const char* value)
{
    try
    {
        c::self_as<tokotachi::StatusBar>(self, __func__)->text = c::str(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API char* tokotachi_status_bar_get_message(tokotachi_status_bar* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::StatusBar>(self, __func__)->message.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_status_bar_show_message(tokotachi_status_bar* self, const char* message, int timeout_ms)
{
    try
    {
        c::self_as<tokotachi::StatusBar>(self, __func__)->show_message(c::str(message), timeout_ms);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_status_bar_clear_message(tokotachi_status_bar* self)
{
    try
    {
        c::self_as<tokotachi::StatusBar>(self, __func__)->clear_message();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_status_bar_add_widget(tokotachi_status_bar* self, tokotachi_widget* widget)
{
    try
    {
        c::self_as<tokotachi::StatusBar>(self, __func__)->add_widget(c::arg_as<tokotachi::Widget>(widget, __func__));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_tool_bar* tokotachi_tool_bar_new(tokotachi_widget* parent)
{
    try
    {
        return c::handle(new tokotachi::ToolBar(c::arg_as<tokotachi::Widget>(parent, __func__)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_tool_button_style tokotachi_tool_bar_get_button_style(tokotachi_tool_bar* self)
{
    try
    {
        return static_cast<tokotachi_tool_button_style>(c::self_as<tokotachi::ToolBar>(self, __func__)->button_style.get());
    }
    TOKOTACHI_C_CATCH(static_cast<tokotachi_tool_button_style>(0))
}

TOKOTACHI_C_API void tokotachi_tool_bar_set_button_style(tokotachi_tool_bar* self, tokotachi_tool_button_style value)
{
    try
    {
        c::self_as<tokotachi::ToolBar>(self, __func__)->button_style = c::enum_in<tokotachi::ToolButtonStyle>(value, 3);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_action* tokotachi_tool_bar_add_item(tokotachi_tool_bar* self, const char* text)
{
    try
    {
        return c::handle(c::self_as<tokotachi::ToolBar>(self, __func__)->add_item(c::str(text)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_tool_bar_add_separator(tokotachi_tool_bar* self)
{
    try
    {
        c::self_as<tokotachi::ToolBar>(self, __func__)->add_separator();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_tool_bar_add_widget(tokotachi_tool_bar* self, tokotachi_widget* widget)
{
    try
    {
        c::self_as<tokotachi::ToolBar>(self, __func__)->add_widget(c::arg_as<tokotachi::Widget>(widget, __func__));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_widget* tokotachi_layout_get_widget(tokotachi_layout* self)
{
    try
    {
        return c::handle(c::self_as<tokotachi::Layout>(self, __func__)->widget.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_margins tokotachi_layout_get_margins(tokotachi_layout* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::Layout>(self, __func__)->margins.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_margins())
}

TOKOTACHI_C_API void tokotachi_layout_set_margins(tokotachi_layout* self, tokotachi_margins value)
{
    try
    {
        c::self_as<tokotachi::Layout>(self, __func__)->margins = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_layout_remove(tokotachi_layout* self, tokotachi_widget* widget)
{
    try
    {
        c::self_as<tokotachi::Layout>(self, __func__)->remove(c::arg_as<tokotachi::Widget>(widget, __func__));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_box_layout* tokotachi_box_layout_new(tokotachi_widget* target, tokotachi_orientation orientation)
{
    try
    {
        return c::handle(new tokotachi::BoxLayout(c::arg_as<tokotachi::Widget>(target, __func__), c::enum_in<tokotachi::Orientation>(orientation, 2)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_orientation tokotachi_box_layout_get_orientation(tokotachi_box_layout* self)
{
    try
    {
        return static_cast<tokotachi_orientation>(c::self_as<tokotachi::BoxLayout>(self, __func__)->orientation.get());
    }
    TOKOTACHI_C_CATCH(static_cast<tokotachi_orientation>(0))
}

TOKOTACHI_C_API void tokotachi_box_layout_set_orientation(tokotachi_box_layout* self, tokotachi_orientation value)
{
    try
    {
        c::self_as<tokotachi::BoxLayout>(self, __func__)->orientation = c::enum_in<tokotachi::Orientation>(value, 2);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API float tokotachi_box_layout_get_spacing(tokotachi_box_layout* self)
{
    try
    {
        return c::self_as<tokotachi::BoxLayout>(self, __func__)->spacing.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_box_layout_set_spacing(tokotachi_box_layout* self, float value)
{
    try
    {
        c::self_as<tokotachi::BoxLayout>(self, __func__)->spacing = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_box_layout_add(tokotachi_box_layout* self, tokotachi_widget* widget, int stretch, tokotachi_align cross_alignment)
{
    try
    {
        c::self_as<tokotachi::BoxLayout>(self, __func__)->add(c::arg_as<tokotachi::Widget>(widget, __func__), stretch, c::enum_in<tokotachi::Align>(cross_alignment, 4));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_box_layout_add_spacing(tokotachi_box_layout* self, float size)
{
    try
    {
        c::self_as<tokotachi::BoxLayout>(self, __func__)->add_spacing(size);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_box_layout_add_stretch(tokotachi_box_layout* self, int stretch)
{
    try
    {
        c::self_as<tokotachi::BoxLayout>(self, __func__)->add_stretch(stretch);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_box_layout_set_stretch(tokotachi_box_layout* self, tokotachi_widget* widget, int stretch)
{
    try
    {
        c::self_as<tokotachi::BoxLayout>(self, __func__)->set_stretch(c::arg_as<tokotachi::Widget>(widget, __func__), stretch);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_box_layout_set_alignment(tokotachi_box_layout* self, tokotachi_widget* widget, tokotachi_align cross_alignment)
{
    try
    {
        c::self_as<tokotachi::BoxLayout>(self, __func__)->set_alignment(c::arg_as<tokotachi::Widget>(widget, __func__), c::enum_in<tokotachi::Align>(cross_alignment, 4));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_grid_layout* tokotachi_grid_layout_new(tokotachi_widget* target)
{
    try
    {
        return c::handle(new tokotachi::GridLayout(c::arg_as<tokotachi::Widget>(target, __func__)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API float tokotachi_grid_layout_get_row_spacing(tokotachi_grid_layout* self)
{
    try
    {
        return c::self_as<tokotachi::GridLayout>(self, __func__)->row_spacing.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_grid_layout_set_row_spacing(tokotachi_grid_layout* self, float value)
{
    try
    {
        c::self_as<tokotachi::GridLayout>(self, __func__)->row_spacing = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API float tokotachi_grid_layout_get_column_spacing(tokotachi_grid_layout* self)
{
    try
    {
        return c::self_as<tokotachi::GridLayout>(self, __func__)->column_spacing.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_grid_layout_set_column_spacing(tokotachi_grid_layout* self, float value)
{
    try
    {
        c::self_as<tokotachi::GridLayout>(self, __func__)->column_spacing = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_grid_layout_add(tokotachi_grid_layout* self, tokotachi_widget* widget, int row, int column, int row_span, int column_span, tokotachi_align horizontal, tokotachi_align vertical)
{
    try
    {
        c::self_as<tokotachi::GridLayout>(self, __func__)->add(c::arg_as<tokotachi::Widget>(widget, __func__), row, column, row_span, column_span, c::enum_in<tokotachi::Align>(horizontal, 4), c::enum_in<tokotachi::Align>(vertical, 4));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_grid_layout_set_row_stretch(tokotachi_grid_layout* self, int row, int stretch)
{
    try
    {
        c::self_as<tokotachi::GridLayout>(self, __func__)->set_row_stretch(row, stretch);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_grid_layout_set_column_stretch(tokotachi_grid_layout* self, int column, int stretch)
{
    try
    {
        c::self_as<tokotachi::GridLayout>(self, __func__)->set_column_stretch(column, stretch);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_grid_layout_set_row_minimum(tokotachi_grid_layout* self, int row, float size)
{
    try
    {
        c::self_as<tokotachi::GridLayout>(self, __func__)->set_row_minimum(row, size);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_grid_layout_set_column_minimum(tokotachi_grid_layout* self, int column, float size)
{
    try
    {
        c::self_as<tokotachi::GridLayout>(self, __func__)->set_column_minimum(column, size);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_place_layout* tokotachi_place_layout_new(tokotachi_widget* target)
{
    try
    {
        return c::handle(new tokotachi::PlaceLayout(c::arg_as<tokotachi::Widget>(target, __func__)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_place_layout_add(tokotachi_place_layout* self, tokotachi_widget* widget, tokotachi_placement placement)
{
    try
    {
        c::self_as<tokotachi::PlaceLayout>(self, __func__)->add(c::arg_as<tokotachi::Widget>(widget, __func__), c::from_c(placement));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_action* tokotachi_action_new(tokotachi_object* owner, const char* text)
{
    try
    {
        return c::handle(new tokotachi::Action(c::arg_as<tokotachi::Object>(owner, __func__), c::str(text)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API char* tokotachi_action_get_text(tokotachi_action* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::Action>(self, __func__)->text.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_action_set_text(tokotachi_action* self, const char* value)
{
    try
    {
        c::self_as<tokotachi::Action>(self, __func__)->text = c::str(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_shortcut tokotachi_action_get_shortcut(tokotachi_action* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::Action>(self, __func__)->shortcut.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_shortcut())
}

TOKOTACHI_C_API void tokotachi_action_set_shortcut(tokotachi_action* self, tokotachi_shortcut value)
{
    try
    {
        c::self_as<tokotachi::Action>(self, __func__)->shortcut = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_image* tokotachi_action_get_icon(tokotachi_action* self)
{
    try
    {
        return c::to_c_image(c::self_as<tokotachi::Action>(self, __func__)->icon.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_action_set_icon(tokotachi_action* self, const tokotachi_image* value)
{
    try
    {
        c::self_as<tokotachi::Action>(self, __func__)->icon = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API char* tokotachi_action_get_tooltip(tokotachi_action* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::Action>(self, __func__)->tooltip.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_action_set_tooltip(tokotachi_action* self, const char* value)
{
    try
    {
        c::self_as<tokotachi::Action>(self, __func__)->tooltip = c::str(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_action_get_enabled(tokotachi_action* self)
{
    try
    {
        return c::self_as<tokotachi::Action>(self, __func__)->enabled.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_action_set_enabled(tokotachi_action* self, bool value)
{
    try
    {
        c::self_as<tokotachi::Action>(self, __func__)->enabled = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_action_get_checkable(tokotachi_action* self)
{
    try
    {
        return c::self_as<tokotachi::Action>(self, __func__)->checkable.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_action_set_checkable(tokotachi_action* self, bool value)
{
    try
    {
        c::self_as<tokotachi::Action>(self, __func__)->checkable = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_action_get_checked(tokotachi_action* self)
{
    try
    {
        return c::self_as<tokotachi::Action>(self, __func__)->checked.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_action_set_checked(tokotachi_action* self, bool value)
{
    try
    {
        c::self_as<tokotachi::Action>(self, __func__)->checked = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_action_set_on_trigger(tokotachi_action* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::Action>(self, __func__)->on_trigger = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_action_trigger(tokotachi_action* self)
{
    try
    {
        c::self_as<tokotachi::Action>(self, __func__)->trigger();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_menu* tokotachi_menu_new(tokotachi_object* owner, const char* title)
{
    try
    {
        return c::handle(new tokotachi::Menu(c::arg_as<tokotachi::Object>(owner, __func__), c::str(title)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API char* tokotachi_menu_get_title(tokotachi_menu* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::Menu>(self, __func__)->title.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_menu_set_title(tokotachi_menu* self, const char* value)
{
    try
    {
        c::self_as<tokotachi::Menu>(self, __func__)->title = c::str(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API size_t tokotachi_menu_get_actions(tokotachi_menu* self, tokotachi_action** out, size_t capacity)
{
    try
    {
        return c::fill_objects(c::self_as<tokotachi::Menu>(self, __func__)->actions.get(), out, capacity);
    }
    TOKOTACHI_C_CATCH(0)
}

TOKOTACHI_C_API tokotachi_action* tokotachi_menu_add_item(tokotachi_menu* self, const char* text)
{
    try
    {
        return c::handle(c::self_as<tokotachi::Menu>(self, __func__)->add_item(c::str(text)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_menu_add_action(tokotachi_menu* self, tokotachi_action* action)
{
    try
    {
        c::self_as<tokotachi::Menu>(self, __func__)->add_action(c::arg_as<tokotachi::Action>(action, __func__));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_menu_remove_action(tokotachi_menu* self, tokotachi_action* action)
{
    try
    {
        c::self_as<tokotachi::Menu>(self, __func__)->remove_action(c::arg_as<tokotachi::Action>(action, __func__));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_menu_add_separator(tokotachi_menu* self)
{
    try
    {
        c::self_as<tokotachi::Menu>(self, __func__)->add_separator();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_menu* tokotachi_menu_add_submenu(tokotachi_menu* self, const char* title)
{
    try
    {
        return c::handle(c::self_as<tokotachi::Menu>(self, __func__)->add_submenu(c::str(title)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_menu_popup(tokotachi_menu* self, tokotachi_widget* anchor_widget, tokotachi_point at)
{
    try
    {
        c::self_as<tokotachi::Menu>(self, __func__)->popup(c::arg_as<tokotachi::Widget>(anchor_widget, __func__), c::from_c(at));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_menu_close(tokotachi_menu* self)
{
    try
    {
        c::self_as<tokotachi::Menu>(self, __func__)->close();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_menu_get_opened(tokotachi_menu* self)
{
    try
    {
        return c::self_as<tokotachi::Menu>(self, __func__)->opened.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API int tokotachi_item_model_row_count(tokotachi_item_model* self, tokotachi_model_index parent)
{
    try
    {
        return c::self_as<tokotachi::ItemModel>(self, __func__)->row_count(c::from_c(parent));
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API tokotachi_model_index tokotachi_item_model_index(tokotachi_item_model* self, int row, tokotachi_model_index parent)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::ItemModel>(self, __func__)->index(row, c::from_c(parent)));
    }
    TOKOTACHI_C_CATCH(tokotachi_model_index())
}

TOKOTACHI_C_API tokotachi_model_index tokotachi_item_model_parent_of(tokotachi_item_model* self, tokotachi_model_index index)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::ItemModel>(self, __func__)->parent_of(c::from_c(index)));
    }
    TOKOTACHI_C_CATCH(tokotachi_model_index())
}

TOKOTACHI_C_API char* tokotachi_item_model_text(tokotachi_item_model* self, tokotachi_model_index index, int column)
{
    try
    {
        return c::dup(c::self_as<tokotachi::ItemModel>(self, __func__)->text(c::from_c(index), column));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_image* tokotachi_item_model_icon(tokotachi_item_model* self, tokotachi_model_index index)
{
    try
    {
        return c::to_c_image(c::self_as<tokotachi::ItemModel>(self, __func__)->icon(c::from_c(index)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API bool tokotachi_item_model_enabled(tokotachi_item_model* self, tokotachi_model_index index)
{
    try
    {
        return c::self_as<tokotachi::ItemModel>(self, __func__)->enabled(c::from_c(index));
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API bool tokotachi_item_model_checkable(tokotachi_item_model* self, tokotachi_model_index index)
{
    try
    {
        return c::self_as<tokotachi::ItemModel>(self, __func__)->checkable(c::from_c(index));
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API bool tokotachi_item_model_checked(tokotachi_item_model* self, tokotachi_model_index index)
{
    try
    {
        return c::self_as<tokotachi::ItemModel>(self, __func__)->checked(c::from_c(index));
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_item_model_set_checked(tokotachi_item_model* self, tokotachi_model_index index, bool checked)
{
    try
    {
        c::self_as<tokotachi::ItemModel>(self, __func__)->set_checked(c::from_c(index), checked);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_theme_color tokotachi_item_model_text_color(tokotachi_item_model* self, tokotachi_model_index index, int column)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::ItemModel>(self, __func__)->text_color(c::from_c(index), column));
    }
    TOKOTACHI_C_CATCH(tokotachi_theme_color())
}

TOKOTACHI_C_API tokotachi_font tokotachi_item_model_font(tokotachi_item_model* self, tokotachi_model_index index, int column)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::ItemModel>(self, __func__)->font(c::from_c(index), column));
    }
    TOKOTACHI_C_CATCH(tokotachi_font())
}

TOKOTACHI_C_API bool tokotachi_item_model_editable(tokotachi_item_model* self, tokotachi_model_index index, int column)
{
    try
    {
        return c::self_as<tokotachi::ItemModel>(self, __func__)->editable(c::from_c(index), column);
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_item_model_set_text(tokotachi_item_model* self, tokotachi_model_index index, int column, const char* text)
{
    try
    {
        c::self_as<tokotachi::ItemModel>(self, __func__)->set_text(c::from_c(index), column, c::str(text));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_item_model_can_move_rows(tokotachi_item_model* self, const tokotachi_model_index* indexes, size_t indexes_count, tokotachi_model_index parent, int row)
{
    try
    {
        return c::self_as<tokotachi::ItemModel>(self, __func__)->can_move_rows(c::indexes(indexes, indexes_count), c::from_c(parent), row);
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API bool tokotachi_item_model_move_rows(tokotachi_item_model* self, const tokotachi_model_index* indexes, size_t indexes_count, tokotachi_model_index parent, int row)
{
    try
    {
        return c::self_as<tokotachi::ItemModel>(self, __func__)->move_rows(c::indexes(indexes, indexes_count), c::from_c(parent), row);
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API bool tokotachi_item_model_can_drop(tokotachi_item_model* self, const tokotachi_drag_data* data, tokotachi_model_index parent, int row)
{
    try
    {
        return c::self_as<tokotachi::ItemModel>(self, __func__)->can_drop(c::drag_data_of(data), c::from_c(parent), row);
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API bool tokotachi_item_model_drop_data(tokotachi_item_model* self, const tokotachi_drag_data* data, tokotachi_drop_action action, tokotachi_model_index parent, int row)
{
    try
    {
        return c::self_as<tokotachi::ItemModel>(self, __func__)->drop_data(c::drag_data_of(data), c::enum_in<tokotachi::DropAction>(action, 3), c::from_c(parent), row);
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_item_model_sort(tokotachi_item_model* self, int column, tokotachi_sort_order order)
{
    try
    {
        c::self_as<tokotachi::ItemModel>(self, __func__)->sort(column, c::enum_in<tokotachi::SortOrder>(order, 2));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_model_index tokotachi_item_model_locate(tokotachi_item_model* self, tokotachi_model_index index)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::ItemModel>(self, __func__)->locate(c::from_c(index)));
    }
    TOKOTACHI_C_CATCH(tokotachi_model_index())
}

TOKOTACHI_C_API char* tokotachi_item_get_text(tokotachi_item* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::Item>(self, __func__)->text.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_item_set_text(tokotachi_item* self, const char* value)
{
    try
    {
        c::self_as<tokotachi::Item>(self, __func__)->text = c::str(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API char** tokotachi_item_get_column_texts(tokotachi_item* self)
{
    try
    {
        return c::dup_strv(c::self_as<tokotachi::Item>(self, __func__)->column_texts.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_item_set_column_texts(tokotachi_item* self, const char* const* value, size_t value_count)
{
    try
    {
        c::self_as<tokotachi::Item>(self, __func__)->column_texts = c::strv(value, value_count);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_image* tokotachi_item_get_icon(tokotachi_item* self)
{
    try
    {
        return c::to_c_image(c::self_as<tokotachi::Item>(self, __func__)->icon.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_item_set_icon(tokotachi_item* self, const tokotachi_image* value)
{
    try
    {
        c::self_as<tokotachi::Item>(self, __func__)->icon = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_item_get_enabled(tokotachi_item* self)
{
    try
    {
        return c::self_as<tokotachi::Item>(self, __func__)->enabled.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_item_set_enabled(tokotachi_item* self, bool value)
{
    try
    {
        c::self_as<tokotachi::Item>(self, __func__)->enabled = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_item_get_checkable(tokotachi_item* self)
{
    try
    {
        return c::self_as<tokotachi::Item>(self, __func__)->checkable.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_item_set_checkable(tokotachi_item* self, bool value)
{
    try
    {
        c::self_as<tokotachi::Item>(self, __func__)->checkable = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_item_get_checked(tokotachi_item* self)
{
    try
    {
        return c::self_as<tokotachi::Item>(self, __func__)->checked.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_item_set_checked(tokotachi_item* self, bool value)
{
    try
    {
        c::self_as<tokotachi::Item>(self, __func__)->checked = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_theme_color tokotachi_item_get_text_color(tokotachi_item* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::Item>(self, __func__)->text_color.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_theme_color())
}

TOKOTACHI_C_API void tokotachi_item_set_text_color(tokotachi_item* self, tokotachi_theme_color value)
{
    try
    {
        c::self_as<tokotachi::Item>(self, __func__)->text_color = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_font tokotachi_item_get_font(tokotachi_item* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::Item>(self, __func__)->font.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_font())
}

TOKOTACHI_C_API void tokotachi_item_set_font(tokotachi_item* self, tokotachi_font value)
{
    try
    {
        c::self_as<tokotachi::Item>(self, __func__)->font = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_item_get_editable(tokotachi_item* self)
{
    try
    {
        return c::self_as<tokotachi::Item>(self, __func__)->editable.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_item_set_editable(tokotachi_item* self, bool value)
{
    try
    {
        c::self_as<tokotachi::Item>(self, __func__)->editable = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API char* tokotachi_item_column_text(tokotachi_item* self, int column)
{
    try
    {
        return c::dup(c::self_as<tokotachi::Item>(self, __func__)->column_text(column));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_item_set_column_text(tokotachi_item* self, int column, const char* value)
{
    try
    {
        c::self_as<tokotachi::Item>(self, __func__)->set_column_text(column, c::str(value));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_list_model* tokotachi_list_model_new(tokotachi_object* owner)
{
    try
    {
        return c::handle(new tokotachi::ListModel(c::arg_as<tokotachi::Object>(owner, __func__)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API size_t tokotachi_list_model_get_items(tokotachi_list_model* self, tokotachi_item** out, size_t capacity)
{
    try
    {
        return c::fill_objects(c::self_as<tokotachi::ListModel>(self, __func__)->items.get(), out, capacity);
    }
    TOKOTACHI_C_CATCH(0)
}

TOKOTACHI_C_API tokotachi_item* tokotachi_list_model_add_item(tokotachi_list_model* self, const char* text)
{
    try
    {
        return c::handle(c::self_as<tokotachi::ListModel>(self, __func__)->add_item(c::str(text)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_item* tokotachi_list_model_insert_item(tokotachi_list_model* self, int index, const char* text)
{
    try
    {
        return c::handle(c::self_as<tokotachi::ListModel>(self, __func__)->insert_item(index, c::str(text)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_list_model_remove_item(tokotachi_list_model* self, tokotachi_item* item)
{
    try
    {
        c::self_as<tokotachi::ListModel>(self, __func__)->remove_item(c::arg_as<tokotachi::Item>(item, __func__));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_list_model_move_item(tokotachi_list_model* self, tokotachi_item* item, int index)
{
    try
    {
        c::self_as<tokotachi::ListModel>(self, __func__)->move_item(c::arg_as<tokotachi::Item>(item, __func__), index);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_list_model_clear(tokotachi_list_model* self)
{
    try
    {
        c::self_as<tokotachi::ListModel>(self, __func__)->clear();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_model_index tokotachi_list_model_index_of(tokotachi_list_model* self, tokotachi_item* item)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::ListModel>(self, __func__)->index_of(c::arg_as<tokotachi::Item>(item, __func__)));
    }
    TOKOTACHI_C_CATCH(tokotachi_model_index())
}

TOKOTACHI_C_API tokotachi_item* tokotachi_list_model_item_at(tokotachi_list_model* self, tokotachi_model_index index)
{
    try
    {
        return c::handle(c::self_as<tokotachi::ListModel>(self, __func__)->item_at(c::from_c(index)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_tree_item* tokotachi_tree_item_get_parent_item(tokotachi_tree_item* self)
{
    try
    {
        return c::handle(c::self_as<tokotachi::TreeItem>(self, __func__)->parent_item.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API size_t tokotachi_tree_item_get_children(tokotachi_tree_item* self, tokotachi_tree_item** out, size_t capacity)
{
    try
    {
        return c::fill_objects(c::self_as<tokotachi::TreeItem>(self, __func__)->children.get(), out, capacity);
    }
    TOKOTACHI_C_CATCH(0)
}

TOKOTACHI_C_API tokotachi_tree_item* tokotachi_tree_item_add_child(tokotachi_tree_item* self, const char* text)
{
    try
    {
        return c::handle(c::self_as<tokotachi::TreeItem>(self, __func__)->add_child(c::str(text)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_tree_item* tokotachi_tree_item_insert_child(tokotachi_tree_item* self, int index, const char* text)
{
    try
    {
        return c::handle(c::self_as<tokotachi::TreeItem>(self, __func__)->insert_child(index, c::str(text)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_tree_model* tokotachi_tree_model_new(tokotachi_object* owner)
{
    try
    {
        return c::handle(new tokotachi::TreeModel(c::arg_as<tokotachi::Object>(owner, __func__)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API size_t tokotachi_tree_model_get_items(tokotachi_tree_model* self, tokotachi_tree_item** out, size_t capacity)
{
    try
    {
        return c::fill_objects(c::self_as<tokotachi::TreeModel>(self, __func__)->items.get(), out, capacity);
    }
    TOKOTACHI_C_CATCH(0)
}

TOKOTACHI_C_API tokotachi_tree_item* tokotachi_tree_model_add_item(tokotachi_tree_model* self, const char* text)
{
    try
    {
        return c::handle(c::self_as<tokotachi::TreeModel>(self, __func__)->add_item(c::str(text)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_tree_item* tokotachi_tree_model_insert_item(tokotachi_tree_model* self, int index, const char* text)
{
    try
    {
        return c::handle(c::self_as<tokotachi::TreeModel>(self, __func__)->insert_item(index, c::str(text)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_tree_model_remove_item(tokotachi_tree_model* self, tokotachi_tree_item* item)
{
    try
    {
        c::self_as<tokotachi::TreeModel>(self, __func__)->remove_item(c::arg_as<tokotachi::TreeItem>(item, __func__));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_tree_model_move_item(tokotachi_tree_model* self, tokotachi_tree_item* item, tokotachi_tree_item* parent, int index)
{
    try
    {
        c::self_as<tokotachi::TreeModel>(self, __func__)->move_item(c::arg_as<tokotachi::TreeItem>(item, __func__), c::arg_as<tokotachi::TreeItem>(parent, __func__), index);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_tree_model_clear(tokotachi_tree_model* self)
{
    try
    {
        c::self_as<tokotachi::TreeModel>(self, __func__)->clear();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_model_index tokotachi_tree_model_index_of(tokotachi_tree_model* self, tokotachi_tree_item* item)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::TreeModel>(self, __func__)->index_of(c::arg_as<tokotachi::TreeItem>(item, __func__)));
    }
    TOKOTACHI_C_CATCH(tokotachi_model_index())
}

TOKOTACHI_C_API tokotachi_tree_item* tokotachi_tree_model_item_at(tokotachi_tree_model* self, tokotachi_model_index index)
{
    try
    {
        return c::handle(c::self_as<tokotachi::TreeModel>(self, __func__)->item_at(c::from_c(index)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API char* tokotachi_column_get_title(tokotachi_column* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::Column>(self, __func__)->title.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_column_set_title(tokotachi_column* self, const char* value)
{
    try
    {
        c::self_as<tokotachi::Column>(self, __func__)->title = c::str(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API float tokotachi_column_get_width(tokotachi_column* self)
{
    try
    {
        return c::self_as<tokotachi::Column>(self, __func__)->width.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_column_set_width(tokotachi_column* self, float value)
{
    try
    {
        c::self_as<tokotachi::Column>(self, __func__)->width = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_align tokotachi_column_get_alignment(tokotachi_column* self)
{
    try
    {
        return static_cast<tokotachi_align>(c::self_as<tokotachi::Column>(self, __func__)->alignment.get());
    }
    TOKOTACHI_C_CATCH(static_cast<tokotachi_align>(0))
}

TOKOTACHI_C_API void tokotachi_column_set_alignment(tokotachi_column* self, tokotachi_align value)
{
    try
    {
        c::self_as<tokotachi::Column>(self, __func__)->alignment = c::enum_in<tokotachi::Align>(value, 4);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_item_model* tokotachi_item_view_get_model(tokotachi_item_view* self)
{
    try
    {
        return c::handle(c::self_as<tokotachi::ItemView>(self, __func__)->model.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_item_view_set_model(tokotachi_item_view* self, tokotachi_item_model* value)
{
    try
    {
        c::self_as<tokotachi::ItemView>(self, __func__)->model = c::arg_as<tokotachi::ItemModel>(value, __func__);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API size_t tokotachi_item_view_get_columns(tokotachi_item_view* self, tokotachi_column** out, size_t capacity)
{
    try
    {
        return c::fill_objects(c::self_as<tokotachi::ItemView>(self, __func__)->columns.get(), out, capacity);
    }
    TOKOTACHI_C_CATCH(0)
}

TOKOTACHI_C_API bool tokotachi_item_view_get_header_visible(tokotachi_item_view* self)
{
    try
    {
        return c::self_as<tokotachi::ItemView>(self, __func__)->header_visible.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_item_view_set_header_visible(tokotachi_item_view* self, bool value)
{
    try
    {
        c::self_as<tokotachi::ItemView>(self, __func__)->header_visible = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_item_view_get_sorting_enabled(tokotachi_item_view* self)
{
    try
    {
        return c::self_as<tokotachi::ItemView>(self, __func__)->sorting_enabled.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_item_view_set_sorting_enabled(tokotachi_item_view* self, bool value)
{
    try
    {
        c::self_as<tokotachi::ItemView>(self, __func__)->sorting_enabled = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API int tokotachi_item_view_get_sort_column(tokotachi_item_view* self)
{
    try
    {
        return c::self_as<tokotachi::ItemView>(self, __func__)->sort_column.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_item_view_set_sort_column(tokotachi_item_view* self, int value)
{
    try
    {
        c::self_as<tokotachi::ItemView>(self, __func__)->sort_column = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_sort_order tokotachi_item_view_get_sort_order(tokotachi_item_view* self)
{
    try
    {
        return static_cast<tokotachi_sort_order>(c::self_as<tokotachi::ItemView>(self, __func__)->sort_order.get());
    }
    TOKOTACHI_C_CATCH(static_cast<tokotachi_sort_order>(0))
}

TOKOTACHI_C_API void tokotachi_item_view_set_sort_order(tokotachi_item_view* self, tokotachi_sort_order value)
{
    try
    {
        c::self_as<tokotachi::ItemView>(self, __func__)->sort_order = c::enum_in<tokotachi::SortOrder>(value, 2);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_selection_mode tokotachi_item_view_get_selection_mode(tokotachi_item_view* self)
{
    try
    {
        return static_cast<tokotachi_selection_mode>(c::self_as<tokotachi::ItemView>(self, __func__)->selection_mode.get());
    }
    TOKOTACHI_C_CATCH(static_cast<tokotachi_selection_mode>(0))
}

TOKOTACHI_C_API void tokotachi_item_view_set_selection_mode(tokotachi_item_view* self, tokotachi_selection_mode value)
{
    try
    {
        c::self_as<tokotachi::ItemView>(self, __func__)->selection_mode = c::enum_in<tokotachi::SelectionMode>(value, 3);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_model_index tokotachi_item_view_get_current(tokotachi_item_view* self)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::ItemView>(self, __func__)->current.get());
    }
    TOKOTACHI_C_CATCH(tokotachi_model_index())
}

TOKOTACHI_C_API void tokotachi_item_view_set_current(tokotachi_item_view* self, tokotachi_model_index value)
{
    try
    {
        c::self_as<tokotachi::ItemView>(self, __func__)->current = c::from_c(value);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API size_t tokotachi_item_view_get_selection(tokotachi_item_view* self, tokotachi_model_index* out, size_t capacity)
{
    try
    {
        return c::fill_indexes(c::self_as<tokotachi::ItemView>(self, __func__)->selection.get(), out, capacity);
    }
    TOKOTACHI_C_CATCH(0)
}

TOKOTACHI_C_API bool tokotachi_item_view_get_editing(tokotachi_item_view* self)
{
    try
    {
        return c::self_as<tokotachi::ItemView>(self, __func__)->editing.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API bool tokotachi_item_view_get_drag_enabled(tokotachi_item_view* self)
{
    try
    {
        return c::self_as<tokotachi::ItemView>(self, __func__)->drag_enabled.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_item_view_set_drag_enabled(tokotachi_item_view* self, bool value)
{
    try
    {
        c::self_as<tokotachi::ItemView>(self, __func__)->drag_enabled = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_item_view_get_drop_enabled(tokotachi_item_view* self)
{
    try
    {
        return c::self_as<tokotachi::ItemView>(self, __func__)->drop_enabled.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_item_view_set_drop_enabled(tokotachi_item_view* self, bool value)
{
    try
    {
        c::self_as<tokotachi::ItemView>(self, __func__)->drop_enabled = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_item_view_set_on_selection_change(tokotachi_item_view* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::ItemView>(self, __func__)->on_selection_change = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_item_view_set_on_activate(tokotachi_item_view* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::ItemView>(self, __func__)->on_activate = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_column* tokotachi_item_view_add_column(tokotachi_item_view* self, const char* title, float width)
{
    try
    {
        return c::handle(c::self_as<tokotachi::ItemView>(self, __func__)->add_column(c::str(title), width));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_item_view_remove_column(tokotachi_item_view* self, tokotachi_column* column)
{
    try
    {
        c::self_as<tokotachi::ItemView>(self, __func__)->remove_column(c::arg_as<tokotachi::Column>(column, __func__));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_item_view_is_selected(tokotachi_item_view* self, tokotachi_model_index index)
{
    try
    {
        return c::self_as<tokotachi::ItemView>(self, __func__)->is_selected(c::from_c(index));
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_item_view_select(tokotachi_item_view* self, tokotachi_model_index index, bool selected)
{
    try
    {
        c::self_as<tokotachi::ItemView>(self, __func__)->select(c::from_c(index), selected);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_item_view_select_all(tokotachi_item_view* self)
{
    try
    {
        c::self_as<tokotachi::ItemView>(self, __func__)->select_all();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_item_view_clear_selection(tokotachi_item_view* self)
{
    try
    {
        c::self_as<tokotachi::ItemView>(self, __func__)->clear_selection();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_item_view_scroll_to(tokotachi_item_view* self, tokotachi_model_index index)
{
    try
    {
        c::self_as<tokotachi::ItemView>(self, __func__)->scroll_to(c::from_c(index));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_model_index tokotachi_item_view_index_at(tokotachi_item_view* self, tokotachi_point p)
{
    try
    {
        return c::to_c(c::self_as<tokotachi::ItemView>(self, __func__)->index_at(c::from_c(p)));
    }
    TOKOTACHI_C_CATCH(tokotachi_model_index())
}

TOKOTACHI_C_API void tokotachi_item_view_edit(tokotachi_item_view* self, tokotachi_model_index index, int column)
{
    try
    {
        c::self_as<tokotachi::ItemView>(self, __func__)->edit(c::from_c(index), column);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_item_view_finish_editing(tokotachi_item_view* self, bool commit)
{
    try
    {
        c::self_as<tokotachi::ItemView>(self, __func__)->finish_editing(commit);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_list_view* tokotachi_list_view_new(tokotachi_widget* parent)
{
    try
    {
        return c::handle(new tokotachi::ListView(c::arg_as<tokotachi::Widget>(parent, __func__)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_table_view* tokotachi_table_view_new(tokotachi_widget* parent)
{
    try
    {
        return c::handle(new tokotachi::TableView(c::arg_as<tokotachi::Widget>(parent, __func__)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API tokotachi_selection_behavior tokotachi_table_view_get_selection_behavior(tokotachi_table_view* self)
{
    try
    {
        return static_cast<tokotachi_selection_behavior>(c::self_as<tokotachi::TableView>(self, __func__)->selection_behavior.get());
    }
    TOKOTACHI_C_CATCH(static_cast<tokotachi_selection_behavior>(0))
}

TOKOTACHI_C_API void tokotachi_table_view_set_selection_behavior(tokotachi_table_view* self, tokotachi_selection_behavior value)
{
    try
    {
        c::self_as<tokotachi::TableView>(self, __func__)->selection_behavior = c::enum_in<tokotachi::SelectionBehavior>(value, 2);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API int tokotachi_table_view_get_current_column(tokotachi_table_view* self)
{
    try
    {
        return c::self_as<tokotachi::TableView>(self, __func__)->current_column.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_table_view_set_current_column(tokotachi_table_view* self, int value)
{
    try
    {
        c::self_as<tokotachi::TableView>(self, __func__)->current_column = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API size_t tokotachi_table_view_get_selected_ranges(tokotachi_table_view* self, tokotachi_cell_range* out, size_t capacity)
{
    try
    {
        return c::fill_ranges(c::self_as<tokotachi::TableView>(self, __func__)->selected_ranges.get(), out, capacity);
    }
    TOKOTACHI_C_CATCH(0)
}

TOKOTACHI_C_API bool tokotachi_table_view_get_grid_visible(tokotachi_table_view* self)
{
    try
    {
        return c::self_as<tokotachi::TableView>(self, __func__)->grid_visible.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_table_view_set_grid_visible(tokotachi_table_view* self, bool value)
{
    try
    {
        c::self_as<tokotachi::TableView>(self, __func__)->grid_visible = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_table_view_get_row_header_visible(tokotachi_table_view* self)
{
    try
    {
        return c::self_as<tokotachi::TableView>(self, __func__)->row_header_visible.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_table_view_set_row_header_visible(tokotachi_table_view* self, bool value)
{
    try
    {
        c::self_as<tokotachi::TableView>(self, __func__)->row_header_visible = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_table_view_is_cell_selected(tokotachi_table_view* self, int row, int column)
{
    try
    {
        return c::self_as<tokotachi::TableView>(self, __func__)->is_cell_selected(row, column);
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_table_view_select_range(tokotachi_table_view* self, tokotachi_cell_range range, bool add)
{
    try
    {
        c::self_as<tokotachi::TableView>(self, __func__)->select_range(c::from_c(range), add);
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API char* tokotachi_table_view_selected_text(tokotachi_table_view* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::TableView>(self, __func__)->selected_text());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_table_view_copy(tokotachi_table_view* self)
{
    try
    {
        c::self_as<tokotachi::TableView>(self, __func__)->copy();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_table_view_paste(tokotachi_table_view* self)
{
    try
    {
        c::self_as<tokotachi::TableView>(self, __func__)->paste();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_table_view_clear_selected_cells(tokotachi_table_view* self)
{
    try
    {
        c::self_as<tokotachi::TableView>(self, __func__)->clear_selected_cells();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API tokotachi_tree_view* tokotachi_tree_view_new(tokotachi_widget* parent)
{
    try
    {
        return c::handle(new tokotachi::TreeView(c::arg_as<tokotachi::Widget>(parent, __func__)));
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API float tokotachi_tree_view_get_indentation(tokotachi_tree_view* self)
{
    try
    {
        return c::self_as<tokotachi::TreeView>(self, __func__)->indentation.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_tree_view_set_indentation(tokotachi_tree_view* self, float value)
{
    try
    {
        c::self_as<tokotachi::TreeView>(self, __func__)->indentation = value;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_tree_view_expand(tokotachi_tree_view* self, tokotachi_model_index index)
{
    try
    {
        c::self_as<tokotachi::TreeView>(self, __func__)->expand(c::from_c(index));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_tree_view_collapse(tokotachi_tree_view* self, tokotachi_model_index index)
{
    try
    {
        c::self_as<tokotachi::TreeView>(self, __func__)->collapse(c::from_c(index));
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_tree_view_is_expanded(tokotachi_tree_view* self, tokotachi_model_index index)
{
    try
    {
        return c::self_as<tokotachi::TreeView>(self, __func__)->is_expanded(c::from_c(index));
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API void tokotachi_tree_view_expand_all(tokotachi_tree_view* self)
{
    try
    {
        c::self_as<tokotachi::TreeView>(self, __func__)->expand_all();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_tree_view_collapse_all(tokotachi_tree_view* self)
{
    try
    {
        c::self_as<tokotachi::TreeView>(self, __func__)->collapse_all();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API bool tokotachi_task_get_running(tokotachi_task* self)
{
    try
    {
        return c::self_as<tokotachi::TaskBase>(self, __func__)->running.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API double tokotachi_task_get_progress(tokotachi_task* self)
{
    try
    {
        return c::self_as<tokotachi::TaskBase>(self, __func__)->progress.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API bool tokotachi_task_get_cancelled(tokotachi_task* self)
{
    try
    {
        return c::self_as<tokotachi::TaskBase>(self, __func__)->cancelled.get();
    }
    TOKOTACHI_C_CATCH({})
}

TOKOTACHI_C_API char* tokotachi_task_get_error(tokotachi_task* self)
{
    try
    {
        return c::dup(c::self_as<tokotachi::TaskBase>(self, __func__)->error.get());
    }
    TOKOTACHI_C_CATCH(nullptr)
}

TOKOTACHI_C_API void tokotachi_task_set_on_progress(tokotachi_task* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::TaskBase>(self, __func__)->on_progress = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_task_set_on_finish(tokotachi_task* self, tokotachi_notify_fn fn, void* user_data, tokotachi_destroy_fn destroy)
{
    try
    {
        tokotachi::NotifyHandler handler = c::notify_handler(fn, user_data, destroy);
        c::self_as<tokotachi::TaskBase>(self, __func__)->on_finish = handler;
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API void tokotachi_task_cancel(tokotachi_task* self)
{
    try
    {
        c::self_as<tokotachi::TaskBase>(self, __func__)->cancel();
    }
    TOKOTACHI_C_CATCH()
}

TOKOTACHI_C_API const char* tokotachi_object_class_name(tokotachi_object* object)
{
    tokotachi::Object* o = reinterpret_cast<tokotachi::Object*>(object);
    if (!o)
    {
        return nullptr;
    }
    if (dynamic_cast<tokotachi::MessageDialog*>(o))
    {
        return "MessageDialog";
    }
    if (dynamic_cast<tokotachi::ColorDialog*>(o))
    {
        return "ColorDialog";
    }
    if (dynamic_cast<tokotachi::Button*>(o))
    {
        return "Button";
    }
    if (dynamic_cast<tokotachi::CheckBox*>(o))
    {
        return "CheckBox";
    }
    if (dynamic_cast<tokotachi::RadioButton*>(o))
    {
        return "RadioButton";
    }
    if (dynamic_cast<tokotachi::LineEdit*>(o))
    {
        return "LineEdit";
    }
    if (dynamic_cast<tokotachi::TextEdit*>(o))
    {
        return "TextEdit";
    }
    if (dynamic_cast<tokotachi::Slider*>(o))
    {
        return "Slider";
    }
    if (dynamic_cast<tokotachi::SpinBox*>(o))
    {
        return "SpinBox";
    }
    if (dynamic_cast<tokotachi::ColorButton*>(o))
    {
        return "ColorButton";
    }
    if (dynamic_cast<tokotachi::Dialog*>(o))
    {
        return "Dialog";
    }
    if (dynamic_cast<tokotachi::Popup*>(o))
    {
        return "Popup";
    }
    if (dynamic_cast<tokotachi::ListView*>(o))
    {
        return "ListView";
    }
    if (dynamic_cast<tokotachi::TableView*>(o))
    {
        return "TableView";
    }
    if (dynamic_cast<tokotachi::TreeView*>(o))
    {
        return "TreeView";
    }
    if (dynamic_cast<tokotachi::ButtonBase*>(o))
    {
        return "ButtonBase";
    }
    if (dynamic_cast<tokotachi::ComboBox*>(o))
    {
        return "ComboBox";
    }
    if (dynamic_cast<tokotachi::Label*>(o))
    {
        return "Label";
    }
    if (dynamic_cast<tokotachi::TextInputBase*>(o))
    {
        return "TextInputBase";
    }
    if (dynamic_cast<tokotachi::PaintBox*>(o))
    {
        return "PaintBox";
    }
    if (dynamic_cast<tokotachi::ProgressBar*>(o))
    {
        return "ProgressBar";
    }
    if (dynamic_cast<tokotachi::RangeBase*>(o))
    {
        return "RangeBase";
    }
    if (dynamic_cast<tokotachi::ScrollArea*>(o))
    {
        return "ScrollArea";
    }
    if (dynamic_cast<tokotachi::ScrollBar*>(o))
    {
        return "ScrollBar";
    }
    if (dynamic_cast<tokotachi::Splitter*>(o))
    {
        return "Splitter";
    }
    if (dynamic_cast<tokotachi::TabWidget*>(o))
    {
        return "TabWidget";
    }
    if (dynamic_cast<tokotachi::GroupBox*>(o))
    {
        return "GroupBox";
    }
    if (dynamic_cast<tokotachi::ImageBox*>(o))
    {
        return "ImageBox";
    }
    if (dynamic_cast<tokotachi::Calendar*>(o))
    {
        return "Calendar";
    }
    if (dynamic_cast<tokotachi::DateEdit*>(o))
    {
        return "DateEdit";
    }
    if (dynamic_cast<tokotachi::Window*>(o))
    {
        return "Window";
    }
    if (dynamic_cast<tokotachi::MenuBar*>(o))
    {
        return "MenuBar";
    }
    if (dynamic_cast<tokotachi::StatusBar*>(o))
    {
        return "StatusBar";
    }
    if (dynamic_cast<tokotachi::ToolBar*>(o))
    {
        return "ToolBar";
    }
    if (dynamic_cast<tokotachi::BoxLayout*>(o))
    {
        return "BoxLayout";
    }
    if (dynamic_cast<tokotachi::GridLayout*>(o))
    {
        return "GridLayout";
    }
    if (dynamic_cast<tokotachi::PlaceLayout*>(o))
    {
        return "PlaceLayout";
    }
    if (dynamic_cast<tokotachi::ListModel*>(o))
    {
        return "ListModel";
    }
    if (dynamic_cast<tokotachi::TreeItem*>(o))
    {
        return "TreeItem";
    }
    if (dynamic_cast<tokotachi::TreeModel*>(o))
    {
        return "TreeModel";
    }
    if (dynamic_cast<tokotachi::ItemView*>(o))
    {
        return "ItemView";
    }
    if (dynamic_cast<c::CallbackModel*>(o))
    {
        return "CustomModel";
    }
    if (dynamic_cast<tokotachi::Application*>(o))
    {
        return "Application";
    }
    if (dynamic_cast<tokotachi::Timer*>(o))
    {
        return "Timer";
    }
    if (dynamic_cast<tokotachi::Widget*>(o))
    {
        return "Widget";
    }
    if (dynamic_cast<tokotachi::SplitterPane*>(o))
    {
        return "SplitterPane";
    }
    if (dynamic_cast<tokotachi::Tab*>(o))
    {
        return "Tab";
    }
    if (dynamic_cast<tokotachi::FileDialog*>(o))
    {
        return "FileDialog";
    }
    if (dynamic_cast<tokotachi::Layout*>(o))
    {
        return "Layout";
    }
    if (dynamic_cast<tokotachi::Action*>(o))
    {
        return "Action";
    }
    if (dynamic_cast<tokotachi::Menu*>(o))
    {
        return "Menu";
    }
    if (dynamic_cast<tokotachi::ItemModel*>(o))
    {
        return "ItemModel";
    }
    if (dynamic_cast<tokotachi::Item*>(o))
    {
        return "Item";
    }
    if (dynamic_cast<tokotachi::Column*>(o))
    {
        return "Column";
    }
    if (c::is_variable(o))
    {
        return "Variable";
    }
    if (dynamic_cast<tokotachi::TaskBase*>(o))
    {
        return "Task";
    }
    return "Object";
}

}  // extern "C"

void tokotachi::c::register_generated_properties()
{
    TOKOTACHI_C_ADD_PROPERTY("color_scheme", &tokotachi::Application::color_scheme);
    TOKOTACHI_C_ADD_PROPERTY("light_theme", &tokotachi::Application::light_theme);
    TOKOTACHI_C_ADD_PROPERTY("dark_theme", &tokotachi::Application::dark_theme);
    TOKOTACHI_C_ADD_PROPERTY("dark", &tokotachi::Application::dark);
    TOKOTACHI_C_ADD_PROPERTY("theme", &tokotachi::Application::theme);
    TOKOTACHI_C_ADD_PROPERTY("language", &tokotachi::Application::language);
    TOKOTACHI_C_ADD_PROPERTY("auto_quit_enabled", &tokotachi::Application::auto_quit_enabled);
    TOKOTACHI_C_ADD_PROPERTY("interval", &tokotachi::Timer::interval);
    TOKOTACHI_C_ADD_PROPERTY("repeat", &tokotachi::Timer::repeat);
    TOKOTACHI_C_ADD_PROPERTY("enabled", &tokotachi::Timer::enabled);
    TOKOTACHI_C_ADD_PROPERTY("x", &tokotachi::Widget::x);
    TOKOTACHI_C_ADD_PROPERTY("y", &tokotachi::Widget::y);
    TOKOTACHI_C_ADD_PROPERTY("width", &tokotachi::Widget::width);
    TOKOTACHI_C_ADD_PROPERTY("height", &tokotachi::Widget::height);
    TOKOTACHI_C_ADD_PROPERTY("bounds", &tokotachi::Widget::bounds);
    TOKOTACHI_C_ADD_PROPERTY("local_rect", &tokotachi::Widget::local_rect);
    TOKOTACHI_C_ADD_PROPERTY("preferred_size", &tokotachi::Widget::preferred_size);
    TOKOTACHI_C_ADD_PROPERTY("minimum_size", &tokotachi::Widget::minimum_size);
    TOKOTACHI_C_ADD_PROPERTY("maximum_size", &tokotachi::Widget::maximum_size);
    TOKOTACHI_C_ADD_PROPERTY("minimum_hint", &tokotachi::Widget::minimum_hint);
    TOKOTACHI_C_ADD_PROPERTY("visible", &tokotachi::Widget::visible);
    TOKOTACHI_C_ADD_PROPERTY("showing", &tokotachi::Widget::showing);
    TOKOTACHI_C_ADD_PROPERTY("enabled", &tokotachi::Widget::enabled);
    TOKOTACHI_C_ADD_PROPERTY("effectively_enabled", &tokotachi::Widget::effectively_enabled);
    TOKOTACHI_C_ADD_PROPERTY("focus_policy", &tokotachi::Widget::focus_policy);
    TOKOTACHI_C_ADD_PROPERTY("focused", &tokotachi::Widget::focused);
    TOKOTACHI_C_ADD_PROPERTY("cursor", &tokotachi::Widget::cursor);
    TOKOTACHI_C_ADD_PROPERTY("accepts_text_input", &tokotachi::Widget::accepts_text_input);
    TOKOTACHI_C_ADD_PROPERTY("display_scale", &tokotachi::Widget::display_scale);
    TOKOTACHI_C_ADD_PROPERTY("tooltip", &tokotachi::Widget::tooltip);
    TOKOTACHI_C_ADD_PROPERTY("theme_override", &tokotachi::Widget::theme_override);
    TOKOTACHI_C_ADD_PROPERTY("theme", &tokotachi::Widget::theme);
    TOKOTACHI_C_ADD_PROPERTY("accepts_drops", &tokotachi::Widget::accepts_drops);
    TOKOTACHI_C_ADD_PROPERTY("text", &tokotachi::ButtonBase::text);
    TOKOTACHI_C_ADD_PROPERTY("font", &tokotachi::ButtonBase::font);
    TOKOTACHI_C_ADD_PROPERTY("style", &tokotachi::Button::style);
    TOKOTACHI_C_ADD_PROPERTY("icon_size", &tokotachi::Button::icon_size);
    TOKOTACHI_C_ADD_PROPERTY("checked", &tokotachi::CheckBox::checked);
    TOKOTACHI_C_ADD_PROPERTY("value", &tokotachi::RadioButton::value);
    TOKOTACHI_C_ADD_PROPERTY("group_value", &tokotachi::RadioButton::group_value);
    TOKOTACHI_C_ADD_PROPERTY("checked", &tokotachi::RadioButton::checked);
    TOKOTACHI_C_ADD_PROPERTY("items", &tokotachi::ComboBox::items);
    TOKOTACHI_C_ADD_PROPERTY("current_index", &tokotachi::ComboBox::current_index);
    TOKOTACHI_C_ADD_PROPERTY("current_text", &tokotachi::ComboBox::current_text);
    TOKOTACHI_C_ADD_PROPERTY("placeholder_text", &tokotachi::ComboBox::placeholder_text);
    TOKOTACHI_C_ADD_PROPERTY("font", &tokotachi::ComboBox::font);
    TOKOTACHI_C_ADD_PROPERTY("popup_open", &tokotachi::ComboBox::popup_open);
    TOKOTACHI_C_ADD_PROPERTY("text", &tokotachi::Label::text);
    TOKOTACHI_C_ADD_PROPERTY("font", &tokotachi::Label::font);
    TOKOTACHI_C_ADD_PROPERTY("text_color", &tokotachi::Label::text_color);
    TOKOTACHI_C_ADD_PROPERTY("alignment", &tokotachi::Label::alignment);
    TOKOTACHI_C_ADD_PROPERTY("word_wrap", &tokotachi::Label::word_wrap);
    TOKOTACHI_C_ADD_PROPERTY("icon_size", &tokotachi::Label::icon_size);
    TOKOTACHI_C_ADD_PROPERTY("text", &tokotachi::TextInputBase::text);
    TOKOTACHI_C_ADD_PROPERTY("placeholder_text", &tokotachi::TextInputBase::placeholder_text);
    TOKOTACHI_C_ADD_PROPERTY("font", &tokotachi::TextInputBase::font);
    TOKOTACHI_C_ADD_PROPERTY("cursor_position", &tokotachi::TextInputBase::cursor_position);
    TOKOTACHI_C_ADD_PROPERTY("has_selection", &tokotachi::TextInputBase::has_selection);
    TOKOTACHI_C_ADD_PROPERTY("selected_text", &tokotachi::TextInputBase::selected_text);
    TOKOTACHI_C_ADD_PROPERTY("can_undo", &tokotachi::TextInputBase::can_undo);
    TOKOTACHI_C_ADD_PROPERTY("can_redo", &tokotachi::TextInputBase::can_redo);
    TOKOTACHI_C_ADD_PROPERTY("word_wrap", &tokotachi::TextEdit::word_wrap);
    TOKOTACHI_C_ADD_PROPERTY("cached", &tokotachi::PaintBox::cached);
    TOKOTACHI_C_ADD_PROPERTY("minimum", &tokotachi::ProgressBar::minimum);
    TOKOTACHI_C_ADD_PROPERTY("maximum", &tokotachi::ProgressBar::maximum);
    TOKOTACHI_C_ADD_PROPERTY("value", &tokotachi::ProgressBar::value);
    TOKOTACHI_C_ADD_PROPERTY("text_visible", &tokotachi::ProgressBar::text_visible);
    TOKOTACHI_C_ADD_PROPERTY("indeterminate", &tokotachi::ProgressBar::indeterminate);
    TOKOTACHI_C_ADD_PROPERTY("minimum", &tokotachi::RangeBase::minimum);
    TOKOTACHI_C_ADD_PROPERTY("maximum", &tokotachi::RangeBase::maximum);
    TOKOTACHI_C_ADD_PROPERTY("value", &tokotachi::RangeBase::value);
    TOKOTACHI_C_ADD_PROPERTY("single_step", &tokotachi::RangeBase::single_step);
    TOKOTACHI_C_ADD_PROPERTY("page_step", &tokotachi::RangeBase::page_step);
    TOKOTACHI_C_ADD_PROPERTY("orientation", &tokotachi::Slider::orientation);
    TOKOTACHI_C_ADD_PROPERTY("tick_interval", &tokotachi::Slider::tick_interval);
    TOKOTACHI_C_ADD_PROPERTY("decimals", &tokotachi::SpinBox::decimals);
    TOKOTACHI_C_ADD_PROPERTY("prefix", &tokotachi::SpinBox::prefix);
    TOKOTACHI_C_ADD_PROPERTY("suffix", &tokotachi::SpinBox::suffix);
    TOKOTACHI_C_ADD_PROPERTY("text", &tokotachi::SpinBox::text);
    TOKOTACHI_C_ADD_PROPERTY("fit_width", &tokotachi::ScrollArea::fit_width);
    TOKOTACHI_C_ADD_PROPERTY("scroll_position", &tokotachi::ScrollArea::scroll_position);
    TOKOTACHI_C_ADD_PROPERTY("orientation", &tokotachi::ScrollBar::orientation);
    TOKOTACHI_C_ADD_PROPERTY("value", &tokotachi::ScrollBar::value);
    TOKOTACHI_C_ADD_PROPERTY("maximum", &tokotachi::ScrollBar::maximum);
    TOKOTACHI_C_ADD_PROPERTY("page_step", &tokotachi::ScrollBar::page_step);
    TOKOTACHI_C_ADD_PROPERTY("single_step", &tokotachi::ScrollBar::single_step);
    TOKOTACHI_C_ADD_PROPERTY("stretch", &tokotachi::SplitterPane::stretch);
    TOKOTACHI_C_ADD_PROPERTY("collapsible", &tokotachi::SplitterPane::collapsible);
    TOKOTACHI_C_ADD_PROPERTY("collapsed", &tokotachi::SplitterPane::collapsed);
    TOKOTACHI_C_ADD_PROPERTY("orientation", &tokotachi::Splitter::orientation);
    TOKOTACHI_C_ADD_PROPERTY("sizes", &tokotachi::Splitter::sizes);
    TOKOTACHI_C_ADD_PROPERTY("title", &tokotachi::Tab::title);
    TOKOTACHI_C_ADD_PROPERTY("enabled", &tokotachi::Tab::enabled);
    TOKOTACHI_C_ADD_PROPERTY("current_index", &tokotachi::TabWidget::current_index);
    TOKOTACHI_C_ADD_PROPERTY("title", &tokotachi::GroupBox::title);
    TOKOTACHI_C_ADD_PROPERTY("content_rect", &tokotachi::GroupBox::content_rect);
    TOKOTACHI_C_ADD_PROPERTY("fit", &tokotachi::ImageBox::fit);
    TOKOTACHI_C_ADD_PROPERTY("date", &tokotachi::Calendar::date);
    TOKOTACHI_C_ADD_PROPERTY("minimum", &tokotachi::Calendar::minimum);
    TOKOTACHI_C_ADD_PROPERTY("maximum", &tokotachi::Calendar::maximum);
    TOKOTACHI_C_ADD_PROPERTY("first_day_of_week", &tokotachi::Calendar::first_day_of_week);
    TOKOTACHI_C_ADD_PROPERTY("shown_year", &tokotachi::Calendar::shown_year);
    TOKOTACHI_C_ADD_PROPERTY("shown_month", &tokotachi::Calendar::shown_month);
    TOKOTACHI_C_ADD_PROPERTY("date", &tokotachi::DateEdit::date);
    TOKOTACHI_C_ADD_PROPERTY("minimum", &tokotachi::DateEdit::minimum);
    TOKOTACHI_C_ADD_PROPERTY("maximum", &tokotachi::DateEdit::maximum);
    TOKOTACHI_C_ADD_PROPERTY("format", &tokotachi::DateEdit::format);
    TOKOTACHI_C_ADD_PROPERTY("first_day_of_week", &tokotachi::DateEdit::first_day_of_week);
    TOKOTACHI_C_ADD_PROPERTY("placeholder_text", &tokotachi::DateEdit::placeholder_text);
    TOKOTACHI_C_ADD_PROPERTY("text", &tokotachi::DateEdit::text);
    TOKOTACHI_C_ADD_PROPERTY("calendar_open", &tokotachi::DateEdit::calendar_open);
    TOKOTACHI_C_ADD_PROPERTY("color", &tokotachi::ColorButton::color);
    TOKOTACHI_C_ADD_PROPERTY("alpha_enabled", &tokotachi::ColorButton::alpha_enabled);
    TOKOTACHI_C_ADD_PROPERTY("dialog_title", &tokotachi::ColorButton::dialog_title);
    TOKOTACHI_C_ADD_PROPERTY("title", &tokotachi::Window::title);
    TOKOTACHI_C_ADD_PROPERTY("background", &tokotachi::Window::background);
    TOKOTACHI_C_ADD_PROPERTY("destroy_on_close", &tokotachi::Window::destroy_on_close);
    TOKOTACHI_C_ADD_PROPERTY("result", &tokotachi::Dialog::result);
    TOKOTACHI_C_ADD_PROPERTY("text", &tokotachi::MessageDialog::text);
    TOKOTACHI_C_ADD_PROPERTY("type", &tokotachi::MessageDialog::type);
    TOKOTACHI_C_ADD_PROPERTY("color", &tokotachi::ColorDialog::color);
    TOKOTACHI_C_ADD_PROPERTY("alpha_enabled", &tokotachi::ColorDialog::alpha_enabled);
    TOKOTACHI_C_ADD_PROPERTY("mode", &tokotachi::FileDialog::mode);
    TOKOTACHI_C_ADD_PROPERTY("title", &tokotachi::FileDialog::title);
    TOKOTACHI_C_ADD_PROPERTY("location", &tokotachi::FileDialog::location);
    TOKOTACHI_C_ADD_PROPERTY("multi_enabled", &tokotachi::FileDialog::multi_enabled);
    TOKOTACHI_C_ADD_PROPERTY("paths", &tokotachi::FileDialog::paths);
    TOKOTACHI_C_ADD_PROPERTY("error", &tokotachi::FileDialog::error);
    TOKOTACHI_C_ADD_PROPERTY("selected_filter", &tokotachi::FileDialog::selected_filter);
    TOKOTACHI_C_ADD_PROPERTY("running", &tokotachi::FileDialog::running);
    TOKOTACHI_C_ADD_PROPERTY("kind", &tokotachi::Popup::kind);
    TOKOTACHI_C_ADD_PROPERTY("text", &tokotachi::StatusBar::text);
    TOKOTACHI_C_ADD_PROPERTY("message", &tokotachi::StatusBar::message);
    TOKOTACHI_C_ADD_PROPERTY("button_style", &tokotachi::ToolBar::button_style);
    TOKOTACHI_C_ADD_PROPERTY("margins", &tokotachi::Layout::margins);
    TOKOTACHI_C_ADD_PROPERTY("orientation", &tokotachi::BoxLayout::orientation);
    TOKOTACHI_C_ADD_PROPERTY("spacing", &tokotachi::BoxLayout::spacing);
    TOKOTACHI_C_ADD_PROPERTY("row_spacing", &tokotachi::GridLayout::row_spacing);
    TOKOTACHI_C_ADD_PROPERTY("column_spacing", &tokotachi::GridLayout::column_spacing);
    TOKOTACHI_C_ADD_PROPERTY("text", &tokotachi::Action::text);
    TOKOTACHI_C_ADD_PROPERTY("shortcut", &tokotachi::Action::shortcut);
    TOKOTACHI_C_ADD_PROPERTY("tooltip", &tokotachi::Action::tooltip);
    TOKOTACHI_C_ADD_PROPERTY("enabled", &tokotachi::Action::enabled);
    TOKOTACHI_C_ADD_PROPERTY("checkable", &tokotachi::Action::checkable);
    TOKOTACHI_C_ADD_PROPERTY("checked", &tokotachi::Action::checked);
    TOKOTACHI_C_ADD_PROPERTY("title", &tokotachi::Menu::title);
    TOKOTACHI_C_ADD_PROPERTY("opened", &tokotachi::Menu::opened);
    TOKOTACHI_C_ADD_PROPERTY("text", &tokotachi::Item::text);
    TOKOTACHI_C_ADD_PROPERTY("column_texts", &tokotachi::Item::column_texts);
    TOKOTACHI_C_ADD_PROPERTY("enabled", &tokotachi::Item::enabled);
    TOKOTACHI_C_ADD_PROPERTY("checkable", &tokotachi::Item::checkable);
    TOKOTACHI_C_ADD_PROPERTY("checked", &tokotachi::Item::checked);
    TOKOTACHI_C_ADD_PROPERTY("text_color", &tokotachi::Item::text_color);
    TOKOTACHI_C_ADD_PROPERTY("font", &tokotachi::Item::font);
    TOKOTACHI_C_ADD_PROPERTY("editable", &tokotachi::Item::editable);
    TOKOTACHI_C_ADD_PROPERTY("title", &tokotachi::Column::title);
    TOKOTACHI_C_ADD_PROPERTY("width", &tokotachi::Column::width);
    TOKOTACHI_C_ADD_PROPERTY("alignment", &tokotachi::Column::alignment);
    TOKOTACHI_C_ADD_PROPERTY("header_visible", &tokotachi::ItemView::header_visible);
    TOKOTACHI_C_ADD_PROPERTY("sorting_enabled", &tokotachi::ItemView::sorting_enabled);
    TOKOTACHI_C_ADD_PROPERTY("sort_column", &tokotachi::ItemView::sort_column);
    TOKOTACHI_C_ADD_PROPERTY("sort_order", &tokotachi::ItemView::sort_order);
    TOKOTACHI_C_ADD_PROPERTY("selection_mode", &tokotachi::ItemView::selection_mode);
    TOKOTACHI_C_ADD_PROPERTY("current", &tokotachi::ItemView::current);
    TOKOTACHI_C_ADD_PROPERTY("selection", &tokotachi::ItemView::selection);
    TOKOTACHI_C_ADD_PROPERTY("editing", &tokotachi::ItemView::editing);
    TOKOTACHI_C_ADD_PROPERTY("drag_enabled", &tokotachi::ItemView::drag_enabled);
    TOKOTACHI_C_ADD_PROPERTY("drop_enabled", &tokotachi::ItemView::drop_enabled);
    TOKOTACHI_C_ADD_PROPERTY("selection_behavior", &tokotachi::TableView::selection_behavior);
    TOKOTACHI_C_ADD_PROPERTY("current_column", &tokotachi::TableView::current_column);
    TOKOTACHI_C_ADD_PROPERTY("selected_ranges", &tokotachi::TableView::selected_ranges);
    TOKOTACHI_C_ADD_PROPERTY("grid_visible", &tokotachi::TableView::grid_visible);
    TOKOTACHI_C_ADD_PROPERTY("row_header_visible", &tokotachi::TableView::row_header_visible);
    TOKOTACHI_C_ADD_PROPERTY("indentation", &tokotachi::TreeView::indentation);
    TOKOTACHI_C_ADD_PROPERTY("running", &tokotachi::TaskBase::running);
    TOKOTACHI_C_ADD_PROPERTY("progress", &tokotachi::TaskBase::progress);
    TOKOTACHI_C_ADD_PROPERTY("cancelled", &tokotachi::TaskBase::cancelled);
    TOKOTACHI_C_ADD_PROPERTY("error", &tokotachi::TaskBase::error);
}

// 列挙の値が C++ の enum class と同じ順であること。
static_assert(static_cast<int>(tokotachi::MouseButton::NONE) == TOKOTACHI_MOUSE_BUTTON_NONE, "MouseButton");
static_assert(static_cast<int>(tokotachi::MouseButton::LEFT) == TOKOTACHI_MOUSE_BUTTON_LEFT, "MouseButton");
static_assert(static_cast<int>(tokotachi::MouseButton::MIDDLE) == TOKOTACHI_MOUSE_BUTTON_MIDDLE, "MouseButton");
static_assert(static_cast<int>(tokotachi::MouseButton::RIGHT) == TOKOTACHI_MOUSE_BUTTON_RIGHT, "MouseButton");
static_assert(static_cast<int>(tokotachi::MouseButton::RIGHT) == 3, "MouseButton の数");
static_assert(static_cast<int>(tokotachi::Key::UNKNOWN) == TOKOTACHI_KEY_UNKNOWN, "Key");
static_assert(static_cast<int>(tokotachi::Key::BACKSPACE) == TOKOTACHI_KEY_BACKSPACE, "Key");
static_assert(static_cast<int>(tokotachi::Key::TAB) == TOKOTACHI_KEY_TAB, "Key");
static_assert(static_cast<int>(tokotachi::Key::RETURN) == TOKOTACHI_KEY_RETURN, "Key");
static_assert(static_cast<int>(tokotachi::Key::ESCAPE) == TOKOTACHI_KEY_ESCAPE, "Key");
static_assert(static_cast<int>(tokotachi::Key::SPACE) == TOKOTACHI_KEY_SPACE, "Key");
static_assert(static_cast<int>(tokotachi::Key::DEL) == TOKOTACHI_KEY_DEL, "Key");
static_assert(static_cast<int>(tokotachi::Key::INSERT) == TOKOTACHI_KEY_INSERT, "Key");
static_assert(static_cast<int>(tokotachi::Key::LEFT) == TOKOTACHI_KEY_LEFT, "Key");
static_assert(static_cast<int>(tokotachi::Key::RIGHT) == TOKOTACHI_KEY_RIGHT, "Key");
static_assert(static_cast<int>(tokotachi::Key::UP) == TOKOTACHI_KEY_UP, "Key");
static_assert(static_cast<int>(tokotachi::Key::DOWN) == TOKOTACHI_KEY_DOWN, "Key");
static_assert(static_cast<int>(tokotachi::Key::HOME) == TOKOTACHI_KEY_HOME, "Key");
static_assert(static_cast<int>(tokotachi::Key::END) == TOKOTACHI_KEY_END, "Key");
static_assert(static_cast<int>(tokotachi::Key::PAGE_UP) == TOKOTACHI_KEY_PAGE_UP, "Key");
static_assert(static_cast<int>(tokotachi::Key::PAGE_DOWN) == TOKOTACHI_KEY_PAGE_DOWN, "Key");
static_assert(static_cast<int>(tokotachi::Key::A) == TOKOTACHI_KEY_A, "Key");
static_assert(static_cast<int>(tokotachi::Key::B) == TOKOTACHI_KEY_B, "Key");
static_assert(static_cast<int>(tokotachi::Key::C) == TOKOTACHI_KEY_C, "Key");
static_assert(static_cast<int>(tokotachi::Key::D) == TOKOTACHI_KEY_D, "Key");
static_assert(static_cast<int>(tokotachi::Key::E) == TOKOTACHI_KEY_E, "Key");
static_assert(static_cast<int>(tokotachi::Key::F) == TOKOTACHI_KEY_F, "Key");
static_assert(static_cast<int>(tokotachi::Key::G) == TOKOTACHI_KEY_G, "Key");
static_assert(static_cast<int>(tokotachi::Key::H) == TOKOTACHI_KEY_H, "Key");
static_assert(static_cast<int>(tokotachi::Key::I) == TOKOTACHI_KEY_I, "Key");
static_assert(static_cast<int>(tokotachi::Key::J) == TOKOTACHI_KEY_J, "Key");
static_assert(static_cast<int>(tokotachi::Key::K) == TOKOTACHI_KEY_K, "Key");
static_assert(static_cast<int>(tokotachi::Key::L) == TOKOTACHI_KEY_L, "Key");
static_assert(static_cast<int>(tokotachi::Key::M) == TOKOTACHI_KEY_M, "Key");
static_assert(static_cast<int>(tokotachi::Key::N) == TOKOTACHI_KEY_N, "Key");
static_assert(static_cast<int>(tokotachi::Key::O) == TOKOTACHI_KEY_O, "Key");
static_assert(static_cast<int>(tokotachi::Key::P) == TOKOTACHI_KEY_P, "Key");
static_assert(static_cast<int>(tokotachi::Key::Q) == TOKOTACHI_KEY_Q, "Key");
static_assert(static_cast<int>(tokotachi::Key::R) == TOKOTACHI_KEY_R, "Key");
static_assert(static_cast<int>(tokotachi::Key::S) == TOKOTACHI_KEY_S, "Key");
static_assert(static_cast<int>(tokotachi::Key::T) == TOKOTACHI_KEY_T, "Key");
static_assert(static_cast<int>(tokotachi::Key::U) == TOKOTACHI_KEY_U, "Key");
static_assert(static_cast<int>(tokotachi::Key::V) == TOKOTACHI_KEY_V, "Key");
static_assert(static_cast<int>(tokotachi::Key::W) == TOKOTACHI_KEY_W, "Key");
static_assert(static_cast<int>(tokotachi::Key::X) == TOKOTACHI_KEY_X, "Key");
static_assert(static_cast<int>(tokotachi::Key::Y) == TOKOTACHI_KEY_Y, "Key");
static_assert(static_cast<int>(tokotachi::Key::Z) == TOKOTACHI_KEY_Z, "Key");
static_assert(static_cast<int>(tokotachi::Key::DIGIT_0) == TOKOTACHI_KEY_DIGIT_0, "Key");
static_assert(static_cast<int>(tokotachi::Key::DIGIT_1) == TOKOTACHI_KEY_DIGIT_1, "Key");
static_assert(static_cast<int>(tokotachi::Key::DIGIT_2) == TOKOTACHI_KEY_DIGIT_2, "Key");
static_assert(static_cast<int>(tokotachi::Key::DIGIT_3) == TOKOTACHI_KEY_DIGIT_3, "Key");
static_assert(static_cast<int>(tokotachi::Key::DIGIT_4) == TOKOTACHI_KEY_DIGIT_4, "Key");
static_assert(static_cast<int>(tokotachi::Key::DIGIT_5) == TOKOTACHI_KEY_DIGIT_5, "Key");
static_assert(static_cast<int>(tokotachi::Key::DIGIT_6) == TOKOTACHI_KEY_DIGIT_6, "Key");
static_assert(static_cast<int>(tokotachi::Key::DIGIT_7) == TOKOTACHI_KEY_DIGIT_7, "Key");
static_assert(static_cast<int>(tokotachi::Key::DIGIT_8) == TOKOTACHI_KEY_DIGIT_8, "Key");
static_assert(static_cast<int>(tokotachi::Key::DIGIT_9) == TOKOTACHI_KEY_DIGIT_9, "Key");
static_assert(static_cast<int>(tokotachi::Key::F1) == TOKOTACHI_KEY_F1, "Key");
static_assert(static_cast<int>(tokotachi::Key::F2) == TOKOTACHI_KEY_F2, "Key");
static_assert(static_cast<int>(tokotachi::Key::F3) == TOKOTACHI_KEY_F3, "Key");
static_assert(static_cast<int>(tokotachi::Key::F4) == TOKOTACHI_KEY_F4, "Key");
static_assert(static_cast<int>(tokotachi::Key::F5) == TOKOTACHI_KEY_F5, "Key");
static_assert(static_cast<int>(tokotachi::Key::F6) == TOKOTACHI_KEY_F6, "Key");
static_assert(static_cast<int>(tokotachi::Key::F7) == TOKOTACHI_KEY_F7, "Key");
static_assert(static_cast<int>(tokotachi::Key::F8) == TOKOTACHI_KEY_F8, "Key");
static_assert(static_cast<int>(tokotachi::Key::F9) == TOKOTACHI_KEY_F9, "Key");
static_assert(static_cast<int>(tokotachi::Key::F10) == TOKOTACHI_KEY_F10, "Key");
static_assert(static_cast<int>(tokotachi::Key::F11) == TOKOTACHI_KEY_F11, "Key");
static_assert(static_cast<int>(tokotachi::Key::F12) == TOKOTACHI_KEY_F12, "Key");
static_assert(static_cast<int>(tokotachi::Key::SHIFT) == TOKOTACHI_KEY_SHIFT, "Key");
static_assert(static_cast<int>(tokotachi::Key::CONTROL) == TOKOTACHI_KEY_CONTROL, "Key");
static_assert(static_cast<int>(tokotachi::Key::ALT) == TOKOTACHI_KEY_ALT, "Key");
static_assert(static_cast<int>(tokotachi::Key::META) == TOKOTACHI_KEY_META, "Key");
static_assert(static_cast<int>(tokotachi::Key::META) == 67, "Key の数");
static_assert(static_cast<int>(tokotachi::FocusPolicy::NONE) == TOKOTACHI_FOCUS_POLICY_NONE, "FocusPolicy");
static_assert(static_cast<int>(tokotachi::FocusPolicy::TAB) == TOKOTACHI_FOCUS_POLICY_TAB, "FocusPolicy");
static_assert(static_cast<int>(tokotachi::FocusPolicy::CLICK) == TOKOTACHI_FOCUS_POLICY_CLICK, "FocusPolicy");
static_assert(static_cast<int>(tokotachi::FocusPolicy::TAB_AND_CLICK) == TOKOTACHI_FOCUS_POLICY_TAB_AND_CLICK, "FocusPolicy");
static_assert(static_cast<int>(tokotachi::FocusPolicy::TAB_AND_CLICK) == 3, "FocusPolicy の数");
static_assert(static_cast<int>(tokotachi::MouseCursor::ARROW) == TOKOTACHI_MOUSE_CURSOR_ARROW, "MouseCursor");
static_assert(static_cast<int>(tokotachi::MouseCursor::IBEAM) == TOKOTACHI_MOUSE_CURSOR_IBEAM, "MouseCursor");
static_assert(static_cast<int>(tokotachi::MouseCursor::POINTING_HAND) == TOKOTACHI_MOUSE_CURSOR_POINTING_HAND, "MouseCursor");
static_assert(static_cast<int>(tokotachi::MouseCursor::WAIT) == TOKOTACHI_MOUSE_CURSOR_WAIT, "MouseCursor");
static_assert(static_cast<int>(tokotachi::MouseCursor::CROSSHAIR) == TOKOTACHI_MOUSE_CURSOR_CROSSHAIR, "MouseCursor");
static_assert(static_cast<int>(tokotachi::MouseCursor::SIZE_HORIZONTAL) == TOKOTACHI_MOUSE_CURSOR_SIZE_HORIZONTAL, "MouseCursor");
static_assert(static_cast<int>(tokotachi::MouseCursor::SIZE_VERTICAL) == TOKOTACHI_MOUSE_CURSOR_SIZE_VERTICAL, "MouseCursor");
static_assert(static_cast<int>(tokotachi::MouseCursor::SIZE_VERTICAL) == 6, "MouseCursor の数");
static_assert(static_cast<int>(tokotachi::Orientation::HORIZONTAL) == TOKOTACHI_ORIENTATION_HORIZONTAL, "Orientation");
static_assert(static_cast<int>(tokotachi::Orientation::VERTICAL) == TOKOTACHI_ORIENTATION_VERTICAL, "Orientation");
static_assert(static_cast<int>(tokotachi::Orientation::VERTICAL) == 1, "Orientation の数");
static_assert(static_cast<int>(tokotachi::Align::START) == TOKOTACHI_ALIGN_START, "Align");
static_assert(static_cast<int>(tokotachi::Align::CENTER) == TOKOTACHI_ALIGN_CENTER, "Align");
static_assert(static_cast<int>(tokotachi::Align::END) == TOKOTACHI_ALIGN_END, "Align");
static_assert(static_cast<int>(tokotachi::Align::FILL) == TOKOTACHI_ALIGN_FILL, "Align");
static_assert(static_cast<int>(tokotachi::Align::FILL) == 3, "Align の数");
static_assert(static_cast<int>(tokotachi::Anchor::TOP_LEFT) == TOKOTACHI_ANCHOR_TOP_LEFT, "Anchor");
static_assert(static_cast<int>(tokotachi::Anchor::TOP) == TOKOTACHI_ANCHOR_TOP, "Anchor");
static_assert(static_cast<int>(tokotachi::Anchor::TOP_RIGHT) == TOKOTACHI_ANCHOR_TOP_RIGHT, "Anchor");
static_assert(static_cast<int>(tokotachi::Anchor::LEFT) == TOKOTACHI_ANCHOR_LEFT, "Anchor");
static_assert(static_cast<int>(tokotachi::Anchor::CENTER) == TOKOTACHI_ANCHOR_CENTER, "Anchor");
static_assert(static_cast<int>(tokotachi::Anchor::RIGHT) == TOKOTACHI_ANCHOR_RIGHT, "Anchor");
static_assert(static_cast<int>(tokotachi::Anchor::BOTTOM_LEFT) == TOKOTACHI_ANCHOR_BOTTOM_LEFT, "Anchor");
static_assert(static_cast<int>(tokotachi::Anchor::BOTTOM) == TOKOTACHI_ANCHOR_BOTTOM, "Anchor");
static_assert(static_cast<int>(tokotachi::Anchor::BOTTOM_RIGHT) == TOKOTACHI_ANCHOR_BOTTOM_RIGHT, "Anchor");
static_assert(static_cast<int>(tokotachi::Anchor::BOTTOM_RIGHT) == 8, "Anchor の数");
static_assert(static_cast<int>(tokotachi::ColorScheme::SYSTEM) == TOKOTACHI_COLOR_SCHEME_SYSTEM, "ColorScheme");
static_assert(static_cast<int>(tokotachi::ColorScheme::LIGHT) == TOKOTACHI_COLOR_SCHEME_LIGHT, "ColorScheme");
static_assert(static_cast<int>(tokotachi::ColorScheme::DARK) == TOKOTACHI_COLOR_SCHEME_DARK, "ColorScheme");
static_assert(static_cast<int>(tokotachi::ColorScheme::DARK) == 2, "ColorScheme の数");
static_assert(static_cast<int>(tokotachi::ColorRole::WINDOW) == TOKOTACHI_COLOR_ROLE_WINDOW, "ColorRole");
static_assert(static_cast<int>(tokotachi::ColorRole::POPUP) == TOKOTACHI_COLOR_ROLE_POPUP, "ColorRole");
static_assert(static_cast<int>(tokotachi::ColorRole::TEXT) == TOKOTACHI_COLOR_ROLE_TEXT, "ColorRole");
static_assert(static_cast<int>(tokotachi::ColorRole::TEXT_SECONDARY) == TOKOTACHI_COLOR_ROLE_TEXT_SECONDARY, "ColorRole");
static_assert(static_cast<int>(tokotachi::ColorRole::TEXT_DISABLED) == TOKOTACHI_COLOR_ROLE_TEXT_DISABLED, "ColorRole");
static_assert(static_cast<int>(tokotachi::ColorRole::PLACEHOLDER) == TOKOTACHI_COLOR_ROLE_PLACEHOLDER, "ColorRole");
static_assert(static_cast<int>(tokotachi::ColorRole::ACCENT) == TOKOTACHI_COLOR_ROLE_ACCENT, "ColorRole");
static_assert(static_cast<int>(tokotachi::ColorRole::ACCENT_HOVER) == TOKOTACHI_COLOR_ROLE_ACCENT_HOVER, "ColorRole");
static_assert(static_cast<int>(tokotachi::ColorRole::ACCENT_PRESSED) == TOKOTACHI_COLOR_ROLE_ACCENT_PRESSED, "ColorRole");
static_assert(static_cast<int>(tokotachi::ColorRole::ACCENT_DISABLED) == TOKOTACHI_COLOR_ROLE_ACCENT_DISABLED, "ColorRole");
static_assert(static_cast<int>(tokotachi::ColorRole::ON_ACCENT) == TOKOTACHI_COLOR_ROLE_ON_ACCENT, "ColorRole");
static_assert(static_cast<int>(tokotachi::ColorRole::FOCUS_RING) == TOKOTACHI_COLOR_ROLE_FOCUS_RING, "ColorRole");
static_assert(static_cast<int>(tokotachi::ColorRole::FIELD) == TOKOTACHI_COLOR_ROLE_FIELD, "ColorRole");
static_assert(static_cast<int>(tokotachi::ColorRole::FIELD_DISABLED) == TOKOTACHI_COLOR_ROLE_FIELD_DISABLED, "ColorRole");
static_assert(static_cast<int>(tokotachi::ColorRole::BORDER) == TOKOTACHI_COLOR_ROLE_BORDER, "ColorRole");
static_assert(static_cast<int>(tokotachi::ColorRole::SELECTION) == TOKOTACHI_COLOR_ROLE_SELECTION, "ColorRole");
static_assert(static_cast<int>(tokotachi::ColorRole::SELECTION_INACTIVE) == TOKOTACHI_COLOR_ROLE_SELECTION_INACTIVE, "ColorRole");
static_assert(static_cast<int>(tokotachi::ColorRole::ON_SELECTION) == TOKOTACHI_COLOR_ROLE_ON_SELECTION, "ColorRole");
static_assert(static_cast<int>(tokotachi::ColorRole::PREEDIT_UNDERLINE) == TOKOTACHI_COLOR_ROLE_PREEDIT_UNDERLINE, "ColorRole");
static_assert(static_cast<int>(tokotachi::ColorRole::SCROLL_TRACK) == TOKOTACHI_COLOR_ROLE_SCROLL_TRACK, "ColorRole");
static_assert(static_cast<int>(tokotachi::ColorRole::SCROLL_THUMB) == TOKOTACHI_COLOR_ROLE_SCROLL_THUMB, "ColorRole");
static_assert(static_cast<int>(tokotachi::ColorRole::SCROLL_THUMB_ACTIVE) == TOKOTACHI_COLOR_ROLE_SCROLL_THUMB_ACTIVE, "ColorRole");
static_assert(static_cast<int>(tokotachi::ColorRole::TOOLTIP) == TOKOTACHI_COLOR_ROLE_TOOLTIP, "ColorRole");
static_assert(static_cast<int>(tokotachi::ColorRole::ON_TOOLTIP) == TOKOTACHI_COLOR_ROLE_ON_TOOLTIP, "ColorRole");
static_assert(static_cast<int>(tokotachi::ColorRole::ON_TOOLTIP) == 23, "ColorRole の数");
static_assert(static_cast<int>(tokotachi::ButtonStyle::PRIMARY) == TOKOTACHI_BUTTON_STYLE_PRIMARY, "ButtonStyle");
static_assert(static_cast<int>(tokotachi::ButtonStyle::SECONDARY) == TOKOTACHI_BUTTON_STYLE_SECONDARY, "ButtonStyle");
static_assert(static_cast<int>(tokotachi::ButtonStyle::SECONDARY) == 1, "ButtonStyle の数");
static_assert(static_cast<int>(tokotachi::DialogButton::NONE) == TOKOTACHI_DIALOG_BUTTON_NONE, "DialogButton");
static_assert(static_cast<int>(tokotachi::DialogButton::OK) == TOKOTACHI_DIALOG_BUTTON_OK, "DialogButton");
static_assert(static_cast<int>(tokotachi::DialogButton::YES) == TOKOTACHI_DIALOG_BUTTON_YES, "DialogButton");
static_assert(static_cast<int>(tokotachi::DialogButton::SAVE) == TOKOTACHI_DIALOG_BUTTON_SAVE, "DialogButton");
static_assert(static_cast<int>(tokotachi::DialogButton::NO) == TOKOTACHI_DIALOG_BUTTON_NO, "DialogButton");
static_assert(static_cast<int>(tokotachi::DialogButton::DISCARD) == TOKOTACHI_DIALOG_BUTTON_DISCARD, "DialogButton");
static_assert(static_cast<int>(tokotachi::DialogButton::CANCEL) == TOKOTACHI_DIALOG_BUTTON_CANCEL, "DialogButton");
static_assert(static_cast<int>(tokotachi::DialogButton::CLOSE) == TOKOTACHI_DIALOG_BUTTON_CLOSE, "DialogButton");
static_assert(static_cast<int>(tokotachi::DialogButton::CLOSE) == 7, "DialogButton の数");
static_assert(static_cast<int>(tokotachi::FileDialogMode::OPEN) == TOKOTACHI_FILE_DIALOG_MODE_OPEN, "FileDialogMode");
static_assert(static_cast<int>(tokotachi::FileDialogMode::SAVE) == TOKOTACHI_FILE_DIALOG_MODE_SAVE, "FileDialogMode");
static_assert(static_cast<int>(tokotachi::FileDialogMode::FOLDER) == TOKOTACHI_FILE_DIALOG_MODE_FOLDER, "FileDialogMode");
static_assert(static_cast<int>(tokotachi::FileDialogMode::FOLDER) == 2, "FileDialogMode の数");
static_assert(static_cast<int>(tokotachi::MessageType::INFORMATION) == TOKOTACHI_MESSAGE_TYPE_INFORMATION, "MessageType");
static_assert(static_cast<int>(tokotachi::MessageType::QUESTION) == TOKOTACHI_MESSAGE_TYPE_QUESTION, "MessageType");
static_assert(static_cast<int>(tokotachi::MessageType::WARNING) == TOKOTACHI_MESSAGE_TYPE_WARNING, "MessageType");
static_assert(static_cast<int>(tokotachi::MessageType::CRITICAL) == TOKOTACHI_MESSAGE_TYPE_CRITICAL, "MessageType");
static_assert(static_cast<int>(tokotachi::MessageType::CRITICAL) == 3, "MessageType の数");
static_assert(static_cast<int>(tokotachi::PopupKind::MENU) == TOKOTACHI_POPUP_KIND_MENU, "PopupKind");
static_assert(static_cast<int>(tokotachi::PopupKind::TOOLTIP) == TOKOTACHI_POPUP_KIND_TOOLTIP, "PopupKind");
static_assert(static_cast<int>(tokotachi::PopupKind::TOOLTIP) == 1, "PopupKind の数");
static_assert(static_cast<int>(tokotachi::PopupPlacement::BELOW) == TOKOTACHI_POPUP_PLACEMENT_BELOW, "PopupPlacement");
static_assert(static_cast<int>(tokotachi::PopupPlacement::RIGHT) == TOKOTACHI_POPUP_PLACEMENT_RIGHT, "PopupPlacement");
static_assert(static_cast<int>(tokotachi::PopupPlacement::AT) == TOKOTACHI_POPUP_PLACEMENT_AT, "PopupPlacement");
static_assert(static_cast<int>(tokotachi::PopupPlacement::AT) == 2, "PopupPlacement の数");
static_assert(static_cast<int>(tokotachi::ToolButtonStyle::ICON_ONLY) == TOKOTACHI_TOOL_BUTTON_STYLE_ICON_ONLY, "ToolButtonStyle");
static_assert(static_cast<int>(tokotachi::ToolButtonStyle::TEXT_BESIDE_ICON) == TOKOTACHI_TOOL_BUTTON_STYLE_TEXT_BESIDE_ICON, "ToolButtonStyle");
static_assert(static_cast<int>(tokotachi::ToolButtonStyle::TEXT_UNDER_ICON) == TOKOTACHI_TOOL_BUTTON_STYLE_TEXT_UNDER_ICON, "ToolButtonStyle");
static_assert(static_cast<int>(tokotachi::ToolButtonStyle::TEXT_UNDER_ICON) == 2, "ToolButtonStyle の数");
static_assert(static_cast<int>(tokotachi::SelectionMode::NONE) == TOKOTACHI_SELECTION_MODE_NONE, "SelectionMode");
static_assert(static_cast<int>(tokotachi::SelectionMode::SINGLE) == TOKOTACHI_SELECTION_MODE_SINGLE, "SelectionMode");
static_assert(static_cast<int>(tokotachi::SelectionMode::MULTI) == TOKOTACHI_SELECTION_MODE_MULTI, "SelectionMode");
static_assert(static_cast<int>(tokotachi::SelectionMode::MULTI) == 2, "SelectionMode の数");
static_assert(static_cast<int>(tokotachi::SortOrder::ASCENDING) == TOKOTACHI_SORT_ORDER_ASCENDING, "SortOrder");
static_assert(static_cast<int>(tokotachi::SortOrder::DESCENDING) == TOKOTACHI_SORT_ORDER_DESCENDING, "SortOrder");
static_assert(static_cast<int>(tokotachi::SortOrder::DESCENDING) == 1, "SortOrder の数");
static_assert(static_cast<int>(tokotachi::LineCap::BUTT) == TOKOTACHI_LINE_CAP_BUTT, "LineCap");
static_assert(static_cast<int>(tokotachi::LineCap::ROUND) == TOKOTACHI_LINE_CAP_ROUND, "LineCap");
static_assert(static_cast<int>(tokotachi::LineCap::SQUARE) == TOKOTACHI_LINE_CAP_SQUARE, "LineCap");
static_assert(static_cast<int>(tokotachi::LineCap::SQUARE) == 2, "LineCap の数");
static_assert(static_cast<int>(tokotachi::LineJoin::MITER) == TOKOTACHI_LINE_JOIN_MITER, "LineJoin");
static_assert(static_cast<int>(tokotachi::LineJoin::ROUND) == TOKOTACHI_LINE_JOIN_ROUND, "LineJoin");
static_assert(static_cast<int>(tokotachi::LineJoin::BEVEL) == TOKOTACHI_LINE_JOIN_BEVEL, "LineJoin");
static_assert(static_cast<int>(tokotachi::LineJoin::BEVEL) == 2, "LineJoin の数");
static_assert(static_cast<int>(tokotachi::Paint::Type::SOLID) == TOKOTACHI_PAINT_TYPE_SOLID, "PaintType");
static_assert(static_cast<int>(tokotachi::Paint::Type::LINEAR) == TOKOTACHI_PAINT_TYPE_LINEAR, "PaintType");
static_assert(static_cast<int>(tokotachi::Paint::Type::RADIAL) == TOKOTACHI_PAINT_TYPE_RADIAL, "PaintType");
static_assert(static_cast<int>(tokotachi::Paint::Type::RADIAL) == 2, "PaintType の数");
static_assert(static_cast<int>(tokotachi::DayOfWeek::SUNDAY) == TOKOTACHI_DAY_OF_WEEK_SUNDAY, "DayOfWeek");
static_assert(static_cast<int>(tokotachi::DayOfWeek::MONDAY) == TOKOTACHI_DAY_OF_WEEK_MONDAY, "DayOfWeek");
static_assert(static_cast<int>(tokotachi::DayOfWeek::TUESDAY) == TOKOTACHI_DAY_OF_WEEK_TUESDAY, "DayOfWeek");
static_assert(static_cast<int>(tokotachi::DayOfWeek::WEDNESDAY) == TOKOTACHI_DAY_OF_WEEK_WEDNESDAY, "DayOfWeek");
static_assert(static_cast<int>(tokotachi::DayOfWeek::THURSDAY) == TOKOTACHI_DAY_OF_WEEK_THURSDAY, "DayOfWeek");
static_assert(static_cast<int>(tokotachi::DayOfWeek::FRIDAY) == TOKOTACHI_DAY_OF_WEEK_FRIDAY, "DayOfWeek");
static_assert(static_cast<int>(tokotachi::DayOfWeek::SATURDAY) == TOKOTACHI_DAY_OF_WEEK_SATURDAY, "DayOfWeek");
static_assert(static_cast<int>(tokotachi::DayOfWeek::SATURDAY) == 6, "DayOfWeek の数");
static_assert(static_cast<int>(tokotachi::DropAction::NONE) == TOKOTACHI_DROP_ACTION_NONE, "DropAction");
static_assert(static_cast<int>(tokotachi::DropAction::COPY) == TOKOTACHI_DROP_ACTION_COPY, "DropAction");
static_assert(static_cast<int>(tokotachi::DropAction::MOVE) == TOKOTACHI_DROP_ACTION_MOVE, "DropAction");
static_assert(static_cast<int>(tokotachi::DropAction::MOVE) == 2, "DropAction の数");
static_assert(static_cast<int>(tokotachi::SelectionBehavior::CELLS) == TOKOTACHI_SELECTION_BEHAVIOR_CELLS, "SelectionBehavior");
static_assert(static_cast<int>(tokotachi::SelectionBehavior::ROWS) == TOKOTACHI_SELECTION_BEHAVIOR_ROWS, "SelectionBehavior");
static_assert(static_cast<int>(tokotachi::SelectionBehavior::ROWS) == 1, "SelectionBehavior の数");
static_assert(static_cast<int>(tokotachi::ImageFit::NONE) == TOKOTACHI_IMAGE_FIT_NONE, "ImageFit");
static_assert(static_cast<int>(tokotachi::ImageFit::CONTAIN) == TOKOTACHI_IMAGE_FIT_CONTAIN, "ImageFit");
static_assert(static_cast<int>(tokotachi::ImageFit::COVER) == TOKOTACHI_IMAGE_FIT_COVER, "ImageFit");
static_assert(static_cast<int>(tokotachi::ImageFit::STRETCH) == TOKOTACHI_IMAGE_FIT_STRETCH, "ImageFit");
static_assert(static_cast<int>(tokotachi::ImageFit::STRETCH) == 3, "ImageFit の数");
static_assert(static_cast<int>(tokotachi::Icon::FILE) == TOKOTACHI_ICON_FILE, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::FILE_PLUS) == TOKOTACHI_ICON_FILE_PLUS, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::FILE_TEXT) == TOKOTACHI_ICON_FILE_TEXT, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::FOLDER) == TOKOTACHI_ICON_FOLDER, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::FOLDER_OPEN) == TOKOTACHI_ICON_FOLDER_OPEN, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::SAVE) == TOKOTACHI_ICON_SAVE, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::PRINTER) == TOKOTACHI_ICON_PRINTER, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::DOWNLOAD) == TOKOTACHI_ICON_DOWNLOAD, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::UPLOAD) == TOKOTACHI_ICON_UPLOAD, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::UNDO_2) == TOKOTACHI_ICON_UNDO_2, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::REDO_2) == TOKOTACHI_ICON_REDO_2, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::SCISSORS) == TOKOTACHI_ICON_SCISSORS, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::COPY) == TOKOTACHI_ICON_COPY, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::CLIPBOARD_PASTE) == TOKOTACHI_ICON_CLIPBOARD_PASTE, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::TRASH) == TOKOTACHI_ICON_TRASH, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::PENCIL) == TOKOTACHI_ICON_PENCIL, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::PLUS) == TOKOTACHI_ICON_PLUS, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::MINUS) == TOKOTACHI_ICON_MINUS, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::X) == TOKOTACHI_ICON_X, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::CHECK) == TOKOTACHI_ICON_CHECK, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::ARROW_LEFT) == TOKOTACHI_ICON_ARROW_LEFT, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::ARROW_RIGHT) == TOKOTACHI_ICON_ARROW_RIGHT, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::ARROW_UP) == TOKOTACHI_ICON_ARROW_UP, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::ARROW_DOWN) == TOKOTACHI_ICON_ARROW_DOWN, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::CHEVRON_LEFT) == TOKOTACHI_ICON_CHEVRON_LEFT, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::CHEVRON_RIGHT) == TOKOTACHI_ICON_CHEVRON_RIGHT, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::CHEVRON_UP) == TOKOTACHI_ICON_CHEVRON_UP, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::CHEVRON_DOWN) == TOKOTACHI_ICON_CHEVRON_DOWN, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::HOUSE) == TOKOTACHI_ICON_HOUSE, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::MENU) == TOKOTACHI_ICON_MENU, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::ELLIPSIS) == TOKOTACHI_ICON_ELLIPSIS, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::REFRESH_CW) == TOKOTACHI_ICON_REFRESH_CW, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::EXTERNAL_LINK) == TOKOTACHI_ICON_EXTERNAL_LINK, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::SEARCH) == TOKOTACHI_ICON_SEARCH, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::ZOOM_IN) == TOKOTACHI_ICON_ZOOM_IN, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::ZOOM_OUT) == TOKOTACHI_ICON_ZOOM_OUT, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::EYE) == TOKOTACHI_ICON_EYE, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::EYE_OFF) == TOKOTACHI_ICON_EYE_OFF, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::FUNNEL) == TOKOTACHI_ICON_FUNNEL, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::SETTINGS) == TOKOTACHI_ICON_SETTINGS, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::SLIDERS_HORIZONTAL) == TOKOTACHI_ICON_SLIDERS_HORIZONTAL, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::LIST) == TOKOTACHI_ICON_LIST, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::LAYOUT_GRID) == TOKOTACHI_ICON_LAYOUT_GRID, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::INFO) == TOKOTACHI_ICON_INFO, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::TRIANGLE_ALERT) == TOKOTACHI_ICON_TRIANGLE_ALERT, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::CIRCLE_X) == TOKOTACHI_ICON_CIRCLE_X, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::CIRCLE_CHECK) == TOKOTACHI_ICON_CIRCLE_CHECK, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::CIRCLE_QUESTION_MARK) == TOKOTACHI_ICON_CIRCLE_QUESTION_MARK, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::BELL) == TOKOTACHI_ICON_BELL, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::PLAY) == TOKOTACHI_ICON_PLAY, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::PAUSE) == TOKOTACHI_ICON_PAUSE, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::SQUARE) == TOKOTACHI_ICON_SQUARE, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::SKIP_BACK) == TOKOTACHI_ICON_SKIP_BACK, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::SKIP_FORWARD) == TOKOTACHI_ICON_SKIP_FORWARD, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::VOLUME_2) == TOKOTACHI_ICON_VOLUME_2, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::VOLUME_X) == TOKOTACHI_ICON_VOLUME_X, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::USER) == TOKOTACHI_ICON_USER, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::USERS) == TOKOTACHI_ICON_USERS, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::MAIL) == TOKOTACHI_ICON_MAIL, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::CALENDAR) == TOKOTACHI_ICON_CALENDAR, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::CLOCK) == TOKOTACHI_ICON_CLOCK, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::LOCK) == TOKOTACHI_ICON_LOCK, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::LOCK_OPEN) == TOKOTACHI_ICON_LOCK_OPEN, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::STAR) == TOKOTACHI_ICON_STAR, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::HEART) == TOKOTACHI_ICON_HEART, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::LINK) == TOKOTACHI_ICON_LINK, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::IMAGE) == TOKOTACHI_ICON_IMAGE, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::LOG_IN) == TOKOTACHI_ICON_LOG_IN, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::LOG_OUT) == TOKOTACHI_ICON_LOG_OUT, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::SUN) == TOKOTACHI_ICON_SUN, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::MOON) == TOKOTACHI_ICON_MOON, "Icon");
static_assert(static_cast<int>(tokotachi::Icon::MOON) == 70, "Icon の数");
