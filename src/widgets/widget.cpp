#include <tokotachi/widgets/widget.hpp>

#include <algorithm>
#include <cstdio>

#include <tokotachi/layout/layout.hpp>
#include <tokotachi/windows/popup.hpp>
#include <tokotachi/windows/window.hpp>

#include "../core/object_impl.hpp"
#include "../layout/layout_access.hpp"
#include "../windows/window_impl.hpp"
#include "../actions/action_access.hpp"
#include "../core/impl.hpp"
#include "theme.hpp"
#include "widget_access.hpp"
#include "../windows/window_impl.hpp"

namespace tokotachi
{

Widget::Widget(Widget* parent_widget)
    : Object(parent_widget),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, Widget*, parent),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, std::vector<Widget*>, children),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, Window*, window),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, float, x),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, float, y),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, float, width),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, float, height),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, Rect, bounds),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, Rect, local_rect),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, Size, preferred_size),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, Size, minimum_size),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, Size, maximum_size),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, Size, minimum_hint),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, Layout*, layout),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, bool, visible),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, bool, showing),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, bool, enabled),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, bool, effectively_enabled),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, FocusPolicy, focus_policy),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, bool, focused),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, MouseCursor, cursor),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, bool, accepts_text_input),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, float, display_scale),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, std::string, tooltip),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, ThemeOverride, theme_override),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, Theme, theme),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, std::vector<Action*>, actions),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, bool, accepts_drops),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, DragHandler, on_drag_move),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_drag_leave),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, DragHandler, on_drop),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, DragFinishHandler, on_drag_finish),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, PaintHandler, on_paint),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_resize),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, MouseHandler, on_mouse_down),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, MouseHandler, on_mouse_up),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, MouseHandler, on_mouse_move),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_mouse_enter),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_mouse_leave),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, WheelHandler, on_mouse_wheel),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, KeyHandler, on_key_down),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, KeyHandler, on_key_up),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_focus_in),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_focus_out),
      d_(new Impl(this))
{
    d_->install(this);
    d_->parent_ = parent_widget;
    d_->maximum_size_ = Size(1e9f, 1e9f);
    d_->layout_ = nullptr;
    d_->layout_dirty_ = true;
    d_->visible_ = true;
    d_->hidden_by_parent_ = false;
    d_->enabled_ = true;
    d_->accepts_text_input_ = false;
    d_->accepts_drops_ = false;
    d_->is_window_ = false;
    d_->focus_policy_ = FocusPolicy::NONE;
    d_->cursor_ = MouseCursor::ARROW;
    if (d_->parent_)
    {
        d_->parent_->d_->children_.push_back(this);
    }
}

void Widget::Impl::set_cursor(const MouseCursor& v)
{
    if (v == cursor_)
    {
        return;
    }
    cursor_ = v;
    // ポインタの下(またはドラッグ中)のウィジェットなら、すぐに OS のカーソルを変える。
    if (Window* w = get_window())
    {
        detail::window_impl_of(w)->cursor_changed(q);
    }
}

void Widget::add_action(Action* action)
{
    if (!action || std::find(d_->actions_.begin(), d_->actions_.end(), action) != d_->actions_.end())
    {
        return;
    }
    d_->actions_.push_back(action);
    detail::ActionAccess::attach_widget(action, this);
    notify_changed(actions);
    d_->hooks_->action_added(action);
}

void Widget::remove_action(Action* action)
{
    const std::vector<Action*>::iterator it = std::find(d_->actions_.begin(), d_->actions_.end(), action);
    if (it == d_->actions_.end())
    {
        return;
    }
    d_->actions_.erase(it);
    detail::ActionAccess::detach_widget(action, this);
    notify_changed(actions);
    d_->hooks_->action_removed(action);
}

Widget::~Widget()
{
    for (std::size_t i = 0; i < d_->actions_.size(); ++i)
    {
        detail::ActionAccess::detach_widget(d_->actions_[i], this);
    }
    d_->actions_.clear();
    // 子(所有しているオブジェクト)は、自分がまだ Widget であるうちに破棄する(子は親の children_ を書き換える)。
    delete_owned();
    if (d_->parent_ && d_->parent_->d_->layout_)
    {
        detail::LayoutAccess::widget_removed(d_->parent_->d_->layout_, this);
        d_->parent_->request_layout();
    }
    if (Window* w = d_->get_window())
    {
        detail::window_impl_of(w)->widget_destroyed(this);
        if (d_->parent_ && d_->displayed())
        {
            d_->parent_->invalidate(d_->bounds_);
        }
    }
    if (d_->parent_)
    {
        std::vector<Widget*>& siblings = d_->parent_->d_->children_;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
        d_->parent_->d_->hooks_->child_removed(this);
    }
    delete d_;
}

