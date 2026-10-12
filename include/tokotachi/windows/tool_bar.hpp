/// @file
/// tokotachi::ToolBar の宣言。
#ifndef TOKOTACHI_WINDOWS_TOOL_BAR_HPP
#define TOKOTACHI_WINDOWS_TOOL_BAR_HPP

#include <string>
#include <vector>

#include "../actions/action.hpp"
#include "../core/export.hpp"
#include "../widgets/widget.hpp"

namespace tokotachi
{

class Menu;

enum class ToolButtonStyle
{
    ICON_ONLY,         ///< 画像だけ(画像が無ければ文字)
    TEXT_BESIDE_ICON,  ///< 画像の右に文字
    TEXT_UNDER_ICON    ///< 画像の下に文字
};

/// ツールバー: Action をボタンとして横に並べる(docs/adr/0030)。
///
/// @image html widget_window_bars.png
///
/// @code
/// tokotachi::ToolBar* tool_bar = new tokotachi::ToolBar();
/// tool_bar->add_action(action_open);                 // 既にある Action(メニューと共有できる)
/// tool_bar->add_item("保存", on_save)->icon = save_icon;
/// tool_bar->add_separator();
/// tool_bar->add_widget(new tokotachi::ComboBox());  // 任意の部品
/// window.tool_bar = tool_bar;                 // メニューバーの下に置く
/// @endcode
///
/// ボタンは Action の画像(無ければ文字)を出し、文字とショートカットはツールチップに出す(button_style で変えられる)。
/// 幅に収まらない項目は隠し、右端の » のメニューから選べる(add_widget の部品は隠すだけ)。
/// 幅に収まらずに隠すときは、部品の visible を変えない(showing が false になる)。add_widget の部品を visible = false にすると、場所を取らない。
/// ウィンドウの tool_bar の Action のショートカットは、ウィンドウ全体で効く。
///
/// @ingroup windows
class TOKOTACHI_API ToolBar : public Widget
{
public:
    explicit ToolBar(Widget* parent = nullptr);
    ~ToolBar();

    /// text の Action を作ってボタンとして加え、返す(このツールバーが所有する)。
    Action* add_item(const std::string& text, const NotifyHandler& on_trigger = NotifyHandler());
    void add_separator();
    /// 任意の部品を加える(ツールバーの子になる)。
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
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
