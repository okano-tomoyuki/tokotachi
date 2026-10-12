#include <tokotachi/widgets/date_edit.hpp>

#include <algorithm>
#include <cmath>

#include <tokotachi/graphics/canvas.hpp>
#include <tokotachi/layout/box_layout.hpp>
#include <tokotachi/widgets/calendar.hpp>
#include <tokotachi/widgets/line_edit.hpp>
#include <tokotachi/windows/popup.hpp>

#include "../core/impl.hpp"
#include "theme.hpp"

namespace tokotachi
{

namespace
{
const float BUTTON_WIDTH = 28;  // 右の暦のボタンの幅(dp)

// 書式 pattern で d を書いたときの、欄(年 'y'・月 'M'・日 'd')ごとの文字の範囲。
struct Field
{
    char kind;
    std::size_t begin;
    std::size_t end;
};

std::vector<Field> fields_of(const std::string& pattern, const Date& d)
{
    std::vector<Field> out;
    std::size_t pos = 0;
    for (std::size_t i = 0; i < pattern.size();)
    {
        const char c = pattern[i];
        std::size_t run = 1;
        while (i + run < pattern.size() && pattern[i + run] == c)
        {
            ++run;
        }
        // その部分だけを書いた長さ
        const std::size_t length = d.format(pattern.substr(i, run)).size();
        if (c == 'y' || c == 'M' || c == 'd')
        {
            const Field f = {c, pos, pos + length};
            out.push_back(f);
        }
        pos += length;
        i += run;
    }
    return out;
}

}  // namespace

namespace detail
{

// DateEdit の中の入力欄。枠は DateEdit が描くので、文字だけを描く。
class DateText : public LineEdit
{
public:
    explicit DateText(DateEdit* edit) : LineEdit(edit), edit_(edit) {}

protected:
    void paint(Canvas& canvas) override
    {
        paint_text(canvas);
        Widget::paint(canvas);
    }

    void focus_in() override
    {
        LineEdit::focus_in();
        edit_->invalidate();  // 枠の色を変える
    }

    void focus_out() override
    {
        LineEdit::focus_out();
        if (!edit_->is_calendar_open())
        {
            edit_->commit();
        }
        edit_->invalidate();
    }

private:
    DateEdit* edit_;
};

// ポップアップの中の暦。Esc で閉じる。
class PopupCalendar : public Calendar
{
public:
    explicit PopupCalendar(Widget* parent_widget, DateEdit* edit) : Calendar(parent_widget), edit_(edit) {}

protected:
    void key_down(KeyEvent& e) override
    {
        if (e.key == Key::ESCAPE)
        {
            e.accepted = true;
            edit_->close_calendar();
            return;
        }
        Calendar::key_down(e);
    }

private:
    DateEdit* edit_;
};

}  // namespace detail

class DateEdit::Impl
{
    TOKOTACHI_IMPL(DateEdit);

    explicit Impl(DateEdit* object) : q(object) {}

    Date get_date() const { return date_; }
    void set_date(const Date& v);
    Date get_minimum() const { return minimum_; }
    void set_minimum(const Date& v);
    Date get_maximum() const { return maximum_; }
    void set_maximum(const Date& v);
    std::string get_format() const { return format_; }
    void set_format(const std::string& v);
    DayOfWeek get_first_day_of_week() const { return first_day_; }
    void set_first_day_of_week(const DayOfWeek& v) { first_day_ = v; }
    std::string get_placeholder_text() const;
    void set_placeholder_text(const std::string& v);
    std::string get_text() const;

    Date clamp(const Date& d) const;
    /// 右の暦のボタンの領域。
    Rect calendar_button() const;
    /// キャレットのある欄(年 'y'・月 'M'・日 'd')を step だけ増減する。
    void step_field(int step);
    /// 文字を今の date の表示に戻す。
    void show_date();

    detail::DateText* edit_;
    Date date_;
    Date minimum_;
    Date maximum_;
    std::string format_;
    DayOfWeek first_day_;
    NotifyHandler on_change_;
    Popup* popup_;
    Calendar* calendar_;
};

DateEdit::DateEdit(Widget* parent_widget)
    : Widget(parent_widget),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, Date, date),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, Date, minimum),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, Date, maximum),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, std::string, format),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, DayOfWeek, first_day_of_week),
      TOKOTACHI_IMPL_PROPERTY_GETTER(Impl, std::string, placeholder_text),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, std::string, text),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_change),
      d_(new Impl(this))
{
    d_->edit_ = nullptr;
    d_->minimum_ = Date(1, 1, 1);
    d_->maximum_ = Date(9999, 12, 31);
    d_->format_ = "yyyy/MM/dd";
    d_->first_day_ = DayOfWeek::SUNDAY;
    d_->popup_ = nullptr;
    d_->calendar_ = nullptr;
    d_->edit_ = new detail::DateText(this);
    d_->edit_->on_submit = [this](Object*) { commit(); };
    d_->edit_->on_change = [this](Object*) { notify_changed(text); };
}

