#include <tokotachi/windows/tool_bar.hpp>

#include <algorithm>
#include <cmath>

#include <tokotachi/graphics/canvas.hpp>
#include <tokotachi/actions/menu.hpp>
#include <tokotachi/graphics/text_layout.hpp>

#include "../actions/action_access.hpp"
#include "../core/property_impl.hpp"
#include "../actions/mnemonic.hpp"
#include "../widgets/theme.hpp"
#include "../widgets/widget_access.hpp"

namespace tokotachi
{

namespace
{
const float BAR_PADDING = 4;   // ツールバーの左右と上下の余白(dp)
const float SPACING = 2;       // 項目の間
const float ICON = 16;         // ボタンの画像の大きさ
const float BUTTON_PADDING = 8;
const float ICON_TEXT_GAP = 6;
const float SEPARATOR = 9;     // 区切りの幅

// 利用者が隠した部品(visible が false)。場所を取らない。
bool user_hidden(const Widget* item) { return item && !item->visible.get(); }
}  // namespace

namespace detail
{

// ツールバーのボタン(内部)。Action を実行する。action が無ければ » のボタン(on_press を呼ぶ)。
class ToolButton : public Widget
{
public:
    ToolButton(ToolBar* bar, Action* action_value, ToolButtonStyle style_value)
        : Widget(bar), action(action_value), style(style_value), hovered_(false), pressed_(false)
    {
        update();
    }

    Action* action;
    ToolButtonStyle style;
    std::function<void()> on_press;

    // ボタンの文字。メニュー向けの表記(末尾の「...」・日本語などの「(&S)」の形のニーモニック)は省く。
    std::string label() const
    {
        if (!action)
        {
            return "\xC2\xBB";  // »
        }
        std::string s = ActionAccess::text(action);
        if (s.size() >= 3 && s.compare(s.size() - 3, 3, "...") == 0)
        {
            s.erase(s.size() - 3);
        }
        if (s.size() >= 4 && s[s.size() - 1] == ')' && s[s.size() - 4] == '(' && s[s.size() - 3] == '&')
        {
            s.erase(s.size() - 4);
        }
        return parse_mnemonic(s).text;
    }

    bool shows_icon() const { return action && !ActionAccess::icon(action).is_null(); }

    bool shows_text() const { return !shows_icon() || style != ToolButtonStyle::ICON_ONLY; }

    // Action の状態をボタンに写す。
    void update()
    {
        if (!action)
        {
            enabled = true;
            return;
        }
        enabled = ActionAccess::enabled(action);
        std::string tip = ActionAccess::tooltip(action);
        if (tip.empty())
        {
            tip = label();
            const std::string sc = ActionAccess::shortcut(action).to_string();
            if (!sc.empty())
            {
                tip += " (" + sc + ")";
            }
        }
        tooltip = tip;
        request_layout();
        invalidate();
    }

protected:
    Size compute_preferred_size() const override
    {
        const TextLayout l(label(), Font(), display_scale);
        const float tw = std::ceil(l.width()), th = std::ceil(l.height());
        const bool icon = shows_icon(), text = shows_text();
        if (icon && !text)
        {
            return Size(ICON + BUTTON_PADDING * 2, ICON + BUTTON_PADDING * 2);
        }
        if (icon && style == ToolButtonStyle::TEXT_UNDER_ICON)
        {
            return Size(std::max(ICON, tw) + BUTTON_PADDING * 2, ICON + 4 + th + 12);
        }
        const float h = std::max(ICON, th) + BUTTON_PADDING * 2;
        return Size((icon ? ICON + ICON_TEXT_GAP : 0) + tw + BUTTON_PADDING * 2, h);
    }

