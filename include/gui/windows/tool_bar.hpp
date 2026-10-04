// ツールバー: Action をボタンとして横に並べる(docs/adr/0030)。
//
//   gui::ToolBar* tools = new gui::ToolBar();
//   tools->add_action(open);                 // 既にある Action(メニューと共有できる)
//   tools->add_item("保存", on_save)->icon = save_icon;
//   tools->add_separator();
//   tools->add_widget(new gui::ComboBox());  // 任意の部品
//   window.tool_bar = tools;                 // メニューバーの下に置く
//
// ボタンは Action の画像(無ければ文字)を出し、文字とショートカットはツールチップに出す(button_style で変えられる)。
// 幅に収まらない項目は隠し、右端の » のメニューから選べる(add_widget の部品は隠すだけ)。
// ウィンドウの tool_bar の Action のショートカットは、ウィンドウ全体で効く。
#ifndef GUI_TOOL_BAR_HPP
#define GUI_TOOL_BAR_HPP

#include <string>
#include <vector>

#include "../actions/action.hpp"
#include "../widgets/widget.hpp"

namespace gui
{

class Menu;

namespace detail
{
class ToolButton;
}

enum class ToolButtonStyle
{
    ICON_ONLY,         // 画像だけ(画像が無ければ文字)
    TEXT_BESIDE_ICON,  // 画像の右に文字
    TEXT_UNDER_ICON    // 画像の下に文字
};

class ToolBar : public Widget
{
public:
    explicit ToolBar(Widget* parent = nullptr);
    ~ToolBar();

    // text の Action を作ってボタンとして加え、返す(このツールバーが所有する)。
    Action* add_item(const std::string& text, const NotifyHandler& on_trigger = NotifyHandler());
    void add_separator();
    // 任意の部品を加える(ツールバーの子になる)。
    void add_widget(Widget* widget);

    Property<ToolButtonStyle> button_style;

protected:
    void paint(Canvas& canvas) override;
    void arrange_children() override;
    void child_removed(Widget* child) override;
    void action_added(Action* action) override;
    void action_removed(Action* action) override;
    void action_changed(Action* action) override;
    Size compute_preferred_size() const override;
    Size compute_minimum_size() const override;

private:
    // 項目: ボタン(Action)・部品・区切りのどれか。
    struct Entry
    {
        Action* action;
        detail::ToolButton* button;
        Widget* widget;
        bool hidden;  // 幅に収まらず隠している
        float x;      // 区切りの位置
    };

    ToolButtonStyle get_button_style() const { return button_style_; }
    void set_button_style(const ToolButtonStyle& v);
    Widget* item_widget(const Entry& e) const;
    float separator_width() const;
    void show_overflow();

    std::vector<Entry> entries_;
    ToolButtonStyle button_style_;
    detail::ToolButton* overflow_button_;  // »
    Menu* overflow_menu_;
    bool destroying_;
};

}  // namespace gui

#endif
