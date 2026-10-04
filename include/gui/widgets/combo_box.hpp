// 項目の一覧から 1 つを選ぶ(docs/adr/0022)。押すと一覧をポップアップで出す。
//
//   gui::ComboBox* size = new gui::ComboBox(&window);
//   size->items = std::vector<std::string>{"小", "中", "大"};
//   size->current_index = 1;
//   size->on_change = [](gui::Object* sender) { ... };
//
// 一覧が閉じているときは ↑↓ で選択を変え、Space・Enter・Alt+↓ で一覧を開く。一覧では ↑↓・Page Up/Down・Home/End で動かし、
// Enter で決め、Esc で閉じる。
#ifndef GUI_COMBO_BOX_HPP
#define GUI_COMBO_BOX_HPP

#include <string>
#include <vector>

#include "../windows/popup.hpp"
#include "../graphics/text_layout.hpp"
#include "widget.hpp"

namespace gui
{

namespace detail
{
class PopupList;
}

class ScrollArea;

class ComboBox : public Widget
{
public:
    explicit ComboBox(Widget* parent = nullptr);

    Property<std::vector<std::string> > items;
    // 選んでいる項目(-1 なら選んでいない)。範囲の外の値は -1 にする。items を変えて範囲の外になったときも -1 になる。
    Property<int> current_index;
    ReadOnlyProperty<std::string> current_text;
    // 選んでいないときに出す文字。
    Property<std::string> placeholder_text;
    Property<Font> font;

    // current_index が変わった(利用者の操作・代入のどちらでも)。
    Property<NotifyHandler> on_change;

    void open_popup();
    void close_popup();
    bool is_popup_open() const;

protected:
    void paint(Canvas& canvas) override;
    void mouse_down(MouseEvent& e) override;
    void mouse_enter() override;
    void mouse_leave() override;
    void key_down(KeyEvent& e) override;
    void focus_in() override;
    void focus_out() override;
    void scale_changed() override;
    Size compute_preferred_size() const override;
    Size compute_minimum_size() const override;

private:
    std::vector<std::string> get_items() const { return items_; }
    void set_items(const std::vector<std::string>& v);
    int get_current_index() const { return current_index_; }
    void set_current_index(const int& v);
    std::string get_current_text() const;
    void set_placeholder_text(const std::string& v);
    void set_font(const Font& v);
    const TextLayout& text_layout() const;

    std::vector<std::string> items_;
    int current_index_;
    std::string placeholder_text_;
    Font font_;
    NotifyHandler on_change_;
    bool hovered_;
    Popup* popup_;
    ScrollArea* scroll_;
    detail::PopupList* list_;
    mutable TextLayout layout_;
};

}  // namespace gui

#endif
