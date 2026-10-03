// 1 行のテキストの入力欄。IME の変換中の文字列は入力欄の中に描く(docs/adr/0009)。
// 編集・選択・クリップボード・変更の通知は TextInputBase が受け持つ(docs/adr/0021)。
#ifndef GUI_LINE_EDIT_HPP
#define GUI_LINE_EDIT_HPP

#include <string>

#include "text_input_base.hpp"

namespace gui
{

class LineEdit : public TextInputBase
{
public:
    explicit LineEdit(Widget* parent = nullptr, const std::string& text = std::string());

    // Enter が押された。
    Property<NotifyHandler> on_submit;

protected:
    Rect text_area() const override;
    void submit() override;
    void paint(Canvas& canvas) override;
    Size compute_preferred_size() const override;
    Size compute_minimum_size() const override;

private:
    NotifyHandler on_submit_;
};

}  // namespace gui

#endif
