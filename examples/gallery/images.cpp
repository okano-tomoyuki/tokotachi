// 画像: ImageBox の合わせ方、ラベル・ボタンの画像、メニュー・ツールバーの画像(Action の icon)。
// ふつうは Image::load("open.png") でファイルから読む。ここではファイルを使わないよう、Bitmap に描いて作る。
#include "gallery.hpp"

namespace
{

// 32×32 の丸い印(倍率 2 の画面でもにじまないよう、16 の 2 倍で描く)。
tokotachi::Image make_icon(const tokotachi::Color& color, bool square)
{
    tokotachi::Bitmap bitmap(32, 32);
    tokotachi::Canvas& c = bitmap.canvas();
    if (square)
    {
        c.fill_round_rect(tokotachi::Rect(4, 4, 24, 24), 5, color);
    }
    else
    {
        tokotachi::Path dot;
        dot.add_ellipse(tokotachi::Point(16, 16), 12, 12);
        c.fill_path(dot, color);
    }
    c.fill_round_rect(tokotachi::Rect(11, 11, 10, 10), 2, tokotachi::Color::rgb(0xffffff));
    return bitmap.to_image();
}

// 横長の風景の絵(空・山。90×45)。
tokotachi::Image make_picture()
{
    tokotachi::Bitmap bitmap(90, 45);
    tokotachi::Canvas& c = bitmap.canvas();
    c.scale_by(0.75f, 0.75f);  // 120×60 の座標で描く
    std::vector<tokotachi::GradientStop> sky;
    sky.push_back(tokotachi::GradientStop(0, tokotachi::Color::rgb(0x60a5fa)));
    sky.push_back(tokotachi::GradientStop(1, tokotachi::Color::rgb(0xe0f2fe)));
    c.fill_round_rect(tokotachi::Rect(0, 0, 120, 60), 0,
                      tokotachi::Paint::linear(tokotachi::Point(0, 0), tokotachi::Point(0, 60), sky));
    tokotachi::Path mountain;
    mountain.move_to(0, 60);
    mountain.line_to(40, 22);
    mountain.line_to(62, 40);
    mountain.line_to(86, 14);
    mountain.line_to(120, 60);
    mountain.close();
    c.fill_path(mountain, tokotachi::Color::rgb(0x15803d));
    return bitmap.to_image();
}

}  // namespace

int main(int argc, char** argv)
{
    tokotachi::Application app(argc, argv);
    tokotachi::Window window("画像", 560, 330);
    const tokotachi::Image icon_open = make_icon(tokotachi::Color::rgb(0xf59e0b), true);
    const tokotachi::Image icon_save = make_icon(tokotachi::Color::rgb(0x2563eb), true);
    const tokotachi::Image icon_grid = make_icon(tokotachi::Color::rgb(0x16a34a), false);
    const tokotachi::Image icon_info = make_icon(tokotachi::Color::rgb(0x0891b2), false);

    // メニューとツールバーに、同じ Action の画像を出す(チェックしている項目は、メニューで画像を枠で囲む)
    tokotachi::Action* action_open = new tokotachi::Action(&window, "開く(&O)...");
    action_open->icon = icon_open;
    action_open->shortcut = "Ctrl+O";
    tokotachi::Action* action_save = new tokotachi::Action(&window, "保存(&S)");
    action_save->icon = icon_save;
    action_save->shortcut = "Ctrl+S";
    tokotachi::Action* action_grid = new tokotachi::Action(&window, "方眼(&G)");
    action_grid->icon = icon_grid;
    action_grid->checkable = true;
    action_grid->checked = true;
    tokotachi::MenuBar* menu_bar = new tokotachi::MenuBar();
    tokotachi::Menu* menu_file = menu_bar->add_menu("ファイル(&F)");
    menu_file->add_action(action_open);
    menu_file->add_action(action_save);
    menu_file->add_separator();
    menu_file->add_item("終了(&X)", [&](tokotachi::Object*) { window.close(); });
    menu_bar->add_menu("表示(&V)")->add_action(action_grid);
    window.menu_bar = menu_bar;
    tokotachi::ToolBar* tool_bar = new tokotachi::ToolBar();
    tool_bar->add_action(action_open);
    tool_bar->add_action(action_save);
    tool_bar->add_separator();
    tool_bar->add_action(action_grid);
    window.tool_bar = tool_bar;

    tokotachi::BoxLayout* box = new tokotachi::BoxLayout(&window);
    box->margins = tokotachi::Margins(12);

    // ImageBox: 同じ絵を、合わせ方を変えて並べる
    tokotachi::Widget* widget_pictures = new tokotachi::Widget(&window);
    tokotachi::GridLayout* grid = new tokotachi::GridLayout(widget_pictures);
    grid->margins = tokotachi::Margins(0);
    const tokotachi::Image picture = make_picture();
    const char* names[] = {"NONE", "CONTAIN", "COVER", "STRETCH"};
    const tokotachi::ImageFit fits[] = {tokotachi::ImageFit::NONE, tokotachi::ImageFit::CONTAIN, tokotachi::ImageFit::COVER,
                                        tokotachi::ImageFit::STRETCH};
    for (int i = 0; i < 4; ++i)
    {
        tokotachi::ImageBox* image_box = new tokotachi::ImageBox(widget_pictures, picture);
        image_box->fit = fits[i];
        image_box->minimum_size = tokotachi::Size(110, 100);
        grid->add(image_box, 0, i);
        grid->add(new tokotachi::Label(widget_pictures, names[i]), 1, i, 1, 1, tokotachi::Align::CENTER);
    }
    box->add(widget_pictures, 1);

    // ラベル・ボタンの画像(文字が無ければ画像だけのボタン)
    tokotachi::Widget* widget_buttons = new tokotachi::Widget(&window);
    tokotachi::BoxLayout* row = new tokotachi::BoxLayout(widget_buttons, tokotachi::Orientation::HORIZONTAL);
    row->margins = tokotachi::Margins(0);
    tokotachi::Label* label_info = new tokotachi::Label(widget_buttons, "画像つきのラベル");
    label_info->icon = icon_info;
    row->add(label_info, 0, tokotachi::Align::CENTER);
    row->add_stretch();
    tokotachi::Button* button_open = new tokotachi::Button(widget_buttons, "開く");
    button_open->icon = icon_open;
    button_open->style = tokotachi::ButtonStyle::SECONDARY;
    row->add(button_open);
    tokotachi::Button* button_save = new tokotachi::Button(widget_buttons, "保存");
    button_save->icon = icon_save;
    row->add(button_save);
    tokotachi::Button* button_grid = new tokotachi::Button(widget_buttons);
    button_grid->icon = icon_grid;
    button_grid->style = tokotachi::ButtonStyle::SECONDARY;
    button_grid->tooltip = "方眼";
    row->add(button_grid);
    box->add(widget_buttons);
    return run_gallery(app, window, argc, argv);
}
