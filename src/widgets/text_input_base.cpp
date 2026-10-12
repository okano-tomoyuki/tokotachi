#include <tokotachi/widgets/text_input_base.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

#include <tokotachi/graphics/canvas.hpp>
#include <tokotachi/core/clipboard.hpp>

#include "../core/impl.hpp"
#include "../graphics/utf8.hpp"
#include "theme.hpp"

namespace tokotachi
{

namespace
{
const int BLINK_MS = 530;
}

class TextInputBase::Impl
{
    TOKOTACHI_IMPL(TextInputBase);

    Impl(TextInputBase* object, bool multi_line) : q(object), multi_line_(multi_line) {}

    /// 操作の前の状態。操作の後に、変わったプロパティを知らせるために使う(docs/adr/0016)。
    struct State
    {
        std::string text;
        std::size_t cursor;
        std::size_t anchor;
    };

    void set_text(const std::string& v);
    void set_placeholder_text(const std::string& v);
    void set_font(const Font& v);
    std::size_t get_cursor_position() const { return cursor_; }
    void set_cursor_position(const std::size_t& v);
    bool get_has_selection() const { return anchor_ != cursor_; }
    std::string get_selected_text() const;

    State state() const;
    /// before から変わったプロパティを知らせ、テキストが変わっていれば on_change を呼ぶ。
    void finish(const State& before);
    std::string sanitize(const std::string& s) const;

    void invalidate_layout();
    /// テキストが変わった後・キャレットが動いた後の描き直し(キャレットが見えるようにスクロールする)。
    void text_touched();
    void caret_touched();
    /// レイアウトの中での、キャレットの位置(変換中ならその中のキャレット)。
    Point caret_in_layout() const;
    /// テキストのレイアウトの左上(自分の座標系。スクロールと、1 行のときの縦の中央寄せ・右から左の右寄せを含む)。
    Point text_origin() const;
    /// キャレットが見えるようにスクロールし(follow_caret のとき)、範囲に収める。
    void update_scroll(bool follow_caret);
    std::size_t offset_at(const Point& p) const;
    void move_cursor(std::size_t pos, bool keep_anchor);
    /// 上下の行へ動かす(lines が負なら上)。キャレットの横の位置(goal_x_)を保つ。
    void move_vertically(int lines, bool keep_anchor);
    void erase_selection();
    void insert_text(const std::string& s);
    void restart_blink();
    void stop_blink();

    const bool multi_line_;
    std::string text_;
    std::string placeholder_text_;
    Font font_;
    NotifyHandler on_change_;
    std::size_t cursor_;
    std::size_t anchor_;
    std::string preedit_;
    int preedit_start_;
    int preedit_length_;
    bool selecting_;  ///< マウスで選択している
    bool caret_on_;
    float goal_x_;  ///< 上下に動かすときに保つキャレットの横の位置(負なら未定)
    Timer blink_;   ///< キャレットの点滅
    Point scroll_;
    mutable TextLayout layout_;
    mutable float layout_wrap_;
    mutable TextLayout placeholder_layout_;
};

TextInputBase::TextInputBase(Widget* parent_widget, const std::string& initial_text, bool multi_line)
    : Widget(parent_widget),
      TOKOTACHI_IMPL_PROPERTY_MEMBER_SELF_NOTIFY(Impl, std::string, text),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, std::string, placeholder_text),
      TOKOTACHI_IMPL_PROPERTY_MEMBER(Impl, Font, font),
      TOKOTACHI_IMPL_PROPERTY_GETTER_SELF_NOTIFY(Impl, std::size_t, cursor_position),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, bool, has_selection),
      TOKOTACHI_IMPL_READONLY_PROPERTY(Impl, std::string, selected_text),
      TOKOTACHI_IMPL_HANDLER_PROPERTY(Impl, NotifyHandler, on_change),
      d_(new Impl(this, multi_line))
{
    d_->text_ = multi_line ? detail::normalize_multiline(initial_text) : detail::strip_control(initial_text);
    d_->cursor_ = d_->text_.size();
    d_->anchor_ = d_->text_.size();
    d_->preedit_start_ = -1;
    d_->preedit_length_ = -1;
    d_->selecting_ = false;
    d_->caret_on_ = true;
    d_->goal_x_ = -1;
    d_->layout_wrap_ = 0;
    focus_policy = FocusPolicy::TAB_AND_CLICK;
    cursor = MouseCursor::IBEAM;
    accepts_text_input = true;
    d_->blink_.interval = BLINK_MS;
    d_->blink_.on_timer = [this](Object*)
    {
        d_->caret_on_ = !d_->caret_on_;
        invalidate();
    };
}

