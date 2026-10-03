// マイルストーン1の検証プログラム。
//
// 確認すること:
//   - Linux では Wayland(無ければ X11)で、Windows では Win32 でウィンドウを出せる
//   - plutovg(CPU のラスタライザ)でアンチエイリアス付きの図形を描き、SDL_Renderer のテクスチャで表示できる
//   - FreeType + HarfBuzz で日本語を描ける(カーニング・合字を含む)
//   - IME の変換中の文字列(プリエディット)を自前で描き、確定した文字列を受け取れる
//   - 分数スケーリング(125%、150% など)で、描画・マウス座標・IME の候補窓の位置がずれない
//   - Windows のウィンドウのリサイズ中(モーダルループ中)も再描画が止まらない
//
// 座標系は3つある:
//   dp     : UI の設計単位。100% のときの 1px。
//   window : SDL のウィンドウ座標(マウスのイベント、SDL_SetTextInputArea)。
//            Windows では物理ピクセル、Wayland では論理ピクセル。
//   pixel  : 描画バッファのピクセル。
//   pixel  = dp * displayScale
//   window = pixel / pixelDensity

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <hb.h>
#include <plutovg.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "font.hpp"

namespace {

const float kWindowWidthDp = 640;
const float kWindowHeightDp = 460;

struct Color {
    uint8_t r, g, b, a;
};

Color rgb(uint32_t v, uint8_t a = 255)
{
    Color c = {static_cast<uint8_t>(v >> 16), static_cast<uint8_t>(v >> 8), static_cast<uint8_t>(v), a};
    return c;
}

struct RectF {
    float x, y, w, h;
    bool contains(float px, float py) const { return px >= x && py >= y && px < x + w && py < y + h; }
};

RectF rectF(float x, float y, float w, float h)
{
    RectF r = {x, y, w, h};
    return r;
}

// UTF-8 の直前の文字の先頭の位置。
size_t utf8Prev(const std::string& s, size_t pos)
{
    if (pos == 0) return 0;
    --pos;
    while (pos > 0 && (static_cast<unsigned char>(s[pos]) & 0xC0) == 0x80) --pos;
    return pos;
}

// 先頭から n 文字目(コードポイント単位)のバイト位置。
size_t utf8Offset(const std::string& s, int n)
{
    size_t pos = 0;
    while (n > 0 && pos < s.size()) {
        ++pos;
        while (pos < s.size() && (static_cast<unsigned char>(s[pos]) & 0xC0) == 0x80) ++pos;
        --n;
    }
    return pos;
}

class App {
public:
    App()
        : window_(nullptr), renderer_(nullptr), texture_(nullptr), surface_(nullptr), canvas_(nullptr),
          arrowCursor_(nullptr), textCursor_(nullptr),
          scale_(1), density_(1), pixelW_(0), pixelH_(0),
          hoverButton_(false), pressButton_(false), clicks_(0), editFocused_(false),
          preStart_(-1), preLength_(-1), lastFrameMs_(0), frames_(0), dirty_(true), running_(true)
    {
        button_ = rectF(24, 132, 160, 40);
        edit_ = rectF(24, 216, 592, 40);
    }

    bool init(int argc, char** argv);
    void run();
    bool saveScreenshot(const char* path);
    void shutdown();

private:
    static bool SDLCALL eventWatch(void* userdata, SDL_Event* e);

    void updateScale();
    void ensureTarget();
    void handle(const SDL_Event& e);
    void setEditFocus(bool focused);
    void render();

    float px(float dp) const { return dp * scale_; }
    float snap(float dp) const { return std::floor(dp * scale_ + 0.5f); }
    float textBaseline(Font& f, const RectF& r) const;

    void fillRect(const RectF& r, float radius, Color fill);
    void drawFrame(const RectF& r, float radius, Color fill, Color stroke, float strokeDp);
    void drawDemoShapes(float top);

    SDL_Window* window_;
    SDL_Renderer* renderer_;
    SDL_Texture* texture_;
    plutovg_surface_t* surface_;  // buffer_ を描画先にする
    plutovg_canvas_t* canvas_;
    SDL_Cursor* arrowCursor_;
    SDL_Cursor* textCursor_;
    std::vector<uint32_t> buffer_;

    Font title_, body_, small_;

