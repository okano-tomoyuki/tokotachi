// キーの組み合わせ(ショートカット。docs/adr/0027)。
//
//   action->shortcut = "Ctrl+S";                                        // 文字から作る
//   action->shortcut = gui::Shortcut(gui::Key::F5);
//   action->shortcut = gui::Shortcut(gui::Key::Z, gui::MODIFIER_CONTROL | gui::MODIFIER_SHIFT);
//
// 文字の書き方: 修飾キー(Ctrl・Shift・Alt・Meta)と 1 つのキーを + でつなぐ。大文字・小文字は問わない。
// キーの名前は A〜Z・0〜9・F1〜F12・Delete・Insert・Backspace・Tab・Enter・Escape・Space・Left・Right・Up・Down・
// Home・End・PageUp・PageDown。読めない文字なら空のショートカットになる。
#ifndef GUI_SHORTCUT_HPP
#define GUI_SHORTCUT_HPP

#include <string>

#include "event.hpp"

namespace gui
{

struct Shortcut
{
    Key key;
    Modifiers modifiers;

    Shortcut() : key(Key::UNKNOWN), modifiers(MODIFIER_NONE) {}
    Shortcut(Key key_, Modifiers modifiers_ = MODIFIER_NONE) : key(key_), modifiers(modifiers_) {}
    // 文字から作る("Ctrl+S" など)。
    Shortcut(const char* text);
    Shortcut(const std::string& text);

    bool is_empty() const { return key == Key::UNKNOWN; }
    // 表示の文字("Ctrl+Shift+S")。空なら空の文字。
    std::string to_string() const;
    // キーの押下がこの組み合わせか(修飾キーは過不足なく一致すること)。
    bool matches(const KeyEvent& e) const;

    bool operator==(const Shortcut& o) const { return key == o.key && modifiers == o.modifiers; }
    bool operator!=(const Shortcut& o) const { return !(*this == o); }
};

}  // namespace gui

#endif