TextInputBase::~TextInputBase() { delete d_; }

Point TextInputBase::scroll_offset() const { return d_->scroll_; }

bool TextInputBase::is_multi_line() const { return d_->multi_line_; }

std::string TextInputBase::Impl::sanitize(const std::string& s) const
{
    return multi_line_ ? detail::normalize_multiline(s) : detail::strip_control(s);
}

// --- 状態の変化の通知(docs/adr/0016) ---

TextInputBase::Impl::State TextInputBase::Impl::state() const
{
    State s;
    s.text = text_;
    s.cursor = cursor_;
    s.anchor = anchor_;
    return s;
}

void TextInputBase::Impl::finish(const State& before)
{
    const bool text_changed = before.text != text_;
    if (text_changed)
    {
        q->notify_changed(q->text);
    }
    if (before.cursor != cursor_)
    {
        q->notify_changed(q->cursor_position);
    }
    const bool had_selection = before.anchor != before.cursor;
    if (had_selection != get_has_selection())
    {
        q->notify_changed(q->has_selection);
    }
    const std::size_t lo = std::min(before.cursor, before.anchor), hi = std::max(before.cursor, before.anchor);
    if (before.text.substr(lo, hi - lo) != get_selected_text())
    {
        q->notify_changed(q->selected_text);
    }
    // ハンドラの中でこのウィジェットが破棄されてもよいよう、最後に呼ぶ。
    if (text_changed)
    {
        Object* self = q;
        detail::fire(on_change_, self);
    }
}

// --- プロパティ ---

void TextInputBase::Impl::set_text(const std::string& v)
{
    const State before = state();
    text_ = sanitize(v);
    cursor_ = anchor_ = text_.size();
    preedit_.clear();
    goal_x_ = -1;
    text_touched();
    finish(before);
}

void TextInputBase::Impl::set_placeholder_text(const std::string& v)
{
    placeholder_text_ = v;
    placeholder_layout_ = TextLayout();
    q->invalidate();
}

void TextInputBase::Impl::set_font(const Font& v)
{
    if (v == font_)
    {
        return;
    }
    font_ = v;
    invalidate_layout();
    placeholder_layout_ = TextLayout();
    q->invalidate();
    q->update_input_method();
    q->request_layout();
}

void TextInputBase::Impl::set_cursor_position(const std::size_t& v)
{
    const State before = state();
    goal_x_ = -1;
    move_cursor(std::min(v, text_.size()), false);
    finish(before);
}

std::string TextInputBase::Impl::get_selected_text() const
{
    const std::size_t lo = std::min(cursor_, anchor_), hi = std::max(cursor_, anchor_);
    return text_.substr(lo, hi - lo);
}

// --- 操作 ---

void TextInputBase::select_all()
{
    const Impl::State before = d_->state();
    d_->anchor_ = 0;
    d_->move_cursor(d_->text_.size(), true);
    d_->finish(before);
}

void TextInputBase::deselect()
{
    const Impl::State before = d_->state();
    d_->move_cursor(d_->cursor_, false);
    d_->finish(before);
}

void TextInputBase::cut()
{
    if (!d_->get_has_selection())
    {
        return;
    }
    const Impl::State before = d_->state();
    copy();
    d_->erase_selection();
    d_->text_touched();
    d_->finish(before);
}

void TextInputBase::copy() const
{
    if (d_->get_has_selection())
    {
        clipboard::set_text(d_->get_selected_text());
    }
}

void TextInputBase::paste() { insert(clipboard::text()); }

void TextInputBase::insert(const std::string& s)
{
    const Impl::State before = d_->state();
    d_->insert_text(s);
    d_->finish(before);
}

// --- 内部の処理(通知はしない。呼び出し側が finish で知らせる) ---

void TextInputBase::Impl::insert_text(const std::string& s)
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

