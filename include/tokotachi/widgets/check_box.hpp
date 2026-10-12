/// @file
/// tokotachi::CheckBox の宣言。
#ifndef TOKOTACHI_WIDGETS_CHECK_BOX_HPP
#define TOKOTACHI_WIDGETS_CHECK_BOX_HPP

#include "../core/export.hpp"
#include "button_base.hpp"

namespace tokotachi
{

/// チェックボックス。押すと checked が切り替わる。
///
/// @image html widget_check_box.png
///
/// @code
/// tokotachi::CheckBox* check_box_notify = new tokotachi::CheckBox(window_main, "通知を受け取る");
/// tokotachi::bind(check_box_notify->checked, notify->value);   // BoolVariable と結ぶ
/// @endcode
///
/// @ingroup widgets_input
class TOKOTACHI_API CheckBox : public ButtonBase
{
public:
    explicit CheckBox(Widget* parent = nullptr, const std::string& text = std::string());
    ~CheckBox() override;

    Property<bool> checked;

    /// checked が変わった(利用者の操作・代入のどちらでも。docs/adr/0016)。押したときは on_change の後に on_click を呼ぶ。
    Property<NotifyHandler> on_change;

protected:
    /// checked を切り替えてから on_click を呼ぶ。
    void click() override;

    void paint(Canvas& canvas) override;
    Size compute_preferred_size() const override;

private:
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
