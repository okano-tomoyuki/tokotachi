#include "font.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <hb-ft.h>
#include <hb.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>

#ifdef SPIKE_USE_FONTCONFIG
#include <fontconfig/fontconfig.h>
#endif

Font::Font() : lib_(nullptr), face_(nullptr), hb_(nullptr), buf_(nullptr), px_(0) {}

Font::~Font()
{
    if (buf_) hb_buffer_destroy(buf_);
    if (hb_) hb_font_destroy(hb_);  // hb_ft_font_create_referenced が face の参照を持つ
    if (face_) FT_Done_Face(face_);
    if (lib_) FT_Done_FreeType(lib_);
}

bool Font::open(const std::string& path, int faceIndex)
{
    if (FT_Init_FreeType(&lib_) != 0) return false;
    if (FT_New_Face(lib_, path.c_str(), faceIndex, &face_) != 0) return false;
    hb_ = hb_ft_font_create_referenced(face_);
    buf_ = hb_buffer_create();
    return true;
}

void Font::setPixelSize(int px)
{
    if (px < 1) px = 1;
    if (px == px_) return;
    px_ = px;
    FT_Set_Pixel_Sizes(face_, 0, static_cast<FT_UInt>(px));
    hb_ft_font_changed(hb_);
    cache_.clear();
}

float Font::ascender() const { return face_->size->metrics.ascender / 64.0f; }
float Font::descender() const { return face_->size->metrics.descender / 64.0f; }

float Font::shape(const std::string& utf8, std::vector<Placed>& out)
{
    out.clear();
    if (utf8.empty()) return 0;
    hb_buffer_clear_contents(buf_);
    hb_buffer_add_utf8(buf_, utf8.data(), static_cast<int>(utf8.size()), 0, static_cast<int>(utf8.size()));
    hb_buffer_guess_segment_properties(buf_);
    hb_shape(hb_, buf_, nullptr, 0);

    unsigned n = 0;
    const hb_glyph_info_t* info = hb_buffer_get_glyph_infos(buf_, &n);
    const hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(buf_, &n);
    float pen = 0;
    out.reserve(n);
    for (unsigned i = 0; i < n; ++i) {
        // hb-ft のフォントの単位は 26.6 固定小数点
        Placed p;
        p.gid = info[i].codepoint;
        p.x = pen + pos[i].x_offset / 64.0f;
        p.y = -pos[i].y_offset / 64.0f;
        out.push_back(p);
        pen += pos[i].x_advance / 64.0f;
    }
    return pen;
}

const Font::Glyph& Font::glyph(unsigned gid)
{
    std::map<unsigned, Glyph>::iterator it = cache_.find(gid);
    if (it != cache_.end()) return it->second;

    Glyph g;
    g.left = g.top = g.width = g.height = 0;
    if (FT_Load_Glyph(face_, gid, FT_LOAD_RENDER | FT_LOAD_TARGET_LIGHT) == 0) {
        FT_GlyphSlot slot = face_->glyph;
        const FT_Bitmap& bm = slot->bitmap;
        if (bm.pixel_mode == FT_PIXEL_MODE_GRAY) {
            g.left = slot->bitmap_left;
            g.top = slot->bitmap_top;
            g.width = static_cast<int>(bm.width);
            g.height = static_cast<int>(bm.rows);
            g.coverage.resize(static_cast<size_t>(g.width) * g.height);
            for (int y = 0; y < g.height; ++y)
                for (int x = 0; x < g.width; ++x)
                    g.coverage[static_cast<size_t>(y) * g.width + x] = bm.buffer[y * bm.pitch + x];
        }
    }
    return cache_.insert(std::make_pair(gid, g)).first->second;
}

float Font::measure(const std::string& utf8)
{
    return shape(utf8, placed_);
}

float Font::draw(Bitmap& dst, const ClipRect& clip, float x, float baseline,
                 const std::string& utf8, uint32_t rgb)
{
    float advance = shape(utf8, placed_);
    const uint32_t sr = (rgb >> 16) & 0xff, sg = (rgb >> 8) & 0xff, sb = rgb & 0xff;
    const int cx0 = clip.x0 < 0 ? 0 : clip.x0;
    const int cy0 = clip.y0 < 0 ? 0 : clip.y0;
    const int cx1 = clip.x1 > dst.width ? dst.width : clip.x1;
    const int cy1 = clip.y1 > dst.height ? dst.height : clip.y1;

    for (size_t i = 0; i < placed_.size(); ++i) {
        const Glyph& g = glyph(placed_[i].gid);
        const int ox = static_cast<int>(std::floor(x + placed_[i].x + 0.5f)) + g.left;
        const int oy = static_cast<int>(std::floor(baseline + placed_[i].y + 0.5f)) - g.top;
        for (int gy = 0; gy < g.height; ++gy) {
            const int py = oy + gy;
            if (py < cy0 || py >= cy1) continue;
            uint32_t* row = dst.pixels + static_cast<size_t>(py) * dst.stride;
            const uint8_t* cov = &g.coverage[static_cast<size_t>(gy) * g.width];
            for (int gx = 0; gx < g.width; ++gx) {
                const int px = ox + gx;
                if (px < cx0 || px >= cx1) continue;
                const uint32_t a = cov[gx];
                if (a == 0) continue;
                // アルファ乗算済みの source-over(ガンマ補正なしの線形合成)
                const uint32_t d = row[px];
                const uint32_t ia = 255 - a;
                const uint32_t da = (d >> 24) & 0xff, dr = (d >> 16) & 0xff, dg = (d >> 8) & 0xff, db = d & 0xff;
                const uint32_t oa = a + (da * ia + 127) / 255;
                const uint32_t orr = (sr * a + dr * ia + 127) / 255;
                const uint32_t og = (sg * a + dg * ia + 127) / 255;
                const uint32_t ob = (sb * a + db * ia + 127) / 255;
                row[px] = (oa << 24) | (orr << 16) | (og << 8) | ob;
            }
        }
    }
    return advance;
}

std::string findJapaneseFont(int* faceIndex)
{
    *faceIndex = 0;
#ifdef SPIKE_USE_FONTCONFIG
    std::string result;
    if (!FcInit()) return result;
    FcPattern* pat = FcNameParse(reinterpret_cast<const FcChar8*>("sans-serif:lang=ja"));
    FcConfigSubstitute(nullptr, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);
    FcResult res;
    FcPattern* match = FcFontMatch(nullptr, pat, &res);
    if (match) {
        FcChar8* file = nullptr;
        int index = 0;
        if (FcPatternGetString(match, FC_FILE, 0, &file) == FcResultMatch) result = reinterpret_cast<const char*>(file);
        if (FcPatternGetInteger(match, FC_INDEX, 0, &index) == FcResultMatch) *faceIndex = index;
        FcPatternDestroy(match);
    }
    FcPatternDestroy(pat);
    return result;
#else
    const char* windir = std::getenv("WINDIR");
    std::string dir = std::string(windir ? windir : "C:\\Windows") + "\\Fonts\\";
    static const char* const candidates[] = {"YuGothM.ttc", "meiryo.ttc", "BIZ-UDGothicR.ttc", "msgothic.ttc"};
    for (size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); ++i) {
        std::string path = dir + candidates[i];
        if (std::FILE* f = std::fopen(path.c_str(), "rb")) {
            std::fclose(f);
            return path;
        }
    }
    return std::string();
#endif
}
