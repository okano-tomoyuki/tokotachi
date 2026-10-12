/// @file
/// tokotachi::TextEdit の宣言。
#ifndef TOKOTACHI_WIDGETS_TEXT_EDIT_HPP
#define TOKOTACHI_WIDGETS_TEXT_EDIT_HPP

#include <string>

#include "../core/export.hpp"
#include "scroll_bar.hpp"
#include "text_input_base.hpp"

namespace tokotachi
{

/// 複数行のテキストの入力欄(docs/adr/0021)。
///
/// @image html widget_text_edit.png
///
/// @code
/// tokotachi::TextEdit* text_edit_memo = new tokotachi::TextEdit(window_main);
/// text_edit_memo->text = "1 行目\n2 行目";
/// @endcode
///
/// Enter で改行を入れる。word_wrap(既定 true)なら幅で折り返す。文字が収まらなければ縦にスクロールし、右にスクロールバーを出す
/// (スクロールバーの幅は常に空けておき、出し入れで折り返しが変わらないようにする)。
/// 編集・選択・クリップボード・IME・変更の通知は TextInputBase が受け持つ。
///
/// @ingroup widgets_input
class TOKOTACHI_API TextEdit : public TextInputBase
{
public:
    explicit TextEdit(Widget* parent = nullptr, const std::string& text = std::string());
    ~TextEdit() override;

    Property<bool> word_wrap;

protected:
    void arrange_children() override;
    void paint(Canvas& canvas) override;
    void mouse_wheel(WheelEvent& e) override;
    Size compute_preferred_size() const override;
    Size compute_minimum_size() const override;

private:
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
