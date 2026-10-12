// リファレンスの部品のページに載せる画像を撮る(docs/adr/0056)。部品を 1 つずつ小さなウィンドウに置き、ライトのテーマで描いて PNG にする。
//   widget_images <保存先のフォルダ>
// リファレンスを作るとき(cmake の docs のターゲット)に、画面なし(SDL_VIDEO_DRIVER=offscreen)で動かす。
// 撮った画像は、クラスの説明の @image html widget_<名前>.png で載せる。
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

#include <tokotachi/tokotachi.hpp>

namespace
{

struct Shot
{
    const char* name;  // 画像の名前(widget_<name>.png)
    float width;
    float height;
    // 部品を作って box に入れる。ダイアログを撮るときは、そのウィンドウを返す(無ければ nullptr でウィンドウを撮る)
    std::function<tokotachi::Window*(tokotachi::Window&, tokotachi::BoxLayout*)> build;
};

tokotachi::Widget* row(tokotachi::Window& w, tokotachi::BoxLayout*& layout)
{
    tokotachi::Widget* widget = new tokotachi::Widget(&w);
    layout = new tokotachi::BoxLayout(widget, tokotachi::Orientation::HORIZONTAL);
    layout->margins = tokotachi::Margins(0);
    layout->spacing = 12;
    return widget;
}

// 横長の風景の絵(空・山)。
tokotachi::Image picture()
{
    tokotachi::Bitmap bitmap(160, 90);
    tokotachi::Canvas& c = bitmap.canvas();
    std::vector<tokotachi::GradientStop> sky;
    sky.push_back(tokotachi::GradientStop(0, tokotachi::Color::rgb(0x60a5fa)));
    sky.push_back(tokotachi::GradientStop(1, tokotachi::Color::rgb(0xe0f2fe)));
    c.fill_round_rect(tokotachi::Rect(0, 0, 160, 90), 0,
                      tokotachi::Paint::linear(tokotachi::Point(0, 0), tokotachi::Point(0, 90), sky));
    tokotachi::Path mountain;
    mountain.move_to(0, 90);
    mountain.line_to(52, 34);
    mountain.line_to(82, 58);
    mountain.line_to(114, 22);
    mountain.line_to(160, 90);
    mountain.close();
    c.fill_path(mountain, tokotachi::Color::rgb(0x15803d));
    return bitmap.to_image();
}

std::vector<Shot> shots()
{
    std::vector<Shot> s;
    s.push_back(Shot{"button", 360, 64, [](tokotachi::Window& w, tokotachi::BoxLayout* box) -> tokotachi::Window*
                     {
                         tokotachi::BoxLayout* h = nullptr;
                         tokotachi::Widget* r = row(w, h);
                         tokotachi::Button* button_save = new tokotachi::Button(r, "保存");
                         button_save->icon = tokotachi::Image::icon(tokotachi::Icon::SAVE);
                         h->add(button_save);
                         tokotachi::Button* button_cancel = new tokotachi::Button(r, "キャンセル");
                         button_cancel->style = tokotachi::ButtonStyle::SECONDARY;
                         h->add(button_cancel);
                         tokotachi::Button* button_disabled = new tokotachi::Button(r, "無効");
                         button_disabled->style = tokotachi::ButtonStyle::SECONDARY;
                         button_disabled->enabled = false;
                         h->add(button_disabled);
                         h->add_stretch();
                         box->add(r);
                         return nullptr;
                     }});
    s.push_back(Shot{"check_box", 360, 64, [](tokotachi::Window& w, tokotachi::BoxLayout* box) -> tokotachi::Window*
                     {
                         tokotachi::BoxLayout* h = nullptr;
                         tokotachi::Widget* r = row(w, h);
                         tokotachi::CheckBox* check_box_on = new tokotachi::CheckBox(r, "通知を受け取る");
                         check_box_on->checked = true;
                         h->add(check_box_on);
                         h->add(new tokotachi::CheckBox(r, "自動で保存"));
                         h->add_stretch();
                         box->add(r);
                         return nullptr;
                     }});
    s.push_back(Shot{"radio_button", 360, 64, [](tokotachi::Window& w, tokotachi::BoxLayout* box) -> tokotachi::Window*
                     {
                         tokotachi::BoxLayout* h = nullptr;
                         tokotachi::Widget* r = row(w, h);
                         const char* const NAMES[] = {"小", "中", "大"};
                         for (int i = 0; i < 3; ++i)
                         {
                             tokotachi::RadioButton* radio_button = new tokotachi::RadioButton(r, NAMES[i], i + 1);
                             radio_button->group_value = 2;
                             h->add(radio_button);
                         }
                         h->add_stretch();
                         box->add(r);
                         return nullptr;
                     }});
    s.push_back(Shot{"line_edit", 360, 104, [](tokotachi::Window& w, tokotachi::BoxLayout* box) -> tokotachi::Window*
                     {
                         box->add(new tokotachi::LineEdit(&w, "Tokotachi"));
                         tokotachi::LineEdit* line_edit_search = new tokotachi::LineEdit(&w);
                         line_edit_search->placeholder_text = "検索する言葉";
                         box->add(line_edit_search);
                         return nullptr;
                     }});
    s.push_back(Shot{"text_edit", 360, 120, [](tokotachi::Window& w, tokotachi::BoxLayout* box) -> tokotachi::Window*
                     {
                         box->add(new tokotachi::TextEdit(&w, "複数行の入力欄。\n長い行は折り返し、多ければスクロールする。"), 1);
                         return nullptr;
                     }});
    s.push_back(Shot{"spin_box", 240, 64, [](tokotachi::Window& w, tokotachi::BoxLayout* box) -> tokotachi::Window*
                     {
                         tokotachi::SpinBox* spin_box_age = new tokotachi::SpinBox(&w);
                         spin_box_age->maximum = 150;
                         spin_box_age->value = 30;
                         spin_box_age->suffix = " 歳";
                         box->add(spin_box_age);
                         return nullptr;
                     }});
    s.push_back(Shot{"slider", 360, 64, [](tokotachi::Window& w, tokotachi::BoxLayout* box) -> tokotachi::Window*
                     {
                         tokotachi::Slider* slider_volume = new tokotachi::Slider(&w);
                         slider_volume->tick_interval = 10;
                         slider_volume->value = 65;
                         box->add(slider_volume);
                         return nullptr;
                     }});
    s.push_back(Shot{"combo_box", 240, 64, [](tokotachi::Window& w, tokotachi::BoxLayout* box) -> tokotachi::Window*
                     {
                         tokotachi::ComboBox* combo_box_city = new tokotachi::ComboBox(&w);
                         combo_box_city->items = std::vector<std::string>{"東京", "大阪", "名古屋"};
                         combo_box_city->current_index = 0;
                         box->add(combo_box_city);
                         return nullptr;
                     }});
    s.push_back(Shot{"date_edit", 240, 64, [](tokotachi::Window& w, tokotachi::BoxLayout* box) -> tokotachi::Window*
                     {
                         tokotachi::DateEdit* date_edit_start = new tokotachi::DateEdit(&w);
                         date_edit_start->date = tokotachi::Date(2026, 10, 11);
                         box->add(date_edit_start);
                         return nullptr;
                     }});
    s.push_back(Shot{"calendar", 300, 300, [](tokotachi::Window& w, tokotachi::BoxLayout* box) -> tokotachi::Window*
                     {
                         tokotachi::Calendar* calendar_due = new tokotachi::Calendar(&w);
                         calendar_due->date = tokotachi::Date(2026, 10, 25);
                         box->add(calendar_due, 1);
                         return nullptr;
                     }});
    s.push_back(Shot{"color_button", 240, 64, [](tokotachi::Window& w, tokotachi::BoxLayout* box) -> tokotachi::Window*
                     {
                         box->add(new tokotachi::ColorButton(&w, tokotachi::Color::rgb(0x2563EB)));
                         return nullptr;
                     }});
    s.push_back(Shot{"label", 360, 64, [](tokotachi::Window& w, tokotachi::BoxLayout* box) -> tokotachi::Window*
                     {
                         tokotachi::Label* label_info = new tokotachi::Label(&w, "画像の付いたラベル");
                         label_info->icon = tokotachi::Image::icon(tokotachi::Icon::INFO);
                         box->add(label_info);
                         return nullptr;
                     }});
    s.push_back(Shot{"progress_bar", 360, 56, [](tokotachi::Window& w, tokotachi::BoxLayout* box) -> tokotachi::Window*
                     {
                         tokotachi::ProgressBar* progress_bar_level = new tokotachi::ProgressBar(&w);
                         progress_bar_level->text_visible = true;
                         progress_bar_level->value = 65;
                         box->add(progress_bar_level);
                         return nullptr;
                     }});
    s.push_back(Shot{"image_box", 240, 140, [](tokotachi::Window& w, tokotachi::BoxLayout* box) -> tokotachi::Window*
                     {
                         tokotachi::ImageBox* image_box_photo = new tokotachi::ImageBox(&w, picture());
                         image_box_photo->fit = tokotachi::ImageFit::CONTAIN;
                         box->add(image_box_photo, 1);
                         return nullptr;
                     }});
    s.push_back(Shot{"group_box", 300, 140, [](tokotachi::Window& w, tokotachi::BoxLayout* box) -> tokotachi::Window*
                     {
                         tokotachi::GroupBox* group_box_size = new tokotachi::GroupBox(&w, "大きさ");
                         tokotachi::BoxLayout* inner = new tokotachi::BoxLayout(group_box_size);
                         const char* const NAMES[] = {"小", "中", "大"};
                         for (int i = 0; i < 3; ++i)
                         {
                             tokotachi::RadioButton* radio_button = new tokotachi::RadioButton(group_box_size, NAMES[i], i + 1);
                             radio_button->group_value = 1;
                             inner->add(radio_button);
                         }
                         box->add(group_box_size, 1);
                         return nullptr;
                     }});
    s.push_back(Shot{"tab_widget", 360, 180, [](tokotachi::Window& w, tokotachi::BoxLayout* box) -> tokotachi::Window*
                     {
                         tokotachi::TabWidget* tab_widget_main = new tokotachi::TabWidget(&w);
                         tab_widget_main->add_tab(new tokotachi::TextEdit(nullptr, "タブの中身。ページごとに好きな部品を置ける。"), "メモ");
                         tab_widget_main->add_tab(new tokotachi::Widget(), "設定");
                         tab_widget_main->add_tab(new tokotachi::Widget(), "履歴");
                         box->add(tab_widget_main, 1);
                         return nullptr;
                     }});
    s.push_back(Shot{"splitter", 420, 180, [](tokotachi::Window& w, tokotachi::BoxLayout* box) -> tokotachi::Window*
                     {
                         tokotachi::Splitter* splitter_main = new tokotachi::Splitter(&w);
                         splitter_main->add_widget(new tokotachi::TextEdit(nullptr, "左の区画"), 0);
                         splitter_main->add_widget(new tokotachi::TextEdit(nullptr, "右の区画。間の仕切りをドラッグして大きさを変える。"), 1);
                         splitter_main->sizes = std::vector<float>{140, 250};
                         box->add(splitter_main, 1);
                         return nullptr;
                     }});
    s.push_back(Shot{"scroll_area", 300, 180, [](tokotachi::Window& w, tokotachi::BoxLayout* box) -> tokotachi::Window*
                     {
                         tokotachi::ScrollArea* scroll_area_options = new tokotachi::ScrollArea(&w);
                         tokotachi::Widget* content = new tokotachi::Widget();
                         tokotachi::BoxLayout* inner = new tokotachi::BoxLayout(content);
                         for (int i = 1; i <= 12; ++i)
                         {
                             inner->add(new tokotachi::CheckBox(content, "設定の項目 " + std::to_string(i)));
                         }
                         scroll_area_options->content = content;
                         box->add(scroll_area_options, 1);
                         return nullptr;
                     }});
    s.push_back(Shot{"paint_box", 300, 160, [](tokotachi::Window& w, tokotachi::BoxLayout* box) -> tokotachi::Window*
                     {
                         tokotachi::PaintBox* paint_box_chart = new tokotachi::PaintBox(&w);
                         paint_box_chart->on_paint = [](tokotachi::Object*, tokotachi::Canvas& c)
                         {
                             const float values[] = {40, 72, 55, 90, 64};
                             for (int i = 0; i < 5; ++i)
                             {
                                 c.fill_round_rect(tokotachi::Rect(20 + i * 50.0f, 120 - values[i], 32, values[i]), 4,
                                                   tokotachi::Color::rgb(0x2563EB));
                             }
                             c.stroke_rect(tokotachi::Rect(10, 10, 256, 116), 1, tokotachi::Color::rgb(0xCBD5E1));
                         };
                         box->add(paint_box_chart, 1);
                         return nullptr;
                     }});
    s.push_back(Shot{"list_view", 420, 180, [](tokotachi::Window& w, tokotachi::BoxLayout* box) -> tokotachi::Window*
                     {
                         tokotachi::ListModel* files = new tokotachi::ListModel(&w);
                         const char* const ROWS[][3] = {{"報告書.txt", "12 KB", "2026-10-01"},
                                                        {"見積もり.xlsx", "48 KB", "2026-09-30"},
                                                        {"議事録.docx", "31 KB", "2026-10-03"}};
                         for (int i = 0; i < 3; ++i)
                         {
                             tokotachi::Item* item = files->add_item(ROWS[i][0]);
                             item->column_texts = std::vector<std::string>{ROWS[i][1], ROWS[i][2]};
                         }
                         tokotachi::ListView* list_view_files = new tokotachi::ListView(&w);
                         list_view_files->add_column("名前", 150);
                         list_view_files->add_column("大きさ", 70)->alignment = tokotachi::Align::END;
                         list_view_files->add_column("更新日", 100);
                         list_view_files->model = files;
                         list_view_files->select(files->index(1));
                         box->add(list_view_files, 1);
                         return nullptr;
                     }});
    s.push_back(Shot{"tree_view", 300, 200, [](tokotachi::Window& w, tokotachi::BoxLayout* box) -> tokotachi::Window*
                     {
                         tokotachi::TreeModel* folders = new tokotachi::TreeModel(&w);
                         tokotachi::TreeItem* documents = folders->add_item("文書");
                         documents->add_child("仕事")->add_child("報告書");
                         documents->add_child("家計簿");
                         folders->add_item("画像")->add_child("旅行");
                         folders->add_item("音楽");
                         tokotachi::TreeView* tree_view_folders = new tokotachi::TreeView(&w);
                         tree_view_folders->model = folders;
                         tree_view_folders->expand_all();
                         box->add(tree_view_folders, 1);
                         return nullptr;
                     }});
    s.push_back(Shot{"table_view", 420, 200, [](tokotachi::Window& w, tokotachi::BoxLayout* box) -> tokotachi::Window*
                     {
                         tokotachi::ListModel* scores = new tokotachi::ListModel(&w);
                         const char* const NAMES[] = {"佐藤", "鈴木", "高橋", "田中"};
                         const char* const POINTS[][3] = {{"82", "75", "90"}, {"64", "88", "71"}, {"95", "92", "87"}, {"70", "66", "79"}};
                         for (int i = 0; i < 4; ++i)
                         {
                             tokotachi::Item* item = scores->add_item(NAMES[i]);
                             item->column_texts = std::vector<std::string>{POINTS[i][0], POINTS[i][1], POINTS[i][2]};
                         }
                         tokotachi::TableView* table_view_scores = new tokotachi::TableView(&w);
                         table_view_scores->add_column("名前", 90);
                         const char* const SUBJECTS[] = {"国語", "数学", "英語"};
                         for (int k = 0; k < 3; ++k)
                         {
                             table_view_scores->add_column(SUBJECTS[k], 70)->alignment = tokotachi::Align::END;
                         }
                         table_view_scores->model = scores;
                         table_view_scores->selection_mode = tokotachi::SelectionMode::MULTI;
                         table_view_scores->select_range(tokotachi::CellRange(1, 1, 2, 3));
                         box->add(table_view_scores, 1);
                         return nullptr;
                     }});
    s.push_back(Shot{"window_bars", 420, 200, [](tokotachi::Window& w, tokotachi::BoxLayout* box) -> tokotachi::Window*
                     {
                         tokotachi::Action* action_open = new tokotachi::Action(&w, "開く(&O)...");
                         action_open->icon = tokotachi::Image::icon(tokotachi::Icon::FOLDER_OPEN);
                         tokotachi::Action* action_save = new tokotachi::Action(&w, "保存(&S)");
                         action_save->icon = tokotachi::Image::icon(tokotachi::Icon::SAVE);
                         tokotachi::MenuBar* menu_bar = new tokotachi::MenuBar();
                         tokotachi::Menu* menu_file = menu_bar->add_menu("ファイル(&F)");
                         menu_file->add_action(action_open);
                         menu_file->add_action(action_save);
                         menu_bar->add_menu("編集(&E)");
                         menu_bar->add_menu("表示(&V)");
                         w.menu_bar = menu_bar;
                         tokotachi::ToolBar* tool_bar = new tokotachi::ToolBar();
                         tool_bar->add_action(action_open);
                         tool_bar->add_action(action_save);
                         w.tool_bar = tool_bar;
                         tokotachi::StatusBar* status_bar = new tokotachi::StatusBar();
                         status_bar->text = "準備完了";
                         w.status_bar = status_bar;
                         box->add(new tokotachi::TextEdit(&w, "メニューバー・ツールバー・ステータスバーを付けたウィンドウ。"), 1);
                         return nullptr;
                     }});
    s.push_back(Shot{"message_dialog", 420, 240, [](tokotachi::Window& w, tokotachi::BoxLayout*) -> tokotachi::Window*
                     {
                         w.visible = true;
                         return tokotachi::MessageDialog::show(
                             &w, "確認", "変更を保存しますか？ 保存しないと、編集した内容は失われます。",
                             std::vector<tokotachi::DialogButton>{tokotachi::DialogButton::SAVE, tokotachi::DialogButton::DISCARD,
                                                                 tokotachi::DialogButton::CANCEL},
                             nullptr, tokotachi::MessageType::QUESTION);
                     }});
    return s;
}

}  // namespace

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::fprintf(stderr, "usage: widget_images <folder>\n");
        return 2;
    }
    const std::string dir = argv[1];
    tokotachi::Application app(argc, argv);
    app.color_scheme = tokotachi::ColorScheme::LIGHT;
    app.language = "ja";
    int failed = 0;
    const std::vector<Shot> list = shots();
    for (std::size_t i = 0; i < list.size(); ++i)
    {
        tokotachi::Window window("", list[i].width, list[i].height);
        tokotachi::BoxLayout* box = new tokotachi::BoxLayout(&window);
        box->margins = tokotachi::Margins(12);
        box->spacing = 8;
        tokotachi::Window* target = list[i].build(window, box);
        window.visible = true;
        const std::string path = dir + "/widget_" + list[i].name + ".png";
        if (!(target ? target : &window)->save_png(path))
        {
            std::fprintf(stderr, "widget_images: cannot save %s\n", path.c_str());
            ++failed;
        }

    }
    return failed == 0 ? 0 : 1;
}