void Widget::Impl::set_parent(Widget* const& p)
{
    if (p == parent_)
    {
        return;
    }
    if (is_window_)
    {
        std::fprintf(stderr, "tokotachi: a window cannot have a parent (ignored)\n");
        return;
    }
    for (const Widget* a = p; a; a = a->d_->parent_)
    {
        if (a == q)
        {
            std::fprintf(stderr, "tokotachi: cannot make a descendant the parent (ignored)\n");
            return;
        }
    }
    Window* old_window = get_window();
    // 移る前の親から外す(レイアウト・表示のツリー・描画)。
    if (parent_)
    {
        if (parent_->d_->layout_)
        {
            detail::LayoutAccess::widget_removed(parent_->d_->layout_, q);
        }
        parent_->request_layout();
        if (displayed())
        {
            parent_->invalidate(bounds_);
        }
        std::vector<Widget*>& siblings = parent_->d_->children_;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), q), siblings.end());
        parent_->d_->hooks_->child_removed(q);
    }
    // 移る前のウィンドウが持つ、自分と子孫への参照(フォーカス・ホバー・捕捉)を外す。
    if (old_window)
    {
        detail::window_impl_of(old_window)->widget_hidden(q);
    }
    parent_ = p;
    detail::change_owner(q, p);
    if (parent_)
    {
        parent_->d_->children_.push_back(q);
    }
    Window* new_window = get_window();
    if (new_window && new_window != old_window)
    {
        scale_changed_tree(q);  // 倍率の違うウィンドウへ移ったかもしれない
    }
    q->request_layout();
    q->invalidate();
    q->notify_changed(q->parent);
}

void Widget::Impl::scale_changed_tree(Widget* w)
{
    w->scale_changed();
    w->d_->layout_dirty_ = true;
    for (std::size_t i = 0; i < w->d_->children_.size(); ++i)
    {
        scale_changed_tree(w->d_->children_[i]);
    }
}

Window* Widget::Impl::get_window() const
{
    const Widget* w = q;
    while (w->d_->parent_)
    {
        w = w->d_->parent_;
    }
    return w->d_->is_window_ ? static_cast<Window*>(const_cast<Widget*>(w)) : nullptr;
}

void Widget::Impl::set_x(const float& v)
{
    Rect r = bounds_;
    r.x = v;
    set_bounds(r);
}

void Widget::Impl::set_y(const float& v)
{
    Rect r = bounds_;
    r.y = v;
    set_bounds(r);
}

void Widget::Impl::set_width(const float& v)
{
    Rect r = bounds_;
    r.width = v;
    set_bounds(r);
}

void Widget::Impl::set_height(const float& v)
{
    Rect r = bounds_;
    r.height = v;
    set_bounds(r);
}

void Widget::Impl::set_bounds(const Rect& r)
{
    if (is_window_)
    {
        // ウィンドウの大きさの変更は SDL3 に依頼し、実際の大きさを反映する(位置は扱わない。docs/adr/0002)。
        detail::window_impl_of(static_cast<Window*>(q))->request_size(r.size());
        return;
    }
    const Rect old = bounds_;
    if (r == old)
    {
        return;
    }
    bounds_ = r;
    if (parent_ && displayed())
    {
        parent_->invalidate(old.united(r));
    }
    // ハンドラ(on_resize)より先に、結びつけた相手へ伝える。
    notify_bounds(old);
    if (old.size() != r.size())
    {
        mark_layout_dirty();
        q->resized();
    }
}

void Widget::Impl::notify_bounds(const Rect& old)
{
    const Rect& r = bounds_;
    if (old.x != r.x)
    {
        q->notify_changed(q->x);
    }
    if (old.y != r.y)
    {
        q->notify_changed(q->y);
    }
    if (old.width != r.width)
    {
        q->notify_changed(q->width);
    }
    if (old.height != r.height)
    {
        q->notify_changed(q->height);
    }
    if (old != r)
    {
        q->notify_changed(q->bounds);
    }
}

