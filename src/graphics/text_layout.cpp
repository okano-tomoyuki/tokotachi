#include <gui/graphics/text_layout.hpp>

#include <hb.h>

#include <SheenBidi/SheenBidi.h>
#include <graphemebreak.h>
#include <linebreak.h>
#include <wordbreak.h>

#include <algorithm>
#include <cmath>

#include "font_cache.hpp"
#include "text_layout_data.hpp"
#include "utf8.hpp"

namespace gui
{

namespace
{

// 直前の文字と同じフォントで描くべき文字(結合文字、ZWJ・ZWNJ、異体字セレクタ)。
bool attaches_to_previous(std::uint32_t cp)
{
    if (cp == 0x200C || cp == 0x200D)
    {
        return true;
    }
    if ((cp >= 0xFE00 && cp <= 0xFE0F) || (cp >= 0xE0100 && cp <= 0xE01EF))
    {
        return true;
    }
    const hb_unicode_general_category_t gc = hb_unicode_general_category(hb_unicode_funcs_get_default(), cp);
    return gc == HB_UNICODE_GENERAL_CATEGORY_NON_SPACING_MARK || gc == HB_UNICODE_GENERAL_CATEGORY_SPACING_MARK ||
           gc == HB_UNICODE_GENERAL_CATEGORY_ENCLOSING_MARK;
}

// フォールバックの順に、文字を持つ最初のフォント。どれも持たなければ主のフォント(豆腐を出す)。
std::size_t pick_face(detail::FontCache& cache, int px, std::uint32_t cp)
{
    for (std::size_t i = 0; i < cache.face_count(); ++i)
    {
        detail::FontInstance* f = cache.get(i, px);
        if (f && f->has_char(cp))
        {
            return i;
        }
    }
    return 0;
}

bool is_line_end_char(unsigned char c) { return c == '\n' || c == '\r'; }

bool is_space_char(unsigned char c) { return c == ' ' || c == '\t'; }

// 同じフォント・同じ埋め込みレベルの文字の連続(論理の順)。
struct Item
{
    std::size_t face, start, end;
    unsigned char level;
};

// シェーピングしたクラスタの見た目の範囲。
struct ClusterBox
{
    std::size_t start;  // クラスタの始め(論理)
    float x0, x1;
};

// レイアウトを作る間の共有の情報。
struct Builder
{
    const std::string& text;
    detail::FontCache& cache;
    int px;
    std::vector<char> graphemes;  // graphemes[i] はバイト i の後ろで書記素が分かれるか
    hb_buffer_t* buf;
    // 今の段落の双方向の情報(段落が無い・左から右だけなら levels は空)。
    SBParagraphRef paragraph;
    std::size_t paragraph_offset;
    const SBLevel* levels;

    Builder(const std::string& t, detail::FontCache& c, int p)
        : text(t), cache(c), px(p), buf(hb_buffer_create()), paragraph(nullptr), paragraph_offset(0), levels(nullptr)
    {
    }
    ~Builder() { hb_buffer_destroy(buf); }

    // offset が書記素の境界か(先頭と末尾は常に境界)。
    bool grapheme_boundary(std::size_t offset) const
    {
        return offset == 0 || offset >= text.size() || graphemes[offset - 1] == GRAPHEMEBREAK_BREAK;
    }

    unsigned char level_at(std::size_t offset) const
    {
        return levels ? static_cast<unsigned char>(levels[offset - paragraph_offset]) : 0;
    }

    // [start, end) を、フォントと埋め込みレベルが同じ連続に分ける(論理の順)。
    std::vector<Item> items(std::size_t start, std::size_t end) const
    {
        std::vector<Item> out;
        for (std::size_t pos = start; pos < end;)
        {
            const std::size_t s = pos;
            const std::uint32_t cp = detail::utf8_decode(text, pos);
            const unsigned char level = level_at(s);
            const bool attach = !out.empty() && attaches_to_previous(cp);
            const std::size_t face = attach ? out.back().face : pick_face(cache, px, cp);
            if (!out.empty() && out.back().face == face && (attach || out.back().level == level))
            {
                out.back().end = pos;
            }
            else
            {
                const Item it = {face, s, pos, level};
                out.push_back(it);
            }
        }
        return out;
    }