    void paint(Canvas& canvas) override
    {
        const Rect r = local_rect;
        const bool enabled_now = effectively_enabled;
        const bool checked = action && ActionAccess::checkable(action) && ActionAccess::checked(action);
        if (enabled_now && (pressed_ || checked))
        {
            canvas.fill_round_rect(r, theme::radius(), theme::selection_inactive());
        }
        else if (enabled_now && hovered_)
        {
            canvas.fill_round_rect(r, theme::radius(), theme::selection_inactive().with_alpha(128));
        }
        const Color fg = enabled_now ? theme::text() : theme::text_disabled();
        const float opacity = enabled_now || ActionAccess::icon(action).is_icon() ? 1.0f : 0.4f;
        const TextLayout l(label(), Font(), display_scale);
        const bool icon = shows_icon(), text = shows_text();
        if (icon && !text)
        {
            canvas.draw_image(ActionAccess::icon(action), Rect((r.width - ICON) / 2, (r.height - ICON) / 2, ICON, ICON), fg,
                              opacity);
        }
        else if (icon && style == ToolButtonStyle::TEXT_UNDER_ICON)
        {
            canvas.draw_image(ActionAccess::icon(action), Rect((r.width - ICON) / 2, 6, ICON, ICON), fg, opacity);
            canvas.draw_text(l, Point((r.width - l.width()) / 2, 6 + ICON + 4), fg);
        }
        else
        {
            float x = BUTTON_PADDING;
            if (icon)
            {
                canvas.draw_image(ActionAccess::icon(action), Rect(x, (r.height - ICON) / 2, ICON, ICON), fg, opacity);
                x += ICON + ICON_TEXT_GAP;
            }
            canvas.draw_text(l, Point(x, (r.height - l.height()) / 2), fg);
        }
        Widget::paint(canvas);
    }

    void mouse_enter() override
    {
        hovered_ = true;
        invalidate();
        Widget::mouse_enter();
    }

    void mouse_leave() override
    {
        hovered_ = false;
        invalidate();
        Widget::mouse_leave();
    }

    void mouse_down(MouseEvent& e) override
    {
        Widget::mouse_down(e);
        if (e.button == MouseButton::LEFT)
        {
            e.accepted = true;
            pressed_ = true;
            invalidate();
        }
    }

