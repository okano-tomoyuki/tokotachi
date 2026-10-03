// ウィンドウ・アプリケーションの実装から Widget の非公開の部分を扱うための窓口(内部)。
#ifndef GUI_SRC_CORE_WIDGET_ACCESS_HPP
#define GUI_SRC_CORE_WIDGET_ACCESS_HPP

#include <gui/canvas.hpp>
#include <gui/widget.hpp>

namespace gui
{
namespace detail
{

struct WidgetAccess
{
    static bool visible_flag(const Widget* w) { return w->visible_; }
    static void set_visible_flag(Widget* w, bool v) { w->visible_ = v; }
    static bool is_window(const Widget* w) { return w->is_window_; }
    static void set_is_window(Widget* w, bool v) { w->is_window_ = v; }
    static Widget* parent_of(const Widget* w) { return w->parent_; }
    static const std::vector<Widget*>& children(const Widget* w) { return w->children_; }
    static const Rect& bounds(const Widget* w) { return w->bounds_; }
    static Window* window_of(const Widget* w) { return w->get_window(); }
    // bounds への代入と違い、親への描き直しの依頼と resized を行わない。
    static void set_bounds_raw(Widget* w, const Rect& r) { w->bounds_ = r; }
    static void notify_bounds(Widget* w, const Rect& old) { w->notify_bounds(old); }
    static void notify_focused(Widget* w) { w->notify_changed(w->focused); }
    static void notify_visible(Widget* w) { w->notify_changed(w->visible); }
    static Layout* layout_of(const Widget* w) { return w->layout_; }
    static void set_layout(Widget* w, Layout* l) { w->layout_ = l; }
    static bool layout_dirty(const Widget* w) { return w->layout_dirty_; }
    static void clear_layout_dirty(Widget* w) { w->layout_dirty_ = false; }
    static void mark_layout_dirty(Widget* w) { w->mark_layout_dirty(); }

    static void paint(Widget* w, Canvas& c) { w->paint(c); }
    static void resized(Widget* w) { w->resized(); }
    static void mouse_down(Widget* w, MouseEvent& e) { w->mouse_down(e); }
    static void mouse_up(Widget* w, MouseEvent& e) { w->mouse_up(e); }
    static void mouse_move(Widget* w, MouseEvent& e) { w->mouse_move(e); }
    static void mouse_enter(Widget* w) { w->mouse_enter(); }
    static void mouse_leave(Widget* w) { w->mouse_leave(); }
    static void mouse_wheel(Widget* w, WheelEvent& e) { w->mouse_wheel(e); }
    static void arrange_children(Widget* w) { w->arrange_children(); }
    static void key_down(Widget* w, KeyEvent& e) { w->key_down(e); }
    static void key_up(Widget* w, KeyEvent& e) { w->key_up(e); }
    static void text_input(Widget* w, TextInputEvent& e) { w->text_input(e); }
    static void text_editing(Widget* w, TextEditingEvent& e) { w->text_editing(e); }
    static void focus_in(Widget* w) { w->focus_in(); }
    static void focus_out(Widget* w) { w->focus_out(); }
    static Rect input_method_rect(const Widget* w) { return w->input_method_rect(); }

    // w と子孫すべてに scale_changed を送る。
    static void scale_changed(Widget* w)
    {
        w->scale_changed();
        w->layout_dirty_ = true;  // 文字の大きさ(ヒンティング)が変わり、望む大きさが変わりうる
        for (std::size_t i = 0; i < w->children_.size(); ++i)
        {
            scale_changed(w->children_[i]);
        }
    }
};

// a が b 自身か b の祖先か。
inline bool is_ancestor_or_self(const Widget* a, const Widget* b)
{
    for (; b; b = WidgetAccess::parent_of(b))
    {
        if (a == b)
        {
            return true;
        }
    }
    return false;
}

}  // namespace detail
}  // namespace gui

#endif
