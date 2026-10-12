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
    ~TextInputBase() override;

    /// 1 行なら改行・タブなどの制御文字を取り除く。複数行なら改行を \n にそろえ、タブは空白にし、その他の制御文字を取り除く。
    Property<std::string> text;
    /// 空のときに薄く出す説明の文字。
    Property<std::string> placeholder_text;
    /// 文字のフォント。
    Property<Font> font;
    /// キャレットの位置(UTF-8 のバイトの位置)。代入すると選択は外れる。
    Property<std::size_t> cursor_position;
    /// 文字を選んでいるか。
    ReadOnlyProperty<bool> has_selection;
    /// 選んでいる文字。
    ReadOnlyProperty<std::string> selected_text;

    /// テキストが変わった(利用者の操作・text への代入のどちらでも。同じ値の代入では呼ばない。docs/adr/0016)。
    Property<NotifyHandler> on_change;

    /// すべての文字を選ぶ。
    void select_all();
    /// 選択を外す(文字は消さない)。
    void clear_selection();
    /// 選んでいる文字をクリップボードに移す。
    void cut();
    /// 選んでいる文字をクリップボードに写す。
    void copy() const;
    /// クリップボードの文字を、選択を消して入れる。
    void paste();
    /// 選択を消して s を入れる。
    void insert(const std::string& s);

protected:
    /// ライブラリの中だけで派生する(LineEdit・TextEdit)。
    TextInputBase(Widget* parent, const detail::InternalTag& tag, const std::string& text, bool multi_line);

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
    template <typename>
    friend struct detail::ImplAccess;
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
