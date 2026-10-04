#include <gui/widgets/widget.hpp>

#include <algorithm>
#include <cstdio>

#include <gui/layout/layout.hpp>
#include <gui/windows/window.hpp>

#include "../layout/layout_access.hpp"
#include "../actions/action_access.hpp"
#include "../core/property_impl.hpp"
#include "widget_access.hpp"
#include "../windows/window_impl.hpp"

namespace gui
{

Widget::Widget(Widget* parent_widget)
    : Object(parent_widget),
      GUI_PROPERTY_GETTER_SELF_NOTIFY(Widget, Widget*, parent),
      GUI_READONLY_PROPERTY(Widget, std::vector<Widget*>, children),
      GUI_READONLY_PROPERTY(Widget, Window*, window),
      GUI_PROPERTY_GETTER_SELF_NOTIFY(Widget, float, x),
      GUI_PROPERTY_GETTER_SELF_NOTIFY(Widget, float, y),
      GUI_PROPERTY_GETTER_SELF_NOTIFY(Widget, float, width),
      GUI_PROPERTY_GETTER_SELF_NOTIFY(Widget, float, height),
      GUI_PROPERTY_GETTER_SELF_NOTIFY(Widget, Rect, bounds),
      GUI_READONLY_PROPERTY(Widget, Rect, local_rect),
      GUI_READONLY_PROPERTY(Widget, Size, preferred_size),
      GUI_PROPERTY_GETTER(Widget, Size, minimum_size),
      GUI_PROPERTY_GETTER(Widget, Size, maximum_size),
      GUI_READONLY_PROPERTY(Widget, Size, minimum_hint),
      GUI_READONLY_PROPERTY(Widget, Layout*, layout),
      GUI_PROPERTY_GETTER_SELF_NOTIFY(Widget, bool, visible),
      GUI_READONLY_PROPERTY(Widget, bool, showing),
      GUI_PROPERTY_GETTER_SELF_NOTIFY(Widget, bool, enabled),
      GUI_READONLY_PROPERTY(Widget, bool, effectively_enabled),
      GUI_PROPERTY_GETTER(Widget, FocusPolicy, focus_policy),
      GUI_PROPERTY_GETTER_SELF_NOTIFY(Widget, bool, focused),
      GUI_PROPERTY_GETTER(Widget, MouseCursor, cursor),
      GUI_PROPERTY_GETTER(Widget, bool, accepts_text_input),
      GUI_READONLY_PROPERTY(Widget, float, display_scale),
      GUI_PROPERTY_MEMBER(Widget, std::string, tooltip),
      GUI_READONLY_PROPERTY(Widget, std::vector<Action*>, actions),
      GUI_HANDLER_PROPERTY(Widget, PaintHandler, on_paint),
      GUI_HANDLER_PROPERTY(Widget, NotifyHandler, on_resize),
      GUI_HANDLER_PROPERTY(Widget, MouseHandler, on_mouse_down),
      GUI_HANDLER_PROPERTY(Widget, MouseHandler, on_mouse_up),
      GUI_HANDLER_PROPERTY(Widget, MouseHandler, on_mouse_move),
      GUI_HANDLER_PROPERTY(Widget, NotifyHandler, on_mouse_enter),
      GUI_HANDLER_PROPERTY(Widget, NotifyHandler, on_mouse_leave),
      GUI_HANDLER_PROPERTY(Widget, WheelHandler, on_mouse_wheel),
      GUI_HANDLER_PROPERTY(Widget, KeyHandler, on_key_down),
      GUI_HANDLER_PROPERTY(Widget, KeyHandler, on_key_up),
      GUI_HANDLER_PROPERTY(Widget, NotifyHandler, on_focus_in),
      GUI_HANDLER_PROPERTY(Widget, NotifyHandler, on_focus_out),
      parent_(parent_widget),
      maximum_size_(1e9f, 1e9f),
      layout_(nullptr),
      layout_dirty_(true),
      visible_(true),
      enabled_(true),
      accepts_text_input_(false),
      is_window_(false),
      focus_policy_(FocusPolicy::NONE),
      cursor_(MouseCursor::ARROW)
{
    if (parent_)
    {
        parent_->children_.push_back(this);
    }
}

void Widget::set_cursor(const MouseCursor& v)
{
    if (v == cursor_)
    {
        return;
    }
    cursor_ = v;
    // ポインタの下(またはドラッグ中)のウィジェットなら、すぐに OS のカーソルを変える。
    if (Window* w = get_window())
    {
        w->impl()->cursor_changed(this);
    }
}

void Widget::add_action(Action* action)
{
    if (!action || std::find(actions_.begin(), actions_.end(), action) != actions_.end())
    {
        return;
    }
    actions_.push_back(action);
    detail::ActionAccess::attach_widget(action, this);
    notify_changed(actions);
    action_added(action);
}

void Widget::remove_action(Action* action)
{
    const std::vector<Action*>::iterator it = std::find(actions_.begin(), actions_.end(), action);
    if (it == actions_.end())
    {
        return;
    }
    actions_.erase(it);
    detail::ActionAccess::detach_widget(action, this);
    notify_changed(actions);
    action_removed(action);
}

Widget::~Widget()
{
    for (std::size_t i = 0; i < actions_.size(); ++i)
    {
        detail::ActionAccess::detach_widget(actions_[i], this);
    }
    actions_.clear();
    // 子(所有しているオブジェクト)は、自分がまだ Widget であるうちに破棄する(子は親の children_ を書き換える)。
    delete_owned();
    if (parent_ && parent_->layout_)
    {
        detail::LayoutAccess::widget_removed(parent_->layout_, this);
        parent_->request_layout();
    }
    if (Window* w = get_window())
    {
        w->impl()->widget_destroyed(this);
        if (parent_ && visible_)
        {
            parent_->invalidate(bounds_);
        }
    }
    if (parent_)
    {
        std::vector<Widget*>& siblings = parent_->children_;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
        parent_->child_removed(this);
    }
}

void Widget::set_parent(Widget* const& p)
{
    if (p == parent_)
    {
        return;
    }
    if (is_window_)
    {
        std::fprintf(stderr, "gui: a window cannot have a parent (ignored)\n");
        return;
    }
    for (const Widget* a = p; a; a = a->parent_)
    {
        if (a == this)
        {
            std::fprintf(stderr, "gui: cannot make a descendant the parent (ignored)\n");
            return;
        }
    }
    Window* old_window = get_window();
    // 移る前の親から外す(レイアウト・表示のツリー・描画)。
    if (parent_)
    {
        if (parent_->layout_)
        {
            detail::LayoutAccess::widget_removed(parent_->layout_, this);
        }
        parent_->request_layout();
        if (visible_)
        {
            parent_->invalidate(bounds_);
        }
        std::vector<Widget*>& siblings = parent_->children_;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
        parent_->child_removed(this);
    }
    // 移る前のウィンドウが持つ、自分と子孫への参照(フォーカス・ホバー・捕捉)を外す。
    if (old_window)
    {
        old_window->impl()->widget_hidden(this);
    }
    parent_ = p;
    change_owner(p);
    if (parent_)
    {
        parent_->children_.push_back(this);
    }
    Window* new_window = get_window();
    if (new_window && new_window != old_window)
    {
        scale_changed_tree(this);  // 倍率の違うウィンドウへ移ったかもしれない
    }
    request_layout();
    invalidate();
    notify_changed(parent);
}

void Widget::scale_changed_tree(Widget* w)
{
    w->scale_changed();
    w->layout_dirty_ = true;
    for (std::size_t i = 0; i < w->children_.size(); ++i)
    {
        scale_changed_tree(w->children_[i]);
    }
}

Window* Widget::get_window() const
{
    const Widget* w = this;
    while (w->parent_)
    {
        w = w->parent_;
    }
    return w->is_window_ ? static_cast<Window*>(const_cast<Widget*>(w)) : nullptr;
}

void Widget::set_x(const float& v)
{
    Rect r = bounds_;
    r.x = v;
    set_bounds(r);
}

void Widget::set_y(const float& v)
{
    Rect r = bounds_;
    r.y = v;
    set_bounds(r);
}

void Widget::set_width(const float& v)
{
    Rect r = bounds_;
    r.width = v;
    set_bounds(r);
}

void Widget::set_height(const float& v)
{
    Rect r = bounds_;
    r.height = v;
    set_bounds(r);
}

void Widget::set_bounds(const Rect& r)
{
    if (is_window_)
    {
        // ウィンドウの大きさの変更は SDL3 に依頼し、実際の大きさを反映する(位置は扱わない。docs/adr/0002)。
        static_cast<Window*>(this)->impl()->request_size(r.size());
        return;
    }
    const Rect old = bounds_;
    if (r == old)
    {
        return;
    }
    bounds_ = r;
    if (parent_ && visible_)
    {
        parent_->invalidate(old.united(r));
    }
    // ハンドラ(on_resize)より先に、結びつけた相手へ伝える。
    notify_bounds(old);
    if (old.size() != r.size())
    {
        mark_layout_dirty();
        resized();
    }
}

void Widget::notify_bounds(const Rect& old)
{
    const Rect& r = bounds_;
    if (old.x != r.x)
    {
        notify_changed(x);
    }
    if (old.y != r.y)
    {
        notify_changed(y);
    }
    if (old.width != r.width)
    {
        notify_changed(width);
    }
    if (old.height != r.height)
    {
        notify_changed(height);
    }
    if (old != r)
    {
        notify_changed(bounds);
    }
}

void Widget::set_visible(const bool& v)
{
    if (v == visible_)
    {
        return;
    }
    if (is_window_)
    {
        static_cast<Window*>(this)->impl()->set_shown(v);
        return;
    }
    if (parent_)
    {
        parent_->invalidate(bounds_);
    }
    visible_ = v;
    if (parent_)
    {
        parent_->request_layout();  // 隠れた子は配置しない
    }
    // フォーカスのハンドラより先に、結びつけた相手へ伝える。
    notify_changed(visible);
    if (Window* w = get_window())
    {
        if (!v)
        {
            w->impl()->widget_hidden(this);
        }
        else
        {
            // フォーカスのあるウィジェット(またはその祖先)が表示されたら、文字入力を有効にするかを判断し直す。
            w->impl()->update_text_input();
        }
    }
}

bool Widget::shown_in_window() const
{
    // ウィンドウ自身が表示されているかは問わない(表示の前にフォーカスを決められるように)。
    for (const Widget* w = this; w && !w->is_window_; w = w->parent_)
    {
        if (!w->visible_)
        {
            return false;
        }
    }
    return true;
}

bool Widget::get_showing() const
{
    for (const Widget* w = this; w; w = w->parent_)
    {
        if (!w->visible_)
        {
            return false;
        }
    }
    return true;
}

void Widget::set_enabled(const bool& v)
{
    if (v == enabled_)
    {
        return;
    }
    enabled_ = v;
    invalidate();
    // フォーカスのハンドラより先に、結びつけた相手へ伝える。
    notify_changed(enabled);
    if (!v)
    {
        if (Window* w = get_window())
        {
            w->impl()->widget_hidden(this);
        }
    }
}

bool Widget::get_effectively_enabled() const
{
    for (const Widget* w = this; w; w = w->parent_)
    {
        if (!w->enabled_)
        {
            return false;
        }
    }
    return true;
}

bool Widget::get_focused() const
{
    Window* w = get_window();
    return w && w->impl()->focus == this;
}

void Widget::set_focused(const bool& v)
{
    // フォーカスが移ると、ウィンドウが移る前と後のウィジェットの focused を知らせる。
    Window* w = get_window();
    const bool before = get_focused();
    if (w && v && !before && focus_policy_ != FocusPolicy::NONE && get_effectively_enabled() && shown_in_window())
    {
        w->impl()->set_focus(this);
    }
    else if (w && !v && before)
    {
        w->impl()->set_focus(nullptr);
    }
}

void Widget::set_accepts_text_input(const bool& v)
{
    if (v == accepts_text_input_)
    {
        return;
    }
    accepts_text_input_ = v;
    if (get_focused())
    {
        get_window()->impl()->update_text_input();
    }
}

float Widget::get_display_scale() const
{
    Window* w = get_window();
    return w ? w->impl()->scale : 1.0f;
}

Size Widget::get_minimum_hint() const
{
    const Size c = compute_minimum_size();
    return Size(std::max(c.width, minimum_size_.width), std::max(c.height, minimum_size_.height));
}

Size Widget::get_preferred_size() const
{
    const Size p = compute_preferred_size();
    const Size m = get_minimum_hint();
    return Size(std::max(m.width, std::min(p.width, maximum_size_.width)),
                std::max(m.height, std::min(p.height, maximum_size_.height)));
}

void Widget::set_minimum_size(const Size& v)
{
    minimum_size_ = v;
    request_layout();
}

void Widget::set_maximum_size(const Size& v)
{
    maximum_size_ = v;
    request_layout();
}

void Widget::request_layout()
{
    // 自分の望む大きさが変わると、祖先の望む大きさも変わりうるので、根まで印を付ける。
    for (Widget* w = this; w; w = w->parent_)
    {
        w->layout_dirty_ = true;
    }
    if (Window* w = get_window())
    {
        w->impl()->layout_pending = true;
    }
}

void Widget::mark_layout_dirty()
{
    layout_dirty_ = true;
    if (Window* w = get_window())
    {
        w->impl()->layout_pending = true;
    }
}

void Widget::apply_layout()
{
    if (Window* w = get_window())
    {
        w->impl()->run_layout();
    }
}

void Widget::invalidate() { invalidate(get_local_rect()); }

void Widget::invalidate(const Rect& r)
{
    Window* w = get_window();
    if (!w || !get_showing())
    {
        return;
    }
    const Point origin = map_to_window(Point());
    w->impl()->invalidate(r.intersected(get_local_rect()).translated(origin.x, origin.y));
}

Point Widget::map_to_window(const Point& p) const
{
    Point out = p;
    for (const Widget* w = this; w && !w->is_window_; w = w->parent_)
    {
        out = out + w->bounds_.position();
    }
    return out;
}

Point Widget::map_from_window(const Point& p) const { return p - map_to_window(Point()); }

Widget* Widget::child_at(const Point& p) const
{
    for (std::size_t i = children_.size(); i-- > 0;)
    {
        Widget* c = children_[i];
        if (!c->visible_ || !c->bounds_.contains(p))
        {
            continue;
        }
        Widget* deeper = c->child_at(p - c->bounds_.position());
        return deeper ? deeper : c;
    }
    return nullptr;
}

void Widget::update_input_method()
{
    if (get_focused())
    {
        get_window()->impl()->ime_dirty = true;
    }
}

// --- 既定の実装: ハンドラを呼ぶ。マウスのボタン・キーは、ハンドラが無ければ親へ送る(docs/adr/0013) ---

void Widget::paint(Canvas& canvas)
{
    Object* self = this;
    detail::fire(on_paint_, self, canvas);
}

void Widget::resized()
{
    Object* self = this;
    detail::fire(on_resize_, self);
}

void Widget::mouse_down(MouseEvent& e)
{
    Object* self = this;
    if (on_mouse_down_)
    {
        detail::fire(on_mouse_down_, self, e);
    }
    else
    {
        e.accepted = false;
    }
}

void Widget::mouse_up(MouseEvent& e)
{
    Object* self = this;
    if (on_mouse_up_)
    {
        detail::fire(on_mouse_up_, self, e);
    }
    else
    {
        e.accepted = false;
    }
}

void Widget::mouse_move(MouseEvent& e)
{
    Object* self = this;
    detail::fire(on_mouse_move_, self, e);
}

void Widget::mouse_enter()
{
    Object* self = this;
    detail::fire(on_mouse_enter_, self);
}

void Widget::mouse_leave()
{
    Object* self = this;
    detail::fire(on_mouse_leave_, self);
}

void Widget::mouse_wheel(WheelEvent& e)
{
    Object* self = this;
    if (on_mouse_wheel_)
    {
        detail::fire(on_mouse_wheel_, self, e);
    }
    else
    {
        e.accepted = false;
    }
}

void Widget::arrange_children()
{
    if (layout_)
    {
        detail::LayoutAccess::arrange(layout_, get_local_rect());
    }
}

void Widget::key_down(KeyEvent& e)
{
    Object* self = this;
    if (on_key_down_)
    {
        detail::fire(on_key_down_, self, e);
    }
    else
    {
        e.accepted = false;
    }
}

void Widget::key_up(KeyEvent& e)
{
    Object* self = this;
    if (on_key_up_)
    {
        detail::fire(on_key_up_, self, e);
    }
    else
    {
        e.accepted = false;
    }
}

void Widget::text_input(TextInputEvent& e) { e.accepted = false; }

void Widget::text_editing(TextEditingEvent& e) { e.accepted = false; }

void Widget::focus_in()
{
    Object* self = this;
    detail::fire(on_focus_in_, self);
}

void Widget::focus_out()
{
    Object* self = this;
    detail::fire(on_focus_out_, self);
}

void Widget::scale_changed() {}

void Widget::theme_changed() {}

void Widget::child_removed(Widget*) {}

void Widget::action_added(Action*) {}

void Widget::action_removed(Action*) {}

void Widget::action_changed(Action*) {}

Rect Widget::input_method_rect() const { return get_local_rect(); }

Size Widget::compute_preferred_size() const
{
    return layout_ ? detail::LayoutAccess::preferred_size(layout_) : Size();
}

Size Widget::compute_minimum_size() const
{
    return layout_ ? detail::LayoutAccess::minimum_size(layout_) : Size();
}

}  // namespace gui
