/// @file
/// tokotachi::Canvas の宣言。
#ifndef TOKOTACHI_GRAPHICS_CANVAS_HPP
#define TOKOTACHI_GRAPHICS_CANVAS_HPP

#include <string>
#include <vector>

#include "../core/geometry.hpp"

namespace tokotachi
{

class Image;
class TextLayout;
struct Font;

/// 図形の輪郭(直線・ベジェ曲線・矩形・楕円・弧を組み合わせる)。Canvas::fill_path・stroke_path で描く。
///
/// @ingroup graphics
class Path
{
public:
    enum class Op
    {
        MOVE_TO,
        LINE_TO,
        CUBIC_TO,
        CLOSE
    };

    struct Element
    {
        Op op;
        float pts[6];
    };

    void move_to(float x, float y);
    void line_to(float x, float y);
    /// 2 次ベジェ(3 次ベジェに直して持つ)。
    void quad_to(float x1, float y1, float x2, float y2);
    void cubic_to(float x1, float y1, float x2, float y2, float x3, float y3);
    void close();
    void add_rect(const Rect& r);
    void add_round_rect(const Rect& r, float radius);
    void add_ellipse(const Point& center, float rx, float ry);
    /// 楕円の弧(角度はラジアン。0 が右、正の向きは時計回り(y が下向きのため))。new_figure なら弧の始点へ move_to する
    /// (false なら今の点から線でつなぐ)。
    void add_arc(const Point& center, float rx, float ry, float start_angle, float sweep_angle, bool new_figure = true);
    /// 折れ線(close なら閉じる)。
    void add_polyline(const std::vector<Point>& points, bool close = false);

    bool empty() const { return elements_.empty(); }
    const std::vector<Element>& elements() const { return elements_; }

private:
    std::vector<Element> elements_;
};

/// グラデーションの色の位置(offset は 0〜1)。
///
/// @ingroup graphics
struct GradientStop
{
    float offset;
    Color color;

    GradientStop() : offset(0) {}
    GradientStop(float offset_, const Color& color_) : offset(offset_), color(color_) {}
};

/// 塗り(単色・線形グラデーション・放射グラデーション)。
/// @ingroup graphics
struct Paint
{
    enum class Type
    {
        SOLID,
        LINEAR,
        RADIAL
    };

    Type type;
    Color color;
    Point p0, p1;  ///< LINEAR: 始点・終点。RADIAL: p0 が中心、p1 が焦点
    float radius;  ///< RADIAL
    std::vector<GradientStop> stops;

    Paint() : type(Type::SOLID), radius(0) {}
    /// 色から暗黙に作れるようにする(canvas.fill_round_rect(r, 4, color))。
    Paint(const Color& c) : type(Type::SOLID), color(c), radius(0) {}

    static Paint linear(const Point& from, const Point& to, const std::vector<GradientStop>& stops);
    static Paint radial(const Point& center, float radius, const Point& focal, const std::vector<GradientStop>& stops);
};

enum class LineCap
{
    BUTT,
    ROUND,
    SQUARE
};

enum class LineJoin
{
    MITER,
    ROUND,
    BEVEL
};

/// 線の形。
/// @ingroup graphics
struct StrokeStyle
{
    float width;                ///< 太さ(dp)
    LineCap cap;                ///< 端の形
    LineJoin join;              ///< 角の形
    std::vector<float> dashes;  ///< 破線の線と隙間の長さ(dp)を交互に。空なら実線
    float dash_offset;          ///< 破線の始まりのずらし(dp)

    StrokeStyle(float width_ = 1, LineCap cap_ = LineCap::BUTT, LineJoin join_ = LineJoin::MITER)
        : width(width_), cap(cap_), join(join_), dash_offset(0)
    {
    }
};

/// 描画の抽象(docs/adr/0006、0032)。
///
/// 使い方: ウィジェットの on_paint、または Widget の派生クラスの paint(Canvas&) の中で描く。
///
/// @code
/// paint_box_chart->on_paint = [](tokotachi::Object*, tokotachi::Canvas& c)
/// {
///     c.fill_rect(tokotachi::Rect(0, 0, 100, 50), tokotachi::Color::rgb(0xf3f4f6));
///     c.draw_line(tokotachi::Point(0, 0), tokotachi::Point(100, 50), 2, tokotachi::Color::rgb(0x2563eb));
///     c.draw_text("こんにちは", tokotachi::Point(8, 8), tokotachi::Color::rgb(0x111827));
/// };
/// @endcode
///
/// 決まり:
/// - Canvas は、描画の呼び出し(paint・on_paint)の間だけ有効。参照を持ち続けない。描きたいときは invalidate で描き直しを頼む。
/// - メインスレッドからだけ使う。
/// - 座標は dp(ウィジェットの左上が原点)。実装が倍率を掛けてピクセルに直す。描画の結果はウィンドウの裏のバッファーに描き、
/// @code
/// まとめて画面に出す(ダブルバッファリング。ちらつかない)。
/// @endcode
/// - 描く範囲はクリップされている。clip_bounds() の外は描いても見えないので、重い描画は範囲の外を飛ばす。
/// - 変換(translate・scale・rotate)とクリップは save・restore で戻す。変換が平行移動だけのときは、矩形の辺と文字を
/// @code
/// ピクセルに揃えてにじまないように描く(docs/adr/0008)。拡大・回転があるときは揃えず、文字は輪郭として描く。
/// @endcode
///
/// @ingroup graphics
class Canvas
{
public:
    virtual ~Canvas() {}