    void mouse_up(MouseEvent& e) override
    {
        Widget::mouse_up(e);
        if (e.button != MouseButton::LEFT || !pressed_)
        {
            return;
        }
        e.accepted = true;
        pressed_ = false;
        invalidate();
        if (!local_rect.get().contains(e.pos))
        {
            return;
        }
        // ハンドラの中でツールバーが破棄されてもよいよう、最後に呼ぶ。
        if (action)
        {
            action->trigger();
        }
        else if (on_press)
        {
            const std::function<void()> fn = on_press;
            fn();
        }
    }

private:
    bool hovered_;
    bool pressed_;
};

}  // namespace detail

ToolBar::ToolBar(Widget* parent_widget)
    : Widget(parent_widget),
      TOKOTACHI_PROPERTY_GETTER(ToolBar, ToolButtonStyle, button_style),
      button_style_(ToolButtonStyle::ICON_ONLY),
      overflow_button_(nullptr),
      overflow_menu_(nullptr),
      destroying_(false)
{
    overflow_button_ = new detail::ToolButton(this, nullptr, button_style_);
    overflow_button_->visible = false;
    overflow_button_->tooltip = "その他の項目";
    overflow_button_->on_press = [this]() { show_overflow(); };
}

ToolBar::~ToolBar()
{
    destroying_ = true;
    delete overflow_menu_;
    overflow_menu_ = nullptr;
    // ボタンを持つ Action を外す(このツールバーが作った Action は、この後 Object として破棄される)。
    const std::vector<Action*> list = actions;
    for (std::size_t i = 0; i < list.size(); ++i)
    {
        remove_action(list[i]);
    }
}

Action* ToolBar::add_item(const std::string& text, const NotifyHandler& on_trigger)
{
    Action* a = new Action(this, text);
    a->on_trigger = on_trigger;
    add_action(a);
    return a;
}

void ToolBar::add_separator()
{
    const Entry e = {nullptr, nullptr, nullptr, false, 0};
    entries_.push_back(e);
    request_layout();
}

void ToolBar::add_widget(Widget* widget)
{
    if (!widget)
    {
        return;
    }
    widget->parent = this;
    if (widget->parent.get() != this)
    {
        return;
    }
    const Entry e = {nullptr, nullptr, widget, false, 0};
    entries_.push_back(e);
    request_layout();
}

void ToolBar::action_added(Action* action)
{
    const Entry e = {action, new detail::ToolButton(this, action, button_style_), nullptr, false, 0};
    entries_.push_back(e);
    request_layout();
}

void ToolBar::action_removed(Action* action)
{
    for (std::size_t i = 0; i < entries_.size(); ++i)
    {
        if (entries_[i].action == action)
        {
            detail::ToolButton* button = entries_[i].button;
            entries_.erase(entries_.begin() + static_cast<std::ptrdiff_t>(i));
            delete button;
            request_layout();
            return;
        }
    }
}

void ToolBar::action_changed(Action* action)
{
    for (std::size_t i = 0; i < entries_.size(); ++i)
    {
        if (entries_[i].action == action)
        {
            entries_[i].button->update();
        }
    }
}

void ToolBar::child_removed(Widget* child)
{
    if (destroying_)
    {
        return;
    }
    for (std::size_t i = 0; i < entries_.size(); ++i)
    {
        if (entries_[i].widget == child)
        {
            entries_.erase(entries_.begin() + static_cast<std::ptrdiff_t>(i));
            request_layout();
            return;
        }
    }
}

void ToolBar::set_button_style(const ToolButtonStyle& v)
{
    button_style_ = v;
    for (std::size_t i = 0; i < entries_.size(); ++i)
    {
        if (entries_[i].button)
        {
            entries_[i].button->style = v;
            entries_[i].button->request_layout();
        }
    }
    request_layout();
    invalidate();
}

Widget* ToolBar::item_widget(const Entry& e) const { return e.button ? e.button : e.widget; }

float ToolBar::separator_width() const { return SEPARATOR; }

std::vector<bool> ToolBar::absent_entries() const
{
    const std::size_t n = entries_.size();
    std::vector<bool> out(n, false);
    for (std::size_t i = 0; i < n; ++i)
    {
        out[i] = user_hidden(item_widget(entries_[i]));
    }
    // 区切りは、前と後ろの両方に並べる項目があるときだけ置く(続く区切りは 1 つにする)。
    bool item_before = false;
    for (std::size_t i = 0; i < n; ++i)
    {
        if (out[i])
        {
            continue;
        }
        if (item_widget(entries_[i]))
        {
            item_before = true;
            continue;
        }
        bool item_after = false;
        for (std::size_t k = i + 1; k < n && !item_after; ++k)
        {
            if (!out[k] && !item_widget(entries_[k]))
            {
                break;  // 次の区切りまでに項目が無い
            }
            item_after = !out[k];
        }
        out[i] = !item_before || !item_after;
        item_before = item_before && out[i];
    }
    return out;
}

// ---- 配置 ----

Size ToolBar::compute_preferred_size() const
{
    float w = BAR_PADDING * 2, h = 0;
    const std::vector<bool> absent = absent_entries();
    for (std::size_t i = 0; i < entries_.size(); ++i)
    {
        const Widget* item = item_widget(entries_[i]);
        if (absent[i])
        {
            continue;
        }
        const Size s = item ? item->preferred_size.get() : Size(SEPARATOR, 0);
        w += s.width + SPACING;
        h = std::max(h, s.height);
    }
    h = std::max(h, ICON + BUTTON_PADDING * 2);
    return Size(w, h + BAR_PADDING * 2);
}

Size ToolBar::compute_minimum_size() const
{
    const Size p = compute_preferred_size();
    return Size(overflow_button_->preferred_size.get().width + BAR_PADDING * 2, p.height);
}

void ToolBar::arrange_children()
{
    const Rect r = local_rect;
    const float inner_h = std::max(0.0f, r.height - BAR_PADDING * 2);
    float total = BAR_PADDING;
    const std::vector<bool> absent = absent_entries();
    for (std::size_t i = 0; i < entries_.size(); ++i)
    {
        const Widget* item = item_widget(entries_[i]);
        if (!absent[i])
        {
            total += (item ? item->preferred_size.get().width : SEPARATOR) + SPACING;
        }
    }
    const float overflow_w = overflow_button_->preferred_size.get().width;
    const bool overflow = total > r.width - BAR_PADDING;
    const float limit = overflow ? r.width - BAR_PADDING - overflow_w - SPACING : r.width - BAR_PADDING;

    // 左から並べ、収まらない項目から後ろを隠す。
    float x = BAR_PADDING;
    bool full = false;
    int last_shown = -1;
    for (std::size_t i = 0; i < entries_.size(); ++i)
    {
        Entry& e = entries_[i];
        Widget* item = item_widget(e);
        if (absent[i])
        {
            e.hidden = !item;  // 区切りは描かない(部品は visible が false なので描かれない)
            continue;
        }
        const Size s = item ? item->preferred_size.get() : Size(SEPARATOR, 0);
        full = full || x + s.width > limit;
        e.hidden = full;
        if (item)
        {
            detail::WidgetAccess::set_hidden_by_parent(item, full);  // 利用者の visible は変えない
        }
        if (full)
        {
            continue;
        }
        if (item)
        {
            const float h = std::min(s.height, inner_h);
            item->bounds = Rect(x, BAR_PADDING + (inner_h - h) / 2, s.width, h);
        }
        else
        {
            e.x = x + SEPARATOR / 2;
        }
        last_shown = static_cast<int>(i);
        x += s.width + SPACING;
    }
    // 見える項目の最後が区切りなら隠す。
    if (overflow && last_shown >= 0 && !item_widget(entries_[static_cast<std::size_t>(last_shown)]))
    {
        entries_[static_cast<std::size_t>(last_shown)].hidden = true;
    }
    overflow_button_->visible = overflow;
    if (overflow)
    {
        const float h = std::min(overflow_button_->preferred_size.get().height, inner_h);
        overflow_button_->bounds = Rect(r.width - BAR_PADDING - overflow_w, BAR_PADDING + (inner_h - h) / 2, overflow_w, h);
    }
    invalidate();
}

void ToolBar::paint(Canvas& canvas)
{
    const Rect r = local_rect;
    canvas.fill_rect(r, theme::window());
    canvas.fill_rect(Rect(0, r.height - 1, r.width, 1), theme::border());
    for (std::size_t i = 0; i < entries_.size(); ++i)
    {
        const Entry& e = entries_[i];
        if (!item_widget(e) && !e.hidden)
        {
            canvas.fill_rect(Rect(std::floor(e.x), BAR_PADDING + 6, 1, r.height - BAR_PADDING * 2 - 12), theme::border());
        }
    }
    Widget::paint(canvas);
}

void ToolBar::show_overflow()
{
    // 隠している Action をメニューに並べる(部品は出さない)。
    delete overflow_menu_;
    overflow_menu_ = new Menu(this);
    bool pending_separator = false;
    for (std::size_t i = 0; i < entries_.size(); ++i)
    {
        const Entry& e = entries_[i];
        if (!e.hidden)
        {
            continue;
        }
        if (!e.action && !e.widget)
        {
            pending_separator = !overflow_menu_->actions.get().empty();
            continue;
        }
        if (e.action)
        {
            if (pending_separator)
            {
                overflow_menu_->add_separator();
                pending_separator = false;
            }
            overflow_menu_->add_action(e.action);
        }
    }
    overflow_menu_->popup(overflow_button_, Point(0, overflow_button_->height));
}

}  // namespace tokotachi
