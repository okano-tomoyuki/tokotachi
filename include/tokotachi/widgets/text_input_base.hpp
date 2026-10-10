// 文字を入力するウィジェット(LineEdit・TextEdit)の共通の部分(docs/adr/0009、0021)。
// 編集・選択・クリップボード・IME の変換中の文字列・キャレットの点滅・スクロール・変更の通知を受け持つ。
// 派生クラスは、文字を描く矩形(text_area)・折り返しの幅(wrap_width)と、枠の描画を決める。
#ifndef TOKOTACHI_WIDGETS_TEXT_INPUT_BASE_HPP
#define TOKOTACHI_WIDGETS_TEXT_INPUT_BASE_HPP

#include <cstddef>
#include <string>

#include "../graphics/text_layout.hpp"
#include "../core/timer.hpp"
#include "widget.hpp"

namespace tokotachi
{

class TextInputBase : public Widget
{
public:
    // 1 行なら改行・タブなどの制御文字を取り除く。複数行なら改行を \n にそろえ、タブは空白にし、その他の制御文字を取り除く。
    Property<std::string> text;
    Property<std::string> placeholder_text;
    Property<Font> font;
    // キャレットの位置(UTF-8 のバイトの位置)。代入すると選択は外れる。
    Property<std::size_t> cursor_position;
    ReadOnlyProperty<bool> has_selection;
    ReadOnlyProperty<std::string> selected_text;

    // テキストが変わった(利用者の操作・text への代入のどちらでも。同じ値の代入では呼ばない。docs/adr/0016)。
    Property<NotifyHandler> on_change;

    void select_all();
    void deselect();
    void cut();
    void copy() const;
    void paste();
    // 選択を消して s を入れる。
    void insert(const std::string& s);

protected:
    TextInputBase(Widget* parent, const std::string& text, bool multi_line);

    // 文字を描く矩形(自分の座標系。枠の内側)。
    virtual Rect text_area() const = 0;
    // 折り返しの幅(dp。0 なら折り返さない)。
    virtual float wrap_width() const { return 0; }
    // Enter が押された(1 行のときだけ呼ぶ。複数行では改行を入れる)。受け取ったら true を返す。
    // false なら Enter は親へ送る(ダイアログの決定のボタンなど)。
    virtual bool submit() { return false; }
    // スクロールの量が変わった(TextEdit がスクロールバーを合わせる)。
    virtual void scroll_changed() {}

    // 文字・選択・変換中の文字列・キャレットを、text_area の中に描く(派生クラスの paint が枠を描いた後に呼ぶ)。
    void paint_text(Canvas& canvas);
    // 表示するテキスト(変換中の文字列をキャレットの位置に挟んだもの)のレイアウト。
    const TextLayout& text_layout() const;
    // スクロールの量(dp。文字の左上が、text_area の左上からずれている量)。
    Point scroll_offset() const { return scroll_; }
    // スクロールの量を変える(範囲の外の値は丸める)。キャレットを追わない。
    void set_scroll_offset(const Point& p);
    bool is_multi_line() const { return multi_line_; }

    void resized() override;
    void mouse_down(MouseEvent& e) override;
    void mouse_up(MouseEvent& e) override;
    void mouse_move(MouseEvent& e) override;
    void key_down(KeyEvent& e) override;
    void text_input(TextInputEvent& e) override;
    void text_editing(TextEditingEvent& e) override;
    void focus_in() override;
    void focus_out() override;
    void scale_changed() override;
    Rect input_method_rect() const override;

private:
    // 操作の前の状態。操作の後に、変わったプロパティを知らせるために使う(docs/adr/0016)。
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
    // before から変わったプロパティを知らせ、テキストが変わっていれば on_change を呼ぶ。
    void finish(const State& before);
    std::string sanitize(const std::string& s) const;

    void invalidate_layout();
    // テキストが変わった後・キャレットが動いた後の描き直し(キャレットが見えるようにスクロールする)。
    void text_touched();
    void caret_touched();
    // レイアウトの中での、キャレットの位置(変換中ならその中のキャレット)。
    Point caret_in_layout() const;
    // テキストのレイアウトの左上(自分の座標系。スクロールと、1 行のときの縦の中央寄せ・右から左の右寄せを含む)。
    Point text_origin() const;
    // キャレットが見えるようにスクロールし(follow_caret のとき)、範囲に収める。
    void update_scroll(bool follow_caret);
    std::size_t offset_at(const Point& p) const;
    void move_cursor(std::size_t pos, bool keep_anchor);
    // 上下の行へ動かす(lines が負なら上)。キャレットの横の位置(goal_x_)を保つ。
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
    bool selecting_;  // マウスで選択している
    bool caret_on_;
    float goal_x_;  // 上下に動かすときに保つキャレットの横の位置(負なら未定)
    Timer blink_;   // キャレットの点滅
    Point scroll_;
    mutable TextLayout layout_;
    mutable float layout_wrap_;
    mutable TextLayout placeholder_layout_;
};

}  // namespace tokotachi

#endif