    // 1 つの連続をシェーピングし、pen から見た目の順にグリフを並べる。クラスタの見た目の範囲を clusters に足す。
    void shape_item(const Item& item, float& pen, detail::GlyphRun* run, std::vector<ClusterBox>& clusters)
    {
        detail::FontInstance* f = cache.get(item.face, px);
        if (!f)
        {
            return;
        }
        hb_buffer_clear_contents(buf);
        // 前後の文字も文脈として渡す(cluster はテキスト全体のバイトの位置になる)。
        hb_buffer_add_utf8(buf, text.data(), static_cast<int>(text.size()), static_cast<unsigned>(item.start),
                           static_cast<int>(item.end - item.start));
        hb_buffer_guess_segment_properties(buf);
        // 右から左なら、HarfBuzz が見た目の順(左から)にグリフを返し、括弧などを鏡像にする。
        hb_buffer_set_direction(buf, (item.level & 1) ? HB_DIRECTION_RTL : HB_DIRECTION_LTR);
        hb_shape(f->hb_font(), buf, nullptr, 0);

        unsigned n = 0;
        const hb_glyph_info_t* info = hb_buffer_get_glyph_infos(buf, &n);
        const hb_glyph_position_t* gp = hb_buffer_get_glyph_positions(buf, &n);
        if (run)
        {
            run->font = f;
        }
        for (unsigned g = 0; g < n; ++g)
        {
            // hb-ft のフォントの単位は 26.6 の固定小数点
            if (run)
            {
                detail::PlacedGlyph pg;
                pg.id = info[g].codepoint;
                pg.x = pen + gp[g].x_offset / 64.0f;
                pg.y = -gp[g].y_offset / 64.0f;
                run->glyphs.push_back(pg);
            }
            const float advance = gp[g].x_advance / 64.0f;
            if (!clusters.empty() && clusters.back().start == info[g].cluster && clusters.back().x1 == pen)
            {
                clusters.back().x1 += advance;  // 同じクラスタの続きのグリフ
            }
            else
            {
                const ClusterBox c = {info[g].cluster, pen, pen + advance};
                clusters.push_back(c);
            }
            pen += advance;
        }
    }

    // クラスタの見た目の範囲から、書記素の見た目の範囲を求める(論理の順)。1 つのクラスタ(合字など)に複数の書記素があれば、
    // クラスタの幅を等分する(右から左のクラスタは右から割り当てる)。
    void graphemes_of(std::vector<ClusterBox> clusters, std::size_t start, std::size_t end,
                      std::vector<detail::GraphemeBox>& out) const
    {
        out.clear();
        std::sort(clusters.begin(), clusters.end(),
                  [](const ClusterBox& a, const ClusterBox& b) { return a.start < b.start; });
        for (std::size_t c = 0; c < clusters.size(); ++c)
        {
            const std::size_t c0 = clusters[c].start;
            const std::size_t c1 = c + 1 < clusters.size() ? clusters[c + 1].start : end;
            if (c1 <= c0 || c0 < start)
            {
                continue;
            }
            std::vector<std::size_t> bounds;
            for (std::size_t o = c0; o < c1; ++o)
            {
                if (o == c0 || grapheme_boundary(o))
                {
                    bounds.push_back(o);
                }
            }
            const bool rtl = (level_at(c0) & 1) != 0;
            const float x0 = clusters[c].x0, x1 = clusters[c].x1;
            const std::size_t k = bounds.size();
            for (std::size_t i = 0; i < k; ++i)
            {
                detail::GraphemeBox g;
                g.start = bounds[i];
                g.end = i + 1 < k ? bounds[i + 1] : c1;
                g.rtl = rtl;
                const float a = x0 + (x1 - x0) * static_cast<float>(i) / k;
                const float b = x0 + (x1 - x0) * static_cast<float>(i + 1) / k;
                g.x0 = rtl ? x1 - (b - x0) : a;
                g.x1 = rtl ? x1 - (a - x0) : b;
                out.push_back(g);
            }
        }
    }

