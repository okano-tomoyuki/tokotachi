// UTF-8 の処理(内部)。
#ifndef TOKOTACHI_SRC_GRAPHICS_UTF8_HPP
#define TOKOTACHI_SRC_GRAPHICS_UTF8_HPP

#include <cstddef>
#include <cstdint>
#include <string>

namespace tokotachi
{
namespace detail
{

// pos から 1 文字読み、pos を次の文字へ進める。不正な並びは U+FFFD として 1 バイト進める。
inline std::uint32_t utf8_decode(const std::string& s, std::size_t& pos)
{
    const unsigned char c = static_cast<unsigned char>(s[pos]);
    std::size_t n = 0;
    std::uint32_t cp = 0, min = 0;
    if (c < 0x80)
    {
        ++pos;
        return c;
    }
    else if ((c & 0xE0) == 0xC0)
    {
        n = 1, cp = c & 0x1F, min = 0x80;
    }
    else if ((c & 0xF0) == 0xE0)
    {
        n = 2, cp = c & 0x0F, min = 0x800;
    }
    else if ((c & 0xF8) == 0xF0)
    {
        n = 3, cp = c & 0x07, min = 0x10000;
    }
    else
    {
        ++pos;
        return 0xFFFD;
    }
    if (pos + n >= s.size())  // 続きのバイトが足りない
    {
        ++pos;
        return 0xFFFD;
    }
    for (std::size_t i = 1; i <= n; ++i)
    {
        const unsigned char cc = static_cast<unsigned char>(s[pos + i]);
        if ((cc & 0xC0) != 0x80)
        {
            ++pos;
            return 0xFFFD;
        }
        cp = (cp << 6) | (cc & 0x3F);
    }
    if (cp < min || cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF))
    {
        ++pos;
        return 0xFFFD;
    }
    pos += n + 1;
    return cp;
}

// 先頭から n 文字目(コードポイントの単位)のバイトの位置。
inline std::size_t utf8_offset_of(const std::string& s, int n)
{
    std::size_t pos = 0;
    while (n > 0 && pos < s.size())
    {
        utf8_decode(s, pos);
        --n;
    }
    return pos;
}

// 改行・タブなどの制御文字を除く(1 行の入力欄に入れるとき)。
inline std::string strip_control(const std::string& s)
{
    std::string out;
    out.reserve(s.size());
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        const unsigned char c = static_cast<unsigned char>(s[i]);
        if (c >= 0x20 && c != 0x7F)
        {
            out += s[i];
        }
    }
    return out;
}

// 複数行の入力欄に入れる文字列に直す: 改行を LF にそろえ(CR LF・CR)、タブは空白に、その他の制御文字は除く。
inline std::string normalize_multiline(const std::string& s)
{
    std::string out;
    out.reserve(s.size());
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        const unsigned char c = static_cast<unsigned char>(s[i]);
        if (c == '\r')
        {
            out += '\n';
            if (i + 1 < s.size() && s[i + 1] == '\n')
            {
                ++i;
            }
        }
        else if (c == '\n')
        {
            out += '\n';
        }
        else if (c == '\t')
        {
            out += ' ';
        }
        else if (c >= 0x20 && c != 0x7F)
        {
            out += s[i];
        }
    }
    return out;
}

}  // namespace detail
}  // namespace tokotachi

#endif
