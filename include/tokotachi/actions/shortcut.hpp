/// @file
/// tokotachi::Shortcut の宣言。
#ifndef TOKOTACHI_ACTIONS_SHORTCUT_HPP
#define TOKOTACHI_ACTIONS_SHORTCUT_HPP

#include <string>

#include "../core/event.hpp"
#include "../core/export.hpp"

namespace tokotachi
{

/// キーの組み合わせ(ショートカット。docs/adr/0027)。
///
/// @code
/// action_save->shortcut = "Ctrl+S";                                        // 文字から作る
/// action_save->shortcut = tokotachi::Shortcut(tokotachi::Key::F5);
/// action_save->shortcut = tokotachi::Shortcut(tokotachi::Key::Z, tokotachi::MODIFIER_CONTROL | tokotachi::MODIFIER_SHIFT);
/// @endcode
///
/// 文字の書き方: 修飾キー(Ctrl・Shift・Alt・Meta)と 1 つのキーを + でつなぐ。大文字・小文字は問わない。
/// キーの名前は A〜Z・0〜9・F1〜F12・Delete・Insert・Backspace・Tab・Enter・Escape・Space・Left・Right・Up・Down・
/// Home・End・PageUp・PageDown。読めない文字なら空のショートカットになる。
///
/// @ingroup actions
struct TOKOTACHI_API Shortcut
{
    Key key;
    Modifiers modifiers;

    Shortcut() : key(Key::UNKNOWN), modifiers(MODIFIER_NONE) {}
    Shortcut(Key key_, Modifiers modifiers_ = MODIFIER_NONE) : key(key_), modifiers(modifiers_) {}
    /// 文字から作る("Ctrl+S" など)。
    Shortcut(const char* text);
    Shortcut(const std::string& text);

    bool is_empty() const { return key == Key::UNKNOWN; }
    /// 表示の文字("Ctrl+Shift+S")。空なら空の文字。
    std::string to_string() const;
    /// キーの押下がこの組み合わせか(修飾キーは過不足なく一致すること)。
    bool matches(const KeyEvent& e) const;

    bool operator==(const Shortcut& o) const { return key == o.key && modifiers == o.modifiers; }
    bool operator!=(const Shortcut& o) const { return !(*this == o); }
};

}  // namespace tokotachi

#endif
