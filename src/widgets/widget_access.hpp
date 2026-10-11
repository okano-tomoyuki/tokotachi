// ウィンドウ・アプリケーションの実装から Widget の非公開の部分を扱うための窓口(内部)。
#ifndef TOKOTACHI_SRC_WIDGETS_WIDGET_ACCESS_HPP
#define TOKOTACHI_SRC_WIDGETS_WIDGET_ACCESS_HPP

#include <tokotachi/graphics/canvas.hpp>
#include <tokotachi/widgets/widget.hpp>

#include "theme.hpp"

namespace tokotachi
{
namespace detail
{

struct WidgetAccess
{
    static const std::vector<Action*>& actions_of(const Widget* w) { return w->actions_; }
    static void action_changed(Widget* w, Action* a) { w->action_changed(a); }
    // 表示されるか(visible で、親に隠されていない)。
    static bool displayed(const Widget* w) { return w->displayed(); }
    // ToolBar・Splitter・TabWidget が、利用者の visible を変えずに子を隠す・戻す。
    static void set_hidden_by_parent(Widget* w, bool hide) { w->set_hidden_by_parent(hide); }
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

    // 描く・イベントを処理する間は、w に効いているテーマを今のテーマにする(docs/adr/0042)。
    static void paint(Widget* w, Canvas& c)
    {
        ThemeScope scope(w);
        w->paint(c);
    }
    static void resized(Widget* w)
    {
        ThemeScope scope(w);
        w->resized();
    }
    static void mouse_down(Widget* w, MouseEvent& e)
    {
        ThemeScope scope(w);
        w->mouse_down(e);
    }
    static void mouse_up(Widget* w, MouseEvent& e)
    {
        ThemeScope scope(w);
        w->mouse_up(e);
    }
    static void mouse_move(Widget* w, MouseEvent& e)
    {
        ThemeScope scope(w);
        w->mouse_move(e);
    }
    static void mouse_enter(Widget* w)
    {
        ThemeScope scope(w);
        w->mouse_enter();
    }
    static void mouse_leave(Widget* w)
    {
        ThemeScope scope(w);
        w->mouse_leave();
    }
    static void mouse_wheel(Widget* w, WheelEvent& e)
    {
        ThemeScope scope(w);
        w->mouse_wheel(e);
    }
    static void arrange_children(Widget* w)
    {
        ThemeScope scope(w);
        w->arrange_children();
    }
    static void key_down(Widget* w, KeyEvent& e)
    {
        ThemeScope scope(w);
        w->key_down(e);
    }
    static void key_up(Widget* w, KeyEvent& e)
    {
        ThemeScope scope(w);
        w->key_up(e);
    }
    static void text_input(Widget* w, TextInputEvent& e)
    {
        ThemeScope scope(w);
        w->text_input(e);
    }
    static void text_editing(Widget* w, TextEditingEvent& e)
    {
        ThemeScope scope(w);
        w->text_editing(e);
    }
    static bool accepts_drops(const Widget* w) { return w->accepts_drops_; }
    static void drag_move(Widget* w, DragEvent& e)
    {
        ThemeScope scope(w);
        w->drag_move(e);
    }
    static void drag_leave(Widget* w)
    {
        ThemeScope scope(w);
        w->drag_leave();
    }
    static void drop(Widget* w, DragEvent& e)
    {
        ThemeScope scope(w);
        w->drop(e);
    }
    static void drag_finished(Widget* w, DropAction action)
    {
        ThemeScope scope(w);
        w->drag_finished(action);
    }
    static void focus_in(Widget* w)
    {
        ThemeScope scope(w);
        w->focus_in();
    }
    static void focus_out(Widget* w)
    {
        ThemeScope scope(w);
        w->focus_out();
    }
    static Rect input_method_rect(const Widget* w)
    {
        ThemeScope scope(w);
        return w->input_method_rect();
    }

    // w と子孫すべてに theme_changed を送る。
    static void theme_changed(Widget* w)
    {
        {
            ThemeScope scope(w);
            w->theme_changed();
        }
        w->notify_changed(w->theme);
        w->layout_dirty_ = true;  // 寸法(余白など)が変わり、望む大きさが変わりうる
        for (std::size_t i = 0; i < w->children_.size(); ++i)
        {
            theme_changed(w->children_[i]);
        }
    }

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
}  // namespace tokotachi

#endif
