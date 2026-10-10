// 押して使うウィジェット(Button・CheckBox・RadioButton)の共通の部分: 文字、マウス・キーでの押下、ホバー。
#ifndef TOKOTACHI_WIDGETS_BUTTON_BASE_HPP
#define TOKOTACHI_WIDGETS_BUTTON_BASE_HPP

#include <string>

#include "../graphics/text_layout.hpp"
#include "widget.hpp"

namespace tokotachi
{

class ButtonBase : public Widget
{
public:
    explicit ButtonBase(Widget* parent = nullptr, const std::string& text = std::string());

    Property<std::string> text;
    Property<Font> font;

    // 上で押して離した・フォーカスがあるときに Space か Enter を押した。
    Property<NotifyHandler> on_click;

protected:
    // 押された。派生クラスは状態を変えてから基底の実装を呼ぶ(基底の実装が on_click を呼ぶ)。
    virtual void click();

    void mouse_down(MouseEvent& e) override;
    void mouse_up(MouseEvent& e) override;
    void mouse_move(MouseEvent& e) override;
    void mouse_enter() override;
    void mouse_leave() override;
    void key_down(KeyEvent& e) override;
    void focus_in() override;
    void focus_out() override;
    void scale_changed() override;
    // 文字が切れないよう、望む大きさを最小とする(派生クラスの compute_preferred_size を使う)。
    Size compute_minimum_size() const override;

    // 描画のための状態。
    bool is_hovered() const { return hovered_; }
    bool is_pressed() const { return pressed_in_; }
    const TextLayout& text_layout() const;

private:
    void set_text(const std::string& v);
    void set_font(const Font& v);

    std::string text_;
    Font font_;
    NotifyHandler on_click_;
    bool hovered_;
    bool pressed_;     // 左ボタンを押している(外に出ても続く)
    bool pressed_in_;  // 押していて、ポインタが上にある
    mutable TextLayout layout_;
};

}  // namespace tokotachi

#endif
