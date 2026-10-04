// フォントの探索・フォールバックの順序・大きさごとのフォントの実体とグリフのキャッシュ(内部。docs/adr/0007)。
#ifndef GUI_SRC_TEXT_FONT_CACHE_HPP
#define GUI_SRC_TEXT_FONT_CACHE_HPP

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <gui/canvas.hpp>

struct FT_LibraryRec_;
struct FT_FaceRec_;
struct hb_font_t;

namespace gui
{
namespace detail
{

struct FaceSource
{
    std::string path;
    int index;
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

    // フォールバックの順に並んだフォントの数。先頭が主のフォント(行の高さを決める)。
    std::size_t face_count() const { return sources_.size(); }
    // face 番目のフォントを pixel_size で開いたもの(開けなければ nullptr)。
    FontInstance* get(std::size_t face, int pixel_size);

private:
    FontCache();
    ~FontCache();

    FT_LibraryRec_* lib_;
    std::vector<FaceSource> sources_;
    std::map<std::pair<std::size_t, int>, FontInstance*> instances_;
};

}  // namespace detail
}  // namespace gui

#endif
