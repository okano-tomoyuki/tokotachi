/// @file
/// tokotachi::LineEdit の宣言。
#ifndef TOKOTACHI_WIDGETS_LINE_EDIT_HPP
#define TOKOTACHI_WIDGETS_LINE_EDIT_HPP

#include <string>

#include "../core/export.hpp"
#include "text_input_base.hpp"

namespace tokotachi
{

/// 1 行のテキストの入力欄。IME の変換中の文字列は入力欄の中に描く(docs/adr/0009)。
/// 編集・選択・クリップボード・変更の通知は TextInputBase が受け持つ(docs/adr/0021)。
///
/// @image html widget_line_edit.png
///
/// @ingroup widgets_input
class TOKOTACHI_API LineEdit : public TextInputBase
{
public:
    explicit LineEdit(Widget* parent = nullptr, const std::string& text = std::string());
    ~LineEdit() override;

    /// Enter が押された。ハンドラが無ければ、Enter は親へ送る(ダイアログでは決定のボタンを押す)。
    Property<NotifyHandler> on_submit;

protected:
    void paint(Canvas& canvas) override;
    Size compute_preferred_size() const override;
    Size compute_minimum_size() const override;

private:
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