void Widget::Impl::set_visible(const bool& v)
{
    if (v == visible_)
    {
        return;
    }
    if (is_window_)
    {
        detail::window_impl_of(static_cast<Window*>(q))->set_shown(v);
        return;
    }
    if (parent_ && !hidden_by_parent_)
    {
        parent_->invalidate(bounds_);
    }
    visible_ = v;
    if (parent_)
    {
        parent_->request_layout();  // 隠れた子は配置しない(ToolBar・Splitter は場所を空けない)
    }
    // フォーカスのハンドラより先に、結びつけた相手へ伝える。
    q->notify_changed(q->visible);
    if (Window* w = get_window())
    {
        if (!v)
        {
            detail::window_impl_of(w)->widget_hidden(q);
        }
        else
        {
            // フォーカスのあるウィジェット(またはその祖先)が表示されたら、文字入力を有効にするかを判断し直す。
            detail::window_impl_of(w)->update_text_input();
        }
    }
}

void Widget::Impl::set_theme_override(const ThemeOverride& v)
{
    if (v == theme_override_)
    {
        return;
    }
    theme_override_ = v;
    q->notify_changed(q->theme_override);
    detail::WidgetAccess::theme_changed(q);  // 自分と子孫へ(寸法が変わりうるので、配置もやり直す)
    q->request_layout();
    q->invalidate();
}

const Widget* Widget::Impl::theme_parent() const
{
    if (parent_)
    {
        return parent_;
    }
    const Popup* popup = is_window_ ? dynamic_cast<const Popup*>(q) : nullptr;
    return popup ? popup->owner_widget.get() : nullptr;
}

bool Widget::Impl::compose_theme(Theme& out) const
{
    std::vector<const ThemeOverride*> chain;  // 自分から根へ
    for (const Widget* w = q; w; w = w->d_->theme_parent())
    {
        if (!w->d_->theme_override_.empty())
        {
            chain.push_back(&w->d_->theme_override_);
        }
    }
    for (std::size_t i = chain.size(); i-- > 0;)
    {
        chain[i]->apply_to(out);  // 根に近いものから重ねる
    }
    return !chain.empty();
}

Theme Widget::Impl::get_theme() const
{
    Theme t = detail::application_theme();
    compose_theme(t);
    return t;
}

namespace detail
{

ThemeScope::ThemeScope(const Widget* w) : theme_(application_theme()), previous_(nullptr)
{
    previous_ = swap_scoped_theme(w && ImplAccess<Widget>::of(w)->compose_theme(theme_) ? &theme_ : nullptr);
}

}  // namespace detail

void Widget::Impl::set_hidden_by_parent(bool hide)
{
    if (hide == hidden_by_parent_)
    {
        return;
    }
    const bool was = displayed();
    hidden_by_parent_ = hide;
    if (was == displayed())
    {
        return;  // 利用者が隠しているので、見え方は変わらない
    }
    if (parent_)
    {
        parent_->invalidate(bounds_);
    }
    if (Window* w = get_window())
    {
        if (hide)
        {
            detail::window_impl_of(w)->widget_hidden(q);
        }
        else
        {
            detail::window_impl_of(w)->update_text_input();
        }
    }
}

bool Widget::Impl::shown_in_window() const
{
    // ウィンドウ自身が表示されているかは問わない(表示の前にフォーカスを決められるように)。
    for (const Widget* w = q; w && !w->d_->is_window_; w = w->d_->parent_)
    {
        if (!w->d_->displayed())
        {
            return false;
        }
    }
    return true;
}

bool Widget::Impl::get_showing() const
{
    for (const Widget* w = q; w; w = w->d_->parent_)
    {
        if (!w->d_->displayed())
        {
            return false;
        }
    }
    return true;
}

void Widget::Impl::set_enabled(const bool& v)
{
    if (v == enabled_)
    {
        return;
    }
    enabled_ = v;
    q->invalidate();
    // フォーカスのハンドラより先に、結びつけた相手へ伝える。
    q->notify_changed(q->enabled);
    if (!v)
    {
        if (Window* w = get_window())
        {
            detail::window_impl_of(w)->widget_hidden(q);
        }
    }
}

bool Widget::Impl::get_effectively_enabled() const
{
    for (const Widget* w = q; w; w = w->d_->parent_)
    {
        if (!w->d_->enabled_)
        {
            return false;
        }
    }
    return true;
}

bool Widget::Impl::get_focused() const
{
    Window* w = get_window();
    return w && detail::window_impl_of(w)->focus == q;
}

void Widget::Impl::set_focused(const bool& v)
{
    // フォーカスが移ると、ウィンドウが移る前と後のウィジェットの focused を知らせる。
    Window* w = get_window();
    const bool before = get_focused();
    if (w && v && !before && focus_policy_ != FocusPolicy::NONE && get_effectively_enabled() && shown_in_window())
    {
        detail::window_impl_of(w)->set_focus(q);
    }
    else if (w && !v && before)
    {
        detail::window_impl_of(w)->set_focus(nullptr);
    }
}

