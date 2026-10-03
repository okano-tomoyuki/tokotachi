#include "font_cache.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include <hb-ft.h>
#include <hb.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef GUI_USE_FONTCONFIG
#include <fontconfig/fontconfig.h>
#endif

namespace gui
{
namespace detail
{

namespace
{

void add_source(std::vector<FaceSource>& out, const std::string& path, int index)
{
    for (std::size_t i = 0; i < out.size(); ++i)
    {
        if (out[i].path == path && out[i].index == index)
        {
            return;
        }
    }
    FaceSource s;
    s.path = path;
    s.index = index;
    out.push_back(s);
}

#ifdef GUI_USE_FONTCONFIG
// fontconfig に sans-serif を問い合わせ、文字の範囲が広い順(FcFontSort)に並べる。
// 漢字の字形が国によって違うので、ロケールの言語を指定する(LANG=ja_JP.UTF-8 なら ja)。
std::vector<FaceSource> find_system_fonts()
{
    std::vector<FaceSource> out;
    if (!FcInit())
    {
        return out;
    }
    std::string query = "sans-serif";
    const char* lang = std::getenv("LC_ALL");
    if (!lang || !*lang)
    {
        lang = std::getenv("LC_CTYPE");
    }
    if (!lang || !*lang)
    {
        lang = std::getenv("LANG");
    }
    if (lang && std::strlen(lang) >= 2 && std::strncmp(lang, "C", 1) != 0 && std::strncmp(lang, "POSIX", 5) != 0)
    {
        query += ":lang=" + std::string(lang, 2);
    }

    FcPattern* pat = FcNameParse(reinterpret_cast<const FcChar8*>(query.c_str()));
    FcPatternAddBool(pat, FC_SCALABLE, FcTrue);
    FcConfigSubstitute(nullptr, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);
    FcResult result;
    FcFontSet* set = FcFontSort(nullptr, pat, FcTrue, nullptr, &result);
    if (set)
    {
        for (int i = 0; i < set->nfont && out.size() < 16; ++i)
        {
            FcChar8* file = nullptr;
            int index = 0;
            if (FcPatternGetString(set->fonts[i], FC_FILE, 0, &file) != FcResultMatch)
            {
                continue;
            }
            FcPatternGetInteger(set->fonts[i], FC_INDEX, 0, &index);
            add_source(out, reinterpret_cast<const char*>(file), index);
        }
        FcFontSetDestroy(set);
    }
    FcPatternDestroy(pat);
    return out;
}
#else
bool file_exists(const std::string& path)
{
    if (std::FILE* f = std::fopen(path.c_str(), "rb"))
    {
        std::fclose(f);
        return true;
    }
    return false;
}

// Windows: Fonts フォルダの既知のファイル。欧文は Segoe UI、日本語は游ゴシック・メイリオ、記号・絵文字は Segoe UI Symbol・Emoji。
std::vector<FaceSource> find_system_fonts()
{
    std::vector<FaceSource> out;
    const char* windir = std::getenv("WINDIR");
    const std::string dir = std::string(windir ? windir : "C:\\Windows") + "\\Fonts\\";
    static const char* const FILES[] = {"segoeui.ttf",  "YuGothM.ttc",  "meiryo.ttc", "msgothic.ttc",
                                        "seguisym.ttf", "seguiemj.ttf", "malgun.ttf", "msyh.ttc"};
    for (std::size_t i = 0; i < sizeof(FILES) / sizeof(FILES[0]); ++i)
    {
        const std::string path = dir + FILES[i];
        if (file_exists(path))
        {
            add_source(out, path, 0);
        }
    }
    return out;
}
#endif

}  // namespace

FontInstance::FontInstance(FT_LibraryRec_* lib, const FaceSource& source, int pixel_size)
    : face_(nullptr), hb_(nullptr), ascent_(0), descent_(0), line_gap_(0)
{
    if (FT_New_Face(lib, source.path.c_str(), source.index, &face_) != 0)
    {
        face_ = nullptr;
        return;
    }
    // 大きさを変えられないフォント(ビットマップだけのカラー絵文字など)は使わない。
    if (!FT_IS_SCALABLE(face_) || FT_Set_Pixel_Sizes(face_, 0, static_cast<FT_UInt>(pixel_size)) != 0)
    {
        return;
    }
    hb_ = hb_ft_font_create_referenced(face_);
    ascent_ = face_->size->metrics.ascender / 64.0f;
    descent_ = -face_->size->metrics.descender / 64.0f;
    line_gap_ = std::max(0.0f, face_->size->metrics.height / 64.0f - ascent_ - descent_);
}

FontInstance::~FontInstance()
{
    if (hb_)
    {
        hb_font_destroy(hb_);
    }
    if (face_)
    {
        FT_Done_Face(face_);
    }
}

bool FontInstance::has_char(std::uint32_t cp) const { return hb_ && FT_Get_Char_Index(face_, cp) != 0; }

const GlyphBitmap& FontInstance::glyph(std::uint32_t glyph_id, int subpixel)
{
    const std::uint64_t key = static_cast<std::uint64_t>(glyph_id) * SUBPIXEL_STEPS + static_cast<std::uint64_t>(subpixel);
    std::map<std::uint64_t, GlyphBitmap>::iterator it = glyphs_.find(key);
    if (it != glyphs_.end())
    {
        return it->second;
    }

    GlyphBitmap g;
    g.left = g.top = g.width = g.height = 0;
    // 横の位置を 1/SUBPIXEL_STEPS ピクセル単位でずらしてから描く(グリフの位置をピクセルに丸めると、文字の間隔がばらつくため)。
    // LIGHT のヒンティングは縦だけに効くので、横にずらしても形は崩れない。
    if (FT_Load_Glyph(face_, glyph_id, FT_LOAD_TARGET_LIGHT) == 0 &&
        (face_->glyph->format != FT_GLYPH_FORMAT_OUTLINE ||
         (FT_Outline_Translate(&face_->glyph->outline, subpixel * 64 / SUBPIXEL_STEPS, 0), true)) &&
        FT_Render_Glyph(face_->glyph, FT_RENDER_MODE_LIGHT) == 0)
    {
        const FT_GlyphSlot slot = face_->glyph;
        const FT_Bitmap& bm = slot->bitmap;
        if (bm.pixel_mode == FT_PIXEL_MODE_GRAY)
        {
            g.left = slot->bitmap_left;
            g.top = slot->bitmap_top;
            g.width = static_cast<int>(bm.width);
            g.height = static_cast<int>(bm.rows);
            g.coverage.resize(static_cast<std::size_t>(g.width) * static_cast<std::size_t>(g.height));
            for (int y = 0; y < g.height; ++y)
            {
                std::memcpy(&g.coverage[static_cast<std::size_t>(y) * static_cast<std::size_t>(g.width)],
                            bm.buffer + y * bm.pitch, static_cast<std::size_t>(g.width));
            }
        }
    }
    return glyphs_.insert(std::make_pair(key, g)).first->second;
}

FontCache& FontCache::instance()
{
    static FontCache cache;
    return cache;
}

FontCache::FontCache() : lib_(nullptr)
{
    if (FT_Init_FreeType(&lib_) != 0)
    {
        lib_ = nullptr;
    }
    // GUI_FONT_FILE でフォントのファイルを先頭に足せる(確認用)。
    const char* extra = std::getenv("GUI_FONT_FILE");
    if (extra && *extra)
    {
        add_source(sources_, extra, 0);
    }
    const std::vector<FaceSource> system = find_system_fonts();
    for (std::size_t i = 0; i < system.size(); ++i)
    {
        add_source(sources_, system[i].path, system[i].index);
    }
    if (sources_.empty())
    {
        std::fprintf(stderr, "gui: no font found\n");
    }
}

FontCache::~FontCache()
{
    for (std::map<std::pair<std::size_t, int>, FontInstance*>::iterator it = instances_.begin();
         it != instances_.end(); ++it)
    {
        delete it->second;
    }
    if (lib_)
    {
        FT_Done_FreeType(lib_);
    }
}

FontInstance* FontCache::get(std::size_t face, int pixel_size)
{
    if (!lib_ || face >= sources_.size())
    {
        return nullptr;
    }
    if (pixel_size < 1)
    {
        pixel_size = 1;
    }
    const std::pair<std::size_t, int> key(face, pixel_size);
    std::map<std::pair<std::size_t, int>, FontInstance*>::iterator it = instances_.find(key);
    if (it == instances_.end())
    {
        it = instances_.insert(std::make_pair(key, new FontInstance(lib_, sources_[face], pixel_size))).first;
    }
    return it->second->valid() ? it->second : nullptr;
}

}  // namespace detail
}  // namespace gui
