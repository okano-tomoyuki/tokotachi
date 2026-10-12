#include "font_cache.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_SYNTHESIS_H
#include <hb-ft.h>
#include <hb.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef TOKOTACHI_USE_FONTCONFIG
#include <fontconfig/fontconfig.h>
#endif

namespace tokotachi
{
namespace detail
{

namespace
{

typedef std::vector<FaceSource> SourceLists[FONT_STYLE_COUNT];

void add_source(std::vector<FaceSource>& out, const std::string& path, int index, bool synthetic_bold = false,
                bool synthetic_italic = false)
{
    for (std::size_t i = 0; i < out.size(); ++i)
    {
        if (out[i].path == path && out[i].index == index && out[i].synthetic_bold == synthetic_bold &&
            out[i].synthetic_italic == synthetic_italic)
        {
            return;
        }
    }
    FaceSource s;
    s.path = path;
    s.index = index;
    s.synthetic_bold = synthetic_bold;
    s.synthetic_italic = synthetic_italic;
    out.push_back(s);
}

#ifdef TOKOTACHI_USE_FONTCONFIG
// fontconfig に sans-serif を問い合わせ、文字の範囲が広い順(FcFontSort)に並べる。
// 漢字の字形が国によって違うので、ロケールの言語を指定する(LANG=ja_JP.UTF-8 なら ja)。
// 太字・斜体は weight・slant を付けて問い合わせ、当たった書体が太字・斜体でなければ合成する(docs/adr/0045)。
void find_system_fonts(SourceLists& out)
{
    if (!FcInit())
    {
        return;
    }
    std::string base = "sans-serif";
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
        base += ":lang=" + std::string(lang, 2);
    }

    for (int style = 0; style < FONT_STYLE_COUNT; ++style)
    {
        const bool bold = (style & 1) != 0;
        const bool italic = (style & 2) != 0;
        const std::string query = base + (bold ? ":weight=bold" : "") + (italic ? ":slant=italic" : "");
        FcPattern* pat = FcNameParse(reinterpret_cast<const FcChar8*>(query.c_str()));
        FcPatternAddBool(pat, FC_SCALABLE, FcTrue);
        FcConfigSubstitute(nullptr, pat, FcMatchPattern);
        FcDefaultSubstitute(pat);
        FcResult result;
        FcFontSet* set = FcFontSort(nullptr, pat, FcTrue, nullptr, &result);
        if (set)
        {
            for (int i = 0; i < set->nfont && out[style].size() < 16; ++i)
            {
                FcChar8* file = nullptr;
                int index = 0;
                if (FcPatternGetString(set->fonts[i], FC_FILE, 0, &file) != FcResultMatch)
                {
                    continue;
                }
                FcPatternGetInteger(set->fonts[i], FC_INDEX, 0, &index);
                int weight = FC_WEIGHT_REGULAR;
                int slant = FC_SLANT_ROMAN;
                FcPatternGetInteger(set->fonts[i], FC_WEIGHT, 0, &weight);
                FcPatternGetInteger(set->fonts[i], FC_SLANT, 0, &slant);
                add_source(out[style], reinterpret_cast<const char*>(file), index, bold && weight < FC_WEIGHT_DEMIBOLD,
                           italic && slant == FC_SLANT_ROMAN);
            }
            FcFontSetDestroy(set);
        }
        FcPatternDestroy(pat);
    }
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

// 標準の書体のファイルと、同じフォルダにある太字・斜体・太字の斜体のファイル(無ければ空。合成する)。
struct StyledFiles
{
    const char* regular;
    const char* styled[FONT_STYLE_COUNT - 1];
};

const StyledFiles STYLED_FILES[] = {
    {"segoeui.ttf", {"segoeuib.ttf", "segoeuii.ttf", "segoeuiz.ttf"}},
    {"YuGothM.ttc", {"YuGothB.ttc", "", ""}},
    {"meiryo.ttc", {"meiryob.ttc", "", ""}},
    {"malgun.ttf", {"malgunbd.ttf", "", ""}},
    {"msyh.ttc", {"msyhbd.ttc", "", ""}},
};

bool same_name(const std::string& a, const char* b)
{
    if (a.size() != std::strlen(b))
    {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i])))
        {
            return false;
        }
    }
    return true;
}