void Widget::Impl::set_accepts_text_input(const bool& v)
{
    if (v == accepts_text_input_)
    {
        return;
    }
    accepts_text_input_ = v;
    if (get_focused())
    {
        detail::window_impl_of(get_window())->update_text_input();
    }
}

float Widget::Impl::get_display_scale() const
{
    Window* w = get_window();
    return w ? detail::window_impl_of(w)->scale : 1.0f;
}

Size Widget::Impl::get_minimum_hint() const
{
    detail::ThemeScope scope(q);  // 寸法(余白・角の丸み)は自分に効いているテーマで決める
    const Size c = q->compute_minimum_size();
    return Size(std::max(c.width, minimum_size_.width), std::max(c.height, minimum_size_.height));
}

Size Widget::Impl::get_preferred_size() const
{
    detail::ThemeScope scope(q);
    const Size p = q->compute_preferred_size();
    const Size m = get_minimum_hint();
    return Size(std::max(m.width, std::min(p.width, maximum_size_.width)),
                std::max(m.height, std::min(p.height, maximum_size_.height)));
}

void Widget::Impl::set_minimum_size(const Size& v)
{
    minimum_size_ = v;
    q->request_layout();
}

void Widget::Impl::set_maximum_size(const Size& v)
{
    maximum_size_ = v;
    q->request_layout();
}

void Widget::request_layout()
{
    // 自分の望む大きさが変わると、祖先の望む大きさも変わりうるので、根まで印を付ける。
    for (Widget* w = this; w; w = w->d_->parent_)
    {
        w->d_->layout_dirty_ = true;
    }
    if (Window* w = d_->get_window())
    {
        detail::window_impl_of(w)->layout_pending = true;
    }
}

void Widget::Impl::mark_layout_dirty()
{
    layout_dirty_ = true;
    if (Window* w = get_window())
    {
        detail::window_impl_of(w)->layout_pending = true;
    }
}

void Widget::apply_layout()
{
    if (Window* w = d_->get_window())
    {
        detail::window_impl_of(w)->run_layout();
    }
}

void Widget::invalidate() { invalidate(d_->get_local_rect()); }

void Widget::invalidate(const Rect& r)
{
    Window* w = d_->get_window();
    if (!w || !d_->get_showing())
    {
        return;
    }
    const Point origin = map_to_window(Point());
    detail::window_impl_of(w)->invalidate(r.intersected(d_->get_local_rect()).translated(origin.x, origin.y));
}

Point Widget::map_to_window(const Point& p) const
{
    Point out = p;
    for (const Widget* w = this; w && !w->d_->is_window_; w = w->d_->parent_)
    {
        out = out + w->d_->bounds_.position();
    }
    return out;
}

Point Widget::map_from_window(const Point& p) const { return p - map_to_window(Point()); }

Widget* Widget::child_at(const Point& p) const
{
    for (std::size_t i = d_->children_.size(); i-- > 0;)
    {
        Widget* c = d_->children_[i];
        if (!c->d_->displayed() || !c->d_->bounds_.contains(p))
        {
            continue;
        }
        Widget* deeper = c->child_at(p - c->d_->bounds_.position());
        return deeper ? deeper : c;
    }
    return nullptr;
}

void Widget::Impl::update_input_method()
{
    if (get_focused())
    {
        detail::window_impl_of(get_window())->ime_dirty = true;
    }
}

// --- 既定の実装: ハンドラを呼ぶ。マウスのボタン・キーは、ハンドラが無ければ親へ送る(docs/adr/0013) ---

void Widget::paint(Canvas& canvas)
{
    Object* self = this;
    detail::fire(d_->on_paint_, self, canvas);
}

void Widget::resized()
{
    Object* self = this;
    detail::fire(d_->on_resize_, self);
}

void Widget::mouse_down(MouseEvent& e)
{
    Object* self = this;
    if (d_->on_mouse_down_)
    {
        detail::fire(d_->on_mouse_down_, self, e);
    }
    else
    {
        e.accepted = false;
    }
}

void Widget::mouse_up(MouseEvent& e)
{
    Object* self = this;
    if (d_->on_mouse_up_)
    {
        detail::fire(d_->on_mouse_up_, self, e);
    }
    else
    {
        e.accepted = false;
    }
}

void Widget::mouse_move(MouseEvent& e)
{
    Object* self = this;
    detail::fire(d_->on_mouse_move_, self, e);
}

void Widget::mouse_enter()
{
    Object* self = this;
    detail::fire(d_->on_mouse_enter_, self);
}