    float scale_;    // displayScale: dp -> pixel
    float density_;  // pixelDensity: window -> pixel
    int pixelW_, pixelH_;

    RectF button_, edit_;
    bool hoverButton_, pressButton_;
    int clicks_;
    bool editFocused_;
    std::string text_;
    std::string preedit_;
    int preStart_, preLength_;

    double lastFrameMs_;
    unsigned long frames_;
    bool dirty_;
    bool running_;
};

bool App::init(int argc, char** argv)
{
#if defined(__linux__)
    // Wayland を優先し、使えなければ X11 にする(環境変数 SDL_VIDEO_DRIVER の指定があればそちらを使う)。
    if (!std::getenv("SDL_VIDEO_DRIVER")) SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "wayland,x11");
#endif
    // 変換中の文字列はアプリケーションが描く(Qt と同じくインラインのプリエディット)。候補窓は OS/IME に任せる。
    SDL_SetHint(SDL_HINT_IME_IMPLEMENTED_UI, "composition");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return false;
    }

    int faceIndex = 0;
    std::string fontPath;
    for (int i = 1; i + 1 < argc; ++i)
        if (std::string(argv[i]) == "--font") fontPath = argv[i + 1];
    if (fontPath.empty()) fontPath = findJapaneseFont(&faceIndex);
    if (fontPath.empty() || !title_.open(fontPath, faceIndex) || !body_.open(fontPath, faceIndex) ||
        !small_.open(fontPath, faceIndex)) {
        std::fprintf(stderr, "font not found or failed to open: '%s' (use --font <file>)\n",
                     fontPath.c_str());
        return false;
    }

    window_ = SDL_CreateWindow("spike_m1: SDL3 + plutovg + HarfBuzz",
                               static_cast<int>(kWindowWidthDp), static_cast<int>(kWindowHeightDp),
                               SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_HIDDEN);
    if (!window_) {
        std::fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        return false;
    }
    // Windows では window 座標が物理ピクセルなので、dp の大きさに合わせて広げる(Wayland では倍率は 1 になる)。
    updateScale();
    SDL_SetWindowSize(window_, static_cast<int>(kWindowWidthDp * scale_ / density_),
                      static_cast<int>(kWindowHeightDp * scale_ / density_));
    SDL_SetWindowMinimumSize(window_, static_cast<int>(320 * scale_ / density_),
                             static_cast<int>(240 * scale_ / density_));

    renderer_ = SDL_CreateRenderer(window_, nullptr);
    if (!renderer_) {
        std::fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        return false;
    }

    arrowCursor_ = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
    textCursor_ = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT);
    SDL_AddEventWatch(&App::eventWatch, this);

    int major = SDL_VERSIONNUM_MAJOR(SDL_GetVersion()), minor = SDL_VERSIONNUM_MINOR(SDL_GetVersion()),
        micro = SDL_VERSIONNUM_MICRO(SDL_GetVersion());
    std::printf("SDL %d.%d.%d, video driver: %s, renderer: %s\n", major, minor, micro,
                SDL_GetCurrentVideoDriver(), SDL_GetRendererName(renderer_));
    std::printf("plutovg %s, HarfBuzz %s\n", plutovg_version_string(), hb_version_string());
    std::printf("font: %s (index %d)\n", fontPath.c_str(), faceIndex);
    std::fflush(stdout);

    SDL_ShowWindow(window_);
    return true;
}

void App::shutdown()
{
    if (window_) SDL_StopTextInput(window_);
    SDL_RemoveEventWatch(&App::eventWatch, this);
    if (canvas_) plutovg_canvas_destroy(canvas_);
    if (surface_) plutovg_surface_destroy(surface_);
    if (arrowCursor_) SDL_DestroyCursor(arrowCursor_);
    if (textCursor_) SDL_DestroyCursor(textCursor_);
    if (texture_) SDL_DestroyTexture(texture_);
    if (renderer_) SDL_DestroyRenderer(renderer_);
    if (window_) SDL_DestroyWindow(window_);
    SDL_Quit();
}