DateEdit::~DateEdit() { delete d_; }

Date DateEdit::Impl::clamp(const Date& d) const { return d.is_null() ? d : std::max(minimum_, std::min(maximum_, d)); }

void DateEdit::Impl::show_date()
{
    const std::string shown = date_.format(format_);
    if (edit_->text.get() != shown)
    {
        edit_->text = shown;  // 同じなら代入しない(キャレットが動かないように)
    }
}

void DateEdit::Impl::set_date(const Date& v)
{
    if (!v.is_null() && !v.is_valid())
    {
        return;
    }
    const Date d = clamp(v);
    if (d == date_)
    {
        show_date();
        return;
    }
    date_ = d;
    show_date();
    // 結びつけた相手へ伝えてから、ハンドラを呼ぶ
    q->notify_changed(q->date);
    Object* self = q;
    detail::fire(on_change_, self);
}

void DateEdit::Impl::set_minimum(const Date& v)
{
    if (!v.is_valid())
    {
        return;
    }
    minimum_ = v;
    if (maximum_ < minimum_)
    {
        maximum_ = minimum_;
        q->notify_changed(q->maximum);
    }
    set_date(date_);
}

void DateEdit::Impl::set_maximum(const Date& v)
{
    if (!v.is_valid())
    {
        return;
    }
    maximum_ = v;
    if (minimum_ > maximum_)
    {
        minimum_ = maximum_;
        q->notify_changed(q->minimum);
    }
    set_date(date_);
}

void DateEdit::Impl::set_format(const std::string& v)
{
    format_ = v.empty() ? std::string("yyyy/MM/dd") : v;
    show_date();
    q->request_layout();
}

std::string DateEdit::Impl::get_placeholder_text() const { return edit_->placeholder_text; }

void DateEdit::Impl::set_placeholder_text(const std::string& v) { edit_->placeholder_text = v; }

std::string DateEdit::Impl::get_text() const { return edit_->text; }

void DateEdit::commit()
{
    const std::string s = d_->edit_->text;
    if (s.find_first_not_of(" \t") == std::string::npos)
    {
        d_->set_date(Date());  // 空にした
        return;
    }
    const Date parsed = Date::parse(s, d_->format_);
    if (parsed.is_null())
    {
        d_->show_date();  // 読めなければ元に戻す
        return;
    }
    d_->set_date(parsed);
}

void DateEdit::Impl::step_field(int step)
{
    const std::size_t caret = edit_->cursor_position;  // 読み直すと動くことがあるので、先に覚える
    q->commit();
    if (date_.is_null())
    {
        set_date(Date::today());
        return;
    }
    // キャレットのある欄(無ければ日)
    const std::vector<Field> fields = fields_of(format_, date_);
    char kind = 'd';
    std::size_t index = 0;
    for (std::size_t k = 0; k < fields.size(); ++k)
    {
        if (caret >= fields[k].begin && caret <= fields[k].end)
        {
            kind = fields[k].kind;
            index = k;
            break;
        }
        if (fields[k].kind == 'd')
        {
            index = k;
        }
    }
    const Date next = kind == 'y' ? date_.add_years(step) : kind == 'M' ? date_.add_months(step) : date_.add_days(step);
    set_date(next);
    // キャレットを同じ欄の終わりに置く(桁が変わっても欄の中に残るように)
    const std::vector<Field> after = fields_of(format_, date_);
    if (index < after.size())
    {
        edit_->cursor_position = after[index].end;
    }
}

// ---- 暦 ----

bool DateEdit::is_calendar_open() const { return d_->popup_ && d_->popup_->visible; }

void DateEdit::open_calendar()
{
    if (is_calendar_open())
    {
        return;
    }
    commit();
    if (!d_->popup_)
    {
        d_->popup_ = new Popup(this);
        BoxLayout* box = new BoxLayout(d_->popup_);
        box->margins = Margins(1);
        d_->calendar_ = new detail::PopupCalendar(d_->popup_, this);
        box->add(d_->calendar_, 1);
        d_->calendar_->on_activate = [this](Object*)
        {
            const Date chosen = d_->calendar_->date;
            close_calendar();
            d_->edit_->focused = true;
            date = chosen;  // ハンドラの中で破棄されてもよいよう、最後に代入する
        };
        d_->popup_->on_closed = [this](Object*) { invalidate(); };
    }
    d_->calendar_->minimum = d_->minimum_;
    d_->calendar_->maximum = d_->maximum_;
    d_->calendar_->first_day_of_week = d_->first_day_;
    d_->calendar_->date = d_->date_;
    if (d_->date_.is_null())
    {
        const Date today = d_->clamp(Date::today());
        d_->calendar_->show_month(today.year, today.month);
    }
    const Size size = d_->calendar_->preferred_size;
    d_->popup_->open(local_rect, Size(size.width + 2, size.height + 2), PopupPlacement::BELOW);
    d_->popup_->apply_layout();
    d_->calendar_->focused = true;
    invalidate();
}

