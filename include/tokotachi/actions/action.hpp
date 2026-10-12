/// @file
/// tokotachi::Action の宣言。
#ifndef TOKOTACHI_ACTIONS_ACTION_HPP
#define TOKOTACHI_ACTIONS_ACTION_HPP

#include <string>
#include <vector>

#include "../core/export.hpp"
#include "../core/event.hpp"
#include "../graphics/image.hpp"
#include "../core/property.hpp"
#include "shortcut.hpp"

namespace tokotachi
{

class Menu;
class Widget;

/// 利用者が実行できる操作(docs/adr/0027)。メニューの項目・ショートカットのキーが同じ Action を指す。
///
/// @code
/// tokotachi::Action* action_save = new tokotachi::Action(window_main, "保存(&S)");
/// action_save->shortcut = "Ctrl+S";
/// action_save->on_trigger = [](tokotachi::Object*) { ... };
/// menu_file->add_action(action_save);   // メニューバーのメニューに入れると、Ctrl+S もウィンドウ全体で効く
/// action_save->enabled = false;         // メニューの項目もキーも無効になる
/// @endcode
///
/// ショートカットが効くのは、次のどちらかに入れた Action だけ:
/// - ウィンドウのメニューバー(menu_bar)のメニュー、または window->add_action: ウィンドウ全体で効く。
/// - ウィジェットの add_action: フォーカスがそのウィジェットの中にあるときだけ効き、ウィンドウのものより優先する。
/// フォーカスのあるウィジェットがキーを受け取ったとき(入力欄の Ctrl+C など)は、ショートカットより優先する。
/// 右クリックのメニューに入れただけの Action のショートカットは、表示だけで、キーでは実行しない。
///
/// text の「&」の次の文字はニーモニック(メニューを開いているときに押すと選ぶ。「&&」は「&」そのもの)。
///
/// @ingroup actions
class TOKOTACHI_API Action : public Object
{
public:
    explicit Action(Object* owner = nullptr, const std::string& text = std::string());
    ~Action();

    Property<std::string> text;
    Property<Shortcut> shortcut;
    /// ツールバーのボタン・メニューの項目に出す画像(メニューでは文字の前。checkable でチェックしているなら画像を枠で囲む)。
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
    template <typename>
    friend struct detail::ImplAccess;
    class Impl;
    Impl* d_;
};

}  // namespace tokotachi

#endif