// 標準の書体のファイルを、各 style に加える。太字・斜体のファイルが同じフォルダにあれば使い、無ければ合成する
// (太字の斜体は、太字のファイルを傾けるか、標準のファイルを太らせて傾ける)。
void add_styled(SourceLists& out, const std::string& path)
{
    add_source(out[0], path, 0);
    const std::size_t slash = path.find_last_of("/\\");
    const std::string dir = slash == std::string::npos ? std::string() : path.substr(0, slash + 1);
    const std::string name = slash == std::string::npos ? path : path.substr(slash + 1);
    std::string files[FONT_STYLE_COUNT];
    for (std::size_t i = 0; i < sizeof(STYLED_FILES) / sizeof(STYLED_FILES[0]); ++i)
    {
        if (same_name(name, STYLED_FILES[i].regular))
        {
            for (int s = 1; s < FONT_STYLE_COUNT; ++s)
            {
                const char* f = STYLED_FILES[i].styled[s - 1];
                if (*f && file_exists(dir + f))
                {
                    files[s] = dir + f;
                }
            }
        }
    }
    // 太字
    if (!files[1].empty())
    {
        add_source(out[1], files[1], 0);
    }
    else
    {
        add_source(out[1], path, 0, true, false);
    }
    // 斜体
    if (!files[2].empty())
    {
        add_source(out[2], files[2], 0);
    }
    else
    {
        add_source(out[2], path, 0, false, true);
    }
    // 太字の斜体
    if (!files[3].empty())
    {
        add_source(out[3], files[3], 0);
    }
    else if (!files[1].empty())
    {
        add_source(out[3], files[1], 0, false, true);
    }
    else
    {
        add_source(out[3], path, 0, true, true);
    }
}

#ifdef __EMSCRIPTEN__
// WebAssembly(デザイナーのキャンバス、docs/adr/0039): 探す手段が無いので、呼び出し側(拡張)が OS のフォントのファイルを
// 環境変数 TOKOTACHI_FONT_FILES に ; 区切りで渡す(Node.js の NODERAWFS で、そのままのパスで読める)。
// 太字・斜体は、ネイティブと同じく同じフォルダのファイルを探す。
// Linux では、拡張が fontconfig で書体ごとに探した面を TOKOTACHI_FONT_SOURCES に渡す(1 行に「書体 番号 太らせる 傾ける ファイル」を
// タブで区切る)。あればそれを使う(ネイティブの Linux と同じ面・同じ合成になる)。
void find_system_fonts(SourceLists& out)
{
    const char* sources = std::getenv("TOKOTACHI_FONT_SOURCES");
    if (sources && *sources)
    {
        std::string rest = sources;
        while (!rest.empty())
        {
            const std::size_t end = rest.find('\n');
            const std::string line = rest.substr(0, end);
            rest = end == std::string::npos ? std::string() : rest.substr(end + 1);
            std::string fields[5];
            std::size_t start = 0;
            for (int f = 0; f < 5; ++f)
            {
                const std::size_t tab = f < 4 ? line.find('\t', start) : std::string::npos;
                fields[f] = line.substr(start, tab == std::string::npos ? std::string::npos : tab - start);
                start = tab == std::string::npos ? line.size() : tab + 1;
            }
            const int style = std::atoi(fields[0].c_str());
            if (style >= 0 && style < FONT_STYLE_COUNT && !fields[4].empty() && out[style].size() < 16 &&
                file_exists(fields[4]))
            {
                add_source(out[style], fields[4], std::atoi(fields[1].c_str()), fields[2] == "1", fields[3] == "1");
            }
        }
        if (!out[0].empty())
        {
            return;
        }
    }
    const char* list = std::getenv("TOKOTACHI_FONT_FILES");
    std::string rest = list ? list : "";
    while (!rest.empty())
    {
        const std::size_t sep = rest.find(';');
        const std::string path = rest.substr(0, sep);
        if (!path.empty() && file_exists(path))
        {
            add_styled(out, path);
        }
        rest = sep == std::string::npos ? std::string() : rest.substr(sep + 1);
    }
}
#else
// Windows: Fonts フォルダの既知のファイル。欧文は Segoe UI、日本語は游ゴシック・メイリオ、記号・絵文字は Segoe UI Symbol・Emoji。
void find_system_fonts(SourceLists& out)
{
    const char* windir = std::getenv("WINDIR");
    const std::string dir = std::string(windir ? windir : "C:\\Windows") + "\\Fonts\\";
    static const char* const FILES[] = {"segoeui.ttf",  "YuGothM.ttc",  "meiryo.ttc", "msgothic.ttc",
                                        "seguisym.ttf", "seguiemj.ttf", "malgun.ttf", "msyh.ttc"};
    for (std::size_t i = 0; i < sizeof(FILES) / sizeof(FILES[0]); ++i)
    {
        const std::string path = dir + FILES[i];
        if (file_exists(path))
        {
            add_styled(out, path);
        }
    }
}
#endif
#endif

}  // namespace

