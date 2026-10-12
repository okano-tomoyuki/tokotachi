// ウィンドウ・アプリケーションの実装から Widget の非公開の部分を扱うための窓口(内部)。
#ifndef TOKOTACHI_SRC_WIDGETS_WIDGET_ACCESS_HPP
#define TOKOTACHI_SRC_WIDGETS_WIDGET_ACCESS_HPP

#include <tokotachi/graphics/canvas.hpp>
#include <tokotachi/widgets/widget.hpp>

#include "../core/impl.hpp"
#include "theme.hpp"

namespace tokotachi
{

class Widget::Impl
{
    TOKOTACHI_IMPL(Widget);

    explicit Impl(Widget* object) : q(object) {}

    Widget* get_parent() const { return parent_; }
    void set_parent(Widget* const& p);
    /// w と子孫に scale_changed を送り、配置のやり直しの印を付ける(別のウィンドウへ移ったとき)。
    static void scale_changed_tree(Widget* w);
    std::vector<Widget*> get_children() const { return children_; }
    Window* get_window() const;
    float get_x() const { return bounds_.x; }
    void set_x(const float& v);
    float get_y() const { return bounds_.y; }
    void set_y(const float& v);
    float get_width() const { return bounds_.width; }
    void set_width(const float& v);
    float get_height() const { return bounds_.height; }
    void set_height(const float& v);
    Rect get_bounds() const { return bounds_; }
    void set_bounds(const Rect& r);
    /// old から変わった位置と大きさのプロパティを知らせる。
    void notify_bounds(const Rect& old);
    Rect get_local_rect() const { return Rect(0, 0, bounds_.width, bounds_.height); }
    Size get_preferred_size() const;
    Size get_minimum_size() const { return minimum_size_; }
    void set_minimum_size(const Size& v);
    Size get_maximum_size() const { return maximum_size_; }
    void set_maximum_size(const Size& v);
    Size get_minimum_hint() const;
    Layout* get_layout() const { return layout_; }

    bool get_visible() const { return visible_; }
    void set_visible(const bool& v);
    bool get_showing() const;
    /// 自分とウィンドウまでの祖先がすべて表示されるか(ウィンドウ自身は問わない)。
    bool shown_in_window() const;
    /// visible で、親に隠されていない(描く・配置する・クリックを受ける)。
    bool displayed() const { return visible_ && !hidden_by_parent_; }
    /// 親(ToolBar・Splitter・TabWidget)が自分の都合で隠す・戻す(visible は変えない。detail::WidgetAccess から使う)。
    void set_hidden_by_parent(bool hide);
    bool get_enabled() const { return enabled_; }
    void set_enabled(const bool& v);
    bool get_effectively_enabled() const;
    FocusPolicy get_focus_policy() const { return focus_policy_; }
    void set_focus_policy(const FocusPolicy& v) { focus_policy_ = v; }
    bool get_focused() const;
    void set_focused(const bool& v);
    MouseCursor get_cursor() const { return cursor_; }
    void set_cursor(const MouseCursor& v);
    bool get_accepts_text_input() const { return accepts_text_input_; }
    void set_accepts_text_input(const bool& v);
    bool get_accepts_drops() const { return accepts_drops_; }
    void set_accepts_drops(const bool& v) { accepts_drops_ = v; }
    float get_display_scale() const;
    void set_tooltip(const std::string& v) { tooltip_ = v; }
    ThemeOverride get_theme_override() const { return theme_override_; }
    void set_theme_override(const ThemeOverride& v);
    Theme get_theme() const;
    /// テーマを継ぐ元(親。ウィンドウは無し。ポップアップは開いた元のウィジェット)。
    const Widget* theme_parent() const;
    /// 自分と祖先の theme_override を out(アプリケーションのテーマ)に重ねる。上書きが 1 つも無ければ false(out は変えない)。
    bool compose_theme(Theme& out) const;
    std::vector<Action*> get_actions() const { return actions_; }