// Windows はウィンドウの枠のドラッグ中に OS のモーダルループに入り、SDL_WaitEvent に戻ってこない。
// その間も SDL_EVENT_WINDOW_EXPOSED(data1 == 1)はメインスレッドで event watch に届くので、ここで描く。
bool SDLCALL App::eventWatch(void* userdata, SDL_Event* e)
{
    App* app = static_cast<App*>(userdata);
    if (e->type == SDL_EVENT_WINDOW_EXPOSED && e->window.data1 == 1) {
        app->updateScale();
        app->render();
    }
    return true;
}

void App::updateScale()
{
    float s = SDL_GetWindowDisplayScale(window_);
    float d = SDL_GetWindowPixelDensity(window_);
    if (s <= 0) s = 1;
    if (d <= 0) d = 1;
    scale_ = s;
    density_ = d;
    title_.setPixelSize(static_cast<int>(std::floor(22 * s + 0.5f)));
    body_.setPixelSize(static_cast<int>(std::floor(15 * s + 0.5f)));
    small_.setPixelSize(static_cast<int>(std::floor(12 * s + 0.5f)));
}

void App::ensureTarget()
{
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(window_, &w, &h);
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    if (w == pixelW_ && h == pixelH_ && texture_) return;
    pixelW_ = w;
    pixelH_ = h;
    buffer_.assign(static_cast<size_t>(w) * h, 0);
    // plutovg の画素はアルファ乗算済みの 0xAARRGGBB の uint32_t で、SDL の SDL_PIXELFORMAT_ARGB8888 と同じ並び。
    if (canvas_) plutovg_canvas_destroy(canvas_);
    if (surface_) plutovg_surface_destroy(surface_);
    surface_ = plutovg_surface_create_for_data(reinterpret_cast<unsigned char*>(buffer_.data()), w, h, w * 4);
    canvas_ = plutovg_canvas_create(surface_);
    if (texture_) SDL_DestroyTexture(texture_);
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
    SDL_SetTextureScaleMode(texture_, SDL_SCALEMODE_NEAREST);
}

void App::setEditFocus(bool focused)
{
    if (focused == editFocused_) return;
    editFocused_ = focused;
    if (focused) {
        SDL_StartTextInput(window_);
    } else {
        SDL_StopTextInput(window_);
        preedit_.clear();
    }
    dirty_ = true;
}

