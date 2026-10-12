/// @file
/// tokotachi::Canvas の宣言。
#ifndef TOKOTACHI_GRAPHICS_CANVAS_HPP
#define TOKOTACHI_GRAPHICS_CANVAS_HPP

#include <string>
#include <vector>

#include "../core/export.hpp"
#include "../core/geometry.hpp"

namespace tokotachi
{

class Image;
class TextLayout;
struct Font;

namespace detail
{
template <typename C>
struct ImplAccess;
}

/// 図形の輪郭(直線・ベジェ曲線・矩形・楕円・弧を組み合わせる)。Canvas::fill_path・stroke_path で描く。
///
/// @ingroup graphics
class TOKOTACHI_API Path
{
public:
    /// 輪郭の要素の種類。
    enum class Op
    {
        MOVE_TO,
        LINE_TO,
        CUBIC_TO,
        CLOSE
    };

    /// 輪郭の要素(種類と点。CUBIC_TO は制御点 2 つと終点の 3 点、MOVE_TO・LINE_TO は 1 点)。
    struct Element
    {
        /// 種類。
        Op op;
        /// 点の座標(x, y の順に並べる。使わない分は 0)。
        float pts[6];
    };

    /// 新しい図形を (x, y) から始める。
    void move_to(float x, float y);
    /// 今の点から (x, y) へ直線を引く。
    void line_to(float x, float y);
    /// 2 次ベジェ(3 次ベジェに直して持つ)。
    void quad_to(float x1, float y1, float x2, float y2);
    /// 3 次ベジェ((x1, y1)・(x2, y2) が制御点、(x3, y3) が終点)。
    void cubic_to(float x1, float y1, float x2, float y2, float x3, float y3);
    /// 今の図形を閉じる(始点へ直線でつなぐ)。
    void close();
    /// 矩形を 1 つの図形として加える。
    void add_rect(const Rect& r);
    /// 角を丸めた矩形を加える。
    void add_round_rect(const Rect& r, float radius);
    /// 楕円を加える(rx・ry は半径)。
    void add_ellipse(const Point& center, float rx, float ry);
    /// 楕円の弧(角度はラジアン。0 が右、正の向きは時計回り(y が下向きのため))。new_figure なら弧の始点へ move_to する
    /// (false なら今の点から線でつなぐ)。
    void add_arc(const Point& center, float rx, float ry, float start_angle, float sweep_angle, bool new_figure = true);
    /// 折れ線(close なら閉じる)。
    void add_polyline(const std::vector<Point>& points, bool close = false);

    /// 要素が無いか。
    bool is_empty() const { return elements_.empty(); }
    /// 要素の並び。
    const std::vector<Element>& elements() const { return elements_; }

private:
    std::vector<Element> elements_;
};

/// グラデーションの色の位置(offset は 0〜1)。
///
/// @ingroup graphics
struct TOKOTACHI_API GradientStop
{
    /// 位置(0〜1。グラデーションの始まりが 0、終わりが 1)。
    float offset;
    /// その位置の色。
    Color color;

    GradientStop() : offset(0) {}
    /// 位置と色から作る。
    GradientStop(float offset_, const Color& color_) : offset(offset_), color(color_) {}
};

/// 塗り(単色・線形グラデーション・放射グラデーション)。
/// @ingroup graphics
struct TOKOTACHI_API Paint
{
    /// 塗りの種類。
    enum class Type
    {
        SOLID,
        LINEAR,
        RADIAL
    };

    /// 種類。
    Type type;
    /// SOLID の色。
    Color color;
    /// グラデーションの点(LINEAR: 始点・終点。RADIAL: p0 が中心、p1 が焦点)。
    ///@{
    Point p0, p1;  ///< LINEAR: 始点・終点。RADIAL: p0 が中心、p1 が焦点
    ///@}
    float radius;  ///< RADIAL
    /// グラデーションの色の位置(offset の順)。
    std::vector<GradientStop> stops;

    Paint() : type(Type::SOLID), radius(0) {}
    /// 色から暗黙に作れるようにする(canvas.fill_round_rect(r, 4, color))。
    Paint(const Color& c) : type(Type::SOLID), color(c), radius(0) {}

    /// from から to への線形グラデーション。
    static Paint linear(const Point& from, const Point& to, const std::vector<GradientStop>& stops);
    /// center を中心とする半径 radius の放射グラデーション(focal は焦点)。
    static Paint radial(const Point& center, float radius, const Point& focal, const std::vector<GradientStop>& stops);
};

/// 線の端の形。
enum class LineCap
{
    BUTT,
    ROUND,
    SQUARE
};

/// 線の角の形。
enum class LineJoin
{
    MITER,
    ROUND,
    BEVEL
};

/// 線の形。
/// @ingroup graphics
struct TOKOTACHI_API StrokeStyle
{
    float width;                ///< 太さ(dp)
    LineCap cap;                ///< 端の形
    LineJoin join;              ///< 角の形
    std::vector<float> dashes;  ///< 破線の線と隙間の長さ(dp)を交互に。空なら実線
    float dash_offset;          ///< 破線の始まりのずらし(dp)