    Widget* parent_;
    std::vector<Widget*> children_;
    Rect bounds_;
    Size minimum_size_;
    Size maximum_size_;
    Layout* layout_;
    bool layout_dirty_;
    bool visible_;
    bool hidden_by_parent_;  ///< 親が自分の都合で隠している(visible_ とは別。displayed を見る)
    bool enabled_;
    bool accepts_text_input_;
    bool accepts_drops_;
    bool is_window_;
    FocusPolicy focus_policy_;
    MouseCursor cursor_;
    std::string tooltip_;
    ThemeOverride theme_override_;
    std::vector<Action*> actions_;
    PaintHandler on_paint_;
    NotifyHandler on_resize_;
    MouseHandler on_mouse_down_;
    MouseHandler on_mouse_up_;
    MouseHandler on_mouse_move_;
    NotifyHandler on_mouse_enter_;
    NotifyHandler on_mouse_leave_;
    WheelHandler on_mouse_wheel_;
    KeyHandler on_key_down_;
    KeyHandler on_key_up_;
    NotifyHandler on_focus_in_;
    NotifyHandler on_focus_out_;
    DragHandler on_drag_move_;
    NotifyHandler on_drag_leave_;
    DragHandler on_drop_;
    DragFinishHandler on_drag_finish_;
};

namespace detail
{

template <>
struct ImplAccess<Widget>
{
    TOKOTACHI_IMPL_ACCESS(Widget)

    static const std::vector<Action*>& actions_of(const Widget* w) { return of(w)->actions_; }
    static void action_changed(Widget* w, Action* a) { w->action_changed(a); }
    // 表示されるか(visible で、親に隠されていない)。
    static bool displayed(const Widget* w) { return of(w)->displayed(); }
    // ToolBar・Splitter・TabWidget が、利用者の visible を変えずに子を隠す・戻す。
    static void set_hidden_by_parent(Widget* w, bool hide) { of(w)->set_hidden_by_parent(hide); }
    static void set_visible_flag(Widget* w, bool v) { of(w)->visible_ = v; }
    static bool is_window(const Widget* w) { return of(w)->is_window_; }
    static void set_is_window(Widget* w, bool v) { of(w)->is_window_ = v; }
    static Widget* parent_of(const Widget* w) { return of(w)->parent_; }
    static const std::vector<Widget*>& children(const Widget* w) { return of(w)->children_; }
    static const Rect& bounds(const Widget* w) { return of(w)->bounds_; }
    static Window* window_of(const Widget* w) { return of(w)->get_window(); }
    static Key mnemonic_key(const Widget* w) { return w->mnemonic_key(); }
    static void mnemonic_activated(Widget* w) { w->mnemonic_activated(); }
    static bool handles_tab_key(const Widget* w) { return w->handles_tab_key(); }
    // bounds への代入と違い、親への描き直しの依頼と resized を行わない。
    static void set_bounds_raw(Widget* w, const Rect& r) { of(w)->bounds_ = r; }
    static void notify_bounds(Widget* w, const Rect& old) { of(w)->notify_bounds(old); }
    static void notify_focused(Widget* w) { w->notify_changed(w->focused); }
    static void notify_visible(Widget* w) { w->notify_changed(w->visible); }
    static Layout* layout_of(const Widget* w) { return of(w)->layout_; }
    static void set_layout(Widget* w, Layout* l) { of(w)->layout_ = l; }
    static bool layout_dirty(const Widget* w) { return of(w)->layout_dirty_; }
    static void clear_layout_dirty(Widget* w) { of(w)->layout_dirty_ = false; }
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
    static bool accepts_drops(const Widget* w) { return of(w)->accepts_drops_; }
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
        of(w)->layout_dirty_ = true;  // 寸法(余白など)が変わり、望む大きさが変わりうる
        for (std::size_t i = 0; i < of(w)->children_.size(); ++i)
        {
            theme_changed(of(w)->children_[i]);
        }
    }

    // w と子孫すべてに scale_changed を送る。
    static void scale_changed(Widget* w)
    {
        w->scale_changed();
        of(w)->layout_dirty_ = true;  // 文字の大きさ(ヒンティング)が変わり、望む大きさが変わりうる
        for (std::size_t i = 0; i < of(w)->children_.size(); ++i)
        {
            scale_changed(of(w)->children_[i]);
        }
    }
};

typedef ImplAccess<Widget> WidgetAccess;

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
