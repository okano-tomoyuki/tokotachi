// チェックボックス。押すと checked が切り替わる。
//
//   tokotachi::CheckBox* check = new tokotachi::CheckBox(&window, "通知を受け取る");
//   tokotachi::bind(check->checked, notify->value);   // BoolVariable と結ぶ
#ifndef TOKOTACHI_WIDGETS_CHECK_BOX_HPP
#define TOKOTACHI_WIDGETS_CHECK_BOX_HPP

#include "button_base.hpp"

namespace tokotachi
{

class CheckBox : public ButtonBase
{
public:
    explicit CheckBox(Widget* parent = nullptr, const std::string& text = std::string());

    Property<bool> checked;

    // checked が変わった(利用者の操作・代入のどちらでも。docs/adr/0016)。押したときは on_change の後に on_click を呼ぶ。
    Property<NotifyHandler> on_change;

protected:
    // checked を切り替えてから on_click を呼ぶ。
    void click() override;

    void paint(Canvas& canvas) override;
    Size compute_preferred_size() const override;

private:
    void set_checked(const bool& v);

    bool checked_;
    NotifyHandler on_change_;
};

}  // namespace tokotachi

#endif
