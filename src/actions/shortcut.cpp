#include <tokotachi/actions/shortcut.hpp>

#include <cctype>
#include <cstdlib>

namespace tokotachi
{

namespace
{

struct KeyName
{
    Key key;
    const char* name;  // 表示の名前(最初のもの)と、読める名前
};

const KeyName KEY_NAMES[] = {
    {Key::DEL, "Delete"},       {Key::DEL, "Del"},          {Key::INSERT, "Insert"}, {Key::INSERT, "Ins"},
    {Key::BACKSPACE, "Backspace"}, {Key::TAB, "Tab"},       {Key::RETURN, "Enter"},  {Key::RETURN, "Return"},
    {Key::ESCAPE, "Escape"},    {Key::ESCAPE, "Esc"},       {Key::SPACE, "Space"},   {Key::LEFT, "Left"},
    {Key::RIGHT, "Right"},      {Key::UP, "Up"},            {Key::DOWN, "Down"},     {Key::HOME, "Home"},
    {Key::END, "End"},          {Key::PAGE_UP, "PageUp"},   {Key::PAGE_DOWN, "PageDown"},
};

std::string lower(const std::string& s)
{
    std::string out = s;
    for (std::size_t i = 0; i < out.size(); ++i)
    {
        out[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(out[i])));
    }
    return out;
}

Key key_from_name(const std::string& name)
{
    const std::string n = lower(name);
    if (n.size() == 1 && n[0] >= 'a' && n[0] <= 'z')
    {
        return static_cast<Key>(static_cast<int>(Key::A) + (n[0] - 'a'));
    }
    if (n.size() == 1 && n[0] >= '0' && n[0] <= '9')
    {
        return static_cast<Key>(static_cast<int>(Key::DIGIT_0) + (n[0] - '0'));
    }
    if (n.size() >= 2 && n[0] == 'f')
    {
        const int number = std::atoi(n.c_str() + 1);
        if (number >= 1 && number <= 12 && n.find_first_not_of("0123456789", 1) == std::string::npos)
        {
            return static_cast<Key>(static_cast<int>(Key::F1) + number - 1);
        }
    }
    for (std::size_t i = 0; i < sizeof(KEY_NAMES) / sizeof(KEY_NAMES[0]); ++i)
    {
        if (n == lower(KEY_NAMES[i].name))
        {
            return KEY_NAMES[i].key;
        }
    }
    return Key::UNKNOWN;
}

std::string key_name(Key key)
{
    const int k = static_cast<int>(key);
    if (k >= static_cast<int>(Key::A) && k <= static_cast<int>(Key::Z))
    {
        return std::string(1, static_cast<char>('A' + (k - static_cast<int>(Key::A))));
    }
    if (k >= static_cast<int>(Key::DIGIT_0) && k <= static_cast<int>(Key::DIGIT_9))
    {
        return std::string(1, static_cast<char>('0' + (k - static_cast<int>(Key::DIGIT_0))));
    }
    if (k >= static_cast<int>(Key::F1) && k <= static_cast<int>(Key::F12))
    {
        return "F" + std::to_string(k - static_cast<int>(Key::F1) + 1);
    }
    for (std::size_t i = 0; i < sizeof(KEY_NAMES) / sizeof(KEY_NAMES[0]); ++i)
    {
        if (KEY_NAMES[i].key == key)
        {
            return KEY_NAMES[i].name;
        }
    }
    return std::string();
}

const Modifiers MODIFIER_MASK = MODIFIER_SHIFT | MODIFIER_CONTROL | MODIFIER_ALT | MODIFIER_META;

}  // namespace

Shortcut::Shortcut(const char* text) : key(Key::UNKNOWN), modifiers(MODIFIER_NONE)
{
    *this = Shortcut(std::string(text ? text : ""));
}

Shortcut::Shortcut(const std::string& text) : key(Key::UNKNOWN), modifiers(MODIFIER_NONE)
{
    Modifiers mods = MODIFIER_NONE;
    std::size_t start = 0;
    while (start <= text.size())
    {
        std::size_t plus = text.find('+', start);
        if (plus == std::string::npos)
        {
            plus = text.size();
        }
        const std::string part = lower(text.substr(start, plus - start));
        const bool last = plus == text.size();
        if (!last)
        {
            if (part == "ctrl" || part == "control")
            {
                mods |= MODIFIER_CONTROL;
            }
            else if (part == "shift")
            {
                mods |= MODIFIER_SHIFT;
            }
            else if (part == "alt")
            {
                mods |= MODIFIER_ALT;
            }
            else if (part == "meta" || part == "super" || part == "win")
            {
                mods |= MODIFIER_META;
            }
            else
            {
                return;  // 読めない修飾キー
            }
        }
        else
        {
            const Key k = key_from_name(part);
            if (k != Key::UNKNOWN)
            {
                key = k;
                modifiers = mods;
            }
            return;
        }
        start = plus + 1;
    }
}

std::string Shortcut::to_string() const
{
    if (is_empty())
    {
        return std::string();
    }
    std::string s;
    if (modifiers & MODIFIER_CONTROL)
    {
        s += "Ctrl+";
    }
    if (modifiers & MODIFIER_ALT)
    {
        s += "Alt+";
    }
    if (modifiers & MODIFIER_SHIFT)
    {
        s += "Shift+";
    }
    if (modifiers & MODIFIER_META)
    {
        s += "Meta+";
    }
    return s + key_name(key);
}

bool Shortcut::matches(const KeyEvent& e) const
{
    return !is_empty() && e.key == key && (e.modifiers & MODIFIER_MASK) == (modifiers & MODIFIER_MASK);
}

}  // namespace tokotachi
