// ニーモニック(「保存(&S)」の & の次の文字。内部。docs/adr/0027)。
#ifndef GUI_SRC_WIDGETS_MNEMONIC_HPP
#define GUI_SRC_WIDGETS_MNEMONIC_HPP

#include <algorithm>
#include <cmath>
#include <string>

#include <gui/canvas.hpp>
#include <gui/event.hpp>
#include <gui/text_layout.hpp>

namespace gui
{
namespace detail
{

struct Mnemonic
{
    std::string text;    // & を除いた表示の文字(&& は & にする)
    Key key;             // ニーモニックのキー(英数字でなければ UNKNOWN)
    std::size_t offset;  // text の中の、その文字の位置(無ければ npos)
};

inline Mnemonic parse_mnemonic(const std::string& s)
{
    Mnemonic m;
    m.key = Key::UNKNOWN;
    m.offset = std::string::npos;
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] != '&' || i + 1 >= s.size())
        {
            m.text += s[i];
            continue;
        }
        ++i;
        if (s[i] == '&')
        {
            m.text += '&';
            continue;
        }
        if (m.offset == std::string::npos)
        {
            m.offset = m.text.size();
            const char c = s[i];
            if (c >= 'a' && c <= 'z')
            {
                m.key = static_cast<Key>(static_cast<int>(Key::A) + (c - 'a'));
            }
            else if (c >= 'A' && c <= 'Z')
            {
                m.key = static_cast<Key>(static_cast<int>(Key::A) + (c - 'A'));
            }
            else if (c >= '0' && c <= '9')
            {
                m.key = static_cast<Key>(static_cast<int>(Key::DIGIT_0) + (c - '0'));
            }
        }
        m.text += s[i];
    }
    return m;
}

// pos に描いた layout の、offset の文字の下に線を引く。
inline void paint_mnemonic_underline(Canvas& canvas, const TextLayout& layout, const Point& pos, std::size_t offset,
                                     const Color& color)
{
    if (offset == std::string::npos || layout.is_null() || offset >= layout.text().size())
    {
        return;
    }
    const float x0 = layout.caret_position(offset).x;
    const float x1 = layout.caret_position(layout.next_offset(offset)).x;
    const float y = pos.y + layout.ascent() + 2;
    canvas.fill_rect(Rect(pos.x + std::min(x0, x1), y, std::fabs(x1 - x0), 1), color);
}

}  // namespace detail
}  // namespace gui

#endif
