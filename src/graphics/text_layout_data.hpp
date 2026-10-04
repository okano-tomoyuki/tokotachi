// TextLayout の中身(内部)。位置はピクセル。
#ifndef GUI_SRC_TEXT_TEXT_LAYOUT_DATA_HPP
#define GUI_SRC_TEXT_TEXT_LAYOUT_DATA_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <gui/graphics/text_layout.hpp>

namespace gui
{
namespace detail
{

class FontInstance;

struct PlacedGlyph
{
    std::uint32_t id;
    float x, y;  // ピクセル。行の左端・ベースラインからの位置。y は下が正
};

// 同じフォントで描くグリフの並び(見た目の順)。
struct GlyphRun
{
    FontInstance* font;
    std::vector<PlacedGlyph> glyphs;
};

// 論理の位置と、その位置までの幅(折り返しの判定に使う。向きによらず論理の順に足した幅)。
struct CaretStop
{
    std::size_t offset;
    float x;
};

// 1 つの書記素の見た目の範囲(docs/adr/0020)。
struct GraphemeBox
{
    std::size_t start, end;  // 論理の範囲
    float x0, x1;            // 見た目の左端・右端(ピクセル。行の左端から)
    bool rtl;                // 右から左へ書く文字か
};

struct LineData
{
    std::size_t start, end;  // 行の文字の範囲(end は改行文字の前)
    std::size_t next;        // 次の行の始め(改行文字の後ろ)
    float x, y;              // 行の左端・上端(ピクセル。レイアウトの左上から)
    float width;             // 行末の空白を除いた幅
    bool rtl;                // 段落の基本の向きが右から左か
    std::vector<GlyphRun> runs;
    std::vector<GraphemeBox> graphemes;  // 論理の順
};

struct TextLayoutData
{
    std::string text;
    Font font;
    float scale;
    float width;                 // 最も広い行の幅
    float ascent, descent, gap;  // ピクセル。gap は行の間の空き
    std::vector<LineData> lines;
    std::vector<char> word_breaks;  // UAX #29。word_breaks[i] はバイト i の後ろで単語が分かれるか
};

}  // namespace detail
}  // namespace gui

#endif
