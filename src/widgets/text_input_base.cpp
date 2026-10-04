#include <gui/text_input_base.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

#include <gui/canvas.hpp>
#include <gui/clipboard.hpp>

#include "../core/property_impl.hpp"
#include "../text/utf8.hpp"
#include "theme.hpp"

namespace gui
{

namespace
{
const int BLINK_MS = 530;
}

TextInputBase::TextInputBase(Widget* parent_widget, const std::string& initial_text, bool multi_line)
    : Widget(parent_widget),
      GUI_PROPERTY_MEMBER_SELF_NOTIFY(TextInputBase, std::string, text),
      GUI_PROPERTY_MEMBER(TextInputBase, std::string, placeholder_text),
      GUI_PROPERTY_MEMBER(TextInputBase, Font, font),
      GUI_PROPERTY_GETTER_SELF_NOTIFY(TextInputBase, std::size_t, cursor_position),
      GUI_READONLY_PROPERTY(TextInputBase, bool, has_selection),
      GUI_READONLY_PROPERTY(TextInputBase, std::string, selected_text),
      GUI_HANDLER_PROPERTY(TextInputBase, NotifyHandler, on_change),
      multi_line_(multi_line),
      text_(multi_line ? detail::normalize_multiline(initial_text) : detail::strip_control(initial_text)),
      cursor_(text_.size()),
      anchor_(text_.size()),
      preedit_start_(-1),
      preedit_length_(-1),
      selecting_(false),
      caret_on_(true),
      goal_x_(-1),
      layout_wrap_(0)
{
    focus_policy = FocusPolicy::TAB_AND_CLICK;
    cursor = MouseCursor::IBEAM;
    accepts_text_input = true;
    blink_.interval = BLINK_MS;
    blink_.on_timer = [this](Object*)
    {
        caret_on_ = !caret_on_;
        invalidate();
    };
}

std::string TextInputBase::sanitize(const std::string& s) const
{
    return multi_line_ ? detail::normalize_multiline(s) : detail::strip_control(s);
}

// --- 状態の変化の通知(docs/adr/0016) ---

TextInputBase::State TextInputBase::state() const
{
    State s;
    s.text = text_;
    s.cursor = cursor_;
    s.anchor = anchor_;
    return s;
}

void TextInputBase::finish(const State& before)
{
    const bool text_changed = before.text != text_;
    if (text_changed)
    {
        notify_changed(text);
    }
    if (before.cursor != cursor_)
    {
        notify_changed(cursor_position);
    }
    const bool had_selection = before.anchor != before.cursor;
    if (had_selection != get_has_selection())
    {
        notify_changed(has_selection);
    }
    const std::size_t lo = std::min(before.cursor, before.anchor), hi = std::max(before.cursor, before.anchor);
    if (before.text.substr(lo, hi - lo) != get_selected_text())
    {
        notify_changed(selected_text);
    }
    // ハンドラの中でこのウィジェットが破棄されてもよいよう、最後に呼ぶ。
    if (text_changed)
    {
        Object* self = this;
        detail::fire(on_change_, self);
    }
}

// --- プロパティ ---

void TextInputBase::set_text(const std::string& v)
{
    const State before = state();
    text_ = sanitize(v);
    cursor_ = anchor_ = text_.size();
    preedit_.clear();
    goal_x_ = -1;
    text_touched();
    finish(before);
}

void TextInputBase::set_placeholder_text(const std::string& v)
{
    placeholder_text_ = v;
    placeholder_layout_ = TextLayout();
    invalidate();
}

void TextInputBase::set_font(const Font& v)
{
    if (v == font_)
    {
        return;
    }
    font_ = v;
    invalidate_layout();
    placeholder_layout_ = TextLayout();
    invalidate();
    update_input_method();
    request_layout();
}

void TextInputBase::set_cursor_position(const std::size_t& v)
{
    const State before = state();
    goal_x_ = -1;
    move_cursor(std::min(v, text_.size()), false);
    finish(before);
}

std::string TextInputBase::get_selected_text() const
{
    const std::size_t lo = std::min(cursor_, anchor_), hi = std::max(cursor_, anchor_);
    return text_.substr(lo, hi - lo);
}

// --- 操作 ---

void TextInputBase::select_all()
{
    const State before = state();
    anchor_ = 0;
    move_cursor(text_.size(), true);
    finish(before);
}

void TextInputBase::deselect()
{
    const State before = state();
    move_cursor(cursor_, false);
    finish(before);
}

void TextInputBase::cut()
{
    if (!get_has_selection())
    {
        return;
    }
    const State before = state();
    copy();
    erase_selection();
    text_touched();
    finish(before);
}

void TextInputBase::copy() const
{
    if (get_has_selection())
    {
        clipboard::set_text(get_selected_text());
    }
}

void TextInputBase::paste() { insert(clipboard::text()); }

void TextInputBase::insert(const std::string& s)
{
    const State before = state();
    insert_text(s);
    finish(before);
}

// --- 内部の処理(通知はしない。呼び出し側が finish で知らせる) ---

void TextInputBase::insert_text(const std::string& s)
{
    const std::string clean = sanitize(s);
    if (clean.empty() && !get_has_selection())
    {
        return;
    }
    erase_selection();
    text_.insert(cursor_, clean);
    cursor_ += clean.size();
    anchor_ = cursor_;
    goal_x_ = -1;
    text_touched();
}

void TextInputBase::move_cursor(std::size_t pos, bool keep_anchor)
{
    cursor_ = pos;
    if (!keep_anchor)
    {
        anchor_ = cursor_;
    }
    caret_touched();
}

void TextInputBase::move_vertically(int lines, bool keep_anchor)
{
    const TextLayout& l = text_layout();
    if (goal_x_ < 0)
    {
        goal_x_ = l.caret_position(cursor_).x;
    }
    const long line = static_cast<long>(l.line_of(cursor_)) + lines;
    std::size_t target = 0;
    if (line < 0)
    {
        target = 0;  // 最初の行より上へは、テキストの始めへ
    }
    else if (line >= static_cast<long>(l.line_count()))
    {
        target = text_.size();  // 最後の行より下へは、テキストの終わりへ
    }
    else
    {
        const Rect r = l.line_rect(static_cast<std::size_t>(line));
        target = l.offset_at(Point(goal_x_, r.y + r.height / 2));
    }
    const float keep = goal_x_;
    move_cursor(target, keep_anchor);
    goal_x_ = keep;  // 上下に続けて動かす間は、最初の横の位置を保つ
}

void TextInputBase::erase_selection()
{
    if (!get_has_selection())
    {
        return;
    }
    const std::size_t lo = std::min(cursor_, anchor_), hi = std::max(cursor_, anchor_);
    text_.erase(lo, hi - lo);
    cursor_ = anchor_ = lo;
    invalidate_layout();
}

void TextInputBase::text_touched()
{
    invalidate_layout();
    caret_touched();
}

void TextInputBase::caret_touched()
{
    restart_blink();
    update_scroll(true);
    invalidate();
    update_input_method();
}

const TextLayout& TextInputBase::text_layout() const
{
    const float scale = display_scale;
    const float wrap = wrap_width();
    if (layout_.is_null() || layout_.scale() != scale || layout_wrap_ != wrap)
    {
        const std::string shown =
            preedit_.empty() ? text_ : text_.substr(0, cursor_) + preedit_ + text_.substr(cursor_);
        layout_ = TextLayout(shown, font_, scale, wrap);
        layout_wrap_ = wrap;
    }
    return layout_;
}

void TextInputBase::invalidate_layout() { layout_ = TextLayout(); }

Point TextInputBase::caret_in_layout() const
{
    const TextLayout& l = text_layout();
    if (preedit_.empty())
    {
        return l.caret_position(cursor_);
    }
    const std::size_t in = preedit_start_ >= 0 ? detail::utf8_offset_of(preedit_, preedit_start_) : preedit_.size();
    return l.caret_position(cursor_ + in);
}

Point TextInputBase::text_origin() const
{
    const Rect area = text_area();
    const TextLayout& l = text_layout();
    if (multi_line_)
    {
        return Point(area.x - scroll_.x, area.y - scroll_.y);
    }
    // 1 行は縦の中央に置く。右から左の段落は、収まるなら右端から並べる。
    const float room = area.width - l.width();
    const float x = area.x - scroll_.x + ((l.line_is_rtl(0) && room > 0) ? room : 0);
    return Point(x, area.y + (area.height - l.height()) / 2);
}

void TextInputBase::update_scroll(bool follow_caret)
{
    const Rect area = text_area();
    if (area.width <= 0 || area.height <= 0)
    {
        return;
    }
    const TextLayout& l = text_layout();
    Point s = scroll_;
    if (follow_caret)
    {
        const Point c = caret_in_layout();
        const float h = l.ascent() + l.descent();
        if (c.x - s.x > area.width - 1)
        {
            s.x = c.x - area.width + 1;
        }
        if (c.x < s.x)
        {
            s.x = c.x;
        }
        if (multi_line_)
        {
            if (c.y + h > s.y + area.height)
            {
                s.y = c.y + h - area.height;
            }
            if (c.y < s.y)
            {
                s.y = c.y;
            }
        }
    }
    // 範囲に収める(末尾の後ろに余白が出ないように)。
    s.x = std::max(0.0f, std::min(s.x, l.width() - area.width + 1));
    s.y = multi_line_ ? std::max(0.0f, std::min(s.y, l.height() - area.height)) : 0;
    if (s != scroll_)
    {
        scroll_ = s;
        invalidate();
        update_input_method();
    }
    scroll_changed();
}

void TextInputBase::set_scroll_offset(const Point& p)
{
    scroll_ = p;
    update_scroll(false);
    invalidate();
}

std::size_t TextInputBase::offset_at(const Point& p) const { return text_layout().offset_at(p - text_origin()); }

void TextInputBase::restart_blink()
{
    caret_on_ = true;
    blink_.enabled = false;
    if (focused)
    {
        blink_.enabled = true;
    }
}

void TextInputBase::stop_blink() { blink_.enabled = false; }

// --- 描画 ---

void TextInputBase::paint_text(Canvas& canvas)
{
    const bool enabled_now = effectively_enabled, has_focus = focused;
    const TextLayout& l = text_layout();
    const Rect area = text_area();
    const Point o = text_origin();
    const Color fg = enabled_now ? theme::text() : theme::text_disabled();

    canvas.save();
    canvas.clip_rect(area);

    if (text_.empty() && preedit_.empty() && !placeholder_text_.empty())
    {
        if (placeholder_layout_.is_null() || placeholder_layout_.scale() != l.scale())
        {
            placeholder_layout_ = TextLayout(placeholder_text_, font_, l.scale(), wrap_width());
        }
        canvas.draw_text(placeholder_layout_, multi_line_ ? Point(area.x, area.y) : Point(area.x, o.y),
                         theme::placeholder());
    }

    // 選択の矩形は、双方向の行・複数行では離れた複数の矩形になる(docs/adr/0020)。
    std::vector<Rect> selection;
    if (get_has_selection() && preedit_.empty())
    {
        selection = l.selection_rects(std::min(cursor_, anchor_), std::max(cursor_, anchor_));
        for (std::size_t i = 0; i < selection.size(); ++i)
        {
            selection[i] = selection[i].translated(o.x, o.y);
            canvas.fill_rect(selection[i], has_focus ? theme::selection() : theme::selection_inactive());
        }
    }

    canvas.draw_text(l, o, fg);
    if (has_focus)
    {
        // 選択した部分の文字は色を変えて描き直す。
        for (std::size_t i = 0; i < selection.size(); ++i)
        {
            canvas.save();
            canvas.clip_rect(selection[i]);
            canvas.draw_text(l, o, theme::on_selection());
            canvas.restore();
        }
    }

    if (!preedit_.empty())
    {
        // 変換中の文字列に細い下線、変換対象の文節に太い下線(docs/adr/0009)。
        const float under = l.ascent() + 2;
        const std::vector<Rect> whole = l.selection_rects(cursor_, cursor_ + preedit_.size());
        for (std::size_t i = 0; i < whole.size(); ++i)
        {
            canvas.fill_rect(Rect(o.x + whole[i].x, o.y + whole[i].y + under, whole[i].width, 1),
                             theme::preedit_underline());
        }
        if (preedit_start_ >= 0 && preedit_length_ > 0)
        {
            const std::vector<Rect> target =
                l.selection_rects(cursor_ + detail::utf8_offset_of(preedit_, preedit_start_),
                                  cursor_ + detail::utf8_offset_of(preedit_, preedit_start_ + preedit_length_));
            for (std::size_t i = 0; i < target.size(); ++i)
            {
                canvas.fill_rect(Rect(o.x + target[i].x, o.y + target[i].y + under - 1, target[i].width, 2),
                                 theme::preedit_underline());
            }
        }
    }

    if (has_focus && caret_on_ && (selection.empty() || !preedit_.empty()))
    {
        const Point c = caret_in_layout();
        canvas.fill_rect(Rect(o.x + c.x, o.y + c.y, 1, l.ascent() + l.descent()), fg);
    }

    canvas.restore();
}

// --- イベント ---

void TextInputBase::resized()
{
    invalidate();
    update_scroll(true);
    update_input_method();
    Widget::resized();
}

void TextInputBase::mouse_down(MouseEvent& e)
{
    Widget::mouse_down(e);
    if (e.button != MouseButton::LEFT)
    {
        return;
    }
    e.accepted = true;
    if (!preedit_.empty())
    {
        return;  // 変換中はキャレットを動かさない
    }
    goal_x_ = -1;
    if (e.clicks >= 3)
    {
        select_all();  // トリプルクリックで全体
        return;
    }
    if (e.clicks == 2)
    {
        // ダブルクリックで単語を選ぶ。
        const State before = state();
        std::size_t start = 0, end = 0;
        text_layout().word_at(offset_at(e.pos), start, end);
        anchor_ = start;
        move_cursor(end, true);
        finish(before);
        return;
    }
    const State before = state();
    move_cursor(offset_at(e.pos), e.has_modifier(MODIFIER_SHIFT));
    selecting_ = true;
    finish(before);
}

void TextInputBase::mouse_move(MouseEvent& e)
{
    Widget::mouse_move(e);
    if (selecting_)
    {
        const State before = state();
        move_cursor(offset_at(e.pos), true);
        finish(before);
    }
}

void TextInputBase::mouse_up(MouseEvent& e)
{
    Widget::mouse_up(e);
    if (e.button == MouseButton::LEFT)
    {
        e.accepted = true;
        selecting_ = false;
    }
}

void TextInputBase::key_down(KeyEvent& e)
{
    Widget::key_down(e);
    // 変換中のキーは IME のもの(docs/adr/0009)。
    if (!preedit_.empty())
    {
        e.accepted = true;
        return;
    }
    const bool shift = e.has_modifier(MODIFIER_SHIFT), ctrl = e.has_modifier(MODIFIER_CONTROL);
    if (ctrl && (e.key == Key::A || e.key == Key::C || e.key == Key::X || e.key == Key::V))
    {
        // それぞれの操作が自分で知らせる。
        e.accepted = true;
        if (e.key == Key::A)
        {
            select_all();
        }
        else if (e.key == Key::C)
        {
            copy();
        }
        else if (e.key == Key::X)
        {
            cut();
        }
        else
        {
            paste();
        }
        return;
    }
    if (e.key == Key::RETURN && !multi_line_)
    {
        // ハンドラの中で破棄されてもよいよう、submit の後は自分に触らない(e は呼び出し側のもの)。
        e.accepted = submit();
        return;
    }

    const TextLayout& l = text_layout();
    const std::size_t lo = std::min(cursor_, anchor_), hi = std::max(cursor_, anchor_);
    const bool selected = get_has_selection();
    const State before = state();
    bool handled = true;
    bool vertical = false;  // 上下の移動(キャレットの横の位置を保つ)
    switch (e.key)
    {
    case Key::LEFT:
        move_cursor(ctrl ? l.previous_word_offset(cursor_) : (selected && !shift ? lo : l.previous_offset(cursor_)),
                    shift);
        break;
    case Key::RIGHT:
        move_cursor(ctrl ? l.next_word_offset(cursor_) : (selected && !shift ? hi : l.next_offset(cursor_)), shift);
        break;
    case Key::UP:
    case Key::DOWN:
    case Key::PAGE_UP:
    case Key::PAGE_DOWN:
        if (!multi_line_)
        {
            handled = false;
            break;
        }
        {
            // Page Up・Down は見えている行の数だけ動かす。
            const int page = std::max(1, static_cast<int>(text_area().height / std::max(1.0f, l.line_height())) - 1);
            const int lines = e.key == Key::UP ? -1 : e.key == Key::DOWN ? 1 : e.key == Key::PAGE_UP ? -page : page;
            move_vertically(lines, shift);
            vertical = true;
        }
        break;
    case Key::HOME:
        // 複数行では行の始め(Ctrl で全体の始め)。
        move_cursor(multi_line_ && !ctrl ? l.line_start(l.line_of(cursor_)) : 0, shift);
        break;
    case Key::END:
        move_cursor(multi_line_ && !ctrl ? l.line_end(l.line_of(cursor_)) : text_.size(), shift);
        break;
    case Key::RETURN:
        insert_text("\n");  // 複数行では改行を入れる
        break;
    case Key::BACKSPACE:
        if (selected)
        {
            erase_selection();
            text_touched();
        }
        else if (cursor_ > 0)
        {
            // Ctrl なら単語の始めまで消す。
            const std::size_t prev = ctrl ? l.previous_word_offset(cursor_) : l.previous_offset(cursor_);
            text_.erase(prev, cursor_ - prev);
            cursor_ = anchor_ = prev;
            text_touched();
        }
        break;
    case Key::DEL:
        if (selected)
        {
            erase_selection();
            text_touched();
        }
        else if (cursor_ < text_.size())
        {
            const std::size_t next = ctrl ? l.next_word_offset(cursor_) : l.next_offset(cursor_);
            text_.erase(cursor_, next - cursor_);
            anchor_ = cursor_;
            text_touched();
        }
        break;
    default:
        handled = false;
        break;
    }
    if (handled)
    {
        if (!vertical)
        {
            goal_x_ = -1;
        }
        e.accepted = true;
        finish(before);
    }
}

void TextInputBase::text_input(TextInputEvent& e)
{
    const State before = state();
    if (!preedit_.empty())
    {
        preedit_.clear();
        invalidate_layout();
    }
    insert_text(e.text);
    finish(before);
}

void TextInputBase::text_editing(TextEditingEvent& e)
{
    const State before = state();
    if (e.text.empty())
    {
        if (!preedit_.empty())
        {
            preedit_.clear();
            invalidate_layout();
            invalidate();
            update_input_method();
        }
        return;
    }
    // 変換を始めるときに選択していた部分は、変換中の文字列で置き換える。
    if (preedit_.empty() && get_has_selection())
    {
        erase_selection();
    }
    preedit_ = e.text;
    preedit_start_ = e.start;
    preedit_length_ = e.length;
    text_touched();
    finish(before);
}

void TextInputBase::focus_in()
{
    restart_blink();
    invalidate();
    Widget::focus_in();
}

void TextInputBase::focus_out()
{
    stop_blink();
    selecting_ = false;
    if (!preedit_.empty())
    {
        preedit_.clear();
        invalidate_layout();
    }
    invalidate();
    Widget::focus_out();
}

void TextInputBase::scale_changed()
{
    invalidate_layout();
    placeholder_layout_ = TextLayout();
}

Rect TextInputBase::input_method_rect() const
{
    const TextLayout& l = text_layout();
    const Point c = caret_in_layout() + text_origin();
    return Rect(c.x, c.y, 1, l.ascent() + l.descent());
}

}  // namespace gui
