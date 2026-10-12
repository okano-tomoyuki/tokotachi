// TextInputBase の実装(内部。LineEdit・TextEdit などが使う。docs/adr/0057)。
#ifndef TOKOTACHI_SRC_WIDGETS_TEXT_INPUT_IMPL_HPP
#define TOKOTACHI_SRC_WIDGETS_TEXT_INPUT_IMPL_HPP

#include <string>
#include <vector>
#include <tokotachi/widgets/text_input_base.hpp>
#include "../core/impl.hpp"
#include "widget_access.hpp"

namespace tokotachi
{

namespace detail
{

/// 入力欄の種類ごとの違い(内部。docs/adr/0058)。LineEdit・TextEdit の実装のクラスが継いで、TextInputBase に差し込む。
class TextInputHooks
{
public:
    virtual ~TextInputHooks() {}
    /// 文字を描く矩形(部品の座標系)。
    virtual Rect text_area() const = 0;
    /// 折り返す幅(0 なら折り返さない)。
    virtual float wrap_width() const { return 0; }
    /// Enter が押された(使ったら true)。
    virtual bool submit() { return false; }
    /// スクロールの位置が変わった。
    virtual void scroll_changed() {}
};

}  // namespace detail

class TextInputBase::Impl : public detail::WidgetHooks
{
    TOKOTACHI_IMPL(TextInputBase);

    Impl(TextInputBase* object, bool multi_line) : q(object), input_(nullptr), multi_line_(multi_line) {}

    /// 差し込まれている入力欄の種類ごとの違い(LineEdit・TextEdit の実装)。
    detail::TextInputHooks* input_;

    /// 文字・選択・変換中の文字列・キャレットを、text_area の中に描く(派生クラスの paint が枠を描いた後に呼ぶ)。
    void paint_text(Canvas& canvas);
    /// 表示するテキスト(変換中の文字列をキャレットの位置に挟んだもの)のレイアウト。
    const TextLayout& text_layout() const;
    /// スクロールの量(dp。文字の左上が、text_area の左上からずれている量)。
    Point scroll_offset() const;
    /// スクロールの量を変える(範囲の外の値は丸める)。キャレットを追わない。
    void set_scroll_offset(const Point& p);
    bool is_multi_line() const;

    // 内部のフック(docs/adr/0058)
    Rect input_method_rect() const override;

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

}  // namespace tokotachi

#endif