    /// 太さと端・角の形から作る(実線)。
    StrokeStyle(float width_ = 1, LineCap cap_ = LineCap::BUTT, LineJoin join_ = LineJoin::MITER)
        : width(width_), cap(cap_), join(join_), dash_offset(0)
    {
    }
};

/// 描く面(docs/adr/0006、0032、0060)。ライブラリが作って渡すので、利用者は作らない・派生させない。
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
class TOKOTACHI_API Canvas
{
public:
    /// dp からピクセルへの倍率。
    float scale() const;

    // ---- 状態 ----
    /// 変換とクリップを積む・戻す。
    void save();
    /// 最後の save の時点の変換とクリップに戻す。
    void restore();
    /// 変換: 以後の座標を、今の座標系で移動・拡大・回転(ラジアン。正の向きは時計回り)する。
    void translate(float dx, float dy);
    /// 以後の座標を拡大・縮小する。
    void scale_by(float sx, float sy);
    /// 以後の座標を回す(ラジアン。正の向きは時計回り)。
    void rotate(float radians);
    /// 現在のクリップと r(または path の内側)の共通部分に描画を限る。
    void clip_rect(const Rect& r);
    /// 現在のクリップと path の内側の共通部分に描画を限る。
    void clip_path(const Path& path);
    /// 現在のクリップを囲む矩形(dp、今の座標系)。
    Rect clip_bounds() const;
    /// グループの透過: begin_layer から end_layer までに描いたものを、一時的な画像に描いてから opacity(0〜1)で重ねる
    /// (重なった部分も濃くならない)。対で呼ぶ(入れ子にできる)。begin_layer は save も兼ね、end_layer で変換とクリップも戻る。
    void begin_layer(float opacity);
    /// begin_layer で始めたグループを重ねる(変換とクリップも戻る)。
    void end_layer();

    // ---- 基本の描画 ----
    /// 矩形を塗る。
    void fill_rect(const Rect& r, const Color& color);
    /// 角を丸めた矩形を塗る。
    void fill_round_rect(const Rect& r, float radius, const Paint& paint);
    /// 線を r の内側に描く(線の外側の辺が r に一致する)。
    void stroke_round_rect(const Rect& r, float radius, float width, const Color& color);
    /// 輪郭の内側を塗る。
    void fill_path(const Path& path, const Paint& paint);
    /// 輪郭に沿って線を引く。
    void stroke_path(const Path& path, const StrokeStyle& style, const Paint& paint);
    /// pos はレイアウトの左上(ベースラインは pos.y + layout.ascent())。
    void draw_text(const TextLayout& layout, const Point& pos, const Color& color);
    /// 画像を矩形 dst(dp)に合わせて伸縮して描く。opacity は 0〜1(無効な部品のアイコンを薄くするときなど)。
    /// アイコン(Image::icon)は、色を決めていなければ黒で塗る(部品の文字の色で塗るには、下の tint を渡す形を使う)。
    void draw_image(const Image& image, const Rect& dst, float opacity = 1);

    // ---- 手軽な描画(上の関数で描く) ----
    /// 輪郭に沿って線を引く(太さと端の形だけを決める)。
    void stroke_path(const Path& path, float width, const Paint& paint, LineCap cap = LineCap::BUTT);
    /// 直線を引く。
    void draw_line(const Point& from, const Point& to, float width, const Paint& paint, LineCap cap = LineCap::BUTT);
    /// 直線を引く(線の形を決める)。
    void draw_line(const Point& from, const Point& to, const StrokeStyle& style, const Paint& paint);
    /// 矩形の枠(線は r の内側)。
    void stroke_rect(const Rect& r, float width, const Color& color);
    /// r に内接する楕円。
    void fill_ellipse(const Rect& r, const Paint& paint);
    /// r に内接する楕円の線を引く。
    void stroke_ellipse(const Rect& r, const StrokeStyle& style, const Paint& paint);
    /// 1 行の文字(pos は左上)。描くたびに文字のレイアウトを作るので、同じ文字を何度も描くなら TextLayout を持って使う。
    void draw_text(const std::string& text, const Point& pos, const Color& color);
    /// 1 行の文字をフォントを決めて描く(pos は左上)。
    void draw_text(const std::string& text, const Point& pos, const Color& color, const Font& font);
    /// 画像を描く。アイコン(Image::icon)は tint で塗り、dst の大きさに合わせて線を引く(色を決めたアイコンはその色)。
    /// ほかの画像は tint を使わない。部品は文字の色を渡す。
    void draw_image(const Image& image, const Rect& dst, const Color& tint, float opacity = 1);

private:
    template <typename>
    friend struct detail::ImplAccess;
    class Impl;
    /// ライブラリの中だけで作る(描画先は backend。所有しない)。
    explicit Canvas(Impl* backend) : d_(backend) {}
    Canvas(const Canvas&) = delete;
    Canvas& operator=(const Canvas&) = delete;

    Impl* d_;
};

}  // namespace tokotachi

#endif