void TextInputBase::Impl::move_cursor(std::size_t pos, bool keep_anchor)
{
    cursor_ = pos;
    if (!keep_anchor)
    {
        anchor_ = cursor_;
    }
    caret_touched();
}

void TextInputBase::Impl::move_vertically(int lines, bool keep_anchor)
{
    const TextLayout& l = q->text_layout();
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

void TextInputBase::Impl::erase_selection()
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

void TextInputBase::Impl::text_touched()
{
    invalidate_layout();
    caret_touched();
}

void TextInputBase::Impl::caret_touched()
{
    restart_blink();
    update_scroll(true);
    q->invalidate();
    q->update_input_method();
}

const TextLayout& TextInputBase::text_layout() const
{
    const float scale = display_scale;
    const float wrap = wrap_width();
    if (d_->layout_.is_null() || d_->layout_.scale() != scale || d_->layout_wrap_ != wrap)
    {
        const std::string shown =
            d_->preedit_.empty() ? d_->text_ : d_->text_.substr(0, d_->cursor_) + d_->preedit_ + d_->text_.substr(d_->cursor_);
        d_->layout_ = TextLayout(shown, d_->font_, scale, wrap);
        d_->layout_wrap_ = wrap;
    }
    return d_->layout_;
}

void TextInputBase::Impl::invalidate_layout() { layout_ = TextLayout(); }

Point TextInputBase::Impl::caret_in_layout() const
{
    const TextLayout& l = q->text_layout();
    if (preedit_.empty())
    {
        return l.caret_position(cursor_);
    }
    const std::size_t in = preedit_start_ >= 0 ? detail::utf8_offset_of(preedit_, preedit_start_) : preedit_.size();
    return l.caret_position(cursor_ + in);
}

Point TextInputBase::Impl::text_origin() const
{
    const Rect area = q->text_area();
    const TextLayout& l = q->text_layout();
    if (multi_line_)
    {
        return Point(area.x - scroll_.x, area.y - scroll_.y);
    }
    // 1 行は縦の中央に置く。右から左の段落は、収まるなら右端から並べる。
    const float room = area.width - l.width();
    const float x = area.x - scroll_.x + ((l.line_is_rtl(0) && room > 0) ? room : 0);
    return Point(x, area.y + (area.height - l.height()) / 2);
}

void TextInputBase::Impl::update_scroll(bool follow_caret)
{
    const Rect area = q->text_area();
    if (area.width <= 0 || area.height <= 0)
    {
        return;
    }
    const TextLayout& l = q->text_layout();
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
        q->invalidate();
        q->update_input_method();
    }
    q->scroll_changed();
}

void TextInputBase::set_scroll_offset(const Point& p)
{
    d_->scroll_ = p;
    d_->update_scroll(false);
    invalidate();
}

std::size_t TextInputBase::Impl::offset_at(const Point& p) const { return q->text_layout().offset_at(p - text_origin()); }

void TextInputBase::Impl::restart_blink()
{
    caret_on_ = true;
    blink_.enabled = false;
    if (q->focused)
    {
        blink_.enabled = true;
    }
}

void TextInputBase::Impl::stop_blink() { blink_.enabled = false; }

// --- 描画 ---