void App::handle(const SDL_Event& e)
{
    // マウスの座標は window 座標なので dp に直す。
    const float toDp = density_ / scale_;
    switch (e.type) {
    case SDL_EVENT_QUIT:
        running_ = false;
        break;
    case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        updateScale();
        dirty_ = true;
        break;
    case SDL_EVENT_WINDOW_EXPOSED:
    case SDL_EVENT_WINDOW_RESIZED:
        dirty_ = true;
        break;
    case SDL_EVENT_MOUSE_MOTION: {
        float x = e.motion.x * toDp, y = e.motion.y * toDp;
        bool hover = button_.contains(x, y);
        if (hover != hoverButton_) {
            hoverButton_ = hover;
            dirty_ = true;
        }
        SDL_SetCursor(edit_.contains(x, y) ? textCursor_ : arrowCursor_);
        break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (e.button.button == SDL_BUTTON_LEFT) {
            float x = e.button.x * toDp, y = e.button.y * toDp;
            pressButton_ = button_.contains(x, y);
            setEditFocus(edit_.contains(x, y));
            dirty_ = true;
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (e.button.button == SDL_BUTTON_LEFT) {
            float x = e.button.x * toDp, y = e.button.y * toDp;
            if (pressButton_ && button_.contains(x, y)) ++clicks_;
            pressButton_ = false;
            dirty_ = true;
        }
        break;
    case SDL_EVENT_TEXT_EDITING:
        preedit_ = e.edit.text ? e.edit.text : "";
        preStart_ = e.edit.start;
        preLength_ = e.edit.length;
        dirty_ = true;
        break;
    case SDL_EVENT_TEXT_INPUT:
        text_ += e.text.text;
        preedit_.clear();
        dirty_ = true;
        break;
    case SDL_EVENT_KEY_DOWN:
        // 変換中のキーは IME のもの。
        if (!editFocused_ || !preedit_.empty()) break;
        if (e.key.key == SDLK_BACKSPACE) {
            text_.erase(utf8Prev(text_, text_.size()));
            dirty_ = true;
        } else if (e.key.key == SDLK_V && (e.key.mod & SDL_KMOD_CTRL)) {
            if (char* clip = SDL_GetClipboardText()) {
                for (const char* p = clip; *p; ++p)
                    if (*p != '\r' && *p != '\n') text_ += *p;
                SDL_free(clip);
            }
            dirty_ = true;
        } else if (e.key.key == SDLK_C && (e.key.mod & SDL_KMOD_CTRL)) {
            SDL_SetClipboardText(text_.c_str());
        } else if (e.key.key == SDLK_ESCAPE) {
            setEditFocus(false);
        }
        break;
    default:
        break;
    }
}

float App::textBaseline(Font& f, const RectF& r) const
{
    // 矩形の上下中央に、ascender と descender の中間を合わせる。
    return std::floor(px(r.y + r.h / 2) + (f.ascender() + f.descender()) / 2 + 0.5f);
}

void setColor(plutovg_canvas_t* c, Color color)
{
    plutovg_canvas_set_rgba(c, color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
}

plutovg_gradient_stop_t stop(float offset, uint32_t v)
{
    plutovg_gradient_stop_t s;
    s.offset = offset;
    s.color.r = ((v >> 16) & 0xff) / 255.0f;
    s.color.g = ((v >> 8) & 0xff) / 255.0f;
    s.color.b = (v & 0xff) / 255.0f;
    s.color.a = 1;
    return s;
}

void App::fillRect(const RectF& r, float radius, Color fill)
{
    float x0 = snap(r.x), y0 = snap(r.y), x1 = snap(r.x + r.w), y1 = snap(r.y + r.h);
    plutovg_canvas_round_rect(canvas_, x0, y0, x1 - x0, y1 - y0, px(radius), px(radius));
    setColor(canvas_, fill);
    plutovg_canvas_fill(canvas_);
}

void App::drawFrame(const RectF& r, float radius, Color fill, Color stroke, float strokeDp)
{
    // 線の中心をピクセルの境界からずらして、線幅が整数ピクセルでもにじまないようにする。
    float sw = std::floor(px(strokeDp) + 0.5f);
    if (sw < 1) sw = 1;
    float x0 = snap(r.x) + sw / 2, y0 = snap(r.y) + sw / 2;
    float x1 = snap(r.x + r.w) - sw / 2, y1 = snap(r.y + r.h) - sw / 2;
    plutovg_canvas_round_rect(canvas_, x0, y0, x1 - x0, y1 - y0, px(radius), px(radius));
    setColor(canvas_, fill);
    plutovg_canvas_fill_preserve(canvas_);
    setColor(canvas_, stroke);
    plutovg_canvas_set_line_width(canvas_, sw);
    plutovg_canvas_stroke(canvas_);
}

void App::drawDemoShapes(float top)
{
    // 線形グラデーションの角丸矩形
    plutovg_gradient_stop_t ls[2] = {stop(0, 0x3b82f6), stop(1, 0xa855f7)};
    plutovg_canvas_set_linear_gradient(canvas_, px(24), px(top), px(204), px(top + 96), PLUTOVG_SPREAD_METHOD_PAD,
                                       ls, 2, nullptr);
    plutovg_canvas_round_rect(canvas_, px(24), px(top), px(180), px(96), px(16), px(16));
    plutovg_canvas_fill(canvas_);

    // 放射グラデーションの円
    float cx = px(280), cy = px(top + 48), cr = px(46);
    plutovg_gradient_stop_t rs[2] = {stop(0, 0xfde68a), stop(1, 0xf97316)};
    plutovg_canvas_set_radial_gradient(canvas_, cx - cr / 3, cy - cr / 3, cr * 1.3f, cx - cr / 3, cy - cr / 3, 0,
                                       PLUTOVG_SPREAD_METHOD_PAD, rs, 2, nullptr);
    plutovg_canvas_circle(canvas_, cx, cy, cr);
    plutovg_canvas_fill(canvas_);

    // 太さ 3dp のベジェ曲線(アンチエイリアスの確認)
    plutovg_canvas_move_to(canvas_, px(350), px(top + 90));
    plutovg_canvas_cubic_to(canvas_, px(420), px(top - 30), px(500), px(top + 140), px(610), px(top + 10));
    plutovg_canvas_set_rgba(canvas_, 0x10 / 255.0f, 0xb9 / 255.0f, 0x81 / 255.0f, 1);
    plutovg_canvas_set_line_width(canvas_, px(3));
    plutovg_canvas_set_line_cap(canvas_, PLUTOVG_LINE_CAP_ROUND);
    plutovg_canvas_stroke(canvas_);
    plutovg_canvas_set_line_cap(canvas_, PLUTOVG_LINE_CAP_BUTT);
}

void App::render()
{
    Uint64 t0 = SDL_GetPerformanceCounter();
    ensureTarget();

    const Color bg = rgb(0xf5f6f8), panel = rgb(0xffffff), border = rgb(0xc8ccd2), accent = rgb(0x2563eb);

    // --- LineEdit のレイアウト(図形とテキストの両方で使うので先に測る) ---
    const float pad = 10;
    const float innerX = edit_.x + pad, innerW = edit_.w - pad * 2;
    float textW = body_.measure(text_) / scale_;
    float preW = body_.measure(text_ + preedit_) / scale_ - textW;
    float caretX = textW + preW;  // dp、内側の左端から
    float selX0 = 0, selX1 = 0;  // プリエディットの中の選択(変換対象の文節)
    if (!preedit_.empty() && preStart_ >= 0) {
        size_t b0 = utf8Offset(preedit_, preStart_);
        size_t b1 = utf8Offset(preedit_, preStart_ + (preLength_ > 0 ? preLength_ : 0));
        selX0 = body_.measure(text_ + preedit_.substr(0, b0)) / scale_;
        selX1 = body_.measure(text_ + preedit_.substr(0, b1)) / scale_;
        if (preLength_ <= 0) caretX = selX0;
    }
    float scroll = caretX > innerW - 2 ? caretX - (innerW - 2) : 0;

    // --- 図形(plutovg) ---
    setColor(canvas_, bg);
    plutovg_canvas_fill_rect(canvas_, 0, 0, static_cast<float>(pixelW_), static_cast<float>(pixelH_));

    Color buttonFill = pressButton_ ? rgb(0x1e40af) : hoverButton_ ? rgb(0x1d4ed8) : accent;
    fillRect(button_, 6, buttonFill);

    drawFrame(edit_, 4, panel, editFocused_ ? accent : border, editFocused_ ? 2 : 1);
    const float lineY = edit_.y + edit_.h - 10;
    if (!preedit_.empty()) {
        // プリエディットの下線。変換対象の文節は太くする。
        float u0 = innerX + textW - scroll, u1 = innerX + textW + preW - scroll;
        if (u0 < innerX) u0 = innerX;
        if (u1 > innerX + innerW) u1 = innerX + innerW;
        if (u1 > u0) fillRect(rectF(u0, lineY, u1 - u0, 1), 0, rgb(0x334155));
        if (selX1 > selX0) {
            float s0 = innerX + selX0 - scroll, s1 = innerX + selX1 - scroll;
            if (s0 < innerX) s0 = innerX;
            if (s1 > innerX + innerW) s1 = innerX + innerW;
            if (s1 > s0) fillRect(rectF(s0, lineY - 1, s1 - s0, 2), 0, rgb(0x334155));
        }
    }
    if (editFocused_) fillRect(rectF(innerX + caretX - scroll, edit_.y + 9, 1, edit_.h - 18), 0, rgb(0x111827));

    drawDemoShapes(310);


    // --- テキスト(FreeType + HarfBuzz) ---
    Bitmap bmp = {buffer_.data(), pixelW_, pixelH_, pixelW_};
    ClipRect all = {0, 0, pixelW_, pixelH_};
    char line[256];

    title_.draw(bmp, all, px(24), px(44), "SDL3 + plutovg + HarfBuzz 検証", 0x111827);
    std::snprintf(line, sizeof(line), "video: %s / renderer: %s / display scale: %.2f / pixel density: %.2f",
                  SDL_GetCurrentVideoDriver(), SDL_GetRendererName(renderer_), scale_, density_);
    small_.draw(bmp, all, px(24), px(72), line, 0x475569);
    int ww = 0, wh = 0;
    SDL_GetWindowSize(window_, &ww, &wh);
    std::snprintf(line, sizeof(line), "window: %dx%d / pixels: %dx%d / dp: %.0fx%.0f / last frame: %.2f ms (#%lu)",
                  ww, wh, pixelW_, pixelH_, pixelW_ / scale_, pixelH_ / scale_, lastFrameMs_, frames_);
    small_.draw(bmp, all, px(24), px(92), line, 0x475569);

    const char* label = "クリック";
    float lw = body_.measure(label);
    body_.draw(bmp, all, std::floor(px(button_.x + button_.w / 2) - lw / 2), textBaseline(body_, button_), label,
               0xffffff);
    std::snprintf(line, sizeof(line), "クリック回数: %d", clicks_);
    body_.draw(bmp, all, px(button_.x + button_.w + 16), textBaseline(body_, button_), line, 0x111827);

    small_.draw(bmp, all, px(24), px(edit_.y - 10),
                "LineEdit: クリックでフォーカス、IME で日本語入力 (Backspace / Ctrl+C / Ctrl+V / Esc)", 0x475569);
    ClipRect inner = {static_cast<int>(snap(innerX)), static_cast<int>(snap(edit_.y)),
                      static_cast<int>(snap(innerX + innerW)), static_cast<int>(snap(edit_.y + edit_.h))};
    float base = textBaseline(body_, edit_);
    float x = px(innerX - scroll);
    x += body_.draw(bmp, inner, x, base, text_, 0x111827);
    body_.draw(bmp, inner, x, base, preedit_, 0x334155);

    body_.draw(bmp, all, px(24), px(286), "日本語: 漢字・ひらがな・カタカナ／カーニング: AVA To Ty／合字: ffi fl", 0x111827);
    small_.draw(bmp, all, px(24), px(436), "plutovg で図形、FreeType + HarfBuzz で文字を同じバッファに描画", 0x64748b);

    // --- 表示 ---
    SDL_UpdateTexture(texture_, nullptr, buffer_.data(), pixelW_ * 4);
    SDL_RenderClear(renderer_);
    SDL_RenderTexture(renderer_, texture_, nullptr, nullptr);
    SDL_RenderPresent(renderer_);

    // IME の候補窓の位置(window 座標)。キャレットの位置を伝える。
    if (editFocused_) {
        const float toWin = scale_ / density_;
        SDL_Rect area;
        area.x = static_cast<int>(edit_.x * toWin);
        area.y = static_cast<int>(edit_.y * toWin);
        area.w = static_cast<int>(edit_.w * toWin);
        area.h = static_cast<int>(edit_.h * toWin);
        SDL_SetTextInputArea(window_, &area, static_cast<int>((pad + caretX - scroll) * toWin));
    }

    lastFrameMs_ = (SDL_GetPerformanceCounter() - t0) * 1000.0 / SDL_GetPerformanceFrequency();
    ++frames_;
    dirty_ = false;
}

// 1フレーム描いて描画バッファを PNG に保存する(目視の確認、CI でのヘッドレスの確認用)。
bool App::saveScreenshot(const char* path)
{
    render();
    SDL_Surface* s = SDL_CreateSurfaceFrom(pixelW_, pixelH_, SDL_PIXELFORMAT_ARGB8888, buffer_.data(), pixelW_ * 4);
    bool ok = s && SDL_SavePNG(s, path);
    if (!ok) std::fprintf(stderr, "screenshot: %s\n", SDL_GetError());
    SDL_DestroySurface(s);
    return ok;
}

void App::run()
{
    // 待機型のループ: イベントが来るまで眠り、変化があったときだけ描く。
    while (running_) {
        SDL_Event e;
        if (!SDL_WaitEvent(&e)) break;
        handle(e);
        while (SDL_PollEvent(&e)) handle(e);
        if (dirty_ && running_) render();
    }
}

}  // namespace

int main(int argc, char** argv)
{
    App app;
    int rc = 1;
    const char* screenshot = nullptr;
    for (int i = 1; i + 1 < argc; ++i)
        if (std::string(argv[i]) == "--screenshot") screenshot = argv[i + 1];
    if (app.init(argc, argv)) {
        if (screenshot)
            rc = app.saveScreenshot(screenshot) ? 0 : 1;
        else {
            app.run();
            rc = 0;
        }
    }
    app.shutdown();
    return rc;
}