    /// dp からピクセルへの倍率。
    virtual float scale() const = 0;

    // ---- 状態 ----
    /// 変換とクリップを積む・戻す。
    virtual void save() = 0;
    virtual void restore() = 0;
    /// 変換: 以後の座標を、今の座標系で移動・拡大・回転(ラジアン。正の向きは時計回り)する。
    virtual void translate(float dx, float dy) = 0;
    virtual void scale_by(float sx, float sy) = 0;
    virtual void rotate(float radians) = 0;
    /// 現在のクリップと r(または path の内側)の共通部分に描画を限る。
    virtual void clip_rect(const Rect& r) = 0;
    virtual void clip_path(const Path& path) = 0;
    /// 現在のクリップを囲む矩形(dp、今の座標系)。
    virtual Rect clip_bounds() const = 0;
    /// グループの透過: begin_layer から end_layer までに描いたものを、一時的な画像に描いてから opacity(0〜1)で重ねる
    /// (重なった部分も濃くならない)。対で呼ぶ(入れ子にできる)。begin_layer は save も兼ね、end_layer で変換とクリップも戻る。
    virtual void begin_layer(float opacity) = 0;
    virtual void end_layer() = 0;

    // ---- 基本の描画 ----
    virtual void fill_rect(const Rect& r, const Color& color) = 0;
    virtual void fill_round_rect(const Rect& r, float radius, const Paint& paint) = 0;
    /// 線を r の内側に描く(線の外側の辺が r に一致する)。
    virtual void stroke_round_rect(const Rect& r, float radius, float width, const Color& color) = 0;
    virtual void fill_path(const Path& path, const Paint& paint) = 0;
    virtual void stroke_path(const Path& path, const StrokeStyle& style, const Paint& paint) = 0;
    /// pos はレイアウトの左上(ベースラインは pos.y + layout.ascent())。
    virtual void draw_text(const TextLayout& layout, const Point& pos, const Color& color) = 0;
    /// 画像を矩形 dst(dp)に合わせて伸縮して描く。opacity は 0〜1(無効な部品のアイコンを薄くするときなど)。
    /// アイコン(Image::icon)は、色を決めていなければ黒で塗る(部品の文字の色で塗るには、下の tint を渡す形を使う)。
    virtual void draw_image(const Image& image, const Rect& dst, float opacity = 1) = 0;

    // ---- 手軽な描画(上の関数で描く) ----
    void stroke_path(const Path& path, float width, const Paint& paint, LineCap cap = LineCap::BUTT);
    void draw_line(const Point& from, const Point& to, float width, const Paint& paint, LineCap cap = LineCap::BUTT);
    void draw_line(const Point& from, const Point& to, const StrokeStyle& style, const Paint& paint);
    /// 矩形の枠(線は r の内側)。
    void stroke_rect(const Rect& r, float width, const Color& color);
    /// r に内接する楕円。
    void fill_ellipse(const Rect& r, const Paint& paint);
    void stroke_ellipse(const Rect& r, const StrokeStyle& style, const Paint& paint);
    /// 1 行の文字(pos は左上)。描くたびに文字のレイアウトを作るので、同じ文字を何度も描くなら TextLayout を持って使う。
    void draw_text(const std::string& text, const Point& pos, const Color& color);
    void draw_text(const std::string& text, const Point& pos, const Color& color, const Font& font);
    /// 画像を描く。アイコン(Image::icon)は tint で塗り、dst の大きさに合わせて線を引く(色を決めたアイコンはその色)。
    /// ほかの画像は tint を使わない。部品は文字の色を渡す。
    void draw_image(const Image& image, const Rect& dst, const Color& tint, float opacity = 1);
};

}  // namespace tokotachi

#endif
