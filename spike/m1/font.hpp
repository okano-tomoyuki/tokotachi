// FreeType + HarfBuzz による最小限のテキスト描画(検証用)。
// - シェーピングは HarfBuzz、ラスタライズは FreeType(グレースケール)。
// - フォントのフォールバック、双方向テキスト、サブピクセル位置決めは扱わない。
#ifndef SPIKE_FONT_HPP
#define SPIKE_FONT_HPP

#include <cstdint>
#include <map>
#include <string>
#include <vector>

struct FT_LibraryRec_;
struct FT_FaceRec_;
struct hb_font_t;
struct hb_buffer_t;

// ThorVG と共有する描画先(ARGB8888、アルファ乗算済み)。
struct Bitmap {
    uint32_t* pixels;
    int width;
    int height;
    int stride;  // 1行の画素数
};

struct ClipRect {
    int x0, y0, x1, y1;  // [x0, x1) x [y0, y1)、ピクセル
};

class Font {
public:
    Font();
    ~Font();

    bool open(const std::string& path, int faceIndex);
    void setPixelSize(int px);
    int pixelSize() const { return px_; }

    // ピクセル単位。descender は負の値。
    float ascender() const;
    float descender() const;

    // テキストの送り幅(ピクセル)。
    float measure(const std::string& utf8);
    // baseline の位置に描き、送り幅(ピクセル)を返す。
    float draw(Bitmap& dst, const ClipRect& clip, float x, float baseline,
               const std::string& utf8, uint32_t rgb);

private:
    struct Glyph {
        int left, top, width, height;
        std::vector<uint8_t> coverage;
    };
    struct Placed {
        unsigned gid;
        float x, y;
    };

    float shape(const std::string& utf8, std::vector<Placed>& out);
    const Glyph& glyph(unsigned gid);

    Font(const Font&);
    Font& operator=(const Font&);

    FT_LibraryRec_* lib_;
    FT_FaceRec_* face_;
    hb_font_t* hb_;
    hb_buffer_t* buf_;
    int px_;
    std::map<unsigned, Glyph> cache_;
    std::vector<Placed> placed_;
};

// 日本語を表示できるシステムのフォントを探す。見つからなければ空文字列。
std::string findJapaneseFont(int* faceIndex);

#endif
