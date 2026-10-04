// 書き換えられる画像(docs/adr/0033)。Canvas で描け、画素を直接読み書きできる。画面に出すときは Image にする。
//
//   gui::Bitmap chart(400, 300);                 // 400 × 300 ピクセル(透明)
//   gui::Canvas& c = chart.canvas();             // 画面と同じ API で描く(座標は dp = ピクセル / scale)
//   c.fill_ellipse(gui::Rect(10, 10, 100, 100), gui::Color::rgb(0x2563eb));
//   chart.set_pixel(0, 0, gui::Color::rgb(0xff0000));
//   gui::Image image = chart.to_image();         // 画素を写した、変更しない画像
//   ... canvas.draw_image(image, gui::Rect(0, 0, 400, 300)) ...
//
// 重い描画を前もって描いておき、paint では貼るだけにするときに使う(PaintBox の cached はこれを自動で行う)。
// 1 つの Bitmap は、同時に 1 つのスレッドからだけ使う。今は、文字を描けるのはメインスレッドだけ(フォントのキャッシュを
// スレッドをまたいで共有しているため。別のスレッドでの文字の描画は後で対応する)。
#ifndef GUI_BITMAP_HPP
#define GUI_BITMAP_HPP

#include <cstdint>

#include "canvas.hpp"
#include "geometry.hpp"
#include "image.hpp"

namespace gui
{

class Bitmap
{
public:
    // 空の画像(is_null() が true)。
    Bitmap();
    // width × height ピクセルの透明な画像。scale は dp からピクセルへの倍率(canvas() の座標は dp)。
    Bitmap(int width, int height, float scale = 1);
    ~Bitmap();

    Bitmap(Bitmap&& other);
    Bitmap& operator=(Bitmap&& other);
    Bitmap(const Bitmap&) = delete;
    Bitmap& operator=(const Bitmap&) = delete;

    // image の画素を写した Bitmap(読み込んだ画像を編集するときなど)。
    static Bitmap from_image(const Image& image, float scale = 1);

    bool is_null() const { return d_ == nullptr; }
    // 大きさ(ピクセル)と、dp での大きさ。
    int width() const;
    int height() const;
    float scale() const;
    Size size() const;

    // この画像に描く Canvas(Bitmap が生きている間有効。変換とクリップは save・restore で戻す)。
    Canvas& canvas();
    // すべての画素を color にする。
    void clear(const Color& color = Color(0, 0, 0, 0));
    // 画素(範囲の外なら透明・何もしない)。色はアルファを乗算していない値。
    Color pixel(int x, int y) const;
    void set_pixel(int x, int y, const Color& color);
    // 画素の並び(行ごとに width 個。各画素はアルファ乗算済みの 0xAARRGGBB)。速く処理したいときに使う。
    std::uint32_t* pixels();
    const std::uint32_t* pixels() const;

    // 画素を写した、変更しない画像(後で Bitmap に描いても変わらない)。
    Image to_image() const;

private:
    struct Impl;
    Impl* d_;
};

}  // namespace gui

#endif
