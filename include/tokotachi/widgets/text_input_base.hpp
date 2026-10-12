/// @file
/// tokotachi::TextInputBase の宣言。
#ifndef TOKOTACHI_WIDGETS_TEXT_INPUT_BASE_HPP
#define TOKOTACHI_WIDGETS_TEXT_INPUT_BASE_HPP

#include <cstddef>
#include <string>

#include "../core/export.hpp"
#include "../graphics/text_layout.hpp"
#include "../core/timer.hpp"
#include "widget.hpp"

namespace tokotachi
{

/// 文字を入力するウィジェット(LineEdit・TextEdit)の共通の部分(docs/adr/0009、0021)。
/// 編集・選択・クリップボード・IME の変換中の文字列・キャレットの点滅・スクロール・変更の通知を受け持つ。
/// 派生クラスは、文字を描く矩形(text_area)・折り返しの幅(wrap_width)と、枠の描画を決める。
///
/// @ingroup widgets_base
class TOKOTACHI_API TextInputBase : public Widget
{
public:
    /// 1 行なら改行・タブなどの制御文字を取り除く。複数行なら改行を \n にそろえ、タブは空白にし、その他の制御文字を取り除く。
    Property<std::string> text;
    Property<std::string> placeholder_text;
    Property<Font> font;
    /// キャレットの位置(UTF-8 のバイトの位置)。代入すると選択は外れる。
    Property<std::size_t> cursor_position;
    ReadOnlyProperty<bool> has_selection;
    ReadOnlyProperty<std::string> selected_text;

    /// テキストが変わった(利用者の操作・text への代入のどちらでも。同じ値の代入では呼ばない。docs/adr/0016)。
    Property<NotifyHandler> on_change;

    void select_all();
    void deselect();
    void cut();
    void copy() const;
    void paste();
    /// 選択を消して s を入れる。
    void insert(const std::string& s);

protected:
    TextInputBase(Widget* parent, const std::string& text, bool multi_line);
    ~TextInputBase() override;

    /// 文字を描く矩形(自分の座標系。枠の内側)。
    virtual Rect text_area() const = 0;
    /// 折り返しの幅(dp。0 なら折り返さない)。
    virtual float wrap_width() const { return 0; }
    /// Enter が押された(1 行のときだけ呼ぶ。複数行では改行を入れる)。受け取ったら true を返す。
    /// false なら Enter は親へ送る(ダイアログの決定のボタンなど)。
    virtual bool submit() { return false; }
    /// スクロールの量が変わった(TextEdit がスクロールバーを合わせる)。
    virtual void scroll_changed() {}

    /// 文字・選択・変換中の文字列・キャレットを、text_area の中に描く(派生クラスの paint が枠を描いた後に呼ぶ)。
    void paint_text(Canvas& canvas);
    /// 表示するテキスト(変換中の文字列をキャレットの位置に挟んだもの)のレイアウト。
    const TextLayout& text_layout() const;
    /// スクロールの量(dp。文字の左上が、text_area の左上からずれている量)。
    Point scroll_offset() const;
    /// スクロールの量を変える(範囲の外の値は丸める)。キャレットを追わない。
    void set_scroll_offset(const Point& p);
    bool is_multi_line() const;

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

private:
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
