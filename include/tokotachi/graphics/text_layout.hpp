/// @file
/// テキストのレイアウト(シェーピング済みのグリフの行の並び)。docs/adr/0007、0019、0020。
/// 位置は dp(レイアウトの左上が原点)、テキストの位置は UTF-8 のバイトの位置で表す。
/// 改行文字(`\n`・`\r\n` など)は常に行を分け、wrap_width が正ならその幅で折り返す(UAX #14。日本語の禁則を含む)。
/// 双方向テキスト(UAX #9): 段落の基本の向きは最初の向きの強い文字で決め、行の中は見た目の順に並べる。
/// キャレットの位置・行の問い合わせは論理の位置(バイトの位置)で受け、見た目の位置を返す。
#ifndef TOKOTACHI_GRAPHICS_TEXT_LAYOUT_HPP
#define TOKOTACHI_GRAPHICS_TEXT_LAYOUT_HPP

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "../layout/alignment.hpp"
#include "../core/geometry.hpp"

namespace tokotachi
{

/// 文字の大きさと太さ・傾き(docs/adr/0045)。
/// 太字・斜体は、OS にその書体のファイル(Segoe UI Bold・游ゴシック Bold 等)があれば使い、無ければ標準の書体を太らせる・傾ける。
/// @code
/// label_title->font = tokotachi::Font(18, true);  // 18dp の太字
/// @endcode
/// @ingroup graphics
struct Font
{
    float size;  ///< dp
    bool bold;
    bool italic;

    Font() : size(14), bold(false), italic(false) {}
    explicit Font(float size_dp, bool bold_ = false, bool italic_ = false)
        : size(size_dp), bold(bold_), italic(italic_)
    {
    }

    bool operator==(const Font& o) const { return size == o.size && bold == o.bold && italic == o.italic; }
    bool operator!=(const Font& o) const { return !(*this == o); }
};

namespace detail
{
struct TextLayoutData;
}

/// 作った後は変更しない値として扱う(コピーは安い)。テキスト・フォント・倍率・折り返しの幅が変わったら作り直す。
/// @ingroup graphics
class TextLayout
{
public:
    TextLayout();
    /// scale は dp からピクセルへの倍率(Widget::display_scale)。
    /// wrap_width(dp)が正ならその幅で折り返す。alignment は各行の横のそろえ方(折り返すときは折り返しの幅、
    /// 折り返さないときは最も広い行の幅に対して。FILL は START と同じ)。
    /// START・END は段落の向きの始め・終わり(右から左の段落では START が右)。
    TextLayout(const std::string& text, const Font& font, float scale, float wrap_width = 0,
               Align alignment = Align::START);

    bool is_null() const { return !d_; }
    const std::string& text() const;
    const Font& font() const;
    float scale() const;

    /// 最も広い行の幅(行末の空白は含めない)と、全体の高さ。
    float width() const;
    float height() const;
    /// 1 行目のベースラインから上・下の高さ(どちらも正の値)。行の高さはフォールバックしたフォントによらず一定。
    float ascent() const;
    float descent() const;
    /// 行の送り(ある行の上端から次の行の上端まで)。
    float line_height() const;

    std::size_t line_count() const;
    /// offset のある行。折り返した行の境目は次の行に、改行文字の直前はその行に属する。
    std::size_t line_of(std::size_t offset) const;
    /// 行の始めと終わり(終わりは改行文字の前)。
    std::size_t line_start(std::size_t line) const;
    std::size_t line_end(std::size_t line) const;
    /// 行の矩形(左右は行の文字の範囲、上下は行の送りの範囲)。
    Rect line_rect(std::size_t line) const;
    /// 行が右から左へ書く段落にあるか。
    bool line_is_rtl(std::size_t line) const;

    /// キャレットの左上(高さは ascent() + descent())。offset から始まる文字の先頭の側(左から右の文字なら左端、
    /// 右から左の文字なら右端)に置く。行の終わりでは、最後の文字の末尾の側。
    Point caret_position(std::size_t offset) const;
    /// [start, end) を選んだときの見た目の矩形(双方向の行では、1 行でも離れた複数の矩形になる)。
    std::vector<Rect> selection_rects(std::size_t start, std::size_t end) const;
    /// p に最も近い、キャレットを置ける位置。
    std::size_t offset_at(const Point& p) const;
    /// 次・前のキャレットを置ける位置(書記素の境界。論理の順で、行の境目もまたぐ)。
    std::size_t next_offset(std::size_t offset) const;
    std::size_t previous_offset(std::size_t offset) const;

    /// 次の単語の始め・前の単語の始め(UAX #29。空白は飛ばす。Ctrl+矢印に使う)。
    std::size_t next_word_offset(std::size_t offset) const;
    std::size_t previous_word_offset(std::size_t offset) const;
    /// offset を含む単語の範囲(ダブルクリックでの選択に使う)。
    void word_at(std::size_t offset, std::size_t& start, std::size_t& end) const;

    const detail::TextLayoutData* data() const { return d_.get(); }

private:
    std::shared_ptr<const detail::TextLayoutData> d_;
};

}  // namespace tokotachi

#endif