// 合成する太字の太らせる量・斜体の傾き(FreeType の FT_GlyphSlot_Embolden・FT_GlyphSlot_Oblique と同じ)。
const float SYNTHETIC_BOLD = 1.0f / 24;
const float SYNTHETIC_SLANT = 0.2f;

FontInstance::FontInstance(FT_LibraryRec_* lib, const FaceSource& source, int pixel_size)
    : face_(nullptr), hb_(nullptr), ascent_(0), descent_(0), line_gap_(0), synthetic_bold_(source.synthetic_bold)
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
    if (source.synthetic_bold)
    {
        // 字の送りも太らせた分だけ広げる(FreeType で輪郭を太らせるのと合わせる)
        hb_font_set_synthetic_bold(hb_, SYNTHETIC_BOLD, SYNTHETIC_BOLD, false);
    }
    if (source.synthetic_italic)
    {
        // 輪郭を傾ける(x に y の SYNTHETIC_SLANT 倍を足す)。字の送りは変わらない
        FT_Matrix shear;
        shear.xx = 0x10000;
        shear.xy = static_cast<FT_Fixed>(SYNTHETIC_SLANT * 0x10000);
        shear.yx = 0;
        shear.yy = 0x10000;
        FT_Set_Transform(face_, &shear, nullptr);
    }
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

std::recursive_mutex& font_mutex()
{
    static std::recursive_mutex m;
    return m;
}

bool FontInstance::has_char(std::uint32_t cp) const
{
    std::lock_guard<std::recursive_mutex> lock(font_mutex());
    return hb_ && FT_Get_Char_Index(face_, cp) != 0;
}