    // [start, end) の論理の位置ごとの、論理の順に足した幅(折り返しの判定に使う)。
    std::vector<detail::CaretStop> measure(std::size_t start, std::size_t end)
    {
        std::vector<ClusterBox> clusters;
        const std::vector<Item> list = items(start, end);
        for (std::size_t i = 0; i < list.size(); ++i)
        {
            float pen = 0;
            shape_item(list[i], pen, nullptr, clusters);
        }
        std::vector<detail::GraphemeBox> boxes;
        graphemes_of(clusters, start, end, boxes);
        std::vector<detail::CaretStop> stops;
        float x = 0;
        for (std::size_t i = 0; i < boxes.size(); ++i)
        {
            const detail::CaretStop s = {boxes[i].start, x};
            stops.push_back(s);
            x += boxes[i].x1 - boxes[i].x0;
        }
        const detail::CaretStop last = {end, x};
        stops.push_back(last);
        return stops;
    }

    // [start, end) を 1 行として、見た目の順に並べてシェーピングする。
    void layout_line(std::size_t start, std::size_t end, detail::LineData& line)
    {
        line.runs.clear();
        float pen = 0;
        std::vector<ClusterBox> clusters;
        // 見た目の順の、同じ埋め込みレベルの範囲(双方向の情報が無ければ行全体を 1 つ)。
        std::vector<std::pair<std::size_t, std::size_t> > visual;
        std::vector<bool> visual_rtl;
        SBLineRef sb_line = (paragraph && end > start) ? SBParagraphCreateLine(paragraph, start, end - start) : nullptr;
        if (sb_line)
        {
            const SBRun* runs = SBLineGetRunsPtr(sb_line);
            for (SBUInteger i = 0; i < SBLineGetRunCount(sb_line); ++i)
            {
                visual.push_back(std::make_pair(runs[i].offset, runs[i].offset + runs[i].length));
                visual_rtl.push_back((runs[i].level & 1) != 0);
            }
            SBLineRelease(sb_line);
        }
        else if (end > start)
        {
            visual.push_back(std::make_pair(start, end));
            visual_rtl.push_back(false);
        }
        for (std::size_t v = 0; v < visual.size(); ++v)
        {
            std::vector<Item> list = items(visual[v].first, visual[v].second);
            if (visual_rtl[v])
            {
                std::reverse(list.begin(), list.end());  // 右から左の範囲は、論理の順の最後の連続が左に来る
            }
            for (std::size_t i = 0; i < list.size(); ++i)
            {
                detail::GlyphRun run;
                run.font = nullptr;
                shape_item(list[i], pen, &run, clusters);
                if (run.font)
                {
                    line.runs.push_back(run);
                }
            }
        }
        graphemes_of(clusters, start, end, line.graphemes);
        // 行末の空白(論理の順の最後)は幅に含めない。
        float trailing = 0;
        for (std::size_t i = line.graphemes.size(); i-- > 0;)
        {
            const detail::GraphemeBox& g = line.graphemes[i];
            if (!is_space_char(static_cast<unsigned char>(text[g.start])))
            {
                break;
            }
            trailing += g.x1 - g.x0;
        }
        line.width = pen - trailing;
    }
};

// stops の中で offset 以下の最後の位置の x。
float x_at(const std::vector<detail::CaretStop>& stops, std::size_t offset)
{
    float x = 0;
    for (std::size_t i = 0; i < stops.size() && stops[i].offset <= offset; ++i)
    {
        x = stops[i].x;
    }
    return x;
}

// 行末の空白を除いた終わり。
std::size_t trim_end(const std::string& text, std::size_t start, std::size_t end)
{
    while (end > start && is_space_char(static_cast<unsigned char>(text[end - 1])))
    {
        --end;
    }
    return end;
}

// 段落 [ps, ce) を、幅 wrap_px に収まるように行の範囲に分ける(貪欲法)。
void wrap_paragraph(Builder& b, const std::vector<char>& line_breaks, std::size_t ps, std::size_t ce, float wrap_px,
                    std::vector<std::pair<std::size_t, std::size_t> >& ranges)
{
    const std::string& text = b.text;
    const std::vector<detail::CaretStop> stops = b.measure(ps, ce);
    std::vector<std::size_t> candidates;  // 改行してよい位置(その位置の前で分ける)
    for (std::size_t i = ps; i + 1 < ce; ++i)
    {
        if (line_breaks[i] == LINEBREAK_ALLOWBREAK)
        {
            candidates.push_back(i + 1);
        }
    }
    candidates.push_back(ce);
    std::size_t start = ps, fit = ps;
    for (std::size_t c = 0; c < candidates.size(); ++c)
    {
        const std::size_t cand = candidates[c];
        const float w = x_at(stops, trim_end(text, start, cand)) - x_at(stops, start);
        if (w <= wrap_px)
        {
            fit = cand;
            continue;
        }
        if (fit > start)
        {
            ranges.push_back(std::make_pair(start, fit));
            start = fit;
            --c;  // 同じ候補を、新しい行の始めから測り直す
            continue;
        }
        // 1 つの単語が幅に収まらない: 書記素の境界で分ける(少なくとも 1 つは入れる)。
        std::size_t cut = start;
        for (std::size_t k = 0; k < stops.size(); ++k)
        {
            const detail::CaretStop& st = stops[k];
            if (st.offset <= start || st.offset > cand)
            {
                continue;
            }
            if (cut == start || st.x - x_at(stops, start) <= wrap_px)
            {
                cut = st.offset;
            }
            else
            {
                break;
            }
        }
        if (cut >= cand)
        {
            fit = cand;
            continue;
        }
        ranges.push_back(std::make_pair(start, cut));
        start = fit = cut;
        --c;
    }
    ranges.push_back(std::make_pair(start, ce));
}

std::shared_ptr<detail::TextLayoutData> build(const std::string& text, const Font& font, float scale, float wrap_width,
                                              Align alignment)
{
    std::shared_ptr<detail::TextLayoutData> d(new detail::TextLayoutData);
    d->text = text;
    d->font = font;
    d->scale = scale;
    d->width = 0;

    detail::FontCache& cache = detail::FontCache::instance();
    const int px = std::max(1, static_cast<int>(std::floor(font.size * scale + 0.5f)));
    // 行の高さは主のフォントで決める(フォールバックしたフォントでベースラインが動かないように)。
    detail::FontInstance* primary = nullptr;
    for (std::size_t i = 0; i < cache.face_count() && !primary; ++i)
    {
        primary = cache.get(i, px);
    }
    d->ascent = primary ? primary->ascent() : px * 0.8f;
    d->descent = primary ? primary->descent() : px * 0.2f;
    d->gap = primary ? primary->line_gap() : 0;

    const utf8_t* s = reinterpret_cast<const utf8_t*>(text.data());
    std::vector<char> line_breaks(text.size());
    Builder b(text, cache, px);
    b.graphemes.resize(text.size());
    d->word_breaks.resize(text.size());
    if (!text.empty())
    {
        set_linebreaks_utf8(s, text.size(), nullptr, &line_breaks[0]);
        set_graphemebreaks_utf8(s, text.size(), nullptr, &b.graphemes[0]);
        set_wordbreaks_utf8(s, text.size(), nullptr, &d->word_breaks[0]);
    }
    const float wrap_px = wrap_width * scale;

    // 双方向の情報(UAX #9)。段落ごとに作る。
    SBCodepointSequence sequence = {SBStringEncodingUTF8, text.data(), text.size()};
    SBAlgorithmRef algorithm = text.empty() ? nullptr : SBAlgorithmCreate(&sequence);

    // 段落(改行文字で分かれた範囲)ごとに、折り返して行を作る。
    std::size_t ps = 0;
    while (true)
    {
        std::size_t ce = ps;  // 段落の文字の終わり(改行文字の前)
        while (ce < text.size() && !is_line_end_char(static_cast<unsigned char>(text[ce])))
        {
            ++ce;
        }
        std::size_t pe = ce;  // 次の段落の始め
        if (pe < text.size())
        {
            pe += (text[pe] == '\r' && pe + 1 < text.size() && text[pe + 1] == '\n') ? 2 : 1;
        }

        bool rtl = false;
        b.paragraph = (algorithm && ce > ps) ? SBAlgorithmCreateParagraph(algorithm, ps, ce - ps, SBLevelDefaultLTR)
                                             : nullptr;
        b.paragraph_offset = ps;
        b.levels = nullptr;
        if (b.paragraph)
        {
            rtl = (SBParagraphGetBaseLevel(b.paragraph) & 1) != 0;
            b.levels = SBParagraphGetLevelsPtr(b.paragraph);
        }

        std::vector<std::pair<std::size_t, std::size_t> > ranges;  // 行の [始め, 終わり)
        if (wrap_px <= 0 || ce == ps)
        {
            ranges.push_back(std::make_pair(ps, ce));
        }
        else
        {
            wrap_paragraph(b, line_breaks, ps, ce, wrap_px, ranges);
        }

        for (std::size_t r = 0; r < ranges.size(); ++r)
        {
            detail::LineData line;
            line.start = ranges[r].first;
            line.end = ranges[r].second;
            line.next = r + 1 < ranges.size() ? ranges[r + 1].first : pe;
            line.rtl = rtl;
            b.layout_line(line.start, line.end, line);
            line.x = 0;
            line.y = static_cast<float>(d->lines.size()) * (d->ascent + d->descent + d->gap);
            d->width = std::max(d->width, line.width);
            d->lines.push_back(line);
        }
        if (b.paragraph)
        {
            SBParagraphRelease(b.paragraph);
            b.paragraph = nullptr;
            b.levels = nullptr;
        }
        if (ce == text.size())
        {
            break;  // 改行文字が無ければ最後の段落
        }
        // 改行文字の後ろから次の段落(改行文字で終わるテキストは、最後に空の行を持つ)。
        ps = pe;
    }
    if (algorithm)
    {
        SBAlgorithmRelease(algorithm);
    }

    // 行のそろえ方。折り返すときは折り返しの幅に、折り返さないときは最も広い行の幅に対してそろえる。
    // START・END は段落の向きの始め・終わり。
    const float reference = wrap_px > 0 ? std::max(wrap_px, d->width) : d->width;
    for (std::size_t i = 0; i < d->lines.size(); ++i)
    {
        detail::LineData& line = d->lines[i];
        Align a = alignment == Align::FILL ? Align::START : alignment;
        if (line.rtl && a != Align::CENTER)
        {
            a = a == Align::START ? Align::END : Align::START;
        }
        if (a == Align::CENTER)
        {
            line.x = (reference - line.width) / 2;
        }
        else if (a == Align::END)
        {
            line.x = reference - line.width;
        }
    }
    return d;
}

// 行の中の、論理の位置 offset のキャレットの x(行の左端から、ピクセル)。
float caret_x(const detail::LineData& l, std::size_t offset)
{
    if (l.graphemes.empty())
    {
        return 0;
    }
    for (std::size_t i = 0; i < l.graphemes.size(); ++i)
    {
        const detail::GraphemeBox& g = l.graphemes[i];
        if (g.start <= offset && offset < g.end)
        {
            return g.rtl ? g.x1 : g.x0;  // 文字の先頭の側
        }
    }
    const detail::GraphemeBox& last = l.graphemes.back();
    return last.rtl ? last.x0 : last.x1;  // 行の終わり: 最後の文字の末尾の側
}

}  // namespace

TextLayout::TextLayout() {}

TextLayout::TextLayout(const std::string& text, const Font& font, float scale, float wrap_width, Align alignment)
{
    // 文字を並べる間は、フォントの処理を 1 つのスレッドに限る(docs/adr/0034)。
    std::lock_guard<std::recursive_mutex> lock(detail::font_mutex());
    d_ = build(text, font, scale, wrap_width, alignment);
}

const std::string& TextLayout::text() const
{
    static const std::string EMPTY;
    return d_ ? d_->text : EMPTY;
}

const Font& TextLayout::font() const
{
    static const Font NONE;
    return d_ ? d_->font : NONE;
}

float TextLayout::scale() const { return d_ ? d_->scale : 1; }

float TextLayout::width() const { return d_ ? d_->width / d_->scale : 0; }

float TextLayout::height() const
{
    if (!d_)
    {
        return 0;
    }
    const float n = static_cast<float>(d_->lines.size());
    return ((n - 1) * (d_->ascent + d_->descent + d_->gap) + d_->ascent + d_->descent) / d_->scale;
}

float TextLayout::ascent() const { return d_ ? d_->ascent / d_->scale : 0; }

float TextLayout::descent() const { return d_ ? d_->descent / d_->scale : 0; }

float TextLayout::line_height() const { return d_ ? (d_->ascent + d_->descent + d_->gap) / d_->scale : 0; }

std::size_t TextLayout::line_count() const { return d_ ? d_->lines.size() : 0; }

std::size_t TextLayout::line_of(std::size_t offset) const
{
    if (!d_)
    {
        return 0;
    }
    for (std::size_t i = 0; i + 1 < d_->lines.size(); ++i)
    {
        if (offset < d_->lines[i].next)
        {
            // 折り返した行の境目(end == next)は次の行に属する。
            if (offset == d_->lines[i].end && d_->lines[i].end == d_->lines[i].next)
            {
                continue;
            }
            return i;
        }
    }
    return d_->lines.size() - 1;
}

std::size_t TextLayout::line_start(std::size_t line) const
{
    return d_ && line < d_->lines.size() ? d_->lines[line].start : 0;
}

std::size_t TextLayout::line_end(std::size_t line) const
{
    return d_ && line < d_->lines.size() ? d_->lines[line].end : 0;
}

Rect TextLayout::line_rect(std::size_t line) const
{
    if (!d_ || line >= d_->lines.size())
    {
        return Rect();
    }
    const detail::LineData& l = d_->lines[line];
    return Rect(l.x / d_->scale, l.y / d_->scale, l.width / d_->scale, line_height());
}

bool TextLayout::line_is_rtl(std::size_t line) const { return d_ && line < d_->lines.size() && d_->lines[line].rtl; }

Point TextLayout::caret_position(std::size_t offset) const
{
    if (!d_)
    {
        return Point();
    }
    const detail::LineData& l = d_->lines[line_of(offset)];
    return Point((l.x + caret_x(l, std::max(offset, l.start))) / d_->scale, l.y / d_->scale);
}

std::vector<Rect> TextLayout::selection_rects(std::size_t start, std::size_t end) const
{
    std::vector<Rect> out;
    if (!d_ || end <= start)
    {
        return out;
    }
    const float h = line_height();
    for (std::size_t li = 0; li < d_->lines.size(); ++li)
    {
        const detail::LineData& l = d_->lines[li];
        if (l.next <= start || l.start >= end)
        {
            continue;
        }
        // 選んだ書記素の見た目の範囲を、左から並べてつながるものをまとめる。
        std::vector<std::pair<float, float> > spans;
        for (std::size_t i = 0; i < l.graphemes.size(); ++i)
        {
            const detail::GraphemeBox& g = l.graphemes[i];
            if (g.start >= start && g.end <= end)
            {
                spans.push_back(std::make_pair(g.x0, g.x1));
            }
        }
        std::sort(spans.begin(), spans.end());
        for (std::size_t i = 0; i < spans.size(); ++i)
        {
            if (!out.empty() && i > 0 && spans[i].first <= spans[i - 1].second + 0.5f &&
                out.back().y == l.y / d_->scale)
            {
                const float right = std::max(out.back().right(), (l.x + spans[i].second) / d_->scale);
                out.back().width = right - out.back().x;
            }
            else
            {
                out.push_back(Rect((l.x + spans[i].first) / d_->scale, l.y / d_->scale,
                                   (spans[i].second - spans[i].first) / d_->scale, h));
            }
        }
    }
    return out;
}

std::size_t TextLayout::offset_at(const Point& p) const
{
    if (!d_)
    {
        return 0;
    }
    const float advance = d_->ascent + d_->descent + d_->gap;
    const float py = p.y * d_->scale;
    std::size_t index = py <= 0 ? 0 : static_cast<std::size_t>(py / advance);
    index = std::min(index, d_->lines.size() - 1);
    const detail::LineData& l = d_->lines[index];
    const float px = p.x * d_->scale - l.x;
    // 書記素の左右の端のうち、最も近いもの。左端・右端がどの論理の位置に当たるかは文字の向きで決まる。
    std::size_t best = l.start;
    float best_dist = -1;
    for (std::size_t i = 0; i < l.graphemes.size(); ++i)
    {
        const detail::GraphemeBox& g = l.graphemes[i];
        const float d0 = std::fabs(g.x0 - px), d1 = std::fabs(g.x1 - px);
        if (best_dist < 0 || d0 < best_dist)
        {
            best_dist = d0;
            best = g.rtl ? g.end : g.start;
        }
        if (d1 < best_dist)
        {
            best_dist = d1;
            best = g.rtl ? g.start : g.end;
        }
    }
    return best;
}

std::size_t TextLayout::next_offset(std::size_t offset) const
{
    if (!d_)
    {
        return 0;
    }
    const std::size_t index = line_of(offset);
    const detail::LineData& l = d_->lines[index];
    for (std::size_t i = 0; i < l.graphemes.size(); ++i)
    {
        if (l.graphemes[i].start > offset)
        {
            return l.graphemes[i].start;
        }
    }
    if (offset < l.end)
    {
        return l.end;
    }
    // 行の終わりからは次の行の始めへ(改行文字を飛ばす)。
    return index + 1 < d_->lines.size() ? d_->lines[index + 1].start : d_->text.size();
}

std::size_t TextLayout::previous_offset(std::size_t offset) const
{
    if (!d_)
    {
        return 0;
    }
    const std::size_t index = line_of(offset);
    const detail::LineData& l = d_->lines[index];
    if (offset <= l.start)
    {
        // 行の始めからは前の行の終わりへ。
        return index > 0 ? d_->lines[index - 1].end : 0;
    }
    std::size_t prev = l.start;
    for (std::size_t i = 0; i < l.graphemes.size() && l.graphemes[i].start < offset; ++i)
    {
        prev = l.graphemes[i].start;
    }
    return prev;
}

namespace
{

// offset が単語の境界か(先頭と末尾は常に境界)。
bool word_boundary(const detail::TextLayoutData& d, std::size_t offset)
{
    return offset == 0 || offset >= d.text.size() || d.word_breaks[offset - 1] == WORDBREAK_BREAK;
}

bool starts_with_space(const std::string& text, std::size_t offset)
{
    if (offset >= text.size())
    {
        return false;
    }
    const unsigned char c = static_cast<unsigned char>(text[offset]);
    if (c == ' ' || c == '\t' || is_line_end_char(c))
    {
        return true;
    }
    std::size_t pos = offset;
    return detail::utf8_decode(text, pos) == 0x3000;  // 全角の空白
}

}  // namespace

std::size_t TextLayout::next_word_offset(std::size_t offset) const
{
    if (!d_)
    {
        return 0;
    }
    const std::size_t n = d_->text.size();
    std::size_t o = offset;
    // 今の単語の終わりまで進み、その後の空白を飛ばして、次の単語の始めで止まる。
    do
    {
        ++o;
    } while (o < n && !word_boundary(*d_, o));
    while (o < n && starts_with_space(d_->text, o))
    {
        do
        {
            ++o;
        } while (o < n && !word_boundary(*d_, o));
    }
    return std::min(o, n);
}

std::size_t TextLayout::previous_word_offset(std::size_t offset) const
{
    if (!d_ || offset == 0)
    {
        return 0;
    }
    std::size_t o = offset;
    // 空白を飛ばして戻り、その単語の始めで止まる。
    while (true)
    {
        do
        {
            --o;
        } while (o > 0 && !word_boundary(*d_, o));
        if (o == 0 || !starts_with_space(d_->text, o))
        {
            return o;
        }
    }
}

void TextLayout::word_at(std::size_t offset, std::size_t& start, std::size_t& end) const
{
    start = end = offset;
    if (!d_ || d_->text.empty())
    {
        return;
    }
    const std::size_t n = d_->text.size();
    if (offset >= n)
    {
        offset = n - 1;
    }
    start = offset;
    while (start > 0 && !word_boundary(*d_, start))
    {
        --start;
    }
    end = offset + 1;
    while (end < n && !word_boundary(*d_, end))
    {
        ++end;
    }
}

}  // namespace gui