void TextInputBase::paint_text(Canvas& canvas)
{
    const bool enabled_now = effectively_enabled, has_focus = focused;
    const TextLayout& l = text_layout();
    const Rect area = text_area();
    const Point o = d_->text_origin();
    const Color fg = enabled_now ? theme::text() : theme::text_disabled();

    canvas.save();
    canvas.clip_rect(area);

    if (d_->text_.empty() && d_->preedit_.empty() && !d_->placeholder_text_.empty())
    {
        if (d_->placeholder_layout_.is_null() || d_->placeholder_layout_.scale() != l.scale())
        {
            d_->placeholder_layout_ = TextLayout(d_->placeholder_text_, d_->font_, l.scale(), wrap_width());
        }
        canvas.draw_text(d_->placeholder_layout_, d_->multi_line_ ? Point(area.x, area.y) : Point(area.x, o.y),
                         theme::placeholder());
    }

    // 選択の矩形は、双方向の行・複数行では離れた複数の矩形になる(docs/adr/0020)。
    std::vector<Rect> selection;
    if (d_->get_has_selection() && d_->preedit_.empty())
    {
        selection = l.selection_rects(std::min(d_->cursor_, d_->anchor_), std::max(d_->cursor_, d_->anchor_));
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

    if (!d_->preedit_.empty())
    {
        // 変換中の文字列に細い下線、変換対象の文節に太い下線(docs/adr/0009)。
        const float under = l.ascent() + 2;
        const std::vector<Rect> whole = l.selection_rects(d_->cursor_, d_->cursor_ + d_->preedit_.size());
        for (std::size_t i = 0; i < whole.size(); ++i)
        {
            canvas.fill_rect(Rect(o.x + whole[i].x, o.y + whole[i].y + under, whole[i].width, 1),
                             theme::preedit_underline());
        }
        if (d_->preedit_start_ >= 0 && d_->preedit_length_ > 0)
        {
            const std::vector<Rect> target =
                l.selection_rects(d_->cursor_ + detail::utf8_offset_of(d_->preedit_, d_->preedit_start_),
                                  d_->cursor_ + detail::utf8_offset_of(d_->preedit_, d_->preedit_start_ + d_->preedit_length_));
            for (std::size_t i = 0; i < target.size(); ++i)
            {
                canvas.fill_rect(Rect(o.x + target[i].x, o.y + target[i].y + under - 1, target[i].width, 2),
                                 theme::preedit_underline());
            }
        }
    }

    if (has_focus && d_->caret_on_ && (selection.empty() || !d_->preedit_.empty()))
    {
        const Point c = d_->caret_in_layout();
        canvas.fill_rect(Rect(o.x + c.x, o.y + c.y, 1, l.ascent() + l.descent()), fg);
    }

    canvas.restore();
}

// --- イベント ---

void TextInputBase::resized()
{
    invalidate();
    d_->update_scroll(true);
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
    if (!d_->preedit_.empty())
    {
        return;  // 変換中はキャレットを動かさない
    }
    d_->goal_x_ = -1;
    if (e.clicks >= 3)
    {
        select_all();  // トリプルクリックで全体
        return;
    }
    if (e.clicks == 2)
    {
        // ダブルクリックで単語を選ぶ。
        const Impl::State before = d_->state();
        std::size_t start = 0, end = 0;
        text_layout().word_at(d_->offset_at(e.pos), start, end);
        d_->anchor_ = start;
        d_->move_cursor(end, true);
        d_->finish(before);
        return;
    }
    const Impl::State before = d_->state();
    d_->move_cursor(d_->offset_at(e.pos), e.has_modifier(MODIFIER_SHIFT));
    d_->selecting_ = true;
    d_->finish(before);
}

void TextInputBase::mouse_move(MouseEvent& e)
{
    Widget::mouse_move(e);
    if (d_->selecting_)
    {
        const Impl::State before = d_->state();
        d_->move_cursor(d_->offset_at(e.pos), true);
        d_->finish(before);
    }
}

void TextInputBase::mouse_up(MouseEvent& e)
{
    Widget::mouse_up(e);
    if (e.button == MouseButton::LEFT)
    {
        e.accepted = true;
        d_->selecting_ = false;
    }
}

void TextInputBase::key_down(KeyEvent& e)
{
    Widget::key_down(e);
    // 変換中のキーは IME のもの(docs/adr/0009)。
    if (!d_->preedit_.empty())
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
    if (e.key == Key::RETURN && !d_->multi_line_)
    {
        // ハンドラの中で破棄されてもよいよう、submit の後は自分に触らない(e は呼び出し側のもの)。
        e.accepted = submit();
        return;
    }

    const TextLayout& l = text_layout();
    const std::size_t lo = std::min(d_->cursor_, d_->anchor_), hi = std::max(d_->cursor_, d_->anchor_);
    const bool selected = d_->get_has_selection();
    const Impl::State before = d_->state();
    bool handled = true;
    bool vertical = false;  // 上下の移動(キャレットの横の位置を保つ)
    switch (e.key)
    {
    case Key::LEFT:
        d_->move_cursor(ctrl ? l.previous_word_offset(d_->cursor_) : (selected && !shift ? lo : l.previous_offset(d_->cursor_)),
                    shift);
        break;
    case Key::RIGHT:
        d_->move_cursor(ctrl ? l.next_word_offset(d_->cursor_) : (selected && !shift ? hi : l.next_offset(d_->cursor_)), shift);
        break;
    case Key::UP:
    case Key::DOWN:
    case Key::PAGE_UP:
    case Key::PAGE_DOWN:
        if (!d_->multi_line_)
        {
            handled = false;
            break;
        }
        {
            // Page Up・Down は見えている行の数だけ動かす。
            const int page = std::max(1, static_cast<int>(text_area().height / std::max(1.0f, l.line_height())) - 1);
            const int lines = e.key == Key::UP ? -1 : e.key == Key::DOWN ? 1 : e.key == Key::PAGE_UP ? -page : page;
            d_->move_vertically(lines, shift);
            vertical = true;
        }
        break;
    case Key::HOME:
        // 複数行では行の始め(Ctrl で全体の始め)。
        d_->move_cursor(d_->multi_line_ && !ctrl ? l.line_start(l.line_of(d_->cursor_)) : 0, shift);
        break;
    case Key::END:
        d_->move_cursor(d_->multi_line_ && !ctrl ? l.line_end(l.line_of(d_->cursor_)) : d_->text_.size(), shift);
        break;
    case Key::RETURN:
        d_->insert_text("\n");  // 複数行では改行を入れる
        break;
    case Key::BACKSPACE:
        if (selected)
        {
            d_->erase_selection();
            d_->text_touched();
        }
        else if (d_->cursor_ > 0)
        {
            // Ctrl なら単語の始めまで消す。
            const std::size_t prev = ctrl ? l.previous_word_offset(d_->cursor_) : l.previous_offset(d_->cursor_);
            d_->text_.erase(prev, d_->cursor_ - prev);
            d_->cursor_ = d_->anchor_ = prev;
            d_->text_touched();
        }
        break;
    case Key::DEL:
        if (selected)
        {
            d_->erase_selection();
            d_->text_touched();
        }
        else if (d_->cursor_ < d_->text_.size())
        {
            const std::size_t next = ctrl ? l.next_word_offset(d_->cursor_) : l.next_offset(d_->cursor_);
            d_->text_.erase(d_->cursor_, next - d_->cursor_);
            d_->anchor_ = d_->cursor_;
            d_->text_touched();
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
            d_->goal_x_ = -1;
        }
        e.accepted = true;
        d_->finish(before);
    }
}

void TextInputBase::text_input(TextInputEvent& e)
{
    const Impl::State before = d_->state();
    if (!d_->preedit_.empty())
    {
        d_->preedit_.clear();
        d_->invalidate_layout();
    }
    d_->insert_text(e.text);
    d_->finish(before);
}

void TextInputBase::text_editing(TextEditingEvent& e)
{
    const Impl::State before = d_->state();
    if (e.text.empty())
    {
        if (!d_->preedit_.empty())
        {
            d_->preedit_.clear();
            d_->invalidate_layout();
            invalidate();
            update_input_method();
        }
        return;
    }
    // 変換を始めるときに選択していた部分は、変換中の文字列で置き換える。
    if (d_->preedit_.empty() && d_->get_has_selection())
    {
        d_->erase_selection();
    }
    d_->preedit_ = e.text;
    d_->preedit_start_ = e.start;
    d_->preedit_length_ = e.length;
    d_->text_touched();
    d_->finish(before);
}

void TextInputBase::focus_in()
{
    d_->restart_blink();
    invalidate();
    Widget::focus_in();
}

void TextInputBase::focus_out()
{
    d_->stop_blink();
    d_->selecting_ = false;
    if (!d_->preedit_.empty())
    {
        d_->preedit_.clear();
        d_->invalidate_layout();
    }
    invalidate();
    Widget::focus_out();
}

void TextInputBase::scale_changed()
{
    d_->invalidate_layout();
    d_->placeholder_layout_ = TextLayout();
}

Rect TextInputBase::input_method_rect() const
{
    const TextLayout& l = text_layout();
    const Point c = d_->caret_in_layout() + d_->text_origin();
    return Rect(c.x, c.y, 1, l.ascent() + l.descent());
}

}  // namespace tokotachi