void Widget::mouse_leave()
{
    Object* self = this;
    detail::fire(d_->on_mouse_leave_, self);
}

void Widget::mouse_wheel(WheelEvent& e)
{
    Object* self = this;
    if (d_->on_mouse_wheel_)
    {
        detail::fire(d_->on_mouse_wheel_, self, e);
    }
    else
    {
        e.accepted = false;
    }
}

bool Widget::start_drag(const DragData& data, const std::string& label)
{
    Window* w = d_->get_window();
    return w && detail::window_impl_of(w)->start_drag(this, data, label);
}

void Widget::drag_move(DragEvent& e)
{
    Object* self = this;
    if (d_->on_drag_move_)
    {
        detail::fire(d_->on_drag_move_, self, e);
    }
    else
    {
        e.accepted = static_cast<bool>(d_->on_drop_);
    }
}

void Widget::drag_leave()
{
    Object* self = this;
    detail::fire(d_->on_drag_leave_, self);
}

void Widget::drop(DragEvent& e)
{
    Object* self = this;
    if (d_->on_drop_)
    {
        detail::fire(d_->on_drop_, self, e);
    }
    else
    {
        e.accepted = false;
    }
}

void Widget::drag_finished(DropAction action)
{
    Object* self = this;
    detail::fire(d_->on_drag_finish_, self, action);
}

void Widget::arrange_children()
{
    if (d_->layout_)
    {
        detail::LayoutAccess::arrange(d_->layout_, d_->get_local_rect());
    }
}

void Widget::key_down(KeyEvent& e)
{
    Object* self = this;
    if (d_->on_key_down_)
    {
        detail::fire(d_->on_key_down_, self, e);
    }
    else
    {
        e.accepted = false;
    }
}

void Widget::key_up(KeyEvent& e)
{
    Object* self = this;
    if (d_->on_key_up_)
    {
        detail::fire(d_->on_key_up_, self, e);
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
    detail::fire(d_->on_focus_in_, self);
}

void Widget::focus_out()
{
    Object* self = this;
    detail::fire(d_->on_focus_out_, self);
}

void Widget::scale_changed() {}

void Widget::theme_changed() {}

namespace detail
{

WidgetHooks::~WidgetHooks()
{
    if (widget_)
    {
        ImplAccess<Widget>::Impl* d = ImplAccess<Widget>::of(widget_);
        // Widget 自身の実装のクラスは、破棄されるときには外さない(フックを呼ぶものはもう無い)
        if (d && static_cast<WidgetHooks*>(d) != this && d->hooks_ == this)
        {
            d->hooks_ = base_;
        }
    }
}

void WidgetHooks::install(Widget* w)
{
    widget_ = w;
    ImplAccess<Widget>::Impl* d = ImplAccess<Widget>::of(w);
    base_ = d->hooks_;
    d->hooks_ = this;
}

void WidgetHooks::child_removed(Widget* child)
{
    if (base_)
    {
        base_->child_removed(child);
    }
}

void WidgetHooks::action_added(Action* action)
{
    if (base_)
    {
        base_->action_added(action);
    }
}

void WidgetHooks::action_removed(Action* action)
{
    if (base_)
    {
        base_->action_removed(action);
    }
}

void WidgetHooks::action_changed(Action* action)
{
    if (base_)
    {
        base_->action_changed(action);
    }
}

Key WidgetHooks::mnemonic_key() const { return base_ ? base_->mnemonic_key() : Key::UNKNOWN; }

void WidgetHooks::mnemonic_activated()
{
    if (base_)
    {
        base_->mnemonic_activated();
    }
}

bool WidgetHooks::handles_tab_key() const { return base_ && base_->handles_tab_key(); }

Rect WidgetHooks::input_method_rect() const { return base_ ? base_->input_method_rect() : widget_->local_rect.get(); }

}  // namespace detail

bool Widget::Impl::mnemonic_cues() const
{
    const Window* w = get_window();
    return w && detail::window_impl_of(w)->cues;
}

bool Widget::focus_next_after()
{
    Window* w = d_->get_window();
    return w && detail::window_impl_of(w)->focus_after(this);
}

bool Widget::focus_first_within()
{
    Window* w = d_->get_window();
    return w && detail::window_impl_of(w)->focus_first_in(this);
}

Size Widget::compute_preferred_size() const
{
    return d_->layout_ ? detail::LayoutAccess::preferred_size(d_->layout_) : Size();
}

Size Widget::compute_minimum_size() const
{
    return d_->layout_ ? detail::LayoutAccess::minimum_size(d_->layout_) : Size();
}

}  // namespace tokotachi
