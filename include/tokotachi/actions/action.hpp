/// @file
/// 利用者が実行できる操作(docs/adr/0027)。メニューの項目・ショートカットのキーが同じ Action を指す。
///
/// @code
/// tokotachi::Action* save = new tokotachi::Action(&window, "保存(&S)");
/// save->shortcut = "Ctrl+S";
/// save->on_trigger = [](tokotachi::Object*) { ... };
/// file_menu->add_action(save);   // メニューバーのメニューに入れると、Ctrl+S もウィンドウ全体で効く
/// save->enabled = false;         // メニューの項目もキーも無効になる
/// @endcode
///
/// ショートカットが効くのは、次のどちらかに入れた Action だけ:
/// - ウィンドウのメニューバー(menu_bar)のメニュー、または window->add_action: ウィンドウ全体で効く。
/// - ウィジェットの add_action: フォーカスがそのウィジェットの中にあるときだけ効き、ウィンドウのものより優先する。
/// フォーカスのあるウィジェットがキーを受け取ったとき(入力欄の Ctrl+C など)は、ショートカットより優先する。
/// 右クリックのメニューに入れただけの Action のショートカットは、表示だけで、キーでは実行しない。
///
/// text の「&」の次の文字はニーモニック(メニューを開いているときに押すと選ぶ。「&&」は「&」そのもの)。
#ifndef TOKOTACHI_ACTIONS_ACTION_HPP
#define TOKOTACHI_ACTIONS_ACTION_HPP

#include <string>
#include <vector>

#include "../core/event.hpp"
#include "../graphics/image.hpp"
#include "../core/property.hpp"
#include "shortcut.hpp"

namespace tokotachi
{

class Menu;
class Widget;

namespace detail
{
struct ActionAccess;
}

class Action : public Object
{
public:
    explicit Action(Object* owner = nullptr, const std::string& text = std::string());
    ~Action();

    Property<std::string> text;
    Property<Shortcut> shortcut;
    /// ツールバーのボタンに出す画像(メニューには出さない)。
    Property<Image> icon;
    /// ツールバーのボタンのツールチップ(空なら、text とショートカットから作る)。
    Property<std::string> tooltip;
    /// false なら、メニューでもキーでも実行しない。
    Property<bool> enabled;
    /// checkable なら、実行するたびに checked を反転する(メニューではチェックの印を出す)。
    Property<bool> checkable;
    Property<bool> checked;

    /// 実行された(メニューで選んだ・ショートカットを押した・trigger を呼んだ)。checked は反転した後。
    Property<NotifyHandler> on_trigger;

    /// enabled なら実行する。
    void trigger();

private:
    friend struct detail::ActionAccess;

    void set_text(const std::string& v);
    void set_shortcut(const Shortcut& v);
    void set_icon(const Image& v);
    void set_tooltip(const std::string& v);
    void set_enabled(const bool& v);
    void set_checkable(const bool& v);
    void set_checked(const bool& v);
    void changed();

    std::string text_;
    Shortcut shortcut_;
    Image icon_;
    std::string tooltip_;
    bool enabled_;
    bool checkable_;
    bool checked_;
    NotifyHandler on_trigger_;
    /// この Action を入れたメニュー・ウィジェット(破棄するときに外すため)。
    std::vector<Menu*> menus_;
    std::vector<Widget*> widgets_;
};

}  // namespace tokotachi

#endif