const GlyphBitmap& FontInstance::glyph(std::uint32_t glyph_id, int subpixel)
{
    std::lock_guard<std::recursive_mutex> lock(font_mutex());
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
        (!synthetic_bold_ || (FT_GlyphSlot_Embolden(face_->glyph), true)) &&
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

namespace
{

// FreeType の輪郭(26.6 の固定小数点、y は上が正)を Path(ピクセル、y は下が正)に写す。
struct OutlineSink
{
    Path* path;
    float x, y;  // 今の点(2 次ベジェを 3 次に直すため)
};

int outline_move(const FT_Vector* to, void* user)
{
    OutlineSink* s = static_cast<OutlineSink*>(user);
    if (!s->path->empty())
    {
        s->path->close();
    }
    s->x = to->x / 64.0f;
    s->y = -to->y / 64.0f;
    s->path->move_to(s->x, s->y);
    return 0;
}

int outline_line(const FT_Vector* to, void* user)
{
    OutlineSink* s = static_cast<OutlineSink*>(user);
    s->x = to->x / 64.0f;
    s->y = -to->y / 64.0f;
    s->path->line_to(s->x, s->y);
    return 0;
}

int outline_conic(const FT_Vector* control, const FT_Vector* to, void* user)
{
    OutlineSink* s = static_cast<OutlineSink*>(user);
    const float cx = control->x / 64.0f, cy = -control->y / 64.0f;
    const float x = to->x / 64.0f, y = -to->y / 64.0f;
    s->path->cubic_to(s->x + 2.0f / 3 * (cx - s->x), s->y + 2.0f / 3 * (cy - s->y), x + 2.0f / 3 * (cx - x),
                      y + 2.0f / 3 * (cy - y), x, y);
    s->x = x;
    s->y = y;
    return 0;
}

int outline_cubic(const FT_Vector* c1, const FT_Vector* c2, const FT_Vector* to, void* user)
{
    OutlineSink* s = static_cast<OutlineSink*>(user);
    s->x = to->x / 64.0f;
    s->y = -to->y / 64.0f;
    s->path->cubic_to(c1->x / 64.0f, -c1->y / 64.0f, c2->x / 64.0f, -c2->y / 64.0f, s->x, s->y);
    return 0;
}

}  // namespace

const Path& FontInstance::outline(std::uint32_t glyph_id)
{
    std::lock_guard<std::recursive_mutex> lock(font_mutex());
    std::map<std::uint32_t, Path>::iterator it = outlines_.find(glyph_id);
    if (it != outlines_.end())
    {
        return it->second;
    }
    Path path;
    if (FT_Load_Glyph(face_, glyph_id, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP) == 0 &&
        face_->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
    {
        if (synthetic_bold_)
        {
            FT_GlyphSlot_Embolden(face_->glyph);
        }
        FT_Outline_Funcs funcs;
        funcs.move_to = &outline_move;
        funcs.line_to = &outline_line;
        funcs.conic_to = &outline_conic;
        funcs.cubic_to = &outline_cubic;
        funcs.shift = 0;
        funcs.delta = 0;
        OutlineSink sink = {&path, 0, 0};
        FT_Outline_Decompose(&face_->glyph->outline, &funcs, &sink);
        if (!path.empty())
        {
            path.close();
        }
    }
    return outlines_.insert(std::make_pair(glyph_id, path)).first->second;
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
    // TOKOTACHI_FONT_FILE でフォントのファイルを先頭に足せる(確認用。太字・斜体は合成する)。
    const char* extra = std::getenv("TOKOTACHI_FONT_FILE");
    if (extra && *extra)
    {
        for (int style = 0; style < FONT_STYLE_COUNT; ++style)
        {
            add_source(sources_[style], extra, 0, (style & 1) != 0, (style & 2) != 0);
        }
    }
    SourceLists system;
    find_system_fonts(system);
    for (int style = 0; style < FONT_STYLE_COUNT; ++style)
    {
        for (std::size_t i = 0; i < system[style].size(); ++i)
        {
            const FaceSource& f = system[style][i];
            add_source(sources_[style], f.path, f.index, f.synthetic_bold, f.synthetic_italic);
        }
    }
    if (sources_[0].empty())
    {
        std::fprintf(stderr, "tokotachi: no font found\n");
    }
}

FontCache::~FontCache()
{
    for (std::map<std::pair<std::pair<int, std::size_t>, int>, FontInstance*>::iterator it = instances_.begin();
         it != instances_.end(); ++it)
    {
        delete it->second;
    }
    if (lib_)
    {
        FT_Done_FreeType(lib_);
    }
}

FontInstance* FontCache::get(std::size_t face, int pixel_size, int style)
{
    std::lock_guard<std::recursive_mutex> lock(font_mutex());
    if (style < 0 || style >= FONT_STYLE_COUNT)
    {
        style = 0;
    }
    if (!lib_ || face >= sources_[style].size())
    {
        return nullptr;
    }
    if (pixel_size < 1)
    {
        pixel_size = 1;
    }
    const std::pair<std::pair<int, std::size_t>, int> key(std::make_pair(style, face), pixel_size);
    std::map<std::pair<std::pair<int, std::size_t>, int>, FontInstance*>::iterator it = instances_.find(key);
    if (it == instances_.end())
    {
        it = instances_.insert(std::make_pair(key, new FontInstance(lib_, sources_[style][face], pixel_size))).first;
    }
    return it->second->valid() ? it->second : nullptr;
}

}  // namespace detail
}  // namespace tokotachi