void DateEdit::close_calendar()
{
    if (is_calendar_open())
    {
        d_->popup_->close();
        invalidate();
    }
}

// ---- 配置と描画 ----

Rect DateEdit::Impl::calendar_button() const
{
    const Rect r = q->local_rect;
    return Rect(r.width - BUTTON_WIDTH, 0, BUTTON_WIDTH, r.height);
}

void DateEdit::arrange_children()
{
    const Rect r = local_rect;
    d_->edit_->bounds = Rect(0, 0, std::max(0.0f, r.width - BUTTON_WIDTH), r.height);
}

Size DateEdit::compute_preferred_size() const
{
    // 書式で一番長くなる日付(2 桁の月・日)が収まる幅
    const float w = TextLayout(Date(2000, 12, 28).format(d_->format_), d_->edit_->font, display_scale).width();
    const Size edit = d_->edit_->preferred_size;
    return Size(std::ceil(w) + theme::field_padding() * 2 + 8 + BUTTON_WIDTH, edit.height);
}

Size DateEdit::compute_minimum_size() const { return compute_preferred_size(); }

void DateEdit::paint(Canvas& canvas)
{
    const Rect r = local_rect;
    const bool enabled_now = effectively_enabled;
    const bool has_focus = d_->edit_->focused || is_calendar_open();
    canvas.fill_round_rect(r, theme::field_radius(), enabled_now ? theme::field() : theme::field_disabled());
    // 暦のボタン: 区切りの線と、暦の印(枠・上の帯・綴じ具)
    const Rect b = d_->calendar_button();
    if (is_calendar_open())
    {
        canvas.fill_rect(b.adjusted(0, 1, 1, 1), theme::selection_inactive());
    }
    canvas.fill_rect(Rect(b.x, 1, 1, r.height - 2), theme::border());
    const Color mark = enabled_now ? theme::text() : theme::text_disabled();
    const Rect icon(std::floor(b.x + (b.width - 14) / 2) + 0.5f, std::floor(r.height / 2 - 6) + 0.5f, 13, 12);
    canvas.stroke_round_rect(icon, 2, 1.2f, mark);
    canvas.fill_rect(Rect(icon.x, icon.y, icon.width, 3), mark);
    canvas.fill_rect(Rect(icon.x + 3, icon.y - 2, 1.5f, 3), mark);
    canvas.fill_rect(Rect(icon.right() - 4.5f, icon.y - 2, 1.5f, 3), mark);
    canvas.stroke_round_rect(r, theme::field_radius(), has_focus ? 2.0f : 1.0f, has_focus ? theme::accent() : theme::border());
    Widget::paint(canvas);
}

// ---- 入力 ----

void DateEdit::mouse_down(MouseEvent& e)
{
    Widget::mouse_down(e);
    if (e.button != MouseButton::LEFT)
    {
        return;
    }
    e.accepted = true;
    if (d_->calendar_button().contains(e.pos))
    {
        if (is_calendar_open())
        {
            close_calendar();
        }
        else
        {
            open_calendar();
        }
        return;
    }
    d_->edit_->focused = true;
}

void DateEdit::mouse_wheel(WheelEvent& e)
{
    Widget::mouse_wheel(e);
    if (!d_->edit_->focused)
    {
        return;  // 親(スクロール領域など)へ送る
    }
    e.accepted = true;
    const float ticks = e.delta.y != 0 ? e.delta.y : e.delta.x;
    d_->step_field(ticks > 0 ? 1 : ticks < 0 ? -1 : 0);
}

void DateEdit::key_down(KeyEvent& e)
{
    Widget::key_down(e);
    if ((e.key == Key::DOWN && e.has_modifier(MODIFIER_ALT)) || (e.key == Key::F4 && !e.modifiers))
    {
        e.accepted = true;
        open_calendar();
        return;
    }
    if (e.modifiers & (MODIFIER_CONTROL | MODIFIER_ALT | MODIFIER_META))
    {
        return;
    }
    switch (e.key)
    {
    case Key::UP:
        e.accepted = true;
        d_->step_field(1);
        break;
    case Key::DOWN:
        e.accepted = true;
        d_->step_field(-1);
        break;
    case Key::PAGE_UP:
    case Key::PAGE_DOWN:
        e.accepted = true;
        commit();
        d_->set_date(d_->date_.is_null() ? Date::today() : d_->date_.add_months(e.key == Key::PAGE_UP ? 1 : -1));
        break;
    default:
        break;
    }
}

}  // namespace tokotachi
