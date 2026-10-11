// フォントの探索・フォールバックの順序・大きさごとのフォントの実体とグリフのキャッシュ(内部。docs/adr/0007)。
#ifndef TOKOTACHI_SRC_GRAPHICS_FONT_CACHE_HPP
#define TOKOTACHI_SRC_GRAPHICS_FONT_CACHE_HPP

#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <tokotachi/graphics/canvas.hpp>

struct FT_LibraryRec_;
struct FT_FaceRec_;
struct hb_font_t;

namespace tokotachi
{
namespace detail
{

// フォントの処理(フォントの実体を開く・グリフを作る・文字を並べる)を守るロック(docs/adr/0034)。
// FreeType の Face と HarfBuzz の文字の並べ方は、同時に複数のスレッドから使えないため。作ったグリフ(GlyphBitmap・Path)は
// 消さないので、受け取った参照はロックの外で読んでよい。
std::recursive_mutex& font_mutex();

// 書体の太さ・傾き(docs/adr/0045)。標準・太字・斜体・太字の斜体の 4 つ。
const int FONT_STYLE_COUNT = 4;
inline int font_style(bool bold, bool italic) { return (bold ? 1 : 0) | (italic ? 2 : 0); }

struct FaceSource
{
    std::string path;
    int index;
    // 太字・斜体の書体のファイルが無いので、標準の書体を太らせる・傾ける。
    bool synthetic_bold;
    bool synthetic_italic;
};

struct GlyphBitmap
{
    int left, top;  // ペンの位置からの、ビットマップの左端・上端(上が正)
    int width, height;
    std::vector<std::uint8_t> coverage;  // 8 ビットのグレースケール
};

// 1 つのフォントのファイルを、1 つのピクセルの大きさで開いたもの。
class FontInstance
{
public:
    FontInstance(FT_LibraryRec_* lib, const FaceSource& source, int pixel_size);
    ~FontInstance();

    FontInstance(const FontInstance&) = delete;
    FontInstance& operator=(const FontInstance&) = delete;

    bool valid() const { return hb_ != nullptr; }
    bool has_char(std::uint32_t cp) const;
    hb_font_t* hb_font() const { return hb_; }
    float ascent() const { return ascent_; }    // ピクセル、正
    float descent() const { return descent_; }  // ピクセル、正
    float line_gap() const { return line_gap_; }  // ピクセル。行の送りから ascent + descent を除いた空き
    // グリフのビットマップ。subpixel(0〜SUBPIXEL_STEPS-1)は横のずれ(1/SUBPIXEL_STEPS ピクセル単位。docs/adr/0020)。
    const GlyphBitmap& glyph(std::uint32_t glyph_id, int subpixel = 0);
    // グリフの輪郭(ピクセル。ペンの位置が原点で、y は下が正。ヒンティングなし)。拡大・回転して描くときに使う。
    // 輪郭の無いグリフ(カラーの絵文字のビットマップなど)は空。
    const Path& outline(std::uint32_t glyph_id);

    static const int SUBPIXEL_STEPS = 4;

private:
    FT_FaceRec_* face_;
    hb_font_t* hb_;
    float ascent_, descent_, line_gap_;
    bool synthetic_bold_;
    std::map<std::uint64_t, GlyphBitmap> glyphs_;  // キーはグリフの番号 * SUBPIXEL_STEPS + 横のずれ
    std::map<std::uint32_t, Path> outlines_;
};

// プロセスで 1 つ。フォントのファイルは最初に使うときに探す。
class FontCache
{
public:
    static FontCache& instance();

    FontCache(const FontCache&) = delete;
    FontCache& operator=(const FontCache&) = delete;

    // style(font_style)の、フォールバックの順に並んだフォントの数。先頭が主のフォント(行の高さを決める)。
    std::size_t face_count(int style = 0) const { return sources_[style].size(); }
    // style の face 番目のフォントを pixel_size で開いたもの(開けなければ nullptr)。
    FontInstance* get(std::size_t face, int pixel_size, int style = 0);

private:
    FontCache();
    ~FontCache();

    FT_LibraryRec_* lib_;
    std::vector<FaceSource> sources_[FONT_STYLE_COUNT];
    // キーは (style, face) と大きさ
    std::map<std::pair<std::pair<int, std::size_t>, int>, FontInstance*> instances_;
};

}  // namespace detail
}  // namespace tokotachi

#endif
