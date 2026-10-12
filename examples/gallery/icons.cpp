// よく使うアイコン(Image::icon。Lucide の線画): ツールバー・メニュー・ボタンでの使い方と、すべてのアイコンの一覧。
// アイコンは部品の文字の色で塗る(ダークのテーマ・無効・強調したボタンでも、文字と同じ色になる)。
//   --dark  ダークのテーマで開く(--screenshot と一緒に使うと、ダークのテーマで撮る)
#include <cstring>

#include "gallery.hpp"
#include "icon_names.hpp"

int main(int argc, char** argv)
{
    tokotachi::Application app(argc, argv);
    tokotachi::Window* window_main = new tokotachi::Window("アイコン", 720, 560);

    // メニューとツールバー(Action の icon)
    tokotachi::Action* action_new = new tokotachi::Action(window_main, "新規(&N)");
    action_new->icon = tokotachi::Image::icon(tokotachi::Icon::FILE_PLUS);
    tokotachi::Action* action_open = new tokotachi::Action(window_main, "開く(&O)...");
    action_open->icon = tokotachi::Image::icon(tokotachi::Icon::FOLDER_OPEN);
    tokotachi::Action* action_save = new tokotachi::Action(window_main, "保存(&S)");
    action_save->icon = tokotachi::Image::icon(tokotachi::Icon::SAVE);
    action_save->enabled = false;  // 無効なら、無効の文字の色で塗る
    tokotachi::Action* action_undo = new tokotachi::Action(window_main, "元に戻す(&U)");
    action_undo->icon = tokotachi::Image::icon(tokotachi::Icon::UNDO_2);
    tokotachi::Action* action_redo = new tokotachi::Action(window_main, "やり直す(&R)");
    action_redo->icon = tokotachi::Image::icon(tokotachi::Icon::REDO_2);
    tokotachi::Action* action_search = new tokotachi::Action(window_main, "検索(&F)");
    action_search->icon = tokotachi::Image::icon(tokotachi::Icon::SEARCH);
    tokotachi::MenuBar* menu_bar = new tokotachi::MenuBar();
    tokotachi::Menu* menu_file = menu_bar->add_menu("ファイル(&F)");
    menu_file->add_action(action_new);
    menu_file->add_action(action_open);
    menu_file->add_action(action_save);
    tokotachi::Menu* menu_edit = menu_bar->add_menu("編集(&E)");
    menu_edit->add_action(action_undo);
    menu_edit->add_action(action_redo);
    window_main->menu_bar = menu_bar;
    tokotachi::ToolBar* tool_bar = new tokotachi::ToolBar();
    tool_bar->add_action(action_new);
    tool_bar->add_action(action_open);
    tool_bar->add_action(action_save);
    tool_bar->add_separator();
    tool_bar->add_action(action_undo);
    tool_bar->add_action(action_redo);
    tool_bar->add_separator();
    tool_bar->add_action(action_search);
    window_main->tool_bar = tool_bar;

    tokotachi::BoxLayout* box = new tokotachi::BoxLayout(window_main);
    box->margins = tokotachi::Margins(12);
    box->spacing = 12;

    // ボタン(強調したボタンは、その文字の色で塗る。色を決めたアイコンは、いつもその色)
    tokotachi::Widget* widget_buttons = new tokotachi::Widget(window_main);
    tokotachi::BoxLayout* row = new tokotachi::BoxLayout(widget_buttons, tokotachi::Orientation::HORIZONTAL);
    row->margins = tokotachi::Margins(0);
    tokotachi::Button* button_save = new tokotachi::Button(widget_buttons, "保存");
    button_save->icon = tokotachi::Image::icon(tokotachi::Icon::SAVE);
    row->add(button_save);
    tokotachi::Button* button_copy = new tokotachi::Button(widget_buttons, "コピー");
    button_copy->icon = tokotachi::Image::icon(tokotachi::Icon::COPY);
    button_copy->style = tokotachi::ButtonStyle::SECONDARY;
    row->add(button_copy);
    tokotachi::Button* button_print = new tokotachi::Button(widget_buttons, "印刷");
    button_print->icon = tokotachi::Image::icon(tokotachi::Icon::PRINTER);
    button_print->style = tokotachi::ButtonStyle::SECONDARY;
    button_print->enabled = false;
    row->add(button_print);
    tokotachi::Button* button_delete = new tokotachi::Button(widget_buttons, "削除");
    button_delete->icon = tokotachi::Image::icon(tokotachi::Icon::TRASH, tokotachi::Color::rgb(0xDC2626));
    button_delete->style = tokotachi::ButtonStyle::SECONDARY;
    row->add(button_delete);
    row->add_stretch();
    tokotachi::ImageBox* image_box_star = new tokotachi::ImageBox(widget_buttons, tokotachi::Image::icon(tokotachi::Icon::STAR));
    image_box_star->fit = tokotachi::ImageFit::CONTAIN;  // 大きくしてもぼやけない
    image_box_star->minimum_size = tokotachi::Size(48, 48);
    row->add(image_box_star);
    box->add(widget_buttons);

    // すべてのアイコン(名前は Lucide の名前。列挙の名前は大文字にして - を _ にしたもの)
    tokotachi::ScrollArea* scroll_area_icons = new tokotachi::ScrollArea(window_main);
    tokotachi::Widget* widget_icons = new tokotachi::Widget(scroll_area_icons);
    tokotachi::GridLayout* grid = new tokotachi::GridLayout(widget_icons);
    grid->margins = tokotachi::Margins(4);
    grid->row_spacing = 6;
    grid->column_spacing = 12;
    const int count = static_cast<int>(sizeof(ICON_NAMES) / sizeof(ICON_NAMES[0]));
    const int columns = 4;
    for (int i = 0; i < count; ++i)
    {
        tokotachi::Label* label = new tokotachi::Label(widget_icons, ICON_NAMES[i]);
        label->icon = tokotachi::Image::icon(static_cast<tokotachi::Icon>(i));
        label->icon_size = 20;
        grid->add(label, i / columns, i % columns);
    }
    scroll_area_icons->content = widget_icons;
    box->add(scroll_area_icons, 1);

    bool dark = false;
    for (int i = 1; i < argc; ++i)
    {
        dark = dark || std::strcmp(argv[i], "--dark") == 0;
    }
    if (dark)
    {
        app.color_scheme = tokotachi::ColorScheme::DARK;
    }
    return run_gallery(app, window_main, argc, argv, nullptr, dark);
}
