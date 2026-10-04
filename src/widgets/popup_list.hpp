// ポップアップの中の項目の一覧(内部。ComboBox が使う。docs/adr/0022)。
#ifndef GUI_SRC_WIDGETS_POPUP_LIST_HPP
#define GUI_SRC_WIDGETS_POPUP_LIST_HPP

#include <functional>
#include <string>
#include <vector>

#include <gui/graphics/text_layout.hpp>
#include <gui/widgets/widget.hpp>

namespace gui
{
namespace detail
{

class PopupList : public Widget
{
public:
    explicit PopupList(Widget* parent);

    void set_items(const std::vector<std::string>& items, const Font& font);
    // 強調する項目(-1 なら無し)。見えるようにスクロールする。
    void highlight(int index);
    int highlighted() const { return highlighted_; }
    float row_height() const;
    std::size_t count() const { return items_.size(); }

    std::function<void(int)> on_choose;           // 項目を選んだ
    std::function<void()> on_cancel;              // Esc で閉じる
    std::function<void(const Rect&)> on_reveal;   // この矩形を見えるようにしてほしい(スクロール)

protected:
    void paint(Canvas& canvas) override;
    void mouse_move(MouseEvent& e) override;
    void mouse_down(MouseEvent& e) override;
    void mouse_up(MouseEvent& e) override;
    void key_down(KeyEvent& e) override;
    void scale_changed() override;
    Size compute_preferred_size() const override;

private:
    int row_at(float y) const;
    const TextLayout& layout(std::size_t i) const;

    std::vector<std::string> items_;
    Font font_;
    int highlighted_;
    bool pressed_;
    mutable std::vector<TextLayout> layouts_;
};

}  // namespace detail
}  // namespace gui

#endif
