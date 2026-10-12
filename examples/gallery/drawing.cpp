// 自由な描画: PaintBox の on_paint で Canvas に描く(図形・グラデーション・回転・文字・透過)。
#include "gallery.hpp"

int main(int argc, char** argv)
{
    tokotachi::Application app(argc, argv);
    tokotachi::Window* window_main = new tokotachi::Window("描画", 420, 220);
    tokotachi::BoxLayout* box = new tokotachi::BoxLayout(window_main);
    box->margins = tokotachi::Margins(0);
    tokotachi::PaintBox* paint_box_chart = new tokotachi::PaintBox(window_main);
    box->add(paint_box_chart, 1);

    paint_box_chart->on_paint = [](tokotachi::Object*, tokotachi::Canvas& c)
    {
        // 棒グラフ。
        const float values[] = {40, 75, 55, 90, 65};
        for (int i = 0; i < 5; ++i)
        {
            c.fill_round_rect(tokotachi::Rect(20 + i * 30.0f, 170 - values[i], 22, values[i]), 4,
                              tokotachi::Color::rgb(0x2563eb));
        }
        c.draw_line(tokotachi::Point(14, 170), tokotachi::Point(170, 170), 1.5f, tokotachi::Color::rgb(0x6b7280));
        c.draw_text("売上", tokotachi::Point(20, 180), tokotachi::Color::rgb(0x374151));

        // 円グラフ(円弧のパス)。
        const float shares[] = {0.45f, 0.3f, 0.25f};
        const std::uint32_t colors[] = {0x059669, 0xf59e0b, 0xdc2626};
        float angle = -1.5707963f;
        for (int i = 0; i < 3; ++i)
        {
            tokotachi::Path slice;
            slice.move_to(260, 95);
            slice.add_arc(tokotachi::Point(260, 95), 55, 55, angle, shares[i] * 6.2831853f, false);
            slice.close();
            c.fill_path(slice, tokotachi::Color::rgb(colors[i]));
            angle += shares[i] * 6.2831853f;
        }

        // 回転とグラデーション、グループの透過。
        c.save();
        c.translate(370, 60);
        c.rotate(0.4f);
        c.fill_round_rect(tokotachi::Rect(-35, -20, 70, 40), 8,
                          tokotachi::Paint::linear(tokotachi::Point(-35, 0), tokotachi::Point(35, 0),
                                                   std::vector<tokotachi::GradientStop>{
                                                       tokotachi::GradientStop(0, tokotachi::Color::rgb(0x2563eb)),
                                                       tokotachi::GradientStop(1, tokotachi::Color::rgb(0x059669))}));
        c.draw_text("回転", tokotachi::Point(-16, -9), tokotachi::Color::rgb(0xffffff));
        c.restore();
        c.begin_layer(0.5f);
        c.fill_ellipse(tokotachi::Rect(340, 120, 50, 50), tokotachi::Color::rgb(0x2563eb));
        c.fill_ellipse(tokotachi::Rect(365, 120, 50, 50), tokotachi::Color::rgb(0xdc2626));
        c.end_layer();
    };
    return run_gallery(app, window_main, argc, argv);
}
